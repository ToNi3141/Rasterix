// Rasterix
// https://github.com/ToNi3141/Rasterix
// Copyright (c) 2023 ToNi3141

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#ifndef RENDERER_MEMORY_OPTIMIZED_HPP
#define RENDERER_MEMORY_OPTIMIZED_HPP

#include <stdint.h>
#include <array>
#include "Vec.hpp"
#include "IRenderer.hpp"
#include "registers/BaseColorReg.hpp"
#include "IBusConnector.hpp"
#include "DisplayList.hpp"
#include "Rasterizer.hpp"
#include <string.h>
#include "DisplayListAssembler.hpp"
#include <algorithm>
#include "TextureMemoryManager.hpp"
#include <limits>

#include "registers/TexEnvReg.hpp"
#include "registers/FragmentPipelineReg.hpp"
#include "registers/FeatureEnableReg.hpp"
#include "registers/TmuTextureReg.hpp"
#include "registers/ColorBufferClearColorReg.hpp"
#include "registers/DepthBufferClearDepthReg.hpp"
#include "registers/FogColorReg.hpp"
#include "registers/RenderResolutionReg.hpp"
#include "registers/ScissorEndReg.hpp"
#include "registers/ScissorStartReg.hpp"
#include "registers/TexEnvColorReg.hpp"
#include "registers/YOffsetReg.hpp"
#include "registers/StencilReg.hpp"
#include "registers/ColorBufferAddrReg.hpp"
#include "registers/DepthBufferAddrReg.hpp"
#include "registers/StencilBufferAddrReg.hpp"
#include "commands/TriangleStreamCmd.hpp"
#include "commands/FogLutStreamCmd.hpp"
#include "commands/FramebufferCmd.hpp"
#include "commands/TextureStreamCmd.hpp"
#include "commands/WriteRegisterCmd.hpp"
#include "RenderConfigs.hpp"

namespace rr
{
// Screen
// <-----------------X_RESOLUTION--------------------------->
// +--------------------------------------------------------+ ^
// |        ^                                               | |
// |        | m_yLineResolution      m_displayLines         | |
// |        |                                               | |
// |        v                                               | |
// |<------------------------------------------------------>| Y
// |                                                        | _
// |                                 m_displayLines         | R
// |                                                        | E
// |                                                        | S
// |<------------------------------------------------------>| O
// |                                                        | L
// |                                 m_displayLines         | U
// |                                                        | T
// |                                                        | I
// |<------------------------------------------------------>| O
// |                                                        | N
// |                                 m_displayLines         | |
// |                                                        | |
// |                                                        | |
// +--------------------------------------------------------+ v
// This renderer collects all triangles in a single display list. It will create only one display list and will transfer
// this display list several times but slightly modified (in respect to the display lines) to the renderer.
// This approach is slower compared to unique display lists for each line, but saves memory on the target.
// The RenderConfig::CMD_STREAM_WIDTH is used to calculate the alignment in the display list.
template <class RenderConfig>
class RendererMemoryOptimized : public IRenderer
{
    static constexpr uint16_t DISPLAY_LINES { ((RenderConfig::MAX_DISPLAY_WIDTH * RenderConfig::MAX_DISPLAY_HEIGHT * 2) / RenderConfig::INTERNAL_FRAMEBUFFER_SIZE) + 1 };
public:
    RendererMemoryOptimized(IBusConnector& busConnector)
        : m_busConnector(busConnector)
    {
        for (auto& entry : m_displayListAssembler)
        {
            entry.clearAssembler();
        }
        auto buffer1 = m_busConnector.requestBuffer(0);
        auto buffer2 = m_busConnector.requestBuffer(1);
        m_displayListAssembler[0].setBuffer(buffer1);
        m_displayListAssembler[1].setBuffer(buffer2);

        if constexpr (RenderConfig::FRAMEBUFFER_TYPE == FramebufferType::INTERNAL_TO_MEMORY)
        {
            setColorBufferAddress(RenderConfig::COLOR_BUFFER_LOC_2);
            enableColorBufferInMemory();
        }
        if constexpr (RenderConfig::FRAMEBUFFER_TYPE == FramebufferType::INTERNAL_TO_STREAM)
        {
            enableColorBufferStream();
        }
        if constexpr ((RenderConfig::FRAMEBUFFER_TYPE == FramebufferType::EXTERNAL_MEMORY_DOUBLE_BUFFER)
            || (RenderConfig::FRAMEBUFFER_TYPE == FramebufferType::EXTERNAL_MEMORY_TO_STREAM))
        {
            setColorBufferAddress(RenderConfig::COLOR_BUFFER_LOC_1);
            setDepthBufferAddress(RenderConfig::DEPTH_BUFFER_LOC);
            setStencilBufferAddress(RenderConfig::STENCIL_BUFFER_LOC);
        }

        setRenderResolution(RenderConfig::MAX_DISPLAY_WIDTH, RenderConfig::MAX_DISPLAY_HEIGHT);
    }

    virtual ~RendererMemoryOptimized()
    {
        setColorBufferAddress(RenderConfig::COLOR_BUFFER_LOC_1);
        render();
    }

    virtual bool drawTriangle(const Triangle& triangle) override
    {
        TriangleStreamCmd<IRenderer::MAX_TMU_COUNT> triangleCmd { m_rasterizer, triangle };

        if (!triangleCmd.isVisible())
        {
            // Triangle is not visible
            return true;
        }

        return m_displayListAssembler[m_backList].addCommand(triangleCmd);
    }

    virtual void render() override 
    {
        // Switch the display lists
        if (m_backList == 0)
        {
            m_backList = 1;
            m_frontList = 0;
        }
        else
        {
            m_backList = 0;
            m_frontList = 1;
        }

        // Swap frame
        // Display list zero is always the last list, and this list is responsible to set the overall SoC state, like
        // the address for the display output
        m_displayListAssembler[m_frontList].addCommand(createSwapFramebufferCommand()); 

        uploadTextures();
        uploadDisplayList();
        m_displayListAssembler[m_backList].clearAssembler();
        swapFramebuffer();
    }

    virtual bool clear(const bool colorBuffer, const bool depthBuffer, const bool stencilBuffer) override
    {
        FramebufferCmd cmd { colorBuffer, depthBuffer, stencilBuffer };
        cmd.enableMemset();
        return m_displayListAssembler[m_backList].addCommand(cmd);
    }

    virtual bool setClearColor(const Vec4i& color) override
    {
        ColorBufferClearColorReg reg;
        reg.setColor(color);
        return writeReg(reg);
    }

    virtual bool setClearDepth(uint16_t depth) override
    {
        DepthBufferClearDepthReg reg;
        reg.setValue(depth);
        return writeReg(reg);
    }

    virtual bool setFragmentPipelineConfig(const FragmentPipelineReg& pipelineConf) override 
    {
        return writeReg(pipelineConf);
    }

    virtual bool setTexEnv(const TexEnvReg& texEnvConfig) override
    {
        return writeReg(texEnvConfig);
    }

    virtual bool setTexEnvColor(const TMU target, const Vec4i& color) override
    {
        TexEnvColorReg reg;
        reg.setTmu(target);
        reg.setColor(color);
        return writeReg(reg);
    }

    virtual bool setFogColor(const Vec4i& color) override
    {
        FogColorReg reg;
        reg.setColor(color);
        return writeReg(reg);
    }

    virtual bool setFogLut(const std::array<float, 33>& fogLut, float start, float end) override
    {
        bool ret = true;
        const FogLutStreamCmd fogLutDesc { fogLut, start, end };
        return m_displayListAssembler[m_backList].addCommand(fogLutDesc);
    }

    virtual std::pair<bool, uint16_t> createTexture() override
    {
        return m_textureManager.createTexture();
    }

    virtual bool updateTexture(const uint16_t texId, const TextureObjectMipmap& textureObject) override
    {
        return m_textureManager.updateTexture(texId, textureObject);
    }

    virtual TextureObjectMipmap getTexture(const uint16_t texId) override
    {
        return m_textureManager.getTexture(texId);
    }

    virtual bool useTexture(const TMU target, const uint16_t texId) override 
    {
        m_boundTextures[target] = texId;
        if (!m_textureManager.textureValid(texId))
        {
            return false;
        }
        bool ret { true };
        const std::span<const uint16_t> pages = m_textureManager.getPages(texId);
        using Command = TextureStreamCmd<RenderConfig>;
        ret = ret && m_displayListAssembler[m_backList].addCommand(Command { target, pages });
        TmuTextureReg reg = m_textureManager.getTmuConfig(texId);
        reg.setTmu(target);
        ret = ret && m_displayListAssembler[m_backList].addCommand(WriteRegisterCmd { reg });
        return ret;
    }

    virtual bool deleteTexture(const uint16_t texId) override 
    {
        return m_textureManager.deleteTexture(texId);
    }

    virtual bool setFeatureEnableConfig(const FeatureEnableReg& featureEnable) override
    {
        m_rasterizer.enableScissor(featureEnable.getEnableScissor());
        static_assert(IRenderer::MAX_TMU_COUNT == 2, "Adapt following code when the TMU count has changed");
        m_rasterizer.enableTmu(0, featureEnable.getEnableTmu(0));
        m_rasterizer.enableTmu(1, featureEnable.getEnableTmu(1));
        return writeReg(featureEnable);
    }

    virtual bool setScissorBox(const int32_t x, const int32_t y, const uint32_t width, const uint32_t height) override
    {
        bool ret = true;

        ScissorStartReg regStart;
        ScissorEndReg regEnd;
        regStart.setX(x);
        regStart.setY(y);
        regEnd.setX(x + width);
        regEnd.setY(y + height);

        ret = ret && writeReg(regStart);
        ret = ret && writeReg(regEnd);

        m_rasterizer.setScissorBox(x, y, width, height);

        return ret;
    }

    virtual bool setTextureWrapModeS(const uint16_t texId, TextureWrapMode mode) override
    {
        m_textureManager.setTextureWrapModeS(texId, mode);
        return writeToTextureConfig(texId, m_textureManager.getTmuConfig(texId));
    }

    virtual bool setTextureWrapModeT(const uint16_t texId, TextureWrapMode mode) override
    {
        m_textureManager.setTextureWrapModeT(texId, mode);
        return writeToTextureConfig(texId, m_textureManager.getTmuConfig(texId)); 
    }

    virtual bool enableTextureMagFiltering(const uint16_t texId, bool filter) override
    {
        m_textureManager.enableTextureMagFiltering(texId, filter);
        return writeToTextureConfig(texId, m_textureManager.getTmuConfig(texId));  
    }

    virtual bool setRenderResolution(const uint16_t x, const uint16_t y) override
    {
        const uint32_t framebufferSize = x * y * 2;
        const uint32_t framebufferLines = (framebufferSize / RenderConfig::INTERNAL_FRAMEBUFFER_SIZE) + ((framebufferSize % RenderConfig::INTERNAL_FRAMEBUFFER_SIZE) ? 1 : 0);
        if (framebufferLines > DISPLAY_LINES)
        {
            // More lines required than lines available
            return false;
        }

        m_yLineResolution = y / framebufferLines;
        m_xResolution = x;
        m_displayLines = framebufferLines;
        
        RenderResolutionReg reg;
        reg.setX(x);
        reg.setY(m_yLineResolution);
        return writeReg(reg);
    }

    virtual uint16_t getMaxTextureSize() const override
    {
        return RenderConfig::MAX_TEXTURE_SIZE;
    }
    
    virtual TMU getTmuCount() const override
    {
        return RenderConfig::TMU_COUNT;
    }

    virtual bool setStencilBufferConfig(const StencilReg& stencilConf) override
    {
        return writeReg(stencilConf);
    }
private:
    static constexpr std::size_t TEXTURE_NUMBER_OF_TEXTURES { RenderConfig::NUMBER_OF_TEXTURE_PAGES }; // Have as many pages as textures can exist. Probably the most reasonable value for the number of pages.

    using ListAssembler = DisplayListAssembler<RenderConfig>;
    using TextureManager = TextureMemoryManager<RenderConfig>; 

    template <typename TArg>
    bool writeReg(const TArg& regVal)
    {
        return m_displayListAssembler[m_backList].addCommand(WriteRegisterCmd { regVal });
    }

    bool writeToTextureConfig(const uint16_t texId, TmuTextureReg tmuConfig)
    {
        // Find the TMU by searching through the bound textures, if the current texture ID is currently used.
        // If not, just ignore it because then the currently used texture must not be changed.
        for (uint8_t tmu = 0; tmu < IRenderer::MAX_TMU_COUNT; tmu++)
        {
            if (m_boundTextures[tmu] == texId)
            {
                tmuConfig.setTmu(tmu);
                return writeReg(tmuConfig);  
            }
        }
        return true;
    }

    bool setColorBufferAddress(const uint32_t addr)
    {
        m_colorBufferAddr = addr + RenderConfig::GRAM_MEMORY_LOC;
        return writeReg(ColorBufferAddrReg { addr + RenderConfig::GRAM_MEMORY_LOC });
    }

    bool setDepthBufferAddress(const uint32_t addr)
    {
        return writeReg(DepthBufferAddrReg { addr + RenderConfig::GRAM_MEMORY_LOC });
    }

    bool setStencilBufferAddress(const uint32_t addr)
    {
        return writeReg(StencilBufferAddrReg { addr + RenderConfig::GRAM_MEMORY_LOC });
    }

    void uploadTextures()
    {
        // Upload textures
        m_textureManager.uploadTextures([&](uint32_t gramAddr, const std::span<const uint8_t> data)
        {
            DisplayListAssembler<RenderConfig> uploader;
            uploader.setBuffer(m_busConnector.requestBuffer(m_busConnector.getBufferCount() - 1));
            uploader.uploadToDeviceMemory(gramAddr, data);

            while (!m_busConnector.clearToSend())
                ;
            m_busConnector.writeData(m_busConnector.getBufferCount() - 1, uploader.getDisplayList()->getSize());
            return true;
        });
    }

    FramebufferCmd createCommitFramebufferCommand(const uint32_t i)
    {
        FramebufferCmd cmd { false, false, false };
        const uint32_t screenSize = static_cast<uint32_t>(m_yLineResolution) * m_xResolution * 2;
        if constexpr ((RenderConfig::FRAMEBUFFER_TYPE == FramebufferType::INTERNAL_TO_STREAM)
            || (RenderConfig::FRAMEBUFFER_TYPE == FramebufferType::INTERNAL_TO_MEMORY))
        {
            cmd.selectColorBuffer();
            cmd.selectDepthBuffer();
            cmd.selectStencilBuffer();
            cmd.commitFramebuffer(screenSize, m_colorBufferAddr + (screenSize * (m_displayLines - i - 1)), !m_colorBufferUseMemory);
        }
        if constexpr (RenderConfig::FRAMEBUFFER_TYPE == FramebufferType::EXTERNAL_MEMORY_TO_STREAM)
        {
            cmd.streamFromFramebuffer(screenSize, m_colorBufferAddr + (screenSize * (m_displayLines - i - 1)));
        }
        return cmd;
    }

    FramebufferCmd createSwapFramebufferCommand()
    {
        FramebufferCmd cmd { false, false, false };
        cmd.selectColorBuffer();
        cmd.swapFramebuffer();
        return cmd;
    }

    void swapFramebuffer()
    {
        if constexpr (RenderConfig::FRAMEBUFFER_TYPE == FramebufferType::EXTERNAL_MEMORY_DOUBLE_BUFFER)
        {
            if (m_backList == 0)
            {
                setColorBufferAddress(RenderConfig::COLOR_BUFFER_LOC_1);
            }
            else
            {
                setColorBufferAddress(RenderConfig::COLOR_BUFFER_LOC_2);
            }
        }
    }

    void uploadDisplayList()
    {
        for (int32_t i = m_displayLines - 1; i >= 0; i--)
        {
            while (!m_busConnector.clearToSend())
                ;
            m_displayListAssembler[m_frontList].setCheckpoint();
            m_displayListAssembler[m_frontList].addCommand(createCommitFramebufferCommand(i));
            // Set YOffset for the next stream
            YOffsetReg reg;
            // Check wrap around
            if ((i - 1) < 0)
            {
                reg.setY((m_displayLines - 1) * m_yLineResolution);
            }
            else
            {
                reg.setY((i - 1) * m_yLineResolution);
            }
            m_displayListAssembler[m_frontList].addCommand(WriteRegisterCmd { reg });
            m_displayListAssembler[m_frontList].finish();
            const typename ListAssembler::List *list = m_displayListAssembler[m_frontList].getDisplayList();
            m_busConnector.writeData(m_frontList, list->getSize());
            m_displayListAssembler[m_frontList].resetToCheckpoint();
        }
    }

    void enableColorBufferInMemory()
    {
        m_colorBufferUseMemory = true;
    }

    void enableColorBufferStream()
    {
        m_colorBufferUseMemory = false;
    }

    bool m_colorBufferUseMemory { true };
    uint32_t m_colorBufferAddr {};

    std::array<ListAssembler, 2> m_displayListAssembler;
    uint8_t m_frontList = 0;
    uint8_t m_backList = 1;

    uint16_t m_yLineResolution { 128 };
    uint16_t m_xResolution { 640 };
    uint16_t m_displayLines { DISPLAY_LINES };

    IBusConnector& m_busConnector;
    TextureManager m_textureManager;
    Rasterizer m_rasterizer;

    // Mapping of texture id and TMU
    std::array<uint16_t, MAX_TMU_COUNT> m_boundTextures {};
};

} // namespace rr
#endif // RENDERER_MEMORY_OPTIMIZED_HPP

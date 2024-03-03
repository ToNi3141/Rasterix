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

#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <stdint.h>
#include <array>
#include <optional>
#include "Vec.hpp"
#include "IRenderer.hpp"
#include "IBusConnector.hpp"
#include "DisplayList.hpp"
#include "Rasterizer.hpp"
#include <string.h>
#include "DisplayListAssembler.hpp"
#include <future>
#include <algorithm>
#include "TextureMemoryManager.hpp"
#include <limits>

#include "registers/BaseColorReg.hpp"
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
// This renderer collects all triangles in a single display list. It will create for each display line a unique display list where
// all triangles and operations are stored, which belonging to this display line. This is probably the fastest method to do this
// but requires much more memory because of lots of duplicated data.
// The RenderConfig::CMD_STREAM_WIDTH is used to calculate the alignment in the display list.
template <class RenderConfig>
class Renderer : public IRenderer
{
    static constexpr uint16_t DISPLAY_LINES { ((RenderConfig::MAX_DISPLAY_WIDTH * RenderConfig::MAX_DISPLAY_HEIGHT) / RenderConfig::FRAMEBUFFER_SIZE_IN_WORDS) + 1 };
public:
    Renderer(IBusConnector& busConnector)
        : m_busConnector(busConnector)
    {
        for (auto& entry : m_displayListAssembler)
        {
            entry.clearAssembler();
        }

        // Fixes the first two frames
        for (uint32_t i = 0; i < m_displayLines; i++)
        {
            auto buffer1 = m_busConnector.requestBuffer(i + (DISPLAY_LINES * m_backList));
            auto buffer2 = m_busConnector.requestBuffer(i + (DISPLAY_LINES * m_frontList));
            m_displayListAssembler[i + (DISPLAY_LINES * m_backList)].setBuffer(buffer1);
            m_displayListAssembler[i + (DISPLAY_LINES * m_frontList)].setBuffer(buffer2);

            YOffsetReg reg;
            reg.setY(i * m_yLineResolution);
            m_displayListAssembler[i + (DISPLAY_LINES * m_backList)].addCommand(WriteRegisterCmd { reg });
            m_displayListAssembler[i + (DISPLAY_LINES * m_frontList)].addCommand(WriteRegisterCmd { reg });
        }

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

        // Initialize the render thread by running it once
        m_renderThread = std::async([&](){
            return true;
        });
    }

    virtual ~Renderer()
    {
        setColorBufferAddress(RenderConfig::COLOR_BUFFER_LOC_1);
        render();
        if (m_renderThread.valid() && (m_renderThread.get() != true))
        {
            // TODO: In the unexpected case, that the render thread fails, this should handle this error somehow
            return;
        }
    }

    virtual bool drawTriangle(const Triangle& triangle) override
    {
        TriangleStreamCmd<IRenderer::MAX_TMU_COUNT, RenderConfig::USE_FLOAT_INTERPOLATION> triangleCmd { m_rasterizer, triangle };

        if (!triangleCmd.isVisible()) [[unlikely]]
        {
            // Triangle is not visible
            return true;
        }

                const uint32_t displayLines = m_displayLines;
        const uint32_t yLineResolution = m_yLineResolution;
        for (uint32_t i = 0; i < displayLines; i++)
        {
            const uint16_t currentScreenPositionStart = i * yLineResolution;
            const uint16_t currentScreenPositionEnd = (i + 1) * yLineResolution;
            if (triangleCmd.isInBounds(currentScreenPositionStart, currentScreenPositionEnd))
            {
                bool ret { false };
                
                // The floating point rasterizer can automatically increment all attributes to the current screen position
                // Therefor no further computing is necessary
                if constexpr (RenderConfig::USE_FLOAT_INTERPOLATION)
                {
                    ret = m_displayListAssembler[i + (DISPLAY_LINES * m_backList)].addCommand(triangleCmd);
                }
                else
                {
                    // The fix point interpolator needs the triangle incremented to the current line
                    TriangleStreamCmd<IRenderer::MAX_TMU_COUNT, RenderConfig::USE_FLOAT_INTERPOLATION> triangleCmdInc = triangleCmd;
                    triangleCmdInc.increment(currentScreenPositionStart, currentScreenPositionEnd);
                    ret = m_displayListAssembler[i + (DISPLAY_LINES * m_backList)].addCommand(triangleCmdInc);
                }
                if (ret == false) [[unlikely]]
                {
                    return false;
                }
            }
        }
        return true;
    }

    virtual void render() override
    {
        // Check if the previous rendering has finished. If not, block till it is finished.
        if (m_renderThread.valid() && (m_renderThread.get() != true)) [[unlikely]]
        {
            // TODO: In the unexpected case, that the render thread fails, this should handle this error somehow
            return;
        }

        // Commit frame 
        for (uint32_t i = 0; i < m_displayLines; i++)
        {
            if (auto cmd = createCommitFramebufferCommand(i); cmd)
            {
                m_displayListAssembler[i + (DISPLAY_LINES * m_backList)].addCommand(*cmd);
            }
        }

        // Swap frame
        // Display list zero is always the last list, and this list is responsible to set the overall SoC state, like
        // the address for the display output
        m_displayListAssembler[(DISPLAY_LINES * m_backList)].addCommand(createSwapFramebufferCommand()); 

        // Finish display list to prepare it for upload
        for (uint32_t i = 0; i < m_displayLines; i++)
        {
            m_displayListAssembler[i + (DISPLAY_LINES * m_backList)].finish();
        }

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

        uploadTextures();
        uploadDisplayList();
        for (int32_t i = m_displayLines - 1; i >= 0; i--)
        {
            clearAndInitDisplayList(i);
        }
        swapFramebuffer();
    }


    virtual bool clear(const bool colorBuffer, const bool depthBuffer, const bool stencilBuffer) override
    {
        FramebufferCmd cmd { colorBuffer, depthBuffer, stencilBuffer };
        cmd.enableMemset();
        bool ret = true;
        for (uint32_t i = 0; i < m_displayLines; i++)
        {
            const uint16_t currentScreenPositionStart = i * m_yLineResolution;
            const uint16_t currentScreenPositionEnd = (i + 1) * m_yLineResolution;
            if (m_scissorEnabled) 
            {
                if ((currentScreenPositionEnd >= m_scissorYStart) && (currentScreenPositionStart < m_scissorYEnd))
                {
                    ret = ret && m_displayListAssembler[i + (DISPLAY_LINES * m_backList)].addCommand(cmd);
                }
            }
            else
            {
                ret = ret && m_displayListAssembler[i + (DISPLAY_LINES * m_backList)].addCommand(cmd);
            }
        }
        return ret;
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

        // Upload data to the display lists
        for (uint32_t i = 0; i < m_displayLines; i++)
        {
            ret = ret && m_displayListAssembler[i + (DISPLAY_LINES * m_backList)].addCommand(fogLutDesc);
        }
        return ret;
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

    virtual bool isTextureValid(const uint16_t texId) const override
    {
        return m_textureManager.textureValid(texId);
    }

    virtual bool createTextureWithName(const uint16_t texId) override 
    {
        return m_textureManager.createTextureWithName(texId);
    }

    virtual bool useTexture(const TMU target, const uint16_t texId) override 
    {
        m_boundTextures[target] = texId;
        if (!m_textureManager.textureValid(texId)) [[unlikely]]
        {
            return false;
        }
        bool ret { true };
        const std::span<const uint16_t> pages = m_textureManager.getPages(texId);
        for (uint32_t i = 0; i < m_displayLines; i++)
        {
            using Command = TextureStreamCmd<RenderConfig>;
            ret = ret && m_displayListAssembler[i + (DISPLAY_LINES * m_backList)].addCommand(Command { target, pages });
            TmuTextureReg reg = m_textureManager.getTmuConfig(texId);
            reg.setTmu(target);
            ret = ret && m_displayListAssembler[i + (DISPLAY_LINES * m_backList)].addCommand(WriteRegisterCmd { reg });
        }
        return ret;
    }

    virtual bool deleteTexture(const uint16_t texId) override 
    {
        return m_textureManager.deleteTexture(texId);
    }

    virtual bool setFeatureEnableConfig(const FeatureEnableReg& featureEnable) override
    {
        m_scissorEnabled = featureEnable.getEnableScissor();
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

        m_scissorYStart = y;
        m_scissorYEnd = y + height;

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

    virtual bool enableTextureMinFiltering(const uint16_t texId, bool filter) override
    {
        m_textureManager.enableTextureMinFiltering(texId, filter);
        return writeToTextureConfig(texId, m_textureManager.getTmuConfig(texId));  
    }

    virtual bool setRenderResolution(const uint16_t x, const uint16_t y) override
    {
        const uint32_t framebufferSize = x * y;
        const uint32_t framebufferLines = (framebufferSize / RenderConfig::FRAMEBUFFER_SIZE_IN_WORDS) + ((framebufferSize % RenderConfig::FRAMEBUFFER_SIZE_IN_WORDS) ? 1 : 0);
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

    virtual bool isMipmappingAvailable() const override
    {
        return RenderConfig::ENABLE_MIPMAPPING;
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
        bool ret = true;
        for (uint32_t i = 0; i < m_displayLines; i++)
        {
            ret = ret && m_displayListAssembler[i + (DISPLAY_LINES * m_backList)].addCommand(WriteRegisterCmd { regVal });
        }
        return ret;
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

    std::optional<FramebufferCmd> createCommitFramebufferCommand(const uint32_t i)
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

    void clearAndInitDisplayList(const uint32_t i)
    {
        m_displayListAssembler[i + (DISPLAY_LINES * m_backList)].clearAssembler();
        YOffsetReg reg;
        reg.setY(i * m_yLineResolution);
        m_displayListAssembler[i + (DISPLAY_LINES * m_backList)].addCommand(WriteRegisterCmd { reg });
    }

    void uploadDisplayList()
    {
        m_renderThread = std::async([&](){
            for (int32_t i = m_displayLines - 1; i >= 0; i--)
            {
                while (!m_busConnector.clearToSend())
                    ;
                const typename ListAssembler::List *list = m_displayListAssembler[i + (DISPLAY_LINES * m_frontList)].getDisplayList();
                m_busConnector.writeData(i + (DISPLAY_LINES * m_frontList), list->getSize());
            }
            return true;
        });
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

    std::array<ListAssembler, DISPLAY_LINES * 2> m_displayListAssembler;
    uint8_t m_frontList = 0;
    uint8_t m_backList = 1;

    // Optimization for the scissor test to filter unecessary clean calls
    bool m_scissorEnabled { false };
    int16_t m_scissorYStart { 0 };
    int16_t m_scissorYEnd { 0 };

    uint16_t m_yLineResolution { 128 };
    uint16_t m_xResolution { 640 };
    uint16_t m_displayLines { DISPLAY_LINES };

    IBusConnector& m_busConnector;
    TextureManager m_textureManager;
    Rasterizer m_rasterizer { !RenderConfig::USE_FLOAT_INTERPOLATION };
    std::future<bool> m_renderThread;

    // Mapping of texture id and TMU
    std::array<uint16_t, MAX_TMU_COUNT> m_boundTextures {};
};

} // namespace rr
#endif // RENDERER_HPP

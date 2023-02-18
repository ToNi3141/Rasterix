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
#include "Vec.hpp"
#include "IRenderer.hpp"
#include "registers/BaseColorReg.hpp"
#include "IBusConnector.hpp"
#include "DisplayList.hpp"
#include "Rasterizer.hpp"
#include <string.h>
#include "DisplayListAssembler.hpp"
#include <future>
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
#include "commands/TriangleStreamCmd.hpp"
#include "commands/FogLutStreamCmd.hpp"
#include "commands/FramebufferCmd.hpp"
#include "commands/TextureStreamCmd.hpp"
#include "commands/WriteRegisterCmd.hpp"

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
    static constexpr uint16_t DISPLAY_LINES { ((RenderConfig::MAX_DISPLAY_WIDTH * RenderConfig::MAX_DISPLAY_HEIGHT * 2) / RenderConfig::INTERNAL_FRAMEBUFFER_SIZE) + 1 };
public:
    Renderer(IBusConnector& busConnector)
        : m_busConnector(busConnector)
    {
        for (auto& entry : m_displayListAssembler)
        {
            entry.clearAssembler();
        }

        setRenderResolution(640, 480);

        // Fixes the first two frames
        for (uint32_t i = 0; i < m_displayLines; i++)
        {
            YOffsetReg reg;
            reg.setY(i * m_yLineResolution);
            m_displayListAssembler[i + (DISPLAY_LINES * m_backList)].addCommand(WriteRegisterCmd { reg });
            m_displayListAssembler[i + (DISPLAY_LINES * m_frontList)].addCommand(WriteRegisterCmd { reg });
        }

        setTexEnvColor(0, { { 0, 0, 0, 0 } });
        setClearColor({{0, 0, 0, 0}});
        setClearDepth(65535);
        setFogColor({ { 255, 255, 255, 255 } });
        std::array<float, 33> fogLut{};
        std::fill(fogLut.begin(), fogLut.end(), 1.0f);
        setFogLut(fogLut, 0.0f, (std::numeric_limits<float>::max)()); // Windows defines macros with max ... parenthesis are a work around against build errors.

        // Initialize the render thread by running it once
        m_renderThread = std::async([&](){
            return true;
        });
    }

    virtual bool drawTriangle(const Triangle& triangle) override
    {
        TriangleStreamCmd<IRenderer::MAX_TMU_COUNT> triangleCmd { m_rasterizer, triangle };

        if (!triangleCmd.isVisible())
        {
            // Triangle is not visible
            return true;
        }

        for (uint32_t i = 0; i < m_displayLines; i++)
        {
            const uint16_t currentScreenPositionStart = i * m_yLineResolution;
            const uint16_t currentScreenPositionEnd = (i + 1) * m_yLineResolution;
            if (triangleCmd.isInBounds(currentScreenPositionStart, currentScreenPositionEnd))
            {
                bool ret = m_displayListAssembler[i + (DISPLAY_LINES * m_backList)].addCommand(triangleCmd);
                if (ret == false)
                {
                    return false;
                }
            }
        }
        return true;
    }

    bool blockTillRenderingFinished()
    {
        return m_renderThread.valid() && (m_renderThread.get() != true);
    }

    virtual void commit() override
    {
        // Check if the previous rendering has finished. If not, block till it is finished.
        if (blockTillRenderingFinished())
        {
            // TODO: In the unexpected case, that the render thread fails, this should handle this error somehow
            return;
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

        // Prepare all display lists
        bool ret = true;
        for (uint32_t i = 0; i < m_displayLines; i++)
        {
            FramebufferCmd cmd { true, true };
            const uint32_t screenSize = static_cast<uint32_t>(m_yLineResolution) * m_xResolution * 2;
            cmd.enableCommit(screenSize, m_colorBufferAddr + (screenSize * (m_displayLines - i - 1)), !m_colorBufferUseMemory);
            ret = ret && m_displayListAssembler[i + (DISPLAY_LINES * m_frontList)].addCommand(cmd);
        }

        // Upload textures
        m_textureManager.uploadTextures([&](uint32_t gramAddr, const tcb::span<const uint8_t> data)
        {
            static constexpr uint32_t TEX_UPLOAD_SIZE { TextureManager::TEXTURE_PAGE_SIZE + ListAssembler::uploadCommandSize() };
            DisplayListAssembler<RenderConfig, TEX_UPLOAD_SIZE> uploader;
            uploader.uploadToDeviceMemory(gramAddr, data);

            while (!m_busConnector.clearToSend())
                ;
            m_busConnector.writeData(uploader.getDisplayList()->getMemPtr());
            return true;
        });

        // Render image (in new thread)
        if (ret)
        {
            m_renderThread = std::async([&](){
                for (int32_t i = m_displayLines - 1; i >= 0; i--)
                {
                    while (!m_busConnector.clearToSend())
                        ;
                    const typename ListAssembler::List *list = m_displayListAssembler[i + (DISPLAY_LINES * m_frontList)].getDisplayList();
                    m_busConnector.writeData(list->getMemPtr());
                    m_displayListAssembler[i + (DISPLAY_LINES * m_frontList)].clearAssembler();
                    YOffsetReg reg;
                    reg.setY(i * m_yLineResolution);
                    m_displayListAssembler[i + (DISPLAY_LINES * m_frontList)].addCommand(WriteRegisterCmd { reg });
                }
                return true;
            });
        }
    }

    virtual bool clear(bool colorBuffer, bool depthBuffer) override
    {
        FramebufferCmd cmd { colorBuffer, depthBuffer };
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

    virtual bool updateTexture(const uint16_t texId, const TextureObject& textureObject) override
    {
        return m_textureManager.updateTexture(texId, textureObject);
    }

    virtual TextureObject getTexture(const uint16_t texId) override
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
        const tcb::span<const uint16_t> pages = m_textureManager.getPages(texId);
        const uint32_t texSize = m_textureManager.getTextureDataSize(texId);
        for (uint32_t i = 0; i < m_displayLines; i++)
        {
            using Command = TextureStreamCmd<RenderConfig>;
            ret = ret && m_displayListAssembler[i + (DISPLAY_LINES * m_backList)].addCommand(Command { target, pages, texSize });
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

    virtual void enableColorBufferInMemory(const uint32_t addr) override
    {
        m_colorBufferAddr = addr;
        m_colorBufferUseMemory = true;
    }

    virtual void enableColorBufferStream() override
    {
        m_colorBufferUseMemory = false;
    }

    virtual uint16_t getMaxTextureSize() const override
    {
        return RenderConfig::MAX_TEXTURE_SIZE;
    }
    
    virtual TMU getTmuCount() const override
    {
        return RenderConfig::TMU_COUNT;
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

    bool m_colorBufferUseMemory { true };
    uint32_t m_colorBufferAddr { 0x02000000 };

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
    Rasterizer m_rasterizer;
    std::future<bool> m_renderThread;

    // Mapping of texture id and TMU
    std::array<uint16_t, MAX_TMU_COUNT> m_boundTextures {};
};

} // namespace rr
#endif // RENDERER_HPP

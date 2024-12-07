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
#include "Renderer.hpp"

namespace rr
{
Renderer::Renderer(IBusConnector& busConnector)
    : m_busConnector(busConnector)
{
    for (auto& entry : m_displayListAssembler)
    {
        entry.clearAssembler();
    }

    // Fixes the first two frames
    for (std::size_t i = 0; i < m_displayLines; i++)
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
}

Renderer::~Renderer()
{
    setColorBufferAddress(RenderConfig::COLOR_BUFFER_LOC_1);
    swapDisplayList();
    uploadDisplayList();
}

bool Renderer::drawTriangle(const TransformedTriangle& triangle)
{
    TriangleStreamCmd<typename ListAssembler::List> triangleCmd { m_rasterizer, triangle };

    if (!triangleCmd.isVisible()) 
    {
        // Triangle is not visible
        return true;
    }

    if constexpr (DISPLAY_LINES == 1)
    {
        return addCommand(0, triangleCmd);
    }
    else
    {
        const std::size_t displayLines = m_displayLines;
        const std::size_t yLineResolution = m_yLineResolution;
        for (std::size_t i = 0; i < displayLines; i++)
        {
            const std::size_t currentScreenPositionStart = i * yLineResolution;
            const std::size_t currentScreenPositionEnd = (i + 1) * yLineResolution;
            if (triangleCmd.isInBounds(currentScreenPositionStart, currentScreenPositionEnd))
            {
                bool ret { false };
                
                // The floating point rasterizer can automatically increment all attributes to the current screen position
                // Therefor no further computing is necessary
                if constexpr (RenderConfig::USE_FLOAT_INTERPOLATION)
                {
                    ret = addCommand(i, triangleCmd);
                }
                else
                {
                    // The fix point interpolator needs the triangle incremented to the current line (when DISPLAY_LINES is greater 1)
                    TriangleStreamCmd<typename ListAssembler::List> triangleCmdInc = triangleCmd;
                    triangleCmdInc.increment(currentScreenPositionStart, currentScreenPositionEnd);
                    ret = addCommand(i, triangleCmdInc);
                }
                if (ret == false) 
                {
                    return false;
                }
            }
        }
    }
    return true;
}

void Renderer::swapDisplayList()
{
    // Commit frame 
    for (std::size_t i = 0; i < m_displayLines; i++)
    {
        if (auto cmd = createCommitFramebufferCommand(i); cmd)
        {
            addCommand(i, *cmd);
        }
    }

    // Swap frame
    // Display list zero is always the last list, and this list is responsible to set the overall SoC state, like
    // the address for the display output
    addCommand(0, createSwapFramebufferCommand());

    // Finish display list to prepare it for upload
    for (std::size_t i = 0; i < m_displayLines; i++)
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
    for (int32_t i = m_displayLines - 1; i >= 0; i--)
    {
        clearAndInitDisplayList(i);
    }
    swapFramebuffer();
}

void Renderer::uploadDisplayList()
{
    for (int32_t i = m_displayLines - 1; i >= 0; i--)
    {
        while (!m_busConnector.clearToSend())
            ;
        const typename ListAssembler::List *list = m_displayListAssembler[i + (DISPLAY_LINES * m_frontList)].getDisplayList();
        m_busConnector.writeData(i + (DISPLAY_LINES * m_frontList), list->getSize());
    }
}

bool Renderer::clear(const bool colorBuffer, const bool depthBuffer, const bool stencilBuffer)
{
    FramebufferCmd cmd { colorBuffer, depthBuffer, stencilBuffer };
    cmd.enableMemset();
    bool ret = true;
    for (std::size_t i = 0; i < m_displayLines; i++)
    {
        const std::size_t currentScreenPositionStart = i * m_yLineResolution;
        const std::size_t currentScreenPositionEnd = (i + 1) * m_yLineResolution;
        if (m_scissorEnabled) 
        {
            if ((currentScreenPositionEnd >= m_scissorYStart) && (currentScreenPositionStart < m_scissorYEnd))
            {
                ret = ret && addCommand(i, cmd);
            }
        }
        else
        {
            ret = ret && addCommand(i, cmd);
        }
    }
    return ret;
}

bool Renderer::setClearColor(const Vec4i& color)
{
    ColorBufferClearColorReg reg;
    reg.setColor(color);
    return writeReg(reg);
}

bool Renderer::setClearDepth(uint16_t depth)
{
    DepthBufferClearDepthReg reg;
    reg.setValue(depth);
    return writeReg(reg);
}

bool Renderer::setTexEnvColor(const std::size_t target, const Vec4i& color)
{
    TexEnvColorReg reg;
    reg.setTmu(target);
    reg.setColor(color);
    return writeReg(reg);
}

bool Renderer::setFogColor(const Vec4i& color)
{
    FogColorReg reg;
    reg.setColor(color);
    return writeReg(reg);
}

bool Renderer::setFogLut(const std::array<float, 33>& fogLut, float start, float end)
{
    bool ret = true;
    const FogLutStreamCmd fogLutDesc { fogLut, start, end };

    // Upload data to the display lists
    for (std::size_t i = 0; i < m_displayLines; i++)
    {
        ret = ret && addCommand(i, fogLutDesc);
    }
    return ret;
}

bool Renderer::useTexture(const std::size_t target, const uint16_t texId) 
{
    m_boundTextures[target] = texId;
    if (!m_textureManager.textureValid(texId)) 
    {
        return false;
    }
    bool ret { true };
    const tcb::span<const uint16_t> pages = m_textureManager.getPages(texId);
    for (std::size_t i = 0; i < m_displayLines; i++)
    {
        using Command = TextureStreamCmd<RenderConfig>;
        ret = ret && addCommand(i, Command { target, pages });
        TmuTextureReg reg = m_textureManager.getTmuConfig(texId);
        reg.setTmu(target);
        ret = ret && addCommand(i, WriteRegisterCmd { reg });
    }
    return ret;
}

bool Renderer::setFeatureEnableConfig(const FeatureEnableReg& featureEnable)
{
    m_scissorEnabled = featureEnable.getEnableScissor();
    m_rasterizer.enableScissor(featureEnable.getEnableScissor());
    m_rasterizer.enableTmu(0, featureEnable.getEnableTmu(0));
    if constexpr (RenderConfig::TMU_COUNT == 2)
        m_rasterizer.enableTmu(1, featureEnable.getEnableTmu(1));
    static_assert(RenderConfig::TMU_COUNT <= 2, "Adapt following code when the TMU count has changed");
    return writeReg(featureEnable);
}

bool Renderer::setScissorBox(const int32_t x, const int32_t y, const uint32_t width, const uint32_t height)
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

bool Renderer::setTextureWrapModeS(const uint16_t texId, TextureWrapMode mode)
{
    m_textureManager.setTextureWrapModeS(texId, mode);
    return writeToTextureConfig(texId, m_textureManager.getTmuConfig(texId));
}

bool Renderer::setTextureWrapModeT(const uint16_t texId, TextureWrapMode mode)
{
    m_textureManager.setTextureWrapModeT(texId, mode);
    return writeToTextureConfig(texId, m_textureManager.getTmuConfig(texId)); 
}

bool Renderer::enableTextureMagFiltering(const uint16_t texId, bool filter)
{
    m_textureManager.enableTextureMagFiltering(texId, filter);
    return writeToTextureConfig(texId, m_textureManager.getTmuConfig(texId));  
}

bool Renderer::enableTextureMinFiltering(const uint16_t texId, bool filter)
{
    m_textureManager.enableTextureMinFiltering(texId, filter);
    return writeToTextureConfig(texId, m_textureManager.getTmuConfig(texId));  
}

bool Renderer::setRenderResolution(const std::size_t x, const std::size_t y)
{
    const std::size_t framebufferSize = x * y;
    const std::size_t framebufferLines = (framebufferSize / RenderConfig::FRAMEBUFFER_SIZE_IN_WORDS) + ((framebufferSize % RenderConfig::FRAMEBUFFER_SIZE_IN_WORDS) ? 1 : 0);
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

bool Renderer::writeToTextureConfig(const uint16_t texId, TmuTextureReg tmuConfig)
{
    // Find the TMU by searching through the bound textures, if the current texture ID is currently used.
    // If not, just ignore it because then the currently used texture must not be changed.
    for (std::size_t tmu = 0; tmu < m_boundTextures.size(); tmu++)
    {
        if (m_boundTextures[tmu] == texId)
        {
            tmuConfig.setTmu(tmu);
            return writeReg(tmuConfig);  
        }
    }
    return true;
}

bool Renderer::setColorBufferAddress(const uint32_t addr)
{
    m_colorBufferAddr = addr + RenderConfig::GRAM_MEMORY_LOC;
    return writeReg(ColorBufferAddrReg { addr + RenderConfig::GRAM_MEMORY_LOC });
}

void Renderer::uploadTextures()
{
    m_textureManager.uploadTextures([&](uint32_t gramAddr, const tcb::span<const uint8_t> data)
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

std::optional<FramebufferCmd> Renderer::createCommitFramebufferCommand(const uint32_t i)
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

FramebufferCmd Renderer::createSwapFramebufferCommand()
{
    FramebufferCmd cmd { false, false, false };
    cmd.selectColorBuffer();
    cmd.swapFramebuffer();
    return cmd;
}

void Renderer::clearAndInitDisplayList(const uint32_t i)
{
    m_displayListAssembler[i + (DISPLAY_LINES * m_backList)].clearAssembler();
    YOffsetReg reg;
    reg.setY(i * m_yLineResolution);
    m_displayListAssembler[i + (DISPLAY_LINES * m_backList)].addCommand(WriteRegisterCmd { reg });
}

void Renderer::swapFramebuffer()
{
    if constexpr (RenderConfig::FRAMEBUFFER_TYPE == FramebufferType::EXTERNAL_MEMORY_DOUBLE_BUFFER)
    {
        if (m_switchColorBuffer)
        {
            setColorBufferAddress(RenderConfig::COLOR_BUFFER_LOC_2);
        }
        else
        {
            setColorBufferAddress(RenderConfig::COLOR_BUFFER_LOC_1);
        }
        m_switchColorBuffer = !m_switchColorBuffer;
    }
}

void Renderer::intermediateUpload()
{
    // Finish display list to prepare it for upload
    m_displayListAssembler[DISPLAY_LINES * m_backList].finish();

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
    clearAndInitDisplayList(0);
    uploadDisplayList();
}

} // namespace rr


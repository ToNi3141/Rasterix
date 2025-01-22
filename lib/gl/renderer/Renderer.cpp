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
    m_displayListBuffer.getBack().clearDisplayListAssembler();
    m_displayListBuffer.getFront().clearDisplayListAssembler();

    auto dl = m_displayListDispatcher[0].getAllDisplayLists();
    for (std::size_t i = 0; i < dl.size(); i++)
    {
        dl[i].setBuffer(m_busConnector.requestBuffer(i), i);
    }
    dl = m_displayListDispatcher[1].getAllDisplayLists();
    for (std::size_t i = 0; i < dl.size(); i++)
    {
        dl[i].setBuffer(m_busConnector.requestBuffer(i + dl.size()), i + dl.size());
    }

    beginFrame();
    setYOffset();

    if constexpr (RenderConfig::FRAMEBUFFER_TYPE == FramebufferType::INTERNAL_TO_MEMORY)
    {
        setColorBufferAddress(RenderConfig::COLOR_BUFFER_LOC_2);
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
    endFrame();
    swapDisplayList();
    uploadDisplayList();
}

bool Renderer::drawTriangle(const TransformedTriangle& triangle)
{
    const TriangleStreamCmd<typename ListAssembler::List> triangleCmd { m_rasterizer, triangle };

    if (!triangleCmd.isVisible()) 
    {
        // Triangle is not visible
        return true;
    }

    if constexpr (DisplayListDispatcherType::getMaxDisplayLines() == 1)
    {
        return addCommand(0, triangleCmd);
    }
    else
    {
        m_triangleLooper.setTriangleCmd(&triangleCmd);
        return displayListLooper(m_triangleLooper);
    }
    return true;
}

void Renderer::swapDisplayList()
{
    bool ret = true;
    saveSectionStart();
    ret = ret && addCommitFramebufferCommand();
    endFrame();
    ret = ret && addDseFramebufferCommand();
    if (!ret)
    {
        removeSection();
    }

    addSwapExternalDisplayFramebufferCommand();

    switchDisplayLists();
    uploadTextures();
    clearDisplayListAssembler();
    beginFrame();

    setYOffset();
    swapFramebuffer();
}

void Renderer::setYOffset()
{
    addCommandWithFactory([](std::size_t i, std::size_t, std::size_t, std::size_t resY) {
        const uint16_t yOffset = i * resY;
        return WriteRegisterCmd<YOffsetReg> { YOffsetReg { 0, yOffset } };
    });
}

void Renderer::uploadDisplayList()
{
    tcb::span<ListAssembler> displayLists = m_displayListBuffer.getFront().getDisplayLists();
    for (int32_t i = displayLists.size() - 1; i >= 0; i--)
    {
        while (!m_busConnector.clearToSend())
            ;
        m_busConnector.writeData(displayLists[i].getDisplayListBufferId(), displayLists[i].getDisplayListSize());
    }
}

bool Renderer::clear(const bool colorBuffer, const bool depthBuffer, const bool stencilBuffer)
{
    FramebufferCmd cmd { colorBuffer, depthBuffer, stencilBuffer };
    cmd.enableMemset();
    return addCommandWithFactory_if([&cmd](std::size_t, std::size_t, std::size_t, std::size_t) {
        return cmd;
    },
    [this](std::size_t i, std::size_t, std::size_t, std::size_t resY) {
        
        if (m_scissorEnabled) 
        {
            const std::size_t currentScreenPositionStart = i * resY;
            const std::size_t currentScreenPositionEnd = (i + 1) * resY;
            if ((static_cast<int32_t>(currentScreenPositionEnd) >= m_scissorYStart) 
                && (static_cast<int32_t>(currentScreenPositionStart) < m_scissorYEnd))
            {
                return true;
            }
        }
        else
        {
            return true;
        }
        return false;
    });
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
    return addCommand(FogLutStreamCmd { fogLut, start, end });
}

bool Renderer::useTexture(const std::size_t target, const uint16_t texId) 
{
    m_boundTextures[target] = texId;
    if (!m_textureManager.textureValid(texId)) 
    {
        return false;
    }
    bool ret { true };

    const tcb::span<const std::size_t> pages = m_textureManager.getPages(texId);
    ret = ret && addCommand(TextureStreamCmd { target, pages });

    TmuTextureReg reg = m_textureManager.getTmuConfig(texId);
    reg.setTmu(target);
    ret = ret && addCommand(WriteRegisterCmd { reg });

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
    // The resolution must be set on both displaylists
    if (!m_displayListBuffer.getBack().setResolution(x, y)
        || !m_displayListBuffer.getFront().setResolution(x, y))
    {
        return false;
    }

    RenderResolutionReg reg;
    reg.setX(x);
    reg.setY(m_displayListBuffer.getBack().getYLineResolution());
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
    m_colorBufferAddr = addr;
    return writeReg(ColorBufferAddrReg { addr });
}

void Renderer::uploadTextures()
{
    m_textureManager.uploadTextures([&](uint32_t gramAddr, const tcb::span<const uint8_t> data)
    {
        displaylist::DisplayListAssembler<RenderConfig> uploader;
        const std::size_t bufferId = m_busConnector.getBufferCount() - 1;
        uploader.setBuffer(m_busConnector.requestBuffer(bufferId), bufferId);
        uploader.addCommand(WriteMemoryCmd { gramAddr, data });

        while (!m_busConnector.clearToSend())
            ;
        m_busConnector.writeData(bufferId, uploader.getDisplayListSize());
        return true;
    });
}

bool Renderer::addCommitFramebufferCommand()
{
    if constexpr (RenderConfig::FRAMEBUFFER_TYPE == FramebufferType::INTERNAL_TO_MEMORY)
    {
        FramebufferCmd cmd { true, true, true };
        cmd.commitFramebuffer();
        return addCommand(cmd);
    }
    if constexpr (RenderConfig::FRAMEBUFFER_TYPE == FramebufferType::INTERNAL_TO_STREAM)
    {
        FramebufferCmd cmd { true, true, true };
        cmd.commitFramebuffer();
        return addCommand(cmd);
    }
    if constexpr (RenderConfig::FRAMEBUFFER_TYPE == FramebufferType::EXTERNAL_MEMORY_TO_STREAM)
    {
        // The external framebuffer does not require special handling
    }

    return true;
}

bool Renderer::addDseFramebufferCommand()
{
    if constexpr (RenderConfig::FRAMEBUFFER_TYPE == FramebufferType::INTERNAL_TO_MEMORY)
    {
        return addCommandWithFactory([this](std::size_t i, std::size_t lines, std::size_t resX, std::size_t resY) {
            const uint32_t screenSize = static_cast<uint32_t>(resY) * resX * 2;
            const uint32_t addr = m_colorBufferAddr + (screenSize * (lines - i - 1));
            return StreamFromRrxToMemoryCmd { addr, screenSize };
        });
    }
    if constexpr (RenderConfig::FRAMEBUFFER_TYPE == FramebufferType::INTERNAL_TO_STREAM)
    {
        return addCommandWithFactory([](std::size_t, std::size_t, std::size_t resX, std::size_t resY) {
            const uint32_t screenSize = static_cast<uint32_t>(resY) * resX * 2;
            return StreamFromRrxToDisplayCmd { screenSize };
        });
    }
    if constexpr (RenderConfig::FRAMEBUFFER_TYPE == FramebufferType::EXTERNAL_MEMORY_TO_STREAM)
    {
        return addCommandWithFactory([this](std::size_t i, std::size_t lines, std::size_t resX, std::size_t resY) {
            const uint32_t screenSize = static_cast<uint32_t>(resY) * resX * 2;
            const uint32_t addr = m_colorBufferAddr + (screenSize * (lines - i - 1));
            return StreamFromMemoryToDisplayCmd { addr, screenSize };
        });
    }
    return true;
}

bool Renderer::addSwapExternalDisplayFramebufferCommand()
{
    // Display list zero is always the last list, and this list is responsible to set the overall system state, like
    // the address for the display output
    m_displayListBuffer.getBack().saveSectionStart();
    bool ret = true;
    ret = ret && m_displayListBuffer.getBack().beginLastFrame();
    FramebufferCmd cmd { false, false, false };
    cmd.selectColorBuffer();
    cmd.swapFramebuffer();
    ret = ret && addLastCommand(cmd);
    m_displayListBuffer.getBack().endLastFrame();
    if (!ret)
    {
        m_displayListBuffer.getBack().removeSection();
    }
    return ret;
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
    endFrame();
    switchDisplayLists();
    uploadTextures();
    uploadDisplayList();
    clearDisplayListAssembler();
    beginFrame();
}

} // namespace rr


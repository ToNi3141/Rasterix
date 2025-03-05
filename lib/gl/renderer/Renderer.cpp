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
Renderer::Renderer(IDevice& device)
    : m_device { device }
{
    m_displayListBuffer.getBack().clearDisplayListAssembler();
    m_displayListBuffer.getFront().clearDisplayListAssembler();

    initDisplayLists();

    setYOffset();

    setColorBufferAddress(RenderConfig::COLOR_BUFFER_LOC_1);
    setDepthBufferAddress(RenderConfig::DEPTH_BUFFER_LOC);
    setStencilBufferAddress(RenderConfig::STENCIL_BUFFER_LOC);

    setRenderResolution(RenderConfig::MAX_DISPLAY_WIDTH, RenderConfig::MAX_DISPLAY_HEIGHT);
}

Renderer::~Renderer()
{
    setColorBufferAddress(RenderConfig::COLOR_BUFFER_LOC_0);
    swapScreenToNewColorBuffer();
    swapDisplayList();
    uploadDisplayList();
}

bool Renderer::drawTriangle(const TransformedTriangle& triangle)
{
    if constexpr (RenderConfig::THREADED_RASTERIZATION)
    {
        RegularTriangleCmd triangleCmd { triangle };
        return addTriangleCmd(triangleCmd);
    }
    else
    {
        TriangleStreamCmd triangleCmd { m_rasterizer, triangle };
        return addTriangleCmd(triangleCmd);
    }
}

void Renderer::initDisplayLists()
{
    for (std::size_t i = 0, buffId = 0; i < m_displayListAssembler[0].size(); i++)
    {
        m_displayListAssembler[0][i].setBuffer(m_device.requestDisplayListBuffer(buffId), buffId);
        buffId++;
        m_displayListAssembler[1][i].setBuffer(m_device.requestDisplayListBuffer(buffId), buffId);
        buffId++;
    }
}

void Renderer::intermediateUpload()
{
    switchDisplayLists();
    uploadTextures();
    uploadDisplayList();
    clearDisplayListAssembler();
}

void Renderer::swapDisplayList()
{
    addLineColorBufferAddresses();
    addCommitFramebufferCommand();
    addColorBufferAddressOfTheScreen();
    swapScreenToNewColorBuffer();
    switchDisplayLists();
    uploadTextures();
    clearDisplayListAssembler();
    setYOffset();
    swapFramebuffer();
}

void Renderer::addLineColorBufferAddresses()
{
    addCommandWithFactory(
        [this](const std::size_t i, const std::size_t lines, const std::size_t resX, const std::size_t resY)
        {
            const uint32_t screenSize = static_cast<uint32_t>(resY) * resX * 2;
            const uint32_t addr = m_colorBufferAddr + (screenSize * (lines - i - 1));
            return WriteRegisterCmd { ColorBufferAddrReg { addr } };
        });
}

void Renderer::addCommitFramebufferCommand()
{
    addCommandWithFactory(
        [](const std::size_t, const std::size_t, const std::size_t resX, const std::size_t resY)
        {
            // The EF config requires a NopCmd or another command like a commit (which is in this config a Nop)
            // to flush the pipeline. This is the easiest way to solve WAR conflicts.
            // This command is required for the IF config.
            const uint32_t screenSize = resY * resX;
            FramebufferCmd cmd { true, true, true, screenSize };
            cmd.commitFramebuffer();
            return cmd;
        });
}

void Renderer::addColorBufferAddressOfTheScreen()
{
    // The last list is responsible for the overall system state
    addLastCommand(WriteRegisterCmd { ColorBufferAddrReg { m_colorBufferAddr } });
}

void Renderer::swapScreenToNewColorBuffer()
{
    addLastCommandWithFactory(
        [this](const std::size_t, const std::size_t, const std::size_t resX, const std::size_t resY)
        {
            const std::size_t screenSize = resY * resX;
            FramebufferCmd cmd { false, false, false, screenSize };
            cmd.selectColorBuffer();
            cmd.swapFramebuffer();
            return cmd;
        });
}

void Renderer::setYOffset()
{
    addCommandWithFactory(
        [](const std::size_t i, const std::size_t, const std::size_t, const std::size_t resY)
        {
            const uint16_t yOffset = i * resY;
            return WriteRegisterCmd<YOffsetReg> { YOffsetReg { 0, yOffset } };
        });
}

void Renderer::uploadDisplayList()
{
    m_displayListBuffer.getFront().displayListLooper(
        [this](
            DisplayListDispatcherType& dispatcher,
            const std::size_t i,
            const std::size_t displayLines,
            const std::size_t,
            const std::size_t)
        {
            const std::size_t index = (displayLines - 1) - i;
            while (!m_device.clearToSend())
                ;
            m_device.streamDisplayList(dispatcher.getDisplayListBufferId(index), dispatcher.getDisplayListSize(index));
            return true;
        });
}

bool Renderer::clear(const bool colorBuffer, const bool depthBuffer, const bool stencilBuffer)
{
    return addCommandWithFactory_if(
        [&](const std::size_t, const std::size_t, const std::size_t x, const std::size_t y)
        {
            FramebufferCmd cmd { colorBuffer, depthBuffer, stencilBuffer, x * y };
            cmd.enableMemset();
            return cmd;
        },
        [&](const std::size_t i, const std::size_t, const std::size_t x, const std::size_t y)
        {
            FramebufferCmd cmd { colorBuffer, depthBuffer, stencilBuffer, x * y };
            cmd.enableMemset();
            if (m_scissorEnabled)
            {
                const std::size_t currentScreenPositionStart = i * y;
                const std::size_t currentScreenPositionEnd = (i + 1) * y;
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
    m_textureManager.uploadTextures(
        [&](uint32_t gramAddr, const tcb::span<const uint8_t> data)
        {
            while (!m_device.clearToSend())
                ;
            m_device.writeToDeviceMemory(data, gramAddr);
            return true;
        });
}

void Renderer::swapFramebuffer()
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

} // namespace rr

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


#include "PixelPipeline.hpp"

namespace rr
{
PixelPipeline::PixelPipeline(IRenderer& renderer) 
    : m_renderer(renderer)
{
    for (uint8_t i = 0; i < MAX_TMU_COUNT; i++)
    {
        m_tmuConf[i].texEnvConf.setTmu(i);
        m_renderer.setTexEnv(m_tmuConf[i].texEnvConf);
    }
    m_renderer.setFeatureEnableConfig(m_featureEnable);
    m_renderer.setFragmentPipelineConfig(m_fragmentPipelineConf);
    m_renderer.setStencilBufferConfig(m_stencilConf);
}

bool PixelPipeline::drawTriangle(const Triangle& triangle) 
{
        return m_renderer.drawTriangle(triangle);
}

bool PixelPipeline::updatePipeline()
{
    bool ret { true };
    for (uint8_t i = 0; i < MAX_TMU_COUNT; i++)
    {
        if ((m_tmuConf[i].texEnvMode == TexEnvMode::COMBINE) && (m_tmuConf[i].texEnvConfUploaded.serialize() != m_tmuConf[i].texEnvConf.serialize())) [[unlikely]]
        {
            m_tmuConf[i].texEnvConf.setTmu(i);
            ret = ret && m_renderer.setTexEnv(m_tmuConf[i].texEnvConf);
            m_tmuConf[i].texEnvConfUploaded = m_tmuConf[i].texEnvConf;
        }
    }
    
    if (m_featureEnableUploaded.serialize() != m_featureEnable.serialize()) [[unlikely]]
    {
        ret = ret && m_renderer.setFeatureEnableConfig(m_featureEnable);
        m_featureEnableUploaded = m_featureEnable;
    }
    if (m_fragmentPipelineConfUploaded.serialize() != m_fragmentPipelineConf.serialize()) [[unlikely]]
    {
        ret = ret && m_renderer.setFragmentPipelineConfig(m_fragmentPipelineConf);
        m_fragmentPipelineConfUploaded = m_fragmentPipelineConf;
    }
    if (m_enableTwoSideStencil)
    {
        if (m_stencilConfUploaded.serialize() != m_stencilConfTwoSide->serialize()) [[unlikely]]
        {
            ret = ret && m_renderer.setStencilBufferConfig(*m_stencilConfTwoSide);
            m_stencilConfUploaded = *m_stencilConfTwoSide;
        }
    }
    else 
    {
        if (m_stencilConfUploaded.serialize() != m_stencilConf.serialize()) [[unlikely]]
        {
            ret = ret && m_renderer.setStencilBufferConfig(m_stencilConf);
            m_stencilConfUploaded = m_stencilConf;
        }
    }

    ret = ret && m_fog.updateFogLut();
    ret = ret && uploadTexture();

    return ret;
}

bool PixelPipeline::uploadTexture()
{
    bool ret { true };
    if (m_textureObjectMipmap)
    {
        ret = m_renderer.updateTexture(m_tmuConf[m_tmu].boundTexture, *m_textureObjectMipmap);
        m_textureObjectMipmap = std::nullopt;

        // Rebind texture to update the rasterizer with the new texture meta information
        // TODO: Check if this is still required
        ret = ret && useTexture();
    }
    return ret;
}

TextureObjectMipmap& PixelPipeline::getTexture()
{
    if (!m_textureObjectMipmap)
    {
        m_textureObjectMipmap = std::make_optional<TextureObjectMipmap>(m_renderer.getTexture(m_tmuConf[m_tmu].boundTexture));
    }
    return *m_textureObjectMipmap;
}



bool PixelPipeline::useTexture()
{
    uploadTexture();
    return m_renderer.useTexture(m_tmu, m_tmuConf[m_tmu].boundTexture);
}

bool PixelPipeline::setTexEnvMode(const TexEnvMode mode)
{
    m_tmuConf[m_tmu].texEnvMode = mode;
    TexEnvReg texEnvConf {};
    switch (mode) {
    case TexEnvMode::DISABLE:
        texEnvConf.setCombineRgb(TexEnvReg::Combine::REPLACE);
        texEnvConf.setCombineRgb(TexEnvReg::Combine::REPLACE);
        texEnvConf.setCombineAlpha(TexEnvReg::Combine::REPLACE);
        texEnvConf.setSrcRegRgb0(TexEnvReg::SrcReg::PRIMARY_COLOR);
        texEnvConf.setSrcRegAlpha0(TexEnvReg::SrcReg::PRIMARY_COLOR);
        if (m_tmu != 0) // Is this the right mode?
        {
            texEnvConf.setSrcRegRgb0(TexEnvReg::SrcReg::PREVIOUS);
            texEnvConf.setSrcRegAlpha0(TexEnvReg::SrcReg::PREVIOUS);
        }
        break;
    case TexEnvMode::REPLACE:
        texEnvConf.setCombineRgb(TexEnvReg::Combine::REPLACE);
        texEnvConf.setCombineAlpha(TexEnvReg::Combine::REPLACE);
        texEnvConf.setSrcRegRgb0(TexEnvReg::SrcReg::TEXTURE);
        texEnvConf.setSrcRegAlpha0(TexEnvReg::SrcReg::TEXTURE);
        break;
    case TexEnvMode::MODULATE:
        texEnvConf.setCombineRgb(TexEnvReg::Combine::MODULATE);
        texEnvConf.setCombineAlpha(TexEnvReg::Combine::MODULATE);
        texEnvConf.setSrcRegRgb0(TexEnvReg::SrcReg::TEXTURE);
        texEnvConf.setSrcRegRgb1(TexEnvReg::SrcReg::PRIMARY_COLOR);
        texEnvConf.setSrcRegAlpha0(TexEnvReg::SrcReg::TEXTURE);
        texEnvConf.setSrcRegAlpha1(TexEnvReg::SrcReg::PRIMARY_COLOR);
        if (m_tmu != 0) // Is this the right mode?
        {
            texEnvConf.setSrcRegRgb1(TexEnvReg::SrcReg::PREVIOUS);
            texEnvConf.setSrcRegAlpha1(TexEnvReg::SrcReg::PREVIOUS);
        }
        break;
    case TexEnvMode::DECAL:
        texEnvConf.setCombineRgb(TexEnvReg::Combine::INTERPOLATE);
        texEnvConf.setCombineAlpha(TexEnvReg::Combine::REPLACE);
        texEnvConf.setSrcRegRgb0(TexEnvReg::SrcReg::TEXTURE);
        texEnvConf.setSrcRegRgb1(TexEnvReg::SrcReg::PRIMARY_COLOR);
        texEnvConf.setSrcRegRgb2(TexEnvReg::SrcReg::TEXTURE);
        texEnvConf.setSrcRegAlpha0(TexEnvReg::SrcReg::PRIMARY_COLOR);
        texEnvConf.setOperandRgb2(TexEnvReg::Operand::SRC_ALPHA);
        if (m_tmu != 0) // Is this the right mode? 
        {
            texEnvConf.setSrcRegRgb1(TexEnvReg::SrcReg::PREVIOUS);
            texEnvConf.setSrcRegAlpha0(TexEnvReg::SrcReg::PREVIOUS);
        }
        break;
    case TexEnvMode::BLEND:
        texEnvConf.setCombineRgb(TexEnvReg::Combine::INTERPOLATE);
        texEnvConf.setCombineAlpha(TexEnvReg::Combine::MODULATE);
        texEnvConf.setSrcRegRgb0(TexEnvReg::SrcReg::CONSTANT);
        texEnvConf.setSrcRegRgb1(TexEnvReg::SrcReg::PRIMARY_COLOR);
        texEnvConf.setSrcRegRgb2(TexEnvReg::SrcReg::TEXTURE);
        texEnvConf.setSrcRegAlpha0(TexEnvReg::SrcReg::PRIMARY_COLOR);
        texEnvConf.setSrcRegAlpha1(TexEnvReg::SrcReg::TEXTURE);
        if (m_tmu != 0) // Is this the right mode?
        {
            texEnvConf.setSrcRegRgb1(TexEnvReg::SrcReg::PREVIOUS);
            texEnvConf.setSrcRegAlpha0(TexEnvReg::SrcReg::PREVIOUS);
        }
        break;
    case TexEnvMode::ADD:
        texEnvConf.setCombineRgb(TexEnvReg::Combine::ADD);
        texEnvConf.setCombineAlpha(TexEnvReg::Combine::ADD);
        texEnvConf.setSrcRegRgb0(TexEnvReg::SrcReg::TEXTURE);
        texEnvConf.setSrcRegRgb1(TexEnvReg::SrcReg::PRIMARY_COLOR);
        texEnvConf.setSrcRegAlpha0(TexEnvReg::SrcReg::TEXTURE);
        texEnvConf.setSrcRegAlpha1(TexEnvReg::SrcReg::PRIMARY_COLOR);
        if (m_tmu != 0) // Is this the right mode?
        {
            texEnvConf.setSrcRegRgb1(TexEnvReg::SrcReg::PREVIOUS);
            texEnvConf.setSrcRegAlpha1(TexEnvReg::SrcReg::PREVIOUS);
        }
        break;
    case TexEnvMode::COMBINE:
        // Nothing to do here.
        break;
    default:
        break;
    }
    if (mode != TexEnvMode::COMBINE)
    {
        texEnvConf.setTmu(m_tmu);
        return m_renderer.setTexEnv(texEnvConf);
    }
    return true;
}

bool PixelPipeline::setTexEnvColor(const Vec4& color)
{
    return m_renderer.setTexEnvColor(m_tmu, { { static_cast<uint8_t>(color[0] * 255.0f),
                                                static_cast<uint8_t>(color[1] * 255.0f),
                                                static_cast<uint8_t>(color[2] * 255.0f),
                                                static_cast<uint8_t>(color[3] * 255.0f) } });
}

bool PixelPipeline::setClearColor(const Vec4& color)
{
    return m_renderer.setClearColor({ { static_cast<uint8_t>(color[0] * 255.0f),
                                        static_cast<uint8_t>(color[1] * 255.0f),
                                        static_cast<uint8_t>(color[2] * 255.0f),
                                        static_cast<uint8_t>(color[3] * 255.0f) } });
}

bool PixelPipeline::setClearDepth(const float depth)
{
    // Convert from signed float (-1.0 .. 1.0) to unsigned fix (0 .. 65535)
    const uint16_t depthx = (depth + 1.0f) * 32767;
    return m_renderer.setClearDepth(depthx);
}

bool PixelPipeline::clearFramebuffer(const bool frameBuffer, const bool zBuffer, const bool stencilBuffer) 
{ 
    bool ret = updatePipeline();
    return ret && m_renderer.clear(frameBuffer, zBuffer, stencilBuffer); 
}

PixelPipeline::StencilConfig& PixelPipeline::stencilConfig()
{ 
    if (m_enableTwoSideStencil)
    {
        if (m_stencilFace == StencilFace::FRONT)
        {
            return m_stencilConfFront;
        }
        return m_stencilConfBack;
    }
    return m_stencilConf;
}

void PixelPipeline::enableTwoSideStencil(const bool enable)
{
    m_enableTwoSideStencil = enable;
}

bool PixelPipeline::getEnableTwoSideStencil() const
{
    return m_enableTwoSideStencil;
}

void PixelPipeline::setStencilFace(const StencilFace face)
{
    m_stencilFace = face;
}

void PixelPipeline::selectStencilTwoSideFrontForDevice() 
{ 
    m_stencilConfTwoSide = &m_stencilConfFront; 
}

void PixelPipeline::selectStencilTwoSideBackForDevice() 
{ 
    m_stencilConfTwoSide = &m_stencilConfBack; 
}

} // namespace rr

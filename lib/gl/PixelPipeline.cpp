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
        m_renderer.setTexEnv(i, m_tmuConf[i].texEnvConf);
    }
    m_renderer.setFeatureEnableConfig(m_featureEnable);
    m_renderer.setFragmentPipelineConfig(m_fragmentPipelineConf);
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
        if ((m_tmuConf[i].texEnvMode == TexEnvMode::COMBINE) && (m_tmuConf[i].texEnvConfUploaded.serialize() != m_tmuConf[i].texEnvConf.serialize()))
        {
            ret = ret && m_renderer.setTexEnv(i, m_tmuConf[i].texEnvConf);
            m_tmuConf[i].texEnvConfUploaded = m_tmuConf[i].texEnvConf;
        }
    }
    
    if (m_featureEnableUploaded.serialize() != m_featureEnable.serialize())
    {
        ret = ret && m_renderer.setFeatureEnableConfig(m_featureEnable);
        m_featureEnableUploaded = m_featureEnable;
    }
    if (m_fragmentPipelineConfUploaded.serialize() != m_fragmentPipelineConf.serialize())
    {
        ret = ret && m_renderer.setFragmentPipelineConfig(m_fragmentPipelineConf);
        m_fragmentPipelineConfUploaded = m_fragmentPipelineConf;
    }
    if (m_fogDirty)
    {
        ret = ret && updateFogLut();
        m_fogDirty = false;
    }
    return ret;
}

bool PixelPipeline::uploadTexture(const std::shared_ptr<const uint16_t> pixels, uint16_t sizeX, uint16_t sizeY, IntendedInternalPixelFormat intendedPixelFormat)
{        
    return uploadTexture({ pixels, sizeX, sizeY, intendedPixelFormat });
}

bool PixelPipeline::uploadTexture(const TextureObject& texObj)
{
    bool ret = m_renderer.updateTexture(m_tmuConf[m_tmu].boundTexture, texObj);
            
    // Rebind texture to update the rasterizer with the new texture meta information
    // TODO: Check if this is still required
    ret = ret && useTexture();
    return ret;
}

void PixelPipeline::setFogMode(const FogMode val)
{
    if (m_fogMode != val)
    {
        m_fogMode = val;
        m_fogDirty = true;
    }
}

void PixelPipeline::setFogStart(const float val)
{
    if (m_fogStart != val)
    {
        m_fogStart = val;
        m_fogDirty = true;
    }
}

void PixelPipeline::setFogEnd(const float val)
{
    if (m_fogEnd != val)
    {
        m_fogEnd = val;
        m_fogDirty = true;
    }
}

void PixelPipeline::setFogDensity(const float val)
{
    if (m_fogDensity != val)
    {
        m_fogDensity = val;
        m_fogDirty = true;
    }
}

bool PixelPipeline::setFogColor(const Vec4& val)
{
    Vec4i color;
    color.fromVec(val.vec);
    color.mul<0>(255);
    return m_renderer.setFogColor(color);
}

bool PixelPipeline::updateFogLut()
{
    std::function <float(float)> fogFunction;

    // Set fog function
    switch (m_fogMode) {
    case FogMode::LINEAR:
        fogFunction = [&](float z) {
            float f = (m_fogEnd - z) / (m_fogEnd - m_fogStart);
            return f;
        };
        break;
    case FogMode::EXP:
        fogFunction = [&](float z) {
            float f = expf(-(m_fogDensity * z));
            return f;
        };
        break;
    case FogMode::EXP2:
        fogFunction = [&](float z) {
            float f = expf(powf(-(m_fogDensity * z), 2));
            return f;
        };
        break;
    default:
        fogFunction = [](float) {
            return 1.0f;
        };
        break;
    }

    // Calculate fog LUT
    std::array<float, 33> lut;
    for (std::size_t i = 0; i < lut.size(); i++)
    {
        float f = fogFunction(powf(2, i));
        lut[i] = f;
    }

    // Set fog LUT
    return m_renderer.setFogLut(lut, m_fogStart, m_fogEnd);
}

bool PixelPipeline::useTexture()
{
    return m_renderer.useTexture(m_tmu, m_tmuConf[m_tmu].boundTexture);
}

bool PixelPipeline::setTexEnvMode(const TexEnvMode mode)
{
    m_tmuConf[m_tmu].texEnvMode = mode;
    IRenderer::TexEnvConf texEnvConf {};
    switch (mode) {
    case TexEnvMode::DISABLE:
        texEnvConf.setCombineRgb(IRenderer::TexEnvConf::Combine::REPLACE);
        texEnvConf.setCombineRgb(IRenderer::TexEnvConf::Combine::REPLACE);
        texEnvConf.setCombineAlpha(IRenderer::TexEnvConf::Combine::REPLACE);
        texEnvConf.setSrcRegRgb0(IRenderer::TexEnvConf::SrcReg::PRIMARY_COLOR);
        texEnvConf.setSrcRegAlpha0(IRenderer::TexEnvConf::SrcReg::PRIMARY_COLOR);
        break;
    case TexEnvMode::REPLACE:
        texEnvConf.setCombineRgb(IRenderer::TexEnvConf::Combine::REPLACE);
        texEnvConf.setCombineAlpha(IRenderer::TexEnvConf::Combine::REPLACE);
        texEnvConf.setSrcRegRgb0(IRenderer::TexEnvConf::SrcReg::TEXTURE);
        texEnvConf.setSrcRegAlpha0(IRenderer::TexEnvConf::SrcReg::TEXTURE);
        break;
    case TexEnvMode::MODULATE:
        texEnvConf.setCombineRgb(IRenderer::TexEnvConf::Combine::MODULATE);
        texEnvConf.setCombineAlpha(IRenderer::TexEnvConf::Combine::MODULATE);
        texEnvConf.setSrcRegRgb0(IRenderer::TexEnvConf::SrcReg::TEXTURE);
        texEnvConf.setSrcRegRgb1(IRenderer::TexEnvConf::SrcReg::PRIMARY_COLOR);
        texEnvConf.setSrcRegAlpha0(IRenderer::TexEnvConf::SrcReg::TEXTURE);
        texEnvConf.setSrcRegAlpha1(IRenderer::TexEnvConf::SrcReg::PRIMARY_COLOR);
        break;
    case TexEnvMode::DECAL:
        texEnvConf.setCombineRgb(IRenderer::TexEnvConf::Combine::INTERPOLATE);
        texEnvConf.setCombineAlpha(IRenderer::TexEnvConf::Combine::REPLACE);
        texEnvConf.setSrcRegRgb0(IRenderer::TexEnvConf::SrcReg::TEXTURE);
        texEnvConf.setSrcRegRgb1(IRenderer::TexEnvConf::SrcReg::PRIMARY_COLOR);
        texEnvConf.setSrcRegRgb2(IRenderer::TexEnvConf::SrcReg::TEXTURE);
        texEnvConf.setSrcRegAlpha0(IRenderer::TexEnvConf::SrcReg::PRIMARY_COLOR);
        texEnvConf.setOperandRgb2(IRenderer::TexEnvConf::Operand::SRC_ALPHA);
        break;
    case TexEnvMode::BLEND:
        texEnvConf.setCombineRgb(IRenderer::TexEnvConf::Combine::INTERPOLATE);
        texEnvConf.setCombineAlpha(IRenderer::TexEnvConf::Combine::MODULATE);
        texEnvConf.setSrcRegRgb0(IRenderer::TexEnvConf::SrcReg::CONSTANT);
        texEnvConf.setSrcRegRgb1(IRenderer::TexEnvConf::SrcReg::PRIMARY_COLOR);
        texEnvConf.setSrcRegRgb2(IRenderer::TexEnvConf::SrcReg::TEXTURE);
        texEnvConf.setSrcRegAlpha0(IRenderer::TexEnvConf::SrcReg::PRIMARY_COLOR);
        texEnvConf.setSrcRegAlpha1(IRenderer::TexEnvConf::SrcReg::TEXTURE);
        break;
    case TexEnvMode::ADD:
        texEnvConf.setCombineRgb(IRenderer::TexEnvConf::Combine::ADD);
        texEnvConf.setCombineAlpha(IRenderer::TexEnvConf::Combine::ADD);
        texEnvConf.setSrcRegRgb0(IRenderer::TexEnvConf::SrcReg::TEXTURE);
        texEnvConf.setSrcRegRgb1(IRenderer::TexEnvConf::SrcReg::PRIMARY_COLOR);
        texEnvConf.setSrcRegAlpha0(IRenderer::TexEnvConf::SrcReg::TEXTURE);
        texEnvConf.setSrcRegAlpha1(IRenderer::TexEnvConf::SrcReg::PRIMARY_COLOR);
        break;
    case TexEnvMode::COMBINE:
        // Nothing to do here.
        break;
    default:
        break;
    }
    if (mode != TexEnvMode::COMBINE)
    {
        return m_renderer.setTexEnv(m_tmu, texEnvConf);
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

bool PixelPipeline::clearFramebuffer(bool frameBuffer, bool zBuffer) 
{ 
    bool ret = updatePipeline();
    return ret && m_renderer.clear(frameBuffer, zBuffer); 
}

} // namespace rr
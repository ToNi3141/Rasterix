// Rasterix
// https://github.com/ToNi3141/Rasterix
// Copyright (c) 2022 ToNi3141

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

PixelPipeline::PixelPipeline(IRenderer& renderer) 
    : m_renderer(renderer)
{
    m_renderer.setTexEnv(IRenderer::TMU::TMU0, m_texEnvConf0);
    m_renderer.setFeatureEnableConfig(m_featureEnable);
    m_renderer.setFragmentPipelineConfig(m_fragmentPipelineConf);
}

bool PixelPipeline::drawTriangle(const Vec4& v0,
                                 const Vec4& v1,
                                 const Vec4& v2,
                                 const Vec4& tc0,
                                 const Vec4& tc1,
                                 const Vec4& tc2,
                                 const Vec4& c0,
                                 const Vec4& c1,
                                 const Vec4& c2) 
{
    return m_renderer.drawTriangle(v0, v1, v2, tc0, tc1, tc2, c0, c1, c2);
}

bool PixelPipeline::updatePipeline()
{
    bool ret { true };
    if ((m_texEnvMode == TexEnvMode::COMBINE) && (m_texEnvConfUploaded0.serialize() != m_texEnvConf0.serialize()))
    {
        ret = ret && m_renderer.setTexEnv(IRenderer::TMU::TMU0, m_texEnvConf0);
        m_texEnvConfUploaded0 = m_texEnvConf0;
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

bool PixelPipeline::uploadTexture(const std::shared_ptr<uint16_t> pixels, uint16_t sizeX, uint16_t sizeY, IRenderer::PixelFormat pixelFormat)
{
    bool ret = m_renderer.updateTexture(m_boundTexture, pixels, sizeX, sizeY, m_texWrapModeS, m_texWrapModeT, m_texEnableMagFilter, pixelFormat);
            
    // Rebind texture to update the rasterizer with the new texture meta information
    // TODO: Check if this is still required
    ret = ret && useTexture(TMU::TMU0);
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

bool PixelPipeline::useTexture(const TMU& tmu)
{
    return m_renderer.useTexture(tmu, m_boundTexture);
}

bool PixelPipeline::setTexEnvMode(const TexEnvMode mode)
{
    m_texEnvMode = mode;
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
        return m_renderer.setTexEnv(IRenderer::TMU::TMU0, texEnvConf);
    }
    return true;
}

bool PixelPipeline::setTexEnvColor(const Vec4& color)
{
    return m_renderer.setTexEnvColor({ { static_cast<uint8_t>(color[0] * 255.0f),
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

uint16_t PixelPipeline::convertColor(IRenderer::PixelFormat& outFormat, const TargetPixelFormat format, const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a)
{
    uint16_t color {};
    switch (format)
    {
        case TargetPixelFormat::ALPHA:
            color = static_cast<uint16_t>(a >> 4);
            outFormat = IRenderer::PixelFormat::RGBA4444;
            break;
        case TargetPixelFormat::LUMINANCE:
            color |= static_cast<uint16_t>(r >> 3) << 11;
            color |= static_cast<uint16_t>(r >> 2) << 5;
            color |= static_cast<uint16_t>(r >> 3) << 0;
            outFormat = IRenderer::PixelFormat::RGB565;
            break;
        case TargetPixelFormat::INTENSITY:
            color |= static_cast<uint16_t>(r >> 4) << 12;
            color |= static_cast<uint16_t>(r >> 4) << 8;
            color |= static_cast<uint16_t>(r >> 4) << 4;
            color |= static_cast<uint16_t>(r >> 4) << 0;
            outFormat = IRenderer::PixelFormat::RGBA4444;
            break;
        case TargetPixelFormat::LUMINANCE_ALPHA:
            color |= static_cast<uint16_t>(r >> 4) << 12;
            color |= static_cast<uint16_t>(r >> 4) << 8;
            color |= static_cast<uint16_t>(r >> 4) << 4;
            color |= static_cast<uint16_t>(a >> 4) << 0;
            outFormat = IRenderer::PixelFormat::RGBA4444;
            break;
        case TargetPixelFormat::RGB:
            color |= static_cast<uint16_t>(r >> 3) << 11;
            color |= static_cast<uint16_t>(g >> 2) << 5;
            color |= static_cast<uint16_t>(b >> 3) << 0;
            outFormat = IRenderer::PixelFormat::RGB565;
            break;
        case TargetPixelFormat::RGBA:
            color |= static_cast<uint16_t>(r >> 4) << 12;
            color |= static_cast<uint16_t>(g >> 4) << 8;
            color |= static_cast<uint16_t>(b >> 4) << 4;
            color |= static_cast<uint16_t>(a >> 4) << 0;
            outFormat = IRenderer::PixelFormat::RGBA4444;
            break;
        case TargetPixelFormat::RGBA1:
            color |= static_cast<uint16_t>(r >> 5) << 11;
            color |= static_cast<uint16_t>(g >> 5) << 6;
            color |= static_cast<uint16_t>(b >> 5) << 1;
            color |= static_cast<uint16_t>(a >> 1) << 0;
            outFormat = IRenderer::PixelFormat::RGBA5551;
            break;
        default:
        break;
    }
    return color;
}
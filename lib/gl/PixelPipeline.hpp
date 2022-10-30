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


#ifndef PIXELPIPELINE_HPP_
#define PIXELPIPELINE_HPP_

#include "IRenderer.hpp"
#include "Vec.hpp"

class PixelPipeline
{
public:
    using Triangle = IRenderer::Triangle;

    enum class FogMode
    {
        ONE,
        LINEAR,
        EXP,
        EXP2
    };

    enum class TexEnvMode
    {
        DISABLE,
        REPLACE,
        MODULATE,
        DECAL,
        BLEND,
        ADD,
        COMBINE
    };

    using FragmentPipeline = IRenderer::FragmentPipelineConf;
    using TestFunc = IRenderer::FragmentPipelineConf::TestFunc;
    using BlendFunc = IRenderer::FragmentPipelineConf::BlendFunc;
    using LogicOp = IRenderer::FragmentPipelineConf::LogicOp;

    using TMU = IRenderer::TMU;
    using TextureWrapMode = IRenderer::TextureObject::TextureWrapMode;
    using PixelFormat = IRenderer::TextureObject::PixelFormat;
    using IntendedInternalPixelFormat = IRenderer::TextureObject::IntendedInternalPixelFormat;
    using Combine = IRenderer::TexEnvConf::Combine;
    using TexEnv = IRenderer::TexEnvConf;
    using TextureObject = IRenderer::TextureObject;
    
    using FeatureEnable = IRenderer::FeatureEnableConf;

    PixelPipeline(IRenderer& renderer);

    bool drawTriangle(const Triangle& triangle);
    bool updatePipeline();

    // Feature Enable
    FeatureEnable& featureEnable() { return m_featureEnable; }

    // Textures
    bool uploadTexture(const std::shared_ptr<const uint16_t> pixels, uint16_t sizeX, uint16_t sizeY, IntendedInternalPixelFormat intendedPixelFormat);
    bool uploadTexture(const TextureObject& texObj);
    TextureObject getTexture() { return m_renderer.getTexture(m_boundTexture); }
    bool useTexture();
    std::pair<bool, uint16_t> createTexture() { return m_renderer.createTexture(); }
    bool deleteTexture(const uint32_t texture) { return m_renderer.deleteTexture(texture); }
    void setBoundTexture(const uint32_t val) { m_boundTexture = val; }
    void setTexWrapModeS(const TextureWrapMode mode) { m_texWrapModeS = mode; }
    void setTexWrapModeT(const TextureWrapMode mode) { m_texWrapModeT = mode; }
    void setEnableMagFilter(const bool val) { m_texEnableMagFilter = val; }

    // Framebuffer
    bool clearFramebuffer(bool frameBuffer, bool zBuffer);
    bool setClearColor(const Vec4& color);
    bool setClearDepth(const float depth);

    // Fragment Pipeline
    FragmentPipeline& fragmentPipeline() { return m_fragmentPipelineConf; }

    // TMU
    bool setTexEnvMode(const TexEnvMode mode);
    TexEnv& texEnv() { return m_texEnvConf[m_tmu]; }
    bool setTexEnvColor(const Vec4& color);
    void activateTmu(const IRenderer::TMU tmu) { m_tmu = tmu; }

    // Fog
    void setFogMode(const FogMode val);
    void setFogStart(const float val);
    void setFogEnd(const float val);
    void setFogDensity(const float val);
    bool setFogColor(const Vec4& val);

    // Scissor 
    void setScissorBox(const int32_t x, int32_t y, const uint32_t width, const uint32_t height) { m_renderer.setScissorBox(x, y, width, height); }

private:
    static constexpr uint8_t MAX_TMU_COUNT { IRenderer::MAX_TMU_COUNT };

    bool updateFogLut();

    IRenderer& m_renderer;

    // Feature Enable
    FeatureEnable m_featureEnable {};
    FeatureEnable m_featureEnableUploaded {};

    // Textures
    uint32_t m_boundTexture { 0 };
    TextureWrapMode m_texWrapModeS { TextureWrapMode::REPEAT };
    TextureWrapMode m_texWrapModeT { TextureWrapMode::REPEAT };
    bool m_texEnableMagFilter { true };

    // TMU
    std::array<TexEnvMode, MAX_TMU_COUNT> m_texEnvMode { TexEnvMode::REPLACE };
    std::array<TexEnv, MAX_TMU_COUNT> m_texEnvConf {};
    std::array<TexEnv, MAX_TMU_COUNT> m_texEnvConfUploaded {};
    uint8_t m_tmu { 0 };

    // Current fragment pipeline configuration 
    FragmentPipeline m_fragmentPipelineConf {};
    FragmentPipeline m_fragmentPipelineConfUploaded {};

    // Fog
    bool m_fogDirty { false };
    FogMode m_fogMode { FogMode::EXP };
    float m_fogStart { 0.0f };
    float m_fogEnd { 1.0f };
    float m_fogDensity { 1.0f };
};

#endif // PIXELPIPELINE_HPP_

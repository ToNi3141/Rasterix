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
    using TextureWrapMode = IRenderer::TextureWrapMode;
    using Combine = IRenderer::TexEnvConf::Combine;
    using TexEnv = IRenderer::TexEnvConf;
    
    using FeatureEnable = IRenderer::FeatureEnableConf;

    PixelPipeline(IRenderer& renderer);

    bool drawTriangle(const Vec4& v0,
                      const Vec4& v1,
                      const Vec4& v2,
                      const Vec2& st0,
                      const Vec2& st1,
                      const Vec2& st2,
                      const Vec4& c0,
                      const Vec4& c1,
                      const Vec4& c2);
    bool updatePipeline();

    // Feature Enable
    FeatureEnable& featureEnable() { return m_featureEnable; }

    // Textures
    // Only RGBA4444 textures are supported
    bool uploadTexture(const std::shared_ptr<uint16_t> pixels, uint16_t sizeX, uint16_t sizeY);
    bool useTexture(const TMU& tmu);
    std::pair<bool, uint16_t> createTexture() { return m_renderer.createTexture(); }
    bool deleteTexture(const uint32_t texture) { return m_renderer.deleteTexture(texture); }
    void setBoundTexture(const uint32_t val) { m_boundTexture = val; }
    void setTexWrapModeS(const TextureWrapMode mode) { m_texWrapModeS = mode; }
    void setTexWrapModeT(const TextureWrapMode mode) { m_texWrapModeT = mode; }
    void setEnableMagFilter(const bool val) { m_texEnableMagFilter = val; }

    // Framebuffer
    bool clearFramebuffer(bool frameBuffer, bool zBuffer) { return m_renderer.clear(frameBuffer, zBuffer); }
    bool setClearColor(const Vec4& color);
    bool setClearDepth(const float depth);

    // Fragment Pipeline
    FragmentPipeline& fragmentPipeline() { return m_fragmentPipelineConf; }

    // TMU
    bool setTexEnvMode(const TexEnvMode mode);
    TexEnv& texEnv() { return m_texEnvConf0; }
    bool setTexEnvColor(const Vec4& color);

    // Fog
    void setFogMode(const FogMode val);
    void setFogStart(const float val);
    void setFogEnd(const float val);
    void setFogDensity(const float val);
    bool setFogColor(const Vec4& val);

private:
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
    TexEnvMode m_texEnvMode { TexEnvMode::REPLACE };
    TexEnv m_texEnvConf0 {};
    TexEnv m_texEnvConfUploaded0 {};

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

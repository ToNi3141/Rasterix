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


#ifndef PIXELPIPELINE_HPP_
#define PIXELPIPELINE_HPP_

#include "IRenderer.hpp"
#include "Vec.hpp"
#include <optional>
#include "Fog.hpp"

namespace rr
{
class PixelPipeline
{
public:
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

    enum class StencilFace 
    {
        FRONT,
        BACK
    };

    using TMU = IRenderer::TMU;
    using TextureWrapMode = IRenderer::TextureWrapMode;

    using FragmentPipeline = FragmentPipelineReg;
    using TexEnv = TexEnvReg;
    using StencilConfig = StencilReg;
    using FeatureEnable = FeatureEnableReg;

    PixelPipeline(IRenderer& renderer);

    bool drawTriangle(const Triangle& triangle);
    bool updatePipeline();

    // Feature Enable
    void setEnableTmu(const bool enable) { m_featureEnable.setEnableTmu(m_tmu, enable); }
    void setEnableAlphaTest(const bool enable) { m_featureEnable.setEnableAlphaTest(enable); }
    void setEnableDepthTest(const bool enable) { m_featureEnable.setEnableDepthTest(enable); }
    void setEnableBlending(const bool enable) { m_featureEnable.setEnableBlending(enable); }
    void setEnableFog(const bool enable) { m_featureEnable.setEnableFog(enable); }
    void setEnableScissor(const bool enable) { m_featureEnable.setEnableScissor(enable); }
    void setEnableStencil(const bool enable) { m_featureEnable.setEnableStencilTest(enable); }
    bool getEnableTmu() const { return m_featureEnable.getEnableTmu(m_tmu); }
    bool getEnableTmu(const uint8_t tmu) const { return m_featureEnable.getEnableTmu(tmu); }
    bool getEnableAlphaTest() const { return m_featureEnable.getEnableAlphaTest(); }
    bool getEnableDepthTest() const { return m_featureEnable.getEnableDepthTest(); }
    bool getEnableBlending() const { return m_featureEnable.getEnableBlending(); }
    bool getEnableFog() const { return m_featureEnable.getEnableFog(); }
    bool getEnableScissor() const { return m_featureEnable.getEnableScissor(); }
    bool getEnableStencil() const { return m_featureEnable.getEnableStencilTest(); }

    // Textures
    bool uploadTexture();
    TextureObjectMipmap& getTexture();
    bool useTexture();
    bool isTextureValid(const uint16_t texId) const { return m_renderer.isTextureValid(texId); };
    std::pair<bool, uint16_t> createTexture() { return m_renderer.createTexture(); }
    bool createTextureWithName(const uint16_t texId) { return m_renderer.createTextureWithName(texId); };
    bool deleteTexture(const uint32_t texture) { return m_renderer.deleteTexture(texture); }
    void setBoundTexture(const uint32_t val) { uploadTexture(); m_tmuConf[m_tmu].boundTexture = val; }
    void setTexWrapModeS(const TextureWrapMode mode) { m_renderer.setTextureWrapModeS(m_tmuConf[m_tmu].boundTexture, mode); }
    void setTexWrapModeT(const TextureWrapMode mode) { m_renderer.setTextureWrapModeT(m_tmuConf[m_tmu].boundTexture, mode); }
    void setEnableMagFilter(const bool val) { m_renderer.enableTextureMagFiltering(m_tmuConf[m_tmu].boundTexture, val); }
    void setEnableMinFilter(const bool val) { m_renderer.enableTextureMinFiltering(m_tmuConf[m_tmu].boundTexture, val); }

    // Framebuffer
    bool clearFramebuffer(const bool frameBuffer, const bool zBuffer, const bool stencilBuffer);
    bool setClearColor(const Vec4& color);
    bool setClearDepth(const float depth);

    // Fragment Pipeline
    FragmentPipeline& fragmentPipeline() { return m_fragmentPipelineConf; }

    // Stencil Buffer
    StencilConfig& stencilConfig();
    void enableTwoSideStencil(const bool enable);
    bool getEnableTwoSideStencil() const;
    void setStencilFace(const StencilFace face);
    void selectStencilTwoSideFrontForDevice();
    void selectStencilTwoSideBackForDevice();

    // TMU
    bool setTexEnvMode(const TexEnvMode mode);
    TexEnv& texEnv() { return m_tmuConf[m_tmu].texEnvConf; }
    bool setTexEnvColor(const Vec4& color);
    void activateTmu(const IRenderer::TMU tmu) { uploadTexture(); m_tmu = tmu; }

    // Fog
    Fog& fog() { return m_fog; }

    // Scissor 
    void setScissorBox(const int32_t x, int32_t y, const uint32_t width, const uint32_t height) { m_renderer.setScissorBox(x, y, width, height); }

private:
    static constexpr uint8_t MAX_TMU_COUNT { IRenderer::MAX_TMU_COUNT };

    struct TmuConfig
    {
        // Textures
        uint32_t boundTexture { 0 };

        // TMU
       TexEnvMode texEnvMode { TexEnvMode::REPLACE };
       TexEnv texEnvConf {};
       TexEnv texEnvConfUploaded {};
    };

    bool updateFogLut();

    IRenderer& m_renderer;

    // Feature Enable
    FeatureEnable m_featureEnable {};
    FeatureEnable m_featureEnableUploaded {};

    // TMU
    std::array<TmuConfig, MAX_TMU_COUNT> m_tmuConf {};
    uint8_t m_tmu { 0 };
    std::optional<TextureObjectMipmap> m_textureObjectMipmap {};

    // Current fragment pipeline configuration 
    FragmentPipeline m_fragmentPipelineConf {};
    FragmentPipeline m_fragmentPipelineConfUploaded {};

    // Current stencil buffer configuration
    bool m_enableTwoSideStencil { false };
    StencilFace m_stencilFace { StencilFace::FRONT };
    StencilConfig m_stencilConf {};
    StencilConfig m_stencilConfFront {};
    StencilConfig m_stencilConfBack {};
    StencilConfig* m_stencilConfTwoSide { &m_stencilConfFront };
    StencilConfig m_stencilConfUploaded {};

    Fog m_fog { m_renderer };
};

} // namespace rr
#endif // PIXELPIPELINE_HPP_

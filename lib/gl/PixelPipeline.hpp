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
#include "Fogging.hpp"
#include "Texture.hpp"
#include "Stencil.hpp"
#include "FragmentPipeline.hpp"

namespace rr
{
class PixelPipeline
{
public:
    using FeatureEnable = FeatureEnableReg;

    PixelPipeline(IRenderer& renderer);

    bool drawTriangle(const Triangle& triangle);
    bool updatePipeline();

    // Feature Enable
    void setEnableTmu(const bool enable) { m_featureEnable.setEnableTmu(m_texture.getActiveTmu(), enable); }
    void setEnableAlphaTest(const bool enable) { m_featureEnable.setEnableAlphaTest(enable); }
    void setEnableDepthTest(const bool enable) { m_featureEnable.setEnableDepthTest(enable); }
    void setEnableBlending(const bool enable) { m_featureEnable.setEnableBlending(enable); }
    void setEnableFog(const bool enable) { m_featureEnable.setEnableFog(enable); }
    void setEnableScissor(const bool enable) { m_featureEnable.setEnableScissor(enable); }
    void setEnableStencil(const bool enable) { m_featureEnable.setEnableStencilTest(enable); }
    bool getEnableTmu() const { return m_featureEnable.getEnableTmu(m_texture.getActiveTmu()); }
    bool getEnableTmu(const uint8_t tmu) const { return m_featureEnable.getEnableTmu(tmu); }
    bool getEnableAlphaTest() const { return m_featureEnable.getEnableAlphaTest(); }
    bool getEnableDepthTest() const { return m_featureEnable.getEnableDepthTest(); }
    bool getEnableBlending() const { return m_featureEnable.getEnableBlending(); }
    bool getEnableFog() const { return m_featureEnable.getEnableFog(); }
    bool getEnableScissor() const { return m_featureEnable.getEnableScissor(); }
    bool getEnableStencil() const { return m_featureEnable.getEnableStencilTest(); }

    // Framebuffer
    bool clearFramebuffer(const bool frameBuffer, const bool zBuffer, const bool stencilBuffer);
    bool setClearColor(const Vec4& color);
    bool setClearDepth(const float depth);


    Fogging& fog() { return m_fog; }
    Texture& texture() { return m_texture; }
    Stencil& stencil() { return m_stencil; }
    FragmentPipeline& fragmentPipeline() { return m_fragmentPipeline; }

    // Scissor 
    void setScissorBox(const int32_t x, int32_t y, const uint32_t width, const uint32_t height) { m_renderer.setScissorBox(x, y, width, height); }

private:
    bool updateFogLut();

    IRenderer& m_renderer;

    // Feature Enable
    FeatureEnable m_featureEnable {};
    FeatureEnable m_featureEnableUploaded {};

    Fogging m_fog { m_renderer };
    Texture m_texture { m_renderer };
    Stencil m_stencil { m_renderer };
    FragmentPipeline m_fragmentPipeline { m_renderer };
};

} // namespace rr
#endif // PIXELPIPELINE_HPP_

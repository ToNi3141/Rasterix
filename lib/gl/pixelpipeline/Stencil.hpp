// Rasterix
// https://github.com/ToNi3141/Rasterix
// Copyright (c) 2024 ToNi3141

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

#ifndef STENCIL_HPP_
#define STENCIL_HPP_

#include "math/Vec.hpp"
#include "renderer/Renderer.hpp"
#include <optional>

namespace rr
{
class Stencil
{
public:
    using StencilConfig = StencilReg;

    enum class StencilFace
    {
        FRONT,
        BACK
    };

    Stencil(Renderer& renderer);

    void setTestFunc(const TestFunc val) { stencilConfig().setTestFunc(val); }
    void setOpZPass(const StencilOp val) { stencilConfig().setOpZPass(val); }
    void setOpZFail(const StencilOp val) { stencilConfig().setOpZFail(val); }
    void setOpFail(const StencilOp val) { stencilConfig().setOpFail(val); }
    void setMask(const uint8_t val) { stencilConfig().setMask(val); }
    void setRef(const uint8_t val) { stencilConfig().setRef(val); }
    void setClearStencil(const uint8_t val) { stencilConfig().setClearStencil(val); }
    void setStencilMask(const uint8_t val) { stencilConfig().setStencilMask(val); }

    TestFunc getTestFunc() const { return stencilConfig().getTestFunc(); }
    StencilOp getOpZPass() const { return stencilConfig().getOpZPass(); }
    StencilOp getOpZFail() const { return stencilConfig().getOpZFail(); }
    StencilOp getOpFail() const { return stencilConfig().getOpFail(); }
    uint8_t getRef() const { return stencilConfig().getRef(); }
    uint8_t getMask() const { return stencilConfig().getMask(); }
    uint8_t getClearStencil() const { return stencilConfig().getClearStencil(); }
    uint8_t getStencilMask() const { return stencilConfig().getStencilMask(); }

    void enableTwoSideStencil(const bool enable) { m_enableTwoSideStencil = enable; }
    void setStencilFace(const StencilFace face) { m_stencilFace = face; }

    bool updateStencilFace(const Vec4& v0, const Vec4& v1, const Vec4& v2);
    bool update();

private:
    void selectStencilTwoSideFrontForDevice() { m_stencilConfTwoSide = &m_stencilConfFront; }
    void selectStencilTwoSideBackForDevice() { m_stencilConfTwoSide = &m_stencilConfBack; }

    StencilConfig& stencilConfig();
    const StencilConfig& stencilConfig() const { return stencilConfig(); };

    Renderer& m_renderer;

    bool m_enableTwoSideStencil { false };
    StencilFace m_stencilFace { StencilFace::FRONT };
    StencilConfig m_stencilConf {};
    StencilConfig m_stencilConfFront {};
    StencilConfig m_stencilConfBack {};
    StencilConfig* m_stencilConfTwoSide { &m_stencilConfFront };
    StencilConfig m_stencilConfUploaded {};
};

} // namespace rr
#endif // STENCIL_HPP_

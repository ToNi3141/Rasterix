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

#include "IRenderer.hpp"
#include "Vec.hpp"
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


    Stencil(IRenderer& renderer);

    StencilConfig& stencilConfig();
    void enableTwoSideStencil(const bool enable) { m_enableTwoSideStencil = enable; }
    bool getEnableTwoSideStencil() const { return m_enableTwoSideStencil; }
    void setStencilFace(const StencilFace face) { m_stencilFace = face; }
    void selectStencilTwoSideFrontForDevice() { m_stencilConfTwoSide = &m_stencilConfFront; }
    void selectStencilTwoSideBackForDevice() { m_stencilConfTwoSide = &m_stencilConfBack; }

    bool update();
private:
    IRenderer& m_renderer;

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

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


#include "Stencil.hpp"
#include "renderer/Rasterizer.hpp"
#include <spdlog/spdlog.h>

namespace rr
{
Stencil::Stencil(IRenderer& renderer)
    : m_renderer(renderer)
{
    m_renderer.setStencilBufferConfig(m_stencilConf);
}

Stencil::StencilConfig& Stencil::stencilConfig()
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

bool Stencil::update()
{
    bool ret { true };

    if (m_enableTwoSideStencil)
    {
        if (m_stencilConfUploaded.serialize() != m_stencilConfTwoSide->serialize()) 
        {
            ret = ret && m_renderer.setStencilBufferConfig(*m_stencilConfTwoSide);
            m_stencilConfUploaded = *m_stencilConfTwoSide;
        }
    }
    else 
    {
        if (m_stencilConfUploaded.serialize() != m_stencilConf.serialize()) 
        {
            ret = ret && m_renderer.setStencilBufferConfig(m_stencilConf);
            m_stencilConfUploaded = m_stencilConf;
        }
    }

    return ret;
}

bool Stencil::updateStencilFace(const Vec4& v0, const Vec4& v1, const Vec4& v2)
{
    if (m_enableTwoSideStencil)
    {
        const float edgeVal = Rasterizer::edgeFunctionFloat(v0, v1, v2);
        const StencilFace currentOrientation = (edgeVal <= 0.0f) ? StencilFace::BACK : StencilFace::FRONT;
        if (currentOrientation != StencilFace::FRONT) // TODO: The rasterizer expects triangles in CW. OpenGL in CCW. Thats the reason why Front and Back a screwed up.
        {
            selectStencilTwoSideFrontForDevice();
        }
        else
        {
            selectStencilTwoSideBackForDevice();
        }
        if (!update()) 
        {
            SPDLOG_ERROR("updateStencilFace(): Cannot update pixel pipeline");
            return false;
        }
    }
    return true;
}

} // namespace rr

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

#include "Culling.hpp"
#include "renderer/Rasterizer.hpp"

namespace rr
{

Culling::Culling()
{

}

bool Culling::cull(const Vec4& v0, const Vec4& v1, const Vec4& v2) const
{
    if (m_enableCulling)
    {
        const float edgeVal { Rasterizer::edgeFunctionFloat(v0, v1, v2) };
        const Face currentOrientation = (edgeVal <= 0.0f) ? Face::BACK : Face::FRONT;
        if (currentOrientation != m_cullMode) [[unlikely]] // TODO: The rasterizer expects triangles in CW. OpenGL in CCW. Thats the reason why Front and Back a screwed up.
        {
            return true;
        }   
    }
    return false;
}

void Culling::setCullMode(const Face mode)
{
    m_cullMode = mode;
}

void Culling::enableCulling(const bool enable)
{
    m_enableCulling = enable;
}

}

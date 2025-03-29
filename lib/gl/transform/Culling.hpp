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

#ifndef CULLING_HPP
#define CULLING_HPP

#include "Enums.hpp"
#include "Types.hpp"
#include "math/Vec.hpp"
#include "renderer/Rasterizer.hpp"

namespace rr::culling
{

struct CullingData
{
    bool enableCulling { false };
    Face cullMode { Face::BACK };
};

class CullingCalc
{
public:
    CullingCalc(const CullingData& cullingData)
        : m_data { cullingData }
    {
    }
    bool cull(const Vec4& v0, const Vec4& v1, const Vec4& v2) const
    {
        if (m_data.enableCulling)
        {
            const float edgeVal { Rasterizer::edgeFunctionFloat(v0, v1, v2) };
            const Face currentOrientation = (edgeVal <= 0.0f) ? Face::BACK : Face::FRONT;
            if (currentOrientation != m_data.cullMode) // TODO: The rasterizer expects triangles in CW. OpenGL in CCW. Thats the reason why Front and Back a screwed up.
            {
                return true;
            }
        }
        return false;
    }

private:
    const CullingData& m_data;
};

class CullingSetter
{
public:
    CullingSetter(CullingData& cullingData)
        : m_data { cullingData }
    {
    }

    void enableCulling(const bool enable);
    void setCullMode(const Face mode);

private:
    CullingData& m_data;
};

} // namespace rr::culling
#endif // CULLING_HPP
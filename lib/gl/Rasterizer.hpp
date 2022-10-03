// Rasterix
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

#ifndef RASTERIZER_HPP
#define RASTERIZER_HPP
#include <stdint.h>
#include <array>
#include "Vec.hpp"
#include "IRenderer.hpp"

class Rasterizer
{
public:  
    struct __attribute__ ((__packed__)) RasterizedTriangle
    {
        uint32_t reserved;
        uint16_t bbStartX;
        uint16_t bbStartY;
        uint16_t bbEndX;
        uint16_t bbEndY;
        Vec3i wInit;
        Vec3i wXInc;
        Vec3i wYInc;
        // TODO: Maybe rearrange the vertex attributes in the hardware to avoid copying of data in the software
        Vec3 texStq;
        Vec3 texStqXInc;
        Vec3 texStqYInc;
        float depthW;
        float depthWXInc;
        float depthWYInc;
        float depthZ;
        float depthZXInc;
        float depthZYInc;
        Vec4 color;
        Vec4 colorXInc;
        Vec4 colorYInc;
    };

    Rasterizer();
    bool rasterize(RasterizedTriangle& rasterizedTriangle,
                   const Vec4& v0f,
                   const Vec4& tc0f,
                   const Vec4& c0f,
                   const Vec4& v1f,
                   const Vec4& tc1f,
                   const Vec4& c1f,
                   const Vec4& v2f,
                   const Vec4& tc2f,
                   const Vec4& c2f);

    void setScissorBox(const int32_t x, const int32_t y, const uint32_t width, const uint32_t height);
    void enableScissor(const bool enable);

    static bool calcLineIncrement(RasterizedTriangle &incrementedTriangle,
                                  const RasterizedTriangle &triangleToIncrement,
                                  const uint16_t lineStart,
                                  const uint16_t lineEnd);

    static float edgeFunctionFloat(const Vec4 &a, const Vec4 &b, const Vec4 &c);

    static bool checkIfTriangleIsInBounds(Rasterizer::RasterizedTriangle &triangle,
                                                 const uint16_t lineStart,
                                                 const uint16_t lineEnd);
private:
    static constexpr uint64_t DECIMAL_POINT = 12;
    inline bool rasterizeFixPoint(RasterizedTriangle& rasterizedTriangle,
                                         const Vec4& v0f,
                                         const Vec4& tc0f,
                                         const Vec4& c0f,
                                         const Vec4& v1f,
                                         const Vec4& tc1f,
                                         const Vec4& c1f,
                                         const Vec4& v2f,
                                         const Vec4& tc2f,
                                         const Vec4& c2f);
    inline static VecInt edgeFunctionFixPoint(const Vec2i &a, const Vec2i &b, const Vec2i &c);
    inline static VecInt calcRecip(VecInt val);

    int32_t m_scissorX { 0 };
    int32_t m_scissorY { 0 };
    uint32_t m_scissorWidth { 0 };
    uint32_t m_scissorHeight { 0 };
    bool m_enableScissor { false };

};

#endif // RASTERIZER_HPP

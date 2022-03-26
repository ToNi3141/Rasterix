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
        uint16_t triangleStaticColor;
        uint16_t triangleConfiguration;
        uint16_t bbStartX;
        uint16_t bbStartY;
        uint16_t bbEndX;
        uint16_t bbEndY;
        Vec3i wInit;
        Vec3i wXInc;
        Vec3i wYInc;
        // TODO: Maybe rearrange the vertex attributes in the hardware to avoid copying of data in the software
        Vec2 texSt;
        Vec2 texStXInc;
        Vec2 texStYInc;
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
    static bool rasterize(RasterizedTriangle& rasterizedTriangle,
                          const Vec4& v0f,
                          const Vec2& st0f,
                          const Vec4& c0f,
                          const Vec4& v1f,
                          const Vec2& st1f,
                          const Vec4& c1f,
                          const Vec4& v2f,
                          const Vec2& st2f,
                          const Vec4& c2f);

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
    inline static bool rasterizeFixPoint(RasterizedTriangle& rasterizedTriangle,
                                         const Vec4& v0f,
                                         const Vec2& st0f,
                                         const Vec4& c0f,
                                         const Vec4& v1f,
                                         const Vec2& st1f,
                                         const Vec4& c1f,
                                         const Vec4& v2f,
                                         const Vec2& st2f,
                                         const Vec4& c2f);
    inline static VecInt edgeFunctionFixPoint(const Vec2i &a, const Vec2i &b, const Vec2i &c);
    inline static VecInt calcRecip(VecInt val);

    inline static bool rasterizeFloat(RasterizedTriangle &rasterizedTriangle,
                                      const Vec4 &v0f,
                                      const Vec2 &st0f,
                                      const Vec4 &v1f,
                                      const Vec2 &st1f,
                                      const Vec4 &v2f,
                                      const Vec2 &st2f);


};

#endif // RASTERIZER_HPP

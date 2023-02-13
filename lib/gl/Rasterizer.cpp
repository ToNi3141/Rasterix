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

#include "Rasterizer.hpp"
#include <cstring>

// The Arduino IDE will produce compile errors when using std::min and std::max
#include <algorithm>    // std::max
#define max std::max
#define min std::min

namespace rr
{
Rasterizer::Rasterizer()
{
}


bool Rasterizer::rasterize(TriangleStreamDesc& desc, const IRenderer::Triangle& triangle)
{
   return rasterizeFixPoint(desc, triangle);
}

bool Rasterizer::checkIfTriangleIsInBounds(TriangleStreamDesc &desc,
                                           const uint16_t lineStart,
                                           const uint16_t lineEnd)
{
    // Check if the triangle is in the current area by checking if the end position is below the start line
    // and if the start of the triangle is within this area
    if ((desc.getDesc().bbEndY >= lineStart) &&
            (desc.getDesc().bbStartY < lineEnd))
    {
        return true;
    }
    return false;
}


VecInt Rasterizer::calcRecip(VecInt val)
{
    // Assume DECIMAL_POINT is 12.
    int64_t div = (1ll << (DECIMAL_POINT * 4)) / val; // Sn48 / Sn4 = Sn44
    return div >> 20; // Sn44 >> 20 = Sn24
}

VecInt Rasterizer::edgeFunctionFixPoint(const Vec2i &a, const Vec2i &b, const Vec2i &c)
{
    VecInt val1 = (c[0] - a[0]) * (b[1] - a[1]);
    VecInt val2 = (c[1] - a[1]) * (b[0] - a[0]);
    VecInt ges = val1 - val2;
    return ges;
}

bool Rasterizer::rasterizeFixPoint(TriangleStreamDesc& desc, const IRenderer::Triangle& triangle)
{
    static constexpr uint32_t EDGE_FUNC_SIZE = 4;
    static constexpr uint32_t HALF_EDGE_FUNC_SIZE = (1 << (EDGE_FUNC_SIZE-1));

    Vec2i v0, v1, v2;
    v0.fromVec<EDGE_FUNC_SIZE>({ triangle.vertex0[0], triangle.vertex0[1] });
    v1.fromVec<EDGE_FUNC_SIZE>({ triangle.vertex1[0], triangle.vertex1[1] });
    v2.fromVec<EDGE_FUNC_SIZE>({ triangle.vertex2[0], triangle.vertex2[1] });
    
    // Initialize Bounding box
    // Get the bounding box
    int32_t bbStartX;
    int32_t bbStartY;
    int32_t bbEndX;
    int32_t bbEndY;
    bbStartX = min(min(v0[0], v1[0]), v2[0]);
    bbStartY = min(min(v0[1], v1[1]), v2[1]);
    bbEndX = max(max(v0[0], v1[0]), v2[0]);
    bbEndY = max(max(v0[1], v1[1]), v2[1]);

    // Convert to integer values
    bbStartX = (bbStartX + HALF_EDGE_FUNC_SIZE) >> EDGE_FUNC_SIZE;
    bbStartY = (bbStartY + HALF_EDGE_FUNC_SIZE) >> EDGE_FUNC_SIZE;
    bbEndX = (bbEndX + HALF_EDGE_FUNC_SIZE) >> EDGE_FUNC_SIZE;
    bbEndY = (bbEndY + HALF_EDGE_FUNC_SIZE) >> EDGE_FUNC_SIZE;

    ++bbEndX; // Increase the size at the end of the bounding box a bit. It can happen otherwise that triangles is discarded because it was too small
    ++bbEndY;

    if (m_enableScissor)
    {
        bbStartX = max(bbStartX, static_cast<int32_t>(m_scissorX));
        bbStartY = max(bbStartY, static_cast<int32_t>(m_scissorY));
        bbEndX = min(bbEndX, static_cast<int32_t>(m_scissorX + m_scissorWidth));
        bbEndY = min(bbEndY, static_cast<int32_t>(m_scissorY + m_scissorHeight));

        if (bbStartX >= bbEndX)
        {
            return false;
        }
        if (bbStartY >= bbEndY)
        {
            return false;
        }
    }

//    // Clamp against the view port
//    // Should not be needed when the clipping is enabled
//         bbStartX = max(bbStartX, (int32_t)0);
//         bbStartY = max(bbStartY, (int32_t)0);
//         bbEndX = min(bbEndX + 1, 480); // Increase the size at the end of the bounding box a bit. It can happen otherwise that triangles is discarded because it was too small
//         bbEndY = min(bbEndY + 1, 320);
//     // Check if the bounding box has at least a width of one. Otherwise the hardware will stuck.
//        if ((bbEndX - bbStartX) == 0)
//            return false;

    desc.getDesc().bbStartX = bbStartX;
    desc.getDesc().bbStartY = bbStartY;
    desc.getDesc().bbEndX = bbEndX;
    desc.getDesc().bbEndY = bbEndY;

    VecInt area = edgeFunctionFixPoint(v0, v1, v2); // Sn.4

    VecInt sign = -1; // 1 backface culling; -1 frontface culling
    sign = (area <= 0) ? -1 : 1; // No culling
    area *= sign;
    if (area <= 0x0)
        return false;

    // Interpolate triangle
    Vec2i p = {{(bbStartX << EDGE_FUNC_SIZE), bbStartY << EDGE_FUNC_SIZE}};
    Vec3i& wi = desc.getDesc().wInit; // Sn.4
    Vec3i& wIncX = desc.getDesc().wXInc;
    Vec3i& wIncY = desc.getDesc().wYInc;
    wi[0] = edgeFunctionFixPoint(v1, v2, p);
    wi[1] = edgeFunctionFixPoint(v2, v0, p);
    wi[2] = edgeFunctionFixPoint(v0, v1, p);
    wi *= sign;
    Vec2i pw = {{(bbStartX << EDGE_FUNC_SIZE) + ((1)<< EDGE_FUNC_SIZE), bbStartY << EDGE_FUNC_SIZE}};
    wIncX[0] = edgeFunctionFixPoint(v1, v2, pw);
    wIncX[1] = edgeFunctionFixPoint(v2, v0, pw);
    wIncX[2] = edgeFunctionFixPoint(v0, v1, pw);
    wIncX *= sign;
    wIncX -= wi;
    Vec2i ph = {{bbStartX << EDGE_FUNC_SIZE, (bbStartY << EDGE_FUNC_SIZE) + ((1) << EDGE_FUNC_SIZE)}};
    wIncY[0] = edgeFunctionFixPoint(v1, v2, ph);
    wIncY[1] = edgeFunctionFixPoint(v2, v0, ph);
    wIncY[2] = edgeFunctionFixPoint(v0, v1, ph);
    wIncY *= sign;
    wIncY -= wi;

    float areaInv = 1.0f / area;

    Vec3 wNorm;
    wNorm.fromArray(&(wi.vec[0]), 3);
    wNorm.mul(areaInv);

    Vec3 wIncXNorm;
    wIncXNorm.fromArray(&(wIncX.vec[0]), 3);
    wIncXNorm.mul(areaInv);

    Vec3 wIncYNorm;
    wIncYNorm.fromArray(&(wIncY.vec[0]), 3);
    wIncYNorm.mul(areaInv);

    // Interpolate texture
    for (uint8_t i = 0; i < desc.getDesc().texture.size(); i++)
    {
        if (m_tmuEnable[i])
        {
            Vec3 texS { { triangle.texture0[i][0][0], triangle.texture1[i][0][0], triangle.texture2[i][0][0] } };
            Vec3 texT { { triangle.texture0[i][0][1], triangle.texture1[i][0][1], triangle.texture2[i][0][1] } };
            Vec3 texQ { { triangle.texture0[i][0][3], triangle.texture1[i][0][3], triangle.texture2[i][0][3] } };

            desc.getDesc().texture[i].texStq[0] = texS.dot(wNorm);
            desc.getDesc().texture[i].texStq[1] = texT.dot(wNorm);
            desc.getDesc().texture[i].texStq[2] = texQ.dot(wNorm);

            desc.getDesc().texture[i].texStqXInc[0] = texS.dot(wIncXNorm);
            desc.getDesc().texture[i].texStqXInc[1] = texT.dot(wIncXNorm);
            desc.getDesc().texture[i].texStqXInc[2] = texQ.dot(wIncXNorm);

            desc.getDesc().texture[i].texStqYInc[0] = texS.dot(wIncYNorm);
            desc.getDesc().texture[i].texStqYInc[1] = texT.dot(wIncYNorm);
            desc.getDesc().texture[i].texStqYInc[2] = texQ.dot(wIncYNorm);
        }
    }

    // Interpolate W
    Vec3 vW { { triangle.vertex0[3], triangle.vertex1[3], triangle.vertex2[3] } };
    desc.getDesc().depthW = vW.dot(wNorm);
    desc.getDesc().depthWXInc = vW.dot(wIncXNorm);
    desc.getDesc().depthWYInc = vW.dot(wIncYNorm);

    // Interpolate Z
    // Using z buffer. Here are two options for the depth buffer:
    // Advantage of a w buffer: All values are equally distributed between 0 and intmax. It seems also to be a better fit for 16bit z buffers
    // Advantage of a z buffer: More precise than the w buffer on near objects. Distribution is therefore uneven. Seems to be a bad choice for 16bit z buffers.
    Vec3 vZ { { triangle.vertex0[2], triangle.vertex1[2], triangle.vertex2[2] } };
    desc.getDesc().depthZ = vZ.dot(wNorm);
    desc.getDesc().depthZXInc = vZ.dot(wIncXNorm);
    desc.getDesc().depthZYInc = vZ.dot(wIncYNorm);

    // Interpolate color
    Vec3 cr { { triangle.color0[0], triangle.color1[0], triangle.color2[0] } };
    Vec3 cg { { triangle.color0[1], triangle.color1[1], triangle.color2[1] } };
    Vec3 cb { { triangle.color0[2], triangle.color1[2], triangle.color2[2] } };
    Vec3 ca { { triangle.color0[3], triangle.color1[3], triangle.color2[3] } };

    desc.getDesc().color[0] = cr.dot(wNorm);
    desc.getDesc().color[1] = cg.dot(wNorm);
    desc.getDesc().color[2] = cb.dot(wNorm);
    desc.getDesc().color[3] = ca.dot(wNorm);

    desc.getDesc().colorXInc[0] = cr.dot(wIncXNorm);
    desc.getDesc().colorXInc[1] = cg.dot(wIncXNorm);
    desc.getDesc().colorXInc[2] = cb.dot(wIncXNorm);
    desc.getDesc().colorXInc[3] = ca.dot(wIncXNorm);

    desc.getDesc().colorYInc[0] = cr.dot(wIncYNorm);
    desc.getDesc().colorYInc[1] = cg.dot(wIncYNorm);
    desc.getDesc().colorYInc[2] = cb.dot(wIncYNorm);
    desc.getDesc().colorYInc[3] = ca.dot(wIncYNorm);

    return true;
}

float Rasterizer::edgeFunctionFloat(const Vec4 &a, const Vec4 &b, const Vec4 &c)
{
    float val1 = (c[0] - a[0]) * (b[1] - a[1]);
    float val2 = (c[1] - a[1]) * (b[0] - a[0]);
    float ges = val1 - val2;
    return ges;
}

void Rasterizer::setScissorBox(const int32_t x, const int32_t y, const uint32_t width, const uint32_t height)
{
    m_scissorX = x;
    m_scissorY = y;
    m_scissorWidth = width;
    m_scissorHeight = height;
}

} // namespace rr
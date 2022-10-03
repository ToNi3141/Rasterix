// Rasterix
// https://github.com/ToNi3141/Rasterix
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

#include "Rasterizer.hpp"
#include <cstring>

// The Arduino IDE will produce compile errors when using std::min and std::max
#include <algorithm>    // std::max
#define max std::max
#define min std::min

Rasterizer::Rasterizer()
{
}


bool Rasterizer::rasterize(RasterizedTriangle& rasterizedTriangle,
                           const Vec4& v0f,
                           const Vec4& st0f,
                           const Vec4& c0f,
                           const Vec4& v1f,
                           const Vec4& st1f,
                           const Vec4& c1f,
                           const Vec4& v2f,
                           const Vec4& st2f,
                           const Vec4& c2f)
{
//        return rasterizeFloat(rasterizedTriangle, v0f, st0f, v1f, st1f, v2f, st2f);
   return rasterizeFixPoint(rasterizedTriangle, v0f, st0f, c0f, v1f, st1f, c1f, v2f, st2f, c2f);
}

bool Rasterizer::calcLineIncrement(RasterizedTriangle &incrementedTriangle,
                                   const RasterizedTriangle &triangleToIncrement,
                                   const uint16_t lineStart,
                                   const uint16_t lineEnd)
{
    if ((lineStart == 0) && (triangleToIncrement.bbStartY < lineEnd))
    {
        // Handle the first case in a special manner, because this case is really fast to calculate. Just check
        // for the bounding box and if the triangle is in this bounding box, just render it
        memcpy(&incrementedTriangle, &triangleToIncrement, sizeof(incrementedTriangle));
        return true;
    }
    else
    {
        // Check if the triangle is in the current area by checking if the end position is below the start line
        // and if the start of the triangle is within this area
        if ((triangleToIncrement.bbEndY >= lineStart) &&
                (triangleToIncrement.bbStartY < lineEnd))
        {
            // Copy entries one by one. It is more efficient for the MCU than a copy constructor or a memcopy.
            // It has a big impact on the performance
            incrementedTriangle.bbStartX = triangleToIncrement.bbStartX;
            incrementedTriangle.bbStartY = triangleToIncrement.bbStartY;
            incrementedTriangle.bbEndX = triangleToIncrement.bbEndX;
            incrementedTriangle.bbEndY = triangleToIncrement.bbEndY;
            incrementedTriangle.wXInc = triangleToIncrement.wXInc;
            incrementedTriangle.wYInc = triangleToIncrement.wYInc;
            incrementedTriangle.texStqXInc = triangleToIncrement.texStqXInc;
            incrementedTriangle.texStqYInc = triangleToIncrement.texStqYInc;
            incrementedTriangle.depthWXInc = triangleToIncrement.depthWXInc;
            incrementedTriangle.depthWYInc = triangleToIncrement.depthWYInc;
            incrementedTriangle.depthZXInc = triangleToIncrement.depthZXInc;
            incrementedTriangle.depthZYInc = triangleToIncrement.depthZYInc;
            incrementedTriangle.colorXInc = triangleToIncrement.colorXInc;
            incrementedTriangle.colorYInc = triangleToIncrement.colorYInc;


            // The triangle is within the current display area
            // Check if the trinagle started in the previous area. If so, we have to move the interpolation factors
            // to the current area
            if (incrementedTriangle.bbStartY < lineStart)
            {
                incrementedTriangle.depthW = triangleToIncrement.depthW;

                incrementedTriangle.depthZ = triangleToIncrement.depthZ;

                const int32_t bbDiff = lineStart - incrementedTriangle.bbStartY;
                incrementedTriangle.bbStartY = 0;
                incrementedTriangle.bbEndY -= lineStart;

                incrementedTriangle.wInit = incrementedTriangle.wYInc;
                incrementedTriangle.wInit *= bbDiff;
                incrementedTriangle.wInit += triangleToIncrement.wInit;


                incrementedTriangle.texStq = incrementedTriangle.texStqYInc;
                incrementedTriangle.texStq *= bbDiff;
                incrementedTriangle.texStq += triangleToIncrement.texStq;

                incrementedTriangle.depthW += incrementedTriangle.depthWYInc * bbDiff;

                incrementedTriangle.depthZ += incrementedTriangle.depthZYInc * bbDiff;

                incrementedTriangle.color = incrementedTriangle.colorYInc;
                incrementedTriangle.color *= bbDiff;
                incrementedTriangle.color += triangleToIncrement.color;
            }
            // The triangle starts in this area. So we just have to readjust the bounding box
            else
            {
                incrementedTriangle.bbStartY -= lineStart;
                incrementedTriangle.bbEndY -= lineStart;
                incrementedTriangle.wInit = triangleToIncrement.wInit;
                incrementedTriangle.texStq = triangleToIncrement.texStq;
                incrementedTriangle.depthW = triangleToIncrement.depthW;
                incrementedTriangle.depthZ = triangleToIncrement.depthZ;
                incrementedTriangle.color = triangleToIncrement.color;
            }

            return true;
        }
    }
    return false;
}

bool Rasterizer::checkIfTriangleIsInBounds(Rasterizer::RasterizedTriangle &triangle,
                                           const uint16_t lineStart,
                                           const uint16_t lineEnd)
{
    // Check if the triangle is in the current area by checking if the end position is below the start line
    // and if the start of the triangle is within this area
    if ((triangle.bbEndY >= lineStart) &&
            (triangle.bbStartY < lineEnd))
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

bool Rasterizer::rasterizeFixPoint(RasterizedTriangle& rasterizedTriangle,
                                   const Vec4& v0f,
                                   const Vec4& tc0f,
                                   const Vec4& c0f,
                                   const Vec4& v1f,
                                   const Vec4& tc1f,
                                   const Vec4& c1f,
                                   const Vec4& v2f,
                                   const Vec4& tc2f,
                                   const Vec4& c2f)
{
    static constexpr uint32_t EDGE_FUNC_SIZE = 4;
    static constexpr uint32_t HALF_EDGE_FUNC_SIZE = (1 << (EDGE_FUNC_SIZE-1));

    Vec2i v0, v1, v2;
    v0.fromVec<EDGE_FUNC_SIZE>({v0f[0], v0f[1]});
    v1.fromVec<EDGE_FUNC_SIZE>({v1f[0], v1f[1]});
    v2.fromVec<EDGE_FUNC_SIZE>({v2f[0], v2f[1]});

    Vec3 texS {{tc0f[0], tc1f[0], tc2f[0]}};
    Vec3 texT {{tc0f[1], tc1f[1], tc2f[1]}};
    Vec3 texQ {{tc0f[3], tc1f[3], tc2f[3]}};

    // Using z buffer. Here are two options for the depth buffer:
    // Advantage of a w buffer: All values are equally distributed between 0 and intmax. It seems also to be a better fit for 16bit z buffers
    // Advantage of a z buffer: More precise than the w buffer on near objects. Distribution is therefore uneven. Seems to be a bad choice for 16bit z buffers.
    Vec3 vW {{v0f[3], v1f[3], v2f[3]}};
    Vec3 vZ {{v0f[2], v1f[2], v2f[2]}};

    Vec3 cr {{c0f[0], c1f[0], c2f[0]}};
    Vec3 cg {{c0f[1], c1f[1], c2f[1]}};
    Vec3 cb {{c0f[2], c1f[2], c2f[2]}};
    Vec3 ca {{c0f[3], c1f[3], c2f[3]}};
    
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
    ++bbEndX; // Increase the size at the end of the bounding box a bit. It can happen otherwise that triangles is discarded because it was too small
    ++bbEndY;


    rasterizedTriangle.bbStartX = bbStartX;
    rasterizedTriangle.bbStartY = bbStartY;
    rasterizedTriangle.bbEndX = bbEndX;
    rasterizedTriangle.bbEndY = bbEndY;

    VecInt area = edgeFunctionFixPoint(v0, v1, v2); // Sn.4

    VecInt sign = -1; // 1 backface culling; -1 frontface culling
    sign = (area <= 0) ? -1 : 1; // No culling
    area *= sign;
    if (area <= 0x0)
        return false;

    // Interpolate triangle
    Vec2i p = {{(bbStartX << EDGE_FUNC_SIZE), bbStartY << EDGE_FUNC_SIZE}};
    Vec3i& wi = rasterizedTriangle.wInit; // Sn.4
    Vec3i& wIncX = rasterizedTriangle.wXInc;
    Vec3i& wIncY = rasterizedTriangle.wYInc;
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
    rasterizedTriangle.texStq[0] = texS.dot(wNorm);
    rasterizedTriangle.texStq[1] = texT.dot(wNorm);
    rasterizedTriangle.texStq[2] = texQ.dot(wNorm);

    rasterizedTriangle.texStqXInc [0] = texS.dot(wIncXNorm);
    rasterizedTriangle.texStqXInc [1] = texT.dot(wIncXNorm);
    rasterizedTriangle.texStqXInc[2] = texQ.dot(wIncXNorm);

    rasterizedTriangle.texStqYInc[0] = texS.dot(wIncYNorm);
    rasterizedTriangle.texStqYInc[1] = texT.dot(wIncYNorm);
    rasterizedTriangle.texStqYInc[2] = texQ.dot(wIncYNorm);

    // Interpolate W
    rasterizedTriangle.depthW = vW.dot(wNorm);
    rasterizedTriangle.depthWXInc = vW.dot(wIncXNorm);
    rasterizedTriangle.depthWYInc = vW.dot(wIncYNorm);

    // Interpolate Z
    rasterizedTriangle.depthZ = vZ.dot(wNorm);
    rasterizedTriangle.depthZXInc = vZ.dot(wIncXNorm);
    rasterizedTriangle.depthZYInc = vZ.dot(wIncYNorm);

    // Interpolate color
    rasterizedTriangle.color[0] = cr.dot(wNorm);
    rasterizedTriangle.color[1] = cg.dot(wNorm);
    rasterizedTriangle.color[2] = cb.dot(wNorm);
    rasterizedTriangle.color[3] = ca.dot(wNorm);

    rasterizedTriangle.colorXInc[0] = cr.dot(wIncXNorm);
    rasterizedTriangle.colorXInc[1] = cg.dot(wIncXNorm);
    rasterizedTriangle.colorXInc[2] = cb.dot(wIncXNorm);
    rasterizedTriangle.colorXInc[3] = ca.dot(wIncXNorm);

    rasterizedTriangle.colorYInc[0] = cr.dot(wIncYNorm);
    rasterizedTriangle.colorYInc[1] = cg.dot(wIncYNorm);
    rasterizedTriangle.colorYInc[2] = cb.dot(wIncYNorm);
    rasterizedTriangle.colorYInc[3] = ca.dot(wIncYNorm);
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

void Rasterizer::enableScissor(const bool enable)
{
    m_enableScissor = enable;
}

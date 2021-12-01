// RasteriCEr
// https://github.com/ToNi3141/RasteriCEr
// Copyright (c) 2021 ToNi3141

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


bool Rasterizer::rasterize(RasterizedTriangle &rasterizedTriangle,
                           const Vec4 &v0f,
                           const Vec2& st0f,
                           const Vec4 &v1f,
                           const Vec2& st1f,
                           const Vec4 &v2f,
                           const Vec2& st2f)
{
//        return rasterizeFloat(rasterizedTriangle, v0f, st0f, v1f, st1f, v2f, st2f);
   return rasterizeFixPoint(rasterizedTriangle, v0f, st0f, v1f, st1f, v2f, st2f);
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
            incrementedTriangle.triangleConfiguration = triangleToIncrement.triangleConfiguration;
            incrementedTriangle.triangleStaticColor = triangleToIncrement.triangleStaticColor;
            incrementedTriangle.bbStartX = triangleToIncrement.bbStartX;
            incrementedTriangle.bbStartY = triangleToIncrement.bbStartY;
            incrementedTriangle.bbEndX = triangleToIncrement.bbEndX;
            incrementedTriangle.bbEndY = triangleToIncrement.bbEndY;
            incrementedTriangle.wXInc = triangleToIncrement.wXInc;
            incrementedTriangle.wYInc = triangleToIncrement.wYInc;
            incrementedTriangle.texStXInc = triangleToIncrement.texStXInc;
            incrementedTriangle.texStYInc = triangleToIncrement.texStYInc;
            incrementedTriangle.depthWXInc = triangleToIncrement.depthWXInc;
            incrementedTriangle.depthWYInc = triangleToIncrement.depthWYInc;


            // The triangle is within the current display area
            // Check if the trinagle started in the previous area. If so, we have to move the interpolation factors
            // to the current area
            if (incrementedTriangle.bbStartY < lineStart)
            {
               incrementedTriangle.depthW = triangleToIncrement.depthW;

                const int32_t bbDiff = lineStart - incrementedTriangle.bbStartY;
                incrementedTriangle.bbStartY = 0;
                incrementedTriangle.bbEndY -= lineStart;

                incrementedTriangle.wInit = incrementedTriangle.wYInc;
                incrementedTriangle.wInit *= bbDiff;
                incrementedTriangle.wInit += triangleToIncrement.wInit;


                incrementedTriangle.texSt = incrementedTriangle.texStYInc;
                incrementedTriangle.texSt *= bbDiff;
                incrementedTriangle.texSt += triangleToIncrement.texSt;

                incrementedTriangle.depthW += incrementedTriangle.depthWYInc * bbDiff;
            }
            // The triangle starts in this area. So we just have to readjust the bounding box
            else
            {
                incrementedTriangle.bbStartY -= lineStart;
                incrementedTriangle.bbEndY -= lineStart;
                incrementedTriangle.wInit = triangleToIncrement.wInit;
                incrementedTriangle.texSt = triangleToIncrement.texSt;
                incrementedTriangle.depthW = triangleToIncrement.depthW;
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

bool Rasterizer::rasterizeFixPoint(RasterizedTriangle &rasterizedTriangle,
                                   const Vec4 &v0f,
                                   const Vec2& st0f,
                                   const Vec4 &v1f,
                                   const Vec2& st1f,
                                   const Vec4 &v2f,
                                   const Vec2& st2f)
{
    static constexpr uint32_t EDGE_FUNC_SIZE = 2;
    static constexpr uint32_t HALF_EDGE_FUNC_SIZE = (1 << (EDGE_FUNC_SIZE-1));

    // Convert to a fixed point representation
    // Use here 4 bits for the integer part. That offers the possibility to use texture coordinates which have a maximum value of 16.
    // This is required for repetitions. On the hardware, overflows are not a big deal during repeats but if we want to clamp, then an overflow
    // destroyes the clamp. Keep in mind that the end result is still a s1.30 number which is send to the hardware.
//    Vec3i stx, sty;
//    stx.fromVec<27>({st0f[0], st1f[0], st2f[0]});
//    sty.fromVec<27>({st0f[1], st1f[1], st2f[1]});

    Vec2i v0, v1, v2;
    v0.fromVec<EDGE_FUNC_SIZE>({v0f[0], v0f[1]});
    v1.fromVec<EDGE_FUNC_SIZE>({v1f[0], v1f[1]});
    v2.fromVec<EDGE_FUNC_SIZE>({v2f[0], v2f[1]});

    Vec3 stx = {{st0f[0], st1f[0], st2f[0]}};
    Vec3 sty = {{st0f[1], st1f[1], st2f[1]}};
    // TODO / Note: Using the w component for the z buffer, which converts the z buffer to a w buffer.
    // Advantage of a w buffer: All values are equally distributed between 0 and intmax. It seems also to be a better fit for 16bit z buffers
    // Advantage of a z buffer: More precise than the w buffer on near objects. Distribution is therefore uneven. Seems to be a bad choice for 16bit z buffers.
    Vec3 vW = {{v0f[3], v1f[3], v2f[3]}};
    
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

    // Clamp against the view port
    // Should not be needed when the clipping is enabled
    //     bbStartX = max(bbStartX, (int32_t)0);
    //     bbStartY = max(bbStartY, (int32_t)0);
    //     bbEndX = min(bbEndX + 1, 480); // Increase the size at the end of the bounding box a bit. It can happen otherwise that triangles is discarded because it was too small
    //     bbEndY = min(bbEndY + 1, 272);
    // Check if the bounding box has at least a width of one. Otherwise the hardware will stuck.
    //    if ((bbEndX - bbStartX) == 0)
    //        return false;
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

    // Calc perspective correction
    // For the attribute calculation, always use the w component. The w component at this point is already the reciprocal, so just multiply
#ifndef NO_PERSP_CORRECT
    stx.mul(vW);
    sty.mul(vW);
#endif

    // Interpolate texture
    Vec2 sti;
    sti[0] = stx.dot(wNorm);
    sti[1] = sty.dot(wNorm);
    Vec2 stw;
    stw[0] = stx.dot(wIncXNorm);
    stw[1] = sty.dot(wIncXNorm);
    Vec2 sth;
    sth[0] = stx.dot(wIncYNorm);
    sth[1] = sty.dot(wIncYNorm);

    // Interpolate W
    float wDepthInit = vW.dot(wNorm);
    float wDepthIncX = vW.dot(wIncXNorm);
    float wDepthIncY = vW.dot(wIncYNorm);

#ifdef SOFTWARE_RENDERER
    // During the H increment, only increment as much, that after the H increment, we are at the beginning of the bounding box
    // That means, if we have a full line increment, we would be after the H increment again at the end of the bounding box
    // With this calculation, we substracting from the H increment the width of the bounding box. That causes, that we start
    // at the beginning of the bounding box

    // Use this when no edge walking algorihm is implemented
    VecInt bbDiff = bbEndX - bbStartX;
    wIncY[0] = wIncY[0] - (wIncX[0] * bbDiff);
    wIncY[1] = wIncY[1] - (wIncX[1] * bbDiff);
    wIncY[2] = wIncY[2] - (wIncX[2] * bbDiff);
    sth[0] = sth[0] - (stw[0] * bbDiff);
    sth[1] = sth[1] - (stw[1] * bbDiff);
    wDepthIncY  = wDepthIncY  - (wDepthIncX  * bbDiff);
#endif
    rasterizedTriangle.texSt = sti;
    rasterizedTriangle.texStXInc = stw;
    rasterizedTriangle.texStYInc = sth;
    rasterizedTriangle.depthW = wDepthInit;
    rasterizedTriangle.depthWXInc = wDepthIncX;
    rasterizedTriangle.depthWYInc = wDepthIncY;
    return true;
}

float Rasterizer::edgeFunctionFloat(const Vec4 &a, const Vec4 &b, const Vec4 &c)
{
    float val1 = (c[0] - a[0]) * (b[1] - a[1]);
    float val2 = (c[1] - a[1]) * (b[0] - a[0]);
    float ges = val1 - val2;
    return ges;
}

bool Rasterizer::rasterizeFloat(RasterizedTriangle &rasterizedTriangle,
                                const Vec4 &v0,
                                const Vec2& st0,
                                const Vec4 &v1,
                                const Vec2& st1,
                                const Vec4 &v2,
                                const Vec2& st2)
{
    Vec3 stx = {{st0[0], st1[0], st2[0]}};
    Vec3 sty = {{st0[1], st1[1], st2[1]}};
    // TODO / Note: Using the w component for the z buffer, which converts the z buffer to a w buffer.
    // Advantage of a w buffer: All values are equally distributed between 0 and intmax. It seems also to be a better fit for 16bit z buffers
    // Advantage of a z buffer: More precise than the w buffer on near objects. Distribution is therefore uneven. Seems to be a bad choice for 16bit z buffers.
    Vec3 vW = {{v0[3], v1[3], v2[3]}};

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

    // Clamp against the view port
    // Should not be needed when the clipping is enabled
    // bbStartX = max(bbStartX, (int32_t)0);
    // bbStartY = max(bbStartY, (int32_t)0);
    // bbEndX = min(bbEndX + 1, resolutionX); // Increase the size at the end of the bounding box a bit. It can happen otherwise that triangles is discarded because it was too small
    // bbEndY = min(bbEndY + 1, resolutionY);
    // Check if the bounding box has at least a width of one. Otherwise the hardware will stuck.
    //    if ((bbEndX - bbStartX) == 0)
    //        return false;
    bbEndX += 1; // Increase the size at the end of the bounding box a bit. It can happen otherwise that triangles is discarded because it was too small
    bbEndY += 1;

    rasterizedTriangle.bbStartX = bbStartX;
    rasterizedTriangle.bbStartY = bbStartY;
    rasterizedTriangle.bbEndX = bbEndX;
    rasterizedTriangle.bbEndY = bbEndY;

    float area = edgeFunctionFloat(v0, v1, v2);

    float sign = -1.0f; // 1 backface culling; -1 frontface culling
    sign = (area <= 0) ? -1.0f : 1.0f; // No culling
    area *= sign;

    if (area <= 0.0f)
        return false;

    // Interpolate triangle
    Vec4 p = {{static_cast<float>(bbStartX), static_cast<float>(bbStartY), 0.0f, 0.0f}};
    Vec3 wi; // Sn.12
    Vec3 wIncX;
    Vec3 wIncY;
    wi[0] = edgeFunctionFloat(v1, v2, p);
    wi[1] = edgeFunctionFloat(v2, v0, p);
    wi[2] = edgeFunctionFloat(v0, v1, p);
    wi *= sign;
    Vec4 pw = {{static_cast<float>(bbStartX) + 1.0f, static_cast<float>(bbStartY), 0.0f, 0.0f}};
    wIncX[0] = edgeFunctionFloat(v1, v2, pw);
    wIncX[1] = edgeFunctionFloat(v2, v0, pw);
    wIncX[2] = edgeFunctionFloat(v0, v1, pw);
    wIncX *= sign;
    wIncX -= wi;
    Vec4 ph = {{static_cast<float>(bbStartX), static_cast<float>(bbStartY) + 1.0f, 0.0f, 0.0f}};
    wIncY[0] = edgeFunctionFloat(v1, v2, ph);
    wIncY[1] = edgeFunctionFloat(v2, v0, ph);
    wIncY[2] = edgeFunctionFloat(v0, v1, ph);
    wIncY *= sign;
    wIncY -= wi;

    float areaInv = 1.0f / area;

    Vec3 wNorm(wi);
    wNorm.mul(areaInv);

    Vec3 wIncXNorm(wIncX);
    wIncXNorm.mul(areaInv);

    Vec3 wIncYNorm(wIncY);
    wIncYNorm.mul(areaInv);

    // Calc perspective correction
    // For the attribute calculation, always use the w component. The w component at this point is already the reciprocal, so just multiply
#ifndef NO_PERSP_CORRECT
    stx.mul(vW);
    sty.mul(vW);
#endif

    // Interpolate texture
    Vec2 sti;
    sti[0] = stx.dot(wNorm);
    sti[1] = sty.dot(wNorm);
    Vec2 stw;
    stw[0] = stx.dot(wIncXNorm);
    stw[1] = sty.dot(wIncXNorm);
    Vec2 sth;
    sth[0] = stx.dot(wIncYNorm);
    sth[1] = sty.dot(wIncYNorm);

    // Interpolate W
    float wDepthInit = vW.dot(wNorm);
    float wDepthIncX = vW.dot(wIncXNorm);
    float wDepthIncY = vW.dot(wIncYNorm);

#ifdef SOFTWARE_RENDERER
    // During the H increment, only increment as much, that after the H increment, we are at the beginning of the bounding box
    // That means, if we have a full line increment, we would be after the H increment again at the end of the bounding box
    // With this calculation, we substracting from the H increment the width of the bounding box. That causes, that we start
    // at the beginning of the bounding box

    // Use this when no edge walking algorihm is implemented
    VecInt bbDiff = bbEndX - bbStartX;
    wIncY[0] = wIncY[0] - (wIncX[0] * bbDiff);
    wIncY[1] = wIncY[1] - (wIncX[1] * bbDiff);
    wIncY[2] = wIncY[2] - (wIncX[2] * bbDiff);
    sth[0] = sth[0] - (stw[0] * bbDiff);
    sth[1] = sth[1] - (stw[1] * bbDiff);
    wDepthIncY  = wDepthIncY  - (wDepthIncX  * bbDiff);
#endif

    rasterizedTriangle.wInit.fromVec<4>({wi[0], wi[1], wi[2]});
    rasterizedTriangle.wXInc.fromVec<4>({wIncX[0], wIncX[1], wIncX[2]});
    rasterizedTriangle.wYInc.fromVec<4>({wIncY[0], wIncY[1], wIncY[2]});

    rasterizedTriangle.texSt = sti;
    rasterizedTriangle.texStXInc = stw;
    rasterizedTriangle.texStYInc = sth;
    rasterizedTriangle.depthW = wDepthInit;
    rasterizedTriangle.depthWXInc = wDepthIncX;
    rasterizedTriangle.depthWYInc = wDepthIncY;

    return true;
}

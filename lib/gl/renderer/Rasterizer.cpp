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

#include <algorithm> // std::max

namespace rr
{

bool Rasterizer::increment(TriangleStreamTypes::TriangleDesc& desc,
    const std::size_t lineStart,
    const std::size_t lineEnd)
{
    TriangleStreamTypes::StaticParams& params = desc.param;
    if ((lineStart == 0) && (params.bbStartY < lineEnd))
    {
        // If the triangle completely in the current line, then nothing has to be done here.
        return true;
    }
    else
    {
        // Check if the triangle is in the current area by checking if the end position is below the start line
        // and if the start of the triangle is within this area
        if ((params.bbEndY >= lineStart) && (params.bbStartY < lineEnd))
        {
            // The triangle is within the current display area
            // Check if the triangle started in the previous area. If so, we have to move the interpolation factors
            // to the current area
            if (params.bbStartY < lineStart)
            {
                const int32_t bbDiff = lineStart - params.bbStartY;

                const auto wInitTmp = params.wInit;
                params.wInit = params.wYInc;
                params.wInit *= bbDiff;
                params.wInit += wInitTmp;

                params.depthW += params.depthWYInc * bbDiff;
                params.depthZ += params.depthZYInc * bbDiff;

                const auto colorTmp = params.color;
                params.color = params.colorYInc;
                params.color *= bbDiff;
                params.color += colorTmp;

                for (std::size_t i = 0; i < desc.texture.size(); i++)
                {
                    const auto texStqTmp = desc.texture[i].texStq;
                    desc.texture[i].texStq = desc.texture[i].texStqYInc;
                    desc.texture[i].texStq *= bbDiff;
                    desc.texture[i].texStq += texStqTmp;
                }
            }

            return true;
        }
    }
    return false;
}

VecInt Rasterizer::edgeFunctionFixPoint(const Vec2i& a, const Vec2i& b, const Vec2i& c)
{
    VecInt val1 = (c[0] - a[0]) * (b[1] - a[1]);
    VecInt val2 = (c[1] - a[1]) * (b[0] - a[0]);
    VecInt ges = val1 - val2;
    return ges;
}

bool Rasterizer::rasterize(TriangleStreamTypes::TriangleDesc& __restrict desc,
    const TransformedTriangle& triangle) const
{
    TriangleStreamTypes::StaticParams& params = desc.param;
    Vec2i v0 = Vec2i::createFromVec<std::array<float, 2>, EDGE_FUNC_SIZE>({ triangle.vertex0[0], triangle.vertex0[1] });
    Vec2i v1 = Vec2i::createFromVec<std::array<float, 2>, EDGE_FUNC_SIZE>({ triangle.vertex1[0], triangle.vertex1[1] });
    Vec2i v2 = Vec2i::createFromVec<std::array<float, 2>, EDGE_FUNC_SIZE>({ triangle.vertex2[0], triangle.vertex2[1] });

    VecInt area = edgeFunctionFixPoint(v0, v1, v2); // Sn.4
    VecInt sign = -1; // 1 backface culling; -1 frontface culling
    sign = (area <= 0) ? -1 : 1; // No culling
    area *= sign;
    if (area <= 0x0)
    {
        return false;
    }

    // Initialize Bounding box
    // Get the bounding box
    int32_t bbStartX = std::min(std::min(v0[0], v1[0]), v2[0]);
    int32_t bbStartY = std::min(std::min(v0[1], v1[1]), v2[1]);
    int32_t bbEndX = std::max(std::max(v0[0], v1[0]), v2[0]);
    int32_t bbEndY = std::max(std::max(v0[1], v1[1]), v2[1]);

    // Convert to integer values
    bbStartX = bbStartX + EDGE_FUNC_ZERO_P_FIVE;
    bbStartY = bbStartY + EDGE_FUNC_ZERO_P_FIVE;
    bbEndX = bbEndX + EDGE_FUNC_ONE_P_ZERO + EDGE_FUNC_ZERO_P_FIVE;
    bbEndY = bbEndY + EDGE_FUNC_ONE_P_ZERO + EDGE_FUNC_ZERO_P_FIVE;

    params.bbStartX = bbStartX >> EDGE_FUNC_SIZE;
    params.bbStartY = bbStartY >> EDGE_FUNC_SIZE;
    params.bbEndX = bbEndX >> EDGE_FUNC_SIZE;
    params.bbEndY = bbEndY >> EDGE_FUNC_SIZE;

    if (m_enableScissor)
    {
        bbStartX = std::max(bbStartX, m_scissorStartX);
        bbStartY = std::max(bbStartY, m_scissorStartY);
        bbEndX = std::min(bbEndX, m_scissorEndX);
        bbEndY = std::min(bbEndY, m_scissorEndY);

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
    //         bbStartX = std::max(bbStartX, (int32_t)0);
    //         bbStartY = std::max(bbStartY, (int32_t)0);
    //         bbEndX = std::min(bbEndX + 1, 480); // Increase the size at the end of the bounding box a bit. It can happen otherwise that triangles is discarded because it was too small
    //         bbEndY = std::min(bbEndY + 1, 320);
    //     // Check if the bounding box has at least a width of one. Otherwise the hardware will stuck.
    //        if ((bbEndX - bbStartX) == 0)
    //            return false;

    bbStartX = params.bbStartX << EDGE_FUNC_SIZE;
    bbStartY = params.bbStartY << EDGE_FUNC_SIZE;

    // Interpolate triangle
    Vec2i p = { bbStartX, bbStartY };
    Vec3i& wi = params.wInit; // Sn.4
    Vec3i& wIncX = params.wXInc;
    Vec3i& wIncY = params.wYInc;
    wi[0] = edgeFunctionFixPoint(v1, v2, p);
    wi[1] = edgeFunctionFixPoint(v2, v0, p);
    wi[2] = edgeFunctionFixPoint(v0, v1, p);
    wi *= sign;
    Vec2i pw = { bbStartX + EDGE_FUNC_ONE_P_ZERO, bbStartY };
    wIncX[0] = edgeFunctionFixPoint(v1, v2, pw);
    wIncX[1] = edgeFunctionFixPoint(v2, v0, pw);
    wIncX[2] = edgeFunctionFixPoint(v0, v1, pw);
    wIncX *= sign;
    wIncX -= wi;
    Vec2i ph = { bbStartX, bbStartY + EDGE_FUNC_ONE_P_ZERO };
    wIncY[0] = edgeFunctionFixPoint(v1, v2, ph);
    wIncY[1] = edgeFunctionFixPoint(v2, v0, ph);
    wIncY[2] = edgeFunctionFixPoint(v0, v1, ph);
    wIncY *= sign;
    wIncY -= wi;

    float areaInv = 1.0f / area;

    Vec3 wNorm = { static_cast<float>(wi[0]), static_cast<float>(wi[1]), static_cast<float>(wi[2]) };
    wNorm.mul(areaInv);

    Vec3 wIncXNorm = { static_cast<float>(wIncX[0]), static_cast<float>(wIncX[1]), static_cast<float>(wIncX[2]) };
    wIncXNorm.mul(areaInv);

    Vec3 wIncYNorm = { static_cast<float>(wIncY[0]), static_cast<float>(wIncY[1]), static_cast<float>(wIncY[2]) };
    wIncYNorm.mul(areaInv);

    Vec3 w = { triangle.vertex0[3], triangle.vertex1[3], triangle.vertex2[3] };
    // Avoid that the w gets too small/big by normalizing it
    if (m_enableScaling)
    {
        w.normalize();
    }

    // Interpolate texture
    for (std::size_t i = 0; i < desc.texture.size(); i++)
    {
        if (m_tmuEnable[i])
        {
            Vec3 texS { triangle.texture0[i][0], triangle.texture1[i][0], triangle.texture2[i][0] };
            Vec3 texT { triangle.texture0[i][1], triangle.texture1[i][1], triangle.texture2[i][1] };
            Vec3 texQ { triangle.texture0[i][3], triangle.texture1[i][3], triangle.texture2[i][3] };

            // Avoid overflowing the integer part by adding an offset
            if (m_enableScaling)
            {
                const float minS = std::min(texS[0], std::min(texS[1], texS[2]));
                const float minT = std::min(texT[0], std::min(texT[1], texT[2]));
                const float maxS = std::max(texS[0], std::max(texS[1], texS[2]));
                const float maxT = std::max(texT[0], std::max(texT[1], texT[2]));

                if (minS < -4.0f)
                {
                    const float minSG = static_cast<int32_t>(minS);
                    texS -= { minSG, minSG, minSG };
                }
                if (minT < -4.0f)
                {
                    const float minTG = static_cast<int32_t>(minT);
                    texT -= { minTG, minTG, minTG };
                }
                if (maxS > 4.0f)
                {
                    const float maxSG = static_cast<int32_t>(maxS);
                    texS -= { maxSG, maxSG, maxSG };
                }
                if (maxT > 4.0f)
                {
                    const float maxTG = static_cast<int32_t>(maxT);
                    texT -= { maxTG, maxTG, maxTG };
                }
            }

            // Perspective correction
            texQ.mul(w);
            texS.mul(w);
            texT.mul(w);

            TriangleStreamTypes::Texture& t = desc.texture[i];

            t.texStq[0] = texS.dot(wNorm);
            t.texStq[1] = texT.dot(wNorm);
            t.texStq[2] = texQ.dot(wNorm);

            t.texStqXInc[0] = texS.dot(wIncXNorm);
            t.texStqXInc[1] = texT.dot(wIncXNorm);
            t.texStqXInc[2] = texQ.dot(wIncXNorm);

            t.texStqYInc[0] = texS.dot(wIncYNorm);
            t.texStqYInc[1] = texT.dot(wIncYNorm);
            t.texStqYInc[2] = texQ.dot(wIncYNorm);
        }
    }

    // Interpolate W
    Vec3 vW { triangle.vertex0[3], triangle.vertex1[3], triangle.vertex2[3] };
    params.depthW = vW.dot(wNorm);
    params.depthWXInc = vW.dot(wIncXNorm);
    params.depthWYInc = vW.dot(wIncYNorm);

    // Interpolate Z
    // Using z buffer. Here are two options for the depth buffer:
    // Advantage of a w buffer: All values are equally distributed between 0 and intmax. It seems also to be a better fit for 16bit z buffers
    // Advantage of a z buffer: More precise than the w buffer on near objects. Distribution is therefore uneven. Seems to be a bad choice for 16bit z buffers.
    Vec3 vZ { triangle.vertex0[2], triangle.vertex1[2], triangle.vertex2[2] };
    params.depthZ = vZ.dot(wNorm);
    params.depthZXInc = vZ.dot(wIncXNorm);
    params.depthZYInc = vZ.dot(wIncYNorm);

    // Interpolate color
    Vec3 cr { triangle.color0[0], triangle.color1[0], triangle.color2[0] };
    Vec3 cg { triangle.color0[1], triangle.color1[1], triangle.color2[1] };
    Vec3 cb { triangle.color0[2], triangle.color1[2], triangle.color2[2] };
    Vec3 ca { triangle.color0[3], triangle.color1[3], triangle.color2[3] };

    params.color[0] = cr.dot(wNorm);
    params.color[1] = cg.dot(wNorm);
    params.color[2] = cb.dot(wNorm);
    params.color[3] = ca.dot(wNorm);

    params.colorXInc[0] = cr.dot(wIncXNorm);
    params.colorXInc[1] = cg.dot(wIncXNorm);
    params.colorXInc[2] = cb.dot(wIncXNorm);
    params.colorXInc[3] = ca.dot(wIncXNorm);

    params.colorYInc[0] = cr.dot(wIncYNorm);
    params.colorYInc[1] = cg.dot(wIncYNorm);
    params.colorYInc[2] = cb.dot(wIncYNorm);
    params.colorYInc[3] = ca.dot(wIncYNorm);

    return true;
}

float Rasterizer::edgeFunctionFloat(const Vec4& a, const Vec4& b, const Vec4& c)
{
    float val1 = (c[0] - a[0]) * (b[1] - a[1]);
    float val2 = (c[1] - a[1]) * (b[0] - a[0]);
    float ges = val1 - val2;
    return ges;
}

void Rasterizer::setScissorBox(const int32_t x, const int32_t y, const uint32_t width, const uint32_t height)
{
    m_scissorStartX = x << EDGE_FUNC_SIZE;
    m_scissorStartY = y << EDGE_FUNC_SIZE;
    m_scissorEndX = (width << EDGE_FUNC_SIZE) + m_scissorStartX;
    m_scissorEndY = (height << EDGE_FUNC_SIZE) + m_scissorStartY;
}

} // namespace rr
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

#include "VertexPipeline.hpp"
#include "Vec.hpp"
#include "Veci.hpp"
#include <string.h>
#include <stdlib.h>
#include "Rasterizer.hpp"
#include <cmath>
#include <spdlog/spdlog.h>

// The Arduino IDE will produce compile errors when using std::min and std::max
#include <algorithm>    // std::max
#define max std::max
#define min std::min

#define __glPi 3.14159265358979323846f

namespace rr
{
VertexPipeline::VertexPipeline(PixelPipeline& renderer)
    : m_renderer(renderer)
{
    m_t.identity();
    m_m.identity();
    m_p.identity();
    m_n.identity();
    for (auto& mat : m_tm)
    {
        mat.identity();
    }
    m_c.identity();
}

void VertexPipeline::getTransformed(Vec4& vertex, Vec4& color, std::array<Vec4, IRenderer::MAX_TMU_COUNT>& tex, const RenderObj &obj, const uint32_t index)
{
    const uint32_t pos = obj.getIndex(index);

    Vec4 v;
    Vec4 c;
    obj.getColor(c, pos);
    // TODO: Check if this required? The standard requires but is it really used?
    //m_c[j].transform(c, c); // Calculate this in one batch to improve performance

    if (obj.vertexArrayEnabled())
    {
        obj.getVertex(v, pos);
        m_t.transform(vertex, v);
    }

    for (uint8_t tu = 0; tu < IRenderer::MAX_TMU_COUNT; tu++)
    {
        if (m_renderer.getEnableTmu(tu))
        {
            obj.getTexCoord(tu, tex[tu], pos);
            m_texGen[tu].calculateTexGenCoords(m_m, tex[tu], v);
            m_tm[tu].transform(tex[tu], tex[tu]); 
        }
    }

    if (m_lighting.lightingEnabled())
    {
        Vec4 vl;
        Vec3 nl;
        Vec3 n;

        obj.getNormal(n, pos);
        m_n.transform(nl, n);

        if (m_enableNormalizing)
        {
            nl.normalize();
        }

        if (obj.vertexArrayEnabled())
        {
            m_m.transform(vl, v);
        }

        m_lighting.calculateLights(color, c, vl, nl);
    }
    else
    {
        color = c;
    }
}

bool VertexPipeline::drawObj(const RenderObj &obj)
{
    recalculateMatrices();
    if (!m_renderer.updatePipeline()) 
    {
        SPDLOG_ERROR("drawObj(): Cannot update pixel pipeline");
        return false;
    }
    obj.logCurrentConfig();
    const bool lines = obj.isLine();
    for (uint32_t it = 0; it < obj.getCount(); it += VERTEX_BUFFER_SIZE)
    {
        const std::size_t diff = obj.getCount() - it;
        // Add a small overlap. This is convenient when rendering the triangle.
        // For instance, if a overlap of 2 is used, then it is possible to draw
        // the complete triangle and don't have to handle the edge case on the 
        // boundaries, which would normally happen, when there is no overlap.
        // (if the mode is triangles, and the size of the buffer is 10, then
        // the renderer could draw 3 triangles but has to handle somehow the last
        // vertex, because 11 and 12 are not known yet. with the overlap, vertex
        // 11 and 12 would be available)
        const std::size_t cnt = min(VERTEX_BUFFER_SIZE + VERTEX_OVERLAP, diff);

        Vec4Array transformedVertex;
        Vec4Array transformedColor;
        TexCoordArray transformedTexCoord;
        Vec3Array transformedNormal;
        for (uint32_t i = 0, itCnt = it; i < cnt; i++, itCnt++)
        {
            getTransformed(transformedVertex[i], transformedColor[i], transformedTexCoord[i], obj, itCnt);
        }

        bool ret = false;
        if (lines)
        {
            ret = drawLineArray(
                transformedVertex,
                transformedColor,
                transformedTexCoord,
                cnt,
                obj.getDrawMode(),
                cnt <= VERTEX_BUFFER_SIZE + VERTEX_OVERLAP);
        }
        else
        {
            ret = drawTriangleArray(
                transformedVertex,
                transformedColor,
                transformedTexCoord,
                cnt,
                obj.getDrawMode());
        }
        
        if (!ret)
        {
            return false;
        }
    }
    return true;
}

// bool VertexPipeline::drawObj(const RenderObj &obj)
// {
//     recalculateMatrices();
//     if (!m_renderer.updatePipeline()) 
//     {
//         SPDLOG_ERROR("drawObj(): Cannot update pixel pipeline");
//         return false;
//     }

//     obj.logCurrentConfig();

//     const bool lines = obj.isLine();
//     for (std::size_t it = 0; it < obj.getCount(); it += VERTEX_BUFFER_SIZE)
//     {
//         const std::size_t diff = obj.getCount() - it;
//         // Add a small overlap. This is convenient when rendering the triangle.
//         // For instance, if a overlap of 2 is used, then it is possible to draw
//         // the complete triangle and don't have to handle the edge case on the 
//         // boundaries, which would normally happen, when there is no overlap.
//         // (if the mode is triangles, and the size of the buffer is 10, then
//         // the renderer could draw 3 triangles but has to handle somehow the last
//         // vertex, because 11 and 12 are not known yet. with the overlap, vertex
//         // 11 and 12 would be available)
//         const std::size_t cnt = min(VERTEX_BUFFER_SIZE + VERTEX_OVERLAP, diff);

//         Vec4Array vertex;
//         Vec4Array color;
//         TexCoordArray texCoord;
//         Vec3Array normal;

//         Vec4Array transformedVertex;
//         Vec4Array transformedColor;
//         TexCoordArray transformedTexCoord;
//         Vec3Array transformedNormal;

//         loadVertexData(obj, vertex, color, normal, texCoord, it, cnt);

//         transform(
//             transformedVertex,
//             transformedColor,
//             transformedNormal,
//             transformedTexCoord,
//             obj.vertexArrayEnabled(),
//             obj.colorArrayEnabled(),
//             obj.normalArrayEnabled(),
//             obj.texCoordArrayEnabled(),
//             vertex,
//             color,
//             normal,
//             texCoord,
//             obj.getVertexColor(),
//             cnt
//         );

//         bool ret = false;
//         if (lines)
//         {
//             ret = drawLineArray(
//                 transformedVertex,
//                 transformedColor,
//                 transformedTexCoord,
//                 cnt,
//                 obj.getDrawMode(),
//                 cnt <= VERTEX_BUFFER_SIZE + VERTEX_OVERLAP);
//         }
//         else
//         {
//             ret = drawTriangleArray(
//                 transformedVertex,
//                 transformedColor,
//                 transformedTexCoord,
//                 cnt,
//                 obj.getDrawMode());
//         }
//         if (!ret)
//         {
//             SPDLOG_ERROR("drawObj(): Cannot draw triangle array");
//             return false;
//         }
//     }
//     return true;
// }

// void VertexPipeline::loadVertexData(const RenderObj& obj, Vec4Array& vertex, Vec4Array& color, Vec3Array& normal, TexCoordArray& tex, const std::size_t offset, const std::size_t count)
// {
//     for (uint32_t o = offset, i = 0; i < count; o++, i++)
//     {
//         const uint32_t index = obj.getIndex(o);
//         obj.getColor(color[i], index);
//         if (obj.vertexArrayEnabled())
//         {
//             obj.getVertex(vertex[i], index);
//         }
//         obj.getNormal(normal[i], index);
//         for (uint8_t j = 0; j < IRenderer::MAX_TMU_COUNT; j++)
//         {
//             obj.getTexCoord(j, tex[i][j], index);
//         }
//     }
// }

// void VertexPipeline::transform(
//     Vec4Array& transformedVertex, 
//     Vec4Array& transformedColor, 
//     Vec3Array& transformedNormal, 
//     TexCoordArray& transformedTex, 
//     const bool enableVertexArray,
//     const bool enableColorArray,
//     const bool enableNormalArray,
//     const std::bitset<IRenderer::MAX_TMU_COUNT> enableTexArray,
//     const Vec4Array& vertex, 
//     const Vec4Array& color, 
//     const Vec3Array& normal, 
//     const TexCoordArray& tex,
//     const Vec4& vertexColor,
//     const std::size_t count)
// {
//     if (m_lighting.lightingEnabled())
//     {
//         if (enableVertexArray)
//             m_m.transform(transformedVertex.data(), vertex.data(), count);

//         // TODO: Increase the performance by only transform one normal when enableNormalArray is false.
//         m_n.transform(transformedNormal.data(), normal.data(), count);

//         for (std::size_t i = 0; i < count; i++)
//         {
//             Vec4 c;
//             if (enableColorArray)
//             {
//                 c = color[i];
//             }
//             else
//             {
//                 c = vertexColor;
//             }

//             if (m_enableNormalizing)
//                 transformedNormal[i].normalize();
//             m_lighting.calculateLights(transformedColor[i], c, transformedVertex[i], transformedNormal[i]);
//         }
//     }
//     else
//     {
//         for (std::size_t i = 0; i < count; i++)
//         {
//             if (enableColorArray)
//             {
//                 transformedColor[i] = color[i];
//             }
//             else
//             {
//                 transformedColor[i] = vertexColor;
//             }
//             // TODO: Check if this required? The standard requires but is it really used?
//             //m_c[j].transform(transformedColor[i], transformedColor[i]); // Calculate this in one batch to improve performance
//         }
//     }

//     for (std::size_t i = 0; i < count; i++)
//     {
//         for (uint8_t j = 0; j < IRenderer::MAX_TMU_COUNT; j++)
//         {
//             if (enableTexArray[j])
//             {
//                 transformedTex[i][j] = tex[i][j];
//             }
//             else
//             {
//                 transformedTex[i][j].initHomogeneous();
//             }
//             m_texGen[j].calculateTexGenCoords(m_m, transformedTex[i][j], vertex[i]);
//             m_tm[j].transform(transformedTex[i][j], transformedTex[i][j]); // Calculate this in one batch to improve performance
//         }
//     }

//     if (enableVertexArray)
//         m_t.transform(transformedVertex.data(), vertex.data(), count);
// }

bool VertexPipeline::drawTriangle(const Triangle& triangle)
{
    Clipper::ClipVertList vertList;
    Clipper::ClipTexCoordList texCoordList;
    Clipper::ClipVertList colorList;
    Clipper::ClipVertList vertListBuffer;
    Clipper::ClipTexCoordList texCoordListBuffer;
    Clipper::ClipVertList colorListBuffer;

    vertList[0] = triangle.v0;
    vertList[1] = triangle.v1;
    vertList[2] = triangle.v2;

    for (uint8_t i = 0; i < IRenderer::MAX_TMU_COUNT; i++)
    {
        if (m_renderer.getEnableTmu(i))
        {
            texCoordList[0][i] = triangle.tc0[i];
            texCoordList[1][i] = triangle.tc1[i];
            texCoordList[2][i] = triangle.tc2[i];
        }
    }


    colorList[0] = triangle.color0;
    colorList[1] = triangle.color1;
    colorList[2] = triangle.color2;

    auto [vertListSize, vertListClipped, texCoordListClipped, colorListClipped] = Clipper::clip(vertList, vertListBuffer, texCoordList, texCoordListBuffer, colorList, colorListBuffer);

    // Calculate for every vertex the perspective division and also apply the viewport transformation
    for (uint8_t i = 0; i < vertListSize; i++)
    {
        // Perspective division
        vertListClipped[i].perspectiveDivide();

        for (uint8_t j = 0; j < IRenderer::MAX_TMU_COUNT; j++)
        {
            // Perspective correction of the texture coordinates
            if (m_renderer.getEnableTmu(j))
                texCoordListClipped[i][j].mul(vertListClipped[i][3]); // since w is already divided, just multiply the 1/w to all elements. Saves one division.
            // TODO: Perspective correction of the color 
            // Each texture uses it's own scaled w (basically q*w). Therefore the hardware must 
            // interpolate (q*w) for each texture. w alone is not enough because OpenGL allows to set q coordinate.
        }

        // Viewport transformation of the vertex
        viewportTransform(vertListClipped[i]);
    }

    // Cull triangle
    if (m_enableCulling)
    {
        // Check only one triangle in the clipped list. The triangles are sub divided, but not rotated. So if one triangle is 
        // facing backwards, then all in the clipping list will do this and vice versa.
        const float edgeVal = Rasterizer::edgeFunctionFloat(vertListClipped[0], vertListClipped[1], vertListClipped[2]);
        const Face currentOrientation = (edgeVal <= 0.0f) ? Face::BACK : Face::FRONT;
        if (currentOrientation != m_cullMode)
            return true;
    }
    
    // Render the triangle
    for (uint8_t i = 3; i <= vertListSize; i++)
    {
        // For a triangle we need atleast 3 vertices. Also treat the clipped list from the clipping as a
        // triangle fan where vert zero is always the center of this fan
        static_assert(IRenderer::MAX_TMU_COUNT == 2, "Adapt the following code when more than two TMUs are configured.");
        std::array<const Vec4* const, IRenderer::MAX_TMU_COUNT> texture0 { { &texCoordListClipped[0][0], &texCoordListClipped[0][1] } };
        std::array<const Vec4* const, IRenderer::MAX_TMU_COUNT> texture1 { { &texCoordListClipped[i - 2][0], &texCoordListClipped[i - 2][1] } };
        std::array<const Vec4* const, IRenderer::MAX_TMU_COUNT> texture2 { { &texCoordListClipped[i - 1][0], &texCoordListClipped[i - 1][1] } };
        const bool success = m_renderer.drawTriangle({ 
                vertListClipped[0],
                vertListClipped[i - 2],
                vertListClipped[i - 1],
                { *(texture0.data()), texture0.size() },
                { *(texture1.data()), texture1.size() },
                { *(texture2.data()), texture2.size() },
                colorListClipped[0],
                colorListClipped[i - 2],
                colorListClipped[i - 1] });
        if (!success)
        {
            return false;
        }
    }
    return true;
}

bool VertexPipeline::drawLine(const Line& line) 
{
    // Copied from swGL and adapted.

    // Get the reciprocal viewport scaling factor
    float rcpViewportScaleX = 2.0f / static_cast<float>(m_viewportWidth);
    float rcpViewportScaleY = 2.0f / static_cast<float>(m_viewportHeight);

    // Calculate the lines normal n = normalize(-dx, dy)
    float nx = -((line.v1[1] / line.v1[3]) - (line.v0[1] / line.v0[3]));
    float ny =  ((line.v1[0] / line.v1[3]) - (line.v0[0] / line.v0[3]));
    float rcpLength = 1.0f / std::sqrt(nx * nx + ny * ny);
    nx *= rcpLength;
    ny *= rcpLength;

    // Scale normal according to the width of the line
    float halfLineWidth = m_lineWidth * 0.5f;
    nx *= halfLineWidth;
    ny *= halfLineWidth;

    // Now create the vertices that define two triangles which are used to draw the line
    Vec4 v[] { line.v0, line.v0, line.v1, line.v1 };

    v[0][0] += ( nx * line.v0[3]) * rcpViewportScaleX;
    v[0][1] += ( ny * line.v0[3]) * rcpViewportScaleY;
    v[1][0] += (-nx * line.v0[3]) * rcpViewportScaleX;
    v[1][1] += (-ny * line.v0[3]) * rcpViewportScaleY;

    v[2][0] += ( nx * line.v1[3]) * rcpViewportScaleX;
    v[2][1] += ( ny * line.v1[3]) * rcpViewportScaleY;
    v[3][0] += (-nx * line.v1[3]) * rcpViewportScaleX;
    v[3][1] += (-ny * line.v1[3]) * rcpViewportScaleY;

    if (!drawTriangle({ v[0], v[1], v[2], line.tc0, line.tc0, line.tc1, line.color0, line.color0, line.color1 })) 
    {
        return false;
    }

    if (!drawTriangle({ v[2], v[1], v[3], line.tc1, line.tc0, line.tc1, line.color1, line.color0, line.color1 }))
    {
        return false;
    }

    return true;
}

bool VertexPipeline::drawTriangleArray(
    const Vec4Array& vertex, 
    const Vec4Array& color, 
    const TexCoordArray& tex, 
    const std::size_t count, 
    const RenderObj::DrawMode drawMode)
{
    const uint32_t c = count - 2;
    for (uint32_t i = 0; i < c; )
    {
        uint32_t i0;
        uint32_t i1;
        uint32_t i2;
        switch (drawMode) {
        case RenderObj::DrawMode::TRIANGLES:
            i0 = i;
            i1 = i + 1;
            i2 = i + 2;
            i += 3;
            break;
        case RenderObj::DrawMode::POLYGON:
        case RenderObj::DrawMode::TRIANGLE_FAN:
            i0 = 0; // BUG: 0 is not the first element, it is the first of the current chunk ...
            i1 = i + 1;
            i2 = i + 2;
            i += 1;
            break;
        case RenderObj::DrawMode::TRIANGLE_STRIP:
            if (i & 0x1)
            {
                i0 = i + 1;
                i1 = i;
                i2 = i + 2;
            }
            else
            {
                i0 = i;
                i1 = i + 1;
                i2 = i + 2;
            }
            i += 1;
            break;
        case RenderObj::DrawMode::QUADS:
            if (i & 0x1)
            {
                i0 = i + 1;
                i1 = i - 1;
                i2 = i + 2;
                i += 3;
            }
            else
            {
                i0 = i;
                i1 = i + 1;
                i2 = i + 2;
                i += 1;
            }
            break;
        case RenderObj::DrawMode::QUAD_STRIP:
            if (i & 0x2)
            {
                i0 = i + 1;
                i1 = i;
                i2 = i + 2;
            }
            else
            {
                i0 = i;
                i1 = i + 1;
                i2 = i + 2;
            }
            i += 1;
            break;
        default:
            return false;
        }

        static_assert(IRenderer::MAX_TMU_COUNT == 2, "Adapt the following code when more than two TMUs are configured.");
        if (!drawTriangle({ vertex[i0], vertex[i1], vertex[i2], { tex[i0][0], tex[i0][1] }, { tex[i1][0], tex[i1][1] }, { tex[i2][0], tex[i2][1] }, color[i0], color[i1], color[i2] }))
        {
            return false;
        }
    }
    return true;
}

bool VertexPipeline::drawLineArray(
    const Vec4Array& vertex, 
    const Vec4Array& color, 
    const TexCoordArray& tex, 
    const std::size_t count, 
    const RenderObj::DrawMode drawMode,
    const bool lastRound)
{
    const uint32_t c = count - 1;
    for (uint32_t i = 0; i < c; )
    {
        uint32_t i0;
        uint32_t i1;
        switch (drawMode) {
        case RenderObj::DrawMode::LINES:
            i0 = i;
            i1 = i + 1;
            i += 2;
            break;
        case RenderObj::DrawMode::LINE_LOOP:
            i0 = i;
            i1 = i + 1;
            if (lastRound && (i1 >= count))
                i1 = 0; // BUG: 0 is not the first element, it is the first of the current chunk ...
            i += 1;
            break;
        case RenderObj::DrawMode::LINE_STRIP:
            i0 = i;
            i1 = i + 1;
            i += 1;
            break;
        default:
            return false;
        }

        static_assert(IRenderer::MAX_TMU_COUNT == 2, "Adapt the following code when more than two TMUs are configured.");
        if (!drawLine({ vertex[i0], vertex[i1], { tex[i0][0], tex[i0][1] }, { tex[i1][0], tex[i1][1] }, color[i0], color[i1] }))
        {
            return false;
        }
    }
    return true;
}

void VertexPipeline::viewportTransform(Vec4 &v0, Vec4 &v1, Vec4 &v2)
{
    v0[0] = ((v0[0] + 1.0f) * m_viewportWidth * 0.5f) + m_viewportX;
    v1[0] = ((v1[0] + 1.0f) * m_viewportWidth * 0.5f) + m_viewportX;
    v2[0] = ((v2[0] + 1.0f) * m_viewportWidth * 0.5f) + m_viewportX;

    v0[1] = ((v0[1] + 1.0f) * m_viewportHeight * 0.5f) + m_viewportY;
    v1[1] = ((v1[1] + 1.0f) * m_viewportHeight * 0.5f) + m_viewportY;
    v2[1] = ((v2[1] + 1.0f) * m_viewportHeight * 0.5f) + m_viewportY;

    v0[2] = (((v0[2] + 1.0f) * 0.25f)) * (m_depthRangeZFar - m_depthRangeZNear);
    v1[2] = (((v1[2] + 1.0f) * 0.25f)) * (m_depthRangeZFar - m_depthRangeZNear);
    v2[2] = (((v2[2] + 1.0f) * 0.25f)) * (m_depthRangeZFar - m_depthRangeZNear);

    // This is a possibility just to calculate the real z value but is not needed for the rasterizer
    //    float n = 0.1;
    //    float f = 100;
    //    float z_ndc0 = 2.0 * v0f[2] - 1.0;
    //    v0f[2] = 2.0 * n * f / (f + n - z_ndc0 * (f - n));
    //    float z_ndc1 = 2.0 * v1f[2] - 1.0;
    //    v1f[2] = 2.0 * n * f / (f + n - z_ndc1 * (f - n));
    //    float z_ndc2 = 2.0 * v2f[2] - 1.0;
    //    v2f[2] = 2.0 * n * f / (f + n - z_ndc2 * (f - n));

}

void VertexPipeline::viewportTransform(Vec4 &v)
{
    // v has the range from -1 to 1. When we multiply it, it has a range from -viewPortWidth/2 to viewPortWidth/2
    // With the addition we shift it from -viewPortWidth/2 to 0 + viewPortX
    v[0] = (((v[0] * m_viewportWidthHalf)) + m_viewportXShift);
    v[1] = (((v[1] * m_viewportHeightHalf)) + m_viewportYShift);
    // Alternative implementation which is basically doing the same but without precomputed variables
    // v[0] = (((v[0] + 1.0f) * m_viewportWidth * 0.5f) + m_viewportX);
    // v[1] = (((v[1] + 1.0f) * m_viewportHeight * 0.5f) + m_viewportY);
    v[2] = (((v[2] + 1.0f) * 0.25f)) * (m_depthRangeZFar - m_depthRangeZNear);
}


void VertexPipeline::setViewport(const int16_t x, const int16_t y, const int16_t width, const int16_t height)
{
    // Note: The screen resolution is width and height. But during view port transformation we are clamping between
    // 0 and height which means a effective screen resolution of height + 1. For instance, we have a resolution of
    // 480 x 272. The view port transformation would go from 0 to 480 which are then 481px. Thats the reason why we
    // decrement here the resolution by one.
    m_viewportHeight = height - 1;
    m_viewportWidth = width - 1;
    m_viewportX = x;
    m_viewportY = y;

    m_viewportHeightHalf = m_viewportHeight / 2.0f;
    m_viewportWidthHalf = m_viewportWidth / 2.0f;
    m_viewportXShift = m_viewportX + m_viewportWidthHalf;
    m_viewportYShift = m_viewportY + m_viewportHeightHalf;

}

void VertexPipeline::setDepthRange(const float zNear, const float zFar)
{
    m_depthRangeZFar = zFar;
    m_depthRangeZNear = zNear;
}

void VertexPipeline::setModelProjectionMatrix(const Mat44 &m)
{
    m_t = m;
}

void VertexPipeline::setModelMatrix(const Mat44 &m)
{
    m_m = m;
    m_modelMatrixChanged = true;
}

void VertexPipeline::setProjectionMatrix(const Mat44 &m)
{
    m_p = m;
    m_projectionMatrixChanged = true;
}

void VertexPipeline::setColorMatrix(const Mat44& m)
{
    m_c = m;
}

void VertexPipeline::setTextureMatrix(const Mat44& m)
{
    m_tm[m_tmu] = m;
}

void VertexPipeline::setNormalMatrix(const Mat44& m)
{
    m_n = m;
}

void VertexPipeline::setCullMode(const VertexPipeline::Face mode)
{
    m_cullMode = mode;
}

void VertexPipeline::enableCulling(const bool enable)
{
    m_enableCulling = enable;
}

void VertexPipeline::multiply(const Mat44& mat)
{
    switch (m_matrixMode)
    {
        case MatrixMode::MODELVIEW:
            setModelMatrix(mat * m_m);
            break;
        case MatrixMode::PROJECTION:
            setProjectionMatrix(mat * m_p);
            break;
        case MatrixMode::TEXTURE:
            setTextureMatrix(mat * m_tm[m_tmu]);
            break;
        case MatrixMode::COLOR:
            setColorMatrix(mat * m_c);
            break;
        default:
            break;
    }
}

void VertexPipeline::translate(const float x, const float y, const float z)
{
    Mat44 m;
    m.identity();
    m[3][0] = x;
    m[3][1] = y;
    m[3][2] = z;
    multiply(m);
}

void VertexPipeline::scale(const float x, const float y, const float z)
{
    Mat44 m;
    m.identity();
    m[0][0] = x;
    m[1][1] = y;
    m[2][2] = z;
    multiply(m);
}

void VertexPipeline::rotate(const float angle, const float x, const float y, const float z)
{
    float angle_rad = angle * (__glPi/180.0f);

    float c = cosf(angle_rad);
    float s = sinf(angle_rad);
    float t = 1.0f - c;

    Mat44 m
    {{{
        {c+x*x*t,   y*x*t+z*s,  z*x*t-y*s,  0.0f},
        {x*y*t-z*s, c+y*y*t,    z*y*t+x*s,  0.0f},
        {x*z*t+y*s, y*z*t-x*s,  z*z*t+c,    0.0f},
        {0.0f,      0.0f,       0.0f,       1.0f}
    }}};

    multiply(m);
}

void VertexPipeline::loadIdentity()
{
    switch (m_matrixMode)
    {
        case MatrixMode::MODELVIEW:
            m_m.identity();
            m_modelMatrixChanged = true;
            break;
        case MatrixMode::PROJECTION:
            m_p.identity();
            m_projectionMatrixChanged = true;
            break;
        case MatrixMode::TEXTURE:
            m_tm[m_tmu].identity();
            break;
        case MatrixMode::COLOR:
            m_c.identity();
            break;
        default:
            break;
    }
}

bool VertexPipeline::pushMatrix()
{
    switch (m_matrixMode)
    {
        case MatrixMode::MODELVIEW:
            return m_mStack.push(m_m);
        case MatrixMode::PROJECTION:
            return m_pStack.push(m_p);
        case MatrixMode::COLOR:
            return m_cStack.push(m_c);
        case MatrixMode::TEXTURE:
            return m_tmStack[m_tmu].push(m_tm[m_tmu]);
        default:
            return false;
    }
}

bool VertexPipeline::popMatrix()
{
    switch (m_matrixMode)
    {
        case MatrixMode::MODELVIEW:
            m_modelMatrixChanged = true;
            return m_mStack.pop(m_m);
        case MatrixMode::PROJECTION:
            m_projectionMatrixChanged = true;
            return m_pStack.pop(m_p);
        case MatrixMode::COLOR:
            return m_cStack.pop(m_c);
        case MatrixMode::TEXTURE:
            return m_tmStack[m_tmu].pop(m_tm[m_tmu]);
        default:
            return false;
    }
}

const Mat44& VertexPipeline::getModelMatrix() const
{
    return m_m;
}

const Mat44& VertexPipeline::getProjectionMatrix() const
{
    return m_p;
}

void VertexPipeline::recalculateMatrices()
{
    if (m_modelMatrixChanged)
    {
        recalculateNormalMatrix();
    }
    if (m_modelMatrixChanged || m_projectionMatrixChanged)
    {
        recalculateModelProjectionMatrix();
    }
    m_modelMatrixChanged = false;
    m_projectionMatrixChanged = false;
}

void VertexPipeline::recalculateModelProjectionMatrix()
{
    // Update transformation matrix
    setModelProjectionMatrix(m_m * m_p);
}

void VertexPipeline::recalculateNormalMatrix()
{
    m_n = m_m;
    m_n.invert();
    m_n.transpose();
}

void VertexPipeline::setMatrixMode(const MatrixMode matrixMode)
{
    m_matrixMode = matrixMode;
}

bool VertexPipeline::loadMatrix(const Mat44& m)
{
    switch (m_matrixMode)
    {
    case MatrixMode::MODELVIEW:
        setModelMatrix(m);
        return true;
    case MatrixMode::PROJECTION:
        setProjectionMatrix(m);
        return true;
    case MatrixMode::TEXTURE:
        setTextureMatrix(m);
        return true;
    case MatrixMode::COLOR:
        setColorMatrix(m);
        return true;
    default:
        break;
    }
    return false;
}

uint8_t VertexPipeline::getModelMatrixStackDepth()
{
    return MODEL_MATRIX_STACK_DEPTH;
}

uint8_t VertexPipeline::getProjectionMatrixStackDepth()
{
    return PROJECTION_MATRIX_STACK_DEPTH;
}

Lighting& VertexPipeline::getLighting()
{
    return m_lighting;
}

TexGen& VertexPipeline::getTexGen()
{
    return m_texGen[m_tmu];
}

void VertexPipeline::setColorMaterialTracking(const Face face, const ColorMaterialTracking material)
{
    switch (material) {
        case ColorMaterialTracking::AMBIENT:
            if (m_enableColorMaterial)
                getLighting().enableColorMaterial(false, true, false, false);
            break;
        case ColorMaterialTracking::DIFFUSE:
            if (m_enableColorMaterial)
                getLighting().enableColorMaterial(false, false, true, false);
            break;
        case ColorMaterialTracking::AMBIENT_AND_DIFFUSE:
            if (m_enableColorMaterial)
                getLighting().enableColorMaterial(false, true, true, false);
            break;
        case ColorMaterialTracking::SPECULAR:
            if (m_enableColorMaterial)
                getLighting().enableColorMaterial(false, false, false, true);
            break;
            case ColorMaterialTracking::EMISSION:
            if (m_enableColorMaterial)
                getLighting().enableColorMaterial(true, false, false, false);
            break;
        default:
            if (m_enableColorMaterial)
                getLighting().enableColorMaterial(false, true, true, false);
            break;
    }
    m_colorMaterialTracking = material;
    m_colorMaterialFace = face;
}

void VertexPipeline::enableColorMaterial(const bool enable)
{
    m_enableColorMaterial = enable;
    if (enable)
    {
        setColorMaterialTracking(m_colorMaterialFace, m_colorMaterialTracking);
    }
    else 
    {
        getLighting().enableColorMaterial(false, false, false, false);
    }
}

void VertexPipeline::setLineWidth(const float width)
{
    m_lineWidth = width;
}

} // namespace rr
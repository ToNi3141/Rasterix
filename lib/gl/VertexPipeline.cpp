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

#include "VertexPipeline.hpp"
#include "Vec.hpp"
#include "Veci.hpp"
#include <string.h>
#include <stdlib.h>
#include "Rasterizer.hpp"

// The Arduino IDE will produce compile errors when using std::min and std::max
#include <algorithm>    // std::max
#define max std::max
#define min std::min


VertexPipeline::VertexPipeline(Lighting& lighting, TexGen& texGen)
    : m_lighting(lighting)
    , m_texGen(texGen)
{
    m_t.identity();
    m_m.identity();
    m_n.identity();
}

bool VertexPipeline::drawObj(IRenderer &renderer, RenderObj &obj)
{
    // Transform vertex and calculate colors
    for (uint32_t i = 0; i < obj.count; i++)
    {
        const uint32_t index = obj.getIndex(i);
        if (!(i < RenderObj::MAX_VERTEX_CNT))
        {
            return false;
        }

        Vec4 v;
        Vec4 c;
        if (obj.colorArrayEnabled)
        {
            obj.getColor(c, index);
        }
        else
        {
            // If no color is defined, use the global color
            c = obj.vertexColor;
        }

        if (obj.vertexArrayEnabled)
        {
            obj.getVertex(v, index);
            m_t.transform(obj.transformedVertex[i], v);
        }

        if (obj.texCoordArrayEnabled)
        {
            obj.getTexCoord(obj.transformedTexCoord[i], index);
        }
        m_texGen.calculateTexGenCoords(m_m, obj.transformedTexCoord[i], v);


        if (m_lighting.lightingEnabled())
        {
            Vec4 vl;
            Vec3 nl;
            Vec3 n;
            if (obj.normalArrayEnabled)
            {
                obj.getNormal(n, index);
                m_n.transform(nl, n);
                // In OpenGL this step can be turned on and off with GL_NORMALIZE, also there is GL_RESCALE_NORMAL which offers a faster way
                // which only works with uniform scales. For now this is constantly enabled because it is usually what someone want.
                nl.normalize();
            }
            if (obj.vertexArrayEnabled)
            {
                m_m.transform(vl, v);
            }

            m_lighting.calculateLights(obj.transformedColor[i], c, vl, nl);
        }
        else
        {
            obj.transformedColor[i] = c;
        }
    }

    Triangle triangle;
    uint32_t index0 = 0;
    uint32_t index1 = 0;
    uint32_t index2 = 0;
    for (uint32_t i = 0; i < obj.count - 2; )
    {
        switch (obj.drawMode) {
        case RenderObj::DrawMode::TRIANGLES:
            index0 = (i);
            index1 = (i + 1);
            index2 = (i + 2);
            i += 3;
            break;
        case RenderObj::DrawMode::TRIANGLE_FAN:
            index0 = (0);
            index1 = (i + 1);
            index2 = (i + 2);
            i += 1;
            break;
        case RenderObj::DrawMode::TRIANGLE_STRIP:
            if (i & 0x1)
            {
                index0 = (i + 1);
                index1 = (i);
                index2 = (i + 2);
            }
            else
            {
                index0 = (i);
                index1 = (i + 1);
                index2 = (i + 2);
            }
            i += 1;
            break;
        case RenderObj::DrawMode::QUAD_STRIP:
            if (i & 0x2)
            {
                index0 = (i + 1);
                index1 = (i);
                index2 = (i + 2);
            }
            else
            {
                index0 = (i);
                index1 = (i + 1);
                index2 = (i + 2);
            }
            i += 1;
            break;
        default:
            break;
        }

        triangle.v0 = obj.transformedVertex[index0];
        triangle.v1 = obj.transformedVertex[index1];
        triangle.v2 = obj.transformedVertex[index2];

        triangle.st0 = obj.transformedTexCoord[index0];
        triangle.st1 = obj.transformedTexCoord[index1];
        triangle.st2 = obj.transformedTexCoord[index2];
        
        triangle.color0 = obj.transformedColor[index0];
        triangle.color1 = obj.transformedColor[index1];
        triangle.color2 = obj.transformedColor[index2];

        if (!drawTriangle(renderer, triangle))
        {
            return false;
        }
    }
    return true;
}

bool VertexPipeline::drawTriangle(IRenderer &renderer, const Triangle& triangle)
{
    Clipper::ClipVertList vertList;
    Clipper::ClipStList stList;
    Clipper::ClipColorList colorList;
    Clipper::ClipVertList vertListBuffer;
    Clipper::ClipStList stListBuffer;
    Clipper::ClipColorList colorListBuffer;

    vertList[0] = triangle.v0;
    vertList[1] = triangle.v1;
    vertList[2] = triangle.v2;

    stList[0] = triangle.st0;
    stList[1] = triangle.st1;
    stList[2] = triangle.st2;

    colorList[0] = triangle.color0;
    colorList[1] = triangle.color1;
    colorList[2] = triangle.color2;

    // Because if flat shading, the color doesn't have to be interpolated during clipping, so it can be ignored for now...
    auto [vertListSize, vertListClipped, stListClipped, colorListClipped] = Clipper::clip(vertList, vertListBuffer, stList, stListBuffer, colorList, colorListBuffer);

    // Calculate for every vertex the perspective division and also apply the viewport transformation
    for (uint8_t i = 0; i < vertListSize; i++)
    {
        perspectiveDivide(vertListClipped[i]);
        viewportTransform(vertListClipped[i]);
    }

    // Cull triangle
    if (m_enableCulling)
    {
        const float edgeVal = Rasterizer::edgeFunctionFloat(vertList[0], vertList[1], vertList[2]);
        const CullMode currentOrientation = (edgeVal <= 0.0f) ? CullMode::BACK : CullMode::FRONT;
        if (currentOrientation != m_cullMode)
            return true;
    }

    // Render the triangle
    for (uint8_t i = 3; i <= vertListSize; i++)
    {
        // For a triangle we need atleast 3 vertices. Also treat the clipped list from the clipping as a
        // triangle fan where vert zero is always the center of this fan
        const bool success = renderer.drawTriangle(vertListClipped[0],
                vertListClipped[i - 2],
                vertListClipped[i - 1],
                stListClipped[0],
                stListClipped[i - 2],
                stListClipped[i - 1],
                colorListClipped[0],
                colorListClipped[i - 2],
                colorListClipped[i - 1]);
        if (!success)
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

void VertexPipeline::perspectiveDivide(Vec4 &v)
{
    v[3] = 1.0f / v[3];
    v[0] = v[0] * v[3];
    v[1] = v[1] * v[3];
    v[2] = v[2] * v[3];
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
}

void VertexPipeline::setNormalMatrix(const Mat44& m)
{
    m_n = m;
}

void VertexPipeline::setCullMode(VertexPipeline::CullMode mode)
{
    m_cullMode = mode;
}

void VertexPipeline::enableCulling(bool enable)
{
    m_enableCulling = enable;
}

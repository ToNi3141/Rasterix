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


namespace rr
{
VertexPipeline::VertexPipeline(PixelPipeline& renderer)
    : m_renderer(renderer)
{
}

void VertexPipeline::fetchAndTransform(PrimitiveAssembler::Triangle::VertexParameter& parameter, const RenderObj& obj, uint32_t i)
{
    const uint32_t pos = obj.getIndex(i);
    parameter.vertex = obj.getVertex(pos);
    for (uint32_t tu = 0; tu < IRenderer::MAX_TMU_COUNT; tu++)
    {
        if (m_renderer.getEnableTmu(tu))
        {
            parameter.tex[tu] = obj.getTexCoord(tu, pos);
            m_texGen[tu].calculateTexGenCoords(m_matrixStack.getModelView(), parameter.tex[tu], parameter.vertex);
            m_matrixStack.getTexture(tu).transform(parameter.tex[tu], parameter.tex[tu]); 
        }
    }

    // TODO: Check if this required? The standard requires but is it really used?
    //m_c[j].transform(color, color); // Calculate this in one batch to improve performance
    parameter.color = obj.getColor(pos);
    if (m_lighting.lightingEnabled())
    {
        Vec4 vl;
        Vec3 normal = m_matrixStack.getNormal().transform(obj.getNormal(pos));

        if (m_enableNormalizing) [[unlikely]]
        {
            normal.normalize();
        }
        if (obj.vertexArrayEnabled())
            m_matrixStack.getModelView().transform(vl, parameter.vertex);
        m_lighting.calculateLights(parameter.color, parameter.color, vl, normal);
    }
    if (obj.vertexArrayEnabled())
        m_matrixStack.getModelViewProjection().transform(parameter.vertex, parameter.vertex);
}

bool VertexPipeline::drawObj(const RenderObj &obj)
{
    m_matrixStack.recalculateMatrices();
    if (!m_renderer.updatePipeline()) [[unlikely]]
    {
        SPDLOG_ERROR("drawObj(): Cannot update pixel pipeline");
        return false;
    }
    obj.logCurrentConfig();
    m_primitiveAssembler.clear();
    m_primitiveAssembler.setDrawMode(obj.getDrawMode());
    m_primitiveAssembler.setExpectedPrimitiveCount(obj.getCount());
    uint32_t count = obj.getCount();
    for (uint32_t it = 0; it < count; it++)
    {
        PrimitiveAssembler::Triangle::VertexParameter& param = m_primitiveAssembler.createParameter();
        fetchAndTransform(param, obj, it);

        const std::span<const PrimitiveAssembler::Triangle> triangles = m_primitiveAssembler.getPrimitive();
        for (const PrimitiveAssembler::Triangle& triangle : triangles)
        {
            if (!drawTriangle(triangle)) [[unlikely]]
            {
                return false;
            }
        }
        if (!triangles.empty())
        {
            m_primitiveAssembler.removePrimitive();
        }
    }

    return true;
}

bool VertexPipeline::drawTriangle(const PrimitiveAssembler::Triangle& triangle)
{
    Clipper::ClipVertList vertList;
    Clipper::ClipTexCoordList texCoordList;
    Clipper::ClipVertList colorList;
    Clipper::ClipVertList vertListBuffer;
    Clipper::ClipTexCoordList texCoordListBuffer;
    Clipper::ClipVertList colorListBuffer;

    vertList[0] = triangle.p0->vertex;
    vertList[1] = triangle.p1->vertex;
    vertList[2] = triangle.p2->vertex;

    for (uint32_t i = 0; i < IRenderer::MAX_TMU_COUNT; i++)
    {
        if (m_renderer.getEnableTmu(i))
        {
            texCoordList[0][i] = triangle.p0->tex[i];
            texCoordList[1][i] = triangle.p1->tex[i];
            texCoordList[2][i] = triangle.p2->tex[i];
        }
    }


    colorList[0] = triangle.p0->color;
    colorList[1] = triangle.p1->color;
    colorList[2] = triangle.p2->color;

    auto [vertListSize, vertListClipped, texCoordListClipped, colorListClipped] = Clipper::clip(vertList, vertListBuffer, texCoordList, texCoordListBuffer, colorList, colorListBuffer);

    std::array<float, vertList.size()> oowList;

    // Calculate for every vertex the perspective division and also apply the viewport transformation
    for (uint32_t i = 0; i < vertListSize; i++)
    {
        // Perspective division
        vertListClipped[i].perspectiveDivide();
        oowList[i] = vertListClipped[i][3];

        // Moved into the Rasterizer.cpp. But it is probably faster to calculate it here ...
        // for (uint8_t j = 0; j < IRenderer::MAX_TMU_COUNT; j++)
        // {
        //     // Perspective correction of the texture coordinates
        //     if (m_renderer.getEnableTmu(j))
        //         texCoordListClipped[i][j].mul(vertListClipped[i][3]); // since w is already divided, just multiply the 1/w to all elements. Saves one division.
        //     // TODO: Perspective correction of the color 
        //     // Each texture uses it's own scaled w (basically q*w). Therefore the hardware must 
        //     // interpolate (q*w) for each texture. w alone is not enough because OpenGL allows to set q coordinate.
        // }

        // Viewport transformation of the vertex
        m_viewPort.transform(vertListClipped[i]);
    }

    // Cull triangle
    // Check only one triangle in the clipped list. The triangles are sub divided, but not rotated. So if one triangle is 
    // facing backwards, then all in the clipping list will do this and vice versa.
    if (m_culling.cull(vertListClipped[0], vertListClipped[1], vertListClipped[2]))
    {
        return true;
    }
    
    if (m_renderer.stencil().getEnableTwoSideStencil()) [[unlikely]]
    {
        const float edgeVal = Rasterizer::edgeFunctionFloat(vertListClipped[0], vertListClipped[1], vertListClipped[2]);
        const Face currentOrientation = (edgeVal <= 0.0f) ? Face::BACK : Face::FRONT;
        if (currentOrientation != Face::FRONT) // TODO: The rasterizer expects triangles in CW. OpenGL in CCW. Thats the reason why Front and Back a screwed up.
        {
            m_renderer.stencil().selectStencilTwoSideFrontForDevice();
        }
        else
        {
            m_renderer.stencil().selectStencilTwoSideBackForDevice();
        }
        if (!m_renderer.updatePipeline()) [[unlikely]]
        {
            SPDLOG_ERROR("drawTriangle(): Cannot update pixel pipeline");
            return false;
        }
    }

    // Render the triangle
    for (uint32_t i = 3; i <= vertListSize; i++)
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
                { { oowList[0], oowList[i - 2], oowList[i - 1] } },
                { *(texture0.data()), texture0.size() },
                { *(texture1.data()), texture1.size() },
                { *(texture2.data()), texture2.size() },
                colorListClipped[0],
                colorListClipped[i - 2],
                colorListClipped[i - 1] });
        if (!success) [[unlikely]]
        {
            return false;
        }
    }
    return true;
}

Culling& VertexPipeline::getCulling()
{
    return m_culling;
}

Lighting& VertexPipeline::getLighting()
{
    return m_lighting;
}

TexGen& VertexPipeline::getTexGen()
{
    return m_texGen[m_tmu];
}

ViewPort& VertexPipeline::getViewPort()
{
    return m_viewPort;
}

MatrixStack& VertexPipeline::getMatrixStack()
{
    return m_matrixStack;
}

void VertexPipeline::setLineWidth(const float width)
{
    m_primitiveAssembler.setLineWidth(width);
}

} // namespace rr

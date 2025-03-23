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
#include "math/Vec.hpp"
#include "math/Veci.hpp"
#include <cmath>
#include <spdlog/spdlog.h>
#include <stdlib.h>
#include <string.h>

// The Arduino IDE will produce compile errors when using std::min and std::max
#include <algorithm> // std::max
#define max std::max
#define min std::min

namespace rr
{
VertexPipeline::VertexPipeline(PixelPipeline& renderer)
    : m_renderer(renderer)
{
    for (auto& tg : m_texGen)
    {
        tg.setMatrixStore(m_matrixStore);
    }
}

void VertexPipeline::fetch(VertexParameter& parameter, const RenderObj& obj, std::size_t i)
{
    const std::size_t pos = obj.getIndex(i);
    parameter.vertex = obj.getVertex(pos);
    parameter.normal = obj.getNormal(pos);
    parameter.color = obj.getColor(pos);
    for (std::size_t tu = 0; tu < RenderConfig::TMU_COUNT; tu++)
    {
        parameter.tex[tu] = obj.getTexCoord(tu, pos);
    }
}

void VertexPipeline::transform(VertexParameter& parameter)
{
    for (std::size_t tu = 0; tu < RenderConfig::TMU_COUNT; tu++)
    {
        if (m_renderer.featureEnable().getEnableTmu(tu))
        {
            m_texGen[tu].calculateTexGenCoords(parameter.tex[tu], parameter.vertex, parameter.normal);
            parameter.tex[tu] = m_matrixStore.getTexture(tu).transform(parameter.tex[tu]);
        }
    }

    // TODO: Check if this required? The standard requires but is it really used?
    // m_c[j].transform(color, color); // Calculate this in one batch to improve performance
    if (m_lighting.lightingEnabled())
    {
        Vec3 normal = m_matrixStore.getNormal().transform(parameter.normal);

        if (m_enableNormalizing)
        {
            normal.normalize();
        }
        const Vec4 vl = m_matrixStore.getModelView().transform(parameter.vertex);
        const Vec4 c = parameter.color;
        m_lighting.calculateLights(parameter.color, c, vl, normal);
    }
    parameter.vertex = m_matrixStore.getModelViewProjection().transform(parameter.vertex);
}

bool VertexPipeline::drawObj(const RenderObj& obj)
{
    if (!obj.vertexArrayEnabled())
    {
        SPDLOG_INFO("drawObj(): Vertex array disabled. No primitive is rendered.");
        return true;
    }
    m_matrixStore.recalculateMatrices();
    stencil().update();
    if (!m_renderer.updatePipeline())
    {
        SPDLOG_ERROR("drawObj(): Cannot update pixel pipeline");
        return false;
    }
    obj.logCurrentConfig();
    m_primitiveAssembler.clear();
    m_primitiveAssembler.setDrawMode(obj.getDrawMode());
    m_primitiveAssembler.setExpectedPrimitiveCount(obj.getCount());
    std::size_t count = obj.getCount();
    for (std::size_t it = 0; it < count; it++)
    {
        VertexParameter& param = m_primitiveAssembler.createParameter();
        fetch(param, obj, it);
        transform(param);

        const tcb::span<const PrimitiveAssembler::Triangle> triangles = m_primitiveAssembler.getPrimitive();
        for (const PrimitiveAssembler::Triangle& triangle : triangles)
        {
            if (!drawTriangle(triangle))
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

bool VertexPipeline::drawClippedTriangleList(tcb::span<VertexParameter> list)
{
    // Calculate for every vertex the perspective division and also apply the viewport transformation
    const std::size_t clippedVertexListSize = list.size();
    for (std::size_t i = 0; i < clippedVertexListSize; i++)
    {
        // Perspective division
        list[i].vertex.perspectiveDivide();

        // Moved into the Rasterizer.cpp. But it is probably faster to calculate it here ...
        // for (std::size_t j = 0; j < RenderConfig::TMU_COUNT; j++)
        // {
        //     // Perspective correction of the texture coordinates
        //     if (m_renderer.getEnableTmu(j))
        //         texCoordListClipped[i][j].mul(vertListClipped[i][3]); // since w is already divided, just multiply the 1/w to all elements. Saves one division.
        //     // TODO: Perspective correction of the color
        //     // Each texture uses it's own scaled w (basically q*w). Therefore the hardware must
        //     // interpolate (q*w) for each texture. w alone is not enough because OpenGL allows to set q coordinate.
        // }

        // Viewport transformation of the vertex
        m_viewPort.transform(list[i].vertex);
    }

    // Cull triangle
    // Check only one triangle in the clipped list. The triangles are sub divided, but not rotated. So if one triangle is
    // facing backwards, then all in the clipping list will do this and vice versa.
    if (m_culling.cull(list[0].vertex, list[1].vertex, list[2].vertex))
    {
        return true;
    }

    if (!stencil().updateStencilFace(list[0].vertex, list[1].vertex, list[2].vertex))
    {
        return false;
    }

    // Render the triangle
    for (std::size_t i = 3; i <= clippedVertexListSize; i++)
    {
        const bool success = m_renderer.drawTriangle({
            list[0].vertex,
            list[i - 2].vertex,
            list[i - 1].vertex,
            list[0].tex,
            list[i - 2].tex,
            list[i - 1].tex,
            list[0].color,
            list[i - 2].color,
            list[i - 1].color,
        });
        if (!success)
        {
            return false;
        }
    }
    return true;
}

bool VertexPipeline::drawUnclippedTriangle(const PrimitiveAssembler::Triangle& triangle)
{
    // Optimized version of the drawTriangle when a triangle is not needed to be clipped.

    Vec4 v0 = triangle[0].get().vertex;
    Vec4 v1 = triangle[1].get().vertex;
    Vec4 v2 = triangle[2].get().vertex;

    // Perspective division
    v0.perspectiveDivide();
    v1.perspectiveDivide();
    v2.perspectiveDivide();

    // Viewport transformation of the vertex
    m_viewPort.transform(v0);
    m_viewPort.transform(v1);
    m_viewPort.transform(v2);

    // Cull triangle
    // Check only one triangle in the clipped list. The triangles are sub divided, but not rotated. So if one triangle is
    // facing backwards, then all in the clipping list will do this and vice versa.
    if (m_culling.cull(v0, v1, v2))
    {
        return true;
    }

    if (!stencil().updateStencilFace(v0, v1, v2))
    {
        return false;
    }

    return m_renderer.drawTriangle({
        v0,
        v1,
        v2,
        triangle[0].get().tex,
        triangle[1].get().tex,
        triangle[2].get().tex,
        triangle[0].get().color,
        triangle[1].get().color,
        triangle[2].get().color,
    });
}

bool VertexPipeline::drawTriangle(const PrimitiveAssembler::Triangle& triangle)
{
    if (Clipper::isInside(triangle[0].get().vertex, triangle[1].get().vertex, triangle[2].get().vertex))
    {
        return drawUnclippedTriangle(triangle);
    }

    if (Clipper::isOutside(triangle[0].get().vertex, triangle[1].get().vertex, triangle[2].get().vertex))
    {
        return true;
    }

    Clipper::ClipList list;
    Clipper::ClipList listBuffer;

    list[0] = triangle[0];
    list[1] = triangle[1];
    list[2] = triangle[2];

    tcb::span<VertexParameter> clippedVertexParameter = Clipper::clip(list, listBuffer);

    if (clippedVertexParameter.empty())
    {
        return true;
    }

    return drawClippedTriangleList(clippedVertexParameter);
}

} // namespace rr

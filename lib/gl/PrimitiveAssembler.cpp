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

#include "PrimitiveAssembler.hpp"

namespace rr
{

std::span<const PrimitiveAssembler::Triangle> PrimitiveAssembler::getPrimitive()
{
    if (m_line) [[unlikely]]
    {
        return constructLine();
    }
    return constructTriangle();
}

std::span<const PrimitiveAssembler::Triangle> PrimitiveAssembler::constructTriangle()
{
    if (m_queue.size() < 3)
    {
        return {};
    }

    switch (m_drawMode) {
    case RenderObj::DrawMode::TRIANGLES:
        m_triangleBuffer[0] = { &m_queue[0], &m_queue[1], &m_queue[2] };
        m_queue.decSize(3);
        break;
    case RenderObj::DrawMode::POLYGON:
    case RenderObj::DrawMode::TRIANGLE_FAN:
        if (m_count == 0)
        {
            m_pTmp = m_queue[0];
        }
        m_triangleBuffer[0] = { &m_pTmp, &m_queue[1], &m_queue[2] };
        m_queue.decSize(1);
        break;
    case RenderObj::DrawMode::TRIANGLE_STRIP:
        if (m_count & 0x1)
        {
            m_triangleBuffer[0] = { &m_queue[1], &m_queue[0], &m_queue[2] };
        }
        else
        {
            m_triangleBuffer[0] = { &m_queue[0], &m_queue[1], &m_queue[2] };
        }
        m_queue.decSize(1);
        break;
    case RenderObj::DrawMode::QUADS:
        if (m_count & 0x1)
        {
            m_triangleBuffer[0] = { &m_pTmp, &m_queue[1], &m_queue[2] };
            m_queue.decSize(3);
        }
        else
        {
            m_pTmp = m_queue[0];
            m_triangleBuffer[0] = { &m_pTmp, &m_queue[1], &m_queue[2] };

            m_queue.decSize(1);
        }
        break;
    case RenderObj::DrawMode::QUAD_STRIP:
        if (m_count & 0x1)
        {
            m_triangleBuffer[0] = { &m_queue[0], &m_queue[2], &m_queue[1] };
        }
        else
        {
            m_triangleBuffer[0] = { &m_queue[0], &m_queue[1], &m_queue[2] };
        }
        m_queue.decSize(1);
        break;
    [[unlikely]] default:
        return {};
    }

    m_count++;
    return { m_triangleBuffer.data(), 1 };
}

std::span<const PrimitiveAssembler::Triangle> PrimitiveAssembler::constructLine()
{
    if (m_queue.size() < 2)
    {
        return {};
    }

    std::size_t last = (m_count == (m_expectedPrimitiveCount - 1));
    RenderObj::VertexParameter* p0;
    RenderObj::VertexParameter* p1;

    switch (m_drawMode) {
    case RenderObj::DrawMode::LINES:
        p0 = &m_queue[0];
        p1 = &m_queue[1];
        m_queue.decSize(2);
        break;
    case RenderObj::DrawMode::LINE_LOOP:
        if (m_count == 0)
        {
            m_pTmp = m_queue[0];
        }
        if (last)
        {
            p0 = &m_queue[0];
            p1 = &m_pTmp;
        }
        else
        {
            p0 = &m_queue[0];
            p1 = &m_queue[1];
        }
        m_queue.decSize(1);
        break;
    case RenderObj::DrawMode::LINE_STRIP:
        p0 = &m_queue[0];
        p1 = &m_queue[1];
        m_queue.decSize(1);
        break;
    [[unlikely]] default:
        return {};
    }

    m_count++;
    return drawLine(p0->vertex, p1->vertex, p0->tex, p1->tex, p0->color, p1->color);
}

std::span<const PrimitiveAssembler::Triangle> PrimitiveAssembler::drawLine(const Vec4& v0, const Vec4& v1, const std::array<Vec4, IRenderer::MAX_TMU_COUNT>& tc0, const std::array<Vec4, IRenderer::MAX_TMU_COUNT>& tc1, const Vec4& c0, const Vec4& c1)
{
    // Copied from swGL and adapted.

    // Get the reciprocal viewport scaling factor
    float rcpViewportScaleX = 2.0f / static_cast<float>(m_viewPort.getViewPortWidth());
    float rcpViewportScaleY = 2.0f / static_cast<float>(m_viewPort.getViewPortHeight());

    // Calculate the lines normal n = normalize(-dx, dy)
    float nx = -((v1[1] / v1[3]) - (v0[1] / v0[3]));
    float ny =  ((v1[0] / v1[3]) - (v0[0] / v0[3]));
    float rcpLength = 1.0f / std::sqrt(nx * nx + ny * ny);
    nx *= rcpLength;
    ny *= rcpLength;

    // Scale normal according to the width of the line
    float halfLineWidth = m_lineWidth * 0.5f;
    nx *= halfLineWidth;
    ny *= halfLineWidth;

    // Now create the vertices that define two triangles which are used to draw the line
    m_lineVertexBuffer = { { v0, v0, v1, v1 } };

    m_lineVertexBuffer[0][0] += ( nx * v0[3]) * rcpViewportScaleX;
    m_lineVertexBuffer[0][1] += ( ny * v0[3]) * rcpViewportScaleY;
    m_lineVertexBuffer[1][0] += (-nx * v0[3]) * rcpViewportScaleX;
    m_lineVertexBuffer[1][1] += (-ny * v0[3]) * rcpViewportScaleY;

    m_lineVertexBuffer[2][0] += ( nx * v1[3]) * rcpViewportScaleX;
    m_lineVertexBuffer[2][1] += ( ny * v1[3]) * rcpViewportScaleY;
    m_lineVertexBuffer[3][0] += (-nx * v1[3]) * rcpViewportScaleX;
    m_lineVertexBuffer[3][1] += (-ny * v1[3]) * rcpViewportScaleY;

    m_p0 = { m_lineVertexBuffer[0], c0, tc0 };
    m_p1 = { m_lineVertexBuffer[1], c0, tc0 };
    m_p2 = { m_lineVertexBuffer[2], c1, tc1 };
    m_p3 = { m_lineVertexBuffer[2], c1, tc1 };
    m_p4 = { m_lineVertexBuffer[1], c0, tc0 };
    m_p5 = { m_lineVertexBuffer[3], c1, tc1 };

    m_triangleBuffer[0] = { &m_p0, &m_p1, &m_p2 };
    m_triangleBuffer[1] = { &m_p3, &m_p4, &m_p5 };

   return { m_triangleBuffer.data(), 2 };
}

void PrimitiveAssembler::setDrawMode(const RenderObj::DrawMode mode)
{
    m_drawMode = mode;
    switch (m_drawMode) {
    case RenderObj::DrawMode::LINES:
    case RenderObj::DrawMode::LINE_LOOP:
    case RenderObj::DrawMode::LINE_STRIP:
        m_line = true;
        break;
    [[unlikely]] default:
        m_line = false;
        break;
    }
}

void PrimitiveAssembler::clear()
{
    m_queue.clear();
    m_count = 0;
}

} // namespace rr

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

#ifndef PRIMITIVE_ASSEMBLER_HPP
#define PRIMITIVE_ASSEMBLER_HPP

#include <tcb/span.hpp>

#include "FixedSizeQueue.hpp"
#include "RenderObj.hpp"
#include "Types.hpp"
#include "ViewPort.hpp"

namespace rr
{

class PrimitiveAssembler
{
public:
    using Triangle = std::array<std::reference_wrapper<const VertexParameter>, 3>;

    PrimitiveAssembler(ViewPort& viewPort)
        : m_viewPort(viewPort)
    {
    }

    tcb::span<const Triangle> getPrimitive()
    {
        if (m_line)
        {
            return constructLine();
        }
        return constructTriangle();
    }
    void removePrimitive() { m_queue.removeElements(m_decrement); }

    void setExpectedPrimitiveCount(const std::size_t count) { m_expectedPrimitiveCount = count; }

    void setDrawMode(const RenderObj::DrawMode mode);
    VertexParameter& createParameter() { return m_queue.create_back(); }
    void clear();

    void setLineWidth(const float width) { m_lineWidth = width; }

    bool hasTriangles() const { return m_queue.size() >= 3; }

private:
    tcb::span<const Triangle> constructTriangle();
    tcb::span<const Triangle> constructLine();
    tcb::span<const Triangle> drawLine(const Vec4& v0,
        const Vec4& v1,
        const std::array<Vec4, RenderConfig::TMU_COUNT>& tc0,
        const std::array<Vec4, RenderConfig::TMU_COUNT>& tc1,
        const Vec4& c0,
        const Vec4& c1);

    RenderObj::DrawMode m_drawMode { RenderObj::DrawMode::TRIANGLES };
    FixedSizeQueue<VertexParameter, 3> m_queue {};

    std::size_t m_expectedPrimitiveCount { 0 };
    std::size_t m_count { 0 };
    VertexParameter m_pTmp {};

    std::size_t m_decrement { 0 };

    ViewPort& m_viewPort;
    float m_lineWidth { 1.0 };
    bool m_line { false };
    std::array<VertexParameter, 6> m_vertexParameters;
    std::array<PrimitiveAssembler::Triangle, 2> m_triangleBuffer { { { { m_pTmp, m_pTmp, m_pTmp } }, { { m_pTmp, m_pTmp, m_pTmp } } } };
};

} // namespace rr
#endif // PRIMITIVE_ASSEMBLER_HPP

// RasterIX
// https://github.com/ToNi3141/RasterIX
// Copyright (c) 2025 ToNi3141

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

#ifndef _REGULAR_TRIANGLE_CMD_HPP_
#define _REGULAR_TRIANGLE_CMD_HPP_

#include "RenderConfigs.hpp"
#include "math/Vec.hpp"
#include "renderer/Rasterizer.hpp"
#include "renderer/Triangle.hpp"
#include "renderer/commands/TriangleStreamTypes.hpp"
#include "renderer/displaylist/DisplayList.hpp"
#include <array>
#include <cstdint>
#include <tcb/span.hpp>
#include <type_traits>
#include <typeinfo>

namespace rr
{

class RegularTriangleCmd
{
    static constexpr uint32_t TRIANGLE_STREAM { 0xF000'0000 };
    static constexpr uint32_t OP_MASK { 0xF000'0000 };

public:
    struct RegularTriangle
    {
#pragma pack(push, 4)
        Vec4 vertex0;
        Vec4 vertex1;
        Vec4 vertex2;
        std::array<Vec4, RenderConfig::TMU_COUNT> texture0;
        std::array<Vec4, RenderConfig::TMU_COUNT> texture1;
        std::array<Vec4, RenderConfig::TMU_COUNT> texture2;
        Vec4 color0;
        Vec4 color1;
        Vec4 color2;
        std::size_t lineStart;
        std::size_t lineEnd;
#pragma pack(pop)
        RegularTriangle& operator=(const RegularTriangle&) = default;
    };

    RegularTriangleCmd() = default;
    RegularTriangleCmd(const TransformedTriangle& triangle)
    {
        RegularTriangle& t = m_desc[0];
        t.vertex0 = triangle.vertex0;
        t.vertex1 = triangle.vertex1;
        t.vertex2 = triangle.vertex2;
        t.texture0 = triangle.texture0;
        t.texture1 = triangle.texture1;
        t.texture2 = triangle.texture2;
        t.color0 = triangle.color0;
        t.color1 = triangle.color1;
        t.color2 = triangle.color2;
        t.lineStart = 0;
        t.lineEnd = 0;

        m_bbStartY = (std::min)((std::min)(t.vertex0[1], t.vertex1[1]), t.vertex2[1]);
        m_bbEndY = (std::max)((std::max)(t.vertex0[1], t.vertex1[1]), t.vertex2[1]);
    }

    RegularTriangleCmd(const RegularTriangleCmd& c) { operator=(c); }

    bool isInBounds(const std::size_t lineStart, const std::size_t lineEnd) const
    {
        return ((m_bbEndY >= lineStart) && (m_bbStartY < lineEnd));
    }

    const RegularTriangleCmd& getIncremented(const std::size_t lineStart, const std::size_t lineEnd)
    {
        m_desc[0].lineStart = lineStart;
        m_desc[0].lineEnd = lineEnd;
        return *this;
    }

    static constexpr bool isVisible() { return true; }

    using PayloadType = std::array<RegularTriangle, 1>;
    const PayloadType& payload() const { return m_desc; }
    using CommandType = uint32_t;
    static constexpr CommandType command() { return TRIANGLE_STREAM | (displaylist::DisplayList::template sizeOf<TransformedTriangle>()); }

    static std::size_t getNumberOfElementsInPayloadByCommand(const uint32_t) { return std::tuple_size<PayloadType> {}; }
    static bool isThis(const CommandType cmd) { return (cmd & OP_MASK) == TRIANGLE_STREAM; }

    RegularTriangleCmd& operator=(const RegularTriangleCmd&) = default;

private:
    std::array<RegularTriangle, 1> m_desc;
    std::size_t m_bbStartY;
    std::size_t m_bbEndY;
};

} // namespace rr

#endif // _REGULAR_TRIANGLE_CMD_HPP_

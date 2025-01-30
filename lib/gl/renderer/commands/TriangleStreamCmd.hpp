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


#ifndef _TRIANGLE_STREAM_CMD_HPP_
#define _TRIANGLE_STREAM_CMD_HPP_

#include <cstdint>
#include <array>
#include <tcb/span.hpp>
#include <type_traits>
#include <typeinfo>
#include "math/Vec.hpp"
#include "renderer/Triangle.hpp"
#include "renderer/Rasterizer.hpp"
#include "renderer/commands/TriangleStreamTypes.hpp"

namespace rr
{

template <typename DisplayList>
class TriangleStreamCmd
{
    static constexpr uint32_t TRIANGLE_STREAM { 0x3000'0000 };
public:


    // Both, the float and fix point variant expecting the triangle parameters as float.
    // Therefore: Set the interpolation by default to float.
    static constexpr bool ENABLE_FLOAT_INTERPOLATION { true };
    using TrDesc = typename std::conditional<ENABLE_FLOAT_INTERPOLATION, TriangleStreamTypes::TriangleDesc, TriangleStreamTypes::TriangleDescX>::type;
    
    TriangleStreamCmd(const Rasterizer& rasterizer, const TransformedTriangle& triangle)
    {
        m_visible = rasterizer.rasterize(m_desc[0], triangle);
    }

    TriangleStreamCmd(const TriangleStreamCmd& c) { operator=(c); }

    bool isInBounds(const std::size_t lineStart, const std::size_t lineEnd) const 
    {
        return Rasterizer::checkIfTriangleIsInBounds(m_desc[0].param, lineStart, lineEnd);
    }

    bool increment(const std::size_t lineStart, const std::size_t lineEnd)
    {
        bool ret = Rasterizer::increment(m_desc[0], lineStart, lineEnd);
        return ret;
    }

    bool isVisible() const { return m_visible; };

    using PayloadType = std::array<TriangleStreamTypes::TriangleDesc, 1>;
    const PayloadType& payload() const { return m_desc; }
    using CommandType = uint32_t;
    static constexpr CommandType command() { return TRIANGLE_STREAM | (DisplayList::template sizeOf<TrDesc>()); }

    TriangleStreamCmd<DisplayList>& operator=(const TriangleStreamCmd<DisplayList>& rhs)
    {
        m_desc = rhs.m_desc;
        m_visible = rhs.m_visible;
        return *this;
    }

private:
    std::array<TriangleStreamTypes::TriangleDesc, 1> m_desc;
    bool m_visible { false };
};

} // namespace rr

#endif // _TRIANGLE_STREAM_CMD_HPP_

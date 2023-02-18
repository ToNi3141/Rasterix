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
#include "Vec.hpp"
#include "Veci.hpp"
#include "Triangle.hpp"
#include "Rasterizer.hpp"
#include "commands/TriangleStreamTypes.hpp"

namespace rr
{

template <uint8_t MAX_TMU_COUNT>
class TriangleStreamCmd
{
    static constexpr uint32_t TRIANGLE_STREAM { 0x3000'0000 };
public:
    struct TriangleDesc
    {
#pragma pack(push, 4)
        TriangleStreamTypes::StaticParams param;
        std::array<TriangleStreamTypes::Texture, MAX_TMU_COUNT> texture;
#pragma pack(pop)
    };

    TriangleStreamCmd(const Rasterizer& rasterizer, const Triangle& triangle)
    {
        m_visible = rasterizer.rasterize(m_desc.param, { m_desc.texture }, triangle);
    }

    bool isInBounds(const uint16_t lineStart, const uint16_t lineEnd) const 
    {
        return Rasterizer::checkIfTriangleIsInBounds(m_desc.param, lineStart, lineEnd);
    }

    bool isVisible() const { return m_visible; };

    using Desc = std::array<tcb::span<TriangleDesc>, 1>;
    void serialize(Desc& desc) const 
    { 
        std::memcpy(desc[0].data(), &m_desc, sizeof(TriangleDesc));
    }
    static constexpr uint32_t command() { return TRIANGLE_STREAM | sizeof(TriangleDesc); }

private:
    TriangleDesc m_desc;
    bool m_visible { false };
};

} // namespace rr

#endif // _TRIANGLE_STREAM_CMD_HPP_

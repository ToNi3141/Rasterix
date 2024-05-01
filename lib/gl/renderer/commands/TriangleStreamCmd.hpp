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
#include <span>
#include <type_traits>
#include <typeinfo>
#include "math/Vec.hpp"
#include "math/Veci.hpp"
#include "renderer/Triangle.hpp"
#include "renderer/Rasterizer.hpp"
#include "renderer/commands/TriangleStreamTypes.hpp"

namespace rr
{

template <uint8_t MAX_TMU_COUNT, bool ENABLE_FLOAT_INTERPOLATION>
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
    struct TriangleDescX
    {
#pragma pack(push, 4)
        TriangleStreamTypes::StaticParamsX param;
        std::array<TriangleStreamTypes::TextureX, MAX_TMU_COUNT> texture;
        void operator=(const TriangleDesc& t)
        {
            param = t.param;
            for (uint32_t i = 0; i < texture.size(); i++)
            {
                texture[i] = t.texture[i];
            }
        };
#pragma pack(pop)
    };

    using TrDesc = typename std::conditional<ENABLE_FLOAT_INTERPOLATION, TriangleDesc, TriangleDescX>::type;
    
    TriangleStreamCmd(const Rasterizer& rasterizer, const TransformedTriangle& triangle)
    {
        m_visible = rasterizer.rasterize(m_desc.param, { m_desc.texture }, triangle);
    }

    bool isInBounds(const uint16_t lineStart, const uint16_t lineEnd) const 
    {
        return Rasterizer::checkIfTriangleIsInBounds(m_desc.param, lineStart, lineEnd);
    }

    bool increment(const uint16_t lineStart, const uint16_t lineEnd)
    {
        return Rasterizer::increment(m_desc.param, { m_desc.texture }, lineStart, lineEnd);
    }

    bool isVisible() const { return m_visible; };

    using Desc = std::array<std::span<TrDesc>, 1>;
    void serialize(Desc& desc) const 
    { 
        desc[0][0] = m_desc;
    }
    static constexpr uint32_t command() { return TRIANGLE_STREAM | sizeof(TrDesc); }

private:
    TriangleDesc m_desc;
    bool m_visible { false };
};

} // namespace rr

#endif // _TRIANGLE_STREAM_CMD_HPP_

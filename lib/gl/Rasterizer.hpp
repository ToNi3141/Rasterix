// Rasterix
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

#ifndef RASTERIZER_HPP
#define RASTERIZER_HPP
#include <stdint.h>
#include <array>
#include <span>
#include "Vec.hpp"
#include "Triangle.hpp"
#include "IRenderer.hpp"
#include <bitset>
#include "commands/TriangleStreamTypes.hpp"

namespace rr
{
class Rasterizer
{
public:  

    Rasterizer(const bool enableScaling)
        : m_enableScaling(enableScaling)
    {}

    bool rasterize(TriangleStreamTypes::StaticParams& params, 
                   const std::span<TriangleStreamTypes::Texture>& texture, 
                   const Triangle& triangle) const;

    void setScissorBox(const int32_t x, const int32_t y, const uint32_t width, const uint32_t height);
    void enableScissor(const bool enable) { m_enableScissor = enable; }
    void enableTmu(const uint8_t tmu, const bool enable) { m_tmuEnable[tmu] = enable; }

    static float edgeFunctionFloat(const Vec4 &a, const Vec4 &b, const Vec4 &c);

    static bool checkIfTriangleIsInBounds(const TriangleStreamTypes::StaticParams& params,
                                          const uint16_t lineStart,
                                          const uint16_t lineEnd);

    static bool increment(TriangleStreamTypes::StaticParams& params, 
                          const std::span<TriangleStreamTypes::Texture>& texture,
                          const uint16_t lineStart,
                          const uint16_t lineEnd);
private:
    static constexpr uint64_t DECIMAL_POINT = 12;
    inline static VecInt edgeFunctionFixPoint(const Vec2i &a, const Vec2i &b, const Vec2i &c);
    inline static VecInt calcRecip(VecInt val);

    int32_t m_scissorX { 0 };
    int32_t m_scissorY { 0 };
    uint32_t m_scissorWidth { 0 };
    uint32_t m_scissorHeight { 0 };
    bool m_enableScissor { false };
    bool m_enableScaling { false };
    std::bitset<IRenderer::MAX_TMU_COUNT> m_tmuEnable {};

};

} // namespace rr
#endif // RASTERIZER_HPP

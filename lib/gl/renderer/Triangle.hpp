// RasterIX
// https://github.com/ToNi3141/RasterIX
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

#ifndef _TRIANGLE_HPP_
#define _TRIANGLE_HPP_

#include "RenderConfigs.hpp"
#include "math/Vec.hpp"
#include <cstdint>
#include <tcb/span.hpp>

namespace rr
{

struct TransformedTriangle
{
    const Vec4& vertex0;
    const Vec4& vertex1;
    const Vec4& vertex2;
    const std::array<Vec4, RenderConfig::TMU_COUNT>& texture0;
    const std::array<Vec4, RenderConfig::TMU_COUNT>& texture1;
    const std::array<Vec4, RenderConfig::TMU_COUNT>& texture2;
    const Vec4& color0;
    const Vec4& color1;
    const Vec4& color2;
};

} // namespace rr

#endif // _TRIANGLE_HPP_

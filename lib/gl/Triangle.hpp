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


#ifndef _TRIANGLE_HPP_
#define _TRIANGLE_HPP_

#include <cstdint>
#include <tcb/span.hpp>
#include "Vec.hpp"

namespace rr
{

struct Triangle
{
    const Vec4& vertex0;
    const Vec4& vertex1;
    const Vec4& vertex2;
    const tcb::span<const Vec4>& texture0;
    const tcb::span<const Vec4>& texture1;
    const tcb::span<const Vec4>& texture2;
    const Vec4& color0;
    const Vec4& color1;
    const Vec4& color2;
};

} // namespace rr

#endif // _TRIANGLE_HPP_

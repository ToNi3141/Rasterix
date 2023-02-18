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


#ifndef _TRIANGLE_STREAM_TYPES_CMD_HPP_
#define _TRIANGLE_STREAM_TYPES_CMD_HPP_

#include <cstdint>
#include "Vec.hpp"
#include "Veci.hpp"

namespace rr
{

namespace TriangleStreamTypes
{

struct Texture
{
    Vec3 texStq;
    Vec3 texStqXInc;
    Vec3 texStqYInc;
};

struct StaticParams
{
    uint32_t reserved;
    uint16_t bbStartX;
    uint16_t bbStartY;
    uint16_t bbEndX;
    uint16_t bbEndY;
    Vec3i wInit;
    Vec3i wXInc;
    Vec3i wYInc;
    Vec4 color;
    Vec4 colorXInc;
    Vec4 colorYInc;
    float depthW;
    float depthWXInc;
    float depthWYInc;
    float depthZ;
    float depthZXInc;
    float depthZYInc;
};

} // namespace TriangleStreamTypes

} // namespace rr

#endif // _TRIANGLE_STREAM_TYPES_CMD_HPP_

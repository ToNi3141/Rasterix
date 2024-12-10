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
#include "RenderConfigs.hpp"
#include "math/Vec.hpp"
#include "math/Veci.hpp"

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

struct TextureX
{
    Vec3i texStq;
    Vec3i texStqXInc;
    Vec3i texStqYInc;

    void operator=(const Texture& t)
    {
        texStq.fromVec<Vec3, 28>(t.texStq);
        texStqXInc.fromVec<Vec3, 28>(t.texStqXInc);
        texStqYInc.fromVec<Vec3, 28>(t.texStqYInc);
    }
};

struct StaticParamsX
{
    uint32_t reserved;
    uint16_t bbStartX;
    uint16_t bbStartY;
    uint16_t bbEndX;
    uint16_t bbEndY;
    Vec3i wInit;
    Vec3i wXInc;
    Vec3i wYInc;
    Vec4i color;
    Vec4i colorXInc;
    Vec4i colorYInc;
    int32_t depthW;
    int32_t depthWXInc;
    int32_t depthWYInc;
    int32_t depthZ;
    int32_t depthZXInc;
    int32_t depthZYInc;

    void operator=(const StaticParams& t)
    {
        bbStartX = t.bbStartX;
        bbStartY = t.bbStartY;
        bbEndX = t.bbEndX;
        bbEndY = t.bbEndY;
        wInit = t.wInit;
        wXInc = t.wXInc;
        wYInc = t.wYInc;
        color.fromVec<Vec4, 24>(t.color);
        colorXInc.fromVec<Vec4, 24>(t.colorXInc);
        colorYInc.fromVec<Vec4, 24>(t.colorYInc);
        depthW = static_cast<int32_t>(t.depthW * (1 << 30));
        depthWXInc = static_cast<int32_t>(t.depthWXInc * (1 << 30));
        depthWYInc = static_cast<int32_t>(t.depthWYInc * (1 << 30));
        depthZ = static_cast<int32_t>(t.depthZ * (1 << 30));
        depthZXInc = static_cast<int32_t>(t.depthZXInc * (1 << 30));
        depthZYInc = static_cast<int32_t>(t.depthZYInc * (1 << 30));
    }
};

struct TriangleDesc
{
#pragma pack(push, 4)
    StaticParams param;
    std::array<Texture, RenderConfig::TMU_COUNT> texture;
#pragma pack(pop)
};

struct TriangleDescX
{
#pragma pack(push, 4)
    StaticParamsX param;
    std::array<TextureX, RenderConfig::TMU_COUNT> texture;
    void operator=(const TriangleDesc& t)
    {
        param = t.param;
        for (std::size_t i = 0; i < RenderConfig::TMU_COUNT; i++)
        {
            texture[i] = t.texture[i];
        }
    };
#pragma pack(pop)
};

} // namespace TriangleStreamTypes

} // namespace rr

#endif // _TRIANGLE_STREAM_TYPES_CMD_HPP_

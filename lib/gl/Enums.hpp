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

#ifndef _ENUMS_HPP_
#define _ENUMS_HPP_
namespace rr
{
enum class TestFunc
{
    ALWAYS,
    NEVER,
    LESS,
    EQUAL,
    LEQUAL,
    GREATER,
    NOTEQUAL,
    GEQUAL
};

enum class BlendFunc
{
    ZERO,
    ONE,
    DST_COLOR,
    SRC_COLOR,
    ONE_MINUS_DST_COLOR,
    ONE_MINUS_SRC_COLOR,
    SRC_ALPHA,
    ONE_MINUS_SRC_ALPHA,
    DST_ALPHA,
    ONE_MINUS_DST_ALPHA,
    SRC_ALPHA_SATURATE
};

enum class LogicOp
{
    CLEAR,
    SET,
    COPY,
    COPY_INVERTED,
    NOOP,
    INVERTED,
    AND,
    NAND,
    OR,
    NOR,
    XOR,
    EQUIV,
    AND_REVERSE,
    AND_INVERTED,
    OR_REVERSE,
    OR_INVERTED
};

enum class StencilOp
{
    KEEP,
    ZERO,
    REPLACE,
    INCR,
    INCR_WRAP,
    DECR,
    DECR_WRAP,
    INVERT
};

enum class Combine
{
    REPLACE,
    MODULATE,
    ADD,
    ADD_SIGNED,
    INTERPOLATE,
    SUBTRACT,
    DOT3_RGB,
    DOT3_RGBA
};

enum class Operand
{
    SRC_ALPHA,
    ONE_MINUS_SRC_ALPHA,
    SRC_COLOR,
    ONE_MINUS_SRC_COLOR
};

enum class SrcReg
{
    TEXTURE,
    CONSTANT,
    PRIMARY_COLOR,
    PREVIOUS
};

enum class TextureWrapMode : uint32_t
{
    REPEAT,
    CLAMP_TO_EDGE
};

enum class PixelFormat : uint32_t
{
    RGBA4444,
    RGBA5551,
    RGB565
};

enum class TexEnvMode
{
    DISABLE,
    REPLACE,
    MODULATE,
    DECAL,
    BLEND,
    ADD,
    COMBINE
};
} // namespace rr

#endif // _ENUMS_HPP_

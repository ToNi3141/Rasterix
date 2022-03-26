// Rasterix
// https://github.com/ToNi3141/Rasterix
// Copyright (c) 2022 ToNi3141

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

#ifndef ICEGLTYPES_H
#define ICEGLTYPES_H
#include <stdint.h>

#define __glPi 3.14159265358979323846f

typedef enum {
    GL_NO_ERROR = 0, // Error not in specification
    GL_OUT_OF_MEMORY, // Error not in specification
    GL_INVALID_ENUM,
    GL_INVALID_VALUE,
    GL_INVALID_OPERATION,
    GL_SPEC_DEVIATION,
    GL_STACK_OVERFLOW,
    GL_STACK_UNDERFLOW,

    GL_UNPACK_ALIGNMENT,
    GL_PACK_ALIGNMENT,
    GL_MODELVIEW,
    GL_PROJECTION,

    GL_TRIANGLE_FAN,
    GL_TRIANGLE_STRIP,
    GL_TRIANGLES,
    GL_QUAD_STRIP,

    GL_ALPHA,
    GL_RGB,
    GL_RGBA,
    GL_LUMINANCE,
    GL_LUMINANCE_ALPHA,
    GL_UNSIGNED_SHORT_5_6_5,
    GL_UNSIGNED_SHORT_4_4_4_4,
    GL_UNSIGNED_SHORT_5_5_5_1,
    GL_TEXTURE_MAG_FILTER,
    GL_TEXTURE_MIN_FILTER,
    GL_NEAREST,

    GL_S,
    GL_T,
    //    GL_R, // Not suppored
    //    GL_Q, // Not suppored
    GL_TEXTURE_GEN_MODE,
    GL_OBJECT_PLANE,
    GL_EYE_PLANE,
    GL_OBJECT_LINEAR,
    GL_EYE_LINEAR,
    GL_SPHERE_MAP,
    GL_TEXTURE_GEN_S,
    GL_TEXTURE_GEN_T,

    GL_LIGHT0,
    GL_LIGHT1,
    GL_LIGHT2,
    GL_LIGHT3,
    GL_LIGHT4,
    GL_LIGHT5,
    GL_LIGHT6,
    GL_LIGHT7,
    GL_LIGHT8,
    GL_MAX_LIGHTS,
    GL_LIGHTING,
    GL_AMBIENT,
    GL_DIFFUSE,
    GL_AMBIENT_AND_DIFFUSE,
    GL_SPECULAR,
    GL_EMISSION,
    GL_SHININESS,
    GL_POSITION,
    GL_SPOT_DIRECTION,
    GL_SPOT_EXPONENT,
    GL_SPOT_CUTOFF,
    GL_CONSTANT_ATTENUATION,
    GL_LINEAR_ATTENUATION,
    GL_QUADRATIC_ATTENUATION,
    GL_LIGHT_MODEL_AMBIENT,
    GL_LIGHT_MODEL_TWO_SIDE,
    GL_FRONT_AND_BACK,
    GL_COLOR_MATERIAL, // TODO: Implement

    GL_ALWAYS,
    GL_NEVER,
    GL_LESS,
    GL_EQUAL,
    GL_LEQUAL,
    GL_GREATER,
    GL_NOTEQUAL,
    GL_GEQUAL,

    GL_TEXTURE_ENV,
    GL_TEXTURE_ENV_MODE,
    GL_TEXTURE_ENV_COLOR,
    GL_REPLACE,
    GL_MODULATE,
    GL_DECAL,
    GL_BLEND,
    GL_ADD,

    GL_ZERO,
    GL_ONE,
    GL_DST_COLOR,
    GL_SRC_COLOR,
    GL_ONE_MINUS_DST_COLOR,
    GL_ONE_MINUS_SRC_COLOR,
    GL_SRC_ALPHA,
    GL_ONE_MINUS_SRC_ALPHA,
    GL_DST_ALPHA,
    GL_ONE_MINUS_DST_ALPHA,
    GL_SRC_ALPHA_SATURATE,

    GL_CLEAR,
    GL_SET,
    GL_COPY,
    GL_COPY_INVERTED,
    GL_NOOP,
    GL_INVERTED,
    GL_AND,
    GL_NAND,
    GL_OR,
    GL_NOR,
    GL_XOR,
    GL_EQUIV,
    GL_AND_REVERSE,
    GL_AND_INVERTED,
    GL_OR_REVERSE,
    GL_OR_INVERTED,

    GL_ALPHA_TEST,
    GL_DEPTH_TEST,
    GL_TEXTURE_2D,

    GL_BYTE,
    GL_SHORT,
    GL_FLOAT,
    GL_UNSIGNED_BYTE,
    GL_UNSIGNED_SHORT,

    GL_COLOR_ARRAY,
    GL_NORMAL_ARRAY,
    // GL_POINT_SIZE_ARRAY_OES, currently not supported in the current implementation
    GL_TEXTURE_COORD_ARRAY,
    GL_VERTEX_ARRAY,

    GL_MAX_MODELVIEW_STACK_DEPTH,
    GL_MAX_PROJECTION_STACK_DEPTH,
    GL_MAX_TEXTURE_SIZE,
    GL_DOUBLEBUFFER,
    GL_RED_BITS,
    GL_BLUE_BITS,
    GL_GREEN_BITS,
    GL_ALPHA_BITS,
    GL_DEPTH_BITS,
    GL_STENCIL_BITS,
    GL_VENDOR,
    GL_RENDERER,
    GL_VERSION,
    GL_EXTENSIONS,

    GL_FRONT,
    GL_BACK,

    GL_DISABLE,

    // Mocked values
    GL_CULL_FACE,
    GL_NORMALIZE,
    GL_STENCIL_TEST,
    GL_SMOOTH,

    GL_FOG,
    GL_FOG_MODE,
    GL_FOG_DENSITY,
    GL_FOG_START,
    GL_FOG_END,
    GL_FOG_COLOR,
    GL_FOG_HINT,
    GL_NICEST,
    GL_FASTEST,
    GL_EXP,
    GL_EXP2,
    GL_LINEAR,

    GL_INT,

    GL_VIEWPORT,
    GL_UNSIGNED_INT,
    GL_TEXTURE_WIDTH,
    GL_TEXTURE_HEIGHT,
    GL_NEAREST_MIPMAP_NEAREST,
    GL_LINEAR_MIPMAP_NEAREST,
    GL_NEAREST_MIPMAP_LINEAR,
    GL_LINEAR_MIPMAP_LINEAR,
    GL_TEXTURE_WRAP_T,
    GL_REPEAT,
    GL_TEXTURE_WRAP_S,
    GL_CLAMP_TO_BORDER,
    GL_CLAMP_TO_EDGE
} GLenum;

#define GL_COLOR_BUFFER_BIT     0x1
#define GL_DEPTH_BUFFER_BIT     0x2
#define GL_STENCIL_BUFFER_BIT   0x4

#define GL_TEXTURE_SIZE_32 32
#define GL_TEXTURE_SIZE_64 64
#define GL_TEXTURE_SIZE_128 128
#define GL_TEXTURE_SUPPORTED_SIZES (GL_TEXTURE_SIZE_32 | GL_TEXTURE_SIZE_64 | GL_TEXTURE_SIZE_128)

#define GL_TRUE 1
#define GL_FALSE 0

typedef double GLdouble;
typedef float GLfloat;
typedef float GLclampd;
typedef float GLclampf;
typedef int GLint;
typedef uint32_t GLuint;
typedef int GLsizei;
typedef void GLvoid;
typedef uint32_t GLbitfield;
typedef uint8_t GLubyte;
typedef uint8_t GLboolean;

#endif // ICEGLTYPES_H

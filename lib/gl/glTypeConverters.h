// Rasterix
// https://github.com/ToNi3141/Rasterix
// Copyright (c) 2024 ToNi3141

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


#ifndef GL_TYPE_CONVERTERS_H 
#define GL_TYPE_CONVERTERS_H

#include "gl.h"
#include <spdlog/spdlog.h>
#include "RRXGL.hpp"
#include <cstring>
#include "vertexpipeline/MatrixStack.hpp"
#include "vertexpipeline/Types.hpp"
#include "pixelpipeline/Texture.hpp"
#include "pixelpipeline/PixelPipeline.hpp"
#include "vertexpipeline/VertexArray.hpp"
#include "vertexpipeline/VertexQueue.hpp"

namespace rr
{

template <uint8_t ColorPos, uint8_t ComponentSize, uint8_t Mask>
uint8_t convertColorComponentToUint8(const uint16_t color)
{
    static constexpr uint8_t ComponentShift = 8 - ComponentSize;
    static constexpr uint8_t ComponentShiftFill = ComponentSize - ComponentShift;
    return (((color >> ColorPos) & Mask) << ComponentShift) | (((color >> ColorPos) & Mask) >> ComponentShiftFill);
}

GLint convertTexEnvMode(Texture::TexEnvMode& mode, const GLint param)
{
    GLint ret = GL_NO_ERROR;
    switch (param) {
//    case GL_DISABLE:
//        mode = Texture::TexEnvMode::DISABLE;
//        break;
    case GL_REPLACE:
        mode = Texture::TexEnvMode::REPLACE;
        break;
    case GL_MODULATE:
        mode = Texture::TexEnvMode::MODULATE;
        break;
    case GL_DECAL:
        mode = Texture::TexEnvMode::DECAL;
        break;
    case GL_BLEND:
        mode = Texture::TexEnvMode::BLEND;
        break;
    case GL_ADD:
        mode = Texture::TexEnvMode::ADD;
        break;
    case GL_COMBINE:
        mode = Texture::TexEnvMode::COMBINE;
        break;
    default:
        SPDLOG_WARN("convertTexEnvMode 0x{:X} not suppored", param);
        ret = GL_INVALID_ENUM;
        mode = Texture::TexEnvMode::REPLACE;
        break;
    }
    return ret;
}

GLint convertCombine(Texture::TexEnv::Combine& conv, GLint val, bool alpha)
{
    GLint ret = GL_NO_ERROR;
    switch (val) 
    {
        case GL_REPLACE:
            conv = Texture::TexEnv::Combine::REPLACE;
            break;
        case GL_MODULATE:
            conv = Texture::TexEnv::Combine::MODULATE;
            break;
        case GL_ADD:
            conv = Texture::TexEnv::Combine::ADD;
            break;
        case GL_ADD_SIGNED:
            conv = Texture::TexEnv::Combine::ADD_SIGNED;
            break;
        case GL_INTERPOLATE:
            conv = Texture::TexEnv::Combine::INTERPOLATE;
            break;
        case GL_SUBTRACT:
            conv = Texture::TexEnv::Combine::SUBTRACT;
            break;
        case GL_DOT3_RGB:
            if (alpha) 
            {
                ret = GL_INVALID_ENUM;
            }
            else 
            {
                conv = Texture::TexEnv::Combine::DOT3_RGB;
            }
            break;
        case GL_DOT3_RGBA:
            if (alpha) 
            {
                ret = GL_INVALID_ENUM;
            }
            else 
            {
                conv = Texture::TexEnv::Combine::DOT3_RGBA;
            }
            break;
        default:
            SPDLOG_WARN("convertCombine 0x{:X} 0x{:X} not suppored", val, alpha);
            ret = GL_INVALID_ENUM;
            break;
    }
    return ret;
}

GLint convertOperand(Texture::TexEnv::Operand& conf, GLint val, bool alpha)
{
    GLint ret = GL_NO_ERROR;
    switch (val)
    {
        case GL_SRC_ALPHA:
            conf = Texture::TexEnv::Operand::SRC_ALPHA;
            break;
        case GL_ONE_MINUS_SRC_ALPHA:
            conf = Texture::TexEnv::Operand::ONE_MINUS_SRC_ALPHA;
            break;
        case GL_SRC_COLOR:
            if (alpha)
            {
                ret = GL_INVALID_ENUM;
            }
            else
            {
                conf = Texture::TexEnv::Operand::SRC_COLOR;
            }
            break;
        case GL_ONE_MINUS_SRC_COLOR:
            if (alpha)
            {
                ret = GL_INVALID_ENUM;
            }
            else
            {
                conf = Texture::TexEnv::Operand::ONE_MINUS_SRC_COLOR;
            }
            break;
        default:
            SPDLOG_WARN("convertOperand 0x{:X} 0x{:X} not suppored", val, alpha);
            ret = GL_INVALID_ENUM;
    }
    return ret;
}

GLint convertSrcReg(Texture::TexEnv::SrcReg& conf, GLint val)
{
    GLint ret = GL_NO_ERROR;
    switch (val)
    {
        case GL_TEXTURE:
            conf = Texture::TexEnv::SrcReg::TEXTURE;
            break;
        case GL_CONSTANT:
            conf = Texture::TexEnv::SrcReg::CONSTANT;
            break;
        case GL_PRIMARY_COLOR:
            conf = Texture::TexEnv::SrcReg::PRIMARY_COLOR;
            break;
        case GL_PREVIOUS:
            conf = Texture::TexEnv::SrcReg::PREVIOUS;
            break;
        default:
            SPDLOG_WARN("convertSrcReg 0x{:X} not suppored", val);
            ret = GL_INVALID_ENUM;
            break;
    }
    return ret;
}


FragmentPipeline::PipelineConfig::BlendFunc convertGlBlendFuncToRenderBlendFunc(const GLenum blendFunc)
{
    switch (blendFunc) {
    case GL_ZERO:
        return FragmentPipeline::PipelineConfig::BlendFunc::ZERO;
    case GL_ONE:
        return FragmentPipeline::PipelineConfig::BlendFunc::ONE;
    case GL_DST_COLOR:
        return FragmentPipeline::PipelineConfig::BlendFunc::DST_COLOR;
    case GL_SRC_COLOR:
        return FragmentPipeline::PipelineConfig::BlendFunc::SRC_COLOR;
    case GL_ONE_MINUS_DST_COLOR:
        return FragmentPipeline::PipelineConfig::BlendFunc::ONE_MINUS_DST_COLOR;
    case GL_ONE_MINUS_SRC_COLOR:
        return FragmentPipeline::PipelineConfig::BlendFunc::ONE_MINUS_SRC_COLOR;
    case GL_SRC_ALPHA:
        return FragmentPipeline::PipelineConfig::BlendFunc::SRC_ALPHA;
    case GL_ONE_MINUS_SRC_ALPHA:
        return FragmentPipeline::PipelineConfig::BlendFunc::ONE_MINUS_SRC_ALPHA;
    case GL_DST_ALPHA:
        return FragmentPipeline::PipelineConfig::BlendFunc::DST_ALPHA;
    case GL_ONE_MINUS_DST_ALPHA:
        return FragmentPipeline::PipelineConfig::BlendFunc::ONE_MINUS_DST_ALPHA;
    case GL_SRC_ALPHA_SATURATE:
        return FragmentPipeline::PipelineConfig::BlendFunc::SRC_ALPHA_SATURATE;
    default:
        SPDLOG_WARN("convertGlBlendFuncToRenderBlendFunc 0x{:X} not suppored", blendFunc);
        RRXGL::getInstance().setError(GL_INVALID_ENUM);
        return FragmentPipeline::PipelineConfig::BlendFunc::ZERO;
    }
    RRXGL::getInstance().setError(GL_INVALID_ENUM);
    return FragmentPipeline::PipelineConfig::BlendFunc::ZERO;
}

void setClientState(const GLenum array, bool enable)
{
    switch (array) {
    case GL_COLOR_ARRAY:
        SPDLOG_DEBUG("setClientState GL_COLOR_ARRAY {}", enable);
        RRXGL::getInstance().vertexArray().enableColorArray(enable);
        break;
    case GL_NORMAL_ARRAY:
        SPDLOG_DEBUG("setClientState GL_NORMAL_ARRAY {}", enable);
        RRXGL::getInstance().vertexArray().enableNormalArray(enable);
        break;
    case GL_TEXTURE_COORD_ARRAY:
        SPDLOG_DEBUG("setClientState GL_TEXTURE_COORD_ARRAY {}", enable);
        RRXGL::getInstance().vertexArray().enableTexCoordArray(enable);
        break;
    case GL_VERTEX_ARRAY:
        SPDLOG_DEBUG("setClientState GL_VERTEX_ARRAY {}", enable);
        RRXGL::getInstance().vertexArray().enableVertexArray(enable);
        break;
    default:
        SPDLOG_WARN("setClientState 0x{:X} 0x{:X} not suppored", array, enable);
        RRXGL::getInstance().setError(GL_INVALID_ENUM);
        break;
    }
}

RenderObj::Type convertType(GLenum type)
{
    switch (type) {
    case GL_BYTE:
        return RenderObj::Type::BYTE;
    case GL_UNSIGNED_BYTE:
        return RenderObj::Type::UNSIGNED_BYTE;
    case GL_SHORT:
        return RenderObj::Type::SHORT;
    case GL_UNSIGNED_SHORT:
        return RenderObj::Type::UNSIGNED_SHORT;
    case GL_FLOAT:
        return RenderObj::Type::FLOAT;
    case GL_UNSIGNED_INT:
        return RenderObj::Type::UNSIGNED_INT;
    default:
        SPDLOG_WARN("convertType 0x{:X} not suppored", type);
        RRXGL::getInstance().setError(GL_INVALID_ENUM);
        return RenderObj::Type::BYTE;
    }
}

RenderObj::DrawMode convertDrawMode(GLenum drawMode)
{
    switch (drawMode) {
    case GL_TRIANGLES:
        return RenderObj::DrawMode::TRIANGLES;
    case GL_TRIANGLE_FAN:
        return RenderObj::DrawMode::TRIANGLE_FAN;
    case GL_TRIANGLE_STRIP:
        return RenderObj::DrawMode::TRIANGLE_STRIP;
    case GL_QUAD_STRIP:
        return RenderObj::DrawMode::QUAD_STRIP;
    case GL_QUADS:
        return RenderObj::DrawMode::QUADS;
    case GL_POLYGON:
        return RenderObj::DrawMode::POLYGON;
    case GL_LINES:
        return RenderObj::DrawMode::LINES;
    case GL_LINE_STRIP:
        return RenderObj::DrawMode::LINE_STRIP;
    case GL_LINE_LOOP:
        return RenderObj::DrawMode::LINE_LOOP;
    default:
        SPDLOG_WARN("convertDrawMode 0x{:X} not suppored", drawMode);
        RRXGL::getInstance().setError(GL_INVALID_ENUM);
        return RenderObj::DrawMode::TRIANGLES;
    }
}

Texture::TextureWrapMode convertGlTextureWrapMode(const GLenum mode)
{
    switch (mode) {
    case GL_CLAMP_TO_EDGE:
    case GL_CLAMP:
        return Texture::TextureWrapMode::CLAMP_TO_EDGE;
    case GL_REPEAT:
        return Texture::TextureWrapMode::REPEAT;
    default:
        SPDLOG_WARN("convertGlTextureWarpMode 0x{:X} not suppored", mode);
        RRXGL::getInstance().setError(GL_INVALID_ENUM);
        return Texture::TextureWrapMode::REPEAT;
    }
}

TestFunc convertTestFunc(const GLenum mode)
{
    switch (mode)
    {
    case GL_ALWAYS:
        return TestFunc::ALWAYS;
    case GL_NEVER:
        return TestFunc::NEVER;
    case GL_LESS:
        return TestFunc::LESS;
    case GL_EQUAL:
        return TestFunc::EQUAL;
    case GL_LEQUAL:
        return TestFunc::LEQUAL;
    case GL_GREATER:
        return TestFunc::GREATER;
    case GL_NOTEQUAL:
        return TestFunc::NOTEQUAL;
    case GL_GEQUAL:
        return TestFunc::GEQUAL;

    default:
        SPDLOG_WARN("convertTestFunc 0x{:X} not suppored", mode);
        RRXGL::getInstance().setError(GL_INVALID_ENUM);
        return TestFunc::ALWAYS;
    }
}

Stencil::StencilConfig::StencilOp convertStencilOp(const GLenum mode)
{
    switch (mode)
    {
    case GL_KEEP:
        return Stencil::StencilConfig::StencilOp::KEEP;
    case GL_ZERO:
        return Stencil::StencilConfig::StencilOp::ZERO;
    case GL_REPLACE:
        return Stencil::StencilConfig::StencilOp::REPLACE;
    case GL_INCR:
        return Stencil::StencilConfig::StencilOp::INCR;
    case GL_INCR_WRAP_EXT:
        return Stencil::StencilConfig::StencilOp::INCR_WRAP;
    case GL_DECR:
        return Stencil::StencilConfig::StencilOp::DECR;
    case GL_DECR_WRAP_EXT:
        return Stencil::StencilConfig::StencilOp::DECR_WRAP;
    case GL_INVERT:
        return Stencil::StencilConfig::StencilOp::INVERT;

    default:
        SPDLOG_WARN("convertStencilOp 0x{:X} not suppored", mode);
        RRXGL::getInstance().setError(GL_INVALID_ENUM);
        return Stencil::StencilConfig::StencilOp::KEEP;
    }
}

} // namespace rr

#endif // GL_TYPE_CONVERTERS_H

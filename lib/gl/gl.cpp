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


#include "gl.h"
#include <spdlog/spdlog.h>
#include "IceGL.hpp"

#pragma GCC diagnostic ignored "-Wunused-parameter"

static constexpr int32_t MAX_TEX_SIZE { 256 };

GLint convertTexEnvMode(PixelPipeline::TexEnvMode& mode, const GLint param) 
{
    GLint ret = GL_NO_ERROR;
    switch (param) {
//    case GL_DISABLE:
//        mode = PixelPipeline::TexEnvMode::DISABLE;
//        break;
    case GL_REPLACE:
        mode = PixelPipeline::TexEnvMode::REPLACE;
        break;
    case GL_MODULATE:
        mode = PixelPipeline::TexEnvMode::MODULATE;
        break;
    case GL_DECAL:
        mode = PixelPipeline::TexEnvMode::DECAL;
        break;
    case GL_BLEND:
        mode = PixelPipeline::TexEnvMode::BLEND;
        break;
    case GL_ADD:
        mode = PixelPipeline::TexEnvMode::ADD;
        break;
    case GL_COMBINE:
        mode = PixelPipeline::TexEnvMode::COMBINE;
        break;
    default:
        ret = GL_INVALID_ENUM;
        mode = PixelPipeline::TexEnvMode::REPLACE;
        break;
    }
    return ret;
}

GLint convertCombine(PixelPipeline::TexEnv::Combine& conv, GLint val, bool alpha) 
{
    GLint ret = GL_NO_ERROR;
    switch (val) 
    {
        case GL_REPLACE:
            conv = PixelPipeline::TexEnv::Combine::REPLACE;
            break;
        case GL_MODULATE:
            conv = PixelPipeline::TexEnv::Combine::MODULATE;
            break;
        case GL_ADD:
            conv = PixelPipeline::TexEnv::Combine::ADD;
            break;
        case GL_ADD_SIGNED:
            conv = PixelPipeline::TexEnv::Combine::ADD_SIGNED;
            break;
        case GL_INTERPOLATE:
            conv = PixelPipeline::TexEnv::Combine::INTERPOLATE;
            break;
        case GL_SUBTRACT:
            conv = PixelPipeline::TexEnv::Combine::SUBTRACT;
            break;
        case GL_DOT3_RGB:
            if (alpha) 
            {
                ret = GL_INVALID_ENUM;
            }
            else 
            {
                conv = PixelPipeline::TexEnv::Combine::DOT3_RGB;
            }
            break;
        case GL_DOT3_RGBA:
            if (alpha) 
            {
                ret = GL_INVALID_ENUM;
            }
            else 
            {
                conv = PixelPipeline::TexEnv::Combine::DOT3_RGBA;
            }
            break;
        default:
            ret = GL_INVALID_ENUM;
            break;
    }
    return ret;
}

GLint convertOperand(PixelPipeline::TexEnv::Operand& conf, GLint val, bool alpha)
{
    GLint ret = GL_NO_ERROR;
    switch (val)
    {
        case GL_SRC_ALPHA:
            conf = PixelPipeline::TexEnv::Operand::SRC_ALPHA;
            break;
        case GL_ONE_MINUS_SRC_ALPHA:
            conf = PixelPipeline::TexEnv::Operand::ONE_MINUS_SRC_ALPHA;
            break;
        case GL_SRC_COLOR:
            if (alpha)
            {
                ret = GL_INVALID_ENUM;
            }
            else
            {
                conf = PixelPipeline::TexEnv::Operand::SRC_COLOR;
            }
            break;
        case GL_ONE_MINUS_SRC_COLOR:
            if (alpha)
            {
                ret = GL_INVALID_ENUM;
            }
            else
            {
                conf = PixelPipeline::TexEnv::Operand::ONE_MINUS_SRC_COLOR;
            }
            break;
        default:
            ret = GL_INVALID_ENUM;
    }
    return ret;
}

GLint convertSrcReg(PixelPipeline::TexEnv::SrcReg& conf, GLint val)
{
    GLint ret = GL_NO_ERROR;
    switch (val)
    {
        case GL_TEXTURE:
            conf = PixelPipeline::TexEnv::SrcReg::TEXTURE;
            break;
        case GL_CONSTANT:
            conf = PixelPipeline::TexEnv::SrcReg::CONSTANT;
            break;
        case GL_PRIMARY_COLOR:
            conf = PixelPipeline::TexEnv::SrcReg::PRIMARY_COLOR;
            break;
        case GL_PREVIOUS:
            conf = PixelPipeline::TexEnv::SrcReg::PREVIOUS;
            break;
        default:
            ret = GL_INVALID_ENUM;
            break;
    }
    return ret;
}


PixelPipeline::BlendFunc convertGlBlendFuncToRenderBlendFunc(const GLenum blendFunc)
{
    switch (blendFunc) {
    case GL_ZERO:
        return PixelPipeline::BlendFunc::ZERO;
    case GL_ONE:
        return PixelPipeline::BlendFunc::ONE;
    case GL_DST_COLOR:
        return PixelPipeline::BlendFunc::DST_COLOR;
    case GL_SRC_COLOR:
        return PixelPipeline::BlendFunc::SRC_COLOR;
    case GL_ONE_MINUS_DST_COLOR:
        return PixelPipeline::BlendFunc::ONE_MINUS_DST_COLOR;
    case GL_ONE_MINUS_SRC_COLOR:
        return PixelPipeline::BlendFunc::ONE_MINUS_SRC_COLOR;
    case GL_SRC_ALPHA:
        return PixelPipeline::BlendFunc::SRC_ALPHA;
    case GL_ONE_MINUS_SRC_ALPHA:
        return PixelPipeline::BlendFunc::ONE_MINUS_SRC_ALPHA;
    case GL_DST_ALPHA:
        return PixelPipeline::BlendFunc::DST_ALPHA;
    case GL_ONE_MINUS_DST_ALPHA:
        return PixelPipeline::BlendFunc::ONE_MINUS_DST_ALPHA;
    case GL_SRC_ALPHA_SATURATE:
        return PixelPipeline::BlendFunc::SRC_ALPHA_SATURATE;
    default:
        IceGL::getInstance().setError(GL_INVALID_ENUM);
        return PixelPipeline::BlendFunc::ZERO;
    }
    IceGL::getInstance().setError(GL_INVALID_ENUM);
    return PixelPipeline::BlendFunc::ZERO;
}

void setClientState(const GLenum array, bool enable)
{
    switch (array) {
    case GL_COLOR_ARRAY:
        IceGL::getInstance().renderObj().enableColorArray(enable);
        break;
    case GL_NORMAL_ARRAY:
        IceGL::getInstance().renderObj().enableNormalArray(enable);
        break;
    case GL_TEXTURE_COORD_ARRAY:
        IceGL::getInstance().renderObj().enableTexCoordArray(enable);
        break;
    case GL_VERTEX_ARRAY:
        IceGL::getInstance().renderObj().enableVertexArray(enable);
        break;
    default:
        IceGL::getInstance().setError(GL_INVALID_ENUM);
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
        IceGL::getInstance().setError(GL_INVALID_ENUM);
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
        IceGL::getInstance().setError(GL_INVALID_ENUM);
        return RenderObj::DrawMode::TRIANGLES;
    }
}

PixelPipeline::TextureWrapMode convertGlTextureWrapMode(const GLenum mode)
{
    switch (mode) {
    case GL_CLAMP_TO_EDGE:
        return PixelPipeline::TextureWrapMode::CLAMP_TO_EDGE;
    case GL_REPEAT:
        return PixelPipeline::TextureWrapMode::REPEAT;
    default:
        IceGL::getInstance().setError(GL_INVALID_ENUM);
        return PixelPipeline::TextureWrapMode::REPEAT;
    }
}









GLAPI void APIENTRY glAccum(GLenum op, GLfloat value)
{
    SPDLOG_DEBUG("glAccum not implemented");
}

GLAPI void APIENTRY glAlphaFunc(GLenum func, GLclampf ref)
{
    SPDLOG_DEBUG("glAlphaFunc func {} ref {}", func, ref);

    PixelPipeline::TestFunc testFunc { PixelPipeline::TestFunc::LESS };
    IceGL::getInstance().setError(GL_NO_ERROR);
    switch (func)
    {
    case GL_ALWAYS:
        testFunc = PixelPipeline::TestFunc::ALWAYS;
        break;
    case GL_NEVER:
        testFunc = PixelPipeline::TestFunc::NEVER;
        break;
    case GL_LESS:
        testFunc = PixelPipeline::TestFunc::LESS;
        break;
    case GL_EQUAL:
        testFunc = PixelPipeline::TestFunc::EQUAL;
        break;
    case GL_LEQUAL:
        testFunc = PixelPipeline::TestFunc::LEQUAL;
        break;
    case GL_GREATER:
        testFunc = PixelPipeline::TestFunc::GREATER;
        break;
    case GL_NOTEQUAL:
        testFunc = PixelPipeline::TestFunc::NOTEQUAL;
        break;
    case GL_GEQUAL:
        testFunc = PixelPipeline::TestFunc::GEQUAL;
        break;

    default:
        IceGL::getInstance().setError(GL_INVALID_ENUM);
        break;
    }

    if (IceGL::getInstance().getError() == GL_NO_ERROR)
    {
        // Convert reference value from float to fix point
        uint8_t refFix = ref * (1 << 8);
        if (ref >= 1.0f)
        {
            refFix = 0xff;
        }
        IceGL::getInstance().pixelPipeline().fragmentPipeline().setAlphaFunc(testFunc);
        IceGL::getInstance().pixelPipeline().fragmentPipeline().setRefAlphaValue(refFix);
    }
}

GLAPI void APIENTRY glBegin(GLenum mode)
{
    SPDLOG_DEBUG("glBegin {} called", mode);
    IceGL::getInstance().vertexQueue().begin(convertDrawMode(mode));
}

GLAPI void APIENTRY glBitmap(GLsizei width, GLsizei height, GLfloat xOrig, GLfloat yOrig, GLfloat xMove, GLfloat yMove, const GLubyte *bitmap)
{
    SPDLOG_DEBUG("glBitmap not implemented");
}

GLAPI void APIENTRY glBlendFunc(GLenum srcFactor, GLenum dstFactor)
{
    SPDLOG_DEBUG("glBlendFunc srcFactor {} dstFactor {} called", srcFactor, dstFactor);
    IceGL::getInstance().setError(GL_NO_ERROR);
    if (srcFactor == GL_SRC_ALPHA_SATURATE)
    {
        IceGL::getInstance().setError(GL_INVALID_ENUM);
    }
    else
    {
        IceGL::getInstance().pixelPipeline().fragmentPipeline().setBlendFuncSFactor(convertGlBlendFuncToRenderBlendFunc(srcFactor));
        IceGL::getInstance().pixelPipeline().fragmentPipeline().setBlendFuncDFactor(convertGlBlendFuncToRenderBlendFunc(dstFactor));
    }
}

GLAPI void APIENTRY glCallList(GLuint list)
{
    SPDLOG_DEBUG("glCallList not implemented");
}

GLAPI void APIENTRY glCallLists(GLsizei n, GLenum type, const GLvoid *lists)
{
    SPDLOG_DEBUG("glCallLists not implemented");
}

GLAPI void APIENTRY glClear(GLbitfield mask)
{
    SPDLOG_DEBUG("glClear mask {} called", mask);
    // TODO GL_STENCIL_BUFFER_BIT has to be implemented
    if (IceGL::getInstance().pixelPipeline().clearFramebuffer(mask & GL_COLOR_BUFFER_BIT, mask & GL_DEPTH_BUFFER_BIT))
    {
        IceGL::getInstance().setError(GL_NO_ERROR);
    }
    else
    {
        SPDLOG_ERROR("glClear Out Of Memory");
    }
}

GLAPI void APIENTRY glClearAccum(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
    SPDLOG_DEBUG("glClearAccum not implemented");
}

GLAPI void APIENTRY glClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
{
    SPDLOG_DEBUG("glClearColor ({}, {}, {}, {}) called", red, green, blue, alpha);
    if (IceGL::getInstance().pixelPipeline().setClearColor({ { red, green, blue, alpha } }))
    {
        IceGL::getInstance().setError(GL_NO_ERROR);
    }
    else
    {
        SPDLOG_ERROR("glClearColor Out Of Memory");
    }
}

GLAPI void APIENTRY glClearDepth(GLclampd depth)
{
    SPDLOG_DEBUG("glClearDepth {} called", depth);
    if (depth < -1.0f)
    {
        depth = -1.0f;
    }
        
    if (depth > 1.0f)
    {
        depth = 1.0f;
    }

    if (IceGL::getInstance().pixelPipeline().setClearDepth(depth))
    {
        IceGL::getInstance().setError(GL_NO_ERROR);
    }
    else
    {
        SPDLOG_ERROR("glClearDepth Out Of Memory");
    }
}

GLAPI void APIENTRY glClearIndex(GLfloat c)
{
    SPDLOG_DEBUG("glClearIndex not implemented");
}

GLAPI void APIENTRY glClearStencil(GLint s)
{
    SPDLOG_DEBUG("glClearStencil not implemented");
}

GLAPI void APIENTRY glClipPlane(GLenum plane, const GLdouble *equation)
{
    SPDLOG_DEBUG("glClipPlane not implemented");
}

GLAPI void APIENTRY glColor3b(GLbyte red, GLbyte green, GLbyte blue)
{
    SPDLOG_DEBUG("glColor3b ({}, {}, {}) called", red, green, blue);
    IceGL::getInstance().vertexQueue().setColor({ {
        (static_cast<float>(red) / 127.0f), 
        (static_cast<float>(green) / 127.0f), 
        (static_cast<float>(blue) / 127.0f), 
        1.0f } });
}

GLAPI void APIENTRY glColor3bv(const GLbyte *v)
{
    SPDLOG_DEBUG("glColor3bv ({}, {}, {}) called", v[0], v[1], v[2]);
    IceGL::getInstance().vertexQueue().setColor({ {
        (static_cast<float>(v[0]) / 127.0f), 
        (static_cast<float>(v[1]) / 127.0f), 
        (static_cast<float>(v[2]) / 127.0f), 
        1.0f } });
}

GLAPI void APIENTRY glColor3d(GLdouble red, GLdouble green, GLdouble blue)
{
    SPDLOG_DEBUG("glColor3d ({}, {}, {}) called", static_cast<float>(red), static_cast<float>(green), static_cast<float>(blue));
    IceGL::getInstance().vertexQueue().setColor({ { static_cast<float>(red), static_cast<float>(green), static_cast<float>(blue), 1.0f } });
}

GLAPI void APIENTRY glColor3dv(const GLdouble *v)
{
    SPDLOG_DEBUG("glColor3dv ({}, {}, {}) called", static_cast<float>(v[0]), static_cast<float>(v[1]), static_cast<float>(v[2]));
    IceGL::getInstance().vertexQueue().setColor({ { static_cast<float>(v[0]), static_cast<float>(v[1]), static_cast<float>(v[2]), 1.0f } });
}

GLAPI void APIENTRY glColor3f(GLfloat red, GLfloat green, GLfloat blue)
{
    SPDLOG_DEBUG("glColor3f ({}, {}, {}) called", red, green, blue);
    IceGL::getInstance().vertexQueue().setColor({ { red, green, blue, 1.0f } });
}

GLAPI void APIENTRY glColor3fv(const GLfloat *v)
{
    SPDLOG_DEBUG("glColor3fv ({}, {}, {}) called", v[0], v[1], v[2]);
    IceGL::getInstance().vertexQueue().setColor({ { v[0], v[1], v[2], 1.0f } });
}

GLAPI void APIENTRY glColor3i(GLint red, GLint green, GLint blue)
{
    SPDLOG_DEBUG("glColor3i ({}, {}, {}) called", red, green, blue);
    IceGL::getInstance().vertexQueue().setColor({ {
        (static_cast<float>(red) / 255.0f), 
        (static_cast<float>(green) / 255.0f), 
        (static_cast<float>(blue) / 255.0f), 
        1.0f } });
}

GLAPI void APIENTRY glColor3iv(const GLint *v)
{
    SPDLOG_DEBUG("glColor3iv ({}, {}, {}) called", v[0], v[1], v[2]);
    IceGL::getInstance().vertexQueue().setColor({ {
        (static_cast<float>(v[0]) / 255.0f), 
        (static_cast<float>(v[1]) / 255.0f), 
        (static_cast<float>(v[2]) / 255.0f), 
        1.0f } });
}

GLAPI void APIENTRY glColor3s(GLshort red, GLshort green, GLshort blue)
{
    SPDLOG_DEBUG("glColor3s ({}, {}, {}) called", red, green, blue);
    IceGL::getInstance().vertexQueue().setColor({ {
        (static_cast<float>(red) / 255.0f), 
        (static_cast<float>(green) / 255.0f), 
        (static_cast<float>(blue) / 255.0f), 
        1.0f } });
}

GLAPI void APIENTRY glColor3sv(const GLshort *v)
{
    SPDLOG_DEBUG("glColor3sv ({}, {}, {}) called", v[0], v[1], v[2]);
    IceGL::getInstance().vertexQueue().setColor({ {
        (static_cast<float>(v[0]) / 255.0f), 
        (static_cast<float>(v[1]) / 255.0f), 
        (static_cast<float>(v[2]) / 255.0f), 
        1.0f } });
}

GLAPI void APIENTRY glColor3ub(GLubyte red, GLubyte green, GLubyte blue)
{
    SPDLOG_DEBUG("glColor3ub ({}, {}, {}) called", red, green, blue);
    IceGL::getInstance().vertexQueue().setColor({ {
        (static_cast<float>(red) / 255.0f), 
        (static_cast<float>(green) / 255.0f), 
        (static_cast<float>(blue) / 255.0f), 
        1.0f } });
}

GLAPI void APIENTRY glColor3ubv(const GLubyte *v)
{
    SPDLOG_DEBUG("glColor3ubv ({}, {}, {}) called", v[0], v[1], v[2]);
    IceGL::getInstance().vertexQueue().setColor({ {
        (static_cast<float>(v[0]) / 255.0f), 
        (static_cast<float>(v[1]) / 255.0f), 
        (static_cast<float>(v[2]) / 255.0f), 
        1.0f } });
}

GLAPI void APIENTRY glColor3ui(GLuint red, GLuint green, GLuint blue)
{
    SPDLOG_DEBUG("glColor3ui ({}, {}, {}) called", red, green, blue);
    IceGL::getInstance().vertexQueue().setColor({ {
        (static_cast<float>(red) / 255.0f), 
        (static_cast<float>(green) / 255.0f), 
        (static_cast<float>(blue) / 255.0f), 
        1.0f } });
}

GLAPI void APIENTRY glColor3uiv(const GLuint *v)
{
    SPDLOG_DEBUG("glColor3uiv ({}, {}, {}) called", v[0], v[1], v[2]);
    IceGL::getInstance().vertexQueue().setColor({ {
        (static_cast<float>(v[0]) / 255.0f), 
        (static_cast<float>(v[1]) / 255.0f), 
        (static_cast<float>(v[2]) / 255.0f), 
        1.0f } });
}

GLAPI void APIENTRY glColor3us(GLushort red, GLushort green, GLushort blue)
{
    SPDLOG_DEBUG("glColor3us ({}, {}, {}) called", red, green, blue);
    IceGL::getInstance().vertexQueue().setColor({ {
        (static_cast<float>(red) / 255.0f), 
        (static_cast<float>(green) / 255.0f), 
        (static_cast<float>(blue) / 255.0f), 
        1.0f } });
}

GLAPI void APIENTRY glColor3usv(const GLushort *v)
{
    SPDLOG_DEBUG("glColor3usv ({}, {}, {}) called", v[0], v[1], v[2]);
    IceGL::getInstance().vertexQueue().setColor({ {
        (static_cast<float>(v[0]) / 255.0f), 
        (static_cast<float>(v[1]) / 255.0f), 
        (static_cast<float>(v[2]) / 255.0f), 
        1.0f } });
}

GLAPI void APIENTRY glColor4b(GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha)
{
    SPDLOG_DEBUG("glColor4b ({}, {}, {}, {}) called", red, green, blue, alpha);
    IceGL::getInstance().vertexQueue().setColor({ {
        (static_cast<float>(red) / 127.0f), 
        (static_cast<float>(green) / 127.0f), 
        (static_cast<float>(blue) / 127.0f), 
        (static_cast<float>(alpha) / 127.0f) } });
}

GLAPI void APIENTRY glColor4bv(const GLbyte *v)
{
    SPDLOG_DEBUG("glColor4bv ({}, {}, {}, {}) called", v[0], v[1], v[2], v[3]);
    IceGL::getInstance().vertexQueue().setColor({ {
        (static_cast<float>(v[0]) / 127.0f), 
        (static_cast<float>(v[1]) / 127.0f), 
        (static_cast<float>(v[2]) / 127.0f), 
        (static_cast<float>(v[3]) / 127.0f) } });

}

GLAPI void APIENTRY glColor4d(GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha)
{
    SPDLOG_DEBUG("glColor4d ({}, {}, {}, {}) called", static_cast<float>(red), static_cast<float>(green), static_cast<float>(blue), static_cast<float>(alpha));
    IceGL::getInstance().vertexQueue().setColor({ { static_cast<float>(red), static_cast<float>(green), static_cast<float>(blue), static_cast<float>(alpha) } });
}

GLAPI void APIENTRY glColor4dv(const GLdouble *v)
{
    SPDLOG_DEBUG("glColor4dv ({}, {}, {}, {}) called", static_cast<float>(v[0]), static_cast<float>(v[1]), static_cast<float>(v[2]), static_cast<float>(v[3]));
    IceGL::getInstance().vertexQueue().setColor({ { static_cast<float>(v[0]), static_cast<float>(v[1]), static_cast<float>(v[2]), static_cast<float>(v[3]) } });
}

GLAPI void APIENTRY glColor4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
    SPDLOG_DEBUG("glColor4f ({}, {}, {}, {}) called", red, green, blue, alpha);
    IceGL::getInstance().vertexQueue().setColor({ { red, green, blue, alpha } });
}

GLAPI void APIENTRY glColor4fv(const GLfloat *v)
{
    SPDLOG_DEBUG("glColor4fv ({}, {}, {}, {}) called", v[0], v[1], v[2], v[3]);
    IceGL::getInstance().vertexQueue().setColor({ { v[0], v[1], v[2], v[3] } });
}

GLAPI void APIENTRY glColor4i(GLint red, GLint green, GLint blue, GLint alpha)
{
    SPDLOG_DEBUG("glColor4i ({}, {}, {}, {}) called", red, green, blue, alpha);
    IceGL::getInstance().vertexQueue().setColor({ {
        (static_cast<float>(red) / 255.0f), 
        (static_cast<float>(green) / 255.0f), 
        (static_cast<float>(blue) / 255.0f), 
        (static_cast<float>(alpha) / 255.0f) } });
}

GLAPI void APIENTRY glColor4iv(const GLint *v)
{
    SPDLOG_DEBUG("glColor4iv ({}, {}, {}, {}) called", v[0], v[1], v[2], v[3]);
    IceGL::getInstance().vertexQueue().setColor({ {
        (static_cast<float>(v[0]) / 255.0f), 
        (static_cast<float>(v[1]) / 255.0f), 
        (static_cast<float>(v[2]) / 255.0f), 
        (static_cast<float>(v[3]) / 255.0f) } });
}

GLAPI void APIENTRY glColor4s(GLshort red, GLshort green, GLshort blue, GLshort alpha)
{
    SPDLOG_DEBUG("glColor4s ({}, {}, {}, {}) called", red, green, blue, alpha);
    IceGL::getInstance().vertexQueue().setColor({ {
        (static_cast<float>(red) / 255.0f), 
        (static_cast<float>(green) / 255.0f), 
        (static_cast<float>(blue) / 255.0f), 
        (static_cast<float>(alpha) / 255.0f) } });
}

GLAPI void APIENTRY glColor4sv(const GLshort *v)
{
    SPDLOG_DEBUG("glColor4sv ({}, {}, {}, {}) called", v[0], v[1], v[2], v[3]);
    IceGL::getInstance().vertexQueue().setColor({ {
        (static_cast<float>(v[0]) / 255.0f), 
        (static_cast<float>(v[1]) / 255.0f), 
        (static_cast<float>(v[2]) / 255.0f), 
        (static_cast<float>(v[3]) / 255.0f) } });
}

GLAPI void APIENTRY glColor4ub(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha)
{
    SPDLOG_DEBUG("glColor4ub ({}, {}, {}, {}) called", red, green, blue, alpha);
    IceGL::getInstance().vertexQueue().setColor({ {
        (static_cast<float>(red) / 255.0f), 
        (static_cast<float>(green) / 255.0f), 
        (static_cast<float>(blue) / 255.0f), 
        (static_cast<float>(alpha) / 255.0f) } });
}

GLAPI void APIENTRY glColor4ubv(const GLubyte *v)
{
    SPDLOG_DEBUG("glColor4ubv ({}, {}, {}, {}) called", v[0], v[1], v[2], v[3]);
    IceGL::getInstance().vertexQueue().setColor({ {
        (static_cast<float>(v[0]) / 255.0f), 
        (static_cast<float>(v[1]) / 255.0f), 
        (static_cast<float>(v[2]) / 255.0f), 
        (static_cast<float>(v[3]) / 255.0f) } });
}

GLAPI void APIENTRY glColor4ui(GLuint red, GLuint green, GLuint blue, GLuint alpha)
{
    SPDLOG_DEBUG("glColor4ui ({}, {}, {}, {}) called", red, green, blue, alpha);
    IceGL::getInstance().vertexQueue().setColor({ {
        (static_cast<float>(red) / 255.0f), 
        (static_cast<float>(green) / 255.0f), 
        (static_cast<float>(blue) / 255.0f), 
        (static_cast<float>(alpha) / 255.0f) } });
}

GLAPI void APIENTRY glColor4uiv(const GLuint *v)
{
    SPDLOG_DEBUG("glColor4uiv ({}, {}, {}, {}) called", v[0], v[1], v[2], v[3]);
    IceGL::getInstance().vertexQueue().setColor({ {
        (static_cast<float>(v[0]) / 255.0f), 
        (static_cast<float>(v[1]) / 255.0f), 
        (static_cast<float>(v[2]) / 255.0f), 
        (static_cast<float>(v[3]) / 255.0f) } });
}

GLAPI void APIENTRY glColor4us(GLushort red, GLushort green, GLushort blue, GLushort alpha)
{
    SPDLOG_DEBUG("glColor4us ({}, {}, {}, {}) called", red, green, blue, alpha);
    IceGL::getInstance().vertexQueue().setColor({ {
        (static_cast<float>(red) / 255.0f), 
        (static_cast<float>(green) / 255.0f), 
        (static_cast<float>(blue) / 255.0f), 
        (static_cast<float>(alpha) / 255.0f) } });
}

GLAPI void APIENTRY glColor4usv(const GLushort *v)
{
    SPDLOG_DEBUG("glColor4usv ({}, {}, {}, {}) called", v[0], v[1], v[2], v[3]);
    IceGL::getInstance().vertexQueue().setColor({ {
        (static_cast<float>(v[0]) / 255.0f), 
        (static_cast<float>(v[1]) / 255.0f), 
        (static_cast<float>(v[2]) / 255.0f), 
        (static_cast<float>(v[3]) / 255.0f) } });
}

GLAPI void APIENTRY glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
{
    SPDLOG_DEBUG("glColorMask red {} green {} blue {} alpha {} called", red, green, blue, alpha);
    IceGL::getInstance().pixelPipeline().fragmentPipeline().setColorMaskR(red == GL_TRUE);
    IceGL::getInstance().pixelPipeline().fragmentPipeline().setColorMaskG(green == GL_TRUE);
    IceGL::getInstance().pixelPipeline().fragmentPipeline().setColorMaskB(blue == GL_TRUE);
    IceGL::getInstance().pixelPipeline().fragmentPipeline().setColorMaskA(alpha == GL_TRUE);
}

GLAPI void APIENTRY glColorMaterial(GLenum face, GLenum mode)
{
    SPDLOG_DEBUG("glColorMaterial face {} mode {} called", face, mode);
    IceGL::getInstance().setError(GL_NO_ERROR);
    if (face == GL_FRONT_AND_BACK)
    {
        VertexPipeline::Face faceConverted {};
        switch (face) {
            case GL_FRONT:
                faceConverted = VertexPipeline::Face::FRONT;
                break;
            case GL_BACK:
                faceConverted = VertexPipeline::Face::BACK;
                break;
            case GL_FRONT_AND_BACK:
                faceConverted = VertexPipeline::Face::FRONT_AND_BACK;
                break;
            default:
                faceConverted = VertexPipeline::Face::FRONT_AND_BACK;
                IceGL::getInstance().setError(GL_INVALID_ENUM);
                break;
        }

        if (IceGL::getInstance().getError() != GL_INVALID_ENUM)
        {
            switch (mode) {
            case GL_AMBIENT:
                IceGL::getInstance().vertexPipeline().setColorMaterialTracking(faceConverted, VertexPipeline::ColorMaterialTracking::AMBIENT);
                break;
            case GL_DIFFUSE:
                IceGL::getInstance().vertexPipeline().setColorMaterialTracking(faceConverted, VertexPipeline::ColorMaterialTracking::DIFFUSE);
                break;
            case GL_AMBIENT_AND_DIFFUSE:
                IceGL::getInstance().vertexPipeline().setColorMaterialTracking(faceConverted, VertexPipeline::ColorMaterialTracking::AMBIENT_AND_DIFFUSE);
                break;
            case GL_SPECULAR:
                IceGL::getInstance().vertexPipeline().setColorMaterialTracking(faceConverted, VertexPipeline::ColorMaterialTracking::SPECULAR);
                break;
            case GL_EMISSION:
                IceGL::getInstance().vertexPipeline().setColorMaterialTracking(faceConverted, VertexPipeline::ColorMaterialTracking::EMISSION);
                break;
            default:
                IceGL::getInstance().setError(GL_INVALID_ENUM);
                IceGL::getInstance().vertexPipeline().setColorMaterialTracking(faceConverted, VertexPipeline::ColorMaterialTracking::AMBIENT_AND_DIFFUSE);
                break;
            }
        }
    }
}

GLAPI void APIENTRY glCopyPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum type)
{
    SPDLOG_DEBUG("glCopyPixels not implemented");
}

GLAPI void APIENTRY glCullFace(GLenum mode)
{
    SPDLOG_DEBUG("glCullFace mode {} called", mode);

    switch (mode) {
    case GL_BACK:
        IceGL::getInstance().vertexPipeline().setCullMode(VertexPipeline::Face::BACK);
        break;
    case GL_FRONT:
        IceGL::getInstance().vertexPipeline().setCullMode(VertexPipeline::Face::FRONT);
        break;
    case GL_FRONT_AND_BACK:
        IceGL::getInstance().vertexPipeline().setCullMode(VertexPipeline::Face::FRONT_AND_BACK);
        break;
    default:
        IceGL::getInstance().setError(GL_INVALID_ENUM);
        break;
    }
}

GLAPI void APIENTRY glDeleteLists(GLuint list, GLsizei range)
{
    SPDLOG_DEBUG("glDeleteLists not implemented");
}

GLAPI void APIENTRY glDepthFunc(GLenum func)
{
    SPDLOG_DEBUG("glDepthFunc {} called", func);
    PixelPipeline::TestFunc testFunc { PixelPipeline::TestFunc::LESS };
    IceGL::getInstance().setError(GL_NO_ERROR);
    switch (func)
    {
    case GL_ALWAYS:
        testFunc = PixelPipeline::TestFunc::ALWAYS;
        break;
    case GL_NEVER:
        testFunc = PixelPipeline::TestFunc::NEVER;
        break;
    case GL_LESS:
        testFunc = PixelPipeline::TestFunc::LESS;
        break;
    case GL_EQUAL:
        testFunc = PixelPipeline::TestFunc::EQUAL;
        break;
    case GL_LEQUAL:
        testFunc = PixelPipeline::TestFunc::LEQUAL;
        break;
    case GL_GREATER:
        testFunc = PixelPipeline::TestFunc::GREATER;
        break;
    case GL_NOTEQUAL:
        testFunc = PixelPipeline::TestFunc::NOTEQUAL;
        break;
    case GL_GEQUAL:
        testFunc = PixelPipeline::TestFunc::GEQUAL;
        break;

    default:
        IceGL::getInstance().setError(GL_INVALID_ENUM);
        break;
    }

    if (IceGL::getInstance().getError() == GL_NO_ERROR)
    {
        IceGL::getInstance().pixelPipeline().fragmentPipeline().setDepthFunc(testFunc);
    }
}

GLAPI void APIENTRY glDepthMask(GLboolean flag)
{
    SPDLOG_DEBUG("glDepthMask flag {} called", flag);
    IceGL::getInstance().pixelPipeline().fragmentPipeline().setDepthMask(flag == GL_TRUE);
}

GLAPI void APIENTRY glDepthRange(GLclampd zNear, GLclampd zFar)
{
    SPDLOG_DEBUG("glDepthRange zNear {} zFar{} called", zNear, zFar);
    if (zNear > 1.0f) zNear = 1.0f;
    if (zNear < -1.0f) zNear = -1.0f;
    if (zFar  > 1.0f) zFar  = 1.0f;
    if (zFar  < -1.0f) zFar  = -1.0f;

    IceGL::getInstance().vertexPipeline().setDepthRange(zNear, zFar);
}

GLAPI void APIENTRY glDisable(GLenum cap)
{
    SPDLOG_DEBUG("glDisable {} called", cap);
    switch (cap)
    {
    case GL_TEXTURE_2D:
    {
        IceGL::getInstance().pixelPipeline().featureEnable().setEnableTmu0(false);
        break;
    }
    case GL_ALPHA_TEST:
    {
        IceGL::getInstance().pixelPipeline().featureEnable().setEnableAlphaTest(false);
        break;
    }
    case GL_DEPTH_TEST:
    {
        IceGL::getInstance().pixelPipeline().featureEnable().setEnableDepthTest(false);
        break;
    }
    case GL_BLEND:
    {
        IceGL::getInstance().pixelPipeline().featureEnable().setEnableBlending(false);
        break;
    }
    case GL_LIGHTING:
        IceGL::getInstance().vertexPipeline().getLighting().enableLighting(false);
        break;
    case GL_LIGHT0:
    case GL_LIGHT1:
    case GL_LIGHT2:
    case GL_LIGHT3:
    case GL_LIGHT4:
    case GL_LIGHT5:
    case GL_LIGHT6:
    case GL_LIGHT7:
        IceGL::getInstance().vertexPipeline().getLighting().enableLight(cap - GL_LIGHT0, false);
        break;
    case GL_TEXTURE_GEN_S:
        IceGL::getInstance().vertexPipeline().getTexGen().enableTexGenS(false);
        break;
    case GL_TEXTURE_GEN_T:
        IceGL::getInstance().vertexPipeline().getTexGen().enableTexGenT(false);
        break;
    case GL_CULL_FACE:
        IceGL::getInstance().vertexPipeline().enableCulling(false);
        break;
    case GL_COLOR_MATERIAL:
        IceGL::getInstance().vertexPipeline().enableColorMaterial(false);
        break;
    case GL_FOG:
        IceGL::getInstance().pixelPipeline().featureEnable().setEnableFog(false);
        break;
    case GL_SCISSOR_TEST:
        IceGL::getInstance().pixelPipeline().featureEnable().setEnableScissor(false);
        break;
    default:
        SPDLOG_WARN("glDisable cap {} not supported", cap);
        IceGL::getInstance().setError(GL_INVALID_ENUM);
        break;
    }
}

GLAPI void APIENTRY glDrawBuffer(GLenum mode)
{
    SPDLOG_DEBUG("glDrawBuffer not implemented");
}

GLAPI void APIENTRY glDrawPixels(GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels)
{
    SPDLOG_DEBUG("glDrawPixels not implemented");
}

GLAPI void APIENTRY glEdgeFlag(GLboolean flag)
{
    SPDLOG_DEBUG("glEdgeFlag not implemented");
}

GLAPI void APIENTRY glEdgeFlagv(const GLboolean *flag)
{
    SPDLOG_DEBUG("glEdgeFlagv not implemented");
}

GLAPI void APIENTRY glEnable(GLenum cap)
{
    SPDLOG_DEBUG("glEnable {} called", cap);

    switch (cap)
    {
    case GL_TEXTURE_2D:
        IceGL::getInstance().pixelPipeline().featureEnable().setEnableTmu0(true);
        break;
    case GL_ALPHA_TEST:
        IceGL::getInstance().pixelPipeline().featureEnable().setEnableAlphaTest(true);
        break;
    case GL_DEPTH_TEST:
        IceGL::getInstance().pixelPipeline().featureEnable().setEnableDepthTest(true);
        break;
    case GL_BLEND:
        IceGL::getInstance().pixelPipeline().featureEnable().setEnableBlending(true);
        break;
    case GL_LIGHTING:
        IceGL::getInstance().vertexPipeline().getLighting().enableLighting(true);
        break;
    case GL_LIGHT0:
    case GL_LIGHT1:
    case GL_LIGHT2:
    case GL_LIGHT3:
    case GL_LIGHT4:
    case GL_LIGHT5:
    case GL_LIGHT6:
    case GL_LIGHT7:
        IceGL::getInstance().vertexPipeline().getLighting().enableLight(cap - GL_LIGHT0, true);
        break;
    case GL_TEXTURE_GEN_S:
        IceGL::getInstance().vertexPipeline().getTexGen().enableTexGenS(true);
        break;
    case GL_TEXTURE_GEN_T:
        IceGL::getInstance().vertexPipeline().getTexGen().enableTexGenT(true);
        break;
    case GL_CULL_FACE:
        IceGL::getInstance().vertexPipeline().enableCulling(true);
        break;
    case GL_COLOR_MATERIAL:
        IceGL::getInstance().vertexPipeline().enableColorMaterial(true);
        break;
    case GL_FOG:
        IceGL::getInstance().pixelPipeline().featureEnable().setEnableFog(true);
        break;
    case GL_SCISSOR_TEST:
        IceGL::getInstance().pixelPipeline().featureEnable().setEnableScissor(true);
        break;
    default:
        SPDLOG_WARN("glEnable cap {} not supported", cap);
        IceGL::getInstance().setError(GL_INVALID_ENUM);
        break;
    }
}

GLAPI void APIENTRY glEnd(void)
{
    SPDLOG_DEBUG("glEnd called");
    IceGL::getInstance().vertexQueue().end();
    IceGL::getInstance().vertexPipeline().drawObj(IceGL::getInstance().vertexQueue().renderObj());
}

GLAPI void APIENTRY glEndList(void)
{
    SPDLOG_DEBUG("glEndList not implemented");
}

GLAPI void APIENTRY glEvalCoord1d(GLdouble u)
{
    SPDLOG_DEBUG("glEvalCoord1d not implemented");
}

GLAPI void APIENTRY glEvalCoord1dv(const GLdouble *u)
{
    SPDLOG_DEBUG("glEvalCoord1dv not implemented");
}

GLAPI void APIENTRY glEvalCoord1f(GLfloat u)
{
    SPDLOG_DEBUG("glEvalCoord1f not implemented");
}

GLAPI void APIENTRY glEvalCoord1fv(const GLfloat *u)
{
    SPDLOG_DEBUG("glEvalCoord1fv not implemented");
}

GLAPI void APIENTRY glEvalCoord2d(GLdouble u, GLdouble v)
{
    SPDLOG_DEBUG("glEvalCoord2d not implemented");
}

GLAPI void APIENTRY glEvalCoord2dv(const GLdouble *u)
{
    SPDLOG_DEBUG("glEvalCoord2dv not implemented");
}

GLAPI void APIENTRY glEvalCoord2f(GLfloat u, GLfloat v)
{
    SPDLOG_DEBUG("glEvalCoord2f not implemented");
}

GLAPI void APIENTRY glEvalCoord2fv(const GLfloat *u)
{
    SPDLOG_DEBUG("glEvalCoord2fv not implemented");
}

GLAPI void APIENTRY glEvalMesh1(GLenum mode, GLint i1, GLint i2)
{
    SPDLOG_DEBUG("glEvalMesh1 not implemented");
}

GLAPI void APIENTRY glEvalMesh2(GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2)
{
    SPDLOG_DEBUG("glEvalMesh2 not implemented");
}

GLAPI void APIENTRY glEvalPoint1(GLint i)
{
    SPDLOG_DEBUG("glEvalPoint1 not implemented");
}

GLAPI void APIENTRY glEvalPoint2(GLint i, GLint j)
{
    SPDLOG_DEBUG("glEvalPoint2 not implemented");
}

GLAPI void APIENTRY glFeedbackBuffer(GLsizei size, GLenum type, GLfloat *buffer)
{
    SPDLOG_DEBUG("glFeedbackBuffer not implemented");
}

GLAPI void APIENTRY glFinish(void)
{
    SPDLOG_DEBUG("glFinish not implemented");
}

GLAPI void APIENTRY glFlush(void)
{
    SPDLOG_DEBUG("glFlush not implemented");
}

GLAPI void APIENTRY glFogf(GLenum pname, GLfloat param)
{
    SPDLOG_DEBUG("glFogf pname {} param {} called", pname, param);
    IceGL::getInstance().setError(GL_NO_ERROR);
    switch (pname) {
    case GL_FOG_MODE:
        switch (static_cast<GLenum>(param)) {
            case GL_EXP:
                IceGL::getInstance().pixelPipeline().setFogMode(PixelPipeline::FogMode::EXP);
                break;
            case GL_EXP2:
                IceGL::getInstance().pixelPipeline().setFogMode(PixelPipeline::FogMode::EXP2);
                break;
            case GL_LINEAR:
                IceGL::getInstance().pixelPipeline().setFogMode(PixelPipeline::FogMode::LINEAR);
                break;
            default:
                IceGL::getInstance().setError(GL_INVALID_ENUM);
        }
        break;
    case GL_FOG_DENSITY:
        if (param >= 0.0f)
        {
            IceGL::getInstance().pixelPipeline().setFogDensity(param);
        }
        else
        {
            IceGL::getInstance().setError(GL_INVALID_VALUE);
        }
        break;
    case GL_FOG_START:
        IceGL::getInstance().pixelPipeline().setFogStart(param);
        break;
    case GL_FOG_END:
        IceGL::getInstance().pixelPipeline().setFogEnd(param);
        break;
    default:
        IceGL::getInstance().setError(GL_INVALID_ENUM);
        break;
    }
}

GLAPI void APIENTRY glFogfv(GLenum pname, const GLfloat *params)
{
    SPDLOG_DEBUG("glFogfv {} called", pname);
    IceGL::getInstance().setError(GL_NO_ERROR);
    switch (pname) {
    case GL_FOG_MODE:
    case GL_FOG_DENSITY:
    case GL_FOG_START:
    case GL_FOG_END:
        SPDLOG_DEBUG("glFogfv redirected to glFogf");
        glFogf(pname, *params);
        break;
    case GL_FOG_COLOR:
    {
        Vec4 fogColor {};
        fogColor.fromArray(params, 4);
        IceGL::getInstance().pixelPipeline().setFogColor({ { fogColor[0], fogColor[1], fogColor[2], fogColor[3] } });
    }
        break;
    default:
        IceGL::getInstance().setError(GL_INVALID_ENUM);
        break;
    }
}

GLAPI void APIENTRY glFogi(GLenum pname, GLint param)
{
    SPDLOG_DEBUG("glFogi redirected to glFogf");
    glFogf(pname, static_cast<float>(param));
}

GLAPI void APIENTRY glFogiv(GLenum pname, const GLint *params)
{
    SPDLOG_DEBUG("glFogiv not implemented");
    IceGL::getInstance().setError(GL_NO_ERROR);
    switch (pname) {
    case GL_FOG_MODE:
    case GL_FOG_DENSITY:
    case GL_FOG_START:
    case GL_FOG_END:
        SPDLOG_DEBUG("glFogiv redirected to glFogi");
        glFogi(pname, *params);
        break;
    case GL_FOG_COLOR:
    {
        Vec4 fogColor {};
        fogColor.fromArray(params, 4);
        fogColor.div(255);
        IceGL::getInstance().pixelPipeline().setFogColor({ { fogColor[0], fogColor[1], fogColor[2], fogColor[3] } });
        break;
    }
    default:
        IceGL::getInstance().setError(GL_INVALID_ENUM);
        break;
    }
}

GLAPI void APIENTRY glFrontFace(GLenum mode)
{
    SPDLOG_DEBUG("glFrontFace not implemented");
}

GLAPI void APIENTRY glFrustum(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar)
{
    SPDLOG_DEBUG("glFrustum not implemented");
}

GLAPI GLuint APIENTRY glGenLists(GLsizei range)
{
    SPDLOG_DEBUG("glGenLists not implemented");
    return 0;
}

GLAPI void APIENTRY glGetBooleanv(GLenum pname, GLboolean *params)
{
    SPDLOG_DEBUG("glGetBooleanv redirected to glGetIntegerv");
    GLint tmp;
    glGetIntegerv(pname, &tmp);
    *params = tmp;
}

GLAPI void APIENTRY glGetClipPlane(GLenum plane, GLdouble *equation)
{
    SPDLOG_DEBUG("glGetClipPlane not implemented");
}

GLAPI void APIENTRY glGetDoublev(GLenum pname, GLdouble *params)
{
    SPDLOG_DEBUG("glGetDoublev not implemented");
}

GLAPI GLenum APIENTRY glGetError(void)
{
    SPDLOG_DEBUG("glGetError not implemented");
    return GL_INVALID_ENUM;
}

GLAPI void APIENTRY glGetFloatv(GLenum pname, GLfloat *params)
{
    SPDLOG_DEBUG("glGetFloatv redirected to glGetIntegerv");
    GLint tmp;
    glGetIntegerv(pname, &tmp);
    *params = tmp;
}

GLAPI void APIENTRY glGetIntegerv(GLenum pname, GLint *params)
{
    SPDLOG_DEBUG("glGetIntegerv pname {} called", pname);
    switch (pname) {
    case GL_MAX_LIGHTS:
        *params = IceGL::getInstance().vertexPipeline().getLighting().MAX_LIGHTS;
        break;
    case GL_MAX_MODELVIEW_STACK_DEPTH:
        *params = VertexPipeline::getModelMatrixStackDepth();
        break;
    case GL_MAX_PROJECTION_STACK_DEPTH:
        *params = VertexPipeline::getProjectionMatrixStackDepth();
        break;
    case GL_MAX_TEXTURE_SIZE:
        *params = IceGL::MAX_TEX_SIZE;
        break;
    case GL_DOUBLEBUFFER:
        *params = 1;
        break;
    case GL_RED_BITS:
    case GL_GREEN_BITS:
    case GL_BLUE_BITS:
    case GL_ALPHA_BITS:
        *params = 4;
        break;
    case GL_DEPTH_BITS:
        *params = 16;
        break;
    case GL_STENCIL_BITS:
        *params = 0;
        break;
    default:
        *params = 0;
        break;
    }
}

GLAPI void APIENTRY glGetLightfv(GLenum light, GLenum pname, GLfloat *params)
{
    SPDLOG_DEBUG("glGetLightfv not implemented");
}

GLAPI void APIENTRY glGetLightiv(GLenum light, GLenum pname, GLint *params)
{
    SPDLOG_DEBUG("glGetLightiv not implemented");
}

GLAPI void APIENTRY glGetMapdv(GLenum target, GLenum query, GLdouble *v)
{
    SPDLOG_DEBUG("glGetMapdv not implemented");
}

GLAPI void APIENTRY glGetMapfv(GLenum target, GLenum query, GLfloat *v)
{
    SPDLOG_DEBUG("glGetMapfv not implemented");
}

GLAPI void APIENTRY glGetMapiv(GLenum target, GLenum query, GLint *v)
{
    SPDLOG_DEBUG("glGetMapiv not implemented");
}

GLAPI void APIENTRY glGetMaterialfv(GLenum face, GLenum pname, GLfloat *params)
{
    SPDLOG_DEBUG("glGetMaterialfv not implemented");
}

GLAPI void APIENTRY glGetMaterialiv(GLenum face, GLenum pname, GLint *params)
{
    SPDLOG_DEBUG("glGetMaterialiv not implemented");
}

GLAPI void APIENTRY glGetPixelMapfv(GLenum map, GLfloat *values)
{
    SPDLOG_DEBUG("glGetPixelMapfv not implemented");
}

GLAPI void APIENTRY glGetPixelMapuiv(GLenum map, GLuint *values)
{
    SPDLOG_DEBUG("glGetPixelMapuiv not implemented");
}

GLAPI void APIENTRY glGetPixelMapusv(GLenum map, GLushort *values)
{
    SPDLOG_DEBUG("glGetPixelMapusv not implemented");
}

GLAPI void APIENTRY glGetPolygonStipple(GLubyte *mask)
{
    SPDLOG_DEBUG("glGetPolygonStipple not implemented");
}

GLAPI const GLubyte * APIENTRY glGetString(GLenum name)
{
    SPDLOG_DEBUG("glGetString {} called", name);

    switch (name) {
    case GL_VENDOR:
        return reinterpret_cast<const GLubyte*>("ToNi3141");
    case GL_RENDERER:
        return reinterpret_cast<const GLubyte*>("Rasterix");
    case GL_VERSION:
        return reinterpret_cast<const GLubyte*>("0.1");
    case GL_EXTENSIONS:
        return reinterpret_cast<const GLubyte*>("");
    default:
        break;
    }
    return reinterpret_cast<const GLubyte*>("");
}

GLAPI void APIENTRY glGetTexEnvfv(GLenum target, GLenum pname, GLfloat *params)
{
    SPDLOG_DEBUG("glGetTexEnvfv not implemented");
}

GLAPI void APIENTRY glGetTexEnviv(GLenum target, GLenum pname, GLint *params)
{
    SPDLOG_DEBUG("glGetTexEnviv not implemented");
}

GLAPI void APIENTRY glGetTexGendv(GLenum coord, GLenum pname, GLdouble *params)
{
    SPDLOG_DEBUG("glGetTexGendv not implemented");
}

GLAPI void APIENTRY glGetTexGenfv(GLenum coord, GLenum pname, GLfloat *params)
{
    SPDLOG_DEBUG("glGetTexGenfv not implemented");
}

GLAPI void APIENTRY glGetTexGeniv(GLenum coord, GLenum pname, GLint *params)
{
    SPDLOG_DEBUG("glGetTexGeniv not implemented");
}

GLAPI void APIENTRY glGetTexImage(GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels)
{
    SPDLOG_DEBUG("glGetTexImage not implemented");
}

GLAPI void APIENTRY glGetTexLevelParameterfv(GLenum target, GLint level, GLenum pname, GLfloat *params)
{
    SPDLOG_DEBUG("glGetTexLevelParameterfv not implemented");
}

GLAPI void APIENTRY glGetTexLevelParameteriv(GLenum target, GLint level, GLenum pname, GLint *params)
{
    SPDLOG_DEBUG("glGetTexLevelParameteriv not implemented");
}

GLAPI void APIENTRY glGetTexParameterfv(GLenum target, GLenum pname, GLfloat *params)
{
    SPDLOG_DEBUG("glGetTexParameterfv not implemented");
}

GLAPI void APIENTRY glGetTexParameteriv(GLenum target, GLenum pname, GLint *params)
{
    SPDLOG_DEBUG("glGetTexParameteriv not implemented");
}

GLAPI void APIENTRY glHint(GLenum target, GLenum mode)
{
    SPDLOG_DEBUG("glHint not implemented");
}

GLAPI void APIENTRY glIndexMask(GLuint mask)
{
    SPDLOG_DEBUG("glIndexMask not implemented");
}

GLAPI void APIENTRY glIndexd(GLdouble c)
{
    SPDLOG_DEBUG("glIndexd not implemented");
}

GLAPI void APIENTRY glIndexdv(const GLdouble *c)
{
    SPDLOG_DEBUG("glIndexdv not implemented");
}

GLAPI void APIENTRY glIndexf(GLfloat c)
{
    SPDLOG_DEBUG("glIndexf not implemented");
}

GLAPI void APIENTRY glIndexfv(const GLfloat *c)
{
    SPDLOG_DEBUG("glIndexfv not implemented");
}

GLAPI void APIENTRY glIndexi(GLint c)
{
    SPDLOG_DEBUG("glIndexi not implemented");
}

GLAPI void APIENTRY glIndexiv(const GLint *c)
{
    SPDLOG_DEBUG("glIndexiv not implemented");
}

GLAPI void APIENTRY glIndexs(GLshort c)
{
    SPDLOG_DEBUG("glIndexs not implemented");
}

GLAPI void APIENTRY glIndexsv(const GLshort *c)
{
    SPDLOG_DEBUG("glIndexsv not implemented");
}

GLAPI void APIENTRY glInitNames(void)
{
    SPDLOG_DEBUG("glInitNames not implemented");
}

GLAPI GLboolean APIENTRY glIsEnabled(GLenum cap)
{
    SPDLOG_DEBUG("glIsEnabled not implemented");
    return false;
}

GLAPI GLboolean APIENTRY glIsList(GLuint list)
{
    SPDLOG_DEBUG("glIsList not implemented");
    return false;
}

GLAPI void APIENTRY glLightModelf(GLenum pname, GLfloat param)
{
    SPDLOG_DEBUG("glLightModelf not implemented");
}

GLAPI void APIENTRY glLightModelfv(GLenum pname, const GLfloat *params)
{
    SPDLOG_DEBUG("glLightModelfv pname {} called", pname);

    if (pname == GL_LIGHT_MODEL_AMBIENT)
    {
        IceGL::getInstance().vertexPipeline().getLighting().setAmbientColorScene({ params });
    }
    else
    {
        glLightModelf(pname, params[0]);
    }
}

GLAPI void APIENTRY glLightModeli(GLenum pname, GLint param)
{
    SPDLOG_DEBUG("glLightModeli not implemented");
}

GLAPI void APIENTRY glLightModeliv(GLenum pname, const GLint *params)
{
    SPDLOG_DEBUG("glLightModeliv redirected to glLightModefv");
    Vec4 color {};
    color.fromArray(params, 4);
    color.div(255);
    glLightModelfv(pname, color.vec.data());
}

GLAPI void APIENTRY glLightf(GLenum light, GLenum pname, GLfloat param)
{
    SPDLOG_DEBUG("glLightf light {} pname {} param {} called", light - GL_LIGHT0, pname, param);
    
    if (light > GL_LIGHT7)
    {
        IceGL::getInstance().setError(GL_INVALID_ENUM);
    }   

    switch (pname) {
    case GL_SPOT_EXPONENT:
        IceGL::getInstance().setError(GL_INVALID_ENUM);
        SPDLOG_DEBUG("glLightf GL_SPOT_EXPONENT not implemented");
        break;
    case GL_SPOT_CUTOFF:
        IceGL::getInstance().setError(GL_INVALID_ENUM);
        SPDLOG_DEBUG("glLightf GL_SPOT_CUTOFF not implemented");
        break;
    case GL_CONSTANT_ATTENUATION:
        IceGL::getInstance().vertexPipeline().getLighting().setConstantAttenuationLight(light - GL_LIGHT0, param);
        break;
    case GL_LINEAR_ATTENUATION:
        IceGL::getInstance().vertexPipeline().getLighting().setLinearAttenuationLight(light - GL_LIGHT0, param);
        break;
    case GL_QUADRATIC_ATTENUATION:
        IceGL::getInstance().vertexPipeline().getLighting().setQuadraticAttenuationLight(light - GL_LIGHT0, param);
        break;
    default:
        IceGL::getInstance().setError(GL_INVALID_ENUM);
        break;
    }
}

GLAPI void APIENTRY glLightfv(GLenum light, GLenum pname, const GLfloat *params)
{
    SPDLOG_DEBUG("glLightfv light {} pname {}", light - GL_LIGHT0, pname);

    if (light > GL_LIGHT7)
    {
        IceGL::getInstance().setError(GL_INVALID_ENUM);
        return;
    }   

    switch (pname) {
    case GL_AMBIENT:
        IceGL::getInstance().vertexPipeline().getLighting().setAmbientColorLight(light - GL_LIGHT0, {params});
        break;
    case GL_DIFFUSE:
        IceGL::getInstance().vertexPipeline().getLighting().setDiffuseColorLight(light - GL_LIGHT0, {params});
        break;
    case GL_SPECULAR:
        IceGL::getInstance().vertexPipeline().getLighting().setSpecularColorLight(light - GL_LIGHT0, {params});
        break;
    case GL_POSITION:
    {
        Vec4 lightPos { params };
        Vec4 lightPosTransformed {};
        IceGL::getInstance().vertexPipeline().getModelMatrix().transform(lightPosTransformed, lightPos);
        IceGL::getInstance().vertexPipeline().getLighting().setPosLight(light - GL_LIGHT0, lightPosTransformed);
        break;
    }
    case GL_SPOT_DIRECTION:
        SPDLOG_DEBUG("glLightfv GL_SPOT_DIRECTION not implemented");
        IceGL::getInstance().setError(GL_INVALID_ENUM);
        break;
    default:
        SPDLOG_DEBUG("glLightfv redirected to glLightf");
        glLightf(light, pname, params[0]);
        break;
    }
}

GLAPI void APIENTRY glLighti(GLenum light, GLenum pname, GLint param)
{
    SPDLOG_DEBUG("glLighti redirected to glLightf");
    glLightf(light, pname, static_cast<float>(param));
}

GLAPI void APIENTRY glLightiv(GLenum light, GLenum pname, const GLint *params)
{
    SPDLOG_DEBUG("glLightiv redirected to glLightfv");
    Vec4 color {};
    color.fromArray(params, 4);
    color.div(255);
    glLightfv(light, pname, color.vec.data());
}

GLAPI void APIENTRY glLineStipple(GLint factor, GLushort pattern)
{
    SPDLOG_DEBUG("glLineStipple not implemented");
}

GLAPI void APIENTRY glLineWidth(GLfloat width)
{
    SPDLOG_DEBUG("glLineWidth {} called", width);
    if (width <= 0.0f)
    {
        IceGL::getInstance().setError(GL_INVALID_VALUE);
    }
    IceGL::getInstance().vertexPipeline().setLineWidth(width);
}

GLAPI void APIENTRY glListBase(GLuint base)
{
    SPDLOG_DEBUG("glListBase not implemented");
}

GLAPI void APIENTRY glLoadIdentity(void)
{
    SPDLOG_DEBUG("glLoadIdentity called");
    IceGL::getInstance().vertexPipeline().loadIdentity();
}

GLAPI void APIENTRY glLoadMatrixd(const GLdouble *m)
{
    SPDLOG_DEBUG("glLoadMatrixd not implemented");
}

GLAPI void APIENTRY glLoadMatrixf(const GLfloat *m)
{
    SPDLOG_DEBUG("glLoadMatrixf not implemented");
}

GLAPI void APIENTRY glLoadName(GLuint name)
{
    SPDLOG_DEBUG("glLoadName not implemented");
}

GLAPI void APIENTRY glLogicOp(GLenum opcode)
{
    SPDLOG_DEBUG("glLogicOp {} not implemented", opcode);

    PixelPipeline::LogicOp logicOp { PixelPipeline::LogicOp::COPY };
    switch (opcode) {
    case GL_CLEAR:
        logicOp = PixelPipeline::LogicOp::CLEAR;
        break;
    case GL_SET:
        logicOp = PixelPipeline::LogicOp::SET;
        break;
    case GL_COPY:
        logicOp = PixelPipeline::LogicOp::COPY;
        break;
    case GL_COPY_INVERTED:
        logicOp = PixelPipeline::LogicOp::COPY_INVERTED;
        break;
    case GL_NOOP:
        logicOp = PixelPipeline::LogicOp::NOOP;
        break;
//    case GL_INVERTED:
//        logicOp = PixelPipeline::LogicOp::INVERTED;
//        break;
    case GL_AND:
        logicOp = PixelPipeline::LogicOp::AND;
        break;
    case GL_NAND:
        logicOp = PixelPipeline::LogicOp::NAND;
        break;
    case GL_OR:
        logicOp = PixelPipeline::LogicOp::OR;
        break;
    case GL_NOR:
        logicOp = PixelPipeline::LogicOp::NOR;
        break;
    case GL_XOR:
        logicOp = PixelPipeline::LogicOp::XOR;
        break;
    case GL_EQUIV:
        logicOp = PixelPipeline::LogicOp::EQUIV;
        break;
    case GL_AND_REVERSE:
        logicOp = PixelPipeline::LogicOp::AND_REVERSE;
        break;
    case GL_AND_INVERTED:
        logicOp = PixelPipeline::LogicOp::AND_INVERTED;
        break;
    case GL_OR_REVERSE:
        logicOp = PixelPipeline::LogicOp::OR_REVERSE;
        break;
    case GL_OR_INVERTED:
        logicOp = PixelPipeline::LogicOp::OR_INVERTED;
        break;
    default:
        logicOp = PixelPipeline::LogicOp::COPY;
        break;
    }
    // pixelPipeline().setLogicOp(setLogicOp); // TODO: Not yet implemented
}

GLAPI void APIENTRY glMap1d(GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble *points)
{
    SPDLOG_DEBUG("glMap1d not implemented");
}

GLAPI void APIENTRY glMap1f(GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *points)
{
    SPDLOG_DEBUG("glMap1f not implemented");
}

GLAPI void APIENTRY glMap2d(GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble *points)
{
    SPDLOG_DEBUG("glMap2d not implemented");
}

GLAPI void APIENTRY glMap2f(GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat *points)
{
    SPDLOG_DEBUG("glMap2f not implemented");
}

GLAPI void APIENTRY glMapGrid1d(GLint un, GLdouble u1, GLdouble u2)
{
    SPDLOG_DEBUG("glMapGrid1d not implemented");
}

GLAPI void APIENTRY glMapGrid1f(GLint un, GLfloat u1, GLfloat u2)
{
    SPDLOG_DEBUG("glMapGrid1f not implemented");
}

GLAPI void APIENTRY glMapGrid2d(GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2)
{
    SPDLOG_DEBUG("glMapGrid2d not implemented");
}

GLAPI void APIENTRY glMapGrid2f(GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2)
{
    SPDLOG_DEBUG("glMapGrid2f not implemented");
}

GLAPI void APIENTRY glMaterialf(GLenum face, GLenum pname, GLfloat param)
{
    SPDLOG_DEBUG("glMaterialf face {} pname {} param {} called", face, pname, param);
    IceGL::getInstance().setError(GL_NO_ERROR);
    if (face == GL_FRONT_AND_BACK)
    {
        if ((pname == GL_SHININESS) && (param >= 0.0f) && (param <= 128.0f))
        {
            IceGL::getInstance().vertexPipeline().getLighting().setSpecularExponentMaterial(param);
        }
        else 
        {
            IceGL::getInstance().setError(GL_INVALID_ENUM);
        }
    }
    else 
    {
        IceGL::getInstance().setError(GL_INVALID_ENUM);
    }
}

GLAPI void APIENTRY glMaterialfv(GLenum face, GLenum pname, const GLfloat *params)
{
    SPDLOG_DEBUG("glMaterialfv face {} pname {} called", face, pname);
    IceGL::getInstance().setError(GL_NO_ERROR);
    if (face == GL_FRONT_AND_BACK)
    {
        switch (pname) {
        case GL_AMBIENT:
            IceGL::getInstance().vertexPipeline().getLighting().setAmbientColorMaterial({params});
            break;
        case GL_DIFFUSE:
            IceGL::getInstance().vertexPipeline().getLighting().setDiffuseColorMaterial({params});
            break;
        case GL_AMBIENT_AND_DIFFUSE:
            IceGL::getInstance().vertexPipeline().getLighting().setAmbientColorMaterial({params});
            IceGL::getInstance().vertexPipeline().getLighting().setDiffuseColorMaterial({params});
            break;
        case GL_SPECULAR:
            IceGL::getInstance().vertexPipeline().getLighting().setSpecularColorMaterial({params});
            break;
        case GL_EMISSION:
            IceGL::getInstance().vertexPipeline().getLighting().setEmissiveColorMaterial({params});
            break;
        default:
            glMaterialf(face, pname, params[0]);
            break;
        }
    }
    else
    {
        IceGL::getInstance().setError(GL_INVALID_ENUM);
    }
}

GLAPI void APIENTRY glMateriali(GLenum face, GLenum pname, GLint param)
{
    SPDLOG_DEBUG("glMateriali redirected to glMaterialf");
    glMaterialf(face, pname, static_cast<float>(param));
}

GLAPI void APIENTRY glMaterialiv(GLenum face, GLenum pname, const GLint *params)
{
    SPDLOG_DEBUG("glMaterialiv redirected to glMaterialfv");
    Vec4 color {};
    color.fromArray(params, 4);
    color.div(255);
    glMaterialfv(face, pname, color.vec.data());
}

GLAPI void APIENTRY glMatrixMode(GLenum mode)
{
    SPDLOG_DEBUG("glMatrixMode {} called", mode);
    IceGL::getInstance().setError(GL_NO_ERROR);
    if (mode == GL_MODELVIEW)
    {
        IceGL::getInstance().vertexPipeline().setMatrixMode(VertexPipeline::MatrixMode::MODELVIEW);
    }
    else if (mode == GL_PROJECTION)
    {
        IceGL::getInstance().vertexPipeline().setMatrixMode(VertexPipeline::MatrixMode::PROJECTION);
    }
    else
    {
        IceGL::getInstance().setError(GL_INVALID_ENUM);
    }
}

GLAPI void APIENTRY glMultMatrixd(const GLdouble *m)
{
    SPDLOG_DEBUG("glMultMatrixd redirected to glMultMatrixf");
    GLfloat mf[16];
    for (uint32_t i = 0; i < 4*4; i++)
    {
        mf[i] = m[i];
    }
    glMultMatrixf(mf);
}

GLAPI void APIENTRY glMultMatrixf(const GLfloat *m)
{
    SPDLOG_DEBUG("glMultMatrixf called");
    const Mat44 *m44 = reinterpret_cast<const Mat44*>(m);
    IceGL::getInstance().vertexPipeline().multiply(*m44);
}

GLAPI void APIENTRY glNewList(GLuint list, GLenum mode)
{
    SPDLOG_DEBUG("glNewList not implemented");
}

GLAPI void APIENTRY glNormal3b(GLbyte nx, GLbyte ny, GLbyte nz)
{
    SPDLOG_DEBUG("glNormal3b ({}, {}, {}) called", static_cast<float>(nx), static_cast<float>(ny), static_cast<float>(nz));
    IceGL::getInstance().vertexQueue().setNormal({ { static_cast<float>(nx), static_cast<float>(ny), static_cast<float>(nz) } });
}

GLAPI void APIENTRY glNormal3bv(const GLbyte *v)
{
    SPDLOG_DEBUG("glNormal3bv ({}, {}, {}) called", static_cast<float>(v[0]), static_cast<float>(v[1]), static_cast<float>(v[2]));
    IceGL::getInstance().vertexQueue().setNormal({ { static_cast<float>(v[0]), static_cast<float>(v[1]), static_cast<float>(v[2]) } });
}

GLAPI void APIENTRY glNormal3d(GLdouble nx, GLdouble ny, GLdouble nz)
{
    SPDLOG_DEBUG("glNormal3d ({}, {}, {}) called", static_cast<float>(nx), static_cast<float>(ny), static_cast<float>(nz));
    IceGL::getInstance().vertexQueue().setNormal({ { static_cast<float>(nx), static_cast<float>(ny), static_cast<float>(nz) } });
}

GLAPI void APIENTRY glNormal3dv(const GLdouble *v)
{
    SPDLOG_DEBUG("glNormal3dv ({}, {}, {}) called", static_cast<float>(v[0]), static_cast<float>(v[1]), static_cast<float>(v[2]));
    IceGL::getInstance().vertexQueue().setNormal({ { static_cast<float>(v[0]), static_cast<float>(v[1]), static_cast<float>(v[2]) } });
}

GLAPI void APIENTRY glNormal3f(GLfloat nx, GLfloat ny, GLfloat nz)
{
    SPDLOG_DEBUG("glNormal3f ({}, {}, {}) called", nx, ny, nz);
    IceGL::getInstance().vertexQueue().setNormal({ { nx, ny, nz } });
}

GLAPI void APIENTRY glNormal3fv(const GLfloat *v)
{
    SPDLOG_DEBUG("glNormal3f ({}, {}, {}) called", v[0], v[1], v[2]);
    IceGL::getInstance().vertexQueue().setNormal({ { v[0], v[1], v[2] } });
}

GLAPI void APIENTRY glNormal3i(GLint nx, GLint ny, GLint nz)
{
    SPDLOG_DEBUG("glNormal3i ({}, {}, {}) called", static_cast<float>(nx), static_cast<float>(ny), static_cast<float>(nz));
    IceGL::getInstance().vertexQueue().setNormal({ { static_cast<float>(nx), static_cast<float>(ny), static_cast<float>(nz) } });
}

GLAPI void APIENTRY glNormal3iv(const GLint *v)
{
    SPDLOG_DEBUG("glNormal3iv ({}, {}, {}) called", static_cast<float>(v[0]), static_cast<float>(v[1]), static_cast<float>(v[2]));
    IceGL::getInstance().vertexQueue().setNormal({ { static_cast<float>(v[0]), static_cast<float>(v[1]), static_cast<float>(v[2]) } });
}

GLAPI void APIENTRY glNormal3s(GLshort nx, GLshort ny, GLshort nz)
{
    SPDLOG_DEBUG("glNormal3s ({}, {}, {}) called", static_cast<float>(nx), static_cast<float>(ny), static_cast<float>(nz));
    IceGL::getInstance().vertexQueue().setNormal({ { static_cast<float>(nx), static_cast<float>(ny), static_cast<float>(nz) } });
}

GLAPI void APIENTRY glNormal3sv(const GLshort *v)
{
    SPDLOG_DEBUG("glNormal3sv ({}, {}, {}) called", static_cast<float>(v[0]), static_cast<float>(v[1]), static_cast<float>(v[2]));
    IceGL::getInstance().vertexQueue().setNormal({ { static_cast<float>(v[0]), static_cast<float>(v[1]), static_cast<float>(v[2]) } });
}

GLAPI void APIENTRY glOrthof(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar)
{
    SPDLOG_DEBUG("glOrthof left {} right {} bottom {} top {} zNear {} zFar {} called", left, right, bottom, top, zNear, zFar);

    if ((zNear == zFar) || (left == right) || (top == bottom))
    {
        IceGL::getInstance().setError(GL_INVALID_VALUE);
        return;
    }

    Mat44 o {{{
        {2.0f / (right - left),              0.0f,                               0.0f,                           0.0f}, // Col 0
        {0.0f,                               2.0f / (top - bottom),              0.0f,                           0.0f}, // Col 1
        {0.0f,                               0.0f,                               -2.0f / (zFar - zNear),           0.0f}, // Col 2
        {-((right + left) / (right - left)), -((top + bottom) / (top - bottom)), -((zFar + zNear) / (zFar - zNear)), 1.0f}  // Col 3
    }}};
    glMultMatrixf(&o[0][0]);
}

GLAPI void APIENTRY glOrtho(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar)
{
    SPDLOG_DEBUG("glOrtho redirected to glOrthof");
    glOrthof(static_cast<float>(left),
        static_cast<float>(right),
        static_cast<float>(bottom),
        static_cast<float>(top),
        static_cast<float>(zNear),
        static_cast<float>(zFar)
    );
}

GLAPI void APIENTRY glPassThrough(GLfloat token)
{
    SPDLOG_DEBUG("glPassThrough not implemented");
}

GLAPI void APIENTRY glPixelMapfv(GLenum map, GLsizei mapsize, const GLfloat *values)
{
    SPDLOG_DEBUG("glPixelMapfv not implemented");
}

GLAPI void APIENTRY glPixelMapuiv(GLenum map, GLsizei mapsize, const GLuint *values)
{
    SPDLOG_DEBUG("glPixelMapuiv not implemented");
}

GLAPI void APIENTRY glPixelMapusv(GLenum map, GLsizei mapsize, const GLushort *values)
{
    SPDLOG_DEBUG("glPixelMapusv not implemented");
}

GLAPI void APIENTRY glPixelStoref(GLenum pname, GLfloat param)
{
    SPDLOG_DEBUG("glPixelStoref not implemented");
}

GLAPI void APIENTRY glPixelStorei(GLenum pname, GLint param)
{
    SPDLOG_DEBUG("glPixelStorei pname {} param {} called", pname, param);
    IceGL::getInstance().setError(GL_NO_ERROR); 

    if (pname == GL_PACK_ALIGNMENT)
    {
        IceGL::getInstance().setError(GL_INVALID_ENUM); 
        return;
    }

    if (pname == GL_UNPACK_ALIGNMENT)
    {
        switch (param)
        {
        case 1:
        case 2:
        case 4:
        case 8:
            // m_unpackAlignment = param;
            break;
        default:
            IceGL::getInstance().setError(GL_INVALID_VALUE); 
            break;
        }
    }
    else
    {
        IceGL::getInstance().setError(GL_INVALID_ENUM); 
    }
}

GLAPI void APIENTRY glPixelTransferf(GLenum pname, GLfloat param)
{
    SPDLOG_DEBUG("glPixelTransferf not implemented");
}

GLAPI void APIENTRY glPixelTransferi(GLenum pname, GLint param)
{
    SPDLOG_DEBUG("glPixelTransferi not implemented");
}

GLAPI void APIENTRY glPixelZoom(GLfloat xfactor, GLfloat yfactor)
{
    SPDLOG_DEBUG("glPixelZoom not implemented");
}

GLAPI void APIENTRY glPointSize(GLfloat size)
{
    SPDLOG_DEBUG("glPointSize not implemented");
}

GLAPI void APIENTRY glPolygonMode(GLenum face, GLenum mode)
{
    SPDLOG_DEBUG("glPolygonMode not implemented");
}

GLAPI void APIENTRY glPolygonStipple(const GLubyte *mask)
{
    SPDLOG_DEBUG("glPolygonStipple not implemented");
}

GLAPI void APIENTRY glPopAttrib(void)
{
    SPDLOG_DEBUG("glPopAttrib not implemented");
}

GLAPI void APIENTRY glPopMatrix(void)
{
    SPDLOG_DEBUG("glPopMatrix called");
    if (IceGL::getInstance().vertexPipeline().popMatrix())
    {
        IceGL::getInstance().setError(GL_NO_ERROR);
    }
    else
    {
        IceGL::getInstance().setError(GL_STACK_OVERFLOW);
    }
}

GLAPI void APIENTRY glPopName(void)
{
    SPDLOG_DEBUG("glPopName not implemented");
}

GLAPI void APIENTRY glPushAttrib(GLbitfield mask)
{
    SPDLOG_DEBUG("glPushAttrib not implemented");
}

GLAPI void APIENTRY glPushMatrix(void)
{
    SPDLOG_DEBUG("glPushMatrix called");
    
    if (IceGL::getInstance().vertexPipeline().pushMatrix())
    {
        IceGL::getInstance().setError(GL_NO_ERROR);
    }
    else
    {
        IceGL::getInstance().setError(GL_STACK_OVERFLOW);
    }
}

GLAPI void APIENTRY glPushName(GLuint name)
{
    SPDLOG_DEBUG("glPushName not implemented");
}

GLAPI void APIENTRY glRasterPos2d(GLdouble x, GLdouble y)
{
    SPDLOG_DEBUG("glRasterPos2d not implemented");
}

GLAPI void APIENTRY glRasterPos2dv(const GLdouble *v)
{
    SPDLOG_DEBUG("glRasterPos2dv not implemented");
}

GLAPI void APIENTRY glRasterPos2f(GLfloat x, GLfloat y)
{
    SPDLOG_DEBUG("glRasterPos2f not implemented");
}

GLAPI void APIENTRY glRasterPos2fv(const GLfloat *v)
{
    SPDLOG_DEBUG("glRasterPos2fv not implemented");
}

GLAPI void APIENTRY glRasterPos2i(GLint x, GLint y)
{
    SPDLOG_DEBUG("glRasterPos2i not implemented");
}

GLAPI void APIENTRY glRasterPos2iv(const GLint *v)
{
    SPDLOG_DEBUG("glRasterPos2iv not implemented");
}

GLAPI void APIENTRY glRasterPos2s(GLshort x, GLshort y)
{
    SPDLOG_DEBUG("glRasterPos2s not implemented");
}

GLAPI void APIENTRY glRasterPos2sv(const GLshort *v)
{
    SPDLOG_DEBUG("glRasterPos2sv not implemented");
}

GLAPI void APIENTRY glRasterPos3d(GLdouble x, GLdouble y, GLdouble z)
{
    SPDLOG_DEBUG("glRasterPos3d not implemented");
}

GLAPI void APIENTRY glRasterPos3dv(const GLdouble *v)
{
    SPDLOG_DEBUG("glRasterPos3dv not implemented");
}

GLAPI void APIENTRY glRasterPos3f(GLfloat x, GLfloat y, GLfloat z)
{
    SPDLOG_DEBUG("glRasterPos3f not implemented");
}

GLAPI void APIENTRY glRasterPos3fv(const GLfloat *v)
{
    SPDLOG_DEBUG("glRasterPos3fv not implemented");
}

GLAPI void APIENTRY glRasterPos3i(GLint x, GLint y, GLint z)
{
    SPDLOG_DEBUG("glRasterPos3i not implemented");
}

GLAPI void APIENTRY glRasterPos3iv(const GLint *v)
{
    SPDLOG_DEBUG("glRasterPos3iv not implemented");
}

GLAPI void APIENTRY glRasterPos3s(GLshort x, GLshort y, GLshort z)
{
    SPDLOG_DEBUG("glRasterPos3s not implemented");
}

GLAPI void APIENTRY glRasterPos3sv(const GLshort *v)
{
    SPDLOG_DEBUG("glRasterPos3sv not implemented");
}

GLAPI void APIENTRY glRasterPos4d(GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
    SPDLOG_DEBUG("glRasterPos4d not implemented");
}

GLAPI void APIENTRY glRasterPos4dv(const GLdouble *v)
{
    SPDLOG_DEBUG("glRasterPos4dv not implemented");
}

GLAPI void APIENTRY glRasterPos4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    SPDLOG_DEBUG("glRasterPos4f not implemented");
}

GLAPI void APIENTRY glRasterPos4fv(const GLfloat *v)
{
    SPDLOG_DEBUG("glRasterPos4fv not implemented");
}

GLAPI void APIENTRY glRasterPos4i(GLint x, GLint y, GLint z, GLint w)
{
    SPDLOG_DEBUG("glRasterPos4i not implemented");
}

GLAPI void APIENTRY glRasterPos4iv(const GLint *v)
{
    SPDLOG_DEBUG("glRasterPos4iv not implemented");
}

GLAPI void APIENTRY glRasterPos4s(GLshort x, GLshort y, GLshort z, GLshort w)
{
    SPDLOG_DEBUG("glRasterPos4s not implemented");
}

GLAPI void APIENTRY glRasterPos4sv(const GLshort *v)
{
    SPDLOG_DEBUG("glRasterPos4sv not implemented");
}

GLAPI void APIENTRY glReadBuffer(GLenum mode)
{
    SPDLOG_DEBUG("glReadBuffer not implemented");
}

GLAPI void APIENTRY glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels)
{
    SPDLOG_DEBUG("glReadPixels not implemented");
}

GLAPI void APIENTRY glRectd(GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2)
{
    SPDLOG_DEBUG("glRectd redirected to glRectf");
    glRectf(static_cast<float>(x1), static_cast<float>(y1), static_cast<float>(x2), static_cast<float>(y2));
}

GLAPI void APIENTRY glRectdv(const GLdouble *v1, const GLdouble *v2)
{
    SPDLOG_DEBUG("glRectdv not implemented");
}

GLAPI void APIENTRY glRectf(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2)
{
    SPDLOG_DEBUG("glRectf constructed with glBegin glVertex2f glEnd");

    glBegin(GL_TRIANGLE_FAN); // The spec says, use GL_POLYGON. GL_POLYGON is not implemented. GL_TRIANGLE_FAN should also do the trick.
    glVertex2f(x1, y1);
    glVertex2f(x2, y1);
    glVertex2f(x2, y2);
    glVertex2f(x1, y2);
    glEnd();
}

GLAPI void APIENTRY glRectfv(const GLfloat *v1, const GLfloat *v2)
{
    SPDLOG_DEBUG("glRectfv not implemented");
}

GLAPI void APIENTRY glRecti(GLint x1, GLint y1, GLint x2, GLint y2)
{
    SPDLOG_DEBUG("glRecti not implemented");
}

GLAPI void APIENTRY glRectiv(const GLint *v1, const GLint *v2)
{
    SPDLOG_DEBUG("glRectiv not implemented");
}

GLAPI void APIENTRY glRects(GLshort x1, GLshort y1, GLshort x2, GLshort y2)
{
    SPDLOG_DEBUG("glRects not implemented");
}

GLAPI void APIENTRY glRectsv(const GLshort *v1, const GLshort *v2)
{
    SPDLOG_DEBUG("glRectsv not implemented");
}

GLAPI GLint APIENTRY glRenderMode(GLenum mode)
{
    SPDLOG_DEBUG("glRenderMode not implemented");
    return 0;
}

GLAPI void APIENTRY glRotated(GLdouble angle, GLdouble x, GLdouble y, GLdouble z)
{
    SPDLOG_DEBUG("glRotated ({}, {}, {}, {}) called", 
        static_cast<float>(angle),
        static_cast<float>(x),
        static_cast<float>(y),
        static_cast<float>(z));
    IceGL::getInstance().vertexPipeline().rotate(static_cast<float>(angle),
        static_cast<float>(x),
        static_cast<float>(y),
        static_cast<float>(z));
}

GLAPI void APIENTRY glRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
    SPDLOG_DEBUG("glRotatef ({}, {}, {}, {}) called", angle, x, y, z);
    IceGL::getInstance().vertexPipeline().rotate(angle, x, y, z);
}

GLAPI void APIENTRY glScaled(GLdouble x, GLdouble y, GLdouble z)
{
    SPDLOG_DEBUG("glScaled ({}, {}, {}) called", 
        static_cast<float>(x),
        static_cast<float>(y),
        static_cast<float>(z));
    IceGL::getInstance().vertexPipeline().scale(static_cast<float>(x),
        static_cast<float>(y),
        static_cast<float>(z));
}

GLAPI void APIENTRY glScalef(GLfloat x, GLfloat y, GLfloat z)
{
    SPDLOG_DEBUG("glScalef ({}, {}, {}) called", x, y, z);
    IceGL::getInstance().vertexPipeline().scale(x, y, z);
}

GLAPI void APIENTRY glScissor(GLint x, GLint y, GLsizei width, GLsizei height)
{
    SPDLOG_DEBUG("glScissor x {} y {} width {} height {} called", x, y, width, height);
    if ((width < 0) || (height < 0))
    {
        IceGL::getInstance().setError(GL_INVALID_VALUE);
    }
    else
    {
        IceGL::getInstance().pixelPipeline().setScissorBox(x, y, width, height);
    }
}

GLAPI void APIENTRY glSelectBuffer(GLsizei size, GLuint *buffer)
{
    SPDLOG_DEBUG("glSelectBuffer not implemented");
}

GLAPI void APIENTRY glShadeModel(GLenum mode)
{
    SPDLOG_DEBUG("glShadeModel not implemented");
}

GLAPI void APIENTRY glStencilFunc(GLenum func, GLint ref, GLuint mask)
{
    SPDLOG_DEBUG("glStencilFunc not implemented");
}

GLAPI void APIENTRY glStencilMask(GLuint mask)
{
    SPDLOG_DEBUG("glStencilMask not implemented");
}

GLAPI void APIENTRY glStencilOp(GLenum fail, GLenum zfail, GLenum zpass)
{
    SPDLOG_DEBUG("glStencilOp not implemented");
}

GLAPI void APIENTRY glTexCoord1d(GLdouble s)
{
    SPDLOG_DEBUG("glTexCoord1d ({}) called", static_cast<float>(s));
    IceGL::getInstance().vertexQueue().setTexCoord({ { static_cast<float>(s), 0.0f, 0.0f, 1.0f } });
}

GLAPI void APIENTRY glTexCoord1dv(const GLdouble *v)
{
    SPDLOG_DEBUG("glTexCoord1dv ({}) called", static_cast<float>(v[0]));
    IceGL::getInstance().vertexQueue().setTexCoord({ { static_cast<float>(v[0]), 0.0f, 0.0f, 1.0f } });
}

GLAPI void APIENTRY glTexCoord1f(GLfloat s)
{
    SPDLOG_DEBUG("glTexCoord1f ({}) called", s);
    IceGL::getInstance().vertexQueue().setTexCoord({ { s, 0.0f, 0.0f, 1.0f } });
}

GLAPI void APIENTRY glTexCoord1fv(const GLfloat *v)
{
    SPDLOG_DEBUG("glTexCoord1fv ({}) called", v[0]);
    IceGL::getInstance().vertexQueue().setTexCoord({ { v[0], 0.0f, 0.0f, 1.0f } });
}

GLAPI void APIENTRY glTexCoord1i(GLint s)
{
    SPDLOG_DEBUG("glTexCoord1i ({}) called", static_cast<float>(s));
    IceGL::getInstance().vertexQueue().setTexCoord({ { static_cast<float>(s), 0.0f, 0.0f, 1.0f } });
}

GLAPI void APIENTRY glTexCoord1iv(const GLint *v)
{
    SPDLOG_DEBUG("glTexCoord1iv ({}) called", static_cast<float>(v[0]));
    IceGL::getInstance().vertexQueue().setTexCoord({ { static_cast<float>(v[0]), 0.0f, 0.0f, 1.0f } });
}

GLAPI void APIENTRY glTexCoord1s(GLshort s)
{
    SPDLOG_DEBUG("glTexCoord1s ({}) called", static_cast<float>(s));
    IceGL::getInstance().vertexQueue().setTexCoord({ { static_cast<float>(s), 0.0f, 0.0f, 1.0f } });
}

GLAPI void APIENTRY glTexCoord1sv(const GLshort *v)
{
    SPDLOG_DEBUG("glTexCoord1sv ({}) called", static_cast<float>(v[0]));
    IceGL::getInstance().vertexQueue().setTexCoord({ { static_cast<float>(v[0]), 0.0f, 0.0f, 1.0f } });
}

GLAPI void APIENTRY glTexCoord2d(GLdouble s, GLdouble t)
{
    SPDLOG_DEBUG("glTexCoord2d ({}, {}) called", static_cast<float>(s), static_cast<float>(t));
    IceGL::getInstance().vertexQueue().setTexCoord({ { static_cast<float>(s), static_cast<float>(t), 0.0f, 1.0f } });
}

GLAPI void APIENTRY glTexCoord2dv(const GLdouble *v)
{
    SPDLOG_DEBUG("glTexCoord2dv ({}, {}) called", static_cast<float>(v[0]), static_cast<float>(v[1]));
    IceGL::getInstance().vertexQueue().setTexCoord({ { static_cast<float>(v[0]), static_cast<float>(v[1]), 0.0f, 1.0f } });
}

GLAPI void APIENTRY glTexCoord2f(GLfloat s, GLfloat t)
{
    SPDLOG_DEBUG("glTexCoord2f ({}, {}) called", s, t);
    IceGL::getInstance().vertexQueue().setTexCoord({ { s, t, 0.0f, 1.0f } });
}

GLAPI void APIENTRY glTexCoord2fv(const GLfloat *v)
{
    SPDLOG_DEBUG("glTexCoord2fv ({}, {}) called", v[0], v[1]);
    IceGL::getInstance().vertexQueue().setTexCoord({ { v[0], v[1], 0.0f, 1.0f } });
}

GLAPI void APIENTRY glTexCoord2i(GLint s, GLint t)
{
    SPDLOG_DEBUG("glTexCoord2i ({}, {}) called", static_cast<float>(s), static_cast<float>(t));
    IceGL::getInstance().vertexQueue().setTexCoord({ { static_cast<float>(s), static_cast<float>(t), 0.0f, 1.0f } });
}

GLAPI void APIENTRY glTexCoord2iv(const GLint *v)
{
    SPDLOG_DEBUG("glTexCoord2iv ({}, {}) called", static_cast<float>(v[0]), static_cast<float>(v[1]));
    IceGL::getInstance().vertexQueue().setTexCoord({ { static_cast<float>(v[0]), static_cast<float>(v[1]), 0.0f, 1.0f } });
}

GLAPI void APIENTRY glTexCoord2s(GLshort s, GLshort t)
{
    SPDLOG_DEBUG("glTexCoord2s ({}, {}) called", static_cast<float>(s), static_cast<float>(t));
    IceGL::getInstance().vertexQueue().setTexCoord({ { static_cast<float>(s), static_cast<float>(t), 0.0f, 1.0f } });
}

GLAPI void APIENTRY glTexCoord2sv(const GLshort *v)
{
    SPDLOG_DEBUG("glTexCoord2sv ({}, {}) called", static_cast<float>(v[0]), static_cast<float>(v[1]));
    IceGL::getInstance().vertexQueue().setTexCoord({ { static_cast<float>(v[0]), static_cast<float>(v[1]), 0.0f, 1.0f } });
}

GLAPI void APIENTRY glTexCoord3d(GLdouble s, GLdouble t, GLdouble r)
{
    SPDLOG_DEBUG("glTexCoord3d ({}, {}, {}) called", static_cast<float>(s), static_cast<float>(t), static_cast<float>(r));
    IceGL::getInstance().vertexQueue().setTexCoord({ { static_cast<float>(s), static_cast<float>(t), static_cast<float>(r), 1.0f } });
}

GLAPI void APIENTRY glTexCoord3dv(const GLdouble *v)
{
    SPDLOG_DEBUG("glTexCoord3dv ({}, {}, {}) called", static_cast<float>(v[0]), static_cast<float>(v[1]), static_cast<float>(v[2]));
    IceGL::getInstance().vertexQueue().setTexCoord({ { static_cast<float>(v[0]), static_cast<float>(v[1]), static_cast<float>(v[2]), 1.0f } });
}

GLAPI void APIENTRY glTexCoord3f(GLfloat s, GLfloat t, GLfloat r)
{
    SPDLOG_DEBUG("glTexCoord3f ({}, {}, {}) called", s, t, r);
    IceGL::getInstance().vertexQueue().setTexCoord({ { s, t, r, 1.0f } });
}

GLAPI void APIENTRY glTexCoord3fv(const GLfloat *v)
{
    SPDLOG_DEBUG("glTexCoord3fv ({}, {}, {}) called", v[0], v[1], v[2]);
    IceGL::getInstance().vertexQueue().setTexCoord({ { v[0], v[1], v[2], 1.0f } });
}

GLAPI void APIENTRY glTexCoord3i(GLint s, GLint t, GLint r)
{
    SPDLOG_DEBUG("glTexCoord3i ({}, {}, {}) called", static_cast<float>(s), static_cast<float>(t), static_cast<float>(r));
    IceGL::getInstance().vertexQueue().setTexCoord({ { static_cast<float>(s), static_cast<float>(t), static_cast<float>(r), 1.0f } });
}

GLAPI void APIENTRY glTexCoord3iv(const GLint *v)
{
    SPDLOG_DEBUG("glTexCoord3iv ({}, {}, {}) called", static_cast<float>(v[0]), static_cast<float>(v[1]), static_cast<float>(v[2]));
    IceGL::getInstance().vertexQueue().setTexCoord({ { static_cast<float>(v[0]), static_cast<float>(v[1]), static_cast<float>(v[2]), 1.0f } });
}

GLAPI void APIENTRY glTexCoord3s(GLshort s, GLshort t, GLshort r)
{
    SPDLOG_DEBUG("glTexCoord3s ({}, {}, {}) called", static_cast<float>(s), static_cast<float>(t), static_cast<float>(r));
    IceGL::getInstance().vertexQueue().setTexCoord({ { static_cast<float>(s), static_cast<float>(t), static_cast<float>(r), 1.0f } });
}

GLAPI void APIENTRY glTexCoord3sv(const GLshort *v)
{
    SPDLOG_DEBUG("glTexCoord3sv ({}, {}, {}) called", static_cast<float>(v[0]), static_cast<float>(v[1]), static_cast<float>(v[2]));
    IceGL::getInstance().vertexQueue().setTexCoord({ { static_cast<float>(v[0]), static_cast<float>(v[1]), static_cast<float>(v[2]), 1.0f } });
}

GLAPI void APIENTRY glTexCoord4d(GLdouble s, GLdouble t, GLdouble r, GLdouble q)
{
    SPDLOG_DEBUG("glTexCoord4d ({}, {}, {}, {}) called", static_cast<float>(s), static_cast<float>(t), static_cast<float>(r), static_cast<float>(q));
    IceGL::getInstance().vertexQueue().setTexCoord({ { static_cast<float>(s), static_cast<float>(t), static_cast<float>(r), static_cast<float>(q) } });
}

GLAPI void APIENTRY glTexCoord4dv(const GLdouble *v)
{
    SPDLOG_DEBUG("glTexCoord4dv ({}, {}, {}, {}) called", static_cast<float>(v[0]), static_cast<float>(v[1]), static_cast<float>(v[2]), static_cast<float>(v[3]));
    IceGL::getInstance().vertexQueue().setTexCoord({ { static_cast<float>(v[0]), static_cast<float>(v[1]), static_cast<float>(v[2]), static_cast<float>(v[3]) } });
}

GLAPI void APIENTRY glTexCoord4f(GLfloat s, GLfloat t, GLfloat r, GLfloat q)
{
    SPDLOG_DEBUG("glTexCoord4f ({}, {}, {}, {}) called", s, t, r, q);
    IceGL::getInstance().vertexQueue().setTexCoord({ { s, t, r, q } });
}

GLAPI void APIENTRY glTexCoord4fv(const GLfloat *v)
{
    SPDLOG_DEBUG("glTexCoord4fv ({}, {}, {}, {}) called", v[0], v[1], v[2], v[3]);
    IceGL::getInstance().vertexQueue().setTexCoord({ { v[0], v[1], v[2], v[3] } });
}

GLAPI void APIENTRY glTexCoord4i(GLint s, GLint t, GLint r, GLint q)
{
    SPDLOG_DEBUG("glTexCoord4i ({}, {}, {}, {}) called", static_cast<float>(s), static_cast<float>(t), static_cast<float>(r), static_cast<float>(q));
    IceGL::getInstance().vertexQueue().setTexCoord({ { static_cast<float>(s), static_cast<float>(t), static_cast<float>(r), static_cast<float>(q) } });
}

GLAPI void APIENTRY glTexCoord4iv(const GLint *v)
{
    SPDLOG_DEBUG("glTexCoord4iv ({}, {}, {}, {}) called", static_cast<float>(v[0]), static_cast<float>(v[1]), static_cast<float>(v[2]), static_cast<float>(v[3]));
    IceGL::getInstance().vertexQueue().setTexCoord({ { static_cast<float>(v[0]), static_cast<float>(v[1]), static_cast<float>(v[2]), static_cast<float>(v[3]) } });
}

GLAPI void APIENTRY glTexCoord4s(GLshort s, GLshort t, GLshort r, GLshort q)
{
    SPDLOG_DEBUG("glTexCoord4s ({}, {}, {}, {}) called", static_cast<float>(s), static_cast<float>(t), static_cast<float>(r), static_cast<float>(q));
    IceGL::getInstance().vertexQueue().setTexCoord({ { static_cast<float>(s), static_cast<float>(t), static_cast<float>(r), static_cast<float>(q) } });
}

GLAPI void APIENTRY glTexCoord4sv(const GLshort *v)
{
    SPDLOG_DEBUG("glTexCoord4sv ({}, {}, {}, {}) called", static_cast<float>(v[0]), static_cast<float>(v[1]), static_cast<float>(v[2]), static_cast<float>(v[3]));
    IceGL::getInstance().vertexQueue().setTexCoord({ { static_cast<float>(v[0]), static_cast<float>(v[1]), static_cast<float>(v[2]), static_cast<float>(v[3]) } });
}

GLAPI void APIENTRY glTexEnvf(GLenum target, GLenum pname, GLfloat param)
{
    SPDLOG_DEBUG("glTexEnvf target {} pname {} param {} redirected to glTexEnvi", target, pname, param);
    glTexEnvi(target, pname, static_cast<GLint>(param));
}

GLAPI void APIENTRY glTexEnvfv(GLenum target, GLenum pname, const GLfloat *params)
{
    SPDLOG_DEBUG("glTexEnvfv target {} param {} called", target, pname);

    if ((target == GL_TEXTURE_ENV) && (pname == GL_TEXTURE_ENV_COLOR))
    {
        if (IceGL::getInstance().pixelPipeline().setTexEnvColor({ { params[0], params[1], params[2], params[3] } }))
        {
            IceGL::getInstance().setError(GL_NO_ERROR);
        }
        else
        {
            SPDLOG_ERROR("glTexEnvfv Out Of Memory");
        }
    }
    else
    {
        IceGL::getInstance().setError(GL_INVALID_ENUM);
    }
}

GLAPI void APIENTRY glTexEnvi(GLenum target, GLenum pname, GLint param)
{
    SPDLOG_DEBUG("glTexEnvi target {} pname {} param {} called", target, pname, param);

    IceGL::getInstance().setError(GL_NO_ERROR);
    GLenum error { GL_NO_ERROR };
    if (target == GL_TEXTURE_ENV)
    {
        switch (pname) 
        {
            case GL_TEXTURE_ENV_MODE:
            {
                PixelPipeline::TexEnvMode mode {};
                error = convertTexEnvMode(mode, param);
                if (error == GL_NO_ERROR)
                    IceGL::getInstance().pixelPipeline().setTexEnvMode(mode); 
                break;
            }
            case GL_COMBINE_RGB:
            {
                PixelPipeline::TexEnv::Combine c {};
                error = convertCombine(c, param, false);
                if (error == GL_NO_ERROR)
                    IceGL::getInstance().pixelPipeline().texEnv().setCombineRgb(c);
                break;
            }
            case GL_COMBINE_ALPHA:
            {
                PixelPipeline::TexEnv::Combine c {};
                error = convertCombine(c, param, true);
                if (error == GL_NO_ERROR)
                    IceGL::getInstance().pixelPipeline().texEnv().setCombineAlpha(c);
                break;
            }
            case GL_SOURCE0_RGB:
            {
                PixelPipeline::TexEnv::SrcReg c {};
                error = convertSrcReg(c, param);
                if (error == GL_NO_ERROR)
                    IceGL::getInstance().pixelPipeline().texEnv().setSrcRegRgb0(c);
                break;
            }
            case GL_SOURCE1_RGB:
            {
                PixelPipeline::TexEnv::SrcReg c {};
                error = convertSrcReg(c, param);
                if (error == GL_NO_ERROR)
                    IceGL::getInstance().pixelPipeline().texEnv().setSrcRegRgb1(c);
                break;            
            }
            case GL_SOURCE2_RGB:
            {
                PixelPipeline::TexEnv::SrcReg c {};
                error = convertSrcReg(c, param);
                if (error == GL_NO_ERROR)
                    IceGL::getInstance().pixelPipeline().texEnv().setSrcRegRgb2(c);
                break;
            }
            case GL_SOURCE0_ALPHA:
            {
                PixelPipeline::TexEnv::SrcReg c {};
                error = convertSrcReg(c, param);
                if (error == GL_NO_ERROR)
                    IceGL::getInstance().pixelPipeline().texEnv().setSrcRegAlpha0(c);
                break;
            }
            case GL_SOURCE1_ALPHA:
            {
                PixelPipeline::TexEnv::SrcReg c {};
                error = convertSrcReg(c, param);
                if (error == GL_NO_ERROR)
                    IceGL::getInstance().pixelPipeline().texEnv().setSrcRegAlpha1(c);
                break;            
            }
            case GL_SOURCE2_ALPHA:
            {
                PixelPipeline::TexEnv::SrcReg c {};
                error = convertSrcReg(c, param);
                if (error == GL_NO_ERROR)
                    IceGL::getInstance().pixelPipeline().texEnv().setSrcRegAlpha2(c);
                break;
            }
            case GL_OPERAND0_RGB:
            {
                PixelPipeline::TexEnv::Operand c {};
                error = convertOperand(c, param, false);
                if (error == GL_NO_ERROR)
                    IceGL::getInstance().pixelPipeline().texEnv().setOperandRgb0(c);
                break;
            }
            case GL_OPERAND1_RGB:
            {
                PixelPipeline::TexEnv::Operand c {};
                error = convertOperand(c, param, false);
                if (error == GL_NO_ERROR)
                    IceGL::getInstance().pixelPipeline().texEnv().setOperandRgb1(c);
                break;
            }
            case GL_OPERAND2_RGB:
            {
                PixelPipeline::TexEnv::Operand c {};
                error = convertOperand(c, param, false);
                if (error == GL_NO_ERROR)
                    IceGL::getInstance().pixelPipeline().texEnv().setOperandRgb2(c);
                break;
            }
            case GL_OPERAND0_ALPHA:
            {
                PixelPipeline::TexEnv::Operand c {};
                error = convertOperand(c, param, true);
                if (error == GL_NO_ERROR)
                    IceGL::getInstance().pixelPipeline().texEnv().setOperandAlpha0(c);
                break;
            }
            case GL_OPERAND1_ALPHA:
            {
                PixelPipeline::TexEnv::Operand c {};
                error = convertOperand(c, param, true);
                if (error == GL_NO_ERROR)
                    IceGL::getInstance().pixelPipeline().texEnv().setOperandAlpha1(c);
                break;
            }
            case GL_OPERAND2_ALPHA:
            {
                PixelPipeline::TexEnv::Operand c {};
                error = convertOperand(c, param, true);
                if (error == GL_NO_ERROR)
                    IceGL::getInstance().pixelPipeline().texEnv().setOperandAlpha2(c);
                break;
            }
            case GL_RGB_SCALE:
                {
                    const uint8_t shift = std::log2f(param);
                    if ((shift >= 0) && (shift <= 2))
                    {
                        IceGL::getInstance().pixelPipeline().texEnv().setShiftRgb(shift);
                        error = GL_NO_ERROR;
                    }
                    else 
                    {
                        error = GL_INVALID_ENUM;
                    }
                }
                break;
            case GL_ALPHA_SCALE:
                {
                    const uint8_t shift = std::log2f(param);
                    if ((shift >= 0) && (shift <= 2))
                    {
                        IceGL::getInstance().pixelPipeline().texEnv().setShiftAlpha(shift);
                        error = GL_NO_ERROR;
                    }
                    else 
                    {
                        error = GL_INVALID_VALUE;
                    }
                }
                break;
            default:
                error = GL_INVALID_ENUM;
                break;
        };
    }
    else
    { 
        error = GL_INVALID_ENUM;
    }
    IceGL::getInstance().setError(error);
}

GLAPI void APIENTRY glTexEnviv(GLenum target, GLenum pname, const GLint *params)
{
    SPDLOG_DEBUG("glTexEnviv not implemented");
}

GLAPI void APIENTRY glTexGend(GLenum coord, GLenum pname, GLdouble param)
{
    SPDLOG_DEBUG("glTexGend redirected to glTexGenf");
    glTexGenf(coord, pname, static_cast<float>(param));
}

GLAPI void APIENTRY glTexGendv(GLenum coord, GLenum pname, const GLdouble *params)
{
    SPDLOG_DEBUG("glTexGendv redirected to glTexGenfv");
    std::array<float, 4> tmp {};
    tmp[0] = static_cast<float>(params[0]);
    tmp[1] = static_cast<float>(params[1]);
    tmp[2] = static_cast<float>(params[2]);
    tmp[3] = static_cast<float>(params[3]);
    glTexGenfv(coord, pname, tmp.data());
}

GLAPI void APIENTRY glTexGenf(GLenum coord, GLenum pname, GLfloat param)
{
    SPDLOG_DEBUG("glTexGenf redirected to glTexGeni");
    glTexGeni(coord, pname, static_cast<GLint>(param));
}

GLAPI void APIENTRY glTexGenfv(GLenum coord, GLenum pname, const GLfloat *params)
{
    SPDLOG_DEBUG("glTexGenfv coord {} pname {} called", coord, pname);
    switch (pname) {
    case GL_OBJECT_PLANE:
        switch (coord) {
            case GL_S:
                IceGL::getInstance().vertexPipeline().getTexGen().setTexGenVecObjS({ params });
                break;
            case GL_T:
                IceGL::getInstance().vertexPipeline().getTexGen().setTexGenVecObjT({ params });
                break;
            case GL_R:
                SPDLOG_WARN("glTexGenfv GL_OBJECT_PLANE GL_R not implemented");
                IceGL::getInstance().setError(GL_INVALID_ENUM);
                break;
            case GL_Q:
                SPDLOG_WARN("glTexGenfv GL_OBJECT_PLANE GL_R not implemented");
                IceGL::getInstance().setError(GL_INVALID_ENUM);
                break;
            default:
                IceGL::getInstance().setError(GL_INVALID_ENUM);
                break;
        }
        break;
    case GL_EYE_PLANE:
        switch (coord) {
            case GL_S:
                IceGL::getInstance().vertexPipeline().getTexGen().setTexGenVecEyeS(IceGL::getInstance().vertexPipeline().getModelMatrix(), { params });
                break;
            case GL_T:
                IceGL::getInstance().vertexPipeline().getTexGen().setTexGenVecEyeT(IceGL::getInstance().vertexPipeline().getModelMatrix(), { params });
                break;
            case GL_R:
                SPDLOG_WARN("glTexGenfv GL_OBJECT_PLANE GL_R not implemented");
                IceGL::getInstance().setError(GL_INVALID_ENUM);
                break;
            case GL_Q:
                SPDLOG_WARN("glTexGenfv GL_OBJECT_PLANE GL_R not implemented");
                IceGL::getInstance().setError(GL_INVALID_ENUM);
                break;
            default:
                IceGL::getInstance().setError(GL_INVALID_ENUM);
                break;
        }
        break;
    default:
        SPDLOG_DEBUG("glTexGenfv redirected to glTexGenf");
        glTexGenf(coord, pname, params[0]);
        break;
    }
}

GLAPI void APIENTRY glTexGeni(GLenum coord, GLenum pname, GLint param)
{
    SPDLOG_DEBUG("glTexGeni coord {} pname {} param {} called", coord, pname, param);
    TexGen::TexGenMode mode {};
    IceGL::getInstance().setError(GL_NO_ERROR);
    switch (param) {
    case GL_OBJECT_LINEAR:
        mode = TexGen::TexGenMode::OBJECT_LINEAR;
        break;
    case GL_EYE_LINEAR:
        mode = TexGen::TexGenMode::EYE_LINEAR;
        break;
    case GL_SPHERE_MAP:
        mode = TexGen::TexGenMode::SPHERE_MAP;
        break;
    default:
        IceGL::getInstance().setError(GL_INVALID_ENUM);
        break;
    }

    if (IceGL::getInstance().getError() == GL_NO_ERROR && pname == GL_TEXTURE_GEN_MODE)
    {
        switch (coord) {
        case GL_S:
            IceGL::getInstance().vertexPipeline().getTexGen().setTexGenModeS(mode);
            break;
        case GL_T:
            IceGL::getInstance().vertexPipeline().getTexGen().setTexGenModeT(mode);
            break;
        case GL_R:
            SPDLOG_WARN("glTexGeni GL_R not implemented");
            IceGL::getInstance().setError(GL_INVALID_ENUM);
            break;
        case GL_Q:
            SPDLOG_WARN("glTexGeni GL_Q not implemented");
            IceGL::getInstance().setError(GL_INVALID_ENUM);
            break;
        default:
            IceGL::getInstance().setError(GL_INVALID_ENUM);
            break;
        }
    }
    else
    {
        IceGL::getInstance().setError(GL_INVALID_ENUM);
    } 
}

GLAPI void APIENTRY glTexGeniv(GLenum coord, GLenum pname, const GLint *params)
{
    SPDLOG_DEBUG("glTexGeniv redirected to glTexGenfv");
    std::array<float, 4> tmp {};
    tmp[0] = static_cast<float>(params[0]);
    tmp[1] = static_cast<float>(params[1]);
    tmp[2] = static_cast<float>(params[2]);
    tmp[3] = static_cast<float>(params[3]);
    glTexGenfv(coord, pname, tmp.data());
}

GLAPI void APIENTRY glTexImage1D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels)
{
    SPDLOG_DEBUG("glTexImage1D not implemented");
}

GLAPI void APIENTRY glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels)
{
    SPDLOG_DEBUG("glTexImage2D target {} level {} internalformat {} width {} height {} border {} format {} type {} called", target, level, internalformat, width, height, border, format, type);

    (void)border;// Border is not supported and is ignored for now. What does border mean: //https://stackoverflow.com/questions/913801/what-does-border-mean-in-the-glteximage2d-function

    IceGL::getInstance().setError(GL_NO_ERROR);

    if ((width > MAX_TEX_SIZE)|| (height > MAX_TEX_SIZE))
    {
        IceGL::getInstance().setError(GL_INVALID_VALUE);
        SPDLOG_WARN("glTexImage2d texture is too big.");
        return;
    }

    if (level != 0)
    {
        SPDLOG_DEBUG("glTexImage2d mip mapping not supported. Only level 0 is used.");
        return;
    }

    // It can happen, that a not power of two texture is used. This little hack allows that the texture can sill be used
    // without crashing the software. But it is likely that it will produce graphical errors.
    const uint16_t widthRounded = powf(2.0f, ceilf(logf(width)/logf(2.0f)));
    const uint16_t heightRounded = powf(2.0f, ceilf(logf(height)/logf(2.0f)));

    PixelPipeline::IntendedInternalPixelFormat intentedInternalPixelFormat { PixelPipeline::IntendedInternalPixelFormat::RGBA };
    
    switch (internalformat)
    {
        case 1:
        case GL_COMPRESSED_ALPHA:
        case GL_ALPHA8:
        case GL_ALPHA12:
        case GL_ALPHA16:
            intentedInternalPixelFormat = PixelPipeline::IntendedInternalPixelFormat::ALPHA;
            break;
        case GL_COMPRESSED_LUMINANCE:
        case GL_LUMINANCE:
        case GL_LUMINANCE4:
        case GL_LUMINANCE8:
        case GL_LUMINANCE12:
        case GL_LUMINANCE16:
            intentedInternalPixelFormat = PixelPipeline::IntendedInternalPixelFormat::LUMINANCE;
            break;
        case GL_COMPRESSED_INTENSITY:
        case GL_INTENSITY:
        case GL_INTENSITY4:
        case GL_INTENSITY8:
        case GL_INTENSITY12:
        case GL_INTENSITY16:
            intentedInternalPixelFormat = PixelPipeline::IntendedInternalPixelFormat::INTENSITY;
            break;
        case 2:
        case GL_COMPRESSED_LUMINANCE_ALPHA:
        case GL_LUMINANCE_ALPHA:
        case GL_LUMINANCE4_ALPHA4:
        case GL_LUMINANCE6_ALPHA2:
        case GL_LUMINANCE8_ALPHA8:
        case GL_LUMINANCE12_ALPHA4:
        case GL_LUMINANCE12_ALPHA12:
        case GL_LUMINANCE16_ALPHA16:
            intentedInternalPixelFormat = PixelPipeline::IntendedInternalPixelFormat::LUMINANCE_ALPHA;
            break;
        case 3:
        case GL_COMPRESSED_RGB:
        case GL_R3_G3_B2:
        case GL_RGB:
        case GL_RGB4:
        case GL_RGB5:
        case GL_RGB8:
        case GL_RGB10:
        case GL_RGB12:
        case GL_RGB16:
            intentedInternalPixelFormat = PixelPipeline::IntendedInternalPixelFormat::RGB;
            break;
        case 4:
        case GL_COMPRESSED_RGBA:
        case GL_RGBA:
        case GL_RGBA2:
        case GL_RGBA4:
        case GL_RGBA8:
        case GL_RGB10_A2:
        case GL_RGBA12:
        case GL_RGBA16:
            intentedInternalPixelFormat = PixelPipeline::IntendedInternalPixelFormat::RGBA;
            break;
        case GL_RGB5_A1:
            intentedInternalPixelFormat = PixelPipeline::IntendedInternalPixelFormat::RGBA1;
            break;
        case GL_DEPTH_COMPONENT:
            SPDLOG_WARN("glTexImage2D internal format GL_DEPTH_COMPONENT not supported");
            break;
        default:
            SPDLOG_WARN("glTexImage2D invalid internalformat");
            IceGL::getInstance().setError(GL_INVALID_ENUM);
            return;
    }

    if (!IceGL::getInstance().pixelPipeline().uploadTexture(std::shared_ptr<const uint16_t>(), widthRounded, heightRounded, intentedInternalPixelFormat))
    {
        IceGL::getInstance().setError(GL_INVALID_VALUE);
        return;
    }

    SPDLOG_DEBUG("glTexImage2D redirect to glTexSubImage2D");
    glTexSubImage2D(target, level, 0, 0, width, height, format, type, pixels);
}

GLAPI void APIENTRY glTexParameterf(GLenum target, GLenum pname, GLfloat param)
{
    SPDLOG_DEBUG("glTexParameterf target {} pname {} param {} redirected to glTexParameteri", target, pname, param);
    glTexParameteri(target, pname, static_cast<GLint>(param));
}

GLAPI void APIENTRY glTexParameterfv(GLenum target, GLenum pname, const GLfloat *params)
{
    SPDLOG_DEBUG("glTexParameterfv not implemented");
}

GLAPI void APIENTRY glTexParameteri(GLenum target, GLenum pname, GLint param)
{
    SPDLOG_DEBUG("glTexParameteri tartget {} pname {} param {}", target, pname, param);
    IceGL::getInstance().setError(GL_NO_ERROR);
    if (target == GL_TEXTURE_2D)
    {
        switch (pname) {
        case GL_TEXTURE_WRAP_S:
        {
            auto mode = convertGlTextureWrapMode(static_cast<GLenum>(param));
            if (IceGL::getInstance().getError() == GL_NO_ERROR)
            {
                IceGL::getInstance().pixelPipeline().setTexWrapModeS(mode);
            }
            break;
        }
        case GL_TEXTURE_WRAP_T:
        {
            auto mode = convertGlTextureWrapMode(static_cast<GLenum>(param));
            if (IceGL::getInstance().getError() == GL_NO_ERROR)
            {
                IceGL::getInstance().pixelPipeline().setTexWrapModeT(mode);
            }
            break;
        }
            break;
        case GL_TEXTURE_MAG_FILTER:
            if ((param == GL_LINEAR) || (param == GL_NEAREST))
            {
                IceGL::getInstance().pixelPipeline().setEnableMagFilter(param == GL_LINEAR);
            }
            else
            {
                IceGL::getInstance().setError(GL_INVALID_ENUM);
            }
            break;
        default:
            IceGL::getInstance().setError(GL_INVALID_ENUM);
            break;
        }
    }
    else
    {
        // TODO: Implement other modes like GL_TEXTURE_MIN_FILTER and so on, but they require hardware support (which is not implemented).
        IceGL::getInstance().setError(GL_INVALID_ENUM);
    }
}

GLAPI void APIENTRY glTexParameteriv(GLenum target, GLenum pname, const GLint *params)
{
    SPDLOG_DEBUG("glTexParameteriv not implemented");
}

GLAPI void APIENTRY glTranslated(GLdouble x, GLdouble y, GLdouble z)
{
    SPDLOG_DEBUG("glTranslated ({}, {}, {}) called", 
        static_cast<float>(x), 
        static_cast<float>(y), 
        static_cast<float>(z));
    IceGL::getInstance().vertexPipeline().translate(static_cast<float>(x),
        static_cast<float>(y), 
        static_cast<float>(z));
}

GLAPI void APIENTRY glTranslatef(GLfloat x, GLfloat y, GLfloat z)
{
    SPDLOG_DEBUG("glTranslatef ({}, {}, {}) called", x, y, z);
    IceGL::getInstance().vertexPipeline().translate(x, y, z);
}

GLAPI void APIENTRY glVertex2d(GLdouble x, GLdouble y)
{
    SPDLOG_DEBUG("glVertex2d ({}, {}) called", static_cast<float>(x), static_cast<float>(y));
    IceGL::getInstance().vertexQueue().addVertex({ { static_cast<float>(x), static_cast<float>(y), 0.0f, 1.0f } });
}

GLAPI void APIENTRY glVertex2dv(const GLdouble *v)
{
    SPDLOG_DEBUG("glVertex2dv ({}, {}) called", static_cast<float>(v[0]), static_cast<float>(v[1]));
    IceGL::getInstance().vertexQueue().addVertex({ { static_cast<float>(v[0]), static_cast<float>(v[1]), 0.0f, 1.0f } });
}

GLAPI void APIENTRY glVertex2f(GLfloat x, GLfloat y)
{
    SPDLOG_DEBUG("glVertex2f ({}, {}) called", x, y);
    IceGL::getInstance().vertexQueue().addVertex({ { x, y, 0.0f, 1.0f } });
}

GLAPI void APIENTRY glVertex2fv(const GLfloat *v)
{
    SPDLOG_DEBUG("glVertex2fv ({}, {}) called", v[0], v[1]);
    IceGL::getInstance().vertexQueue().addVertex({ {  v[0], v[1], 0.0f, 1.0f } });
}

GLAPI void APIENTRY glVertex2i(GLint x, GLint y)
{
    SPDLOG_DEBUG("glVertex2i ({}, {}) called", static_cast<float>(x), static_cast<float>(y));
    IceGL::getInstance().vertexQueue().addVertex({ { static_cast<float>(x), static_cast<float>(y), 0.0f, 1.0f } });
}

GLAPI void APIENTRY glVertex2iv(const GLint *v)
{
    SPDLOG_DEBUG("glVertex2iv ({}, {}) called", static_cast<float>(v[0]), static_cast<float>(v[1]));
    IceGL::getInstance().vertexQueue().addVertex({ { static_cast<float>(v[0]), static_cast<float>(v[1]), 0.0f, 1.0f } });
}

GLAPI void APIENTRY glVertex2s(GLshort x, GLshort y)
{
    SPDLOG_DEBUG("glVertex2s ({}, {}) called", static_cast<float>(x), static_cast<float>(y));
    IceGL::getInstance().vertexQueue().addVertex({ { static_cast<float>(x), static_cast<float>(y), 0.0f, 1.0f } });
}

GLAPI void APIENTRY glVertex2sv(const GLshort *v)
{
    SPDLOG_DEBUG("glVertex2sv ({}, {}) called", static_cast<float>(v[0]), static_cast<float>(v[1]));
    IceGL::getInstance().vertexQueue().addVertex({ { static_cast<float>(v[0]), static_cast<float>(v[1]), 0.0f, 1.0f } });
}

GLAPI void APIENTRY glVertex3d(GLdouble x, GLdouble y, GLdouble z)
{
    SPDLOG_DEBUG("glVertex3d ({}, {}, {}) called", static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));
    IceGL::getInstance().vertexQueue().addVertex({ { static_cast<float>(x), static_cast<float>(y), static_cast<float>(z), 1.0f } });
}

GLAPI void APIENTRY glVertex3dv(const GLdouble *v)
{
    SPDLOG_DEBUG("glVertex3dv ({}, {}, {}) called", static_cast<float>(v[0]), static_cast<float>(v[1]), static_cast<float>(v[2]));
    IceGL::getInstance().vertexQueue().addVertex({ { static_cast<float>(v[0]), static_cast<float>(v[1]), static_cast<float>(v[2]), 1.0f } });
}

GLAPI void APIENTRY glVertex3f(GLfloat x, GLfloat y, GLfloat z)
{
    SPDLOG_DEBUG("glVertex3f ({}, {}, {}) called", x, y, z);
    IceGL::getInstance().vertexQueue().addVertex({ { x, y, z, 1.0f } });
}

GLAPI void APIENTRY glVertex3fv(const GLfloat *v)
{
    SPDLOG_DEBUG("glVertex3fv ({}, {}, {}) called", v[0], v[1], v[2]);
    IceGL::getInstance().vertexQueue().addVertex({ { v[0], v[1], v[2], 1.0f } });
}

GLAPI void APIENTRY glVertex3i(GLint x, GLint y, GLint z)
{
    SPDLOG_DEBUG("glVertex3i ({}, {}, {}) called", static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));
    IceGL::getInstance().vertexQueue().addVertex({ { static_cast<float>(x), static_cast<float>(y), static_cast<float>(z), 1.0f } });
}

GLAPI void APIENTRY glVertex3iv(const GLint *v)
{
    SPDLOG_DEBUG("glVertex3iv ({}, {}, {}) called", static_cast<float>(v[0]), static_cast<float>(v[1]), static_cast<float>(v[2]));
    IceGL::getInstance().vertexQueue().addVertex({ { static_cast<float>(v[0]), static_cast<float>(v[1]), static_cast<float>(v[2]), 1.0f } });
}

GLAPI void APIENTRY glVertex3s(GLshort x, GLshort y, GLshort z)
{
    SPDLOG_DEBUG("glVertex3s ({}, {}, {}) called", static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));
    IceGL::getInstance().vertexQueue().addVertex({ { static_cast<float>(x), static_cast<float>(y), static_cast<float>(z), 1.0f } });
}

GLAPI void APIENTRY glVertex3sv(const GLshort *v)
{
    SPDLOG_DEBUG("glVertex3sv ({}, {}, {}) called", static_cast<float>(v[0]), static_cast<float>(v[1]), static_cast<float>(v[2]));
    IceGL::getInstance().vertexQueue().addVertex({ { static_cast<float>(v[0]), static_cast<float>(v[1]), static_cast<float>(v[2]), 1.0f } });
}

GLAPI void APIENTRY glVertex4d(GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
    SPDLOG_DEBUG("glVertex4d ({}, {}, {}, {}) called", static_cast<float>(x), static_cast<float>(y), static_cast<float>(z), static_cast<float>(w));
    IceGL::getInstance().vertexQueue().addVertex({ { static_cast<float>(x), static_cast<float>(y), static_cast<float>(z), static_cast<float>(w) } });
}

GLAPI void APIENTRY glVertex4dv(const GLdouble *v)
{
    SPDLOG_DEBUG("glVertex4dv ({}, {}, {}, {}) called", static_cast<float>(v[0]), static_cast<float>(v[1]), static_cast<float>(v[2]), static_cast<float>(v[3]));
    IceGL::getInstance().vertexQueue().addVertex({ { static_cast<float>(v[0]), static_cast<float>(v[1]), static_cast<float>(v[2]), static_cast<float>(v[3]) } });
}

GLAPI void APIENTRY glVertex4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    SPDLOG_DEBUG("glVertex4f ({}, {}, {}, {}) called", x, y, z, w);
    IceGL::getInstance().vertexQueue().addVertex({ { x, y, z, w } });
}

GLAPI void APIENTRY glVertex4fv(const GLfloat *v)
{
    SPDLOG_DEBUG("glVertex4fv ({}, {}, {}, {}) called", v[0], v[1], v[2], v[3]);
    IceGL::getInstance().vertexQueue().addVertex({ { v[0], v[1], v[2], v[3] } });
}

GLAPI void APIENTRY glVertex4i(GLint x, GLint y, GLint z, GLint w)
{
    SPDLOG_DEBUG("glVertex4i ({}, {}, {}, {}) called", static_cast<float>(x), static_cast<float>(y), static_cast<float>(z), static_cast<float>(w));
    IceGL::getInstance().vertexQueue().addVertex({ { static_cast<float>(x), static_cast<float>(y), static_cast<float>(z), static_cast<float>(w) } });
}

GLAPI void APIENTRY glVertex4iv(const GLint *v)
{
    SPDLOG_DEBUG("glVertex4iv ({}, {}, {}, {}) called", static_cast<float>(v[0]), static_cast<float>(v[1]), static_cast<float>(v[2]), static_cast<float>(v[3]));
    IceGL::getInstance().vertexQueue().addVertex({ { static_cast<float>(v[0]), static_cast<float>(v[1]), static_cast<float>(v[2]), static_cast<float>(v[3]) } });
}

GLAPI void APIENTRY glVertex4s(GLshort x, GLshort y, GLshort z, GLshort w)
{
    SPDLOG_DEBUG("glVertex4s ({}, {}, {}, {}) called", static_cast<float>(x), static_cast<float>(y), static_cast<float>(z), static_cast<float>(w));
    IceGL::getInstance().vertexQueue().addVertex({ { static_cast<float>(x), static_cast<float>(y), static_cast<float>(z), static_cast<float>(w) } });
}

GLAPI void APIENTRY glVertex4sv(const GLshort *v)
{
    SPDLOG_DEBUG("glVertex4sv ({}, {}, {}, {}) called", static_cast<float>(v[0]), static_cast<float>(v[1]), static_cast<float>(v[2]), static_cast<float>(v[3]));
    IceGL::getInstance().vertexQueue().addVertex({ { static_cast<float>(v[0]), static_cast<float>(v[1]), static_cast<float>(v[2]), static_cast<float>(v[3]) } });
}

GLAPI void APIENTRY glViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
    SPDLOG_DEBUG("glViewport ({}, {}) width {} heigh {} called", x, y, width, height);
    // TODO: Generate a GL_INVALID_VALUE if width or height is negative
    // TODO: Reversed mapping is not working right now, for instance if zFar < zNear
    // Note: The screen resolution is width and height. But during view port transformation it is clamped between
    // 0 and height which means a effective screen resolution of height + 1. For instance, a resolution of
    // 480 x 272. The view port transformation would go from 0 to 480 which is then 481px. Thats the reason why the
    // resolution is decremented by one.
    IceGL::getInstance().vertexPipeline().setViewport(x, y, width, height);
}

// -------------------------------------------------------

// Open GL 1.1
// -------------------------------------------------------
GLAPI GLboolean APIENTRY glAreTexturesResident(GLsizei n, const GLuint *textures, GLboolean *residences)
{
    SPDLOG_DEBUG("glAreTexturesResident not implemented");
    return false;
}

GLAPI void APIENTRY glArrayElement(GLint i)
{
    SPDLOG_DEBUG("glArrayElement not implemented");
}

GLAPI void APIENTRY glBindTexture(GLenum target, GLuint texture)
{
    SPDLOG_DEBUG("glBindTexture target {} texture {}", target, texture);
    IceGL::getInstance().setError(GL_NO_ERROR);
    if (target != GL_TEXTURE_2D)
    {
        IceGL::getInstance().setError(GL_INVALID_ENUM);
        return;
    }

    // TODO: Validate if the texture is valid
    IceGL::getInstance().pixelPipeline().setBoundTexture(texture);

    if (IceGL::getInstance().getError() == GL_NO_ERROR)
    {
        IceGL::getInstance().pixelPipeline().useTexture(PixelPipeline::TMU::TMU0);
    }
    else 
    {
        // If the bound texture is 0 or if using an invalid texture, then use the default texture.
        // Assume the default texture as no texture -> disable texture unit
        glDisable(GL_TEXTURE_2D);
    }
}

GLAPI void APIENTRY glColorPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
    SPDLOG_DEBUG("glColorPointer size {} type {} stride {} called", size, type, stride);

    IceGL::getInstance().renderObj().setColorSize(size);
    IceGL::getInstance().renderObj().setColorType(convertType(type));
    IceGL::getInstance().renderObj().setColorStride(stride);
    IceGL::getInstance().renderObj().setColorPointer(pointer);
}

GLAPI void APIENTRY glCopyTexImage1D(GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLint border)
{
    SPDLOG_DEBUG("glCopyTexImage1D not implemented");
}

GLAPI void APIENTRY glCopyTexImage2D(GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border)
{
    SPDLOG_DEBUG("glCopyTexImage2D not implemented");
}

GLAPI void APIENTRY glCopyTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width)
{
    SPDLOG_DEBUG("glCopyTexSubImage1D not implemented");
}

GLAPI void APIENTRY glCopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
    SPDLOG_DEBUG("glCopyTexSubImage2D not implemented");
}

GLAPI void APIENTRY glDeleteTextures(GLsizei n, const GLuint *textures)
{
    SPDLOG_DEBUG("glDeleteTextures {} called", n);
    IceGL::getInstance().setError(GL_NO_ERROR);
    if (n < 0)
    {
        IceGL::getInstance().setError(GL_INVALID_VALUE);
        return;
    }

    for (GLsizei i = 0; i < n; i++)
    {
        // From the specification: glDeleteTextures silently ignores 0's and names that do not correspond to existing textures.
        if (textures[i] != 0) 
        {
            SPDLOG_DEBUG("glDeleteTextures delete {}", textures[i]);
            IceGL::getInstance().pixelPipeline().deleteTexture(textures[i]);
        }
        
    }
}

GLAPI void APIENTRY glDisableClientState(GLenum cap)
{
    SPDLOG_DEBUG("glDisableClientState cap {} called", cap);
    setClientState(cap, false);
}

GLAPI void APIENTRY glDrawArrays(GLenum mode, GLint first, GLsizei count)
{
    SPDLOG_DEBUG("glDrawArrays mode {} first {} count {} called", mode, first, count);

    IceGL::getInstance().renderObj().setCount(count);
    IceGL::getInstance().renderObj().setArrayOffset(first);
    IceGL::getInstance().renderObj().setDrawMode(convertDrawMode(mode));
    IceGL::getInstance().renderObj().enableIndices(false);
    IceGL::getInstance().renderObj().setVertexColor(IceGL::getInstance().vertexQueue().color());

    IceGL::getInstance().vertexPipeline().drawObj(IceGL::getInstance().renderObj());
}

GLAPI void APIENTRY glDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices)
{
    SPDLOG_DEBUG("glDrawElements mode {} count {} type {} called", mode, count, type);
    IceGL::getInstance().setError(GL_NO_ERROR);
    switch (type) {
    case GL_UNSIGNED_BYTE:
        IceGL::getInstance().renderObj().setIndicesType(RenderObj::Type::BYTE);
        break;
    case GL_UNSIGNED_SHORT:
        IceGL::getInstance().renderObj().setIndicesType(RenderObj::Type::SHORT);
        break;
    case GL_UNSIGNED_INT:
        IceGL::getInstance().renderObj().setIndicesType(RenderObj::Type::UNSIGNED_INT);
        break;
    default:
        IceGL::getInstance().setError(GL_INVALID_ENUM);
        return;
    }

    IceGL::getInstance().renderObj().setCount(count);
    IceGL::getInstance().renderObj().setDrawMode(convertDrawMode(mode));
    IceGL::getInstance().renderObj().setIndicesPointer(indices);
    IceGL::getInstance().renderObj().enableIndices(true);
    IceGL::getInstance().renderObj().setVertexColor(IceGL::getInstance().vertexQueue().color());

    if (IceGL::getInstance().getError() == GL_NO_ERROR)
    {
        IceGL::getInstance().vertexPipeline().drawObj(IceGL::getInstance().renderObj());
    }
}

GLAPI void APIENTRY glEdgeFlagPointer(GLsizei stride, const GLvoid *pointer)
{
    SPDLOG_DEBUG("glEdgeFlagPointer not implemented");
}

GLAPI void APIENTRY glEnableClientState(GLenum cap)
{
    SPDLOG_DEBUG("glEnableClientState cap {} called", cap);
    setClientState(cap, true);
}

GLAPI void APIENTRY glGenTextures(GLsizei n, GLuint *textures)
{
    SPDLOG_DEBUG("glGenTextures {} called", n);
    IceGL::getInstance().setError(GL_NO_ERROR);
    if (n < 0)
    {
        IceGL::getInstance().setError(GL_INVALID_VALUE);
        return;
    }

    for (GLsizei i = 0; i < n; i++)
    {
        std::pair<bool, uint16_t> ret { IceGL::getInstance().pixelPipeline().createTexture() };
        if (ret.first)
        {
            textures[i] = ret.second;
        }
        else
        {
            // TODO: Free allocated textures to avoid leaks
            SPDLOG_ERROR("glGenTextures Out Of Memory");
            return;
        }
    }
}

GLAPI void APIENTRY glGetPointerv(GLenum pname, GLvoid **params)
{
    SPDLOG_DEBUG("glGetPointerv not implemented");
}

GLAPI GLboolean APIENTRY glIsTexture(GLuint texture)
{
    SPDLOG_DEBUG("glIsTexture not implemented");
    return false;
}

GLAPI void APIENTRY glIndexPointer(GLenum type, GLsizei stride, const GLvoid *pointer)
{
    SPDLOG_DEBUG("glIndexPointer not implemented");
}

GLAPI void APIENTRY glIndexub(GLubyte c)
{
    SPDLOG_DEBUG("glIndexub not implemented");
}

GLAPI void APIENTRY glIndexubv(const GLubyte *c)
{
    SPDLOG_DEBUG("glIndexubv not implemented");
}

GLAPI void APIENTRY glInterleavedArrays(GLenum format, GLsizei stride, const GLvoid *pointer)
{
    SPDLOG_DEBUG("glInterleavedArrays not implemented");
}

GLAPI void APIENTRY glNormalPointer(GLenum type, GLsizei stride, const GLvoid *pointer)
{
    SPDLOG_DEBUG("glNormalPointer type {} stride {} called", type, stride);

    IceGL::getInstance().renderObj().setNormalType(convertType(type));
    IceGL::getInstance().renderObj().setNormalStride(stride);
    IceGL::getInstance().renderObj().setNormalPointer(pointer);
}

GLAPI void APIENTRY glPolygonOffset(GLfloat factor, GLfloat units)
{
    SPDLOG_DEBUG("glPolygonOffset not implemented");
}

GLAPI void APIENTRY glPopClientAttrib(void)
{
    SPDLOG_DEBUG("glPopClientAttrib not implemented");
}

GLAPI void APIENTRY glPrioritizeTextures(GLsizei n, const GLuint *textures, const GLclampf *priorities)
{
    SPDLOG_DEBUG("glPrioritizeTextures not implemented");
}

GLAPI void APIENTRY glPushClientAttrib(GLbitfield mask)
{
    SPDLOG_DEBUG("glPushClientAttrib not implemented");
}

GLAPI void APIENTRY glTexCoordPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
    SPDLOG_DEBUG("glTexCoordPointer size {} type {} stride {} called", size, type, stride);

    IceGL::getInstance().renderObj().setTexCoordSize(size);
    IceGL::getInstance().renderObj().setTexCoordType(convertType(type));
    IceGL::getInstance().renderObj().setTexCoordStride(stride);
    IceGL::getInstance().renderObj().setTexCoordPointer(pointer);
}

GLAPI void APIENTRY glTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels)
{
    SPDLOG_DEBUG("glTexSubImage1D not implemented");
}

GLAPI void APIENTRY glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels)
{
    SPDLOG_DEBUG("glTexSubImage2D target {} level {} xoffset {} yoffset {} width {} height {} format {} type {} called", target, level, xoffset, yoffset, width, height, format, type);

    IceGL::getInstance().setError(GL_NO_ERROR);

    PixelPipeline::TextureObject texObj { IceGL::getInstance().pixelPipeline().getTexture() };

    if (level != 0)
    {
        SPDLOG_DEBUG("glTexSubImage2D mip mapping not supported. Only level 0 is used.");
        return;
    }

    std::shared_ptr<uint16_t> texMemShared(new uint16_t[(texObj.width * texObj.height * 2)], [] (const uint16_t *p) { delete [] p; });
    if (!texMemShared)
    {
        SPDLOG_ERROR("glTexSubImage2D Out Of Memory");
        return;
    }

    // In case, the current object contains pixel data, copy the data. Otherwise just initialize the memory.
    if (texObj.pixels)
    {
        memcpy(texMemShared.get(), texObj.pixels.get(), texObj.width * texObj.height * 2);
    }
    else
    {
        // Initialize the memory with zero for non power of two textures.
        // Its probably the most reasonable init, because if the alpha channel is activated,
        // then the not used area is then just transparent.
        memset(texMemShared.get(), 0, texObj.width * texObj.height * 2);
    }

    // Check if pixels is null. If so, just set the empty memory area and don't copy anything.
    if (pixels != nullptr)
    {
        int32_t i = 0;
        for (int32_t y = yoffset; y < height; y++)
        {
            for (int32_t x = xoffset; x < width; x++)
            {
                const int32_t texPos { (y * width) + x };
                switch (format)
                {
                    case GL_RGB:
                        switch (type)
                        {
                            case GL_UNSIGNED_SHORT_5_6_5:
                                {
                                    const uint16_t color = reinterpret_cast< const uint16_t*>(pixels)[i];
                                    texMemShared.get()[texPos] = texObj.convertColor(
                                        ((color >> 11) & 0x1f) << 3, 
                                        ((color >> 5) & 0x3f) << 2, 
                                        (color & 0x1f) << 3, 
                                        0xff);
                                    i++;
                                }
                                break;
                            case GL_UNSIGNED_BYTE:
                                texMemShared.get()[texPos] = texObj.convertColor(
                                    reinterpret_cast<const uint8_t*>(pixels)[i + 0], 
                                    reinterpret_cast<const uint8_t*>(pixels)[i + 1], 
                                    reinterpret_cast<const uint8_t*>(pixels)[i + 2], 
                                    0xff);
                                i += 3;
                                break;
                            case GL_BYTE:
                            case GL_BITMAP:
                            case GL_UNSIGNED_SHORT:
                            case GL_UNSIGNED_INT:
                            case GL_INT:
                            case GL_FLOAT:
                            case GL_UNSIGNED_BYTE_3_3_2:
                            case GL_UNSIGNED_BYTE_2_3_3_REV:
                            case GL_UNSIGNED_SHORT_5_6_5_REV:
                                SPDLOG_WARN("glTexSubImage2D unsupported type");
                                return;
                            case GL_UNSIGNED_SHORT_5_5_5_1:
                            case GL_UNSIGNED_SHORT_1_5_5_5_REV:
                            case GL_UNSIGNED_SHORT_4_4_4_4:
                            case GL_UNSIGNED_SHORT_4_4_4_4_REV:
                            case GL_UNSIGNED_INT_8_8_8_8:
                            case GL_UNSIGNED_INT_8_8_8_8_REV:
                            case GL_UNSIGNED_INT_10_10_10_2:
                            case GL_UNSIGNED_INT_2_10_10_10_REV:
                                SPDLOG_WARN("glTexSubImage2D invalid operation");
                                IceGL::getInstance().setError(GL_INVALID_OPERATION);
                                return;
                            default:
                                SPDLOG_WARN("glTexSubImage2D invalid type");
                                IceGL::getInstance().setError(GL_INVALID_ENUM);
                                return;
                        }
                        break;
                    case GL_RGBA:
                        switch (type)
                        {
                            case GL_UNSIGNED_SHORT_5_5_5_1:
                                {
                                    const uint16_t color = reinterpret_cast< const uint16_t*>(pixels)[i];
                                    texMemShared.get()[texPos] = texObj.convertColor(
                                        ((color >> 11) & 0x1f) << 3, 
                                        ((color >> 6) & 0x1f) << 3, 
                                        ((color >> 1) & 0x1f) << 3, 
                                        (color & 0x1) << 7);
                                    i++;
                                }
                                break;
                            case GL_UNSIGNED_SHORT_4_4_4_4:
                                {
                                    const uint16_t color = reinterpret_cast< const uint16_t*>(pixels)[i];
                                    texMemShared.get()[texPos] = texObj.convertColor(
                                        ((color >> 12) & 0xf) << 4, 
                                        ((color >> 8) & 0xf) << 4, 
                                        ((color >> 4) & 0xf) << 4, 
                                        (color & 0xf) << 4);
                                    i++;
                                }
                                break;
                            case GL_UNSIGNED_BYTE:
                                texMemShared.get()[texPos] = texObj.convertColor(
                                    reinterpret_cast<const uint8_t*>(pixels)[i + 0], 
                                    reinterpret_cast<const uint8_t*>(pixels)[i + 1], 
                                    reinterpret_cast<const uint8_t*>(pixels)[i + 2], 
                                    reinterpret_cast<const uint8_t*>(pixels)[i + 3]);
                                    i += 4;
                                break;
                            case GL_BYTE:
                            case GL_BITMAP:
                            case GL_UNSIGNED_SHORT:
                            case GL_UNSIGNED_INT:
                            case GL_INT:
                            case GL_FLOAT:
                            case GL_UNSIGNED_SHORT_1_5_5_5_REV:
                            case GL_UNSIGNED_SHORT_4_4_4_4_REV:
                            case GL_UNSIGNED_INT_8_8_8_8:
                            case GL_UNSIGNED_INT_8_8_8_8_REV:
                            case GL_UNSIGNED_INT_10_10_10_2:
                            case GL_UNSIGNED_INT_2_10_10_10_REV:
                                SPDLOG_WARN("glTexSubImage2D unsupported type");
                                return;
                            case GL_UNSIGNED_BYTE_3_3_2:
                            case GL_UNSIGNED_BYTE_2_3_3_REV:
                            case GL_UNSIGNED_SHORT_5_6_5:
                            case GL_UNSIGNED_SHORT_5_6_5_REV:
                                SPDLOG_WARN("glTexSubImage2D invalid operation");
                                IceGL::getInstance().setError(GL_INVALID_OPERATION);
                                return;
                            default:
                                SPDLOG_WARN("glTexSubImage2D invalid type");
                                IceGL::getInstance().setError(GL_INVALID_ENUM);
                                return;
                        }
                        break;
                    case GL_ALPHA:
                    case GL_RED:
                    case GL_GREEN:
                    case GL_BLUE:
                    case GL_BGR:
                    case GL_BGRA:
                        switch (type)
                        {
                            case GL_UNSIGNED_SHORT_1_5_5_5_REV:
                                {
                                    const uint16_t color = reinterpret_cast< const uint16_t*>(pixels)[i];
                                    texMemShared.get()[texPos] = texObj.convertColor(
                                        ((color >> 10) & 0x1f) << 3, 
                                        ((color >> 5) & 0x1f) << 3, 
                                        ((color >> 0) & 0x1f) << 3,
                                        ((color >> 15) & 0x1) << 7);
                                    i++;
                                }
                                break;
                            case GL_UNSIGNED_SHORT_4_4_4_4_REV:
                                {
                                    const uint16_t color = reinterpret_cast< const uint16_t*>(pixels)[i];
                                    texMemShared.get()[texPos] = texObj.convertColor(
                                        ((color >> 8) & 0xf) << 4, 
                                        ((color >> 4) & 0xf) << 4, 
                                        ((color >> 0) & 0xf) << 4, 
                                        ((color >> 12) & 0xf) << 4);
                                    i++;
                                }
                                break;
                            case GL_UNSIGNED_BYTE:
                                texMemShared.get()[texPos] = texObj.convertColor(
                                    reinterpret_cast<const uint8_t*>(pixels)[i + 2], 
                                    reinterpret_cast<const uint8_t*>(pixels)[i + 1], 
                                    reinterpret_cast<const uint8_t*>(pixels)[i + 0], 
                                    reinterpret_cast<const uint8_t*>(pixels)[i + 3]);
                                    i += 4;
                                break;
                            case GL_BYTE:
                            case GL_BITMAP:
                            case GL_UNSIGNED_SHORT:
                            case GL_UNSIGNED_INT:
                            case GL_INT:
                            case GL_FLOAT:
                            case GL_UNSIGNED_SHORT_5_5_5_1:
                            case GL_UNSIGNED_SHORT_4_4_4_4:
                            case GL_UNSIGNED_INT_8_8_8_8:
                            case GL_UNSIGNED_INT_8_8_8_8_REV:
                            case GL_UNSIGNED_INT_10_10_10_2:
                            case GL_UNSIGNED_INT_2_10_10_10_REV:
                                SPDLOG_WARN("glTexSubImage2D unsupported type");
                                return;
                            case GL_UNSIGNED_BYTE_3_3_2:
                            case GL_UNSIGNED_BYTE_2_3_3_REV:
                            case GL_UNSIGNED_SHORT_5_6_5:
                            case GL_UNSIGNED_SHORT_5_6_5_REV:
                                SPDLOG_WARN("glTexSubImage2D invalid operation");
                                IceGL::getInstance().setError(GL_INVALID_OPERATION);
                                return;
                            default:
                                SPDLOG_WARN("glTexSubImage2D invalid type");
                                IceGL::getInstance().setError(GL_INVALID_ENUM);
                                return;
                        }
                    case GL_LUMINANCE:
                    case GL_LUMINANCE_ALPHA:
                        SPDLOG_WARN("glTexSubImage2D unsupported format");
                        return;
                    default:
                        SPDLOG_WARN("glTexSubImage2D invalid format");
                        IceGL::getInstance().setError(GL_INVALID_ENUM);
                        return;
                }
            }
        }
    }

    texObj.pixels = texMemShared;       
    if (!IceGL::getInstance().pixelPipeline().uploadTexture(texObj))
    {
        IceGL::getInstance().setError(GL_INVALID_VALUE);
        return;
    }
}

GLAPI void APIENTRY glVertexPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
    SPDLOG_DEBUG("glVertexPointer size {} type {} stride {} called", size, type, stride);

    IceGL::getInstance().renderObj().setVertexSize(size);
    IceGL::getInstance().renderObj().setVertexType(convertType(type));
    IceGL::getInstance().renderObj().setVertexStride(stride);
    IceGL::getInstance().renderObj().setVertexPointer(pointer);
}

// -------------------------------------------------------

// Open GL 1.2
// -------------------------------------------------------
GLAPI void APIENTRY glDrawRangeElements(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices)
{
    SPDLOG_DEBUG("glDrawRangeElements not implemented");
}

GLAPI void APIENTRY glTexImage3D(GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *data)
{
    SPDLOG_DEBUG("glTexImage3D not implemented");
}

GLAPI void APIENTRY glTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels)
{
    SPDLOG_DEBUG("glTexSubImage3D not implemented");
}

GLAPI void APIENTRY glCopyTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
    SPDLOG_DEBUG("glCopyTexSubImage3D not implemented");
}

// -------------------------------------------------------

// Open GL 1.3
// -------------------------------------------------------
GLAPI void APIENTRY glActiveTexture(GLenum texture)
{
    SPDLOG_DEBUG("glActiveTexture not implemented");
}

GLAPI void APIENTRY glSampleCoverage(GLfloat value, GLboolean invert)
{
    SPDLOG_DEBUG("glSampleCoverage not implemented");
}

GLAPI void APIENTRY glCompressedTexImage3D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid *data)
{
    SPDLOG_DEBUG("glCompressedTexImage3D not implemented");
}

GLAPI void APIENTRY glCompressedTexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data)
{
    SPDLOG_DEBUG("glCompressedTexImage2D not implemented");
}

GLAPI void APIENTRY glCompressedTexImage1D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid *data)
{
    SPDLOG_DEBUG("glCompressedTexImage1D not implemented");
}

GLAPI void APIENTRY glCompressedTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid *data)
{
    SPDLOG_DEBUG("glCompressedTexSubImage3D not implemented");
}

GLAPI void APIENTRY glCompressedTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data)
{
    SPDLOG_DEBUG("glCompressedTexSubImage2D not implemented");
}

GLAPI void APIENTRY glCompressedTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid *data)
{
    SPDLOG_DEBUG("glCompressedTexSubImage1D not implemented");
}

GLAPI void APIENTRY glGetCompressedTexImage(GLenum target, GLint level, GLvoid *pixels)
{
    SPDLOG_DEBUG("glGetCompressedTexImage not implemented");
}

GLAPI void APIENTRY glClientActiveTexture(GLenum texture)
{
    SPDLOG_DEBUG("glClientActiveTexture not implemented");
}

GLAPI void APIENTRY glMultiTexCoord1d(GLenum target, GLdouble s)
{
    SPDLOG_DEBUG("glMultiTexCoord1d not implemented");
}

GLAPI void APIENTRY glMultiTexCoord1dv(GLenum target, const GLdouble *v)
{
    SPDLOG_DEBUG("glMultiTexCoord1dv not implemented");
}

GLAPI void APIENTRY glMultiTexCoord1f(GLenum target, GLfloat s)
{
    SPDLOG_DEBUG("glMultiTexCoord1f not implemented");
}

GLAPI void APIENTRY glMultiTexCoord1fv(GLenum target, const GLfloat *v)
{
    SPDLOG_DEBUG("glMultiTexCoord1fv not implemented");
}

GLAPI void APIENTRY glMultiTexCoord1i(GLenum target, GLint s)
{
    SPDLOG_DEBUG("glMultiTexCoord1i not implemented");
}

GLAPI void APIENTRY glMultiTexCoord1iv(GLenum target, const GLint *v)
{
    SPDLOG_DEBUG("glMultiTexCoord1iv not implemented");
}

GLAPI void APIENTRY glMultiTexCoord1s(GLenum target, GLshort s)
{
    SPDLOG_DEBUG("glMultiTexCoord1s not implemented");
}

GLAPI void APIENTRY glMultiTexCoord1sv(GLenum target, const GLshort *v)
{
    SPDLOG_DEBUG("glMultiTexCoord1sv not implemented");
}

GLAPI void APIENTRY glMultiTexCoord2d(GLenum target, GLdouble s, GLdouble t)
{
    SPDLOG_DEBUG("glMultiTexCoord2d not implemented");
}

GLAPI void APIENTRY glMultiTexCoord2dv(GLenum target, const GLdouble *v)
{
    SPDLOG_DEBUG("glMultiTexCoord2dv not implemented");
}

GLAPI void APIENTRY glMultiTexCoord2f(GLenum target, GLfloat s, GLfloat t)
{
    SPDLOG_DEBUG("glMultiTexCoord2f not implemented");
}

GLAPI void APIENTRY glMultiTexCoord2fv(GLenum target, const GLfloat *v)
{
    SPDLOG_DEBUG("glMultiTexCoord2fv not implemented");
}

GLAPI void APIENTRY glMultiTexCoord2i(GLenum target, GLint s, GLint t)
{
    SPDLOG_DEBUG("glMultiTexCoord2i not implemented");
}

GLAPI void APIENTRY glMultiTexCoord2iv(GLenum target, const GLint *v)
{
    SPDLOG_DEBUG("glMultiTexCoord2iv not implemented");
}

GLAPI void APIENTRY glMultiTexCoord2s(GLenum target, GLshort s, GLshort t)
{
    SPDLOG_DEBUG("glMultiTexCoord2s not implemented");
}

GLAPI void APIENTRY glMultiTexCoord2sv(GLenum target, const GLshort *v)
{
    SPDLOG_DEBUG("glMultiTexCoord2sv not implemented");
}

GLAPI void APIENTRY glMultiTexCoord3d(GLenum target, GLdouble s, GLdouble t, GLdouble r)
{
    SPDLOG_DEBUG("glMultiTexCoord3d not implemented");
}

GLAPI void APIENTRY glMultiTexCoord3dv(GLenum target, const GLdouble *v)
{
    SPDLOG_DEBUG("glMultiTexCoord3dv not implemented");
}

GLAPI void APIENTRY glMultiTexCoord3f(GLenum target, GLfloat s, GLfloat t, GLfloat r)
{
    SPDLOG_DEBUG("glMultiTexCoord3f not implemented");
}

GLAPI void APIENTRY glMultiTexCoord3fv(GLenum target, const GLfloat *v)
{
    SPDLOG_DEBUG("glMultiTexCoord3fv not implemented");
}

GLAPI void APIENTRY glMultiTexCoord3i(GLenum target, GLint s, GLint t, GLint r)
{
    SPDLOG_DEBUG("glMultiTexCoord3i not implemented");
}

GLAPI void APIENTRY glMultiTexCoord3iv(GLenum target, const GLint *v)
{
    SPDLOG_DEBUG("glMultiTexCoord3iv not implemented");
}

GLAPI void APIENTRY glMultiTexCoord3s(GLenum target, GLshort s, GLshort t, GLshort r)
{
    SPDLOG_DEBUG("glMultiTexCoord3s not implemented");
}

GLAPI void APIENTRY glMultiTexCoord3sv(GLenum target, const GLshort *v)
{
    SPDLOG_DEBUG("glMultiTexCoord3sv not implemented");
}

GLAPI void APIENTRY glMultiTexCoord4d(GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q)
{
    SPDLOG_DEBUG("glMultiTexCoord4d not implemented");
}

GLAPI void APIENTRY glMultiTexCoord4dv(GLenum target, const GLdouble *v)
{
    SPDLOG_DEBUG("glMultiTexCoord4dv not implemented");
}

GLAPI void APIENTRY glMultiTexCoord4f(GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q)
{
    SPDLOG_DEBUG("glMultiTexCoord4f not implemented");
}

GLAPI void APIENTRY glMultiTexCoord4fv(GLenum target, const GLfloat *v)
{
    SPDLOG_DEBUG("glMultiTexCoord4fv not implemented");
}

GLAPI void APIENTRY glMultiTexCoord4i(GLenum target, GLint s, GLint t, GLint r, GLint q)
{
    SPDLOG_DEBUG("glMultiTexCoord4i not implemented");
}

GLAPI void APIENTRY glMultiTexCoord4iv(GLenum target, const GLint *v)
{
    SPDLOG_DEBUG("glMultiTexCoord4iv not implemented");
}

GLAPI void APIENTRY glMultiTexCoord4s(GLenum target, GLshort s, GLshort t, GLshort r, GLshort q)
{
    SPDLOG_DEBUG("glMultiTexCoord4s not implemented");
}

GLAPI void APIENTRY glMultiTexCoord4sv(GLenum target, const GLshort *v)
{
    SPDLOG_DEBUG("glMultiTexCoord4sv not implemented");
}

GLAPI void APIENTRY glLoadTransposeMatrixf(const GLfloat *m)
{
    SPDLOG_DEBUG("glLoadTransposeMatrixf not implemented");
}

GLAPI void APIENTRY glLoadTransposeMatrixd(const GLdouble *m)
{
    SPDLOG_DEBUG("glLoadTransposeMatrixd not implemented");
}

GLAPI void APIENTRY glMultTransposeMatrixf(const GLfloat *m)
{
    SPDLOG_DEBUG("glMultTransposeMatrixf not implemented");
}

GLAPI void APIENTRY glMultTransposeMatrixd(const GLdouble *m)
{
    SPDLOG_DEBUG("glMultTransposeMatrixd not implemented");
}

// -------------------------------------------------------

// Some extensions
// -------------------------------------------------------
GLAPI void APIENTRY glLockArrays(GLint first, GLsizei count)
{
    SPDLOG_DEBUG("glLockArrays not implemented");
}

GLAPI void APIENTRY glUnlockArrays()
{
    SPDLOG_DEBUG("glUnlockArrays not implemented");
}

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

#ifndef ICEGL_HPP
#define ICEGL_HPP

#include <vector>
#include <array>
#include <functional>
#include "IRenderer.hpp"
#include "VertexPipeline.hpp"
#include "Vec.hpp"
#include "Lighting.hpp"
#include "TexGen.hpp"
#include "RenderObj.hpp"
#include "PixelPipeline.hpp"
#include "VertexQueue.hpp"
#include "gl.h"

class IceGL
{
public:
    IceGL(IRenderer& renderer);

    void glMatrixMode(GLenum mm);
    void glMultMatrixf(const GLfloat* m);
    void glMultMatrixd(const GLdouble* m);
    void glTranslatef(GLfloat x, GLfloat y, GLfloat z);
    void glRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
    void glScalef(GLfloat x, GLfloat y, GLfloat z);
    void glLoadIdentity();
    void glOrthof(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat near, GLfloat far);
    void glViewport(GLint x, GLint y, GLsizei width, GLsizei height);
    void glDepthRange(GLclampd zNear, GLclampd zFar);
    void glBegin(GLenum mode);
    void glEnd();
    void glRect(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2);
    void glVertex2f(GLfloat x, GLfloat y);
    void glVertex3f(GLfloat x, GLfloat y, GLfloat z);
    void glNormal3f(GLfloat nx, GLfloat ny, GLfloat nz);
    void glTexCoord2f(GLfloat s, GLfloat t);
    void glTexGeni(GLenum coord, GLenum pname, GLint param);
    void glTexGenfv(GLenum coord, GLenum pname, const GLfloat* param);
    void glColor4f(	GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
    void glColor4ub(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha);
    void glColor3f(	GLfloat red, GLfloat green, GLfloat blue);
    void glColor3ub(GLubyte red, GLubyte green, GLubyte blue);
    void glColor3dv(const GLdouble* v);
    void glColor4dv(const GLdouble* v);
    void glColor4fv(const GLfloat* v);
    void glVertexPointer(GLint size, GLenum type, GLsizei stride, const void* pointer);
    void glTexCoordPointer(GLint size, GLenum type, GLsizei stride, const void* pointer);
    void glNormalPointer(GLenum type, GLsizei stride, const void* pointer);
    void glColorPointer(GLint size, GLenum type, GLsizei stride, const void* pointer);
    void glDrawArrays(GLenum mode, GLint first, GLsizei count);
    void glDrawElements(GLenum mode, GLsizei count, GLenum type, const void* indices);
    void glEnableClientState(GLenum array);
    void glDisableClientState(GLenum array);


    void glTexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
    void glPixelStorei(GLenum pname, GLint param);
    void glGenTextures(GLsizei n, GLuint *textures);
    void glDeleteTextures(GLsizei n, const GLuint * textures);
    void glBindTexture(GLenum target, GLuint texture);
    void glTexParameteri(GLenum target, GLenum pname, GLint param);
    void glTexParameterf(GLenum target, GLenum pname, GLfloat param);
    void glTexEnvi(GLenum target, GLenum pname, GLint param);
    void glTexEnvf(GLenum target, GLenum pname, GLfloat param);
    void glTexEnvfv(GLenum target, GLenum pname, const GLfloat* param);

    void glMaterialf(GLenum face, GLenum pname, GLfloat param);
    void glMaterialfv(GLenum face, GLenum pname, const GLfloat* params);
    void glColorMaterial(GLenum face, GLenum pname);
    void glLightf(GLenum light, GLenum pname, GLfloat param);
    void glLightfv(GLenum light, GLenum pname, const GLfloat* params);
    void glLightModelf(GLenum pname, GLfloat param);
    void glLightModelfv(GLenum pname, const GLfloat* params);

    void glClear(GLbitfield mask);
    void glClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
    void glClearDepthf(GLclampf depth);
    void glClearStencil(GLint s);
    void glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);

    void glDepthMask(GLboolean flag);
    void glDepthFunc(GLenum func);
    void glAlphaFunc(GLenum func, GLclampf ref);

    void glBlendFunc(GLenum sfactor, GLenum dfactor);
    void glLogicOp(GLenum opcode);

    void glFogf(GLenum pname, GLfloat param);
    void glFogfv(GLenum pname, const GLfloat* params);
    void glFogi(GLenum pname, GLint param);
    void glFogiv(GLenum pname, const GLint* params);

    void glPushMatrix();
    void glPopMatrix();

    void glCullFace(GLenum mode);

    void glEnable(GLenum cap);
    void glDisable(GLenum cap);

    const GLubyte* glGetString(GLenum name);
    void glGetIntegerv(GLenum pname, GLint* params);
    void glGetFloatv(GLenum pname, GLfloat* params);
    void glGetBooleanv(GLenum pname, GLboolean* params);

    void gluPerspective(GLfloat fovy, GLfloat aspect, GLfloat zNear, GLfloat zFar);
    void gluLookAt(GLfloat eyex, GLfloat eyey, GLfloat eyez, GLfloat centerx,
                   GLfloat centery, GLfloat centerz, GLfloat upx, GLfloat upy,
                   GLfloat upz);


    static IceGL& getInstance();
    static bool createInstance(IRenderer& renderer);

    void setError(const uint32_t error) { m_error = error; }
    uint32_t getError() const { return m_error; }

    VertexPipeline& vertexPipeline() { return m_vertexPipeline; }
    PixelPipeline& pixelPipeline() { return m_pixelPipeline; }
    RenderObj& renderObj() { return m_renderObj; }
    VertexQueue& vertexQueue() { return m_vertexQueue; }
    void commit();

    static constexpr uint16_t MAX_TEX_SIZE = 256;
private:


    void setClientState(const GLenum array, bool enable);
    // It would be nice to have std::optional, but it does not work with arduino
    // If we have this, we could make all this functions const and return an empty optional if the conversion failed
    PixelPipeline::FragmentPipeline::BlendFunc convertGlBlendFuncToRenderBlendFunc(const GLenum blendFunc);
    RenderObj::Type convertType(GLenum type);
    RenderObj::DrawMode convertDrawMode(GLenum drawMode);
    PixelPipeline::TextureWrapMode convertGlTextureWrapMode(const GLenum mode);
    GLint convertTexEnvMode(PixelPipeline::TexEnvMode& mode, const GLint param);
    GLint convertCombine(PixelPipeline::TexEnv::Combine& conv, GLint val, bool alpha);
    GLint convertOperand(PixelPipeline::TexEnv::Operand& conf, GLint val, bool alpha);
    GLint convertSrcReg(PixelPipeline::TexEnv::SrcReg& conf, GLint val);

    IRenderer& m_renderer;
    PixelPipeline m_pixelPipeline;
    VertexPipeline m_vertexPipeline;
    RenderObj m_renderObj;
    VertexQueue m_vertexQueue {};

    // Errors
    uint32_t m_error { 0 };
};

#endif // ICEGL_HPP

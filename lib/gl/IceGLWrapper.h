// RasteriCEr
// https://github.com/ToNi3141/RasteriCEr
// Copyright (c) 2021 ToNi3141

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

#ifndef ICEGLWRAPPER_H
#define ICEGLWRAPPER_H
#include "IceGLTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

void initIceGlCWrapper(void* wrapper);

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
void glRectf(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2);
void glRecti(GLint x1, GLint y1, GLint x2, GLint y2);
void glVertex2f(GLfloat x, GLfloat y);
void glVertex3f(GLfloat x, GLfloat y, GLfloat z);
void glVertex2dv(const GLdouble* v);
void glNormal3f(GLfloat nx, GLfloat ny, GLfloat nz);
void glTexCoord2f(GLfloat s, GLfloat t);
void glTexCoord2dv(const GLdouble* v);
void glTexCoord2d(GLdouble s, GLdouble t);
void glTexGeni(GLenum coord, GLenum pname, GLint param);
void glTexGenfv(GLenum coord, GLenum pname, const GLfloat* param);
void glColor4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
void glColor4ub(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha);
void glColor3f(GLfloat red, GLfloat green, GLfloat blue);
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
void glTexEnvi(GLenum target, GLenum pname, GLint param);
void glTexEnvf(GLenum target, GLenum pname, GLfloat param);
void glTexEnvfv(GLenum target, GLenum pname, const GLfloat* param);
void glMaterialf(GLenum face, GLenum pname, GLfloat param);
void glMaterialfv(GLenum face, GLenum pname, const GLfloat* params);
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
void glPushMatrix();
void glPopMatrix();
void glEnable(GLenum cap);
void glDisable(GLenum cap);
void gluPerspective(GLfloat fovy, GLfloat aspect, GLfloat zNear, GLfloat zFar);
void gluLookAt(GLfloat eyex, GLfloat eyey, GLfloat eyez, GLfloat centerx,
               GLfloat centery, GLfloat centerz, GLfloat upx, GLfloat upy,
               GLfloat upz);
const GLubyte* glGetString(GLenum name);
void glGetIntegerv(GLenum pname, GLint* param);
void glGetFloatv(GLenum pname, GLfloat* param);
void glGetBooleanv(GLenum pname, GLboolean* param);
void glCullFace(GLenum mode);
void glFogf(GLenum pname, GLfloat param);
void glFogfv(GLenum pname, const GLfloat* params);
void glFogi(GLenum pname, GLint param);
void glFogiv(GLenum pname, const GLint *params);

// Mock
void glHint(GLenum target, GLenum hint);
void glShadeModel(GLenum name);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // ICEGLWRAPPER_H

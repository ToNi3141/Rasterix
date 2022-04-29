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

#include "IceGLWrapper.h"
#include "IceGL.hpp"

IceGL* iceGlCWrap = 0;

void initIceGlCWrapper(void *wrapper) {iceGlCWrap = (IceGL*)wrapper;}

void glMatrixMode(GLenum mm) {iceGlCWrap->glMatrixMode(mm);}
void glMultMatrixf(const GLfloat* m) {iceGlCWrap->glMultMatrixf(m);}
void glMultMatrixd(const GLdouble* m) {iceGlCWrap->glMultMatrixd(m);}
void glTranslatef(GLfloat x, GLfloat y, GLfloat z) {iceGlCWrap->glTranslatef(x, y, z);}
void glRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z) {iceGlCWrap->glRotatef(angle, x, y, z);}
void glScalef(GLfloat x, GLfloat y, GLfloat z) {iceGlCWrap->glScalef(x, y, z);}
void glLoadIdentity() {iceGlCWrap->glLoadIdentity();}
void glOrthof(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat near, GLfloat far) {iceGlCWrap->glOrthof(left, right, bottom, top, near, far);}
void glOrtho(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble near, GLdouble far) {iceGlCWrap->glOrthof(left, right, bottom, top, near, far);}
void glViewport(GLint x, GLint y, GLsizei width, GLsizei height) {iceGlCWrap->glViewport(x, y, width, height);}
void glDepthRange(GLclampd zNear, GLclampd zFar) {iceGlCWrap->glDepthRange(zNear, zFar);}
void glBegin(GLenum mode) {iceGlCWrap->glBegin(mode);}
void glEnd() {iceGlCWrap->glEnd();}
void glRect(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2) {iceGlCWrap->glRect(x1, y1, x2, y2);}
void glRectf(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2) {iceGlCWrap->glRect(x1, y1, x2, y2);}
void glRecti(GLint x1, GLint y1, GLint x2, GLint y2) {iceGlCWrap->glRect(x1, y1, x2, y2);}
void glVertex2f(GLfloat x, GLfloat y) {iceGlCWrap->glVertex2f(x, y);}
void glVertex2dv(const GLdouble* v) {glVertex2f(v[0], v[1]);}
void glVertex3f(GLfloat x, GLfloat y, GLfloat z) {iceGlCWrap->glVertex3f(x, y, z);}
void glNormal3f(GLfloat nx, GLfloat ny, GLfloat nz) {iceGlCWrap->glNormal3f(nx, ny, nz);}
void glTexCoord2f(GLfloat s, GLfloat t) {iceGlCWrap->glTexCoord2f(s, t);}
void glTexCoord2dv(const GLdouble* v) {glTexCoord2f(v[0], v[1]);}
void glTexCoord2d(GLdouble s, GLdouble t) {iceGlCWrap->glTexCoord2f(s, t);}
void glTexGeni(GLenum coord, GLenum pname, GLint param) {iceGlCWrap->glTexGeni(coord, pname, param);}
void glTexGenfv(GLenum coord, GLenum pname, const GLfloat* param) {iceGlCWrap->glTexGenfv(coord, pname, param);}
void glColor4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) {iceGlCWrap->glColor4f(red, green, blue, alpha);}
void glColor4ub(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha) {iceGlCWrap->glColor4ub(red, green, blue, alpha);}
void glColor3f(GLfloat red, GLfloat green, GLfloat blue) {iceGlCWrap->glColor3f(red, green, blue);}
void glColor3ub(GLubyte red, GLubyte green, GLubyte blue) {iceGlCWrap->glColor3ub(red, green, blue);}
void glColor3dv(const GLdouble* v) {iceGlCWrap->glColor3dv(v);}
void glColor4dv(const GLdouble* v) {iceGlCWrap->glColor4dv(v);}
void glColor4fv(const GLfloat* v) {iceGlCWrap->glColor4fv(v);}
void glVertexPointer(GLint size, GLenum type, GLsizei stride, const void* pointer) {iceGlCWrap->glVertexPointer(size, type, stride, pointer);}
void glTexCoordPointer(GLint size, GLenum type, GLsizei stride, const void* pointer) {iceGlCWrap->glTexCoordPointer(size, type, stride, pointer);}
void glNormalPointer(GLenum type, GLsizei stride, const void* pointer) {iceGlCWrap->glNormalPointer(type, stride, pointer);}
void glColorPointer(GLint size, GLenum type, GLsizei stride, const void* pointer) {iceGlCWrap->glColorPointer(size, type, stride, pointer);}
void glDrawArrays(GLenum mode, GLint first, GLsizei count) {iceGlCWrap->glDrawArrays(mode, first, count);}
void glDrawElements(GLenum mode, GLsizei count, GLenum type, const void* indices) {iceGlCWrap->glDrawElements(mode, count, type, indices);}
void glEnableClientState(GLenum array) {iceGlCWrap->glEnableClientState(array);}
void glDisableClientState(GLenum array) {iceGlCWrap->glDisableClientState(array);}
void glTexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels) {iceGlCWrap->glTexImage2D(target, level, internalformat, width, height, border, format, type, pixels);}
void glPixelStorei(GLenum pname, GLint param) {iceGlCWrap->glPixelStorei(pname, param);}
void glGenTextures(GLsizei n, GLuint *textures) {iceGlCWrap->glGenTextures(n, textures);}
void glDeleteTextures(GLsizei n, const GLuint * textures) {iceGlCWrap->glDeleteTextures(n, textures);}
void glBindTexture(GLenum target, GLuint texture) {iceGlCWrap->glBindTexture(target, texture);}
void glTexParameteri(GLenum target, GLenum pname, GLint param) {iceGlCWrap->glTexParameteri(target, pname, param);}
void glTexParameterf(GLenum target, GLenum pname, GLfloat param) {iceGlCWrap->glTexParameterf(target, pname, param);}
void glTexEnvi(GLenum target, GLenum pname, GLint param) {iceGlCWrap->glTexEnvi(target, pname, param);}
void glTexEnvf(GLenum target, GLenum pname, GLfloat param) {iceGlCWrap->glTexEnvf(target, pname, param);}
void glTexEnvfv(GLenum target, GLenum pname, const GLfloat* param) {iceGlCWrap->glTexEnvfv(target, pname, param);}
void glMaterialf(GLenum face, GLenum pname, GLfloat param) {iceGlCWrap->glMaterialf(face, pname, param);}
void glMaterialfv(GLenum face, GLenum pname, const GLfloat* params) {iceGlCWrap->glMaterialfv(face, pname, params);}
void glLightf(GLenum light, GLenum pname, GLfloat param) {iceGlCWrap->glLightf(light, pname, param);}
void glLightfv(GLenum light, GLenum pname, const GLfloat* params) {iceGlCWrap->glLightfv(light, pname, params);}
void glLightModelf(GLenum pname, GLfloat param) {iceGlCWrap->glLightModelf(pname, param);}
void glLightModelfv(GLenum pname, const GLfloat* params) {iceGlCWrap->glLightModelfv(pname, params);}
void glClear(GLbitfield mask) {iceGlCWrap->glClear(mask);}
void glClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha) {iceGlCWrap->glClearColor(red, green, blue, alpha);}
void glClearDepthf(GLclampf depth) {iceGlCWrap->glClearDepthf(depth);}
void glClearStencil(GLint s) {iceGlCWrap->glClearStencil(s);}
void glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha) {iceGlCWrap->glColorMask(red, green, blue, alpha);}
void glDepthMask(GLboolean flag) {iceGlCWrap->glDepthMask(flag);}
void glDepthFunc(GLenum func) {iceGlCWrap->glDepthFunc(func);}
void glAlphaFunc(GLenum func, GLclampf ref) {iceGlCWrap->glAlphaFunc(func, ref);}
void glBlendFunc(GLenum sfactor, GLenum dfactor) {iceGlCWrap->glBlendFunc(sfactor, dfactor);}
void glLogicOp(GLenum opcode) {iceGlCWrap->glLogicOp(opcode);}
void glPushMatrix() {iceGlCWrap->glPushMatrix();}
void glPopMatrix() {iceGlCWrap->glPopMatrix();}
void glEnable(GLenum cap) {iceGlCWrap->glEnable(cap);}
void glDisable(GLenum cap) {iceGlCWrap->glDisable(cap);}
void gluPerspective(GLfloat fovy, GLfloat aspect, GLfloat zNear, GLfloat zFar) {iceGlCWrap->gluPerspective(fovy, aspect, zNear, zFar);}
void gluLookAt(GLfloat eyex, GLfloat eyey, GLfloat eyez, GLfloat centerx,
               GLfloat centery, GLfloat centerz, GLfloat upx, GLfloat upy,
               GLfloat upz) {iceGlCWrap->gluLookAt(eyex, eyey, eyez, centerx, centery, centerz, upx, upy, upz);}
const GLubyte *glGetString(GLenum name) {return iceGlCWrap->glGetString(name);}
void glGetIntegerv(GLenum pname, GLint *param) {iceGlCWrap->glGetIntegerv(pname, param);}
void glGetFloatv(GLenum pname, GLfloat *param) {iceGlCWrap->glGetFloatv(pname, param);}
void glGetBooleanv(GLenum pname, GLboolean *param) {iceGlCWrap->glGetBooleanv(pname, param);}
void glCullFace(GLenum mode){iceGlCWrap->glCullFace(mode);}

void glFogf(GLenum pname, GLfloat param) {iceGlCWrap->glFogf(pname, param);}
void glFogfv(GLenum pname, const GLfloat *params) {iceGlCWrap->glFogfv(pname, params);}
void glFogi(GLenum pname, GLint param) {iceGlCWrap->glFogi(pname, param);}
void glFogiv(GLenum pname, const GLint *params) {iceGlCWrap->glFogiv(pname, params);}

void glShadeModel(GLenum name) { (void)name; }
void glHint(GLenum target, GLenum hint) { (void)target; (void)hint;}



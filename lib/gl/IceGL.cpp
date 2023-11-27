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

#include "IceGL.hpp"
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <spdlog/spdlog.h>

#define ADDRESS_OF(X) reinterpret_cast<const void *>(&X)
namespace rr
{
IceGL* instance { nullptr };

IceGL& IceGL::getInstance()
{
    return *instance;
}

bool IceGL::createInstance(IRenderer& renderer)
{
    if (instance)
    {
        delete instance;
    }
    instance = new IceGL { renderer };
    return instance != nullptr;
}

IceGL::IceGL(IRenderer &renderer)
    : m_renderer(renderer)
    , m_pixelPipeline(renderer)
    , m_vertexPipeline(m_pixelPipeline)
{
    // Preallocate the first texture. This is the default texture which also can't be deleted.
    m_renderer.createTexture();

    // Set initial values
    m_renderer.setTexEnvColor(0, { { 0, 0, 0, 0 } });
    m_renderer.setClearColor({ {0, 0, 0, 0 } });
    m_renderer.setClearDepth(65535);
    m_renderer.setFogColor({ { 255, 255, 255, 255 } });
    std::array<float, 33> fogLut {};
    std::fill(fogLut.begin(), fogLut.end(), 1.0f);
    m_renderer.setFogLut(fogLut, 0.0f, (std::numeric_limits<float>::max)()); // Windows defines macros with max ... parenthesis are a work around against build errors.

    // Register Open GL 1.0 procedures
    addProcedure("glAccum", ADDRESS_OF(glAccum));
    addProcedure("glAlphaFunc", ADDRESS_OF(glAlphaFunc));
    addProcedure("glBegin", ADDRESS_OF(glBegin));
    addProcedure("glBitmap", ADDRESS_OF(glBitmap));
    addProcedure("glBlendFunc", ADDRESS_OF(glBlendFunc));
    addProcedure("glCallList", ADDRESS_OF(glCallList));
    addProcedure("glCallLists", ADDRESS_OF(glCallLists));
    addProcedure("glClear", ADDRESS_OF(glClear));
    addProcedure("glClearAccum", ADDRESS_OF(glClearAccum));
    addProcedure("glClearColor", ADDRESS_OF(glClearColor));
    addProcedure("glClearDepth", ADDRESS_OF(glClearDepth));
    addProcedure("glClearIndex", ADDRESS_OF(glClearIndex));
    addProcedure("glClearStencil", ADDRESS_OF(glClearStencil));
    addProcedure("glClipPlane", ADDRESS_OF(glClipPlane));
    addProcedure("glColor3b", ADDRESS_OF(glColor3b));
    addProcedure("glColor3bv", ADDRESS_OF(glColor3bv));
    addProcedure("glColor3d", ADDRESS_OF(glColor3d));
    addProcedure("glColor3dv", ADDRESS_OF(glColor3dv));
    addProcedure("glColor3f", ADDRESS_OF(glColor3f));
    addProcedure("glColor3fv", ADDRESS_OF(glColor3fv));
    addProcedure("glColor3i", ADDRESS_OF(glColor3i));
    addProcedure("glColor3iv", ADDRESS_OF(glColor3iv));
    addProcedure("glColor3s", ADDRESS_OF(glColor3s));
    addProcedure("glColor3sv", ADDRESS_OF(glColor3sv));
    addProcedure("glColor3ub", ADDRESS_OF(glColor3ub));
    addProcedure("glColor3ubv", ADDRESS_OF(glColor3ubv));
    addProcedure("glColor3ui", ADDRESS_OF(glColor3ui));
    addProcedure("glColor3uiv", ADDRESS_OF(glColor3uiv));
    addProcedure("glColor3us", ADDRESS_OF(glColor3us));
    addProcedure("glColor3usv", ADDRESS_OF(glColor3usv));
    addProcedure("glColor4b", ADDRESS_OF(glColor4b));
    addProcedure("glColor4bv", ADDRESS_OF(glColor4bv));
    addProcedure("glColor4d", ADDRESS_OF(glColor4d));
    addProcedure("glColor4dv", ADDRESS_OF(glColor4dv));
    addProcedure("glColor4f", ADDRESS_OF(glColor4f));
    addProcedure("glColor4fv", ADDRESS_OF(glColor4fv));
    addProcedure("glColor4i", ADDRESS_OF(glColor4i));
    addProcedure("glColor4iv", ADDRESS_OF(glColor4iv));
    addProcedure("glColor4s", ADDRESS_OF(glColor4s));
    addProcedure("glColor4sv", ADDRESS_OF(glColor4sv));
    addProcedure("glColor4ub", ADDRESS_OF(glColor4ub));
    addProcedure("glColor4ubv", ADDRESS_OF(glColor4ubv));
    addProcedure("glColor4ui", ADDRESS_OF(glColor4ui));
    addProcedure("glColor4uiv", ADDRESS_OF(glColor4uiv));
    addProcedure("glColor4us", ADDRESS_OF(glColor4us));
    addProcedure("glColor4usv", ADDRESS_OF(glColor4usv));
    addProcedure("glColorMask", ADDRESS_OF(glColorMask));
    addProcedure("glColorMaterial", ADDRESS_OF(glColorMaterial));
    addProcedure("glCopyPixels", ADDRESS_OF(glCopyPixels));
    addProcedure("glCullFace", ADDRESS_OF(glCullFace));
    addProcedure("glDeleteLists", ADDRESS_OF(glDeleteLists));
    addProcedure("glDepthFunc", ADDRESS_OF(glDepthFunc));
    addProcedure("glDepthMask", ADDRESS_OF(glDepthMask));
    addProcedure("glDepthRange", ADDRESS_OF(glDepthRange));
    addProcedure("glDisable", ADDRESS_OF(glDisable));
    addProcedure("glDrawBuffer", ADDRESS_OF(glDrawBuffer));
    addProcedure("glDrawPixels", ADDRESS_OF(glDrawPixels));
    addProcedure("glEdgeFlag", ADDRESS_OF(glEdgeFlag));
    addProcedure("glEdgeFlagv", ADDRESS_OF(glEdgeFlagv));
    addProcedure("glEnable", ADDRESS_OF(glEnable));
    addProcedure("glEnd", ADDRESS_OF(glEnd));
    addProcedure("glEndList", ADDRESS_OF(glEndList));
    addProcedure("glEvalCoord1d", ADDRESS_OF(glEvalCoord1d));
    addProcedure("glEvalCoord1dv", ADDRESS_OF(glEvalCoord1dv));
    addProcedure("glEvalCoord1f", ADDRESS_OF(glEvalCoord1f));
    addProcedure("glEvalCoord1fv", ADDRESS_OF(glEvalCoord1fv));
    addProcedure("glEvalCoord2d", ADDRESS_OF(glEvalCoord2d));
    addProcedure("glEvalCoord2dv", ADDRESS_OF(glEvalCoord2dv));
    addProcedure("glEvalCoord2f", ADDRESS_OF(glEvalCoord2f));
    addProcedure("glEvalCoord2fv", ADDRESS_OF(glEvalCoord2fv));
    addProcedure("glEvalMesh1", ADDRESS_OF(glEvalMesh1));
    addProcedure("glEvalMesh2", ADDRESS_OF(glEvalMesh2));
    addProcedure("glEvalPoint1", ADDRESS_OF(glEvalPoint1));
    addProcedure("glEvalPoint2", ADDRESS_OF(glEvalPoint2));
    addProcedure("glFeedbackBuffer", ADDRESS_OF(glFeedbackBuffer));
    addProcedure("glFinish", ADDRESS_OF(glFinish));
    addProcedure("glFlush", ADDRESS_OF(glFlush));
    addProcedure("glFogf", ADDRESS_OF(glFogf));
    addProcedure("glFogfv", ADDRESS_OF(glFogfv));
    addProcedure("glFogi", ADDRESS_OF(glFogi));
    addProcedure("glFogiv", ADDRESS_OF(glFogiv));
    addProcedure("glFrontFace", ADDRESS_OF(glFrontFace));
    addProcedure("glFrustum", ADDRESS_OF(glFrustum));
    addProcedure("glGenLists", ADDRESS_OF(glGenLists));
    addProcedure("glGetBooleanv", ADDRESS_OF(glGetBooleanv));
    addProcedure("glGetClipPlane", ADDRESS_OF(glGetClipPlane));
    addProcedure("glGetDoublev", ADDRESS_OF(glGetDoublev));
    addProcedure("glGetError", ADDRESS_OF(glGetError));
    addProcedure("glGetFloatv", ADDRESS_OF(glGetFloatv));
    addProcedure("glGetIntegerv", ADDRESS_OF(glGetIntegerv));
    addProcedure("glGetLightfv", ADDRESS_OF(glGetLightfv));
    addProcedure("glGetLightiv", ADDRESS_OF(glGetLightiv));
    addProcedure("glGetMapdv", ADDRESS_OF(glGetMapdv));
    addProcedure("glGetMapfv", ADDRESS_OF(glGetMapfv));
    addProcedure("glGetMapiv", ADDRESS_OF(glGetMapiv));
    addProcedure("glGetMaterialfv", ADDRESS_OF(glGetMaterialfv));
    addProcedure("glGetMaterialiv", ADDRESS_OF(glGetMaterialiv));
    addProcedure("glGetPixelMapfv", ADDRESS_OF(glGetPixelMapfv));
    addProcedure("glGetPixelMapuiv", ADDRESS_OF(glGetPixelMapuiv));
    addProcedure("glGetPixelMapusv", ADDRESS_OF(glGetPixelMapusv));
    addProcedure("glGetPolygonStipple", ADDRESS_OF(glGetPolygonStipple));
    addProcedure("glGetString", ADDRESS_OF(glGetString));
    addProcedure("glGetTexEnvfv", ADDRESS_OF(glGetTexEnvfv));
    addProcedure("glGetTexEnviv", ADDRESS_OF(glGetTexEnviv));
    addProcedure("glGetTexGendv", ADDRESS_OF(glGetTexGendv));
    addProcedure("glGetTexGenfv", ADDRESS_OF(glGetTexGenfv));
    addProcedure("glGetTexGeniv", ADDRESS_OF(glGetTexGeniv));
    addProcedure("glGetTexImage", ADDRESS_OF(glGetTexImage));
    addProcedure("glGetTexLevelParameterfv", ADDRESS_OF(glGetTexLevelParameterfv));
    addProcedure("glGetTexLevelParameteriv", ADDRESS_OF(glGetTexLevelParameteriv));
    addProcedure("glGetTexParameterfv", ADDRESS_OF(glGetTexParameterfv));
    addProcedure("glGetTexParameteriv", ADDRESS_OF(glGetTexParameteriv));
    addProcedure("glHint", ADDRESS_OF(glHint));
    addProcedure("glIndexMask", ADDRESS_OF(glIndexMask));
    addProcedure("glIndexd", ADDRESS_OF(glIndexd));
    addProcedure("glIndexdv", ADDRESS_OF(glIndexdv));
    addProcedure("glIndexf", ADDRESS_OF(glIndexf));
    addProcedure("glIndexfv", ADDRESS_OF(glIndexfv));
    addProcedure("glIndexi", ADDRESS_OF(glIndexi));
    addProcedure("glIndexiv", ADDRESS_OF(glIndexiv));
    addProcedure("glIndexs", ADDRESS_OF(glIndexs));
    addProcedure("glIndexsv", ADDRESS_OF(glIndexsv));
    addProcedure("glInitNames", ADDRESS_OF(glInitNames));
    addProcedure("glIsEnabled", ADDRESS_OF(glIsEnabled));
    addProcedure("glIsList", ADDRESS_OF(glIsList));
    addProcedure("glLightModelf", ADDRESS_OF(glLightModelf));
    addProcedure("glLightModelfv", ADDRESS_OF(glLightModelfv));
    addProcedure("glLightModeli", ADDRESS_OF(glLightModeli));
    addProcedure("glLightModeliv", ADDRESS_OF(glLightModeliv));
    addProcedure("glLightf", ADDRESS_OF(glLightf));
    addProcedure("glLightfv", ADDRESS_OF(glLightfv));
    addProcedure("glLighti", ADDRESS_OF(glLighti));
    addProcedure("glLightiv", ADDRESS_OF(glLightiv));
    addProcedure("glLineStipple", ADDRESS_OF(glLineStipple));
    addProcedure("glLineWidth", ADDRESS_OF(glLineWidth));
    addProcedure("glListBase", ADDRESS_OF(glListBase));
    addProcedure("glLoadIdentity", ADDRESS_OF(glLoadIdentity));
    addProcedure("glLoadMatrixd", ADDRESS_OF(glLoadMatrixd));
    addProcedure("glLoadMatrixf", ADDRESS_OF(glLoadMatrixf));
    addProcedure("glLoadName", ADDRESS_OF(glLoadName));
    addProcedure("glLogicOp", ADDRESS_OF(glLogicOp));
    addProcedure("glMap1d", ADDRESS_OF(glMap1d));
    addProcedure("glMap1f", ADDRESS_OF(glMap1f));
    addProcedure("glMap2d", ADDRESS_OF(glMap2d));
    addProcedure("glMap2f", ADDRESS_OF(glMap2f));
    addProcedure("glMapGrid1d", ADDRESS_OF(glMapGrid1d));
    addProcedure("glMapGrid1f", ADDRESS_OF(glMapGrid1f));
    addProcedure("glMapGrid2d", ADDRESS_OF(glMapGrid2d));
    addProcedure("glMapGrid2f", ADDRESS_OF(glMapGrid2f));
    addProcedure("glMaterialf", ADDRESS_OF(glMaterialf));
    addProcedure("glMaterialfv", ADDRESS_OF(glMaterialfv));
    addProcedure("glMateriali", ADDRESS_OF(glMateriali));
    addProcedure("glMaterialiv", ADDRESS_OF(glMaterialiv));
    addProcedure("glMatrixMode", ADDRESS_OF(glMatrixMode));
    addProcedure("glMultMatrixd", ADDRESS_OF(glMultMatrixd));
    addProcedure("glMultMatrixf", ADDRESS_OF(glMultMatrixf));
    addProcedure("glNewList", ADDRESS_OF(glNewList));
    addProcedure("glNormal3b", ADDRESS_OF(glNormal3b));
    addProcedure("glNormal3bv", ADDRESS_OF(glNormal3bv));
    addProcedure("glNormal3d", ADDRESS_OF(glNormal3d));
    addProcedure("glNormal3dv", ADDRESS_OF(glNormal3dv));
    addProcedure("glNormal3f", ADDRESS_OF(glNormal3f));
    addProcedure("glNormal3fv", ADDRESS_OF(glNormal3fv));
    addProcedure("glNormal3i", ADDRESS_OF(glNormal3i));
    addProcedure("glNormal3iv", ADDRESS_OF(glNormal3iv));
    addProcedure("glNormal3s", ADDRESS_OF(glNormal3s));
    addProcedure("glNormal3sv", ADDRESS_OF(glNormal3sv));
    addProcedure("glOrtho", ADDRESS_OF(glOrtho));
    addProcedure("glPassThrough", ADDRESS_OF(glPassThrough));
    addProcedure("glPixelMapfv", ADDRESS_OF(glPixelMapfv));
    addProcedure("glPixelMapuiv", ADDRESS_OF(glPixelMapuiv));
    addProcedure("glPixelMapusv", ADDRESS_OF(glPixelMapusv));
    addProcedure("glPixelStoref", ADDRESS_OF(glPixelStoref));
    addProcedure("glPixelStorei", ADDRESS_OF(glPixelStorei));
    addProcedure("glPixelTransferf", ADDRESS_OF(glPixelTransferf));
    addProcedure("glPixelTransferi", ADDRESS_OF(glPixelTransferi));
    addProcedure("glPixelZoom", ADDRESS_OF(glPixelZoom));
    addProcedure("glPointSize", ADDRESS_OF(glPointSize));
    addProcedure("glPolygonMode", ADDRESS_OF(glPolygonMode));
    addProcedure("glPolygonStipple", ADDRESS_OF(glPolygonStipple));
    addProcedure("glPopAttrib", ADDRESS_OF(glPopAttrib));
    addProcedure("glPopMatrix", ADDRESS_OF(glPopMatrix));
    addProcedure("glPopName", ADDRESS_OF(glPopName));
    addProcedure("glPushAttrib", ADDRESS_OF(glPushAttrib));
    addProcedure("glPushMatrix", ADDRESS_OF(glPushMatrix));
    addProcedure("glPushName", ADDRESS_OF(glPushName));
    addProcedure("glRasterPos2d", ADDRESS_OF(glRasterPos2d));
    addProcedure("glRasterPos2dv", ADDRESS_OF(glRasterPos2dv));
    addProcedure("glRasterPos2f", ADDRESS_OF(glRasterPos2f));
    addProcedure("glRasterPos2fv", ADDRESS_OF(glRasterPos2fv));
    addProcedure("glRasterPos2i", ADDRESS_OF(glRasterPos2i));
    addProcedure("glRasterPos2iv", ADDRESS_OF(glRasterPos2iv));
    addProcedure("glRasterPos2s", ADDRESS_OF(glRasterPos2s));
    addProcedure("glRasterPos2sv", ADDRESS_OF(glRasterPos2sv));
    addProcedure("glRasterPos3d", ADDRESS_OF(glRasterPos3d));
    addProcedure("glRasterPos3dv", ADDRESS_OF(glRasterPos3dv));
    addProcedure("glRasterPos3f", ADDRESS_OF(glRasterPos3f));
    addProcedure("glRasterPos3fv", ADDRESS_OF(glRasterPos3fv));
    addProcedure("glRasterPos3i", ADDRESS_OF(glRasterPos3i));
    addProcedure("glRasterPos3iv", ADDRESS_OF(glRasterPos3iv));
    addProcedure("glRasterPos3s", ADDRESS_OF(glRasterPos3s));
    addProcedure("glRasterPos3sv", ADDRESS_OF(glRasterPos3sv));
    addProcedure("glRasterPos4d", ADDRESS_OF(glRasterPos4d));
    addProcedure("glRasterPos4dv", ADDRESS_OF(glRasterPos4dv));
    addProcedure("glRasterPos4f", ADDRESS_OF(glRasterPos4f));
    addProcedure("glRasterPos4fv", ADDRESS_OF(glRasterPos4fv));
    addProcedure("glRasterPos4i", ADDRESS_OF(glRasterPos4i));
    addProcedure("glRasterPos4iv", ADDRESS_OF(glRasterPos4iv));
    addProcedure("glRasterPos4s", ADDRESS_OF(glRasterPos4s));
    addProcedure("glRasterPos4sv", ADDRESS_OF(glRasterPos4sv));
    addProcedure("glReadBuffer", ADDRESS_OF(glReadBuffer));
    addProcedure("glReadPixels", ADDRESS_OF(glReadPixels));
    addProcedure("glRectd", ADDRESS_OF(glRectd));
    addProcedure("glRectdv", ADDRESS_OF(glRectdv));
    addProcedure("glRectf", ADDRESS_OF(glRectf));
    addProcedure("glRectfv", ADDRESS_OF(glRectfv));
    addProcedure("glRecti", ADDRESS_OF(glRecti));
    addProcedure("glRectiv", ADDRESS_OF(glRectiv));
    addProcedure("glRects", ADDRESS_OF(glRects));
    addProcedure("glRectsv", ADDRESS_OF(glRectsv));
    addProcedure("glRenderMode", ADDRESS_OF(glRenderMode));
    addProcedure("glRotated", ADDRESS_OF(glRotated));
    addProcedure("glRotatef", ADDRESS_OF(glRotatef));
    addProcedure("glScaled", ADDRESS_OF(glScaled));
    addProcedure("glScalef", ADDRESS_OF(glScalef));
    addProcedure("glScissor", ADDRESS_OF(glScissor));
    addProcedure("glSelectBuffer", ADDRESS_OF(glSelectBuffer));
    addProcedure("glShadeModel", ADDRESS_OF(glShadeModel));
    addProcedure("glStencilFunc", ADDRESS_OF(glStencilFunc));
    addProcedure("glStencilMask", ADDRESS_OF(glStencilMask));
    addProcedure("glStencilOp", ADDRESS_OF(glStencilOp));
    addProcedure("glTexCoord1d", ADDRESS_OF(glTexCoord1d));
    addProcedure("glTexCoord1dv", ADDRESS_OF(glTexCoord1dv));
    addProcedure("glTexCoord1f", ADDRESS_OF(glTexCoord1f));
    addProcedure("glTexCoord1fv", ADDRESS_OF(glTexCoord1fv));
    addProcedure("glTexCoord1i", ADDRESS_OF(glTexCoord1i));
    addProcedure("glTexCoord1iv", ADDRESS_OF(glTexCoord1iv));
    addProcedure("glTexCoord1s", ADDRESS_OF(glTexCoord1s));
    addProcedure("glTexCoord1sv", ADDRESS_OF(glTexCoord1sv));
    addProcedure("glTexCoord2d", ADDRESS_OF(glTexCoord2d));
    addProcedure("glTexCoord2dv", ADDRESS_OF(glTexCoord2dv));
    addProcedure("glTexCoord2f", ADDRESS_OF(glTexCoord2f));
    addProcedure("glTexCoord2fv", ADDRESS_OF(glTexCoord2fv));
    addProcedure("glTexCoord2i", ADDRESS_OF(glTexCoord2i));
    addProcedure("glTexCoord2iv", ADDRESS_OF(glTexCoord2iv));
    addProcedure("glTexCoord2s", ADDRESS_OF(glTexCoord2s));
    addProcedure("glTexCoord2sv", ADDRESS_OF(glTexCoord2sv));
    addProcedure("glTexCoord3d", ADDRESS_OF(glTexCoord3d));
    addProcedure("glTexCoord3dv", ADDRESS_OF(glTexCoord3dv));
    addProcedure("glTexCoord3f", ADDRESS_OF(glTexCoord3f));
    addProcedure("glTexCoord3fv", ADDRESS_OF(glTexCoord3fv));
    addProcedure("glTexCoord3i", ADDRESS_OF(glTexCoord3i));
    addProcedure("glTexCoord3iv", ADDRESS_OF(glTexCoord3iv));
    addProcedure("glTexCoord3s", ADDRESS_OF(glTexCoord3s));
    addProcedure("glTexCoord3sv", ADDRESS_OF(glTexCoord3sv));
    addProcedure("glTexCoord4d", ADDRESS_OF(glTexCoord4d));
    addProcedure("glTexCoord4dv", ADDRESS_OF(glTexCoord4dv));
    addProcedure("glTexCoord4f", ADDRESS_OF(glTexCoord4f));
    addProcedure("glTexCoord4fv", ADDRESS_OF(glTexCoord4fv));
    addProcedure("glTexCoord4i", ADDRESS_OF(glTexCoord4i));
    addProcedure("glTexCoord4iv", ADDRESS_OF(glTexCoord4iv));
    addProcedure("glTexCoord4s", ADDRESS_OF(glTexCoord4s));
    addProcedure("glTexCoord4sv", ADDRESS_OF(glTexCoord4sv));
    addProcedure("glTexEnvf", ADDRESS_OF(glTexEnvf));
    addProcedure("glTexEnvfv", ADDRESS_OF(glTexEnvfv));
    addProcedure("glTexEnvi", ADDRESS_OF(glTexEnvi));
    addProcedure("glTexEnviv", ADDRESS_OF(glTexEnviv));
    addProcedure("glTexGend", ADDRESS_OF(glTexGend));
    addProcedure("glTexGendv", ADDRESS_OF(glTexGendv));
    addProcedure("glTexGenf", ADDRESS_OF(glTexGenf));
    addProcedure("glTexGenfv", ADDRESS_OF(glTexGenfv));
    addProcedure("glTexGeni", ADDRESS_OF(glTexGeni));
    addProcedure("glTexGeniv", ADDRESS_OF(glTexGeniv));
    addProcedure("glTexImage1D", ADDRESS_OF(glTexImage1D));
    addProcedure("glTexImage2D", ADDRESS_OF(glTexImage2D));
    addProcedure("glTexParameterf", ADDRESS_OF(glTexParameterf));
    addProcedure("glTexParameterfv", ADDRESS_OF(glTexParameterfv));
    addProcedure("glTexParameteri", ADDRESS_OF(glTexParameteri));
    addProcedure("glTexParameteriv", ADDRESS_OF(glTexParameteriv));
    addProcedure("glTranslated", ADDRESS_OF(glTranslated));
    addProcedure("glTranslatef", ADDRESS_OF(glTranslatef));
    addProcedure("glVertex2d", ADDRESS_OF(glVertex2d));
    addProcedure("glVertex2dv", ADDRESS_OF(glVertex2dv));
    addProcedure("glVertex2f", ADDRESS_OF(glVertex2f));
    addProcedure("glVertex2fv", ADDRESS_OF(glVertex2fv));
    addProcedure("glVertex2i", ADDRESS_OF(glVertex2i));
    addProcedure("glVertex2iv", ADDRESS_OF(glVertex2iv));
    addProcedure("glVertex2s", ADDRESS_OF(glVertex2s));
    addProcedure("glVertex2sv", ADDRESS_OF(glVertex2sv));
    addProcedure("glVertex3d", ADDRESS_OF(glVertex3d));
    addProcedure("glVertex3dv", ADDRESS_OF(glVertex3dv));
    addProcedure("glVertex3f", ADDRESS_OF(glVertex3f));
    addProcedure("glVertex3fv", ADDRESS_OF(glVertex3fv));
    addProcedure("glVertex3i", ADDRESS_OF(glVertex3i));
    addProcedure("glVertex3iv", ADDRESS_OF(glVertex3iv));
    addProcedure("glVertex3s", ADDRESS_OF(glVertex3s));
    addProcedure("glVertex3sv", ADDRESS_OF(glVertex3sv));
    addProcedure("glVertex4d", ADDRESS_OF(glVertex4d));
    addProcedure("glVertex4dv", ADDRESS_OF(glVertex4dv));
    addProcedure("glVertex4f", ADDRESS_OF(glVertex4f));
    addProcedure("glVertex4fv", ADDRESS_OF(glVertex4fv));
    addProcedure("glVertex4i", ADDRESS_OF(glVertex4i));
    addProcedure("glVertex4iv", ADDRESS_OF(glVertex4iv));
    addProcedure("glVertex4s", ADDRESS_OF(glVertex4s));
    addProcedure("glVertex4sv", ADDRESS_OF(glVertex4sv));
    addProcedure("glViewport", ADDRESS_OF(glViewport));

    // Register Open GL 1.1 procedures
    addProcedure("glAreTexturesResident", ADDRESS_OF(glAreTexturesResident));
    addProcedure("glArrayElement", ADDRESS_OF(glArrayElement));
    addProcedure("glBindTexture", ADDRESS_OF(glBindTexture));
    addProcedure("glColorPointer", ADDRESS_OF(glColorPointer));
    addProcedure("glCopyTexImage1D", ADDRESS_OF(glCopyTexImage1D));
    addProcedure("glCopyTexImage2D", ADDRESS_OF(glCopyTexImage2D));
    addProcedure("glCopyTexSubImage1D", ADDRESS_OF(glCopyTexSubImage1D));
    addProcedure("glCopyTexSubImage2D", ADDRESS_OF(glCopyTexSubImage2D));
    addProcedure("glDeleteTextures", ADDRESS_OF(glDeleteTextures));
    addProcedure("glDisableClientState", ADDRESS_OF(glDisableClientState));
    addProcedure("glDrawArrays", ADDRESS_OF(glDrawArrays));
    addProcedure("glDrawElements", ADDRESS_OF(glDrawElements));
    addProcedure("glEdgeFlagPointer", ADDRESS_OF(glEdgeFlagPointer));
    addProcedure("glEnableClientState", ADDRESS_OF(glEnableClientState));
    addProcedure("glGenTextures", ADDRESS_OF(glGenTextures));
    addProcedure("glGetPointerv", ADDRESS_OF(glGetPointerv));
    addProcedure("glIsTexture", ADDRESS_OF(glIsTexture));
    addProcedure("glIndexPointer", ADDRESS_OF(glIndexPointer));
    addProcedure("glIndexub", ADDRESS_OF(glIndexub));
    addProcedure("glIndexubv", ADDRESS_OF(glIndexubv));
    addProcedure("glInterleavedArrays", ADDRESS_OF(glInterleavedArrays));
    addProcedure("glNormalPointer", ADDRESS_OF(glNormalPointer));
    addProcedure("glPolygonOffset", ADDRESS_OF(glPolygonOffset));
    addProcedure("glPopClientAttrib", ADDRESS_OF(glPopClientAttrib));
    addProcedure("glPrioritizeTextures", ADDRESS_OF(glPrioritizeTextures));
    addProcedure("glPushClientAttrib", ADDRESS_OF(glPushClientAttrib));
    addProcedure("glTexCoordPointer", ADDRESS_OF(glTexCoordPointer));
    addProcedure("glTexSubImage1D", ADDRESS_OF(glTexSubImage1D));
    addProcedure("glTexSubImage2D", ADDRESS_OF(glTexSubImage2D));
    addProcedure("glVertexPointer", ADDRESS_OF(glVertexPointer));

    // Register Open GL 1.2 procedures
    addProcedure("glDrawRangeElements", ADDRESS_OF(glDrawRangeElements));
    addProcedure("glTexImage3D", ADDRESS_OF(glTexImage3D));
    addProcedure("glTexSubImage3D", ADDRESS_OF(glTexSubImage3D));
    addProcedure("glCopyTexSubImage3D", ADDRESS_OF(glCopyTexSubImage3D));

    // Register Open GL 1.3 procedures
    addProcedure("glActiveTexture", ADDRESS_OF(glActiveTexture));
    addProcedure("glSampleCoverage", ADDRESS_OF(glSampleCoverage));
    addProcedure("glCompressedTexImage3D", ADDRESS_OF(glCompressedTexImage3D));
    addProcedure("glCompressedTexImage2D", ADDRESS_OF(glCompressedTexImage2D));
    addProcedure("glCompressedTexImage1D", ADDRESS_OF(glCompressedTexImage1D));
    addProcedure("glCompressedTexSubImage3D", ADDRESS_OF(glCompressedTexSubImage3D));
    addProcedure("glCompressedTexSubImage2D", ADDRESS_OF(glCompressedTexSubImage2D));
    addProcedure("glCompressedTexSubImage1D", ADDRESS_OF(glCompressedTexSubImage1D));
    addProcedure("glGetCompressedTexImage", ADDRESS_OF(glGetCompressedTexImage));
    addProcedure("glClientActiveTexture", ADDRESS_OF(glClientActiveTexture));
    addProcedure("glMultiTexCoord1d", ADDRESS_OF(glMultiTexCoord1d));
    addProcedure("glMultiTexCoord1dv", ADDRESS_OF(glMultiTexCoord1dv));
    addProcedure("glMultiTexCoord1f", ADDRESS_OF(glMultiTexCoord1f));
    addProcedure("glMultiTexCoord1fv", ADDRESS_OF(glMultiTexCoord1fv));
    addProcedure("glMultiTexCoord1i", ADDRESS_OF(glMultiTexCoord1i));
    addProcedure("glMultiTexCoord1iv", ADDRESS_OF(glMultiTexCoord1iv));
    addProcedure("glMultiTexCoord1s", ADDRESS_OF(glMultiTexCoord1s));
    addProcedure("glMultiTexCoord1sv", ADDRESS_OF(glMultiTexCoord1sv));
    addProcedure("glMultiTexCoord2d", ADDRESS_OF(glMultiTexCoord2d));
    addProcedure("glMultiTexCoord2dv", ADDRESS_OF(glMultiTexCoord2dv));
    addProcedure("glMultiTexCoord2f", ADDRESS_OF(glMultiTexCoord2f));
    addProcedure("glMultiTexCoord2fv", ADDRESS_OF(glMultiTexCoord2fv));
    addProcedure("glMultiTexCoord2i", ADDRESS_OF(glMultiTexCoord2i));
    addProcedure("glMultiTexCoord2iv", ADDRESS_OF(glMultiTexCoord2iv));
    addProcedure("glMultiTexCoord2s", ADDRESS_OF(glMultiTexCoord2s));
    addProcedure("glMultiTexCoord2sv", ADDRESS_OF(glMultiTexCoord2sv));
    addProcedure("glMultiTexCoord3d", ADDRESS_OF(glMultiTexCoord3d));
    addProcedure("glMultiTexCoord3dv", ADDRESS_OF(glMultiTexCoord3dv));
    addProcedure("glMultiTexCoord3f", ADDRESS_OF(glMultiTexCoord3f));
    addProcedure("glMultiTexCoord3fv", ADDRESS_OF(glMultiTexCoord3fv));
    addProcedure("glMultiTexCoord3i", ADDRESS_OF(glMultiTexCoord3i));
    addProcedure("glMultiTexCoord3iv", ADDRESS_OF(glMultiTexCoord3iv));
    addProcedure("glMultiTexCoord3s", ADDRESS_OF(glMultiTexCoord3s));
    addProcedure("glMultiTexCoord3sv", ADDRESS_OF(glMultiTexCoord3sv));
    addProcedure("glMultiTexCoord4d", ADDRESS_OF(glMultiTexCoord4d));
    addProcedure("glMultiTexCoord4dv", ADDRESS_OF(glMultiTexCoord4dv));
    addProcedure("glMultiTexCoord4f", ADDRESS_OF(glMultiTexCoord4f));
    addProcedure("glMultiTexCoord4fv", ADDRESS_OF(glMultiTexCoord4fv));
    addProcedure("glMultiTexCoord4i", ADDRESS_OF(glMultiTexCoord4i));
    addProcedure("glMultiTexCoord4iv", ADDRESS_OF(glMultiTexCoord4iv));
    addProcedure("glMultiTexCoord4s", ADDRESS_OF(glMultiTexCoord4s));
    addProcedure("glMultiTexCoord4sv", ADDRESS_OF(glMultiTexCoord4sv));
    addProcedure("glLoadTransposeMatrixf", ADDRESS_OF(glLoadTransposeMatrixf));
    addProcedure("glLoadTransposeMatrixd", ADDRESS_OF(glLoadTransposeMatrixd));
    addProcedure("glMultTransposeMatrixf", ADDRESS_OF(glMultTransposeMatrixf));
    addProcedure("glMultTransposeMatrixd", ADDRESS_OF(glMultTransposeMatrixd));

    // Register the implemented extensions
    addExtension("GL_ARB_texture_env_add");
    addExtension("GL_ARB_texture_env_combine");
    // addExtension("GL_ARB_texture_cube_map");
    addExtension("GL_ARB_texture_env_dot3");
    addExtension("GL_ARB_multitexture");
    {

        addProcedure("glMultiTexCoord1dARB", ADDRESS_OF(glMultiTexCoord1d));
        addProcedure("glMultiTexCoord1dvARB", ADDRESS_OF(glMultiTexCoord1dv));
        addProcedure("glMultiTexCoord1fARB", ADDRESS_OF(glMultiTexCoord1f));
        addProcedure("glMultiTexCoord1fvARB", ADDRESS_OF(glMultiTexCoord1fv));
        addProcedure("glMultiTexCoord1iARB", ADDRESS_OF(glMultiTexCoord1i));
        addProcedure("glMultiTexCoord1ivARB", ADDRESS_OF(glMultiTexCoord1iv));
        addProcedure("glMultiTexCoord1sARB", ADDRESS_OF(glMultiTexCoord1s));
        addProcedure("glMultiTexCoord1svARB", ADDRESS_OF(glMultiTexCoord1sv));
        addProcedure("glMultiTexCoord2dARB", ADDRESS_OF(glMultiTexCoord2d));
        addProcedure("glMultiTexCoord2dvARB", ADDRESS_OF(glMultiTexCoord2dv));
        addProcedure("glMultiTexCoord2fARB", ADDRESS_OF(glMultiTexCoord2f));
        addProcedure("glMultiTexCoord2fvARB", ADDRESS_OF(glMultiTexCoord2fv));
        addProcedure("glMultiTexCoord2iARB", ADDRESS_OF(glMultiTexCoord2i));
        addProcedure("glMultiTexCoord2ivARB", ADDRESS_OF(glMultiTexCoord2iv));
        addProcedure("glMultiTexCoord2sARB", ADDRESS_OF(glMultiTexCoord2s));
        addProcedure("glMultiTexCoord2svARB", ADDRESS_OF(glMultiTexCoord2sv));
        addProcedure("glMultiTexCoord3dARB", ADDRESS_OF(glMultiTexCoord3d));
        addProcedure("glMultiTexCoord3dvARB", ADDRESS_OF(glMultiTexCoord3dv));
        addProcedure("glMultiTexCoord3fARB", ADDRESS_OF(glMultiTexCoord3f));
        addProcedure("glMultiTexCoord3fvARB", ADDRESS_OF(glMultiTexCoord3fv));
        addProcedure("glMultiTexCoord3iARB", ADDRESS_OF(glMultiTexCoord3i));
        addProcedure("glMultiTexCoord3ivARB", ADDRESS_OF(glMultiTexCoord3iv));
        addProcedure("glMultiTexCoord3sARB", ADDRESS_OF(glMultiTexCoord3s));
        addProcedure("glMultiTexCoord3svARB", ADDRESS_OF(glMultiTexCoord3sv));
        addProcedure("glMultiTexCoord4dARB", ADDRESS_OF(glMultiTexCoord4d));
        addProcedure("glMultiTexCoord4dvARB", ADDRESS_OF(glMultiTexCoord4dv));
        addProcedure("glMultiTexCoord4fARB", ADDRESS_OF(glMultiTexCoord4f));
        addProcedure("glMultiTexCoord4fvARB", ADDRESS_OF(glMultiTexCoord4fv));
        addProcedure("glMultiTexCoord4iARB", ADDRESS_OF(glMultiTexCoord4i));
        addProcedure("glMultiTexCoord4ivARB", ADDRESS_OF(glMultiTexCoord4iv));
        addProcedure("glMultiTexCoord4sARB", ADDRESS_OF(glMultiTexCoord4s));
        addProcedure("glMultiTexCoord4svARB", ADDRESS_OF(glMultiTexCoord4sv));
        addProcedure("glActiveTextureARB", ADDRESS_OF(glActiveTexture));
        addProcedure("glClientActiveTextureARB", ADDRESS_OF(glClientActiveTexture));
    }

    addProcedure("glLockArrays", ADDRESS_OF(glLockArrays));
    addProcedure("glUnlockArrays", ADDRESS_OF(glUnlockArrays));
    addProcedure("glActiveStencilFaceEXT", ADDRESS_OF(glActiveStencilFaceEXT));
    addProcedure("glBlendEquation", ADDRESS_OF(glBlendEquation));
    addProcedure("glBlendFuncSeparate", ADDRESS_OF(glBlendFuncSeparate));
    // addExtension("GL_EXT_compiled_vertex_array");
    // {

    //     addProcedure("glLockArraysEXT", ADDRESS_OF(glLockArrays));
    //     addProcedure("glUnlockArraysEXT", ADDRESS_OF(glUnlockArrays));
    // }
    // addExtension("WGL_3DFX_gamma_control");
    // {

    //     addProcedure("wglGetDeviceGammaRamp3DFX", ADDRESS_OF(wglGetDeviceGammaRamp));
    //     addProcedure("wglSetDeviceGammaRamp3DFX", ADDRESS_OF(wglSetDeviceGammaRamp));
    // }
    // addExtension("WGL_ARB_extensions_string");
    // {

    //     addProcedure("wglGetExtensionsStringARB", ADDRESS_OF(wglGetExtensionsString));
    // }
}

void IceGL::render()
{
    SPDLOG_INFO("Render called");
    m_renderer.render();
}

const char *IceGL::getLibExtensions() const
{
    return m_glExtensions.c_str();
}

const void *IceGL::getLibProcedure(std::string name) const
{
    auto proc = m_glProcedures.find(name);
    if (proc != m_glProcedures.end())
    {

        return proc->second;
    }

    SPDLOG_WARN("Procedure {} isn't implemented", name.c_str());

    return nullptr;
}

void IceGL::addProcedure(std::string name, const void *address)
{
    m_glProcedures[name] = address;
}

void IceGL::addExtension(std::string extension)
{
    if (!m_glExtensions.empty())
    {

        m_glExtensions.append(" ");
    }
    m_glExtensions.append(extension);
}
} // namespace rr
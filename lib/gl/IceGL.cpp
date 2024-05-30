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

#include "IceGL.hpp"
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <spdlog/spdlog.h>
#include "RenderConfigs.hpp"
#include "renderer/Renderer.hpp"

#define ADDRESS_OF(X) reinterpret_cast<const void *>(&X)
namespace rr
{
IceGL* instance { nullptr };

IceGL& IceGL::getInstance()
{
    return *instance;
}

class RenderDevice
{
public:
    RenderDevice(IBusConnector& busConnector)
        : renderer { busConnector }
        , pixelPipeline { renderer }
        , vertexPipeline { pixelPipeline }
    {
    }

    Renderer<RenderConfig> renderer;
    VertexPipeline vertexPipeline;
    PixelPipeline pixelPipeline;
};

bool IceGL::createInstance(IBusConnector& busConnector)
{
    if (instance)
    {
        delete instance;
    }
    instance = new IceGL { busConnector };
    return instance != nullptr;
}

void IceGL::destroy()
{
    if (instance)
    {
        delete instance;
        instance = nullptr;
    }
}

IceGL::IceGL(IBusConnector& busConnector)
    : m_renderDevice { new RenderDevice { busConnector } }
{
    // Set initial values
    m_renderDevice->renderer.setTexEnvColor(0, { { 0, 0, 0, 0 } });
    m_renderDevice->renderer.setClearColor({ {0, 0, 0, 0 } });
    m_renderDevice->renderer.setClearDepth(65535);
    m_renderDevice->renderer.setFogColor({ { 255, 255, 255, 255 } });
    std::array<float, 33> fogLut {};
    std::fill(fogLut.begin(), fogLut.end(), 1.0f);
    m_renderDevice->renderer.setFogLut(fogLut, 0.0f, (std::numeric_limits<float>::max)()); // Windows defines macros with max ... parenthesis are a work around against build errors.

    // Register Open GL 1.0 procedures
    addLibProcedure("glAccum", ADDRESS_OF(glAccum));
    addLibProcedure("glAlphaFunc", ADDRESS_OF(glAlphaFunc));
    addLibProcedure("glBegin", ADDRESS_OF(glBegin));
    addLibProcedure("glBitmap", ADDRESS_OF(glBitmap));
    addLibProcedure("glBlendFunc", ADDRESS_OF(glBlendFunc));
    addLibProcedure("glCallList", ADDRESS_OF(glCallList));
    addLibProcedure("glCallLists", ADDRESS_OF(glCallLists));
    addLibProcedure("glClear", ADDRESS_OF(glClear));
    addLibProcedure("glClearAccum", ADDRESS_OF(glClearAccum));
    addLibProcedure("glClearColor", ADDRESS_OF(glClearColor));
    addLibProcedure("glClearDepth", ADDRESS_OF(glClearDepth));
    addLibProcedure("glClearIndex", ADDRESS_OF(glClearIndex));
    addLibProcedure("glClearStencil", ADDRESS_OF(glClearStencil));
    addLibProcedure("glClipPlane", ADDRESS_OF(glClipPlane));
    addLibProcedure("glColor3b", ADDRESS_OF(glColor3b));
    addLibProcedure("glColor3bv", ADDRESS_OF(glColor3bv));
    addLibProcedure("glColor3d", ADDRESS_OF(glColor3d));
    addLibProcedure("glColor3dv", ADDRESS_OF(glColor3dv));
    addLibProcedure("glColor3f", ADDRESS_OF(glColor3f));
    addLibProcedure("glColor3fv", ADDRESS_OF(glColor3fv));
    addLibProcedure("glColor3i", ADDRESS_OF(glColor3i));
    addLibProcedure("glColor3iv", ADDRESS_OF(glColor3iv));
    addLibProcedure("glColor3s", ADDRESS_OF(glColor3s));
    addLibProcedure("glColor3sv", ADDRESS_OF(glColor3sv));
    addLibProcedure("glColor3ub", ADDRESS_OF(glColor3ub));
    addLibProcedure("glColor3ubv", ADDRESS_OF(glColor3ubv));
    addLibProcedure("glColor3ui", ADDRESS_OF(glColor3ui));
    addLibProcedure("glColor3uiv", ADDRESS_OF(glColor3uiv));
    addLibProcedure("glColor3us", ADDRESS_OF(glColor3us));
    addLibProcedure("glColor3usv", ADDRESS_OF(glColor3usv));
    addLibProcedure("glColor4b", ADDRESS_OF(glColor4b));
    addLibProcedure("glColor4bv", ADDRESS_OF(glColor4bv));
    addLibProcedure("glColor4d", ADDRESS_OF(glColor4d));
    addLibProcedure("glColor4dv", ADDRESS_OF(glColor4dv));
    addLibProcedure("glColor4f", ADDRESS_OF(glColor4f));
    addLibProcedure("glColor4fv", ADDRESS_OF(glColor4fv));
    addLibProcedure("glColor4i", ADDRESS_OF(glColor4i));
    addLibProcedure("glColor4iv", ADDRESS_OF(glColor4iv));
    addLibProcedure("glColor4s", ADDRESS_OF(glColor4s));
    addLibProcedure("glColor4sv", ADDRESS_OF(glColor4sv));
    addLibProcedure("glColor4ub", ADDRESS_OF(glColor4ub));
    addLibProcedure("glColor4ubv", ADDRESS_OF(glColor4ubv));
    addLibProcedure("glColor4ui", ADDRESS_OF(glColor4ui));
    addLibProcedure("glColor4uiv", ADDRESS_OF(glColor4uiv));
    addLibProcedure("glColor4us", ADDRESS_OF(glColor4us));
    addLibProcedure("glColor4usv", ADDRESS_OF(glColor4usv));
    addLibProcedure("glColorMask", ADDRESS_OF(glColorMask));
    addLibProcedure("glColorMaterial", ADDRESS_OF(glColorMaterial));
    addLibProcedure("glCopyPixels", ADDRESS_OF(glCopyPixels));
    addLibProcedure("glCullFace", ADDRESS_OF(glCullFace));
    addLibProcedure("glDeleteLists", ADDRESS_OF(glDeleteLists));
    addLibProcedure("glDepthFunc", ADDRESS_OF(glDepthFunc));
    addLibProcedure("glDepthMask", ADDRESS_OF(glDepthMask));
    addLibProcedure("glDepthRange", ADDRESS_OF(glDepthRange));
    addLibProcedure("glDisable", ADDRESS_OF(glDisable));
    addLibProcedure("glDrawBuffer", ADDRESS_OF(glDrawBuffer));
    addLibProcedure("glDrawPixels", ADDRESS_OF(glDrawPixels));
    addLibProcedure("glEdgeFlag", ADDRESS_OF(glEdgeFlag));
    addLibProcedure("glEdgeFlagv", ADDRESS_OF(glEdgeFlagv));
    addLibProcedure("glEnable", ADDRESS_OF(glEnable));
    addLibProcedure("glEnd", ADDRESS_OF(glEnd));
    addLibProcedure("glEndList", ADDRESS_OF(glEndList));
    addLibProcedure("glEvalCoord1d", ADDRESS_OF(glEvalCoord1d));
    addLibProcedure("glEvalCoord1dv", ADDRESS_OF(glEvalCoord1dv));
    addLibProcedure("glEvalCoord1f", ADDRESS_OF(glEvalCoord1f));
    addLibProcedure("glEvalCoord1fv", ADDRESS_OF(glEvalCoord1fv));
    addLibProcedure("glEvalCoord2d", ADDRESS_OF(glEvalCoord2d));
    addLibProcedure("glEvalCoord2dv", ADDRESS_OF(glEvalCoord2dv));
    addLibProcedure("glEvalCoord2f", ADDRESS_OF(glEvalCoord2f));
    addLibProcedure("glEvalCoord2fv", ADDRESS_OF(glEvalCoord2fv));
    addLibProcedure("glEvalMesh1", ADDRESS_OF(glEvalMesh1));
    addLibProcedure("glEvalMesh2", ADDRESS_OF(glEvalMesh2));
    addLibProcedure("glEvalPoint1", ADDRESS_OF(glEvalPoint1));
    addLibProcedure("glEvalPoint2", ADDRESS_OF(glEvalPoint2));
    addLibProcedure("glFeedbackBuffer", ADDRESS_OF(glFeedbackBuffer));
    addLibProcedure("glFinish", ADDRESS_OF(glFinish));
    addLibProcedure("glFlush", ADDRESS_OF(glFlush));
    addLibProcedure("glFogf", ADDRESS_OF(glFogf));
    addLibProcedure("glFogfv", ADDRESS_OF(glFogfv));
    addLibProcedure("glFogi", ADDRESS_OF(glFogi));
    addLibProcedure("glFogiv", ADDRESS_OF(glFogiv));
    addLibProcedure("glFrontFace", ADDRESS_OF(glFrontFace));
    addLibProcedure("glFrustum", ADDRESS_OF(glFrustum));
    addLibProcedure("glGenLists", ADDRESS_OF(glGenLists));
    addLibProcedure("glGetBooleanv", ADDRESS_OF(glGetBooleanv));
    addLibProcedure("glGetClipPlane", ADDRESS_OF(glGetClipPlane));
    addLibProcedure("glGetDoublev", ADDRESS_OF(glGetDoublev));
    addLibProcedure("glGetError", ADDRESS_OF(glGetError));
    addLibProcedure("glGetFloatv", ADDRESS_OF(glGetFloatv));
    addLibProcedure("glGetIntegerv", ADDRESS_OF(glGetIntegerv));
    addLibProcedure("glGetLightfv", ADDRESS_OF(glGetLightfv));
    addLibProcedure("glGetLightiv", ADDRESS_OF(glGetLightiv));
    addLibProcedure("glGetMapdv", ADDRESS_OF(glGetMapdv));
    addLibProcedure("glGetMapfv", ADDRESS_OF(glGetMapfv));
    addLibProcedure("glGetMapiv", ADDRESS_OF(glGetMapiv));
    addLibProcedure("glGetMaterialfv", ADDRESS_OF(glGetMaterialfv));
    addLibProcedure("glGetMaterialiv", ADDRESS_OF(glGetMaterialiv));
    addLibProcedure("glGetPixelMapfv", ADDRESS_OF(glGetPixelMapfv));
    addLibProcedure("glGetPixelMapuiv", ADDRESS_OF(glGetPixelMapuiv));
    addLibProcedure("glGetPixelMapusv", ADDRESS_OF(glGetPixelMapusv));
    addLibProcedure("glGetPolygonStipple", ADDRESS_OF(glGetPolygonStipple));
    addLibProcedure("glGetString", ADDRESS_OF(glGetString));
    addLibProcedure("glGetTexEnvfv", ADDRESS_OF(glGetTexEnvfv));
    addLibProcedure("glGetTexEnviv", ADDRESS_OF(glGetTexEnviv));
    addLibProcedure("glGetTexGendv", ADDRESS_OF(glGetTexGendv));
    addLibProcedure("glGetTexGenfv", ADDRESS_OF(glGetTexGenfv));
    addLibProcedure("glGetTexGeniv", ADDRESS_OF(glGetTexGeniv));
    addLibProcedure("glGetTexImage", ADDRESS_OF(glGetTexImage));
    addLibProcedure("glGetTexLevelParameterfv", ADDRESS_OF(glGetTexLevelParameterfv));
    addLibProcedure("glGetTexLevelParameteriv", ADDRESS_OF(glGetTexLevelParameteriv));
    addLibProcedure("glGetTexParameterfv", ADDRESS_OF(glGetTexParameterfv));
    addLibProcedure("glGetTexParameteriv", ADDRESS_OF(glGetTexParameteriv));
    addLibProcedure("glHint", ADDRESS_OF(glHint));
    addLibProcedure("glIndexMask", ADDRESS_OF(glIndexMask));
    addLibProcedure("glIndexd", ADDRESS_OF(glIndexd));
    addLibProcedure("glIndexdv", ADDRESS_OF(glIndexdv));
    addLibProcedure("glIndexf", ADDRESS_OF(glIndexf));
    addLibProcedure("glIndexfv", ADDRESS_OF(glIndexfv));
    addLibProcedure("glIndexi", ADDRESS_OF(glIndexi));
    addLibProcedure("glIndexiv", ADDRESS_OF(glIndexiv));
    addLibProcedure("glIndexs", ADDRESS_OF(glIndexs));
    addLibProcedure("glIndexsv", ADDRESS_OF(glIndexsv));
    addLibProcedure("glInitNames", ADDRESS_OF(glInitNames));
    addLibProcedure("glIsEnabled", ADDRESS_OF(glIsEnabled));
    addLibProcedure("glIsList", ADDRESS_OF(glIsList));
    addLibProcedure("glLightModelf", ADDRESS_OF(glLightModelf));
    addLibProcedure("glLightModelfv", ADDRESS_OF(glLightModelfv));
    addLibProcedure("glLightModeli", ADDRESS_OF(glLightModeli));
    addLibProcedure("glLightModeliv", ADDRESS_OF(glLightModeliv));
    addLibProcedure("glLightf", ADDRESS_OF(glLightf));
    addLibProcedure("glLightfv", ADDRESS_OF(glLightfv));
    addLibProcedure("glLighti", ADDRESS_OF(glLighti));
    addLibProcedure("glLightiv", ADDRESS_OF(glLightiv));
    addLibProcedure("glLineStipple", ADDRESS_OF(glLineStipple));
    addLibProcedure("glLineWidth", ADDRESS_OF(glLineWidth));
    addLibProcedure("glListBase", ADDRESS_OF(glListBase));
    addLibProcedure("glLoadIdentity", ADDRESS_OF(glLoadIdentity));
    addLibProcedure("glLoadMatrixd", ADDRESS_OF(glLoadMatrixd));
    addLibProcedure("glLoadMatrixf", ADDRESS_OF(glLoadMatrixf));
    addLibProcedure("glLoadName", ADDRESS_OF(glLoadName));
    addLibProcedure("glLogicOp", ADDRESS_OF(glLogicOp));
    addLibProcedure("glMap1d", ADDRESS_OF(glMap1d));
    addLibProcedure("glMap1f", ADDRESS_OF(glMap1f));
    addLibProcedure("glMap2d", ADDRESS_OF(glMap2d));
    addLibProcedure("glMap2f", ADDRESS_OF(glMap2f));
    addLibProcedure("glMapGrid1d", ADDRESS_OF(glMapGrid1d));
    addLibProcedure("glMapGrid1f", ADDRESS_OF(glMapGrid1f));
    addLibProcedure("glMapGrid2d", ADDRESS_OF(glMapGrid2d));
    addLibProcedure("glMapGrid2f", ADDRESS_OF(glMapGrid2f));
    addLibProcedure("glMaterialf", ADDRESS_OF(glMaterialf));
    addLibProcedure("glMaterialfv", ADDRESS_OF(glMaterialfv));
    addLibProcedure("glMateriali", ADDRESS_OF(glMateriali));
    addLibProcedure("glMaterialiv", ADDRESS_OF(glMaterialiv));
    addLibProcedure("glMatrixMode", ADDRESS_OF(glMatrixMode));
    addLibProcedure("glMultMatrixd", ADDRESS_OF(glMultMatrixd));
    addLibProcedure("glMultMatrixf", ADDRESS_OF(glMultMatrixf));
    addLibProcedure("glNewList", ADDRESS_OF(glNewList));
    addLibProcedure("glNormal3b", ADDRESS_OF(glNormal3b));
    addLibProcedure("glNormal3bv", ADDRESS_OF(glNormal3bv));
    addLibProcedure("glNormal3d", ADDRESS_OF(glNormal3d));
    addLibProcedure("glNormal3dv", ADDRESS_OF(glNormal3dv));
    addLibProcedure("glNormal3f", ADDRESS_OF(glNormal3f));
    addLibProcedure("glNormal3fv", ADDRESS_OF(glNormal3fv));
    addLibProcedure("glNormal3i", ADDRESS_OF(glNormal3i));
    addLibProcedure("glNormal3iv", ADDRESS_OF(glNormal3iv));
    addLibProcedure("glNormal3s", ADDRESS_OF(glNormal3s));
    addLibProcedure("glNormal3sv", ADDRESS_OF(glNormal3sv));
    addLibProcedure("glOrtho", ADDRESS_OF(glOrtho));
    addLibProcedure("glPassThrough", ADDRESS_OF(glPassThrough));
    addLibProcedure("glPixelMapfv", ADDRESS_OF(glPixelMapfv));
    addLibProcedure("glPixelMapuiv", ADDRESS_OF(glPixelMapuiv));
    addLibProcedure("glPixelMapusv", ADDRESS_OF(glPixelMapusv));
    addLibProcedure("glPixelStoref", ADDRESS_OF(glPixelStoref));
    addLibProcedure("glPixelStorei", ADDRESS_OF(glPixelStorei));
    addLibProcedure("glPixelTransferf", ADDRESS_OF(glPixelTransferf));
    addLibProcedure("glPixelTransferi", ADDRESS_OF(glPixelTransferi));
    addLibProcedure("glPixelZoom", ADDRESS_OF(glPixelZoom));
    addLibProcedure("glPointSize", ADDRESS_OF(glPointSize));
    addLibProcedure("glPolygonMode", ADDRESS_OF(glPolygonMode));
    addLibProcedure("glPolygonStipple", ADDRESS_OF(glPolygonStipple));
    addLibProcedure("glPopAttrib", ADDRESS_OF(glPopAttrib));
    addLibProcedure("glPopMatrix", ADDRESS_OF(glPopMatrix));
    addLibProcedure("glPopName", ADDRESS_OF(glPopName));
    addLibProcedure("glPushAttrib", ADDRESS_OF(glPushAttrib));
    addLibProcedure("glPushMatrix", ADDRESS_OF(glPushMatrix));
    addLibProcedure("glPushName", ADDRESS_OF(glPushName));
    addLibProcedure("glRasterPos2d", ADDRESS_OF(glRasterPos2d));
    addLibProcedure("glRasterPos2dv", ADDRESS_OF(glRasterPos2dv));
    addLibProcedure("glRasterPos2f", ADDRESS_OF(glRasterPos2f));
    addLibProcedure("glRasterPos2fv", ADDRESS_OF(glRasterPos2fv));
    addLibProcedure("glRasterPos2i", ADDRESS_OF(glRasterPos2i));
    addLibProcedure("glRasterPos2iv", ADDRESS_OF(glRasterPos2iv));
    addLibProcedure("glRasterPos2s", ADDRESS_OF(glRasterPos2s));
    addLibProcedure("glRasterPos2sv", ADDRESS_OF(glRasterPos2sv));
    addLibProcedure("glRasterPos3d", ADDRESS_OF(glRasterPos3d));
    addLibProcedure("glRasterPos3dv", ADDRESS_OF(glRasterPos3dv));
    addLibProcedure("glRasterPos3f", ADDRESS_OF(glRasterPos3f));
    addLibProcedure("glRasterPos3fv", ADDRESS_OF(glRasterPos3fv));
    addLibProcedure("glRasterPos3i", ADDRESS_OF(glRasterPos3i));
    addLibProcedure("glRasterPos3iv", ADDRESS_OF(glRasterPos3iv));
    addLibProcedure("glRasterPos3s", ADDRESS_OF(glRasterPos3s));
    addLibProcedure("glRasterPos3sv", ADDRESS_OF(glRasterPos3sv));
    addLibProcedure("glRasterPos4d", ADDRESS_OF(glRasterPos4d));
    addLibProcedure("glRasterPos4dv", ADDRESS_OF(glRasterPos4dv));
    addLibProcedure("glRasterPos4f", ADDRESS_OF(glRasterPos4f));
    addLibProcedure("glRasterPos4fv", ADDRESS_OF(glRasterPos4fv));
    addLibProcedure("glRasterPos4i", ADDRESS_OF(glRasterPos4i));
    addLibProcedure("glRasterPos4iv", ADDRESS_OF(glRasterPos4iv));
    addLibProcedure("glRasterPos4s", ADDRESS_OF(glRasterPos4s));
    addLibProcedure("glRasterPos4sv", ADDRESS_OF(glRasterPos4sv));
    addLibProcedure("glReadBuffer", ADDRESS_OF(glReadBuffer));
    addLibProcedure("glReadPixels", ADDRESS_OF(glReadPixels));
    addLibProcedure("glRectd", ADDRESS_OF(glRectd));
    addLibProcedure("glRectdv", ADDRESS_OF(glRectdv));
    addLibProcedure("glRectf", ADDRESS_OF(glRectf));
    addLibProcedure("glRectfv", ADDRESS_OF(glRectfv));
    addLibProcedure("glRecti", ADDRESS_OF(glRecti));
    addLibProcedure("glRectiv", ADDRESS_OF(glRectiv));
    addLibProcedure("glRects", ADDRESS_OF(glRects));
    addLibProcedure("glRectsv", ADDRESS_OF(glRectsv));
    addLibProcedure("glRenderMode", ADDRESS_OF(glRenderMode));
    addLibProcedure("glRotated", ADDRESS_OF(glRotated));
    addLibProcedure("glRotatef", ADDRESS_OF(glRotatef));
    addLibProcedure("glScaled", ADDRESS_OF(glScaled));
    addLibProcedure("glScalef", ADDRESS_OF(glScalef));
    addLibProcedure("glScissor", ADDRESS_OF(glScissor));
    addLibProcedure("glSelectBuffer", ADDRESS_OF(glSelectBuffer));
    addLibProcedure("glShadeModel", ADDRESS_OF(glShadeModel));
    addLibProcedure("glStencilFunc", ADDRESS_OF(glStencilFunc));
    addLibProcedure("glStencilMask", ADDRESS_OF(glStencilMask));
    addLibProcedure("glStencilOp", ADDRESS_OF(glStencilOp));
    addLibProcedure("glTexCoord1d", ADDRESS_OF(glTexCoord1d));
    addLibProcedure("glTexCoord1dv", ADDRESS_OF(glTexCoord1dv));
    addLibProcedure("glTexCoord1f", ADDRESS_OF(glTexCoord1f));
    addLibProcedure("glTexCoord1fv", ADDRESS_OF(glTexCoord1fv));
    addLibProcedure("glTexCoord1i", ADDRESS_OF(glTexCoord1i));
    addLibProcedure("glTexCoord1iv", ADDRESS_OF(glTexCoord1iv));
    addLibProcedure("glTexCoord1s", ADDRESS_OF(glTexCoord1s));
    addLibProcedure("glTexCoord1sv", ADDRESS_OF(glTexCoord1sv));
    addLibProcedure("glTexCoord2d", ADDRESS_OF(glTexCoord2d));
    addLibProcedure("glTexCoord2dv", ADDRESS_OF(glTexCoord2dv));
    addLibProcedure("glTexCoord2f", ADDRESS_OF(glTexCoord2f));
    addLibProcedure("glTexCoord2fv", ADDRESS_OF(glTexCoord2fv));
    addLibProcedure("glTexCoord2i", ADDRESS_OF(glTexCoord2i));
    addLibProcedure("glTexCoord2iv", ADDRESS_OF(glTexCoord2iv));
    addLibProcedure("glTexCoord2s", ADDRESS_OF(glTexCoord2s));
    addLibProcedure("glTexCoord2sv", ADDRESS_OF(glTexCoord2sv));
    addLibProcedure("glTexCoord3d", ADDRESS_OF(glTexCoord3d));
    addLibProcedure("glTexCoord3dv", ADDRESS_OF(glTexCoord3dv));
    addLibProcedure("glTexCoord3f", ADDRESS_OF(glTexCoord3f));
    addLibProcedure("glTexCoord3fv", ADDRESS_OF(glTexCoord3fv));
    addLibProcedure("glTexCoord3i", ADDRESS_OF(glTexCoord3i));
    addLibProcedure("glTexCoord3iv", ADDRESS_OF(glTexCoord3iv));
    addLibProcedure("glTexCoord3s", ADDRESS_OF(glTexCoord3s));
    addLibProcedure("glTexCoord3sv", ADDRESS_OF(glTexCoord3sv));
    addLibProcedure("glTexCoord4d", ADDRESS_OF(glTexCoord4d));
    addLibProcedure("glTexCoord4dv", ADDRESS_OF(glTexCoord4dv));
    addLibProcedure("glTexCoord4f", ADDRESS_OF(glTexCoord4f));
    addLibProcedure("glTexCoord4fv", ADDRESS_OF(glTexCoord4fv));
    addLibProcedure("glTexCoord4i", ADDRESS_OF(glTexCoord4i));
    addLibProcedure("glTexCoord4iv", ADDRESS_OF(glTexCoord4iv));
    addLibProcedure("glTexCoord4s", ADDRESS_OF(glTexCoord4s));
    addLibProcedure("glTexCoord4sv", ADDRESS_OF(glTexCoord4sv));
    addLibProcedure("glTexEnvf", ADDRESS_OF(glTexEnvf));
    addLibProcedure("glTexEnvfv", ADDRESS_OF(glTexEnvfv));
    addLibProcedure("glTexEnvi", ADDRESS_OF(glTexEnvi));
    addLibProcedure("glTexEnviv", ADDRESS_OF(glTexEnviv));
    addLibProcedure("glTexGend", ADDRESS_OF(glTexGend));
    addLibProcedure("glTexGendv", ADDRESS_OF(glTexGendv));
    addLibProcedure("glTexGenf", ADDRESS_OF(glTexGenf));
    addLibProcedure("glTexGenfv", ADDRESS_OF(glTexGenfv));
    addLibProcedure("glTexGeni", ADDRESS_OF(glTexGeni));
    addLibProcedure("glTexGeniv", ADDRESS_OF(glTexGeniv));
    addLibProcedure("glTexImage1D", ADDRESS_OF(glTexImage1D));
    addLibProcedure("glTexImage2D", ADDRESS_OF(glTexImage2D));
    addLibProcedure("glTexParameterf", ADDRESS_OF(glTexParameterf));
    addLibProcedure("glTexParameterfv", ADDRESS_OF(glTexParameterfv));
    addLibProcedure("glTexParameteri", ADDRESS_OF(glTexParameteri));
    addLibProcedure("glTexParameteriv", ADDRESS_OF(glTexParameteriv));
    addLibProcedure("glTranslated", ADDRESS_OF(glTranslated));
    addLibProcedure("glTranslatef", ADDRESS_OF(glTranslatef));
    addLibProcedure("glVertex2d", ADDRESS_OF(glVertex2d));
    addLibProcedure("glVertex2dv", ADDRESS_OF(glVertex2dv));
    addLibProcedure("glVertex2f", ADDRESS_OF(glVertex2f));
    addLibProcedure("glVertex2fv", ADDRESS_OF(glVertex2fv));
    addLibProcedure("glVertex2i", ADDRESS_OF(glVertex2i));
    addLibProcedure("glVertex2iv", ADDRESS_OF(glVertex2iv));
    addLibProcedure("glVertex2s", ADDRESS_OF(glVertex2s));
    addLibProcedure("glVertex2sv", ADDRESS_OF(glVertex2sv));
    addLibProcedure("glVertex3d", ADDRESS_OF(glVertex3d));
    addLibProcedure("glVertex3dv", ADDRESS_OF(glVertex3dv));
    addLibProcedure("glVertex3f", ADDRESS_OF(glVertex3f));
    addLibProcedure("glVertex3fv", ADDRESS_OF(glVertex3fv));
    addLibProcedure("glVertex3i", ADDRESS_OF(glVertex3i));
    addLibProcedure("glVertex3iv", ADDRESS_OF(glVertex3iv));
    addLibProcedure("glVertex3s", ADDRESS_OF(glVertex3s));
    addLibProcedure("glVertex3sv", ADDRESS_OF(glVertex3sv));
    addLibProcedure("glVertex4d", ADDRESS_OF(glVertex4d));
    addLibProcedure("glVertex4dv", ADDRESS_OF(glVertex4dv));
    addLibProcedure("glVertex4f", ADDRESS_OF(glVertex4f));
    addLibProcedure("glVertex4fv", ADDRESS_OF(glVertex4fv));
    addLibProcedure("glVertex4i", ADDRESS_OF(glVertex4i));
    addLibProcedure("glVertex4iv", ADDRESS_OF(glVertex4iv));
    addLibProcedure("glVertex4s", ADDRESS_OF(glVertex4s));
    addLibProcedure("glVertex4sv", ADDRESS_OF(glVertex4sv));
    addLibProcedure("glViewport", ADDRESS_OF(glViewport));

    // Register Open GL 1.1 procedures
    addLibProcedure("glAreTexturesResident", ADDRESS_OF(glAreTexturesResident));
    addLibProcedure("glArrayElement", ADDRESS_OF(glArrayElement));
    addLibProcedure("glBindTexture", ADDRESS_OF(glBindTexture));
    addLibProcedure("glColorPointer", ADDRESS_OF(glColorPointer));
    addLibProcedure("glCopyTexImage1D", ADDRESS_OF(glCopyTexImage1D));
    addLibProcedure("glCopyTexImage2D", ADDRESS_OF(glCopyTexImage2D));
    addLibProcedure("glCopyTexSubImage1D", ADDRESS_OF(glCopyTexSubImage1D));
    addLibProcedure("glCopyTexSubImage2D", ADDRESS_OF(glCopyTexSubImage2D));
    addLibProcedure("glDeleteTextures", ADDRESS_OF(glDeleteTextures));
    addLibProcedure("glDisableClientState", ADDRESS_OF(glDisableClientState));
    addLibProcedure("glDrawArrays", ADDRESS_OF(glDrawArrays));
    addLibProcedure("glDrawElements", ADDRESS_OF(glDrawElements));
    addLibProcedure("glEdgeFlagPointer", ADDRESS_OF(glEdgeFlagPointer));
    addLibProcedure("glEnableClientState", ADDRESS_OF(glEnableClientState));
    addLibProcedure("glGenTextures", ADDRESS_OF(glGenTextures));
    addLibProcedure("glGetPointerv", ADDRESS_OF(glGetPointerv));
    addLibProcedure("glIsTexture", ADDRESS_OF(glIsTexture));
    addLibProcedure("glIndexPointer", ADDRESS_OF(glIndexPointer));
    addLibProcedure("glIndexub", ADDRESS_OF(glIndexub));
    addLibProcedure("glIndexubv", ADDRESS_OF(glIndexubv));
    addLibProcedure("glInterleavedArrays", ADDRESS_OF(glInterleavedArrays));
    addLibProcedure("glNormalPointer", ADDRESS_OF(glNormalPointer));
    addLibProcedure("glPolygonOffset", ADDRESS_OF(glPolygonOffset));
    addLibProcedure("glPopClientAttrib", ADDRESS_OF(glPopClientAttrib));
    addLibProcedure("glPrioritizeTextures", ADDRESS_OF(glPrioritizeTextures));
    addLibProcedure("glPushClientAttrib", ADDRESS_OF(glPushClientAttrib));
    addLibProcedure("glTexCoordPointer", ADDRESS_OF(glTexCoordPointer));
    addLibProcedure("glTexSubImage1D", ADDRESS_OF(glTexSubImage1D));
    addLibProcedure("glTexSubImage2D", ADDRESS_OF(glTexSubImage2D));
    addLibProcedure("glVertexPointer", ADDRESS_OF(glVertexPointer));

    // Register Open GL 1.2 procedures
    addLibProcedure("glDrawRangeElements", ADDRESS_OF(glDrawRangeElements));
    addLibProcedure("glTexImage3D", ADDRESS_OF(glTexImage3D));
    addLibProcedure("glTexSubImage3D", ADDRESS_OF(glTexSubImage3D));
    addLibProcedure("glCopyTexSubImage3D", ADDRESS_OF(glCopyTexSubImage3D));

    // Register Open GL 1.3 procedures
    addLibProcedure("glActiveTexture", ADDRESS_OF(glActiveTexture));
    addLibProcedure("glSampleCoverage", ADDRESS_OF(glSampleCoverage));
    addLibProcedure("glCompressedTexImage3D", ADDRESS_OF(glCompressedTexImage3D));
    addLibProcedure("glCompressedTexImage2D", ADDRESS_OF(glCompressedTexImage2D));
    addLibProcedure("glCompressedTexImage1D", ADDRESS_OF(glCompressedTexImage1D));
    addLibProcedure("glCompressedTexSubImage3D", ADDRESS_OF(glCompressedTexSubImage3D));
    addLibProcedure("glCompressedTexSubImage2D", ADDRESS_OF(glCompressedTexSubImage2D));
    addLibProcedure("glCompressedTexSubImage1D", ADDRESS_OF(glCompressedTexSubImage1D));
    addLibProcedure("glGetCompressedTexImage", ADDRESS_OF(glGetCompressedTexImage));
    addLibProcedure("glClientActiveTexture", ADDRESS_OF(glClientActiveTexture));
    addLibProcedure("glMultiTexCoord1d", ADDRESS_OF(glMultiTexCoord1d));
    addLibProcedure("glMultiTexCoord1dv", ADDRESS_OF(glMultiTexCoord1dv));
    addLibProcedure("glMultiTexCoord1f", ADDRESS_OF(glMultiTexCoord1f));
    addLibProcedure("glMultiTexCoord1fv", ADDRESS_OF(glMultiTexCoord1fv));
    addLibProcedure("glMultiTexCoord1i", ADDRESS_OF(glMultiTexCoord1i));
    addLibProcedure("glMultiTexCoord1iv", ADDRESS_OF(glMultiTexCoord1iv));
    addLibProcedure("glMultiTexCoord1s", ADDRESS_OF(glMultiTexCoord1s));
    addLibProcedure("glMultiTexCoord1sv", ADDRESS_OF(glMultiTexCoord1sv));
    addLibProcedure("glMultiTexCoord2d", ADDRESS_OF(glMultiTexCoord2d));
    addLibProcedure("glMultiTexCoord2dv", ADDRESS_OF(glMultiTexCoord2dv));
    addLibProcedure("glMultiTexCoord2f", ADDRESS_OF(glMultiTexCoord2f));
    addLibProcedure("glMultiTexCoord2fv", ADDRESS_OF(glMultiTexCoord2fv));
    addLibProcedure("glMultiTexCoord2i", ADDRESS_OF(glMultiTexCoord2i));
    addLibProcedure("glMultiTexCoord2iv", ADDRESS_OF(glMultiTexCoord2iv));
    addLibProcedure("glMultiTexCoord2s", ADDRESS_OF(glMultiTexCoord2s));
    addLibProcedure("glMultiTexCoord2sv", ADDRESS_OF(glMultiTexCoord2sv));
    addLibProcedure("glMultiTexCoord3d", ADDRESS_OF(glMultiTexCoord3d));
    addLibProcedure("glMultiTexCoord3dv", ADDRESS_OF(glMultiTexCoord3dv));
    addLibProcedure("glMultiTexCoord3f", ADDRESS_OF(glMultiTexCoord3f));
    addLibProcedure("glMultiTexCoord3fv", ADDRESS_OF(glMultiTexCoord3fv));
    addLibProcedure("glMultiTexCoord3i", ADDRESS_OF(glMultiTexCoord3i));
    addLibProcedure("glMultiTexCoord3iv", ADDRESS_OF(glMultiTexCoord3iv));
    addLibProcedure("glMultiTexCoord3s", ADDRESS_OF(glMultiTexCoord3s));
    addLibProcedure("glMultiTexCoord3sv", ADDRESS_OF(glMultiTexCoord3sv));
    addLibProcedure("glMultiTexCoord4d", ADDRESS_OF(glMultiTexCoord4d));
    addLibProcedure("glMultiTexCoord4dv", ADDRESS_OF(glMultiTexCoord4dv));
    addLibProcedure("glMultiTexCoord4f", ADDRESS_OF(glMultiTexCoord4f));
    addLibProcedure("glMultiTexCoord4fv", ADDRESS_OF(glMultiTexCoord4fv));
    addLibProcedure("glMultiTexCoord4i", ADDRESS_OF(glMultiTexCoord4i));
    addLibProcedure("glMultiTexCoord4iv", ADDRESS_OF(glMultiTexCoord4iv));
    addLibProcedure("glMultiTexCoord4s", ADDRESS_OF(glMultiTexCoord4s));
    addLibProcedure("glMultiTexCoord4sv", ADDRESS_OF(glMultiTexCoord4sv));
    addLibProcedure("glLoadTransposeMatrixf", ADDRESS_OF(glLoadTransposeMatrixf));
    addLibProcedure("glLoadTransposeMatrixd", ADDRESS_OF(glLoadTransposeMatrixd));
    addLibProcedure("glMultTransposeMatrixf", ADDRESS_OF(glMultTransposeMatrixf));
    addLibProcedure("glMultTransposeMatrixd", ADDRESS_OF(glMultTransposeMatrixd));

    // Register the implemented extensions
    addLibExtension("GL_ARB_texture_env_add");
    addLibExtension("GL_ARB_texture_env_combine");
    // addLibExtension("GL_ARB_texture_cube_map");
    addLibExtension("GL_ARB_texture_env_dot3");
    addLibExtension("GL_ARB_multitexture");
    {

        addLibProcedure("glMultiTexCoord1dARB", ADDRESS_OF(glMultiTexCoord1d));
        addLibProcedure("glMultiTexCoord1dvARB", ADDRESS_OF(glMultiTexCoord1dv));
        addLibProcedure("glMultiTexCoord1fARB", ADDRESS_OF(glMultiTexCoord1f));
        addLibProcedure("glMultiTexCoord1fvARB", ADDRESS_OF(glMultiTexCoord1fv));
        addLibProcedure("glMultiTexCoord1iARB", ADDRESS_OF(glMultiTexCoord1i));
        addLibProcedure("glMultiTexCoord1ivARB", ADDRESS_OF(glMultiTexCoord1iv));
        addLibProcedure("glMultiTexCoord1sARB", ADDRESS_OF(glMultiTexCoord1s));
        addLibProcedure("glMultiTexCoord1svARB", ADDRESS_OF(glMultiTexCoord1sv));
        addLibProcedure("glMultiTexCoord2dARB", ADDRESS_OF(glMultiTexCoord2d));
        addLibProcedure("glMultiTexCoord2dvARB", ADDRESS_OF(glMultiTexCoord2dv));
        addLibProcedure("glMultiTexCoord2fARB", ADDRESS_OF(glMultiTexCoord2f));
        addLibProcedure("glMultiTexCoord2fvARB", ADDRESS_OF(glMultiTexCoord2fv));
        addLibProcedure("glMultiTexCoord2iARB", ADDRESS_OF(glMultiTexCoord2i));
        addLibProcedure("glMultiTexCoord2ivARB", ADDRESS_OF(glMultiTexCoord2iv));
        addLibProcedure("glMultiTexCoord2sARB", ADDRESS_OF(glMultiTexCoord2s));
        addLibProcedure("glMultiTexCoord2svARB", ADDRESS_OF(glMultiTexCoord2sv));
        addLibProcedure("glMultiTexCoord3dARB", ADDRESS_OF(glMultiTexCoord3d));
        addLibProcedure("glMultiTexCoord3dvARB", ADDRESS_OF(glMultiTexCoord3dv));
        addLibProcedure("glMultiTexCoord3fARB", ADDRESS_OF(glMultiTexCoord3f));
        addLibProcedure("glMultiTexCoord3fvARB", ADDRESS_OF(glMultiTexCoord3fv));
        addLibProcedure("glMultiTexCoord3iARB", ADDRESS_OF(glMultiTexCoord3i));
        addLibProcedure("glMultiTexCoord3ivARB", ADDRESS_OF(glMultiTexCoord3iv));
        addLibProcedure("glMultiTexCoord3sARB", ADDRESS_OF(glMultiTexCoord3s));
        addLibProcedure("glMultiTexCoord3svARB", ADDRESS_OF(glMultiTexCoord3sv));
        addLibProcedure("glMultiTexCoord4dARB", ADDRESS_OF(glMultiTexCoord4d));
        addLibProcedure("glMultiTexCoord4dvARB", ADDRESS_OF(glMultiTexCoord4dv));
        addLibProcedure("glMultiTexCoord4fARB", ADDRESS_OF(glMultiTexCoord4f));
        addLibProcedure("glMultiTexCoord4fvARB", ADDRESS_OF(glMultiTexCoord4fv));
        addLibProcedure("glMultiTexCoord4iARB", ADDRESS_OF(glMultiTexCoord4i));
        addLibProcedure("glMultiTexCoord4ivARB", ADDRESS_OF(glMultiTexCoord4iv));
        addLibProcedure("glMultiTexCoord4sARB", ADDRESS_OF(glMultiTexCoord4s));
        addLibProcedure("glMultiTexCoord4svARB", ADDRESS_OF(glMultiTexCoord4sv));
        addLibProcedure("glActiveTextureARB", ADDRESS_OF(glActiveTexture));
        addLibProcedure("glClientActiveTextureARB", ADDRESS_OF(glClientActiveTexture));
    }

    addLibProcedure("glLockArrays", ADDRESS_OF(glLockArrays));
    addLibProcedure("glUnlockArrays", ADDRESS_OF(glUnlockArrays));
    addLibProcedure("glActiveStencilFaceEXT", ADDRESS_OF(glActiveStencilFaceEXT));
    addLibProcedure("glBlendEquation", ADDRESS_OF(glBlendEquation));
    addLibProcedure("glBlendFuncSeparate", ADDRESS_OF(glBlendFuncSeparate));
    // addLibExtension("GL_EXT_compiled_vertex_array");
    // {

    //     addLibProcedure("glLockArraysEXT", ADDRESS_OF(glLockArrays));
    //     addLibProcedure("glUnlockArraysEXT", ADDRESS_OF(glUnlockArrays));
    // }
    // addLibExtension("WGL_3DFX_gamma_control");
    // {

    //     addLibProcedure("wglGetDeviceGammaRamp3DFX", ADDRESS_OF(wglGetDeviceGammaRamp));
    //     addLibProcedure("wglSetDeviceGammaRamp3DFX", ADDRESS_OF(wglSetDeviceGammaRamp));
    // }
    // addLibExtension("WGL_ARB_extensions_string");
    // {

    //     addLibProcedure("wglGetExtensionsStringARB", ADDRESS_OF(wglGetExtensionsString));
    // }
}

IceGL::~IceGL()
{
    delete m_renderDevice;
}

void IceGL::swapDisplayList()
{
    SPDLOG_INFO("swapDisplayLIst called");
    m_renderDevice->renderer.swapDisplayList();
}

void IceGL::uploadDisplayList()
{
    SPDLOG_INFO("uploadDisplayList called");
    m_renderDevice->renderer.uploadDisplayList();
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

void IceGL::addLibProcedure(std::string name, const void *address)
{
    m_glProcedures[name] = address;
}

void IceGL::addLibExtension(std::string extension)
{
    if (!m_glExtensions.empty())
    {

        m_glExtensions.append(" ");
    }
    m_glExtensions.append(extension);
}

VertexPipeline& IceGL::vertexPipeline()
{
    return m_renderDevice->vertexPipeline;
}

PixelPipeline& IceGL::pixelPipeline()
{
    return m_renderDevice->pixelPipeline;
}

VertexQueue& IceGL::vertexQueue()
{
    return m_vertexQueue;
}

uint16_t IceGL::getMaxTextureSize() const
{
    return m_renderDevice->renderer.getMaxTextureSize();
}

std::size_t IceGL::getTmuCount() const
{
    return m_renderDevice->renderer.getTmuCount();
}

bool IceGL::isMipmappingAvailable() const
{
    return m_renderDevice->renderer.isMipmappingAvailable();
}

bool IceGL::setRenderResolution(const uint16_t x, const uint16_t y)
{
    return m_renderDevice->renderer.setRenderResolution(x, y);
}

} // namespace rr
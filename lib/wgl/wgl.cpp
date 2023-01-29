#include "wgl.h"
#include <spdlog/spdlog.h>
#include "IceGL.hpp"
#include "Renderer.hpp"
#include "FT60XBusConnector.hpp"

using namespace rr;

static const uint32_t RESOLUTION_H = 600;
static const uint32_t RESOLUTION_W = 1024;
FT60XBusConnector m_busConnector;
Renderer<16 * 1024 * 1024, 5, 256 * 1024, 128, 7680> m_renderer{m_busConnector};

// Wiggle API
// -------------------------------------------------------
GLAPI int APIENTRY impl_wglChoosePixelFormat(HDC hdc, CONST PIXELFORMATDESCRIPTOR *ppfd)
{
    SPDLOG_DEBUG("wglChoosePixelFormat Flags: {}, Stencil Bits: {}, Accum Bits: {}, Color Bits: {}, Alpha Bits: {}, Red Bits: {}, Green Bits: {}, Blue Bits: {}, Depth Bits: {} called", ppfd->dwFlags, ppfd->cStencilBits, ppfd->cAccumBits, ppfd->cColorBits, ppfd->cAlphaBits, ppfd->cRedBits, ppfd->cGreenBits, ppfd->cBlueBits, ppfd->cDepthBits);
    return 1;
}

GLAPI BOOL APIENTRY impl_wglCopyContext(HGLRC hglrc, HGLRC hglrc2, UINT i)
{
    SPDLOG_WARN("wglCopyContext not implemented");
    return false;
}

GLAPI HGLRC APIENTRY impl_wglCreateContext(HDC hdc)
{
    IceGL::createInstance(m_renderer);
    m_renderer.setRenderResolution(RESOLUTION_W, RESOLUTION_H);
    SPDLOG_DEBUG("wglCreateContext called");
    return reinterpret_cast<HGLRC>(&IceGL::getInstance());
}

GLAPI HGLRC APIENTRY impl_wglCreateLayerContext(HDC hdc, int iLayerPlane)
{
    SPDLOG_WARN("wglCreateLayerContext not implemented");
    return reinterpret_cast<HGLRC>(0);
}

GLAPI BOOL APIENTRY impl_wglDeleteContext(HGLRC hglrc)
{
    SPDLOG_DEBUG("wglDeleteContext called");
    return TRUE;
}

GLAPI BOOL APIENTRY impl_wglDescribeLayerPlane(HDC hdc, int iPixelFormat, int iLayerPlane, UINT nBytes, LPLAYERPLANEDESCRIPTOR plpd)
{
    SPDLOG_WARN("wglDescribeLayerPlane not implemented");
    return false;
}

GLAPI int APIENTRY impl_wglDescribePixelFormat(HDC hdc, int iPixelFormat, UINT nBytes, LPPIXELFORMATDESCRIPTOR ppfd)
{
    SPDLOG_DEBUG("wglDescribePixelFormat Pixelformat: {}, Bytes: {} called", iPixelFormat, nBytes);

    if (ppfd != nullptr) {

        memset(ppfd, 0, sizeof(PIXELFORMATDESCRIPTOR));

        ppfd->nSize = sizeof(PIXELFORMATDESCRIPTOR);
        ppfd->nVersion = 1;
        ppfd->dwFlags = PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL;
        ppfd->iPixelType = PFD_TYPE_RGBA;
        ppfd->cColorBits = 32;
        ppfd->cAlphaBits = 8; ppfd->cAlphaShift = 24;
        ppfd->cRedBits = 8; ppfd->cRedShift = 16;
        ppfd->cGreenBits = 8; ppfd->cGreenShift = 8;
        ppfd->cBlueBits = 8; ppfd->cBlueShift = 0;
        ppfd->cDepthBits = 24;
    }

    return 1;
}

GLAPI HGLRC APIENTRY impl_wglGetCurrentContext()
{
    SPDLOG_DEBUG("wglGetCurrentContext called");
    return reinterpret_cast<HGLRC>(1);
}

GLAPI HDC APIENTRY impl_wglGetCurrentDC()
{
    SPDLOG_WARN("wglGetCurrentDC not implemented");
    return reinterpret_cast<HDC>(0);
}

GLAPI int APIENTRY impl_wglGetLayerPaletteEntries(HDC hdc, int iLayerPlane, int iStart, int cEntries, COLORREF *pcr)
{
    SPDLOG_WARN("wglGetLayerPaletteEntries not implemented");
    return 0;
}

GLAPI int APIENTRY impl_wglGetPixelFormat(HDC hdc)
{
    SPDLOG_DEBUG("wglGetPixelFormat called");
    return 1;
}

GLAPI PROC APIENTRY impl_wglGetProcAddress(LPCSTR s)
{
    SPDLOG_INFO("wglGetProcAddress {} called", s);

    return reinterpret_cast<PROC>(IceGL::getInstance().getLibProcedure(s));
}

GLAPI BOOL APIENTRY impl_wglMakeCurrent(HDC hdc, HGLRC hglrc)
{
    SPDLOG_WARN("wglMakeCurrent not implemented");
    return TRUE;
}

GLAPI BOOL APIENTRY impl_wglRealizeLayerPalette(HDC hdc, int iLayerPlane, BOOL b)
{
    SPDLOG_WARN("wglRealizeLayerPalette not implemented");
    return FALSE;
}

GLAPI int APIENTRY impl_wglSetLayerPaletteEntries(HDC hdc, int iLayerPlane, int iStart, int cEntries, CONST COLORREF *pcr)
{
    SPDLOG_WARN("wglSetLayerPaletteEntries not implemented");
    return FALSE;
}

GLAPI BOOL APIENTRY impl_wglSetPixelFormat(HDC hdc, int format, CONST PIXELFORMATDESCRIPTOR *ppfd)
{
    SPDLOG_DEBUG("wglSetPixelFormat Format: {}", format);

    return format == 1;
}

GLAPI BOOL APIENTRY impl_wglShareLists(HGLRC hglrc, HGLRC hglrc2)
{
    SPDLOG_WARN("wglShareLists not implemented");
    return FALSE;
}

GLAPI BOOL APIENTRY impl_wglSwapBuffers(HDC hdc)
{
    SPDLOG_DEBUG("wglSwapBuffers called");
    IceGL::getInstance().commit();
    return TRUE;
}

GLAPI BOOL APIENTRY impl_wglSwapLayerBuffers(HDC hdc, UINT planes)
{
    SPDLOG_DEBUG("wglSwapLayerBuffers planes {} called", planes);

    if ((planes & WGL_SWAP_MAIN_PLANE) != 0U) {

        IceGL::getInstance().commit();
    }

    return TRUE;
}

GLAPI BOOL APIENTRY impl_wglUseFontBitmapsA(HDC hdc, DWORD dw1, DWORD dw2, DWORD dw3)
{
    SPDLOG_WARN("wglUseFontBitmapsA not implemented");
    return FALSE;
}

GLAPI BOOL APIENTRY impl_wglUseFontBitmapsW(HDC hdc, DWORD dw1, DWORD dw2, DWORD dw3)
{
    SPDLOG_WARN("wglUseFontBitmapsW not implemented");
    return FALSE;
}

GLAPI BOOL APIENTRY impl_wglUseFontOutlinesA(HDC hdc, DWORD dw1, DWORD dw2, DWORD dw3, FLOAT f1, FLOAT f2, int i, LPGLYPHMETRICSFLOAT pgmf)
{
    SPDLOG_WARN("wglUseFontOutlinesA not implemented");
    return FALSE;
}

GLAPI BOOL APIENTRY impl_wglUseFontOutlinesW(HDC hdc, DWORD dw1, DWORD dw2, DWORD dw3, FLOAT f1, FLOAT f2, int i, LPGLYPHMETRICSFLOAT pgmf)
{
    SPDLOG_WARN("wglUseFontOutlinesW not implemented");
    return FALSE;
}

// -------------------------------------------------------

// Some extensions
// -------------------------------------------------------
GLAPI BOOL APIENTRY impl_wglSetDeviceGammaRamp(HDC hdc, LPVOID lpRamp)
{
    SPDLOG_WARN("wglSetDeviceGammaRamp not implemented");
    return FALSE;
}

GLAPI BOOL APIENTRY impl_wglGetDeviceGammaRamp(HDC hdc, LPVOID lpRamp)
{
    SPDLOG_WARN("wglGetDeviceGammaRamp not implemented");
    return FALSE;
}

GLAPI const char * APIENTRY impl_wglGetExtensionsString(HDC hdc)
{
    SPDLOG_WARN("wglGetExtensionsString not implemented");
    return nullptr;
}

// -------------------------------------------------------
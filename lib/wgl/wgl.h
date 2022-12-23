#ifndef WGL_HPP_
#define WGL_HPP_

#ifdef __cplusplus
extern "C" {
#endif

#include <Windows.h>

#include "gl.h"

// Wiggle API
// -------------------------------------------------------
GLAPI int APIENTRY impl_wglChoosePixelFormat(HDC hdc, CONST PIXELFORMATDESCRIPTOR *ppfd);
GLAPI BOOL APIENTRY impl_wglCopyContext(HGLRC hglrc, HGLRC hglrc2, UINT i);
GLAPI HGLRC APIENTRY impl_wglCreateContext(HDC hdc);
GLAPI HGLRC APIENTRY impl_wglCreateLayerContext(HDC hdc, int iLayerPlane);
GLAPI BOOL APIENTRY impl_wglDeleteContext(HGLRC hglrc);
GLAPI BOOL APIENTRY impl_wglDescribeLayerPlane(HDC hdc, int iPixelFormat, int iLayerPlane, UINT nBytes, LPLAYERPLANEDESCRIPTOR plpd);
GLAPI int APIENTRY impl_wglDescribePixelFormat(HDC hdc, int iPixelFormat, UINT nBytes, LPPIXELFORMATDESCRIPTOR ppfd);
GLAPI HGLRC APIENTRY impl_wglGetCurrentContext();
GLAPI HDC APIENTRY impl_wglGetCurrentDC();
GLAPI int APIENTRY impl_wglGetLayerPaletteEntries(HDC hdc, int iLayerPlane, int iStart, int cEntries, COLORREF *pcr);
GLAPI int APIENTRY impl_wglGetPixelFormat(HDC hdc);
GLAPI PROC APIENTRY impl_wglGetProcAddress(LPCSTR s);
GLAPI BOOL APIENTRY impl_wglMakeCurrent(HDC hdc, HGLRC hglrc);
GLAPI BOOL APIENTRY impl_wglRealizeLayerPalette(HDC hdc, int iLayerPlane, BOOL b);
GLAPI int APIENTRY impl_wglSetLayerPaletteEntries(HDC hdc, int iLayerPlane, int iStart, int cEntries, CONST COLORREF *pcr);
GLAPI BOOL APIENTRY impl_wglSetPixelFormat(HDC hdc, int format, CONST PIXELFORMATDESCRIPTOR *ppfd);
GLAPI BOOL APIENTRY impl_wglShareLists(HGLRC hglrc, HGLRC hglrc2);
GLAPI BOOL APIENTRY impl_wglSwapBuffers(HDC hdc);
GLAPI BOOL APIENTRY impl_wglSwapLayerBuffers(HDC hdc, UINT planes);
GLAPI BOOL APIENTRY impl_wglUseFontBitmapsA(HDC hdc, DWORD dw1, DWORD dw2, DWORD dw3);
GLAPI BOOL APIENTRY impl_wglUseFontBitmapsW(HDC hdc, DWORD dw1, DWORD dw2, DWORD dw3);
GLAPI BOOL APIENTRY impl_wglUseFontOutlinesA(HDC hdc, DWORD dw1, DWORD dw2, DWORD dw3, FLOAT f1, FLOAT f2, int i, LPGLYPHMETRICSFLOAT pgmf);
GLAPI BOOL APIENTRY impl_wglUseFontOutlinesW(HDC hdc, DWORD dw1, DWORD dw2, DWORD dw3, FLOAT f1, FLOAT f2, int i, LPGLYPHMETRICSFLOAT pgmf);
// -------------------------------------------------------

// Some extensions
// -------------------------------------------------------
GLAPI BOOL APIENTRY impl_wglSetDeviceGammaRamp(HDC hdc, LPVOID lpRamp);
GLAPI BOOL APIENTRY impl_wglGetDeviceGammaRamp(HDC hdc, LPVOID lpRamp);
GLAPI const char * APIENTRY impl_wglGetExtensionsString(HDC hdc);
// -------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif // WGL_HPP_
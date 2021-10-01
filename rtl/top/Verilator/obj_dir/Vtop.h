// Verilated -*- C++ -*-
// DESCRIPTION: Verilator output: Primary design header
//
// This header should be included by all source files instantiating the design.
// The class here is then constructed to instantiate the design.
// See the Verilator manual for examples.

#ifndef _VTOP_H_
#define _VTOP_H_  // guard

#include "verilated.h"

//==========

class Vtop__Syms;

//----------

VL_MODULE(Vtop) {
  public:
    
    // PORTS
    // The application code writes and reads these signals to
    // propagate new values into/out from the Verilated model.
    VL_IN8(aclk,0,0);
    VL_IN8(resetn,0,0);
    VL_IN8(s_cmd_axis_tvalid,0,0);
    VL_OUT8(s_cmd_axis_tready,0,0);
    VL_IN8(s_cmd_axis_tlast,0,0);
    VL_OUT8(m_framebuffer_axis_tvalid,0,0);
    VL_IN8(m_framebuffer_axis_tready,0,0);
    VL_OUT8(m_framebuffer_axis_tlast,0,0);
    VL_IN(s_cmd_axis_tdata,31,0);
    VL_OUT64(m_framebuffer_axis_tdata,63,0);
    
    // LOCAL SIGNALS
    // Internals; generally not touched by application code
    // Anonymous structures to workaround compiler member-count bugs
    struct {
        CData/*0:0*/ top__DOT__m_cmd_axis_tvalid;
        CData/*0:0*/ top__DOT__m_cmd_axis_tready;
        CData/*5:0*/ top__DOT__parser__DOT__state;
        CData/*3:0*/ top__DOT__rasteriCEr__DOT__textureMode;
        CData/*0:0*/ top__DOT__rasteriCEr__DOT__colorWriteEnable;
        CData/*0:0*/ top__DOT__rasteriCEr__DOT__depthWriteEnable;
        CData/*0:0*/ top__DOT__rasteriCEr__DOT__s_rasterizer_axis_tvalid;
        CData/*0:0*/ top__DOT__rasteriCEr__DOT__s_rasterizer_axis_tready;
        CData/*0:0*/ top__DOT__rasteriCEr__DOT__s_rasterizer_axis_tlast;
        CData/*0:0*/ top__DOT__rasteriCEr__DOT__colorBufferApply;
        CData/*0:0*/ top__DOT__rasteriCEr__DOT__colorBufferApplied;
        CData/*0:0*/ top__DOT__rasteriCEr__DOT__colorBufferCmdCommit;
        CData/*0:0*/ top__DOT__rasteriCEr__DOT__colorBufferCmdMemset;
        CData/*0:0*/ top__DOT__rasteriCEr__DOT__depthBufferApply;
        CData/*0:0*/ top__DOT__rasteriCEr__DOT__depthBufferApplied;
        CData/*0:0*/ top__DOT__rasteriCEr__DOT__depthBufferCmdCommit;
        CData/*0:0*/ top__DOT__rasteriCEr__DOT__depthBufferCmdMemset;
        CData/*0:0*/ top__DOT__rasteriCEr__DOT__s_texture_axis_tvalid;
        CData/*0:0*/ top__DOT__rasteriCEr__DOT__s_texture_axis_tready;
        CData/*0:0*/ top__DOT__rasteriCEr__DOT__s_texture_axis_tlast;
        CData/*0:0*/ top__DOT__rasteriCEr__DOT__m_fragment_axis_tvalid;
        CData/*0:0*/ top__DOT__rasteriCEr__DOT__commandParser__DOT__apply;
        CData/*0:0*/ top__DOT__rasteriCEr__DOT__commandParser__DOT__applied;
        CData/*4:0*/ top__DOT__rasteriCEr__DOT__commandParser__DOT__state;
        CData/*1:0*/ top__DOT__rasteriCEr__DOT__commandParser__DOT__fbControlState;
        CData/*0:0*/ top__DOT__rasteriCEr__DOT__commandParser__DOT__wlsp;
        CData/*5:0*/ top__DOT__rasteriCEr__DOT__depthBuffer__DOT__stateTileControl;
        CData/*0:0*/ top__DOT__rasteriCEr__DOT__depthBuffer__DOT__commandRunning;
        CData/*0:0*/ top__DOT__rasteriCEr__DOT__depthBuffer__DOT__fbWr;
        CData/*0:0*/ top__DOT__rasteriCEr__DOT__depthBuffer__DOT__fbWrBus;
        CData/*5:0*/ top__DOT__rasteriCEr__DOT__colorBuffer__DOT__stateTileControl;
        CData/*0:0*/ top__DOT__rasteriCEr__DOT__colorBuffer__DOT__commandRunning;
        CData/*0:0*/ top__DOT__rasteriCEr__DOT__colorBuffer__DOT__fbWr;
        CData/*0:0*/ top__DOT__rasteriCEr__DOT__colorBuffer__DOT__fbWrBus;
        CData/*5:0*/ top__DOT__rasteriCEr__DOT__rop__DOT__paramIndex;
        CData/*0:0*/ top__DOT__rasteriCEr__DOT__rop__DOT__parameterComplete;
        CData/*5:0*/ top__DOT__rasteriCEr__DOT__rop__DOT__rasterizerState;
        CData/*0:0*/ top__DOT__rasteriCEr__DOT__rop__DOT__isInTriangle;
        CData/*0:0*/ top__DOT__rasteriCEr__DOT__rop__DOT__isInTriangleAndInBounds;
        CData/*5:0*/ top__DOT__rasteriCEr__DOT__rop__DOT__edgeWalkingState;
        CData/*0:0*/ top__DOT__rasteriCEr__DOT__rop__DOT__edgeWalkTryOtherside;
        CData/*0:0*/ top__DOT__rasteriCEr__DOT__rop__DOT__edgeWalkingDirection;
        CData/*0:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__depthTestPassed;
        CData/*0:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__depthTestLess;
        CData/*0:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__depthTestGreater;
        CData/*0:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__depthTestEqual;
        CData/*3:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__alphaTestFragmentVal;
        CData/*0:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__alphaTestPassed;
        CData/*0:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__alphaTestLess;
        CData/*0:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__alphaTestGreater;
        CData/*0:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__alphaTestEqual;
        CData/*0:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepWaitForRequestValid;
        CData/*0:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepCalculatePerspectiveCorrectionValid;
        CData/*0:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepWaitForMemoryValid;
        CData/*0:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepTexEnvValid;
        CData/*7:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepTexEnvV00;
        CData/*7:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepTexEnvV01;
        CData/*7:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepTexEnvV02;
        CData/*7:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepTexEnvV03;
        CData/*7:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepTexEnvV10;
        CData/*7:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepTexEnvV11;
        CData/*7:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepTexEnvV12;
        CData/*7:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepTexEnvV13;
        CData/*0:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepTexEnvResultValid;
    };
    struct {
        CData/*0:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepTexEnvResultWriteColor;
        CData/*0:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepBlendValid;
        CData/*0:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepBlendWriteColor;
        CData/*7:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepBlendV00;
        CData/*7:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepBlendV01;
        CData/*7:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepBlendV02;
        CData/*7:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepBlendV03;
        CData/*7:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepBlendV10;
        CData/*7:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepBlendV11;
        CData/*7:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepBlendV12;
        CData/*7:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepBlendV13;
        CData/*0:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepBlendResultValid;
        CData/*0:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepBlendResultWriteColor;
        CData/*0:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepBubbleValid;
        CData/*0:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepBubbleWriteColor;
        CData/*3:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__TexEnvCalc__DOT__rs;
        CData/*3:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__TexEnvCalc__DOT__gs;
        CData/*3:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__TexEnvCalc__DOT__bs;
        CData/*3:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__TexEnvCalc__DOT__as;
        CData/*3:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__TexEnvCalc__DOT__rp;
        CData/*3:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__TexEnvCalc__DOT__gp;
        CData/*3:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__TexEnvCalc__DOT__bp;
        CData/*3:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__TexEnvCalc__DOT__ap;
        CData/*3:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__TexEnvCalc__DOT__rc;
        CData/*3:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__TexEnvCalc__DOT__gc;
        CData/*3:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__TexEnvCalc__DOT__bc;
        CData/*3:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__TexEnvCalc__DOT__v00;
        CData/*3:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__TexEnvCalc__DOT__v01;
        CData/*3:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__TexEnvCalc__DOT__v02;
        CData/*3:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__TexEnvCalc__DOT__v03;
        CData/*3:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__TexEnvCalc__DOT__v10;
        CData/*3:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__TexEnvCalc__DOT__v11;
        CData/*3:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__TexEnvCalc__DOT__v12;
        CData/*3:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__TexEnvCalc__DOT__v13;
        CData/*3:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__TexEnvCalc__DOT__v20;
        CData/*3:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__TexEnvCalc__DOT__v21;
        CData/*3:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__TexEnvCalc__DOT__v22;
        CData/*3:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__TexEnvCalc__DOT__v23;
        CData/*3:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__TexEnvCalc__DOT__v30;
        CData/*3:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__TexEnvCalc__DOT__v31;
        CData/*3:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__TexEnvCalc__DOT__v32;
        CData/*3:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__TexEnvCalc__DOT__v33;
        CData/*3:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__BlendCalc__DOT__rs;
        CData/*3:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__BlendCalc__DOT__gs;
        CData/*3:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__BlendCalc__DOT__bs;
        CData/*3:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__BlendCalc__DOT__as;
        CData/*3:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__BlendCalc__DOT__rd;
        CData/*3:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__BlendCalc__DOT__gd;
        CData/*3:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__BlendCalc__DOT__bd;
        CData/*3:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__BlendCalc__DOT__ad;
        CData/*3:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__BlendCalc__DOT__v00;
        CData/*3:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__BlendCalc__DOT__v01;
        CData/*3:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__BlendCalc__DOT__v02;
        CData/*3:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__BlendCalc__DOT__v03;
        CData/*3:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__BlendCalc__DOT__v10;
        CData/*3:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__BlendCalc__DOT__v11;
        CData/*3:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__BlendCalc__DOT__v12;
        CData/*3:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__BlendCalc__DOT__v13;
        SData/*15:0*/ top__DOT__rasteriCEr__DOT__texel;
        SData/*15:0*/ top__DOT__rasteriCEr__DOT__colorOut;
        SData/*15:0*/ top__DOT__rasteriCEr__DOT__depthOut;
        SData/*13:0*/ top__DOT__rasteriCEr__DOT__commandParser__DOT__streamCounter;
        SData/*15:0*/ top__DOT__rasteriCEr__DOT__commandParser__DOT__Copy16__DOT__lsp;
        SData/*12:0*/ top__DOT__rasteriCEr__DOT__texCache__DOT__memWriteAddr;
    };
    struct {
        SData/*13:0*/ top__DOT__rasteriCEr__DOT__texCache__DOT__texelIndexConf;
        SData/*13:0*/ top__DOT__rasteriCEr__DOT__texCache__DOT__texelIndexConfDelay;
        SData/*15:0*/ top__DOT__rasteriCEr__DOT__depthBuffer__DOT__writeStrobe;
        SData/*14:0*/ top__DOT__rasteriCEr__DOT__depthBuffer__DOT__counter;
        SData/*14:0*/ top__DOT__rasteriCEr__DOT__depthBuffer__DOT__counterNext;
        SData/*14:0*/ top__DOT__rasteriCEr__DOT__depthBuffer__DOT__fbAddrBusWrite;
        SData/*15:0*/ top__DOT__rasteriCEr__DOT__depthBuffer__DOT__fbWrMaskBus;
        SData/*15:0*/ top__DOT__rasteriCEr__DOT__colorBuffer__DOT__writeStrobe;
        SData/*14:0*/ top__DOT__rasteriCEr__DOT__colorBuffer__DOT__counter;
        SData/*14:0*/ top__DOT__rasteriCEr__DOT__colorBuffer__DOT__counterNext;
        SData/*14:0*/ top__DOT__rasteriCEr__DOT__colorBuffer__DOT__fbAddrBusWrite;
        SData/*15:0*/ top__DOT__rasteriCEr__DOT__colorBuffer__DOT__fbWrMaskBus;
        SData/*9:0*/ top__DOT__rasteriCEr__DOT__rop__DOT__y;
        SData/*10:0*/ top__DOT__rasteriCEr__DOT__rop__DOT__x;
        SData/*15:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__textureS;
        SData/*15:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__textureT;
        SData/*15:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__depthTestFragmentVal;
        SData/*15:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__depthTestDepthBufferVal;
        SData/*15:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__textureSTmp;
        SData/*15:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__textureTTmp;
        SData/*15:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__recipIn;
        SData/*15:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__wValue;
        SData/*15:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepWaitForRequestTriangleColor;
        SData/*15:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepCalculatePerspectiveCorrectionDepthBufferVal;
        SData/*15:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepCalculatePerspectiveCorrectionTriangleColor;
        SData/*15:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepWaitForMemoryDepthValue;
        SData/*15:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepWaitForMemoryTriangleColor;
        SData/*15:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepTexEnvColorFrag;
        SData/*15:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepTexEnvDepthValue;
        SData/*15:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepTexEnvResultColorFrag;
        SData/*15:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepTexEnvResultColorTex;
        SData/*15:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepTexEnvResultDepthValue;
        SData/*15:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepBlendDepthValue;
        SData/*15:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepBlendResultColorFrag;
        SData/*15:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepBlendResultDepthValue;
        SData/*15:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepBubbleColorFrag;
        SData/*15:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepBubbleDepthValue;
        SData/*8:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__TexEnvResultCalc__DOT__r;
        SData/*8:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__TexEnvResultCalc__DOT__g;
        SData/*8:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__TexEnvResultCalc__DOT__b;
        SData/*8:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__TexEnvResultCalc__DOT__a;
        SData/*8:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__BlendResultCalc__DOT__r;
        SData/*8:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__BlendResultCalc__DOT__g;
        SData/*8:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__BlendResultCalc__DOT__b;
        SData/*8:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__BlendResultCalc__DOT__a;
        IData/*31:0*/ top__DOT__m_cmd_axis_tdata;
        IData/*27:0*/ top__DOT__parser__DOT__counter;
        IData/*31:0*/ top__DOT__rasteriCEr__DOT__texelIndex;
        IData/*16:0*/ top__DOT__rasteriCEr__DOT__colorIndexRead;
        IData/*16:0*/ top__DOT__rasteriCEr__DOT__colorIndexWrite;
        IData/*16:0*/ top__DOT__rasteriCEr__DOT__depthIndexRead;
        IData/*16:0*/ top__DOT__rasteriCEr__DOT__depthIndexWrite;
        IData/*31:0*/ top__DOT__rasteriCEr__DOT__s_rasterizer_axis_tdata;
        IData/*31:0*/ top__DOT__rasteriCEr__DOT__s_texture_axis_tdata;
        WData/*128:0*/ top__DOT__rasteriCEr__DOT__m_fragment_axis_tdata[5];
        IData/*31:0*/ top__DOT__rasteriCEr__DOT__texCache__DOT__texCache__DOT__memOut;
        IData/*16:0*/ top__DOT__rasteriCEr__DOT__depthBuffer__DOT__fragAddrReadDelay;
        IData/*16:0*/ top__DOT__rasteriCEr__DOT__colorBuffer__DOT__fragAddrReadDelay;
        IData/*16:0*/ top__DOT__rasteriCEr__DOT__rop__DOT__fbIndex;
        IData/*31:0*/ top__DOT__rasteriCEr__DOT__rop__DOT__RasterizerCopy16__DOT__tmp;
        IData/*16:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__fbIndex;
        IData/*16:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepCalculatePerspectiveCorrectionfbIndex;
        IData/*16:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepWaitForMemoryFbIndex;
        IData/*16:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepTexEnvFbIndex;
    };
    struct {
        IData/*16:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepTexEnvResultFbIndex;
        IData/*16:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepBlendFbIndex;
        IData/*16:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepBlendResultFbIndex;
        IData/*16:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepBubbleFbIndex;
        IData/*31:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__recip__DOT__j;
        IData/*31:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__recip__DOT__nextB;
        IData/*31:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__recip__DOT__currentB;
        IData/*31:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__recip__DOT__deltaB;
        QData/*63:0*/ top__DOT__rasteriCEr__DOT__depthBuffer__DOT__fbDataInBus;
        QData/*63:0*/ top__DOT__rasteriCEr__DOT__depthBuffer__DOT__ramTile__DOT__memOut;
        QData/*63:0*/ top__DOT__rasteriCEr__DOT__colorBuffer__DOT__fbDataInBus;
        QData/*63:0*/ top__DOT__rasteriCEr__DOT__colorBuffer__DOT__ramTile__DOT__memOut;
        SData/*15:0*/ top__DOT__rasteriCEr__DOT__commandParser__DOT__configReg[5];
        IData/*31:0*/ top__DOT__rasteriCEr__DOT__texCache__DOT__texCache__DOT__mem[8192];
        QData/*63:0*/ top__DOT__rasteriCEr__DOT__depthBuffer__DOT__ramTile__DOT__mem[32768];
        QData/*63:0*/ top__DOT__rasteriCEr__DOT__colorBuffer__DOT__ramTile__DOT__mem[32768];
        IData/*31:0*/ top__DOT__rasteriCEr__DOT__rop__DOT__paramMem[21];
        SData/*15:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__recip__DOT__lutB[2048];
        SData/*15:0*/ top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__recip__DOT__lutM[2048];
    };
    
    // LOCAL VARIABLES
    // Internals; generally not touched by application code
    CData/*0:0*/ __Vclklast__TOP__aclk;
    SData/*15:0*/ top__DOT__rasteriCEr__DOT__commandParser__DOT____Vlvbound1;
    SData/*15:0*/ __Vfunc_top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__truncate16__0__Vfuncout;
    IData/*31:0*/ top__DOT__rasteriCEr__DOT__rop__DOT____Vlvbound1;
    IData/*31:0*/ top__DOT__rasteriCEr__DOT__rop__DOT____Vlvbound2;
    IData/*31:0*/ __Vfunc_top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__truncate16__0__in;
    
    // INTERNAL VARIABLES
    // Internals; generally not touched by application code
    Vtop__Syms* __VlSymsp;  // Symbol table
    
    // CONSTRUCTORS
  private:
    VL_UNCOPYABLE(Vtop);  ///< Copying not allowed
  public:
    /// Construct the model; called by application code
    /// The special name  may be used to make a wrapper with a
    /// single model invisible with respect to DPI scope names.
    Vtop(const char* name = "TOP");
    /// Destroy the model; called (often implicitly) by application code
    ~Vtop();
    
    // API METHODS
    /// Evaluate the model.  Application must call when inputs change.
    void eval() { eval_step(); }
    /// Evaluate when calling multiple units/models per time step.
    void eval_step();
    /// Evaluate at end of a timestep for tracing, when using eval_step().
    /// Application must call after all eval() and before time changes.
    void eval_end_step() {}
    /// Simulation complete, run final blocks.  Application must call on completion.
    void final();
    
    // INTERNAL METHODS
  private:
    static void _eval_initial_loop(Vtop__Syms* __restrict vlSymsp);
  public:
    void __Vconfigure(Vtop__Syms* symsp, bool first);
  private:
    static QData _change_request(Vtop__Syms* __restrict vlSymsp);
    static QData _change_request_1(Vtop__Syms* __restrict vlSymsp);
    void _ctor_var_reset() VL_ATTR_COLD;
  public:
    static void _eval(Vtop__Syms* __restrict vlSymsp);
  private:
#ifdef VL_DEBUG
    void _eval_debug_assertions();
#endif  // VL_DEBUG
  public:
    static void _eval_initial(Vtop__Syms* __restrict vlSymsp) VL_ATTR_COLD;
    static void _eval_settle(Vtop__Syms* __restrict vlSymsp) VL_ATTR_COLD;
    static void _initial__TOP__2(Vtop__Syms* __restrict vlSymsp) VL_ATTR_COLD;
    static void _sequent__TOP__1(Vtop__Syms* __restrict vlSymsp);
    static void _settle__TOP__3(Vtop__Syms* __restrict vlSymsp) VL_ATTR_COLD;
} VL_ATTR_ALIGNED(VL_CACHE_LINE_BYTES);

//----------


#endif  // guard

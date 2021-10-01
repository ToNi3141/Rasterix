// Verilated -*- C++ -*-
// DESCRIPTION: Verilator output: Design implementation internals
// See Vtop.h for the primary calling header

#include "Vtop.h"
#include "Vtop__Syms.h"

//==========

VL_CTOR_IMP(Vtop) {
    Vtop__Syms* __restrict vlSymsp = __VlSymsp = new Vtop__Syms(this, name());
    Vtop* const __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    // Reset internal values
    
    // Reset structure values
    _ctor_var_reset();
}

void Vtop::__Vconfigure(Vtop__Syms* vlSymsp, bool first) {
    if (false && first) {}  // Prevent unused
    this->__VlSymsp = vlSymsp;
    if (false && this->__VlSymsp) {}  // Prevent unused
    Verilated::timeunit(-12);
    Verilated::timeprecision(-12);
}

Vtop::~Vtop() {
    VL_DO_CLEAR(delete __VlSymsp, __VlSymsp = nullptr);
}

void Vtop::_initial__TOP__2(Vtop__Syms* __restrict vlSymsp) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vtop::_initial__TOP__2\n"); );
    Vtop* const __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    // Body
    vlTOPp->top__DOT__rasteriCEr__DOT__texCache__DOT__memWriteAddr = 0U;
    vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepBubbleFbIndex = 0U;
    vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepBubbleColorFrag = 0U;
    vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepBubbleDepthValue = 0U;
    vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepBubbleWriteColor = 0U;
    vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepBlendResultFbIndex = 0U;
    vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepBlendResultColorFrag = 0U;
    vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepBlendResultDepthValue = 0U;
    vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepBlendResultWriteColor = 0U;
    vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepBlendFbIndex = 0U;
    vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepBlendDepthValue = 0U;
    vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepBlendWriteColor = 0U;
    vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepTexEnvResultFbIndex = 0U;
    vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepTexEnvResultColorFrag = 0U;
    vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepTexEnvResultColorTex = 0U;
    vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepTexEnvResultDepthValue = 0U;
    vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepTexEnvResultWriteColor = 0U;
    vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepTexEnvFbIndex = 0U;
    vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepTexEnvColorFrag = 0U;
    vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepTexEnvDepthValue = 0U;
    vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepWaitForMemoryFbIndex = 0U;
    vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepWaitForMemoryDepthValue = 0U;
    vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepWaitForMemoryTriangleColor = 0U;
    vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepCalculatePerspectiveCorrectionDepthBufferVal = 0U;
    vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepCalculatePerspectiveCorrectionfbIndex = 0U;
    vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepCalculatePerspectiveCorrectionTriangleColor = 0U;
    vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepWaitForRequestTriangleColor = 0U;
    vlTOPp->top__DOT__rasteriCEr__DOT__commandParser__DOT__wlsp = 0U;
    vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepBubbleValid = 0U;
    vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__recip__DOT__j = 0U;
    while (VL_GTS_III(1,32,32, 0x800U, vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__recip__DOT__j)) {
        if ((0U == vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__recip__DOT__j)) {
            vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__recip__DOT__currentB = 0xffffffffU;
            vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__recip__DOT__nextB 
                = (((IData)(0x10U) + VL_DIV_III(32, (IData)(0x7fffffU), 
                                                ((IData)(1U) 
                                                 + vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__recip__DOT__j))) 
                   >> 5U);
        } else {
            vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__recip__DOT__currentB 
                = (((IData)(0x10U) + VL_DIV_III(32, (IData)(0x7fffffU), vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__recip__DOT__j)) 
                   >> 5U);
            vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__recip__DOT__nextB 
                = (((IData)(0x10U) + VL_DIV_III(32, (IData)(0x7fffffU), 
                                                ((IData)(1U) 
                                                 + vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__recip__DOT__j))) 
                   >> 5U);
        }
        vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__recip__DOT__deltaB 
            = (vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__recip__DOT__currentB 
               - vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__recip__DOT__nextB);
        vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__recip__DOT__lutB[(0x7ffU 
                                                                                & vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__recip__DOT__j)] 
            = (0xffffU & vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__recip__DOT__currentB);
        vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__recip__DOT__lutM[(0x7ffU 
                                                                                & vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__recip__DOT__j)] 
            = (0xffffU & vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__recip__DOT__deltaB);
        vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__recip__DOT__j 
            = ((IData)(1U) + vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__recip__DOT__j);
    }
    vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepBlendResultValid = 0U;
    vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepBlendValid = 0U;
    vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepTexEnvResultValid = 0U;
    vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepTexEnvValid = 0U;
    vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepWaitForMemoryValid = 0U;
    vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepCalculatePerspectiveCorrectionValid = 0U;
    vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepWaitForRequestValid = 0U;
}

void Vtop::_settle__TOP__3(Vtop__Syms* __restrict vlSymsp) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vtop::_settle__TOP__3\n"); );
    Vtop* const __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    // Body
    vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__depthTestLess 
        = ((IData)(vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__depthTestFragmentVal) 
           < (IData)(vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__depthTestDepthBufferVal));
    vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__depthTestGreater 
        = ((IData)(vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__depthTestFragmentVal) 
           > (IData)(vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__depthTestDepthBufferVal));
    vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__depthTestEqual 
        = ((IData)(vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__depthTestFragmentVal) 
           == (IData)(vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__depthTestDepthBufferVal));
    vlTOPp->top__DOT__rasteriCEr__DOT__depthBuffer__DOT__counterNext 
        = (0x7fffU & ((IData)(1U) + (IData)(vlTOPp->top__DOT__rasteriCEr__DOT__depthBuffer__DOT__counter)));
    if (vlTOPp->top__DOT__rasteriCEr__DOT__depthBuffer__DOT__commandRunning) {
        vlTOPp->top__DOT__rasteriCEr__DOT__depthBuffer__DOT__fbAddrBusWrite 
            = (0x7fffU & (IData)(vlTOPp->top__DOT__rasteriCEr__DOT__depthBuffer__DOT__counter));
        vlTOPp->top__DOT__rasteriCEr__DOT__depthBuffer__DOT__fbWrBus 
            = vlTOPp->top__DOT__rasteriCEr__DOT__depthBuffer__DOT__fbWr;
    } else {
        vlTOPp->top__DOT__rasteriCEr__DOT__depthBuffer__DOT__fbAddrBusWrite 
            = (0x7fffU & (vlTOPp->top__DOT__rasteriCEr__DOT__depthIndexWrite 
                          >> 2U));
        vlTOPp->top__DOT__rasteriCEr__DOT__depthBuffer__DOT__fbWrBus 
            = vlTOPp->top__DOT__rasteriCEr__DOT__depthWriteEnable;
    }
    vlTOPp->top__DOT__rasteriCEr__DOT__texel = (0xffffU 
                                                & (vlTOPp->top__DOT__rasteriCEr__DOT__texCache__DOT__texCache__DOT__memOut 
                                                   >> 
                                                   (0x10U 
                                                    & ((IData)(vlTOPp->top__DOT__rasteriCEr__DOT__texCache__DOT__texelIndexConfDelay) 
                                                       << 4U))));
    vlTOPp->m_framebuffer_axis_tdata = vlTOPp->top__DOT__rasteriCEr__DOT__colorBuffer__DOT__ramTile__DOT__memOut;
    vlTOPp->top__DOT__rasteriCEr__DOT__colorBuffer__DOT__counterNext 
        = (0x7fffU & ((IData)(1U) + (IData)(vlTOPp->top__DOT__rasteriCEr__DOT__colorBuffer__DOT__counter)));
    if (vlTOPp->top__DOT__rasteriCEr__DOT__colorBuffer__DOT__commandRunning) {
        vlTOPp->top__DOT__rasteriCEr__DOT__colorBuffer__DOT__fbAddrBusWrite 
            = (0x7fffU & (IData)(vlTOPp->top__DOT__rasteriCEr__DOT__colorBuffer__DOT__counter));
        vlTOPp->top__DOT__rasteriCEr__DOT__colorBuffer__DOT__fbWrBus 
            = vlTOPp->top__DOT__rasteriCEr__DOT__colorBuffer__DOT__fbWr;
    } else {
        vlTOPp->top__DOT__rasteriCEr__DOT__colorBuffer__DOT__fbAddrBusWrite 
            = (0x7fffU & (vlTOPp->top__DOT__rasteriCEr__DOT__colorIndexWrite 
                          >> 2U));
        vlTOPp->top__DOT__rasteriCEr__DOT__colorBuffer__DOT__fbWrBus 
            = vlTOPp->top__DOT__rasteriCEr__DOT__colorWriteEnable;
    }
    vlTOPp->top__DOT__rasteriCEr__DOT__commandParser__DOT__applied 
        = ((IData)(vlTOPp->top__DOT__rasteriCEr__DOT__colorBufferApplied) 
           & (IData)(vlTOPp->top__DOT__rasteriCEr__DOT__depthBufferApplied));
    vlTOPp->top__DOT__rasteriCEr__DOT__texCache__DOT__texelIndexConf 
        = ((1U == (IData)(vlTOPp->top__DOT__rasteriCEr__DOT__textureMode))
            ? ((0x3e0U & (vlTOPp->top__DOT__rasteriCEr__DOT__texelIndex 
                          >> 0x15U)) | (0x1fU & (vlTOPp->top__DOT__rasteriCEr__DOT__texelIndex 
                                                 >> 0xaU)))
            : ((2U == (IData)(vlTOPp->top__DOT__rasteriCEr__DOT__textureMode))
                ? ((0xfc0U & (vlTOPp->top__DOT__rasteriCEr__DOT__texelIndex 
                              >> 0x13U)) | (0x3fU & 
                                            (vlTOPp->top__DOT__rasteriCEr__DOT__texelIndex 
                                             >> 9U)))
                : ((4U == (IData)(vlTOPp->top__DOT__rasteriCEr__DOT__textureMode))
                    ? ((0x3f80U & (vlTOPp->top__DOT__rasteriCEr__DOT__texelIndex 
                                   >> 0x11U)) | (0x7fU 
                                                 & (vlTOPp->top__DOT__rasteriCEr__DOT__texelIndex 
                                                    >> 8U)))
                    : 0U)));
    vlTOPp->top__DOT__rasteriCEr__DOT__depthBuffer__DOT__fbDataInBus 
        = ((IData)(vlTOPp->top__DOT__rasteriCEr__DOT__depthBuffer__DOT__commandRunning)
            ? (((QData)((IData)(vlTOPp->top__DOT__rasteriCEr__DOT__commandParser__DOT__configReg
                                [1U])) << 0x30U) | 
               (((QData)((IData)(vlTOPp->top__DOT__rasteriCEr__DOT__commandParser__DOT__configReg
                                 [1U])) << 0x20U) | 
                (((QData)((IData)(vlTOPp->top__DOT__rasteriCEr__DOT__commandParser__DOT__configReg
                                  [1U])) << 0x10U) 
                 | (QData)((IData)(vlTOPp->top__DOT__rasteriCEr__DOT__commandParser__DOT__configReg
                                   [1U]))))) : (((QData)((IData)(vlTOPp->top__DOT__rasteriCEr__DOT__depthOut)) 
                                                 << 0x30U) 
                                                | (((QData)((IData)(vlTOPp->top__DOT__rasteriCEr__DOT__depthOut)) 
                                                    << 0x20U) 
                                                   | (((QData)((IData)(vlTOPp->top__DOT__rasteriCEr__DOT__depthOut)) 
                                                       << 0x10U) 
                                                      | (QData)((IData)(vlTOPp->top__DOT__rasteriCEr__DOT__depthOut))))));
    vlTOPp->top__DOT__rasteriCEr__DOT__colorBuffer__DOT__fbDataInBus 
        = ((IData)(vlTOPp->top__DOT__rasteriCEr__DOT__colorBuffer__DOT__commandRunning)
            ? (((QData)((IData)(vlTOPp->top__DOT__rasteriCEr__DOT__commandParser__DOT__configReg
                                [0U])) << 0x30U) | 
               (((QData)((IData)(vlTOPp->top__DOT__rasteriCEr__DOT__commandParser__DOT__configReg
                                 [0U])) << 0x20U) | 
                (((QData)((IData)(vlTOPp->top__DOT__rasteriCEr__DOT__commandParser__DOT__configReg
                                  [0U])) << 0x10U) 
                 | (QData)((IData)(vlTOPp->top__DOT__rasteriCEr__DOT__commandParser__DOT__configReg
                                   [0U]))))) : (((QData)((IData)(vlTOPp->top__DOT__rasteriCEr__DOT__colorOut)) 
                                                 << 0x30U) 
                                                | (((QData)((IData)(vlTOPp->top__DOT__rasteriCEr__DOT__colorOut)) 
                                                    << 0x20U) 
                                                   | (((QData)((IData)(vlTOPp->top__DOT__rasteriCEr__DOT__colorOut)) 
                                                       << 0x10U) 
                                                      | (QData)((IData)(vlTOPp->top__DOT__rasteriCEr__DOT__colorOut))))));
    vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__alphaTestLess 
        = ((IData)(vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__alphaTestFragmentVal) 
           < (0xfU & (vlTOPp->top__DOT__rasteriCEr__DOT__commandParser__DOT__configReg
                      [2U] >> 7U)));
    vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__alphaTestGreater 
        = ((IData)(vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__alphaTestFragmentVal) 
           > (0xfU & (vlTOPp->top__DOT__rasteriCEr__DOT__commandParser__DOT__configReg
                      [2U] >> 7U)));
    vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__alphaTestEqual 
        = ((IData)(vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__alphaTestFragmentVal) 
           == (0xfU & (vlTOPp->top__DOT__rasteriCEr__DOT__commandParser__DOT__configReg
                       [2U] >> 7U)));
    vlTOPp->top__DOT__rasteriCEr__DOT__depthBuffer__DOT__writeStrobe 
        = ((0xfffeU & (IData)(vlTOPp->top__DOT__rasteriCEr__DOT__depthBuffer__DOT__writeStrobe)) 
           | ((0U == (3U & vlTOPp->top__DOT__rasteriCEr__DOT__depthIndexWrite)) 
              & (vlTOPp->top__DOT__rasteriCEr__DOT__commandParser__DOT__configReg
                 [2U] >> 0xbU)));
    vlTOPp->top__DOT__rasteriCEr__DOT__depthBuffer__DOT__writeStrobe 
        = ((0xfffdU & (IData)(vlTOPp->top__DOT__rasteriCEr__DOT__depthBuffer__DOT__writeStrobe)) 
           | (0x3ffffeU & (((0U == (3U & vlTOPp->top__DOT__rasteriCEr__DOT__depthIndexWrite)) 
                            << 1U) & (vlTOPp->top__DOT__rasteriCEr__DOT__commandParser__DOT__configReg
                                      [2U] >> 0xaU))));
    vlTOPp->top__DOT__rasteriCEr__DOT__depthBuffer__DOT__writeStrobe 
        = ((0xfffbU & (IData)(vlTOPp->top__DOT__rasteriCEr__DOT__depthBuffer__DOT__writeStrobe)) 
           | (0x7ffffcU & (((0U == (3U & vlTOPp->top__DOT__rasteriCEr__DOT__depthIndexWrite)) 
                            << 2U) & (vlTOPp->top__DOT__rasteriCEr__DOT__commandParser__DOT__configReg
                                      [2U] >> 9U))));
    vlTOPp->top__DOT__rasteriCEr__DOT__depthBuffer__DOT__writeStrobe 
        = ((0xfff7U & (IData)(vlTOPp->top__DOT__rasteriCEr__DOT__depthBuffer__DOT__writeStrobe)) 
           | (0xfffff8U & (((0U == (3U & vlTOPp->top__DOT__rasteriCEr__DOT__depthIndexWrite)) 
                            << 3U) & (vlTOPp->top__DOT__rasteriCEr__DOT__commandParser__DOT__configReg
                                      [2U] >> 8U))));
    vlTOPp->top__DOT__rasteriCEr__DOT__depthBuffer__DOT__writeStrobe 
        = ((0xffefU & (IData)(vlTOPp->top__DOT__rasteriCEr__DOT__depthBuffer__DOT__writeStrobe)) 
           | (0x1fffff0U & (((1U == (3U & vlTOPp->top__DOT__rasteriCEr__DOT__depthIndexWrite)) 
                             << 4U) & (vlTOPp->top__DOT__rasteriCEr__DOT__commandParser__DOT__configReg
                                       [2U] >> 7U))));
    vlTOPp->top__DOT__rasteriCEr__DOT__depthBuffer__DOT__writeStrobe 
        = ((0xffdfU & (IData)(vlTOPp->top__DOT__rasteriCEr__DOT__depthBuffer__DOT__writeStrobe)) 
           | (0x3ffffe0U & (((1U == (3U & vlTOPp->top__DOT__rasteriCEr__DOT__depthIndexWrite)) 
                             << 5U) & (vlTOPp->top__DOT__rasteriCEr__DOT__commandParser__DOT__configReg
                                       [2U] >> 6U))));
    vlTOPp->top__DOT__rasteriCEr__DOT__depthBuffer__DOT__writeStrobe 
        = ((0xffbfU & (IData)(vlTOPp->top__DOT__rasteriCEr__DOT__depthBuffer__DOT__writeStrobe)) 
           | (0x7ffffc0U & (((1U == (3U & vlTOPp->top__DOT__rasteriCEr__DOT__depthIndexWrite)) 
                             << 6U) & (vlTOPp->top__DOT__rasteriCEr__DOT__commandParser__DOT__configReg
                                       [2U] >> 5U))));
    vlTOPp->top__DOT__rasteriCEr__DOT__depthBuffer__DOT__writeStrobe 
        = ((0xff7fU & (IData)(vlTOPp->top__DOT__rasteriCEr__DOT__depthBuffer__DOT__writeStrobe)) 
           | (0xfffff80U & (((1U == (3U & vlTOPp->top__DOT__rasteriCEr__DOT__depthIndexWrite)) 
                             << 7U) & (vlTOPp->top__DOT__rasteriCEr__DOT__commandParser__DOT__configReg
                                       [2U] >> 4U))));
    vlTOPp->top__DOT__rasteriCEr__DOT__depthBuffer__DOT__writeStrobe 
        = ((0xfeffU & (IData)(vlTOPp->top__DOT__rasteriCEr__DOT__depthBuffer__DOT__writeStrobe)) 
           | (0x1fffff00U & (((2U == (3U & vlTOPp->top__DOT__rasteriCEr__DOT__depthIndexWrite)) 
                              << 8U) & (vlTOPp->top__DOT__rasteriCEr__DOT__commandParser__DOT__configReg
                                        [2U] >> 3U))));
    vlTOPp->top__DOT__rasteriCEr__DOT__depthBuffer__DOT__writeStrobe 
        = ((0xfdffU & (IData)(vlTOPp->top__DOT__rasteriCEr__DOT__depthBuffer__DOT__writeStrobe)) 
           | (0x3ffffe00U & (((2U == (3U & vlTOPp->top__DOT__rasteriCEr__DOT__depthIndexWrite)) 
                              << 9U) & (vlTOPp->top__DOT__rasteriCEr__DOT__commandParser__DOT__configReg
                                        [2U] >> 2U))));
    vlTOPp->top__DOT__rasteriCEr__DOT__depthBuffer__DOT__writeStrobe 
        = ((0xfbffU & (IData)(vlTOPp->top__DOT__rasteriCEr__DOT__depthBuffer__DOT__writeStrobe)) 
           | (0x7ffffc00U & (((2U == (3U & vlTOPp->top__DOT__rasteriCEr__DOT__depthIndexWrite)) 
                              << 0xaU) & (vlTOPp->top__DOT__rasteriCEr__DOT__commandParser__DOT__configReg
                                          [2U] >> 1U))));
    vlTOPp->top__DOT__rasteriCEr__DOT__depthBuffer__DOT__writeStrobe 
        = ((0xf7ffU & (IData)(vlTOPp->top__DOT__rasteriCEr__DOT__depthBuffer__DOT__writeStrobe)) 
           | (0xfffff800U & (((2U == (3U & vlTOPp->top__DOT__rasteriCEr__DOT__depthIndexWrite)) 
                              << 0xbU) & vlTOPp->top__DOT__rasteriCEr__DOT__commandParser__DOT__configReg
                             [2U])));
    vlTOPp->top__DOT__rasteriCEr__DOT__depthBuffer__DOT__writeStrobe 
        = ((0xefffU & (IData)(vlTOPp->top__DOT__rasteriCEr__DOT__depthBuffer__DOT__writeStrobe)) 
           | (0xfffff000U & (((3U == (3U & vlTOPp->top__DOT__rasteriCEr__DOT__depthIndexWrite)) 
                              << 0xcU) & (vlTOPp->top__DOT__rasteriCEr__DOT__commandParser__DOT__configReg
                                          [2U] << 1U))));
    vlTOPp->top__DOT__rasteriCEr__DOT__depthBuffer__DOT__writeStrobe 
        = ((0xdfffU & (IData)(vlTOPp->top__DOT__rasteriCEr__DOT__depthBuffer__DOT__writeStrobe)) 
           | (0xffffe000U & (((3U == (3U & vlTOPp->top__DOT__rasteriCEr__DOT__depthIndexWrite)) 
                              << 0xdU) & (vlTOPp->top__DOT__rasteriCEr__DOT__commandParser__DOT__configReg
                                          [2U] << 2U))));
    vlTOPp->top__DOT__rasteriCEr__DOT__depthBuffer__DOT__writeStrobe 
        = ((0xbfffU & (IData)(vlTOPp->top__DOT__rasteriCEr__DOT__depthBuffer__DOT__writeStrobe)) 
           | (0xffffc000U & (((3U == (3U & vlTOPp->top__DOT__rasteriCEr__DOT__depthIndexWrite)) 
                              << 0xeU) & (vlTOPp->top__DOT__rasteriCEr__DOT__commandParser__DOT__configReg
                                          [2U] << 3U))));
    vlTOPp->top__DOT__rasteriCEr__DOT__depthBuffer__DOT__writeStrobe 
        = ((0x7fffU & (IData)(vlTOPp->top__DOT__rasteriCEr__DOT__depthBuffer__DOT__writeStrobe)) 
           | (0xffff8000U & (((3U == (3U & vlTOPp->top__DOT__rasteriCEr__DOT__depthIndexWrite)) 
                              << 0xfU) & (vlTOPp->top__DOT__rasteriCEr__DOT__commandParser__DOT__configReg
                                          [2U] << 4U))));
    vlTOPp->top__DOT__rasteriCEr__DOT__colorBuffer__DOT__writeStrobe 
        = ((0xfffeU & (IData)(vlTOPp->top__DOT__rasteriCEr__DOT__colorBuffer__DOT__writeStrobe)) 
           | ((0U == (3U & vlTOPp->top__DOT__rasteriCEr__DOT__colorIndexWrite)) 
              & (vlTOPp->top__DOT__rasteriCEr__DOT__commandParser__DOT__configReg
                 [2U] >> 0xcU)));
    vlTOPp->top__DOT__rasteriCEr__DOT__colorBuffer__DOT__writeStrobe 
        = ((0xfffdU & (IData)(vlTOPp->top__DOT__rasteriCEr__DOT__colorBuffer__DOT__writeStrobe)) 
           | (0xffffeU & (((0U == (3U & vlTOPp->top__DOT__rasteriCEr__DOT__colorIndexWrite)) 
                           << 1U) & (vlTOPp->top__DOT__rasteriCEr__DOT__commandParser__DOT__configReg
                                     [2U] >> 0xcU))));
    vlTOPp->top__DOT__rasteriCEr__DOT__colorBuffer__DOT__writeStrobe 
        = ((0xfffbU & (IData)(vlTOPp->top__DOT__rasteriCEr__DOT__colorBuffer__DOT__writeStrobe)) 
           | (0xffffcU & (((0U == (3U & vlTOPp->top__DOT__rasteriCEr__DOT__colorIndexWrite)) 
                           << 2U) & (vlTOPp->top__DOT__rasteriCEr__DOT__commandParser__DOT__configReg
                                     [2U] >> 0xcU))));
    vlTOPp->top__DOT__rasteriCEr__DOT__colorBuffer__DOT__writeStrobe 
        = ((0xfff7U & (IData)(vlTOPp->top__DOT__rasteriCEr__DOT__colorBuffer__DOT__writeStrobe)) 
           | (0xffff8U & (((0U == (3U & vlTOPp->top__DOT__rasteriCEr__DOT__colorIndexWrite)) 
                           << 3U) & (vlTOPp->top__DOT__rasteriCEr__DOT__commandParser__DOT__configReg
                                     [2U] >> 0xcU))));
    vlTOPp->top__DOT__rasteriCEr__DOT__colorBuffer__DOT__writeStrobe 
        = ((0xffefU & (IData)(vlTOPp->top__DOT__rasteriCEr__DOT__colorBuffer__DOT__writeStrobe)) 
           | (0xfffff0U & (((1U == (3U & vlTOPp->top__DOT__rasteriCEr__DOT__colorIndexWrite)) 
                            << 4U) & (vlTOPp->top__DOT__rasteriCEr__DOT__commandParser__DOT__configReg
                                      [2U] >> 8U))));
    vlTOPp->top__DOT__rasteriCEr__DOT__colorBuffer__DOT__writeStrobe 
        = ((0xffdfU & (IData)(vlTOPp->top__DOT__rasteriCEr__DOT__colorBuffer__DOT__writeStrobe)) 
           | (0xffffe0U & (((1U == (3U & vlTOPp->top__DOT__rasteriCEr__DOT__colorIndexWrite)) 
                            << 5U) & (vlTOPp->top__DOT__rasteriCEr__DOT__commandParser__DOT__configReg
                                      [2U] >> 8U))));
    vlTOPp->top__DOT__rasteriCEr__DOT__colorBuffer__DOT__writeStrobe 
        = ((0xffbfU & (IData)(vlTOPp->top__DOT__rasteriCEr__DOT__colorBuffer__DOT__writeStrobe)) 
           | (0xffffc0U & (((1U == (3U & vlTOPp->top__DOT__rasteriCEr__DOT__colorIndexWrite)) 
                            << 6U) & (vlTOPp->top__DOT__rasteriCEr__DOT__commandParser__DOT__configReg
                                      [2U] >> 8U))));
    vlTOPp->top__DOT__rasteriCEr__DOT__colorBuffer__DOT__writeStrobe 
        = ((0xff7fU & (IData)(vlTOPp->top__DOT__rasteriCEr__DOT__colorBuffer__DOT__writeStrobe)) 
           | (0xffff80U & (((1U == (3U & vlTOPp->top__DOT__rasteriCEr__DOT__colorIndexWrite)) 
                            << 7U) & (vlTOPp->top__DOT__rasteriCEr__DOT__commandParser__DOT__configReg
                                      [2U] >> 8U))));
    vlTOPp->top__DOT__rasteriCEr__DOT__colorBuffer__DOT__writeStrobe 
        = ((0xfeffU & (IData)(vlTOPp->top__DOT__rasteriCEr__DOT__colorBuffer__DOT__writeStrobe)) 
           | (0xfffff00U & (((2U == (3U & vlTOPp->top__DOT__rasteriCEr__DOT__colorIndexWrite)) 
                             << 8U) & (vlTOPp->top__DOT__rasteriCEr__DOT__commandParser__DOT__configReg
                                       [2U] >> 4U))));
    vlTOPp->top__DOT__rasteriCEr__DOT__colorBuffer__DOT__writeStrobe 
        = ((0xfdffU & (IData)(vlTOPp->top__DOT__rasteriCEr__DOT__colorBuffer__DOT__writeStrobe)) 
           | (0xffffe00U & (((2U == (3U & vlTOPp->top__DOT__rasteriCEr__DOT__colorIndexWrite)) 
                             << 9U) & (vlTOPp->top__DOT__rasteriCEr__DOT__commandParser__DOT__configReg
                                       [2U] >> 4U))));
    vlTOPp->top__DOT__rasteriCEr__DOT__colorBuffer__DOT__writeStrobe 
        = ((0xfbffU & (IData)(vlTOPp->top__DOT__rasteriCEr__DOT__colorBuffer__DOT__writeStrobe)) 
           | (0xffffc00U & (((2U == (3U & vlTOPp->top__DOT__rasteriCEr__DOT__colorIndexWrite)) 
                             << 0xaU) & (vlTOPp->top__DOT__rasteriCEr__DOT__commandParser__DOT__configReg
                                         [2U] >> 4U))));
    vlTOPp->top__DOT__rasteriCEr__DOT__colorBuffer__DOT__writeStrobe 
        = ((0xf7ffU & (IData)(vlTOPp->top__DOT__rasteriCEr__DOT__colorBuffer__DOT__writeStrobe)) 
           | (0xffff800U & (((2U == (3U & vlTOPp->top__DOT__rasteriCEr__DOT__colorIndexWrite)) 
                             << 0xbU) & (vlTOPp->top__DOT__rasteriCEr__DOT__commandParser__DOT__configReg
                                         [2U] >> 4U))));
    vlTOPp->top__DOT__rasteriCEr__DOT__colorBuffer__DOT__writeStrobe 
        = ((0xefffU & (IData)(vlTOPp->top__DOT__rasteriCEr__DOT__colorBuffer__DOT__writeStrobe)) 
           | (0xfffff000U & (((3U == (3U & vlTOPp->top__DOT__rasteriCEr__DOT__colorIndexWrite)) 
                              << 0xcU) & vlTOPp->top__DOT__rasteriCEr__DOT__commandParser__DOT__configReg
                             [2U])));
    vlTOPp->top__DOT__rasteriCEr__DOT__colorBuffer__DOT__writeStrobe 
        = ((0xdfffU & (IData)(vlTOPp->top__DOT__rasteriCEr__DOT__colorBuffer__DOT__writeStrobe)) 
           | (0xffffe000U & (((3U == (3U & vlTOPp->top__DOT__rasteriCEr__DOT__colorIndexWrite)) 
                              << 0xdU) & vlTOPp->top__DOT__rasteriCEr__DOT__commandParser__DOT__configReg
                             [2U])));
    vlTOPp->top__DOT__rasteriCEr__DOT__colorBuffer__DOT__writeStrobe 
        = ((0xbfffU & (IData)(vlTOPp->top__DOT__rasteriCEr__DOT__colorBuffer__DOT__writeStrobe)) 
           | (0xffffc000U & (((3U == (3U & vlTOPp->top__DOT__rasteriCEr__DOT__colorIndexWrite)) 
                              << 0xeU) & vlTOPp->top__DOT__rasteriCEr__DOT__commandParser__DOT__configReg
                             [2U])));
    vlTOPp->top__DOT__rasteriCEr__DOT__colorBuffer__DOT__writeStrobe 
        = ((0x7fffU & (IData)(vlTOPp->top__DOT__rasteriCEr__DOT__colorBuffer__DOT__writeStrobe)) 
           | (0xffff8000U & (((3U == (3U & vlTOPp->top__DOT__rasteriCEr__DOT__colorIndexWrite)) 
                              << 0xfU) & vlTOPp->top__DOT__rasteriCEr__DOT__commandParser__DOT__configReg
                             [2U])));
    vlTOPp->top__DOT__rasteriCEr__DOT__rop__DOT__fbIndex 
        = (0x1ffffU & (((IData)(0x280U) * ((IData)(0x77U) 
                                           - (IData)(vlTOPp->top__DOT__rasteriCEr__DOT__rop__DOT__y))) 
                       + (IData)(vlTOPp->top__DOT__rasteriCEr__DOT__rop__DOT__x)));
    vlTOPp->__Vfunc_top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__truncate16__0__in 
        = (vlTOPp->top__DOT__rasteriCEr__DOT__m_fragment_axis_tdata[2U] 
           >> 0xfU);
    vlTOPp->__Vfunc_top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__truncate16__0__Vfuncout 
        = (0xffffU & vlTOPp->__Vfunc_top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__truncate16__0__in);
    vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__recipIn 
        = vlTOPp->__Vfunc_top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__truncate16__0__Vfuncout;
    vlTOPp->top__DOT__rasteriCEr__DOT__rop__DOT__isInTriangle 
        = (1U & (~ (((vlTOPp->top__DOT__rasteriCEr__DOT__rop__DOT__paramMem
                      [3U] | vlTOPp->top__DOT__rasteriCEr__DOT__rop__DOT__paramMem
                      [4U]) | vlTOPp->top__DOT__rasteriCEr__DOT__rop__DOT__paramMem
                     [5U]) >> 0x1fU)));
    vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__depthTestPassed 
        = (1U & ((~ ((((((((0U == (7U & (vlTOPp->top__DOT__rasteriCEr__DOT__commandParser__DOT__configReg
                                         [2U] >> 1U))) 
                           | (1U == (7U & (vlTOPp->top__DOT__rasteriCEr__DOT__commandParser__DOT__configReg
                                           [2U] >> 1U)))) 
                          | (2U == (7U & (vlTOPp->top__DOT__rasteriCEr__DOT__commandParser__DOT__configReg
                                          [2U] >> 1U)))) 
                         | (3U == (7U & (vlTOPp->top__DOT__rasteriCEr__DOT__commandParser__DOT__configReg
                                         [2U] >> 1U)))) 
                        | (4U == (7U & (vlTOPp->top__DOT__rasteriCEr__DOT__commandParser__DOT__configReg
                                        [2U] >> 1U)))) 
                       | (5U == (7U & (vlTOPp->top__DOT__rasteriCEr__DOT__commandParser__DOT__configReg
                                       [2U] >> 1U)))) 
                      | (6U == (7U & (vlTOPp->top__DOT__rasteriCEr__DOT__commandParser__DOT__configReg
                                      [2U] >> 1U)))) 
                     | (7U == (7U & (vlTOPp->top__DOT__rasteriCEr__DOT__commandParser__DOT__configReg
                                     [2U] >> 1U))))) 
                 | ((0U == (7U & (vlTOPp->top__DOT__rasteriCEr__DOT__commandParser__DOT__configReg
                                  [2U] >> 1U))) | (
                                                   (1U 
                                                    != 
                                                    (7U 
                                                     & (vlTOPp->top__DOT__rasteriCEr__DOT__commandParser__DOT__configReg
                                                        [2U] 
                                                        >> 1U))) 
                                                   & ((2U 
                                                       == 
                                                       (7U 
                                                        & (vlTOPp->top__DOT__rasteriCEr__DOT__commandParser__DOT__configReg
                                                           [2U] 
                                                           >> 1U)))
                                                       ? (IData)(vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__depthTestLess)
                                                       : 
                                                      ((3U 
                                                        == 
                                                        (7U 
                                                         & (vlTOPp->top__DOT__rasteriCEr__DOT__commandParser__DOT__configReg
                                                            [2U] 
                                                            >> 1U)))
                                                        ? (IData)(vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__depthTestEqual)
                                                        : 
                                                       ((4U 
                                                         == 
                                                         (7U 
                                                          & (vlTOPp->top__DOT__rasteriCEr__DOT__commandParser__DOT__configReg
                                                             [2U] 
                                                             >> 1U)))
                                                         ? 
                                                        ((IData)(vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__depthTestLess) 
                                                         | (IData)(vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__depthTestEqual))
                                                         : 
                                                        ((5U 
                                                          == 
                                                          (7U 
                                                           & (vlTOPp->top__DOT__rasteriCEr__DOT__commandParser__DOT__configReg
                                                              [2U] 
                                                              >> 1U)))
                                                          ? (IData)(vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__depthTestGreater)
                                                          : 
                                                         ((6U 
                                                           == 
                                                           (7U 
                                                            & (vlTOPp->top__DOT__rasteriCEr__DOT__commandParser__DOT__configReg
                                                               [2U] 
                                                               >> 1U)))
                                                           ? 
                                                          (~ (IData)(vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__depthTestEqual))
                                                           : 
                                                          ((IData)(vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__depthTestGreater) 
                                                           | (IData)(vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__depthTestEqual)))))))))));
    vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__alphaTestPassed 
        = (1U & ((~ ((((((((0U == (7U & (vlTOPp->top__DOT__rasteriCEr__DOT__commandParser__DOT__configReg
                                         [2U] >> 4U))) 
                           | (1U == (7U & (vlTOPp->top__DOT__rasteriCEr__DOT__commandParser__DOT__configReg
                                           [2U] >> 4U)))) 
                          | (2U == (7U & (vlTOPp->top__DOT__rasteriCEr__DOT__commandParser__DOT__configReg
                                          [2U] >> 4U)))) 
                         | (3U == (7U & (vlTOPp->top__DOT__rasteriCEr__DOT__commandParser__DOT__configReg
                                         [2U] >> 4U)))) 
                        | (4U == (7U & (vlTOPp->top__DOT__rasteriCEr__DOT__commandParser__DOT__configReg
                                        [2U] >> 4U)))) 
                       | (5U == (7U & (vlTOPp->top__DOT__rasteriCEr__DOT__commandParser__DOT__configReg
                                       [2U] >> 4U)))) 
                      | (6U == (7U & (vlTOPp->top__DOT__rasteriCEr__DOT__commandParser__DOT__configReg
                                      [2U] >> 4U)))) 
                     | (7U == (7U & (vlTOPp->top__DOT__rasteriCEr__DOT__commandParser__DOT__configReg
                                     [2U] >> 4U))))) 
                 | ((0U == (7U & (vlTOPp->top__DOT__rasteriCEr__DOT__commandParser__DOT__configReg
                                  [2U] >> 4U))) | (
                                                   (1U 
                                                    != 
                                                    (7U 
                                                     & (vlTOPp->top__DOT__rasteriCEr__DOT__commandParser__DOT__configReg
                                                        [2U] 
                                                        >> 4U))) 
                                                   & ((2U 
                                                       == 
                                                       (7U 
                                                        & (vlTOPp->top__DOT__rasteriCEr__DOT__commandParser__DOT__configReg
                                                           [2U] 
                                                           >> 4U)))
                                                       ? (IData)(vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__alphaTestLess)
                                                       : 
                                                      ((3U 
                                                        == 
                                                        (7U 
                                                         & (vlTOPp->top__DOT__rasteriCEr__DOT__commandParser__DOT__configReg
                                                            [2U] 
                                                            >> 4U)))
                                                        ? (IData)(vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__alphaTestEqual)
                                                        : 
                                                       ((4U 
                                                         == 
                                                         (7U 
                                                          & (vlTOPp->top__DOT__rasteriCEr__DOT__commandParser__DOT__configReg
                                                             [2U] 
                                                             >> 4U)))
                                                         ? 
                                                        ((IData)(vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__alphaTestLess) 
                                                         | (IData)(vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__alphaTestEqual))
                                                         : 
                                                        ((5U 
                                                          == 
                                                          (7U 
                                                           & (vlTOPp->top__DOT__rasteriCEr__DOT__commandParser__DOT__configReg
                                                              [2U] 
                                                              >> 4U)))
                                                          ? (IData)(vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__alphaTestGreater)
                                                          : 
                                                         ((6U 
                                                           == 
                                                           (7U 
                                                            & (vlTOPp->top__DOT__rasteriCEr__DOT__commandParser__DOT__configReg
                                                               [2U] 
                                                               >> 4U)))
                                                           ? 
                                                          (~ (IData)(vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__alphaTestEqual))
                                                           : 
                                                          ((IData)(vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__alphaTestGreater) 
                                                           | (IData)(vlTOPp->top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__alphaTestEqual)))))))))));
    vlTOPp->top__DOT__rasteriCEr__DOT__depthBuffer__DOT__fbWrMaskBus 
        = (0xffffU & ((IData)(vlTOPp->top__DOT__rasteriCEr__DOT__depthBuffer__DOT__commandRunning)
                       ? (- (IData)((1U & (vlTOPp->top__DOT__rasteriCEr__DOT__commandParser__DOT__configReg
                                           [2U] >> 0xbU))))
                       : (IData)(vlTOPp->top__DOT__rasteriCEr__DOT__depthBuffer__DOT__writeStrobe)));
    vlTOPp->top__DOT__rasteriCEr__DOT__colorBuffer__DOT__fbWrMaskBus 
        = (0xffffU & ((IData)(vlTOPp->top__DOT__rasteriCEr__DOT__colorBuffer__DOT__commandRunning)
                       ? ((0xf000U & vlTOPp->top__DOT__rasteriCEr__DOT__commandParser__DOT__configReg
                           [2U]) | ((0xf00U & (vlTOPp->top__DOT__rasteriCEr__DOT__commandParser__DOT__configReg
                                               [2U] 
                                               >> 4U)) 
                                    | ((0xf0U & (vlTOPp->top__DOT__rasteriCEr__DOT__commandParser__DOT__configReg
                                                 [2U] 
                                                 >> 8U)) 
                                       | (0xfU & (vlTOPp->top__DOT__rasteriCEr__DOT__commandParser__DOT__configReg
                                                  [2U] 
                                                  >> 0xcU)))))
                       : (IData)(vlTOPp->top__DOT__rasteriCEr__DOT__colorBuffer__DOT__writeStrobe)));
    vlTOPp->top__DOT__rasteriCEr__DOT__rop__DOT__isInTriangleAndInBounds 
        = (((IData)(vlTOPp->top__DOT__rasteriCEr__DOT__rop__DOT__isInTriangle) 
            & ((IData)(vlTOPp->top__DOT__rasteriCEr__DOT__rop__DOT__x) 
               < (0x7ffU & vlTOPp->top__DOT__rasteriCEr__DOT__rop__DOT__paramMem
                  [2U]))) & ((IData)(vlTOPp->top__DOT__rasteriCEr__DOT__rop__DOT__x) 
                             >= (0x7ffU & vlTOPp->top__DOT__rasteriCEr__DOT__rop__DOT__paramMem
                                 [1U])));
}

void Vtop::_eval_initial(Vtop__Syms* __restrict vlSymsp) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vtop::_eval_initial\n"); );
    Vtop* const __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    // Body
    vlTOPp->__Vclklast__TOP__aclk = vlTOPp->aclk;
    vlTOPp->_initial__TOP__2(vlSymsp);
}

void Vtop::final() {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vtop::final\n"); );
    // Variables
    Vtop__Syms* __restrict vlSymsp = this->__VlSymsp;
    Vtop* const __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
}

void Vtop::_eval_settle(Vtop__Syms* __restrict vlSymsp) {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vtop::_eval_settle\n"); );
    Vtop* const __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    // Body
    vlTOPp->_settle__TOP__3(vlSymsp);
}

void Vtop::_ctor_var_reset() {
    VL_DEBUG_IF(VL_DBG_MSGF("+    Vtop::_ctor_var_reset\n"); );
    // Body
    aclk = VL_RAND_RESET_I(1);
    resetn = VL_RAND_RESET_I(1);
    s_cmd_axis_tvalid = VL_RAND_RESET_I(1);
    s_cmd_axis_tready = VL_RAND_RESET_I(1);
    s_cmd_axis_tlast = VL_RAND_RESET_I(1);
    s_cmd_axis_tdata = VL_RAND_RESET_I(32);
    m_framebuffer_axis_tvalid = VL_RAND_RESET_I(1);
    m_framebuffer_axis_tready = VL_RAND_RESET_I(1);
    m_framebuffer_axis_tlast = VL_RAND_RESET_I(1);
    m_framebuffer_axis_tdata = VL_RAND_RESET_Q(64);
    top__DOT__m_cmd_axis_tvalid = VL_RAND_RESET_I(1);
    top__DOT__m_cmd_axis_tready = VL_RAND_RESET_I(1);
    top__DOT__m_cmd_axis_tdata = VL_RAND_RESET_I(32);
    top__DOT__parser__DOT__state = VL_RAND_RESET_I(6);
    top__DOT__parser__DOT__counter = VL_RAND_RESET_I(28);
    top__DOT__rasteriCEr__DOT__texelIndex = VL_RAND_RESET_I(32);
    top__DOT__rasteriCEr__DOT__texel = VL_RAND_RESET_I(16);
    top__DOT__rasteriCEr__DOT__textureMode = VL_RAND_RESET_I(4);
    top__DOT__rasteriCEr__DOT__colorIndexRead = VL_RAND_RESET_I(17);
    top__DOT__rasteriCEr__DOT__colorIndexWrite = VL_RAND_RESET_I(17);
    top__DOT__rasteriCEr__DOT__colorWriteEnable = VL_RAND_RESET_I(1);
    top__DOT__rasteriCEr__DOT__colorOut = VL_RAND_RESET_I(16);
    top__DOT__rasteriCEr__DOT__depthIndexRead = VL_RAND_RESET_I(17);
    top__DOT__rasteriCEr__DOT__depthIndexWrite = VL_RAND_RESET_I(17);
    top__DOT__rasteriCEr__DOT__depthWriteEnable = VL_RAND_RESET_I(1);
    top__DOT__rasteriCEr__DOT__depthOut = VL_RAND_RESET_I(16);
    top__DOT__rasteriCEr__DOT__s_rasterizer_axis_tvalid = VL_RAND_RESET_I(1);
    top__DOT__rasteriCEr__DOT__s_rasterizer_axis_tready = VL_RAND_RESET_I(1);
    top__DOT__rasteriCEr__DOT__s_rasterizer_axis_tlast = VL_RAND_RESET_I(1);
    top__DOT__rasteriCEr__DOT__s_rasterizer_axis_tdata = VL_RAND_RESET_I(32);
    top__DOT__rasteriCEr__DOT__colorBufferApply = VL_RAND_RESET_I(1);
    top__DOT__rasteriCEr__DOT__colorBufferApplied = VL_RAND_RESET_I(1);
    top__DOT__rasteriCEr__DOT__colorBufferCmdCommit = VL_RAND_RESET_I(1);
    top__DOT__rasteriCEr__DOT__colorBufferCmdMemset = VL_RAND_RESET_I(1);
    top__DOT__rasteriCEr__DOT__depthBufferApply = VL_RAND_RESET_I(1);
    top__DOT__rasteriCEr__DOT__depthBufferApplied = VL_RAND_RESET_I(1);
    top__DOT__rasteriCEr__DOT__depthBufferCmdCommit = VL_RAND_RESET_I(1);
    top__DOT__rasteriCEr__DOT__depthBufferCmdMemset = VL_RAND_RESET_I(1);
    top__DOT__rasteriCEr__DOT__s_texture_axis_tvalid = VL_RAND_RESET_I(1);
    top__DOT__rasteriCEr__DOT__s_texture_axis_tready = VL_RAND_RESET_I(1);
    top__DOT__rasteriCEr__DOT__s_texture_axis_tlast = VL_RAND_RESET_I(1);
    top__DOT__rasteriCEr__DOT__s_texture_axis_tdata = VL_RAND_RESET_I(32);
    top__DOT__rasteriCEr__DOT__m_fragment_axis_tvalid = VL_RAND_RESET_I(1);
    VL_RAND_RESET_W(129, top__DOT__rasteriCEr__DOT__m_fragment_axis_tdata);
    { int __Vi0=0; for (; __Vi0<5; ++__Vi0) {
            top__DOT__rasteriCEr__DOT__commandParser__DOT__configReg[__Vi0] = VL_RAND_RESET_I(16);
    }}
    top__DOT__rasteriCEr__DOT__commandParser__DOT__apply = VL_RAND_RESET_I(1);
    top__DOT__rasteriCEr__DOT__commandParser__DOT__applied = VL_RAND_RESET_I(1);
    top__DOT__rasteriCEr__DOT__commandParser__DOT__streamCounter = VL_RAND_RESET_I(14);
    top__DOT__rasteriCEr__DOT__commandParser__DOT__state = VL_RAND_RESET_I(5);
    top__DOT__rasteriCEr__DOT__commandParser__DOT__fbControlState = VL_RAND_RESET_I(2);
    top__DOT__rasteriCEr__DOT__commandParser__DOT__wlsp = VL_RAND_RESET_I(1);
    top__DOT__rasteriCEr__DOT__commandParser__DOT__Copy16__DOT__lsp = VL_RAND_RESET_I(16);
    top__DOT__rasteriCEr__DOT__commandParser__DOT____Vlvbound1 = VL_RAND_RESET_I(16);
    top__DOT__rasteriCEr__DOT__texCache__DOT__memWriteAddr = VL_RAND_RESET_I(13);
    top__DOT__rasteriCEr__DOT__texCache__DOT__texelIndexConf = VL_RAND_RESET_I(14);
    top__DOT__rasteriCEr__DOT__texCache__DOT__texelIndexConfDelay = VL_RAND_RESET_I(14);
    { int __Vi0=0; for (; __Vi0<8192; ++__Vi0) {
            top__DOT__rasteriCEr__DOT__texCache__DOT__texCache__DOT__mem[__Vi0] = VL_RAND_RESET_I(32);
    }}
    top__DOT__rasteriCEr__DOT__texCache__DOT__texCache__DOT__memOut = VL_RAND_RESET_I(32);
    top__DOT__rasteriCEr__DOT__depthBuffer__DOT__stateTileControl = VL_RAND_RESET_I(6);
    top__DOT__rasteriCEr__DOT__depthBuffer__DOT__writeStrobe = VL_RAND_RESET_I(16);
    top__DOT__rasteriCEr__DOT__depthBuffer__DOT__fragAddrReadDelay = VL_RAND_RESET_I(17);
    top__DOT__rasteriCEr__DOT__depthBuffer__DOT__commandRunning = VL_RAND_RESET_I(1);
    top__DOT__rasteriCEr__DOT__depthBuffer__DOT__counter = VL_RAND_RESET_I(15);
    top__DOT__rasteriCEr__DOT__depthBuffer__DOT__counterNext = VL_RAND_RESET_I(15);
    top__DOT__rasteriCEr__DOT__depthBuffer__DOT__fbWr = VL_RAND_RESET_I(1);
    top__DOT__rasteriCEr__DOT__depthBuffer__DOT__fbAddrBusWrite = VL_RAND_RESET_I(15);
    top__DOT__rasteriCEr__DOT__depthBuffer__DOT__fbDataInBus = VL_RAND_RESET_Q(64);
    top__DOT__rasteriCEr__DOT__depthBuffer__DOT__fbWrBus = VL_RAND_RESET_I(1);
    top__DOT__rasteriCEr__DOT__depthBuffer__DOT__fbWrMaskBus = VL_RAND_RESET_I(16);
    { int __Vi0=0; for (; __Vi0<32768; ++__Vi0) {
            top__DOT__rasteriCEr__DOT__depthBuffer__DOT__ramTile__DOT__mem[__Vi0] = VL_RAND_RESET_Q(64);
    }}
    top__DOT__rasteriCEr__DOT__depthBuffer__DOT__ramTile__DOT__memOut = VL_RAND_RESET_Q(64);
    top__DOT__rasteriCEr__DOT__colorBuffer__DOT__stateTileControl = VL_RAND_RESET_I(6);
    top__DOT__rasteriCEr__DOT__colorBuffer__DOT__writeStrobe = VL_RAND_RESET_I(16);
    top__DOT__rasteriCEr__DOT__colorBuffer__DOT__fragAddrReadDelay = VL_RAND_RESET_I(17);
    top__DOT__rasteriCEr__DOT__colorBuffer__DOT__commandRunning = VL_RAND_RESET_I(1);
    top__DOT__rasteriCEr__DOT__colorBuffer__DOT__counter = VL_RAND_RESET_I(15);
    top__DOT__rasteriCEr__DOT__colorBuffer__DOT__counterNext = VL_RAND_RESET_I(15);
    top__DOT__rasteriCEr__DOT__colorBuffer__DOT__fbWr = VL_RAND_RESET_I(1);
    top__DOT__rasteriCEr__DOT__colorBuffer__DOT__fbAddrBusWrite = VL_RAND_RESET_I(15);
    top__DOT__rasteriCEr__DOT__colorBuffer__DOT__fbDataInBus = VL_RAND_RESET_Q(64);
    top__DOT__rasteriCEr__DOT__colorBuffer__DOT__fbWrBus = VL_RAND_RESET_I(1);
    top__DOT__rasteriCEr__DOT__colorBuffer__DOT__fbWrMaskBus = VL_RAND_RESET_I(16);
    { int __Vi0=0; for (; __Vi0<32768; ++__Vi0) {
            top__DOT__rasteriCEr__DOT__colorBuffer__DOT__ramTile__DOT__mem[__Vi0] = VL_RAND_RESET_Q(64);
    }}
    top__DOT__rasteriCEr__DOT__colorBuffer__DOT__ramTile__DOT__memOut = VL_RAND_RESET_Q(64);
    { int __Vi0=0; for (; __Vi0<21; ++__Vi0) {
            top__DOT__rasteriCEr__DOT__rop__DOT__paramMem[__Vi0] = VL_RAND_RESET_I(32);
    }}
    top__DOT__rasteriCEr__DOT__rop__DOT__paramIndex = VL_RAND_RESET_I(6);
    top__DOT__rasteriCEr__DOT__rop__DOT__parameterComplete = VL_RAND_RESET_I(1);
    top__DOT__rasteriCEr__DOT__rop__DOT__rasterizerState = VL_RAND_RESET_I(6);
    top__DOT__rasteriCEr__DOT__rop__DOT__y = VL_RAND_RESET_I(10);
    top__DOT__rasteriCEr__DOT__rop__DOT__x = VL_RAND_RESET_I(11);
    top__DOT__rasteriCEr__DOT__rop__DOT__isInTriangle = VL_RAND_RESET_I(1);
    top__DOT__rasteriCEr__DOT__rop__DOT__isInTriangleAndInBounds = VL_RAND_RESET_I(1);
    top__DOT__rasteriCEr__DOT__rop__DOT__fbIndex = VL_RAND_RESET_I(17);
    top__DOT__rasteriCEr__DOT__rop__DOT__edgeWalkingState = VL_RAND_RESET_I(6);
    top__DOT__rasteriCEr__DOT__rop__DOT__edgeWalkTryOtherside = VL_RAND_RESET_I(1);
    top__DOT__rasteriCEr__DOT__rop__DOT__edgeWalkingDirection = VL_RAND_RESET_I(1);
    top__DOT__rasteriCEr__DOT__rop__DOT__RasterizerCopy16__DOT__tmp = VL_RAND_RESET_I(32);
    top__DOT__rasteriCEr__DOT__rop__DOT____Vlvbound1 = VL_RAND_RESET_I(32);
    top__DOT__rasteriCEr__DOT__rop__DOT____Vlvbound2 = VL_RAND_RESET_I(32);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__textureS = VL_RAND_RESET_I(16);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__textureT = VL_RAND_RESET_I(16);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__fbIndex = VL_RAND_RESET_I(17);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__depthTestFragmentVal = VL_RAND_RESET_I(16);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__depthTestDepthBufferVal = VL_RAND_RESET_I(16);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__depthTestPassed = VL_RAND_RESET_I(1);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__depthTestLess = VL_RAND_RESET_I(1);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__depthTestGreater = VL_RAND_RESET_I(1);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__depthTestEqual = VL_RAND_RESET_I(1);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__alphaTestFragmentVal = VL_RAND_RESET_I(4);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__alphaTestPassed = VL_RAND_RESET_I(1);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__alphaTestLess = VL_RAND_RESET_I(1);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__alphaTestGreater = VL_RAND_RESET_I(1);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__alphaTestEqual = VL_RAND_RESET_I(1);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__textureSTmp = VL_RAND_RESET_I(16);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__textureTTmp = VL_RAND_RESET_I(16);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__recipIn = VL_RAND_RESET_I(16);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__wValue = VL_RAND_RESET_I(16);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepWaitForRequestValid = VL_RAND_RESET_I(1);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepWaitForRequestTriangleColor = VL_RAND_RESET_I(16);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepCalculatePerspectiveCorrectionValid = VL_RAND_RESET_I(1);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepCalculatePerspectiveCorrectionDepthBufferVal = VL_RAND_RESET_I(16);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepCalculatePerspectiveCorrectionfbIndex = VL_RAND_RESET_I(17);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepCalculatePerspectiveCorrectionTriangleColor = VL_RAND_RESET_I(16);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepWaitForMemoryValid = VL_RAND_RESET_I(1);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepWaitForMemoryFbIndex = VL_RAND_RESET_I(17);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepWaitForMemoryDepthValue = VL_RAND_RESET_I(16);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepWaitForMemoryTriangleColor = VL_RAND_RESET_I(16);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepTexEnvValid = VL_RAND_RESET_I(1);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepTexEnvFbIndex = VL_RAND_RESET_I(17);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepTexEnvColorFrag = VL_RAND_RESET_I(16);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepTexEnvDepthValue = VL_RAND_RESET_I(16);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepTexEnvV00 = VL_RAND_RESET_I(8);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepTexEnvV01 = VL_RAND_RESET_I(8);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepTexEnvV02 = VL_RAND_RESET_I(8);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepTexEnvV03 = VL_RAND_RESET_I(8);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepTexEnvV10 = VL_RAND_RESET_I(8);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepTexEnvV11 = VL_RAND_RESET_I(8);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepTexEnvV12 = VL_RAND_RESET_I(8);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepTexEnvV13 = VL_RAND_RESET_I(8);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepTexEnvResultValid = VL_RAND_RESET_I(1);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepTexEnvResultFbIndex = VL_RAND_RESET_I(17);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepTexEnvResultColorFrag = VL_RAND_RESET_I(16);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepTexEnvResultColorTex = VL_RAND_RESET_I(16);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepTexEnvResultDepthValue = VL_RAND_RESET_I(16);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepTexEnvResultWriteColor = VL_RAND_RESET_I(1);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepBlendValid = VL_RAND_RESET_I(1);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepBlendFbIndex = VL_RAND_RESET_I(17);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepBlendDepthValue = VL_RAND_RESET_I(16);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepBlendWriteColor = VL_RAND_RESET_I(1);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepBlendV00 = VL_RAND_RESET_I(8);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepBlendV01 = VL_RAND_RESET_I(8);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepBlendV02 = VL_RAND_RESET_I(8);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepBlendV03 = VL_RAND_RESET_I(8);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepBlendV10 = VL_RAND_RESET_I(8);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepBlendV11 = VL_RAND_RESET_I(8);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepBlendV12 = VL_RAND_RESET_I(8);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepBlendV13 = VL_RAND_RESET_I(8);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepBlendResultValid = VL_RAND_RESET_I(1);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepBlendResultFbIndex = VL_RAND_RESET_I(17);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepBlendResultColorFrag = VL_RAND_RESET_I(16);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepBlendResultDepthValue = VL_RAND_RESET_I(16);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepBlendResultWriteColor = VL_RAND_RESET_I(1);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepBubbleValid = VL_RAND_RESET_I(1);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepBubbleFbIndex = VL_RAND_RESET_I(17);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepBubbleColorFrag = VL_RAND_RESET_I(16);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepBubbleDepthValue = VL_RAND_RESET_I(16);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__stepBubbleWriteColor = VL_RAND_RESET_I(1);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__TexEnvCalc__DOT__rs = VL_RAND_RESET_I(4);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__TexEnvCalc__DOT__gs = VL_RAND_RESET_I(4);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__TexEnvCalc__DOT__bs = VL_RAND_RESET_I(4);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__TexEnvCalc__DOT__as = VL_RAND_RESET_I(4);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__TexEnvCalc__DOT__rp = VL_RAND_RESET_I(4);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__TexEnvCalc__DOT__gp = VL_RAND_RESET_I(4);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__TexEnvCalc__DOT__bp = VL_RAND_RESET_I(4);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__TexEnvCalc__DOT__ap = VL_RAND_RESET_I(4);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__TexEnvCalc__DOT__rc = VL_RAND_RESET_I(4);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__TexEnvCalc__DOT__gc = VL_RAND_RESET_I(4);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__TexEnvCalc__DOT__bc = VL_RAND_RESET_I(4);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__TexEnvCalc__DOT__v00 = VL_RAND_RESET_I(4);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__TexEnvCalc__DOT__v01 = VL_RAND_RESET_I(4);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__TexEnvCalc__DOT__v02 = VL_RAND_RESET_I(4);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__TexEnvCalc__DOT__v03 = VL_RAND_RESET_I(4);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__TexEnvCalc__DOT__v10 = VL_RAND_RESET_I(4);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__TexEnvCalc__DOT__v11 = VL_RAND_RESET_I(4);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__TexEnvCalc__DOT__v12 = VL_RAND_RESET_I(4);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__TexEnvCalc__DOT__v13 = VL_RAND_RESET_I(4);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__TexEnvCalc__DOT__v20 = VL_RAND_RESET_I(4);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__TexEnvCalc__DOT__v21 = VL_RAND_RESET_I(4);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__TexEnvCalc__DOT__v22 = VL_RAND_RESET_I(4);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__TexEnvCalc__DOT__v23 = VL_RAND_RESET_I(4);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__TexEnvCalc__DOT__v30 = VL_RAND_RESET_I(4);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__TexEnvCalc__DOT__v31 = VL_RAND_RESET_I(4);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__TexEnvCalc__DOT__v32 = VL_RAND_RESET_I(4);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__TexEnvCalc__DOT__v33 = VL_RAND_RESET_I(4);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__TexEnvResultCalc__DOT__r = VL_RAND_RESET_I(9);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__TexEnvResultCalc__DOT__g = VL_RAND_RESET_I(9);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__TexEnvResultCalc__DOT__b = VL_RAND_RESET_I(9);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__TexEnvResultCalc__DOT__a = VL_RAND_RESET_I(9);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__BlendCalc__DOT__rs = VL_RAND_RESET_I(4);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__BlendCalc__DOT__gs = VL_RAND_RESET_I(4);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__BlendCalc__DOT__bs = VL_RAND_RESET_I(4);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__BlendCalc__DOT__as = VL_RAND_RESET_I(4);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__BlendCalc__DOT__rd = VL_RAND_RESET_I(4);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__BlendCalc__DOT__gd = VL_RAND_RESET_I(4);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__BlendCalc__DOT__bd = VL_RAND_RESET_I(4);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__BlendCalc__DOT__ad = VL_RAND_RESET_I(4);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__BlendCalc__DOT__v00 = VL_RAND_RESET_I(4);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__BlendCalc__DOT__v01 = VL_RAND_RESET_I(4);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__BlendCalc__DOT__v02 = VL_RAND_RESET_I(4);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__BlendCalc__DOT__v03 = VL_RAND_RESET_I(4);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__BlendCalc__DOT__v10 = VL_RAND_RESET_I(4);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__BlendCalc__DOT__v11 = VL_RAND_RESET_I(4);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__BlendCalc__DOT__v12 = VL_RAND_RESET_I(4);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__BlendCalc__DOT__v13 = VL_RAND_RESET_I(4);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__BlendResultCalc__DOT__r = VL_RAND_RESET_I(9);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__BlendResultCalc__DOT__g = VL_RAND_RESET_I(9);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__BlendResultCalc__DOT__b = VL_RAND_RESET_I(9);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__BlendResultCalc__DOT__a = VL_RAND_RESET_I(9);
    { int __Vi0=0; for (; __Vi0<2048; ++__Vi0) {
            top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__recip__DOT__lutB[__Vi0] = VL_RAND_RESET_I(16);
    }}
    { int __Vi0=0; for (; __Vi0<2048; ++__Vi0) {
            top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__recip__DOT__lutM[__Vi0] = VL_RAND_RESET_I(16);
    }}
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__recip__DOT__j = VL_RAND_RESET_I(32);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__recip__DOT__nextB = VL_RAND_RESET_I(32);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__recip__DOT__currentB = VL_RAND_RESET_I(32);
    top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__recip__DOT__deltaB = VL_RAND_RESET_I(32);
    __Vfunc_top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__truncate16__0__Vfuncout = VL_RAND_RESET_I(16);
    __Vfunc_top__DOT__rasteriCEr__DOT__fragmentPipeline__DOT__truncate16__0__in = VL_RAND_RESET_I(32);
}

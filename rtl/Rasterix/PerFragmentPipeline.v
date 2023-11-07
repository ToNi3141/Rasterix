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

// This pipeline is part of the pixel pipeline and handles the fragment buffer part of the pipeline
// It gets a fully calculated texel from the previous step, reads from the color and depth buffer,
// executes alpha and depth tests, and blends the texel into the current fragment
// Pipelined: yes
// Depth: 4 cycles
module PerFragmentPipeline
#(
    // The minimum bit width which is required to contain the resolution
    parameter FRAMEBUFFER_INDEX_WIDTH = 14,
    parameter SCREEN_POS_WIDTH = 16,

    parameter DEPTH_WIDTH = 16,

    parameter STENCIL_WIDTH = 4,

    parameter SUB_PIXEL_WIDTH = 8,
    localparam NUMBER_OF_SUB_PIXELS = 4,
    localparam PIXEL_WIDTH = NUMBER_OF_SUB_PIXELS * SUB_PIXEL_WIDTH,

    localparam FLOAT_SIZE = 32,

    localparam KEEP_WIDTH = 1
)
(
    input  wire                                     aclk,
    input  wire                                     resetn,

    // Shader configurations
    input  wire [31 : 0]                            conf,
    input  wire [31 : 0]                            confFeatureEnable,
    input  wire [31 : 0]                            confStencilBufferConfig,

    // Fragment input
    output wire                                     s_frag_tready,
    input  wire                                     s_frag_tlast,
    input  wire [KEEP_WIDTH - 1 : 0]                s_frag_tkeep,
    input  wire                                     s_frag_tvalid,
    input  wire [PIXEL_WIDTH - 1 : 0]               s_frag_tcolor,
    input  wire [31 : 0]                            s_frag_tdepth,
    input  wire [FRAMEBUFFER_INDEX_WIDTH - 1 : 0]   s_frag_tindex,
    input  wire [SCREEN_POS_WIDTH - 1 : 0]          s_frag_tscreenPosX,
    input  wire [SCREEN_POS_WIDTH - 1 : 0]          s_frag_tscreenPosY,

    // Signals when the fragment wents through the pipeline
    output reg                                      fragmentProcessed,

    // Frame buffer access
    // Read
    output wire                                     s_color_rready,
    input  wire                                     s_color_rvalid,
    input  wire [PIXEL_WIDTH - 1 : 0]               s_color_rdata,
    // Write
    output reg  [FRAMEBUFFER_INDEX_WIDTH - 1 : 0]   m_color_waddr,
    output reg                                      m_color_wvalid,
    output reg                                      m_color_wlast,
    output reg  [PIXEL_WIDTH - 1 : 0]               m_color_wdata,
    output reg                                      m_color_wstrb,
    output reg  [SCREEN_POS_WIDTH - 1 : 0]          m_color_wscreenPosX,
    output reg  [SCREEN_POS_WIDTH - 1 : 0]          m_color_wscreenPosY,

    // ZBuffer buffer access
    // Read
    output wire                                     s_depth_rready,
    input  wire                                     s_depth_rvalid,
    input  wire [DEPTH_WIDTH - 1 : 0]               s_depth_rdata,
    // Write
    output reg  [FRAMEBUFFER_INDEX_WIDTH - 1 : 0]   m_depth_waddr,
    output reg                                      m_depth_wvalid,
    output reg                                      m_depth_wlast,
    output reg  [DEPTH_WIDTH - 1 : 0]               m_depth_wdata,
    output reg                                      m_depth_wstrb,
    output reg  [SCREEN_POS_WIDTH - 1 : 0]          m_depth_wscreenPosX,
    output reg  [SCREEN_POS_WIDTH - 1 : 0]          m_depth_wscreenPosY,

    // ZBuffer buffer access
    // Read
    output wire                                     s_stencil_rready,
    input  wire                                     s_stencil_rvalid,
    input  wire [STENCIL_WIDTH - 1 : 0]             s_stencil_rdata,
    // Write
    output reg  [FRAMEBUFFER_INDEX_WIDTH - 1 : 0]   m_stencil_waddr,
    output reg                                      m_stencil_wvalid,
    output reg                                      m_stencil_wlast,
    output reg  [STENCIL_WIDTH - 1 : 0]             m_stencil_wdata,
    output reg                                      m_stencil_wstrb,
    output reg  [SCREEN_POS_WIDTH - 1 : 0]          m_stencil_wscreenPosX,
    output reg  [SCREEN_POS_WIDTH - 1 : 0]          m_stencil_wscreenPosY
);
`include "RegisterAndDescriptorDefines.vh"

    function [DEPTH_WIDTH - 1 : 0] clampDepth;
        input [31 : 0] depth;
        begin
            if (depth[31]) // If z is negative
            begin
                clampDepth = 0;
            end
            else if (|depth[32 - DEPTH_WIDTH +: DEPTH_WIDTH]) // If z is greater than 1.0
            begin
                clampDepth = {DEPTH_WIDTH{1'b1}};
            end
            else // If z is between 0.0 and 1.0
            begin
                clampDepth = depth[0 +: DEPTH_WIDTH];
            end
        end
    endfunction

    wire rready_sig = s_frag_tvalid & s_color_rvalid & s_depth_rvalid & s_stencil_rvalid;

    assign s_color_rready = rready_sig;
    assign s_depth_rready = rready_sig;
    assign s_stencil_rready = rready_sig;
    assign s_frag_tready = rready_sig;

    ////////////////////////////////////////////////////////////////////////////
    // STEP 0
    // Read from framebuffer
    // Clocks: 0
    ////////////////////////////////////////////////////////////////////////////
    wire                                    step0_valid = rready_sig;
    wire                                    step0_last = s_frag_tlast;
    wire [KEEP_WIDTH  - 1 : 0]              step0_keep = s_frag_tkeep;
    wire [FRAMEBUFFER_INDEX_WIDTH - 1 : 0]  step0_index = s_frag_tindex;
    wire [DEPTH_WIDTH - 1 : 0]              step0_depth = clampDepth(s_frag_tdepth);
    wire [SCREEN_POS_WIDTH - 1 : 0]         step0_screenPosX = s_frag_tscreenPosX;
    wire [SCREEN_POS_WIDTH - 1 : 0]         step0_screenPosY = s_frag_tscreenPosY;
    wire [PIXEL_WIDTH - 1 : 0]              step0_fragmentColor = s_frag_tcolor;

    ////////////////////////////////////////////////////////////////////////////
    // STEP 1
    // Blend color and test functions
    // Clocks: 3 
    ////////////////////////////////////////////////////////////////////////////
    wire                                    step1_valid;
    wire                                    step1_last;
    wire [KEEP_WIDTH  - 1 : 0]              step1_tkeep;
    wire [FRAMEBUFFER_INDEX_WIDTH - 1 : 0]  step1_index;
    wire [SCREEN_POS_WIDTH - 1 : 0]         step1_screenPosX;
    wire [SCREEN_POS_WIDTH - 1 : 0]         step1_screenPosY;
    wire [DEPTH_WIDTH - 1 : 0]              step1_depth;
    wire [PIXEL_WIDTH - 1 : 0]              step1_fragmentColor;
    wire                                    step1_writeFramebuffer;
    wire [STENCIL_WIDTH - 1 : 0 ]           step1_stencil;
    wire                                    step1_writeStencilBuffer;

    ValueDelay #(.VALUE_SIZE(1), .DELAY(3)) 
        step1_validDelay (.clk(aclk), .in(step0_valid), .out(step1_valid));
    ValueDelay #(.VALUE_SIZE(1), .DELAY(3)) 
        step1_lastDelay (.clk(aclk), .in(step0_last), .out(step1_last));
    ValueDelay #(.VALUE_SIZE(KEEP_WIDTH), .DELAY(3)) 
        step1_keepDelay (.clk(aclk), .in(step0_keep), .out(step1_keep));
    ValueDelay #(.VALUE_SIZE(FRAMEBUFFER_INDEX_WIDTH), .DELAY(3)) 
        step1_indexDelay (.clk(aclk), .in(step0_index), .out(step1_index));
    ValueDelay #(.VALUE_SIZE(SCREEN_POS_WIDTH), .DELAY(3)) 
        step1_screenPosXDelay (.clk(aclk), .in(step0_screenPosX), .out(step1_screenPosX));
    ValueDelay #(.VALUE_SIZE(SCREEN_POS_WIDTH), .DELAY(3)) 
        step1_screenPosYDelay (.clk(aclk), .in(step0_screenPosY), .out(step1_screenPosY));
    ValueDelay #(.VALUE_SIZE(DEPTH_WIDTH), .DELAY(3)) 
        step1_depthDelay (.clk(aclk), .in(step0_depth), .out(step1_depth)); 

    ColorBlender #(
        .SUB_PIXEL_WIDTH(SUB_PIXEL_WIDTH)
    ) colorBlender (
        .aclk(aclk),
        .resetn(resetn),

        .funcSFactor(conf[RENDER_CONFIG_FRAGMENT_BLEND_FUNC_SFACTOR_POS +: RENDER_CONFIG_FRAGMENT_BLEND_FUNC_SFACTOR_SIZE]),
        .funcDFactor(conf[RENDER_CONFIG_FRAGMENT_BLEND_FUNC_DFACTOR_POS +: RENDER_CONFIG_FRAGMENT_BLEND_FUNC_DFACTOR_SIZE]),
        .confEnable(confFeatureEnable[RENDER_CONFIG_FEATURE_ENABLE_BLENDING_POS]),
        .sourceColor(step0_fragmentColor),
        .destColor(s_color_rdata),

        .color(step1_fragmentColor)
    );

    // Clocks: 1
    wire step1_alphaTestTmp;
    TestFunc #(
        .SUB_PIXEL_WIDTH(SUB_PIXEL_WIDTH)
    ) alphaTest (
        .aclk(aclk),
        .resetn(resetn),
        .func(conf[RENDER_CONFIG_FRAGMENT_ALPHA_TEST_FUNC_POS +: RENDER_CONFIG_FRAGMENT_ALPHA_TEST_FUNC_SIZE]),
        .confEnable(confFeatureEnable[RENDER_CONFIG_FEATURE_ENABLE_ALPHA_TEST_POS]),
        .refVal(conf[RENDER_CONFIG_FRAGMENT_ALPHA_TEST_REF_VALUE_POS +: RENDER_CONFIG_FRAGMENT_ALPHA_TEST_REF_VALUE_SIZE]),
        .val(step0_fragmentColor[COLOR_A_POS +: SUB_PIXEL_WIDTH]),
        .success(step1_alphaTestTmp)
    );

    // Clocks: 1
    wire step1_depthTestTmp;
    TestFunc #(
        .SUB_PIXEL_WIDTH(DEPTH_WIDTH)
    ) depthTest (
        .aclk(aclk),
        .resetn(resetn),
        .func(conf[RENDER_CONFIG_FRAGMENT_DEPTH_TEST_FUNC_POS +: RENDER_CONFIG_FRAGMENT_DEPTH_TEST_FUNC_SIZE]),
        .confEnable(confFeatureEnable[RENDER_CONFIG_FEATURE_ENABLE_DEPTH_TEST_POS]),
        .refVal(s_depth_rdata),
        .val(step0_depth),
        .success(step1_depthTestTmp)
    );

    // Clocks: 1
    wire step1_stencilTestTmp;
    wire [STENCIL_WIDTH - 1 : 0] step1_stencilBufferValTmp;
    wire [STENCIL_WIDTH - 1 : 0] step1_stencilRefValTmp = confStencilBufferConfig[RENDER_CONFIG_STENCIL_BUFFER_REF_POS +: RENDER_CONFIG_STENCIL_BUFFER_REF_SIZE - (STENCIL_WIDTH - RENDER_CONFIG_STENCIL_BUFFER_REF_SIZE)];
    wire [STENCIL_WIDTH - 1 : 0] step1_stencilMaskTmp = confStencilBufferConfig[RENDER_CONFIG_STENCIL_BUFFER_MASK_POS +: RENDER_CONFIG_STENCIL_BUFFER_MASK_SIZE - (STENCIL_WIDTH - RENDER_CONFIG_STENCIL_BUFFER_MASK_SIZE)];
    ValueDelay #(.VALUE_SIZE(STENCIL_WIDTH), .DELAY(1)) 
        step1_stencilBufferDelay (.clk(aclk), .in(s_stencil_rdata), .out(step1_stencilBufferValTmp)); 
    TestFunc #(
        .SUB_PIXEL_WIDTH(STENCIL_WIDTH)
    ) stencilTest (
        .aclk(aclk),
        .resetn(resetn),
        .func(confStencilBufferConfig[RENDER_CONFIG_STENCIL_BUFFER_FUNC_POS +: RENDER_CONFIG_STENCIL_BUFFER_FUNC_SIZE]),
        .confEnable(confFeatureEnable[RENDER_CONFIG_FEATURE_ENABLE_STENCIL_TEST_POS]),
        .refVal(s_stencil_rdata & step1_stencilMaskTmp),
        .val(step1_stencilRefValTmp & step1_stencilMaskTmp),
        .success(step1_stencilTestTmp)
    );

    // Clocks: 1
    wire                            step1_writeStencilBufferTmp;
    wire [STENCIL_WIDTH - 1 : 0 ]   step1_stencilTmp;
    StencilOp #(
        .STENCIL_WIDTH(STENCIL_WIDTH)
    ) stencilOp (
        .aclk(aclk),
        .resetn(resetn),
        .opZFail(confStencilBufferConfig[RENDER_CONFIG_STENCIL_BUFFER_OP_ZFAIL_POS +: RENDER_CONFIG_STENCIL_BUFFER_OP_ZFAIL_SIZE]),
        .opZPass(confStencilBufferConfig[RENDER_CONFIG_STENCIL_BUFFER_OP_ZPASS_POS +: RENDER_CONFIG_STENCIL_BUFFER_OP_ZPASS_SIZE]),
        .opFail(confStencilBufferConfig[RENDER_CONFIG_STENCIL_BUFFER_OP_FAIL_POS +: RENDER_CONFIG_STENCIL_BUFFER_OP_FAIL_SIZE]),
        .refVal(confStencilBufferConfig[RENDER_CONFIG_STENCIL_BUFFER_REF_POS +: RENDER_CONFIG_STENCIL_BUFFER_REF_SIZE - (STENCIL_WIDTH - RENDER_CONFIG_STENCIL_BUFFER_REF_SIZE)]),
        .enable(confFeatureEnable[RENDER_CONFIG_FEATURE_ENABLE_STENCIL_TEST_POS]),
        .zTest(step1_depthTestTmp),
        .sTest(step1_stencilTestTmp),
        .sValIn(step1_stencilBufferValTmp),
        .sValOut(step1_stencilTmp),
        .write(step1_writeStencilBufferTmp)
    );

    ValueDelay #(.VALUE_SIZE(STENCIL_WIDTH), .DELAY(1)) 
        step1_stencilDelay (.clk(aclk), .in(step1_stencilTmp), .out(step1_stencil));

    ValueDelay #(.VALUE_SIZE(1), .DELAY(1)) 
        step1_writeStencilDelay (.clk(aclk), .in(step1_writeStencilBufferTmp), .out(step1_writeStencilBuffer));

    ValueDelay #(.VALUE_SIZE(1), .DELAY(2)) 
        step1_writeFramebufferDelay (.clk(aclk), .in(step1_depthTestTmp & step1_alphaTestTmp & step1_stencilTestTmp), .out(step1_writeFramebuffer));

    ////////////////////////////////////////////////////////////////////////////
    // STEP 2
    // Write back
    // Clocks: 1 
    ////////////////////////////////////////////////////////////////////////////
    always @(posedge aclk)
    begin
        if (step1_valid)
        begin
            m_color_waddr <= step1_index;
            m_color_wscreenPosX <= step1_screenPosX;
            m_color_wscreenPosY <= step1_screenPosY;
            m_depth_waddr <= step1_index;
            m_depth_wscreenPosX <= step1_screenPosX;
            m_depth_wscreenPosY <= step1_screenPosY;
            m_stencil_waddr <= step1_index;
            m_stencil_wscreenPosX <= step1_screenPosX;
            m_stencil_wscreenPosY <= step1_screenPosY;
            m_depth_wdata <= step1_depth;
            m_color_wdata <= step1_fragmentColor;
            m_stencil_wdata <= step1_stencil;
        end
        fragmentProcessed <= step1_valid;
        m_color_wvalid <= step1_valid;
        m_depth_wvalid <= step1_valid;
        m_stencil_wvalid <= step1_valid;
        m_color_wlast <= step1_last;
        m_depth_wlast <= step1_last;
        m_stencil_wlast <= step1_last;
        m_color_wstrb <= step1_keep & step1_writeFramebuffer;
        m_depth_wstrb <= step1_keep & step1_writeFramebuffer && confFeatureEnable[RENDER_CONFIG_FEATURE_ENABLE_DEPTH_TEST_POS];
        m_stencil_wstrb <= step1_keep & step1_writeStencilBuffer && confFeatureEnable[RENDER_CONFIG_FEATURE_ENABLE_STENCIL_TEST_POS];
    end
endmodule


 
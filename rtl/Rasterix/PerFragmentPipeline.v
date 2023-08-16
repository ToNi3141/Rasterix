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
// Depth: 5 cycles
module PerFragmentPipeline
#(
    // The minimum bit width which is required to contain the resolution
    parameter FRAMEBUFFER_INDEX_WIDTH = 14,
    parameter SCREEN_POS_WIDTH = 16,

    parameter DEPTH_WIDTH = 16,

    parameter STENCIL_WIDTH = 4,

    parameter SUB_PIXEL_WIDTH = 8,
    localparam PIXEL_WIDTH = 4 * SUB_PIXEL_WIDTH,

    localparam FLOAT_SIZE = 32
)
(
    input  wire                                     aclk,
    input  wire                                     resetn,

    // Shader configurations
    input  wire [31 : 0]                            conf,
    input  wire [31 : 0]                            confFeatureEnable,
    input  wire [31 : 0]                            confStencilBufferConfig,

    // Fragment input
    input  wire                                     valid,
    input  wire [PIXEL_WIDTH - 1 : 0]               fragmentColor,
    input  wire [31 : 0]                            depth,
    input  wire [FRAMEBUFFER_INDEX_WIDTH - 1 : 0]   index,
    input  wire [SCREEN_POS_WIDTH - 1 : 0]          screenPosX,
    input  wire [SCREEN_POS_WIDTH - 1 : 0]          screenPosY,

    // Signals when the fragment wents through the pipeline
    output reg                                      fragmentProcessed,

    // Frame buffer access
    // Read
    output wire                                     color_arvalid,
    output wire                                     color_arlast,
    output wire [FRAMEBUFFER_INDEX_WIDTH - 1 : 0]   color_araddr,
    input  wire                                     color_rvalid,
    input  wire                                     color_rlast,
    input  wire [PIXEL_WIDTH - 1 : 0]               color_rdata,
    // Write
    output reg  [FRAMEBUFFER_INDEX_WIDTH - 1 : 0]   color_waddr,
    output reg                                      color_wvalid,
    output reg  [PIXEL_WIDTH - 1 : 0]               color_wdata,
    output reg  [SCREEN_POS_WIDTH - 1 : 0]          color_wscreenPosX,
    output reg  [SCREEN_POS_WIDTH - 1 : 0]          color_wscreenPosY,

    // ZBuffer buffer access
    // Read
    output wire                                     depth_arvalid,
    output wire                                     depth_arlast,
    output wire [FRAMEBUFFER_INDEX_WIDTH - 1 : 0]   depth_araddr,
    input  wire                                     depth_rvalid,
    input  wire                                     depth_rlast,
    input  wire [DEPTH_WIDTH - 1 : 0]               depth_rdata,
    // Write
    output reg  [FRAMEBUFFER_INDEX_WIDTH - 1 : 0]   depth_waddr,
    output reg                                      depth_wvalid,
    output reg  [DEPTH_WIDTH - 1 : 0]               depth_wdata,
    output reg  [SCREEN_POS_WIDTH - 1 : 0]          depth_wscreenPosX,
    output reg  [SCREEN_POS_WIDTH - 1 : 0]          depth_wscreenPosY,

    // ZBuffer buffer access
    // Read
    output wire                                     stencil_arvalid,
    output wire                                     stencil_arlast,
    output wire [FRAMEBUFFER_INDEX_WIDTH - 1 : 0]   stencil_araddr,
    input  wire                                     stencil_rvalid,
    input  wire                                     stencil_rlast,
    input  wire [STENCIL_WIDTH - 1 : 0]             stencil_rdata,
    // Write
    output reg  [FRAMEBUFFER_INDEX_WIDTH - 1 : 0]   stencil_waddr,
    output reg                                      stencil_wvalid,
    output reg  [STENCIL_WIDTH - 1 : 0]             stencil_wdata,
    output reg  [SCREEN_POS_WIDTH - 1 : 0]          stencil_wscreenPosX,
    output reg  [SCREEN_POS_WIDTH - 1 : 0]          stencil_wscreenPosY
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

    assign color_arvalid = 1;
    assign color_arlast = 0;
    assign depth_arvalid = 1;
    assign depth_arlast = 0;
    assign stencil_arvalid = 1;
    assign stencil_arlast = 0;

    ////////////////////////////////////////////////////////////////////////////
    // STEP 0
    // Read from framebuffer
    // Clocks: 2 (delay of the framebuffer read)
    ////////////////////////////////////////////////////////////////////////////
    wire                                    step0_valid;
    wire [FRAMEBUFFER_INDEX_WIDTH - 1 : 0]  step0_index;
    wire [DEPTH_WIDTH - 1 : 0]              step0_depth;
    wire [SCREEN_POS_WIDTH - 1 : 0]         step0_screenPosX;
    wire [SCREEN_POS_WIDTH - 1 : 0]         step0_screenPosY;
    wire [PIXEL_WIDTH - 1 : 0]              step0_fragmentColor;

    ValueDelay #(.VALUE_SIZE(1), .DELAY(2)) 
        step0_validDelay (.clk(aclk), .in(valid), .out(step0_valid));
    ValueDelay #(.VALUE_SIZE(FRAMEBUFFER_INDEX_WIDTH), .DELAY(2)) 
        step0_indexDelay (.clk(aclk), .in(index), .out(step0_index));
    ValueDelay #(.VALUE_SIZE(SCREEN_POS_WIDTH), .DELAY(2)) 
        step0_screenPosXDelay (.clk(aclk), .in(screenPosX), .out(step0_screenPosX));
    ValueDelay #(.VALUE_SIZE(SCREEN_POS_WIDTH), .DELAY(2)) 
        step0_screenPosYDelay (.clk(aclk), .in(screenPosY), .out(step0_screenPosY));
    ValueDelay #(.VALUE_SIZE(DEPTH_WIDTH), .DELAY(2)) 
        step0_depthDelay (.clk(aclk), .in(clampDepth(depth)), .out(step0_depth));
    ValueDelay #(.VALUE_SIZE(PIXEL_WIDTH), .DELAY(2)) 
        step0_fragmentColorDelay (.clk(aclk), .in(fragmentColor), .out(step0_fragmentColor));

    assign color_araddr = index;
    assign depth_araddr = index;
    assign stencil_araddr = index;

    ////////////////////////////////////////////////////////////////////////////
    // STEP 1
    // Blend color and test functions
    // Clocks: 2 
    ////////////////////////////////////////////////////////////////////////////
    wire                                    step1_valid;
    wire [FRAMEBUFFER_INDEX_WIDTH - 1 : 0]  step1_index;
    wire [SCREEN_POS_WIDTH - 1 : 0]         step1_screenPosX;
    wire [SCREEN_POS_WIDTH - 1 : 0]         step1_screenPosY;
    wire [DEPTH_WIDTH - 1 : 0]              step1_depth;
    wire [PIXEL_WIDTH - 1 : 0]              step1_fragmentColor;
    reg                                     step1_writeFramebuffer;
    wire [STENCIL_WIDTH - 1 : 0 ]           step1_stencil;
    wire                                    step1_writeStencilBuffer;

    ValueDelay #(.VALUE_SIZE(1), .DELAY(2)) 
        step1_validDelay (.clk(aclk), .in(step0_valid), .out(step1_valid));
    ValueDelay #(.VALUE_SIZE(FRAMEBUFFER_INDEX_WIDTH), .DELAY(2)) 
        step1_indexDelay (.clk(aclk), .in(step0_index), .out(step1_index));
    ValueDelay #(.VALUE_SIZE(SCREEN_POS_WIDTH), .DELAY(2)) 
        step1_screenPosXDelay (.clk(aclk), .in(step0_screenPosX), .out(step1_screenPosX));
    ValueDelay #(.VALUE_SIZE(SCREEN_POS_WIDTH), .DELAY(2)) 
        step1_screenPosYDelay (.clk(aclk), .in(step0_screenPosY), .out(step1_screenPosY));
    ValueDelay #(.VALUE_SIZE(DEPTH_WIDTH), .DELAY(2)) 
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
        .destColor(color_rdata),

        .color(step1_fragmentColor)
    );

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

    wire step1_depthTestTmp;
    TestFunc #(
        .SUB_PIXEL_WIDTH(DEPTH_WIDTH)
    ) depthTest (
        .aclk(aclk),
        .resetn(resetn),
        .func(conf[RENDER_CONFIG_FRAGMENT_DEPTH_TEST_FUNC_POS +: RENDER_CONFIG_FRAGMENT_DEPTH_TEST_FUNC_SIZE]),
        .confEnable(confFeatureEnable[RENDER_CONFIG_FEATURE_ENABLE_DEPTH_TEST_POS]),
        .refVal(depth_rdata),
        .val(step0_depth),
        .success(step1_depthTestTmp)
    );

    wire step1_stencilTestTmp;
    wire [STENCIL_WIDTH - 1 : 0] step1_stencilBufferValTmp;
    wire [STENCIL_WIDTH - 1 : 0] step1_stencilRefValTmp = confStencilBufferConfig[RENDER_CONFIG_STENCIL_BUFFER_REF_POS +: RENDER_CONFIG_STENCIL_BUFFER_REF_SIZE - (STENCIL_WIDTH - RENDER_CONFIG_STENCIL_BUFFER_REF_SIZE)];
    wire [STENCIL_WIDTH - 1 : 0] step1_stencilMaskTmp = confStencilBufferConfig[RENDER_CONFIG_STENCIL_BUFFER_MASK_POS +: RENDER_CONFIG_STENCIL_BUFFER_MASK_SIZE - (STENCIL_WIDTH - RENDER_CONFIG_STENCIL_BUFFER_MASK_SIZE)];
    ValueDelay #(.VALUE_SIZE(STENCIL_WIDTH), .DELAY(1)) 
        step1_stencilBufferDelay (.clk(aclk), .in(stencil_rdata), .out(step1_stencilBufferValTmp)); 
    TestFunc #(
        .SUB_PIXEL_WIDTH(STENCIL_WIDTH)
    ) stencilTest (
        .aclk(aclk),
        .resetn(resetn),
        .func(confStencilBufferConfig[RENDER_CONFIG_STENCIL_BUFFER_FUNC_POS +: RENDER_CONFIG_STENCIL_BUFFER_FUNC_SIZE]),
        .confEnable(confFeatureEnable[RENDER_CONFIG_FEATURE_ENABLE_STENCIL_TEST_POS]),
        .refVal(stencil_rdata & step1_stencilMaskTmp),
        .val(step1_stencilRefValTmp & step1_stencilMaskTmp),
        .success(step1_stencilTestTmp)
    );

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
        .sValOut(step1_stencil),
        .write(step1_writeStencilBuffer)
    );

    always @(posedge aclk)
    begin
        step1_writeFramebuffer <= step1_depthTestTmp & step1_alphaTestTmp & step1_stencilTestTmp;
    end

    ////////////////////////////////////////////////////////////////////////////
    // STEP 2
    // Write back
    // Clocks: 1 
    ////////////////////////////////////////////////////////////////////////////
    always @(posedge aclk)
    begin
        if (step1_valid)
        begin
            color_waddr <= step1_index;
            color_wscreenPosX <= step1_screenPosX;
            color_wscreenPosY <= step1_screenPosY;
            depth_waddr <= step1_index;
            depth_wscreenPosX <= step1_screenPosX;
            depth_wscreenPosY <= step1_screenPosY;
            stencil_waddr <= step1_index;
            stencil_wscreenPosX <= step1_screenPosX;
            stencil_wscreenPosY <= step1_screenPosY;
            depth_wdata <= step1_depth;
            color_wdata <= step1_fragmentColor;
            stencil_wdata <= step1_stencil;
        end
        fragmentProcessed <= step1_valid;
        color_wvalid <= step1_valid & step1_writeFramebuffer;
        depth_wvalid <= step1_valid & step1_writeFramebuffer & confFeatureEnable[RENDER_CONFIG_FEATURE_ENABLE_DEPTH_TEST_POS];
        stencil_wvalid <= step1_valid & step1_writeStencilBuffer & confFeatureEnable[RENDER_CONFIG_FEATURE_ENABLE_STENCIL_TEST_POS];
    end
endmodule


 
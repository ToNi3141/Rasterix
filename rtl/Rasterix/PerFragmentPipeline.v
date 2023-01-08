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

    parameter SUB_PIXEL_WIDTH = 8,
    localparam PIXEL_WIDTH = 4 * SUB_PIXEL_WIDTH,

    localparam FLOAT_SIZE = 32
)
(
    input  wire                         aclk,
    input  wire                         resetn,

    // Shader configurations
    input  wire [31 : 0]                conf,
    input  wire [31 : 0]                confFeatureEnable,

    // Fragment input
    input  wire                         valid,
    input  wire [PIXEL_WIDTH - 1 : 0]   fragmentColor,
    input  wire [31 : 0]                depth,
    input  wire [FRAMEBUFFER_INDEX_WIDTH - 1 : 0] index,
    input  wire [SCREEN_POS_WIDTH - 1 : 0]  screenPosX,
    input  wire [SCREEN_POS_WIDTH - 1 : 0]  screenPosY,

    // Signals when the fragment wents through the pipeline
    output reg                          fragmentProcessed,

    // Frame buffer access
    // Read
    output wire [FRAMEBUFFER_INDEX_WIDTH - 1 : 0] colorIndexRead,
    input  wire [PIXEL_WIDTH - 1 : 0]   colorIn,
    // Write
    output reg  [FRAMEBUFFER_INDEX_WIDTH - 1 : 0] colorIndexWrite,
    output reg                          colorWriteEnable,
    output reg  [PIXEL_WIDTH - 1 : 0]   colorOut,
    output reg  [SCREEN_POS_WIDTH - 1 : 0]  colorOutScreenPosX,
    output reg  [SCREEN_POS_WIDTH - 1 : 0]  colorOutScreenPosY,

    // ZBuffer buffer access
    // Read
    output wire [FRAMEBUFFER_INDEX_WIDTH - 1 : 0] depthIndexRead,
    input  wire [DEPTH_WIDTH - 1 : 0]   depthIn,
    // Write
    output reg  [FRAMEBUFFER_INDEX_WIDTH - 1 : 0] depthIndexWrite,
    output reg                          depthWriteEnable,
    output reg  [DEPTH_WIDTH - 1 : 0]   depthOut,
    output reg  [SCREEN_POS_WIDTH - 1 : 0]  depthOutScreenPosX,
    output reg  [SCREEN_POS_WIDTH - 1 : 0]  depthOutScreenPosY
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

    assign colorIndexRead = index;
    assign depthIndexRead = index;

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
        .destColor(colorIn),

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
        .refVal(depthIn),
        .val(step0_depth),
        .success(step1_depthTestTmp)
    );

    always @(posedge aclk)
    begin
        step1_writeFramebuffer <= step1_depthTestTmp & step1_alphaTestTmp; // Check that both test passing
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
            colorIndexWrite <= step1_index;
            colorOutScreenPosX <= step1_screenPosX;
            colorOutScreenPosY <= step1_screenPosY;
            depthIndexWrite <= step1_index;
            depthOutScreenPosX <= step1_screenPosX;
            depthOutScreenPosY <= step1_screenPosY;
            depthOut <= step1_depth;
            colorOut <= step1_fragmentColor;
        end
        fragmentProcessed <= step1_valid;
        colorWriteEnable <= step1_valid & step1_writeFramebuffer;
        depthWriteEnable <= step1_valid & step1_writeFramebuffer & confFeatureEnable[RENDER_CONFIG_FEATURE_ENABLE_DEPTH_TEST_POS];
    end
endmodule


 
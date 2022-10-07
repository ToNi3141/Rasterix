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

// This module is used to calculate a whole pixel, starting from the
// meta data from the rasterizer till it is written into the frame buffer,
// including, blending, fogging, texenv, testing and so on.
// It prepares the meta data (basically converts the floats from the)
// interpolator into fixed point numbers, which can be used from the 
// fragment and framebuffer pipeline.
// Pipelined: yes
// Depth: 26 cycles
module PixelPipeline
#(
    parameter CMD_STREAM_WIDTH = 64,

    // The minimum bit width which is required to contain the resolution
    parameter FRAMEBUFFER_INDEX_WIDTH = 14,

    localparam DEPTH_WIDTH = 16,

    parameter SUB_PIXEL_WIDTH = 8,
    localparam PIXEL_WIDTH = 4 * SUB_PIXEL_WIDTH,

    localparam FLOAT_SIZE = 32,

    localparam TEX_ADDR_WIDTH = 16 
)
(
    input  wire                         aclk,
    input  wire                         resetn,
    output wire                         pixelInPipeline,

    // Fog function LUT stream
    input  wire                         s_fog_lut_axis_tvalid,
    output wire                         s_fog_lut_axis_tready,
    input  wire                         s_fog_lut_axis_tlast,
    input  wire [CMD_STREAM_WIDTH - 1 : 0] s_fog_lut_axis_tdata,

    // Shader configurations
    input  wire [31 : 0]                confFeatureEnable,
    input  wire [31 : 0]                confFragmentPipelineConfig,
    input  wire [PIXEL_WIDTH - 1 : 0]   confFragmentPipelineFogColor,
    input  wire [31 : 0]                confTMU0TexEnvConfig,
    input  wire [31 : 0]                confTMU0TextureConfig,
    input  wire [PIXEL_WIDTH - 1 : 0]   confTMU0TexEnvColor,

    // Fragment Stream
    input  wire                         s_axis_tvalid,
    output wire                         s_axis_tready,
    input  wire                         s_axis_tlast,
    input  wire [ATTR_INTERP_AXIS_PARAMETER_SIZE - 1 : 0] s_axis_tdata,

    // Texture access
    // Texture memory access of a texel quad
    output wire [TEX_ADDR_WIDTH - 1 : 0]    texelAddr00,
    output wire [TEX_ADDR_WIDTH - 1 : 0]    texelAddr01,
    output wire [TEX_ADDR_WIDTH - 1 : 0]    texelAddr10,
    output wire [TEX_ADDR_WIDTH - 1 : 0]    texelAddr11,
    input  wire [PIXEL_WIDTH - 1 : 0]   texelInput00,
    input  wire [PIXEL_WIDTH - 1 : 0]   texelInput01,
    input  wire [PIXEL_WIDTH - 1 : 0]   texelInput10,
    input  wire [PIXEL_WIDTH - 1 : 0]   texelInput11,

    // Frame buffer access
    // Read
    output wire [FRAMEBUFFER_INDEX_WIDTH - 1 : 0] colorIndexRead,
    input  wire [PIXEL_WIDTH - 1 : 0]   colorIn,
    // Write
    output wire [FRAMEBUFFER_INDEX_WIDTH - 1 : 0] colorIndexWrite,
    output wire                         colorWriteEnable,
    output wire [PIXEL_WIDTH - 1 : 0]   colorOut,
    output wire [SCREEN_POS_WIDTH - 1 : 0]  colorOutScreenPosX,
    output wire [SCREEN_POS_WIDTH - 1 : 0]  colorOutScreenPosY,

    // ZBuffer buffer access
    // Read
    output wire [FRAMEBUFFER_INDEX_WIDTH - 1 : 0] depthIndexRead,
    input  wire [DEPTH_WIDTH - 1 : 0]   depthIn,
    // Write
    output wire [FRAMEBUFFER_INDEX_WIDTH - 1 : 0] depthIndexWrite,
    output wire                         depthWriteEnable,
    output wire [DEPTH_WIDTH - 1 : 0]   depthOut,
    output wire [SCREEN_POS_WIDTH - 1 : 0]  depthOutScreenPosX,
    output wire [SCREEN_POS_WIDTH - 1 : 0]  depthOutScreenPosY
);
`include "RegisterAndDescriptorDefines.vh"
`include "AttributeInterpolatorDefines.vh"
    localparam SCREEN_POS_WIDTH = ATTR_INTERP_AXIS_SCREEN_POS_SIZE;

    localparam [SUB_PIXEL_WIDTH - 1 : 0] ONE_POINT_ZERO = { SUB_PIXEL_WIDTH{1'h1} };
    localparam [(SUB_PIXEL_WIDTH * 2) - 1 : 0] ONE_POINT_ZERO_BIG = { { SUB_PIXEL_WIDTH{1'h0} }, ONE_POINT_ZERO };

    assign s_axis_tready = 1;

    wire fragmentProcessed;
    ValueTrack pixelTracker (
        .aclk(aclk),
        .resetn(resetn),
        
        .sigIncommingValue(s_axis_tvalid & s_axis_tready),
        .sigOutgoingValue(fragmentProcessed),
        .valueInPipeline(pixelInPipeline)
    );

    ////////////////////////////////////////////////////////////////////////////
    // STEP 0
    // Convert float to integer
    // Clocks: 4
    ////////////////////////////////////////////////////////////////////////////
    wire [ATTR_INTERP_AXIS_VERTEX_ATTRIBUTE_SIZE - 1 : 0]  step_convert_framebuffer_index;
    wire [SCREEN_POS_WIDTH - 1 : 0] step_convert_sreen_pos_x;
    wire [SCREEN_POS_WIDTH - 1 : 0] step_convert_sreen_pos_y;
    wire [FLOAT_SIZE - 1 : 0]   step_convert_depth_w_float;
    wire [31 : 0]               step_convert_texture_s;
    wire [31 : 0]               step_convert_texture_t;
    wire [31 : 0]               step_convert_depth_z;
    wire [31 : 0]               step_convert_color_r;
    wire [31 : 0]               step_convert_color_g;
    wire [31 : 0]               step_convert_color_b;
    wire [31 : 0]               step_convert_color_a;
    wire                        step_convert_tvalid;
    wire [31 : 0]               step_convert_w;

    ValueDelay #(.VALUE_SIZE(ATTR_INTERP_AXIS_VERTEX_ATTRIBUTE_SIZE), .DELAY(4)) 
        convert_framebuffer_delay (.clk(aclk), .in(s_axis_tdata[ATTR_INTERP_AXIS_FRAMEBUFFER_INDEX_POS +: ATTR_INTERP_AXIS_VERTEX_ATTRIBUTE_SIZE]), .out(step_convert_framebuffer_index));

    ValueDelay #(.VALUE_SIZE(SCREEN_POS_WIDTH), .DELAY(4)) 
        convert_screen_pos_x_delay (.clk(aclk), .in(s_axis_tdata[ATTR_INTERP_AXIS_SCREEN_XY_POS + ATTR_INTERP_AXIS_SCREEN_X_POS +: ATTR_INTERP_AXIS_SCREEN_POS_SIZE]), .out(step_convert_sreen_pos_x));
    ValueDelay #(.VALUE_SIZE(SCREEN_POS_WIDTH), .DELAY(4)) 
        convert_screen_pos_y_delay (.clk(aclk), .in(s_axis_tdata[ATTR_INTERP_AXIS_SCREEN_XY_POS + ATTR_INTERP_AXIS_SCREEN_Y_POS +: ATTR_INTERP_AXIS_SCREEN_POS_SIZE]), .out(step_convert_sreen_pos_y));

    ValueDelay #(.VALUE_SIZE(FLOAT_SIZE), .DELAY(4)) 
        convert_depth_delay (.clk(aclk), .in(s_axis_tdata[ATTR_INTERP_AXIS_DEPTH_W_POS + (ATTR_INTERP_AXIS_VERTEX_ATTRIBUTE_SIZE - FLOAT_SIZE) +: FLOAT_SIZE]), .out(step_convert_depth_w_float));
    ValueDelay #(.VALUE_SIZE(1), .DELAY(4)) 
        convert_valid_delay (.clk(aclk), .in(s_axis_tvalid), .out(step_convert_tvalid));

    FloatToInt #(.MANTISSA_SIZE(FLOAT_SIZE - 9), .EXPONENT_SIZE(8), .INT_SIZE(32), .EXPONENT_BIAS_OFFSET(-15))
        convert_floatToInt_TextureS (.clk(aclk), .in(s_axis_tdata[ATTR_INTERP_AXIS_TEXTURE_S_POS + (ATTR_INTERP_AXIS_VERTEX_ATTRIBUTE_SIZE - FLOAT_SIZE)+: FLOAT_SIZE]), .out(step_convert_texture_s));
    FloatToInt #(.MANTISSA_SIZE(FLOAT_SIZE - 9), .EXPONENT_SIZE(8), .INT_SIZE(32), .EXPONENT_BIAS_OFFSET(-15))
        convert_floatToInt_TextureT (.clk(aclk), .in(s_axis_tdata[ATTR_INTERP_AXIS_TEXTURE_T_POS + (ATTR_INTERP_AXIS_VERTEX_ATTRIBUTE_SIZE - FLOAT_SIZE) +: FLOAT_SIZE]), .out(step_convert_texture_t));   
    FloatToInt #(.MANTISSA_SIZE(FLOAT_SIZE - 9), .EXPONENT_SIZE(8), .INT_SIZE(32), .EXPONENT_BIAS_OFFSET(-7))
        convert_floatToInt_DepthW (.clk(aclk), .in(s_axis_tdata[ATTR_INTERP_AXIS_DEPTH_W_POS + (ATTR_INTERP_AXIS_VERTEX_ATTRIBUTE_SIZE - FLOAT_SIZE) +: FLOAT_SIZE]), .out(step_convert_w));  
    FloatToInt #(.MANTISSA_SIZE(FLOAT_SIZE - 9), .EXPONENT_SIZE(8), .INT_SIZE(32), .EXPONENT_BIAS_OFFSET(-DEPTH_WIDTH))
        convert_floatToInt_DepthZ (.clk(aclk), .in(s_axis_tdata[ATTR_INTERP_AXIS_DEPTH_Z_POS + (ATTR_INTERP_AXIS_VERTEX_ATTRIBUTE_SIZE - FLOAT_SIZE) +: FLOAT_SIZE]), .out(step_convert_depth_z));   
    FloatToInt #(.MANTISSA_SIZE(FLOAT_SIZE - 9), .EXPONENT_SIZE(8), .INT_SIZE(32), .EXPONENT_BIAS_OFFSET(-16))
        convert_floatToInt_ColorR (.clk(aclk), .in(s_axis_tdata[ATTR_INTERP_AXIS_COLOR_R_POS + (ATTR_INTERP_AXIS_VERTEX_ATTRIBUTE_SIZE - FLOAT_SIZE) +: FLOAT_SIZE]), .out(step_convert_color_r));
    FloatToInt #(.MANTISSA_SIZE(FLOAT_SIZE - 9), .EXPONENT_SIZE(8), .INT_SIZE(32), .EXPONENT_BIAS_OFFSET(-16))
        convert_floatToInt_ColorG (.clk(aclk), .in(s_axis_tdata[ATTR_INTERP_AXIS_COLOR_G_POS + (ATTR_INTERP_AXIS_VERTEX_ATTRIBUTE_SIZE - FLOAT_SIZE) +: FLOAT_SIZE]), .out(step_convert_color_g));   
    FloatToInt #(.MANTISSA_SIZE(FLOAT_SIZE - 9), .EXPONENT_SIZE(8), .INT_SIZE(32), .EXPONENT_BIAS_OFFSET(-16))
        convert_floatToInt_ColorB (.clk(aclk), .in(s_axis_tdata[ATTR_INTERP_AXIS_COLOR_B_POS + (ATTR_INTERP_AXIS_VERTEX_ATTRIBUTE_SIZE - FLOAT_SIZE) +: FLOAT_SIZE]), .out(step_convert_color_b));   
    FloatToInt #(.MANTISSA_SIZE(FLOAT_SIZE - 9), .EXPONENT_SIZE(8), .INT_SIZE(32), .EXPONENT_BIAS_OFFSET(-16))
        convert_floatToInt_ColorA (.clk(aclk), .in(s_axis_tdata[ATTR_INTERP_AXIS_COLOR_A_POS + (ATTR_INTERP_AXIS_VERTEX_ATTRIBUTE_SIZE - FLOAT_SIZE) +: FLOAT_SIZE]), .out(step_convert_color_a));   


    ////////////////////////////////////////////////////////////////////////////
    // STEP 1
    // Calculate fragment color
    // Clocks: 11
    ////////////////////////////////////////////////////////////////////////////
    wire [PIXEL_WIDTH - 1 : 0]              step1_fragmentColor;
    wire [FRAMEBUFFER_INDEX_WIDTH - 1 : 0]  step1_index;
    wire [SCREEN_POS_WIDTH - 1 : 0]         step1_screenPosX;
    wire [SCREEN_POS_WIDTH - 1 : 0]         step1_screenPosY;
    wire [31 : 0]                           step1_depth;
    wire [31 : 0]                           step1_depthWFloat;
    wire                                    step1_valid;
    wire [PIXEL_WIDTH - 1 : 0]              step1_primaryColor = {
        // clamp colors 
        (|step_convert_color_r[16 +: 16]) ? ONE_POINT_ZERO : step_convert_color_r[16 - SUB_PIXEL_WIDTH +: SUB_PIXEL_WIDTH],
        (|step_convert_color_g[16 +: 16]) ? ONE_POINT_ZERO : step_convert_color_g[16 - SUB_PIXEL_WIDTH +: SUB_PIXEL_WIDTH],
        (|step_convert_color_b[16 +: 16]) ? ONE_POINT_ZERO : step_convert_color_b[16 - SUB_PIXEL_WIDTH +: SUB_PIXEL_WIDTH],
        (|step_convert_color_a[16 +: 16]) ? ONE_POINT_ZERO : step_convert_color_a[16 - SUB_PIXEL_WIDTH +: SUB_PIXEL_WIDTH]
    };

    ValueDelay #(.VALUE_SIZE(FRAMEBUFFER_INDEX_WIDTH), .DELAY(11)) 
        step1_indexDelay (.clk(aclk), .in(step_convert_framebuffer_index[0 +: FRAMEBUFFER_INDEX_WIDTH]), .out(step1_index));

    ValueDelay #(.VALUE_SIZE(SCREEN_POS_WIDTH), .DELAY(11)) 
        step1_screenPosXDelay (.clk(aclk), .in(step_convert_sreen_pos_x), .out(step1_screenPosX));
    ValueDelay #(.VALUE_SIZE(SCREEN_POS_WIDTH), .DELAY(11)) 
        step1_screenPosYDelay (.clk(aclk), .in(step_convert_sreen_pos_y), .out(step1_screenPosY));

    ValueDelay #(.VALUE_SIZE(FLOAT_SIZE), .DELAY(11)) 
        step1_depthDelay (.clk(aclk), .in(step_convert_depth_z), .out(step1_depth));
    ValueDelay #(.VALUE_SIZE(FLOAT_SIZE), .DELAY(11)) 
        step1_depthWDelay (.clk(aclk), .in(step_convert_depth_w_float), .out(step1_depthWFloat));
    ValueDelay #(.VALUE_SIZE(1), .DELAY(11)) 
        step1_validDelay (.clk(aclk), .in(step_convert_tvalid), .out(step1_valid));

    TextureMappingUnit #(
        .CMD_STREAM_WIDTH(CMD_STREAM_WIDTH),
        .SUB_PIXEL_WIDTH(SUB_PIXEL_WIDTH)
    ) textureMappingUnitPipeline (
        .aclk(aclk),
        .resetn(resetn),

        .confFunc(confTMU0TexEnvConfig),
        .confTextureEnvColor(confTMU0TexEnvColor),
        .confTextureConfig(confTMU0TextureConfig),
        .confEnable(confFeatureEnable[RENDER_CONFIG_FEATURE_ENABLE_TMU0_POS]),

        .texelAddr00(texelAddr00),
        .texelAddr01(texelAddr01),
        .texelAddr10(texelAddr10),
        .texelAddr11(texelAddr11),

        .texelInput00(texelInput00),
        .texelInput01(texelInput01),
        .texelInput10(texelInput10),
        .texelInput11(texelInput11),

        .primaryColor(step1_primaryColor),
        .textureS(step_convert_texture_s),
        .textureT(step_convert_texture_t),

        .previousColor(step1_primaryColor), // For TMU0 it is the primary color, for TMUn-1 it is the output of the previous one

        .fragmentColor(step1_fragmentColor)
    );

    ////////////////////////////////////////////////////////////////////////////
    // STEP 2
    // Calculate Fog
    // Clocks: 6
    ////////////////////////////////////////////////////////////////////////////
    wire [PIXEL_WIDTH - 1 : 0]              step2_fragmentColor;
    wire [FRAMEBUFFER_INDEX_WIDTH - 1 : 0]  step2_index;
    wire [SCREEN_POS_WIDTH - 1 : 0]         step2_screenPosX;
    wire [SCREEN_POS_WIDTH - 1 : 0]         step2_screenPosY;
    wire [31 : 0]                           step2_depth;
    wire                                    step2_valid;

    ValueDelay #(.VALUE_SIZE(FRAMEBUFFER_INDEX_WIDTH), .DELAY(6)) 
        step2_indexDelay (.clk(aclk), .in(step1_index), .out(step2_index));
    ValueDelay #(.VALUE_SIZE(SCREEN_POS_WIDTH), .DELAY(6)) 
        step2_screenPosXDelay (.clk(aclk), .in(step1_screenPosX), .out(step2_screenPosX));
    ValueDelay #(.VALUE_SIZE(SCREEN_POS_WIDTH), .DELAY(6)) 
        step2_screenPosYDelay (.clk(aclk), .in(step1_screenPosY), .out(step2_screenPosY));
    ValueDelay #(.VALUE_SIZE(FLOAT_SIZE), .DELAY(6)) 
        step2_depthDelay (.clk(aclk), .in(step1_depth), .out(step2_depth));
    ValueDelay #(.VALUE_SIZE(1), .DELAY(6)) 
        step2_validDelay (.clk(aclk), .in(step1_valid), .out(step2_valid));

    Fog #(
        .SUB_PIXEL_WIDTH(SUB_PIXEL_WIDTH),
        .CMD_STREAM_WIDTH(CMD_STREAM_WIDTH)
    ) fog (
        .aclk(aclk),
        .resetn(resetn),

        .s_fog_lut_axis_tvalid(s_fog_lut_axis_tvalid),
        .s_fog_lut_axis_tready(s_fog_lut_axis_tready),
        .s_fog_lut_axis_tlast(s_fog_lut_axis_tlast),
        .s_fog_lut_axis_tdata(s_fog_lut_axis_tdata),

        .confFogColor(confFragmentPipelineFogColor),
        .confEnable(confFeatureEnable[RENDER_CONFIG_FEATURE_ENABLE_FOG_POS]),

        .depth(step1_depthWFloat),
        .texelColor(step1_fragmentColor),

        .color(step2_fragmentColor)
    );

    ////////////////////////////////////////////////////////////////////////////
    // STEP 3
    // Access framebuffer, blend, test and save pixel in framebuffer
    // Clocks: 5
    ////////////////////////////////////////////////////////////////////////////
    PerFragmentPipeline #(
        .FRAMEBUFFER_INDEX_WIDTH(FRAMEBUFFER_INDEX_WIDTH),
        .SCREEN_POS_WIDTH(SCREEN_POS_WIDTH),
        .DEPTH_WIDTH(DEPTH_WIDTH),
        .SUB_PIXEL_WIDTH(SUB_PIXEL_WIDTH)
    ) perFragmentPipeline (
        .aclk(aclk),
        .resetn(resetn),

        .conf(confFragmentPipelineConfig),
        .confFeatureEnable(confFeatureEnable),

        .valid(step2_valid),
        .fragmentColor(step2_fragmentColor),
        .depth(step2_depth),
        .index(step2_index),
        .screenPosX(step2_screenPosX),
        .screenPosY(step2_screenPosY),

        .fragmentProcessed(fragmentProcessed),

        .colorIndexRead(colorIndexRead),
        .colorIn(colorIn),

        .colorIndexWrite(colorIndexWrite),
        .colorWriteEnable(colorWriteEnable),
        .colorOut(colorOut),
        .colorOutScreenPosX(colorOutScreenPosX),
        .colorOutScreenPosY(colorOutScreenPosY),

        .depthIndexRead(depthIndexRead),
        .depthIn(depthIn),
        
        .depthIndexWrite(depthIndexWrite),
        .depthWriteEnable(depthWriteEnable),
        .depthOut(depthOut),
        .depthOutScreenPosX(depthOutScreenPosX),
        .depthOutScreenPosY(depthOutScreenPosY)
    );
endmodule


 
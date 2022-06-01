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


module PixelPipeline
#(
    parameter CMD_STREAM_WIDTH = 64,

    // The minimum bit width which is required to contain the resolution
    parameter FRAMEBUFFER_INDEX_WIDTH = 14,

    localparam DEPTH_WIDTH = 16,

    localparam SUB_PIXEL_WIDTH = 8,
    localparam PIXEL_WIDTH = 4 * SUB_PIXEL_WIDTH,

    localparam FLOAT_SIZE = 32
)
(
    input  wire                         clk,
    input  wire                         reset,
    output wire                         pixelInPipeline,

    // Fog function LUT stream
    input  wire                         s_fog_lut_axis_tvalid,
    output wire                         s_fog_lut_axis_tready,
    input  wire                         s_fog_lut_axis_tlast,
    input  wire [CMD_STREAM_WIDTH - 1 : 0] s_fog_lut_axis_tdata,

    // Shader configurations
    input  wire [31 : 0]                confReg1,
    input  wire [31 : 0]                confReg2,
    input  wire                         confTextureClampS,
    input  wire                         confTextureClampT,
    input  wire [PIXEL_WIDTH - 1 : 0]   confTextureEnvColor,
    input  wire [PIXEL_WIDTH - 1 : 0]   confFogColor,

    // Fragment Stream
    input  wire                         s_axis_tvalid,
    output wire                         s_axis_tready,
    input  wire                         s_axis_tlast,
    input  wire [ATTR_INTERP_AXIS_PARAMETER_SIZE - 1 : 0] s_axis_tdata,

    // Texture access
    output wire [15 : 0]                texelS,
    output wire [15 : 0]                texelT,
    input  wire [PIXEL_WIDTH - 1 : 0]   texel,

    // Frame buffer access
    // Read
    output wire [FRAMEBUFFER_INDEX_WIDTH - 1 : 0] colorIndexRead,
    input  wire [PIXEL_WIDTH - 1 : 0]   colorIn,
    // Write
    output wire [FRAMEBUFFER_INDEX_WIDTH - 1 : 0] colorIndexWrite,
    output wire                         colorWriteEnable,
    output wire [PIXEL_WIDTH - 1 : 0]   colorOut,

    // ZBuffer buffer access
    // Read
    output wire [FRAMEBUFFER_INDEX_WIDTH - 1 : 0] depthIndexRead,
    input  wire [DEPTH_WIDTH - 1 : 0]   depthIn,
    // Write
    output wire [FRAMEBUFFER_INDEX_WIDTH - 1 : 0] depthIndexWrite,
    output wire                         depthWriteEnable,
    output wire [DEPTH_WIDTH - 1 : 0]   depthOut
);
`include "RegisterAndDescriptorDefines.vh"
`include "AttributeInterpolatorDefines.vh"

    localparam [SUB_PIXEL_WIDTH - 1 : 0] ONE_POINT_ZERO = { SUB_PIXEL_WIDTH{1'h1} };
    localparam [(SUB_PIXEL_WIDTH * 2) - 1 : 0] ONE_POINT_ZERO_BIG = { { SUB_PIXEL_WIDTH{1'h0} }, ONE_POINT_ZERO };

    assign s_axis_tready = 1;

    wire fragmentProcessed;
    ValueTrack pixelTracker (
        .aclk(clk),
        .resetn(!reset),
        
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
    wire [FLOAT_SIZE - 1 : 0]  step_convert_depth_w_float;
    wire [31 : 0]                   step_convert_texture_s;
    wire [31 : 0]                   step_convert_texture_t;
    wire [31 : 0]                   step_convert_depth_z;
    wire [31 : 0]                   step_convert_color_r;
    wire [31 : 0]                   step_convert_color_g;
    wire [31 : 0]                   step_convert_color_b;
    wire [31 : 0]                   step_convert_color_a;
    wire                            step_convert_tvalid;
    wire [31 : 0]                   step_convert_w;
    wire [23 : 0]                   step_convert_fog_intensity;

    ValueDelay #(.VALUE_SIZE(ATTR_INTERP_AXIS_VERTEX_ATTRIBUTE_SIZE), .DELAY(4)) 
        convert_framebuffer_delay (.clk(clk), .in(s_axis_tdata[ATTR_INTERP_AXIS_FRAMEBUFFER_INDEX_POS +: ATTR_INTERP_AXIS_VERTEX_ATTRIBUTE_SIZE]), .out(step_convert_framebuffer_index));
    ValueDelay #(.VALUE_SIZE(FLOAT_SIZE), .DELAY(4)) 
        convert_depth_delay (.clk(clk), .in(s_axis_tdata[ATTR_INTERP_AXIS_DEPTH_W_POS + (ATTR_INTERP_AXIS_VERTEX_ATTRIBUTE_SIZE - FLOAT_SIZE) +: FLOAT_SIZE]), .out(step_convert_depth_w_float));
    ValueDelay #(.VALUE_SIZE(1), .DELAY(4)) 
        convert_valid_delay (.clk(clk), .in(s_axis_tvalid), .out(step_convert_tvalid));

    FloatToInt #(.MANTISSA_SIZE(FLOAT_SIZE - 9), .EXPONENT_SIZE(8), .INT_SIZE(32), .EXPONENT_BIAS_OFFSET(-15))
        convert_floatToInt_TextureS (.clk(clk), .in(s_axis_tdata[ATTR_INTERP_AXIS_TEXTURE_S_POS + (ATTR_INTERP_AXIS_VERTEX_ATTRIBUTE_SIZE - FLOAT_SIZE)+: FLOAT_SIZE]), .out(step_convert_texture_s));
    FloatToInt #(.MANTISSA_SIZE(FLOAT_SIZE - 9), .EXPONENT_SIZE(8), .INT_SIZE(32), .EXPONENT_BIAS_OFFSET(-15))
        convert_floatToInt_TextureT (.clk(clk), .in(s_axis_tdata[ATTR_INTERP_AXIS_TEXTURE_T_POS + (ATTR_INTERP_AXIS_VERTEX_ATTRIBUTE_SIZE - FLOAT_SIZE) +: FLOAT_SIZE]), .out(step_convert_texture_t));   
    FloatToInt #(.MANTISSA_SIZE(FLOAT_SIZE - 9), .EXPONENT_SIZE(8), .INT_SIZE(32), .EXPONENT_BIAS_OFFSET(-7))
        convert_floatToInt_DepthW (.clk(clk), .in(s_axis_tdata[ATTR_INTERP_AXIS_DEPTH_W_POS + (ATTR_INTERP_AXIS_VERTEX_ATTRIBUTE_SIZE - FLOAT_SIZE) +: FLOAT_SIZE]), .out(step_convert_w));  
    FloatToInt #(.MANTISSA_SIZE(FLOAT_SIZE - 9), .EXPONENT_SIZE(8), .INT_SIZE(32), .EXPONENT_BIAS_OFFSET(-DEPTH_WIDTH))
        convert_floatToInt_DepthZ (.clk(clk), .in(s_axis_tdata[ATTR_INTERP_AXIS_DEPTH_Z_POS + (ATTR_INTERP_AXIS_VERTEX_ATTRIBUTE_SIZE - FLOAT_SIZE) +: FLOAT_SIZE]), .out(step_convert_depth_z));   
    FloatToInt #(.MANTISSA_SIZE(FLOAT_SIZE - 9), .EXPONENT_SIZE(8), .INT_SIZE(32), .EXPONENT_BIAS_OFFSET(-16))
        convert_floatToInt_ColorR (.clk(clk), .in(s_axis_tdata[ATTR_INTERP_AXIS_COLOR_R_POS + (ATTR_INTERP_AXIS_VERTEX_ATTRIBUTE_SIZE - FLOAT_SIZE) +: FLOAT_SIZE]), .out(step_convert_color_r));
    FloatToInt #(.MANTISSA_SIZE(FLOAT_SIZE - 9), .EXPONENT_SIZE(8), .INT_SIZE(32), .EXPONENT_BIAS_OFFSET(-16))
        convert_floatToInt_ColorG (.clk(clk), .in(s_axis_tdata[ATTR_INTERP_AXIS_COLOR_G_POS + (ATTR_INTERP_AXIS_VERTEX_ATTRIBUTE_SIZE - FLOAT_SIZE) +: FLOAT_SIZE]), .out(step_convert_color_g));   
    FloatToInt #(.MANTISSA_SIZE(FLOAT_SIZE - 9), .EXPONENT_SIZE(8), .INT_SIZE(32), .EXPONENT_BIAS_OFFSET(-16))
        convert_floatToInt_ColorB (.clk(clk), .in(s_axis_tdata[ATTR_INTERP_AXIS_COLOR_B_POS + (ATTR_INTERP_AXIS_VERTEX_ATTRIBUTE_SIZE - FLOAT_SIZE) +: FLOAT_SIZE]), .out(step_convert_color_b));   
    FloatToInt #(.MANTISSA_SIZE(FLOAT_SIZE - 9), .EXPONENT_SIZE(8), .INT_SIZE(32), .EXPONENT_BIAS_OFFSET(-16))
        convert_floatToInt_ColorA (.clk(clk), .in(s_axis_tdata[ATTR_INTERP_AXIS_COLOR_A_POS + (ATTR_INTERP_AXIS_VERTEX_ATTRIBUTE_SIZE - FLOAT_SIZE) +: FLOAT_SIZE]), .out(step_convert_color_a));   


    ////////////////////////////////////////////////////////////////////////////
    // STEP 1
    // Calculate fragment color
    // Clocks: 11
    ////////////////////////////////////////////////////////////////////////////
    wire [PIXEL_WIDTH - 1 : 0]                              step1_fragmentColor;
    wire [ATTR_INTERP_AXIS_VERTEX_ATTRIBUTE_SIZE - 1 : 0]   step1_index;
    wire [31 : 0]                                           step1_depth;
    wire                                                    step1_valid;

    ValueDelay #(.VALUE_SIZE(ATTR_INTERP_AXIS_VERTEX_ATTRIBUTE_SIZE), .DELAY(11)) 
        step1_indexDelay (.clk(clk), .in(step_convert_framebuffer_index), .out(step1_index));
    ValueDelay #(.VALUE_SIZE(FLOAT_SIZE), .DELAY(11)) 
        step1_depthDelay (.clk(clk), .in(step_convert_depth_z), .out(step1_depth));
    ValueDelay #(.VALUE_SIZE(1), .DELAY(11)) 
        step1_validDelay (.clk(clk), .in(step_convert_tvalid), .out(step1_valid));

    FragmentPipeline #(
        .CMD_STREAM_WIDTH(CMD_STREAM_WIDTH),
        .SUB_PIXEL_WIDTH(SUB_PIXEL_WIDTH)
    ) fragmentPipeline (
        .aclk(clk),
        .resetn(!reset),

        .s_fog_lut_axis_tvalid(s_fog_lut_axis_tvalid),
        .s_fog_lut_axis_tready(s_fog_lut_axis_tready),
        .s_fog_lut_axis_tlast(s_fog_lut_axis_tlast),
        .s_fog_lut_axis_tdata(s_fog_lut_axis_tdata),

        .confReg1(confReg1),
        .confReg2(confReg2),
        .confTextureClampS(confTextureClampS),
        .confTextureClampT(confTextureClampT),
        .confTextureEnvColor(confTextureEnvColor),
        .confFogColor(confFogColor),

        .texelS(texelS),
        .texelT(texelT),
        .texel(texel),

        .triangleColor({
            // clamp colors 
            (|step_convert_color_r[16 +: 16]) ? ONE_POINT_ZERO : step_convert_color_r[16 - SUB_PIXEL_WIDTH +: SUB_PIXEL_WIDTH],
            (|step_convert_color_g[16 +: 16]) ? ONE_POINT_ZERO : step_convert_color_g[16 - SUB_PIXEL_WIDTH +: SUB_PIXEL_WIDTH],
            (|step_convert_color_b[16 +: 16]) ? ONE_POINT_ZERO : step_convert_color_b[16 - SUB_PIXEL_WIDTH +: SUB_PIXEL_WIDTH],
            (|step_convert_color_a[16 +: 16]) ? ONE_POINT_ZERO : step_convert_color_a[16 - SUB_PIXEL_WIDTH +: SUB_PIXEL_WIDTH]
        }),
        .depth(step_convert_depth_w_float),
        .textureS(step_convert_texture_s[0 +: 24]),
        .textureT(step_convert_texture_t[0 +: 24]),
        .fragmentColor(step1_fragmentColor)
    );

    ////////////////////////////////////////////////////////////////////////////
    // STEP 2
    // Access framebuffer, blend, test and save pixel in framebuffer
    // Clocks: 4
    ////////////////////////////////////////////////////////////////////////////
    FramebufferPipeline #(
        .FRAMEBUFFER_INDEX_WIDTH(FRAMEBUFFER_INDEX_WIDTH),
        .DEPTH_WIDTH(DEPTH_WIDTH),
        .SUB_PIXEL_WIDTH(SUB_PIXEL_WIDTH)
    ) framebufferPipeline (
        .aclk(clk),
        .resetn(!reset),

        .confReg1(confReg1),
        .confReg2(confReg2),

        .valid(step1_valid),
        .fragmentColor(step1_fragmentColor),
        .depth(step1_depth),
        .index(step1_index[0 +: FRAMEBUFFER_INDEX_WIDTH]),

        .fragmentProcessed(fragmentProcessed),

        .colorIndexRead(colorIndexRead),
        .colorIn(colorIn),

        .colorIndexWrite(colorIndexWrite),
        .colorWriteEnable(colorWriteEnable),
        .colorOut(colorOut),

        .depthIndexRead(depthIndexRead),
        .depthIn(depthIn),
        
        .depthIndexWrite(depthIndexWrite),
        .depthWriteEnable(depthWriteEnable),
        .depthOut(depthOut)
    );
endmodule


 
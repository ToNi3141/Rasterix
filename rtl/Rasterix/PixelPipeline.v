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

// This module is used to calculate a whole pixel, starting from the
// meta data from the rasterizer till it is written into the frame buffer,
// including, blending, fogging, texenv, testing and so on.
// It prepares the meta data (basically converts the floats from the)
// interpolator into fixed point numbers, which can be used from the 
// fragment and framebuffer pipeline.
// Pipelined: yes
// Depth: 30 cycles
module PixelPipeline
#(
    parameter CMD_STREAM_WIDTH = 64,

    // The minimum bit width which is required to contain the resolution
    parameter FRAMEBUFFER_INDEX_WIDTH = 14,

    parameter DEPTH_WIDTH = 16,

    parameter STENCIL_WIDTH = 4,

    parameter SUB_PIXEL_WIDTH = 8,
    localparam PIXEL_WIDTH = 4 * SUB_PIXEL_WIDTH,

    localparam FLOAT_SIZE = 32,

    localparam TEX_ADDR_WIDTH = 16,

    parameter ENABLE_SECOND_TMU = 1,
    
    parameter SCREEN_POS_WIDTH = 11,

    localparam KEEP_WIDTH = 1
)
(
    input  wire                                     aclk,
    input  wire                                     resetn,

    // Fog function LUT stream
    input  wire                                     s_fog_lut_axis_tvalid,
    output wire                                     s_fog_lut_axis_tready,
    input  wire                                     s_fog_lut_axis_tlast,
    input  wire [CMD_STREAM_WIDTH - 1 : 0]          s_fog_lut_axis_tdata,

    // Shader configurations
    input  wire [31 : 0]                            confFeatureEnable,
    input  wire [31 : 0]                            confFragmentPipelineConfig,
    input  wire [PIXEL_WIDTH - 1 : 0]               confFragmentPipelineFogColor,
    input  wire [31 : 0]                            confStencilBufferConfig,
    input  wire [31 : 0]                            confTMU0TexEnvConfig,
    input  wire [31 : 0]                            confTMU0TextureConfig,
    input  wire [PIXEL_WIDTH - 1 : 0]               confTMU0TexEnvColor,
    input  wire [31 : 0]                            confTMU1TexEnvConfig,
    input  wire [31 : 0]                            confTMU1TextureConfig,
    input  wire [PIXEL_WIDTH - 1 : 0]               confTMU1TexEnvColor,

    // Fragment Stream
    input  wire                                     s_axis_tvalid,
    output wire                                     s_axis_tready,
    input  wire                                     s_axis_tlast,
    input  wire [KEEP_WIDTH - 1 : 0]                s_axis_tkeep,
    input  wire [ATTR_INTERP_AXIS_PARAMETER_SIZE - 1 : 0] s_axis_tdata,

    // Texture access
    // TMU0 texel quad access
    output wire [TEX_ADDR_WIDTH - 1 : 0]            texel0Addr00,
    output wire [TEX_ADDR_WIDTH - 1 : 0]            texel0Addr01,
    output wire [TEX_ADDR_WIDTH - 1 : 0]            texel0Addr10,
    output wire [TEX_ADDR_WIDTH - 1 : 0]            texel0Addr11,
    input  wire [PIXEL_WIDTH - 1 : 0]               texel0Input00,
    input  wire [PIXEL_WIDTH - 1 : 0]               texel0Input01,
    input  wire [PIXEL_WIDTH - 1 : 0]               texel0Input10,
    input  wire [PIXEL_WIDTH - 1 : 0]               texel0Input11,
    // TMU1 texel quad access
    output wire [TEX_ADDR_WIDTH - 1 : 0]            texel1Addr00,
    output wire [TEX_ADDR_WIDTH - 1 : 0]            texel1Addr01,
    output wire [TEX_ADDR_WIDTH - 1 : 0]            texel1Addr10,
    output wire [TEX_ADDR_WIDTH - 1 : 0]            texel1Addr11,
    input  wire [PIXEL_WIDTH - 1 : 0]               texel1Input00,
    input  wire [PIXEL_WIDTH - 1 : 0]               texel1Input01,
    input  wire [PIXEL_WIDTH - 1 : 0]               texel1Input10,
    input  wire [PIXEL_WIDTH - 1 : 0]               texel1Input11,

    output wire [PIXEL_WIDTH - 1 : 0]               m_framebuffer_fragmentColor,
    output wire [FRAMEBUFFER_INDEX_WIDTH - 1 : 0]   m_framebuffer_index,
    output wire [SCREEN_POS_WIDTH - 1 : 0]          m_framebuffer_screenPosX,
    output wire [SCREEN_POS_WIDTH - 1 : 0]          m_framebuffer_screenPosY,
    output wire [31 : 0]                            m_framebuffer_depth,
    output wire                                     m_framebuffer_valid,
    output wire                                     m_framebuffer_last,
    output wire                                     m_framebuffer_keep
);
`include "RegisterAndDescriptorDefines.vh"
`include "AttributeInterpolatorDefines.vh"

    localparam [SUB_PIXEL_WIDTH - 1 : 0] ONE_POINT_ZERO = { SUB_PIXEL_WIDTH{1'h1} };
    localparam [(SUB_PIXEL_WIDTH * 2) - 1 : 0] ONE_POINT_ZERO_BIG = { { SUB_PIXEL_WIDTH{1'h0} }, ONE_POINT_ZERO };

    assign s_axis_tready = 1;

    ////////////////////////////////////////////////////////////////////////////
    // STEP 0
    // Convert float to integer
    // Clocks: 4
    ////////////////////////////////////////////////////////////////////////////
    wire [ATTR_INTERP_AXIS_VERTEX_ATTRIBUTE_SIZE - 1 : 0]  step_convert_framebuffer_index;
    wire [SCREEN_POS_WIDTH - 1 : 0] step_convert_sreen_pos_x;
    wire [SCREEN_POS_WIDTH - 1 : 0] step_convert_sreen_pos_y;
    wire [FLOAT_SIZE - 1 : 0]   step_convert_depth_w_float;
    wire [31 : 0]               step_convert_texture0_s;
    wire [31 : 0]               step_convert_texture0_t;
    wire [31 : 0]               step_convert_texture1_s;
    wire [31 : 0]               step_convert_texture1_t;
    wire [31 : 0]               step_convert_depth_z;
    wire [31 : 0]               step_convert_color_r;
    wire [31 : 0]               step_convert_color_g;
    wire [31 : 0]               step_convert_color_b;
    wire [31 : 0]               step_convert_color_a;
    wire                        step_convert_tvalid;
    wire [KEEP_WIDTH - 1 : 0]   step_convert_tkeep;
    wire                        step_convert_tlast;

    parameter CONV_DELAY = 0;

    // Framebuffer Index
    ValueDelay #(.VALUE_SIZE(ATTR_INTERP_AXIS_VERTEX_ATTRIBUTE_SIZE), .DELAY(2 + CONV_DELAY)) 
        convert_framebuffer_delay (.clk(aclk), .in(s_axis_tdata[ATTR_INTERP_AXIS_FRAMEBUFFER_INDEX_POS +: ATTR_INTERP_AXIS_VERTEX_ATTRIBUTE_SIZE]), .out(step_convert_framebuffer_index));

    // Screen Poisition
    ValueDelay #(.VALUE_SIZE(SCREEN_POS_WIDTH), .DELAY(2 + CONV_DELAY)) 
        convert_screen_pos_x_delay (.clk(aclk), .in(s_axis_tdata[ATTR_INTERP_AXIS_SCREEN_XY_POS + ATTR_INTERP_AXIS_SCREEN_X_POS +: SCREEN_POS_WIDTH]), .out(step_convert_sreen_pos_x));
    ValueDelay #(.VALUE_SIZE(SCREEN_POS_WIDTH), .DELAY(2 + CONV_DELAY)) 
        convert_screen_pos_y_delay (.clk(aclk), .in(s_axis_tdata[ATTR_INTERP_AXIS_SCREEN_XY_POS + ATTR_INTERP_AXIS_SCREEN_Y_POS +: SCREEN_POS_WIDTH]), .out(step_convert_sreen_pos_y));

    // Fragment stream flags
    ValueDelay #(.VALUE_SIZE(1), .DELAY(2 + CONV_DELAY)) 
        convert_valid_delay (.clk(aclk), .in(s_axis_tvalid), .out(step_convert_tvalid));
    ValueDelay #(.VALUE_SIZE(KEEP_WIDTH), .DELAY(2 + CONV_DELAY)) 
        convert_keep_delay (.clk(aclk), .in(s_axis_tkeep), .out(step_convert_tkeep));
    ValueDelay #(.VALUE_SIZE(1), .DELAY(2 + CONV_DELAY)) 
        convert_last_delay (.clk(aclk), .in(s_axis_tlast), .out(step_convert_tlast));

    // Depth
    ValueDelay #(.VALUE_SIZE(FLOAT_SIZE), .DELAY(2 + CONV_DELAY)) 
        convert_depth_delay (.clk(aclk), .in(s_axis_tdata[ATTR_INTERP_AXIS_DEPTH_W_POS + (ATTR_INTERP_AXIS_VERTEX_ATTRIBUTE_SIZE - FLOAT_SIZE) +: FLOAT_SIZE]), .out(step_convert_depth_w_float));
    FloatToInt #(.MANTISSA_SIZE(FLOAT_SIZE - 9), .EXPONENT_SIZE(8), .INT_SIZE(32), .EXPONENT_BIAS_OFFSET(-DEPTH_WIDTH), .DELAY(CONV_DELAY))
        convert_floatToInt_DepthZ (.clk(aclk), .in(s_axis_tdata[ATTR_INTERP_AXIS_DEPTH_Z_POS + (ATTR_INTERP_AXIS_VERTEX_ATTRIBUTE_SIZE - FLOAT_SIZE) +: FLOAT_SIZE]), .out(step_convert_depth_z)); 

    // Tex Coords TMU0
    FloatToInt #(.MANTISSA_SIZE(FLOAT_SIZE - 9), .EXPONENT_SIZE(8), .INT_SIZE(32), .EXPONENT_BIAS_OFFSET(-15), .DELAY(CONV_DELAY))
        convert_floatToInt_tmu0_textureS (.clk(aclk), .in(s_axis_tdata[ATTR_INTERP_AXIS_TEXTURE0_S_POS + (ATTR_INTERP_AXIS_VERTEX_ATTRIBUTE_SIZE - FLOAT_SIZE) +: FLOAT_SIZE]), .out(step_convert_texture0_s));
    FloatToInt #(.MANTISSA_SIZE(FLOAT_SIZE - 9), .EXPONENT_SIZE(8), .INT_SIZE(32), .EXPONENT_BIAS_OFFSET(-15), .DELAY(CONV_DELAY))
        convert_floatToInt_tmu0_textureT (.clk(aclk), .in(s_axis_tdata[ATTR_INTERP_AXIS_TEXTURE0_T_POS + (ATTR_INTERP_AXIS_VERTEX_ATTRIBUTE_SIZE - FLOAT_SIZE) +: FLOAT_SIZE]), .out(step_convert_texture0_t));   

    // Tex Coords TMU1
    FloatToInt #(.MANTISSA_SIZE(FLOAT_SIZE - 9), .EXPONENT_SIZE(8), .INT_SIZE(32), .EXPONENT_BIAS_OFFSET(-15), .DELAY(CONV_DELAY))
        convert_floatToInt_tmu1_textureS (.clk(aclk), .in(s_axis_tdata[ATTR_INTERP_AXIS_TEXTURE1_S_POS + (ATTR_INTERP_AXIS_VERTEX_ATTRIBUTE_SIZE - FLOAT_SIZE) +: FLOAT_SIZE]), .out(step_convert_texture1_s));
    FloatToInt #(.MANTISSA_SIZE(FLOAT_SIZE - 9), .EXPONENT_SIZE(8), .INT_SIZE(32), .EXPONENT_BIAS_OFFSET(-15), .DELAY(CONV_DELAY))
        convert_floatToInt_tmu1_textureT (.clk(aclk), .in(s_axis_tdata[ATTR_INTERP_AXIS_TEXTURE1_T_POS + (ATTR_INTERP_AXIS_VERTEX_ATTRIBUTE_SIZE - FLOAT_SIZE) +: FLOAT_SIZE]), .out(step_convert_texture1_t));   
  
    // Fragment Color
    FloatToInt #(.MANTISSA_SIZE(FLOAT_SIZE - 9), .EXPONENT_SIZE(8), .INT_SIZE(32), .EXPONENT_BIAS_OFFSET(-16), .DELAY(CONV_DELAY))
        convert_floatToInt_ColorR (.clk(aclk), .in(s_axis_tdata[ATTR_INTERP_AXIS_COLOR_R_POS + (ATTR_INTERP_AXIS_VERTEX_ATTRIBUTE_SIZE - FLOAT_SIZE) +: FLOAT_SIZE]), .out(step_convert_color_r));
    FloatToInt #(.MANTISSA_SIZE(FLOAT_SIZE - 9), .EXPONENT_SIZE(8), .INT_SIZE(32), .EXPONENT_BIAS_OFFSET(-16), .DELAY(CONV_DELAY))
        convert_floatToInt_ColorG (.clk(aclk), .in(s_axis_tdata[ATTR_INTERP_AXIS_COLOR_G_POS + (ATTR_INTERP_AXIS_VERTEX_ATTRIBUTE_SIZE - FLOAT_SIZE) +: FLOAT_SIZE]), .out(step_convert_color_g));   
    FloatToInt #(.MANTISSA_SIZE(FLOAT_SIZE - 9), .EXPONENT_SIZE(8), .INT_SIZE(32), .EXPONENT_BIAS_OFFSET(-16), .DELAY(CONV_DELAY))
        convert_floatToInt_ColorB (.clk(aclk), .in(s_axis_tdata[ATTR_INTERP_AXIS_COLOR_B_POS + (ATTR_INTERP_AXIS_VERTEX_ATTRIBUTE_SIZE - FLOAT_SIZE) +: FLOAT_SIZE]), .out(step_convert_color_b));   
    FloatToInt #(.MANTISSA_SIZE(FLOAT_SIZE - 9), .EXPONENT_SIZE(8), .INT_SIZE(32), .EXPONENT_BIAS_OFFSET(-16), .DELAY(CONV_DELAY))
        convert_floatToInt_ColorA (.clk(aclk), .in(s_axis_tdata[ATTR_INTERP_AXIS_COLOR_A_POS + (ATTR_INTERP_AXIS_VERTEX_ATTRIBUTE_SIZE - FLOAT_SIZE) +: FLOAT_SIZE]), .out(step_convert_color_a));   

    wire [PIXEL_WIDTH - 1 : 0] convert_primary_color = {
        // clamp colors 
        (|step_convert_color_r[16 +: 16]) ? ONE_POINT_ZERO : step_convert_color_r[16 - SUB_PIXEL_WIDTH +: SUB_PIXEL_WIDTH],
        (|step_convert_color_g[16 +: 16]) ? ONE_POINT_ZERO : step_convert_color_g[16 - SUB_PIXEL_WIDTH +: SUB_PIXEL_WIDTH],
        (|step_convert_color_b[16 +: 16]) ? ONE_POINT_ZERO : step_convert_color_b[16 - SUB_PIXEL_WIDTH +: SUB_PIXEL_WIDTH],
        (|step_convert_color_a[16 +: 16]) ? ONE_POINT_ZERO : step_convert_color_a[16 - SUB_PIXEL_WIDTH +: SUB_PIXEL_WIDTH]
    };

    ////////////////////////////////////////////////////////////////////////////
    // STEP 1
    // TMU0
    // Clocks: 11
    ////////////////////////////////////////////////////////////////////////////
    wire [PIXEL_WIDTH - 1 : 0]              step1_fragmentColor;
    wire [FRAMEBUFFER_INDEX_WIDTH - 1 : 0]  step1_index;
    wire [SCREEN_POS_WIDTH - 1 : 0]         step1_screenPosX;
    wire [SCREEN_POS_WIDTH - 1 : 0]         step1_screenPosY;
    wire [31 : 0]                           step1_depth;
    wire [FLOAT_SIZE - 1 : 0]               step1_depthWFloat;
    wire                                    step1_valid;
    wire [PIXEL_WIDTH - 1 : 0]              step1_primaryColor;
    wire [31 : 0]                           step1_texture1S;
    wire [31 : 0]                           step1_texture1T;
    wire [KEEP_WIDTH - 1 : 0]               step1_keep;
    wire                                    step1_last;


    ValueDelay #(.VALUE_SIZE(FRAMEBUFFER_INDEX_WIDTH), .DELAY(11)) 
        step1_indexDelay (.clk(aclk), .in(step_convert_framebuffer_index[0 +: FRAMEBUFFER_INDEX_WIDTH]), .out(step1_index));

    ValueDelay #(.VALUE_SIZE(SCREEN_POS_WIDTH), .DELAY(11)) 
        step1_screenPosXDelay (.clk(aclk), .in(step_convert_sreen_pos_x), .out(step1_screenPosX));
    ValueDelay #(.VALUE_SIZE(SCREEN_POS_WIDTH), .DELAY(11)) 
        step1_screenPosYDelay (.clk(aclk), .in(step_convert_sreen_pos_y), .out(step1_screenPosY));

    ValueDelay #(.VALUE_SIZE(32), .DELAY(11)) 
        step1_depthDelay (.clk(aclk), .in(step_convert_depth_z), .out(step1_depth));
    ValueDelay #(.VALUE_SIZE(FLOAT_SIZE), .DELAY(11)) 
        step1_depthWDelay (.clk(aclk), .in(step_convert_depth_w_float), .out(step1_depthWFloat));

    ValueDelay #(.VALUE_SIZE(1), .DELAY(11)) 
        step1_validDelay (.clk(aclk), .in(step_convert_tvalid), .out(step1_valid));
    ValueDelay #(.VALUE_SIZE(KEEP_WIDTH), .DELAY(11)) 
        step1_keepDelay (.clk(aclk), .in(step_convert_tkeep), .out(step1_keep));
    ValueDelay #(.VALUE_SIZE(1), .DELAY(11)) 
        step1_lastDelay (.clk(aclk), .in(step_convert_tlast), .out(step1_last));

    ValueDelay #(.VALUE_SIZE(32), .DELAY(11)) 
        step1_texture1SDelay (.clk(aclk), .in(step_convert_texture1_s), .out(step1_texture1S));
    ValueDelay #(.VALUE_SIZE(32), .DELAY(11)) 
        step1_texture1TDelay (.clk(aclk), .in(step_convert_texture1_t), .out(step1_texture1T));

    TextureMappingUnit #(
        .CMD_STREAM_WIDTH(CMD_STREAM_WIDTH),
        .SUB_PIXEL_WIDTH(SUB_PIXEL_WIDTH)
    ) tmu0 (
        .aclk(aclk),
        .resetn(resetn),

        .confFunc(confTMU0TexEnvConfig),
        .confTextureEnvColor(confTMU0TexEnvColor),
        .confTextureConfig(confTMU0TextureConfig),
        .confEnable(confFeatureEnable[RENDER_CONFIG_FEATURE_ENABLE_TMU0_POS]),

        .texelAddr00(texel0Addr00),
        .texelAddr01(texel0Addr01),
        .texelAddr10(texel0Addr10),
        .texelAddr11(texel0Addr11),

        .texelInput00(texel0Input00),
        .texelInput01(texel0Input01),
        .texelInput10(texel0Input10),
        .texelInput11(texel0Input11),

        .primaryColor(convert_primary_color),
        .textureS(step_convert_texture0_s),
        .textureT(step_convert_texture0_t),

        .previousColor(convert_primary_color), // For TMU0 it is the primary color, for TMUn-1 it is the output of the previous one

        .fragmentColor(step1_fragmentColor)
    );

    ////////////////////////////////////////////////////////////////////////////
    // STEP 2
    // TMU1
    // Clocks: 11
    ////////////////////////////////////////////////////////////////////////////
    wire [PIXEL_WIDTH - 1 : 0]              step2_fragmentColor;
    wire [FRAMEBUFFER_INDEX_WIDTH - 1 : 0]  step2_index;
    wire [SCREEN_POS_WIDTH - 1 : 0]         step2_screenPosX;
    wire [SCREEN_POS_WIDTH - 1 : 0]         step2_screenPosY;
    wire [31 : 0]                           step2_depth;
    wire [FLOAT_SIZE : 0]                   step2_depthWFloat;
    wire                                    step2_valid;
    wire [KEEP_WIDTH - 1 : 0]               step2_keep;
    wire                                    step2_last;

    generate
        if (ENABLE_SECOND_TMU)
        begin
            ValueDelay #(.VALUE_SIZE(FRAMEBUFFER_INDEX_WIDTH), .DELAY(11)) 
                step2_indexDelay (.clk(aclk), .in(step1_index), .out(step2_index));

            ValueDelay #(.VALUE_SIZE(SCREEN_POS_WIDTH), .DELAY(11)) 
                step2_screenPosXDelay (.clk(aclk), .in(step1_screenPosX), .out(step2_screenPosX));
            ValueDelay #(.VALUE_SIZE(SCREEN_POS_WIDTH), .DELAY(11)) 
                step2_screenPosYDelay (.clk(aclk), .in(step1_screenPosY), .out(step2_screenPosY));

            ValueDelay #(.VALUE_SIZE(32), .DELAY(11)) 
                step2_depthDelay (.clk(aclk), .in(step1_depth), .out(step2_depth));
            ValueDelay #(.VALUE_SIZE(FLOAT_SIZE), .DELAY(11)) 
                step2_depthWDelay (.clk(aclk), .in(step1_depthWFloat), .out(step2_depthWFloat));

            ValueDelay #(.VALUE_SIZE(1), .DELAY(11)) 
                step2_validDelay (.clk(aclk), .in(step1_valid), .out(step2_valid));
            ValueDelay #(.VALUE_SIZE(KEEP_WIDTH), .DELAY(11)) 
                step2_keepDelay (.clk(aclk), .in(step1_keep), .out(step2_keep));
            ValueDelay #(.VALUE_SIZE(1), .DELAY(11)) 
                step2_lastDelay (.clk(aclk), .in(step1_last), .out(step2_last));

            TextureMappingUnit #(
                .CMD_STREAM_WIDTH(CMD_STREAM_WIDTH),
                .SUB_PIXEL_WIDTH(SUB_PIXEL_WIDTH)
            ) tmu1 (
                .aclk(aclk),
                .resetn(resetn),

                .confFunc(confTMU1TexEnvConfig),
                .confTextureEnvColor(confTMU1TexEnvColor),
                .confTextureConfig(confTMU1TextureConfig),
                .confEnable(confFeatureEnable[RENDER_CONFIG_FEATURE_ENABLE_TMU1_POS]),

                .texelAddr00(texel1Addr00),
                .texelAddr01(texel1Addr01),
                .texelAddr10(texel1Addr10),
                .texelAddr11(texel1Addr11),

                .texelInput00(texel1Input00),
                .texelInput01(texel1Input01),
                .texelInput10(texel1Input10),
                .texelInput11(texel1Input11),

                .primaryColor(step1_primaryColor),
                .textureS(step1_texture1S),
                .textureT(step1_texture1T),

                .previousColor(step1_fragmentColor),

                .fragmentColor(step2_fragmentColor)
            );
        end
        else
        begin
            assign step2_fragmentColor = step1_fragmentColor;
            assign step2_index = step1_index;
            assign step2_screenPosX = step1_screenPosX;
            assign step2_screenPosY = step1_screenPosY;
            assign step2_depth = step1_depth;
            assign step2_depthWFloat = step1_depthWFloat;
            assign step2_valid = step1_valid;
            assign step2_keep = step1_keep;
            assign step2_last = step1_last;
        end
    endgenerate


    ////////////////////////////////////////////////////////////////////////////
    // STEP 3
    // Calculate Fog
    // Clocks: 6
    ////////////////////////////////////////////////////////////////////////////
    wire [PIXEL_WIDTH - 1 : 0]              step3_fragmentColor;
    wire [FRAMEBUFFER_INDEX_WIDTH - 1 : 0]  step3_index;
    wire [SCREEN_POS_WIDTH - 1 : 0]         step3_screenPosX;
    wire [SCREEN_POS_WIDTH - 1 : 0]         step3_screenPosY;
    wire [31 : 0]                           step3_depth;
    wire                                    step3_valid;
    wire [KEEP_WIDTH - 1 : 0]               step3_keep;
    wire                                    step3_last;

    ValueDelay #(.VALUE_SIZE(FRAMEBUFFER_INDEX_WIDTH), .DELAY(6)) 
        step3_indexDelay (.clk(aclk), .in(step2_index), .out(step3_index));
    ValueDelay #(.VALUE_SIZE(SCREEN_POS_WIDTH), .DELAY(6)) 
        step3_screenPosXDelay (.clk(aclk), .in(step2_screenPosX), .out(step3_screenPosX));
    ValueDelay #(.VALUE_SIZE(SCREEN_POS_WIDTH), .DELAY(6)) 
        step3_screenPosYDelay (.clk(aclk), .in(step2_screenPosY), .out(step3_screenPosY));
    ValueDelay #(.VALUE_SIZE(32), .DELAY(6)) 
        step3_depthDelay (.clk(aclk), .in(step2_depth), .out(step3_depth));
    ValueDelay #(.VALUE_SIZE(1), .DELAY(6)) 
        step3_validDelay (.clk(aclk), .in(step2_valid), .out(step3_valid));
    ValueDelay #(.VALUE_SIZE(KEEP_WIDTH), .DELAY(6)) 
        step3_keepDelay (.clk(aclk), .in(step2_keep), .out(step3_keep));
    ValueDelay #(.VALUE_SIZE(1), .DELAY(6)) 
        step3_lastDelay (.clk(aclk), .in(step2_last), .out(step3_last));

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

        .depth({step2_depthWFloat, {(32 - FLOAT_SIZE){1'b0}}}), // Fullup mantissa to cast back to 32 bit float
        .texelColor(step2_fragmentColor),

        .color(step3_fragmentColor)
    );

    assign m_framebuffer_fragmentColor = step3_fragmentColor;
    assign m_framebuffer_index = step3_index;
    assign m_framebuffer_screenPosX = step3_screenPosX;
    assign m_framebuffer_screenPosY = step3_screenPosY;
    assign m_framebuffer_depth = step3_depth;
    assign m_framebuffer_valid = step3_valid;
    assign m_framebuffer_keep = step3_keep;
    assign m_framebuffer_last = step3_last;

endmodule


 
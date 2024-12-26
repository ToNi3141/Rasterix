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
    // The minimum bit width which is required to contain the resolution
    parameter INDEX_WIDTH = 14,

    parameter STENCIL_WIDTH = 4,

    parameter SUB_PIXEL_WIDTH = 8,
    localparam PIXEL_WIDTH = 4 * SUB_PIXEL_WIDTH,

    localparam FLOAT_SIZE = 32,

    localparam TEX_ADDR_WIDTH = 17,

    parameter ENABLE_SECOND_TMU = 1,
    parameter ENABLE_LOD_CALC = 1,
    
    parameter SCREEN_POS_WIDTH = 11,

    localparam KEEP_WIDTH = 1,

    localparam ATTRIBUTE_SIZE = 32
)
(
    input  wire                                     aclk,
    input  wire                                     resetn,

    // Fog function LUT stream
    input  wire                                     s_fog_lut_axis_tvalid,
    output wire                                     s_fog_lut_axis_tready,
    input  wire                                     s_fog_lut_axis_tlast,
    input  wire [31 : 0]                            s_fog_lut_axis_tdata,

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
    output wire                                     s_attrb_tready,
    input  wire                                     s_attrb_tvalid,
    input  wire                                     s_attrb_tlast,
    input  wire [KEEP_WIDTH - 1 : 0]                s_attrb_tkeep,
    input  wire [SCREEN_POS_WIDTH - 1 : 0]          s_attrb_tspx,
    input  wire [SCREEN_POS_WIDTH - 1 : 0]          s_attrb_tspy,
    input  wire [INDEX_WIDTH - 1 : 0]               s_attrb_tindex,
    input  wire [ATTRIBUTE_SIZE - 1 : 0]            s_attrb_tdepth_w, // Float
    input  wire [31 : 0]                            s_attrb_tdepth_z, // Q16.16
    input  wire [31 : 0]                            s_attrb_ttexture0_t, // S16.15
    input  wire [31 : 0]                            s_attrb_ttexture0_s, // S16.15
    input  wire [31 : 0]                            s_attrb_tmipmap0_t, // S16.15
    input  wire [31 : 0]                            s_attrb_tmipmap0_s, // S16.15
    input  wire [31 : 0]                            s_attrb_ttexture1_t, // S16.15
    input  wire [31 : 0]                            s_attrb_ttexture1_s, // S16.15
    input  wire [31 : 0]                            s_attrb_tmipmap1_t, // S16.15
    input  wire [31 : 0]                            s_attrb_tmipmap1_s, // S16.15
    input  wire [SUB_PIXEL_WIDTH - 1 : 0]           s_attrb_tcolor_a, // Qn
    input  wire [SUB_PIXEL_WIDTH - 1 : 0]           s_attrb_tcolor_b, // Qn
    input  wire [SUB_PIXEL_WIDTH - 1 : 0]           s_attrb_tcolor_g, // Qn
    input  wire [SUB_PIXEL_WIDTH - 1 : 0]           s_attrb_tcolor_r, // Qn

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

    input  wire                                     m_frag_tready,
    output wire [PIXEL_WIDTH - 1 : 0]               m_frag_tfragmentColor,
    output wire [INDEX_WIDTH - 1 : 0]               m_frag_tindex,
    output wire [SCREEN_POS_WIDTH - 1 : 0]          m_frag_tscreenPosX,
    output wire [SCREEN_POS_WIDTH - 1 : 0]          m_frag_tscreenPosY,
    output wire [31 : 0]                            m_frag_tdepth,
    output wire                                     m_frag_tvalid,
    output wire                                     m_frag_tlast,
    output wire                                     m_frag_tkeep
);
`include "RegisterAndDescriptorDefines.vh"
    localparam TMU_DELAY = 14;

    // Flow Control
    wire ce;
    assign ce = m_frag_tready;
    assign s_attrb_tready = m_frag_tready;

    ////////////////////////////////////////////////////////////////////////////
    // STEP 1
    // TMU0
    // Clocks: 14
    ////////////////////////////////////////////////////////////////////////////
    wire [PIXEL_WIDTH - 1 : 0]              step1_fragmentColor;
    wire [INDEX_WIDTH - 1 : 0]              step1_index;
    wire [SCREEN_POS_WIDTH - 1 : 0]         step1_screenPosX;
    wire [SCREEN_POS_WIDTH - 1 : 0]         step1_screenPosY;
    wire [31 : 0]                           step1_depth;
    wire [FLOAT_SIZE - 1 : 0]               step1_depthWFloat;
    wire                                    step1_valid;
    wire [PIXEL_WIDTH - 1 : 0]              step1_primaryColor;
    wire [31 : 0]                           step1_texture1S;
    wire [31 : 0]                           step1_texture1T;
    wire [31 : 0]                           step1_mipmap1S;
    wire [31 : 0]                           step1_mipmap1T;
    wire [KEEP_WIDTH - 1 : 0]               step1_keep;
    wire                                    step1_last;

    wire [PIXEL_WIDTH - 1 : 0]  step_1_primary_color = {
        s_attrb_tcolor_r,
        s_attrb_tcolor_g,
        s_attrb_tcolor_b,
        s_attrb_tcolor_a
    };

    ValueDelay #(.VALUE_SIZE(INDEX_WIDTH), .DELAY(TMU_DELAY)) 
        step1_indexDelay (.clk(aclk), .ce(ce), .in(s_attrb_tindex), .out(step1_index));

    ValueDelay #(.VALUE_SIZE(SCREEN_POS_WIDTH), .DELAY(TMU_DELAY)) 
        step1_screenPosXDelay (.clk(aclk), .ce(ce), .in(s_attrb_tspx), .out(step1_screenPosX));
    ValueDelay #(.VALUE_SIZE(SCREEN_POS_WIDTH), .DELAY(TMU_DELAY)) 
        step1_screenPosYDelay (.clk(aclk), .ce(ce), .in(s_attrb_tspy), .out(step1_screenPosY));

    ValueDelay #(.VALUE_SIZE(32), .DELAY(TMU_DELAY)) 
        step1_depthDelay (.clk(aclk), .ce(ce), .in(s_attrb_tdepth_z), .out(step1_depth));
    ValueDelay #(.VALUE_SIZE(FLOAT_SIZE), .DELAY(TMU_DELAY)) 
        step1_depthWDelay (.clk(aclk), .ce(ce), .in(s_attrb_tdepth_w), .out(step1_depthWFloat));

    ValueDelay #(.VALUE_SIZE(1), .DELAY(TMU_DELAY)) 
        step1_validDelay (.clk(aclk), .ce(ce), .in(s_attrb_tvalid), .out(step1_valid));
    ValueDelay #(.VALUE_SIZE(KEEP_WIDTH), .DELAY(TMU_DELAY)) 
        step1_keepDelay (.clk(aclk), .ce(ce), .in(s_attrb_tkeep), .out(step1_keep));
    ValueDelay #(.VALUE_SIZE(1), .DELAY(TMU_DELAY)) 
        step1_lastDelay (.clk(aclk), .ce(ce), .in(s_attrb_tlast), .out(step1_last));

    generate
        if (ENABLE_SECOND_TMU)
        begin
            ValueDelay #(.VALUE_SIZE(32), .DELAY(TMU_DELAY)) 
                step1_texture1SDelay (.clk(aclk), .ce(ce), .in(s_attrb_ttexture1_s), .out(step1_texture1S));
            ValueDelay #(.VALUE_SIZE(32), .DELAY(TMU_DELAY)) 
                step1_texture1TDelay (.clk(aclk), .ce(ce), .in(s_attrb_ttexture1_t), .out(step1_texture1T));
            ValueDelay #(.VALUE_SIZE(32), .DELAY(TMU_DELAY)) 
                step1_mipmap1SDelay (.clk(aclk), .ce(ce), .in(s_attrb_tmipmap1_s), .out(step1_mipmap1S));
            ValueDelay #(.VALUE_SIZE(32), .DELAY(TMU_DELAY)) 
                step1_mipmap1TDelay (.clk(aclk), .ce(ce), .in(s_attrb_tmipmap1_t), .out(step1_mipmap1T));
        end
    endgenerate

    TextureMappingUnit #(
        .SUB_PIXEL_WIDTH(SUB_PIXEL_WIDTH),
        .ENABLE_LOD_CALC(ENABLE_LOD_CALC)
    ) tmu0 (
        .aclk(aclk),
        .resetn(resetn),
        .ce(ce),

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

        .primaryColor(step_1_primary_color),
        .textureS(s_attrb_ttexture0_s),
        .textureT(s_attrb_ttexture0_t),
        .mipmapS(s_attrb_tmipmap0_s),
        .mipmapT(s_attrb_tmipmap0_t),

        .previousColor(step_1_primary_color), // For TMU0 it is the primary color, for TMUn-1 it is the output of the previous one

        .fragmentColor(step1_fragmentColor)
    );

    ////////////////////////////////////////////////////////////////////////////
    // STEP 2
    // TMU1
    // Clocks: 14
    ////////////////////////////////////////////////////////////////////////////
    wire [PIXEL_WIDTH - 1 : 0]              step2_fragmentColor;
    wire [INDEX_WIDTH - 1 : 0]              step2_index;
    wire [SCREEN_POS_WIDTH - 1 : 0]         step2_screenPosX;
    wire [SCREEN_POS_WIDTH - 1 : 0]         step2_screenPosY;
    wire [31 : 0]                           step2_depth;
    wire [FLOAT_SIZE - 1 : 0]               step2_depthWFloat;
    wire                                    step2_valid;
    wire [KEEP_WIDTH - 1 : 0]               step2_keep;
    wire                                    step2_last;

    generate
        if (ENABLE_SECOND_TMU)
        begin
            ValueDelay #(.VALUE_SIZE(INDEX_WIDTH), .DELAY(TMU_DELAY)) 
                step2_indexDelay (.clk(aclk), .ce(ce), .in(step1_index), .out(step2_index));

            ValueDelay #(.VALUE_SIZE(SCREEN_POS_WIDTH), .DELAY(TMU_DELAY)) 
                step2_screenPosXDelay (.clk(aclk), .ce(ce), .in(step1_screenPosX), .out(step2_screenPosX));
            ValueDelay #(.VALUE_SIZE(SCREEN_POS_WIDTH), .DELAY(TMU_DELAY)) 
                step2_screenPosYDelay (.clk(aclk), .ce(ce), .in(step1_screenPosY), .out(step2_screenPosY));

            ValueDelay #(.VALUE_SIZE(32), .DELAY(TMU_DELAY)) 
                step2_depthDelay (.clk(aclk), .ce(ce), .in(step1_depth), .out(step2_depth));
            ValueDelay #(.VALUE_SIZE(FLOAT_SIZE), .DELAY(TMU_DELAY)) 
                step2_depthWDelay (.clk(aclk), .ce(ce), .in(step1_depthWFloat), .out(step2_depthWFloat));

            ValueDelay #(.VALUE_SIZE(1), .DELAY(TMU_DELAY)) 
                step2_validDelay (.clk(aclk), .ce(ce), .in(step1_valid), .out(step2_valid));
            ValueDelay #(.VALUE_SIZE(KEEP_WIDTH), .DELAY(TMU_DELAY)) 
                step2_keepDelay (.clk(aclk), .ce(ce), .in(step1_keep), .out(step2_keep));
            ValueDelay #(.VALUE_SIZE(1), .DELAY(TMU_DELAY)) 
                step2_lastDelay (.clk(aclk), .ce(ce), .in(step1_last), .out(step2_last));

            TextureMappingUnit #(
                .SUB_PIXEL_WIDTH(SUB_PIXEL_WIDTH),
                .ENABLE_LOD_CALC(ENABLE_LOD_CALC)
            ) tmu1 (
                .aclk(aclk),
                .resetn(resetn),
                .ce(ce),

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
                .mipmapS(step1_mipmap1S),
                .mipmapT(step1_mipmap1T),

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
            assign texel1Addr00 = 0;
            assign texel1Addr01 = 0;
            assign texel1Addr10 = 0;
            assign texel1Addr11 = 0;
        end
    endgenerate


    ////////////////////////////////////////////////////////////////////////////
    // STEP 3
    // Calculate Fog
    // Clocks: 6
    ////////////////////////////////////////////////////////////////////////////
    wire [PIXEL_WIDTH - 1 : 0]              step3_fragmentColor;
    wire [INDEX_WIDTH - 1 : 0]              step3_index;
    wire [SCREEN_POS_WIDTH - 1 : 0]         step3_screenPosX;
    wire [SCREEN_POS_WIDTH - 1 : 0]         step3_screenPosY;
    wire [31 : 0]                           step3_depth;
    wire                                    step3_valid;
    wire [KEEP_WIDTH - 1 : 0]               step3_keep;
    wire                                    step3_last;

    ValueDelay #(.VALUE_SIZE(INDEX_WIDTH), .DELAY(6)) 
        step3_indexDelay (.clk(aclk), .ce(ce), .in(step2_index), .out(step3_index));
    ValueDelay #(.VALUE_SIZE(SCREEN_POS_WIDTH), .DELAY(6)) 
        step3_screenPosXDelay (.clk(aclk), .ce(ce), .in(step2_screenPosX), .out(step3_screenPosX));
    ValueDelay #(.VALUE_SIZE(SCREEN_POS_WIDTH), .DELAY(6)) 
        step3_screenPosYDelay (.clk(aclk), .ce(ce), .in(step2_screenPosY), .out(step3_screenPosY));
    ValueDelay #(.VALUE_SIZE(32), .DELAY(6)) 
        step3_depthDelay (.clk(aclk), .ce(ce), .in(step2_depth), .out(step3_depth));
    ValueDelay #(.VALUE_SIZE(1), .DELAY(6)) 
        step3_validDelay (.clk(aclk), .ce(ce), .in(step2_valid), .out(step3_valid));
    ValueDelay #(.VALUE_SIZE(KEEP_WIDTH), .DELAY(6)) 
        step3_keepDelay (.clk(aclk), .ce(ce), .in(step2_keep), .out(step3_keep));
    ValueDelay #(.VALUE_SIZE(1), .DELAY(6)) 
        step3_lastDelay (.clk(aclk), .ce(ce), .in(step2_last), .out(step3_last));

    Fog #(
        .SUB_PIXEL_WIDTH(SUB_PIXEL_WIDTH)
    ) fog (
        .aclk(aclk),
        .resetn(resetn),
        .ce(ce),

        .s_fog_lut_axis_tvalid(s_fog_lut_axis_tvalid),
        .s_fog_lut_axis_tready(s_fog_lut_axis_tready),
        .s_fog_lut_axis_tlast(s_fog_lut_axis_tlast),
        .s_fog_lut_axis_tdata(s_fog_lut_axis_tdata),

        .confFogColor(confFragmentPipelineFogColor),
        .confEnable(confFeatureEnable[RENDER_CONFIG_FEATURE_ENABLE_FOG_POS]),

        .depth({step2_depthWFloat, {(32 - FLOAT_SIZE){1'b0}}}), // Fillup mantissa to cast back to 32 bit float
        .texelColor(step2_fragmentColor),

        .color(step3_fragmentColor)
    );

    assign m_frag_tfragmentColor = step3_fragmentColor;
    assign m_frag_tindex = step3_index;
    assign m_frag_tscreenPosX = step3_screenPosX;
    assign m_frag_tscreenPosY = step3_screenPosY;
    assign m_frag_tdepth = step3_depth;
    assign m_frag_tvalid = step3_valid;
    assign m_frag_tkeep = step3_keep;
    assign m_frag_tlast = step3_last;

endmodule


 
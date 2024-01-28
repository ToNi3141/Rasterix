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

// Converts float values from the attribute interpolator into fix point values
// for the pixel pipeline.
// Pipelined: yes
// Depth: 2 cycles
module AttributeF2XConverter #(
    parameter INDEX_WIDTH = 32,
    parameter SCREEN_POS_WIDTH = 11,
    parameter ENABLE_LOD_CALC = 1,
    parameter ENABLE_SECOND_TMU = 1,

    parameter SUB_PIXEL_WIDTH = 8,
    parameter DEPTH_WIDTH = 16,

    localparam FLOAT_SIZE = 32,
    localparam KEEP_WIDTH = 1
)
(
    input  wire                             aclk,
    input  wire                             resetn,
    
    input  wire                             s_ftx_tvalid,
    input  wire                             s_ftx_tlast,
    input  wire [KEEP_WIDTH - 1 : 0]        s_ftx_tkeep,
    input  wire [SCREEN_POS_WIDTH - 1 : 0]  s_ftx_tspx,
    input  wire [SCREEN_POS_WIDTH - 1 : 0]  s_ftx_tspy,
    input  wire [INDEX_WIDTH - 1 : 0]       s_ftx_tindex,
    input  wire [FLOAT_SIZE - 1 : 0]        s_ftx_tdepth_w,
    input  wire [FLOAT_SIZE - 1 : 0]        s_ftx_tdepth_z,
    input  wire [FLOAT_SIZE - 1 : 0]        s_ftx_ttexture0_t,
    input  wire [FLOAT_SIZE - 1 : 0]        s_ftx_ttexture0_s,
    input  wire [FLOAT_SIZE - 1 : 0]        s_ftx_tmipmap0_t,
    input  wire [FLOAT_SIZE - 1 : 0]        s_ftx_tmipmap0_s,
    input  wire [FLOAT_SIZE - 1 : 0]        s_ftx_ttexture1_t,
    input  wire [FLOAT_SIZE - 1 : 0]        s_ftx_ttexture1_s,
    input  wire [FLOAT_SIZE - 1 : 0]        s_ftx_tmipmap1_t,
    input  wire [FLOAT_SIZE - 1 : 0]        s_ftx_tmipmap1_s,
    input  wire [FLOAT_SIZE - 1 : 0]        s_ftx_tcolor_a,
    input  wire [FLOAT_SIZE - 1 : 0]        s_ftx_tcolor_b,
    input  wire [FLOAT_SIZE - 1 : 0]        s_ftx_tcolor_g,
    input  wire [FLOAT_SIZE - 1 : 0]        s_ftx_tcolor_r,

    // Pixel Stream Interpolated
    output wire                             m_ftx_tvalid,
    output wire                             m_ftx_tlast,
    output wire [KEEP_WIDTH - 1 : 0]        m_ftx_tkeep,
    output wire [SCREEN_POS_WIDTH - 1 : 0]  m_ftx_tspx,
    output wire [SCREEN_POS_WIDTH - 1 : 0]  m_ftx_tspy,
    output wire [INDEX_WIDTH - 1 : 0]       m_ftx_tindex,
    output wire [FLOAT_SIZE - 1 : 0]        m_ftx_tdepth_w, // Float
    output wire [31 : 0]                    m_ftx_tdepth_z, // Q16.16
    output wire [31 : 0]                    m_ftx_ttexture0_t, // S16.15
    output wire [31 : 0]                    m_ftx_ttexture0_s, // S16.15
    output wire [31 : 0]                    m_ftx_tmipmap0_t, // S16.15
    output wire [31 : 0]                    m_ftx_tmipmap0_s, // S16.15
    output wire [31 : 0]                    m_ftx_ttexture1_t, // S16.15
    output wire [31 : 0]                    m_ftx_ttexture1_s, // S16.15
    output wire [31 : 0]                    m_ftx_tmipmap1_t, // S16.15
    output wire [31 : 0]                    m_ftx_tmipmap1_s, // S16.15
    output wire [SUB_PIXEL_WIDTH - 1 : 0]   m_ftx_tcolor_a, // Qn
    output wire [SUB_PIXEL_WIDTH - 1 : 0]   m_ftx_tcolor_b, // Qn
    output wire [SUB_PIXEL_WIDTH - 1 : 0]   m_ftx_tcolor_g, // Qn
    output wire [SUB_PIXEL_WIDTH - 1 : 0]   m_ftx_tcolor_r // Qn
);
    localparam [SUB_PIXEL_WIDTH - 1 : 0] ONE_POINT_ZERO = { SUB_PIXEL_WIDTH { 1'h1 } };

    ////////////////////////////////////////////////////////////////////////////
    // STEP 0
    // Convert float to integer
    // Clocks: 2
    ////////////////////////////////////////////////////////////////////////////
    wire [INDEX_WIDTH - 1 : 0]  step_convert_framebuffer_index;
    wire [SCREEN_POS_WIDTH - 1 : 0] step_convert_screen_pos_x;
    wire [SCREEN_POS_WIDTH - 1 : 0] step_convert_screen_pos_y;
    wire [FLOAT_SIZE - 1 : 0]   step_convert_depth_w_float;
    wire [31 : 0]               step_convert_texture0_s;
    wire [31 : 0]               step_convert_texture0_t;
    wire [31 : 0]               step_convert_mipmap0_s;
    wire [31 : 0]               step_convert_mipmap0_t;
    wire [31 : 0]               step_convert_texture1_s;
    wire [31 : 0]               step_convert_texture1_t;
    wire [31 : 0]               step_convert_mipmap1_s;
    wire [31 : 0]               step_convert_mipmap1_t;
    wire [31 : 0]               step_convert_depth_z;
    wire [31 : 0]               step_convert_color_r;   
    wire [31 : 0]               step_convert_color_g;
    wire [31 : 0]               step_convert_color_b;
    wire [31 : 0]               step_convert_color_a;
    wire                        step_convert_tvalid;
    wire [KEEP_WIDTH - 1 : 0]   step_convert_tkeep;
    wire                        step_convert_tlast;

    localparam CONV_DELAY = 2;

    // Framebuffer Index
    ValueDelay #(.VALUE_SIZE(INDEX_WIDTH), .DELAY(CONV_DELAY)) 
        convert_framebuffer_delay (.clk(aclk), .in(s_ftx_tindex), .out(step_convert_framebuffer_index));

    // Screen Poisition
    ValueDelay #(.VALUE_SIZE(SCREEN_POS_WIDTH), .DELAY(CONV_DELAY)) 
        convert_screen_pos_x_delay (.clk(aclk), .in(s_ftx_tspx), .out(step_convert_screen_pos_x));
    ValueDelay #(.VALUE_SIZE(SCREEN_POS_WIDTH), .DELAY(CONV_DELAY)) 
        convert_screen_pos_y_delay (.clk(aclk), .in(s_ftx_tspy), .out(step_convert_screen_pos_y));

    // Fragment stream flags
    ValueDelay #(.VALUE_SIZE(1), .DELAY(CONV_DELAY)) 
        convert_valid_delay (.clk(aclk), .in(s_ftx_tvalid), .out(step_convert_tvalid));
    ValueDelay #(.VALUE_SIZE(KEEP_WIDTH), .DELAY(CONV_DELAY)) 
        convert_keep_delay (.clk(aclk), .in(s_ftx_tkeep), .out(step_convert_tkeep));
    ValueDelay #(.VALUE_SIZE(1), .DELAY(CONV_DELAY)) 
        convert_last_delay (.clk(aclk), .in(s_ftx_tlast), .out(step_convert_tlast));

    // Depth
    ValueDelay #(.VALUE_SIZE(FLOAT_SIZE), .DELAY(CONV_DELAY)) 
        convert_depth_delay (.clk(aclk), .in(s_ftx_tdepth_w), .out(step_convert_depth_w_float));
    FloatToInt #(.MANTISSA_SIZE(FLOAT_SIZE - 9), .EXPONENT_SIZE(8), .INT_SIZE(32), .EXPONENT_BIAS_OFFSET(-DEPTH_WIDTH), .DELAY(0))
        convert_floatToInt_DepthZ (.clk(aclk), .in(s_ftx_tdepth_z), .out(step_convert_depth_z)); 

    // Tex Coords TMU0
    FloatToInt #(.MANTISSA_SIZE(FLOAT_SIZE - 9), .EXPONENT_SIZE(8), .INT_SIZE(32), .EXPONENT_BIAS_OFFSET(-15), .DELAY(0))
        convert_floatToInt_tmu0_textureS (.clk(aclk), .in(s_ftx_ttexture0_s), .out(step_convert_texture0_s));
    FloatToInt #(.MANTISSA_SIZE(FLOAT_SIZE - 9), .EXPONENT_SIZE(8), .INT_SIZE(32), .EXPONENT_BIAS_OFFSET(-15), .DELAY(0))
        convert_floatToInt_tmu0_textureT (.clk(aclk), .in(s_ftx_ttexture0_t), .out(step_convert_texture0_t));   

    generate 
        if (ENABLE_LOD_CALC)
        begin
            FloatToInt #(.MANTISSA_SIZE(FLOAT_SIZE - 9), .EXPONENT_SIZE(8), .INT_SIZE(32), .EXPONENT_BIAS_OFFSET(-15), .DELAY(0))
                convert_floatToInt_tmu0_mipmapS (.clk(aclk), .in(s_ftx_tmipmap0_s), .out(step_convert_mipmap0_s));
            FloatToInt #(.MANTISSA_SIZE(FLOAT_SIZE - 9), .EXPONENT_SIZE(8), .INT_SIZE(32), .EXPONENT_BIAS_OFFSET(-15), .DELAY(0))
                convert_floatToInt_tmu0_mipmapT (.clk(aclk), .in(s_ftx_tmipmap0_t), .out(step_convert_mipmap0_t));   
        end
    endgenerate

    // Tex Coords TMU1
    generate 
        if (ENABLE_SECOND_TMU)
        begin

            FloatToInt #(.MANTISSA_SIZE(FLOAT_SIZE - 9), .EXPONENT_SIZE(8), .INT_SIZE(32), .EXPONENT_BIAS_OFFSET(-15), .DELAY(0))
                convert_floatToInt_tmu1_textureS (.clk(aclk), .in(s_ftx_ttexture1_s), .out(step_convert_texture1_s));
            FloatToInt #(.MANTISSA_SIZE(FLOAT_SIZE - 9), .EXPONENT_SIZE(8), .INT_SIZE(32), .EXPONENT_BIAS_OFFSET(-15), .DELAY(0))
                convert_floatToInt_tmu1_textureT (.clk(aclk), .in(s_ftx_ttexture1_t), .out(step_convert_texture1_t));

            if (ENABLE_LOD_CALC)
            begin
                FloatToInt #(.MANTISSA_SIZE(FLOAT_SIZE - 9), .EXPONENT_SIZE(8), .INT_SIZE(32), .EXPONENT_BIAS_OFFSET(-15), .DELAY(0))
                    convert_floatToInt_tmu1_mipmapS (.clk(aclk), .in(s_ftx_tmipmap1_s), .out(step_convert_mipmap1_s));
                FloatToInt #(.MANTISSA_SIZE(FLOAT_SIZE - 9), .EXPONENT_SIZE(8), .INT_SIZE(32), .EXPONENT_BIAS_OFFSET(-15), .DELAY(0))
                    convert_floatToInt_tmu1_mipmapT (.clk(aclk), .in(s_ftx_tmipmap1_t), .out(step_convert_mipmap1_t));        
            end
        end
    endgenerate

    // Fragment Color
    FloatToInt #(.MANTISSA_SIZE(FLOAT_SIZE - 9), .EXPONENT_SIZE(8), .INT_SIZE(32), .EXPONENT_BIAS_OFFSET(-16), .DELAY(0))
        convert_floatToInt_ColorR (.clk(aclk), .in(s_ftx_tcolor_r), .out(step_convert_color_r));
    FloatToInt #(.MANTISSA_SIZE(FLOAT_SIZE - 9), .EXPONENT_SIZE(8), .INT_SIZE(32), .EXPONENT_BIAS_OFFSET(-16), .DELAY(0))
        convert_floatToInt_ColorG (.clk(aclk), .in(s_ftx_tcolor_g), .out(step_convert_color_g));   
    FloatToInt #(.MANTISSA_SIZE(FLOAT_SIZE - 9), .EXPONENT_SIZE(8), .INT_SIZE(32), .EXPONENT_BIAS_OFFSET(-16), .DELAY(0))
        convert_floatToInt_ColorB (.clk(aclk), .in(s_ftx_tcolor_b), .out(step_convert_color_b));   
    FloatToInt #(.MANTISSA_SIZE(FLOAT_SIZE - 9), .EXPONENT_SIZE(8), .INT_SIZE(32), .EXPONENT_BIAS_OFFSET(-16), .DELAY(0))
        convert_floatToInt_ColorA (.clk(aclk), .in(s_ftx_tcolor_a), .out(step_convert_color_a));   

    ////////////////////////////////////////////////////////////////////////////
    // STEP 1 
    // Output calculated values
    // Clocks: 0
    ///////////////////////////////////////////////////////////////////////////
    assign m_ftx_tvalid = step_convert_tvalid;
    assign m_ftx_tlast = step_convert_tlast;
    assign m_ftx_tkeep = step_convert_tkeep;
    assign m_ftx_tspx = step_convert_screen_pos_x;
    assign m_ftx_tspy = step_convert_screen_pos_y;
    assign m_ftx_tindex = step_convert_framebuffer_index;
    assign m_ftx_tdepth_w = step_convert_depth_w_float;
    assign m_ftx_tdepth_z = step_convert_depth_z;
    assign m_ftx_ttexture0_t = step_convert_texture0_t;
    assign m_ftx_ttexture0_s = step_convert_texture0_s;
    generate
        if (ENABLE_LOD_CALC)
        begin
            assign m_ftx_tmipmap0_t = step_convert_mipmap0_t;
            assign m_ftx_tmipmap0_s = step_convert_mipmap0_s;
        end
        else
        begin
            assign m_ftx_tmipmap0_t = step_convert_texture0_t;
            assign m_ftx_tmipmap0_s = step_convert_texture0_s;
        end
    endgenerate
    assign m_ftx_ttexture1_t = step_convert_texture1_t;
    assign m_ftx_ttexture1_s = step_convert_texture1_s;
    generate
        if (ENABLE_LOD_CALC)
        begin
            assign m_ftx_tmipmap1_t = step_convert_mipmap1_t;
            assign m_ftx_tmipmap1_s = step_convert_mipmap1_s;
        end
        else
        begin
            assign m_ftx_tmipmap1_t = step_convert_texture1_t;
            assign m_ftx_tmipmap1_s = step_convert_texture1_s;
        end
    endgenerate
    assign m_ftx_tcolor_a = (|step_convert_color_a[16 +: 16]) ? ONE_POINT_ZERO : step_convert_color_a[16 - SUB_PIXEL_WIDTH +: SUB_PIXEL_WIDTH];
    assign m_ftx_tcolor_b = (|step_convert_color_b[16 +: 16]) ? ONE_POINT_ZERO : step_convert_color_b[16 - SUB_PIXEL_WIDTH +: SUB_PIXEL_WIDTH];
    assign m_ftx_tcolor_g = (|step_convert_color_g[16 +: 16]) ? ONE_POINT_ZERO : step_convert_color_g[16 - SUB_PIXEL_WIDTH +: SUB_PIXEL_WIDTH];
    assign m_ftx_tcolor_r = (|step_convert_color_r[16 +: 16]) ? ONE_POINT_ZERO : step_convert_color_r[16 - SUB_PIXEL_WIDTH +: SUB_PIXEL_WIDTH];

endmodule

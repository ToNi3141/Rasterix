// RasteriCEr
// https://github.com/ToNi3141/RasteriCEr
// Copyright (c) 2021 ToNi3141

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

module AttributeInterpolator #(
    parameter FLOAT_SIZE = 20,
    localparam AXIS_WORD_WIDTH = 32,
    localparam FRAMEBUFFER_INDEX_WIDTH = AXIS_WORD_WIDTH,
    localparam AXIS_OUT_SIZE = (5 * AXIS_WORD_WIDTH), // Framebuffer Index, Triangle color, s, t, w
    localparam RASTERIZER_STREAM_WIDTH = FRAMEBUFFER_INDEX_WIDTH + RASTERIZER_AXIS_PARAMETER_SIZE
)
(
    input wire                              clk,
    input wire                              reset,
    
    // Interpolation Control
    output reg                              pixelInPipeline,

    // Pixel Stream
    input  wire                             s_axis_tvalid,
    output wire                             s_axis_tready,
    input  wire                             s_axis_tlast,
    input  wire [RASTERIZER_STREAM_WIDTH - 1 : 0]  s_axis_tdata,

    // Pixel Stream Interpolated
    output wire                             m_axis_tvalid,
    input  wire                             m_axis_tready,
    output wire                             m_axis_tlast,
    output wire [AXIS_OUT_SIZE - 1 : 0]     m_axis_tdata
);
`include "RasterizerDefines.vh"
`include "RegisterAndDescriptorDefines.vh"
    localparam EXPONENT_SIZE = 8; // Size of a IEEE 754 32 bit float
    localparam MANTISSA_SIZE = FLOAT_SIZE - 1 - EXPONENT_SIZE; // Calculate the mantissa size by substracting from the FLOAT_SIZE the sign and exponent
    localparam FLOAT_SIZE_DIFF = RASTERIZER_AXIS_VERTEX_ATTRIBUTE_SIZE - FLOAT_SIZE;

    localparam INT_32_DIFF = AXIS_WORD_WIDTH - RASTERIZER_AXIS_SCREEN_POS_SIZE;

    localparam FRAMEBUFFER_INDEX_DELAY = 28; // 5 steps + 8 clk (reciprocal)

    // Selecting the vertex attributes from the axis
    // Convert them from a 32 bit float to a FLOAT_SIZE float. It can easily be done by cutting off bits from the mantissa as long as the exponent keeps it size.
    wire [FLOAT_SIZE - 1 : 0] inc_texture_s_x = s_axis_tdata[RASTERIZER_AXIS_INC_TEXTURE_S_X_POS + FLOAT_SIZE_DIFF +: RASTERIZER_AXIS_VERTEX_ATTRIBUTE_SIZE - FLOAT_SIZE_DIFF];
    wire [FLOAT_SIZE - 1 : 0] inc_texture_s_y = s_axis_tdata[RASTERIZER_AXIS_INC_TEXTURE_S_Y_POS + FLOAT_SIZE_DIFF +: RASTERIZER_AXIS_VERTEX_ATTRIBUTE_SIZE - FLOAT_SIZE_DIFF];
    wire [FLOAT_SIZE - 1 : 0] inc_texture_t_x = s_axis_tdata[RASTERIZER_AXIS_INC_TEXTURE_T_X_POS + FLOAT_SIZE_DIFF +: RASTERIZER_AXIS_VERTEX_ATTRIBUTE_SIZE - FLOAT_SIZE_DIFF];
    wire [FLOAT_SIZE - 1 : 0] inc_texture_t_y = s_axis_tdata[RASTERIZER_AXIS_INC_TEXTURE_T_Y_POS + FLOAT_SIZE_DIFF +: RASTERIZER_AXIS_VERTEX_ATTRIBUTE_SIZE - FLOAT_SIZE_DIFF];
    wire [FLOAT_SIZE - 1 : 0] inc_depth_w_x   = s_axis_tdata[RASTERIZER_AXIS_INC_DEPTH_W_X_POS   + FLOAT_SIZE_DIFF +: RASTERIZER_AXIS_VERTEX_ATTRIBUTE_SIZE - FLOAT_SIZE_DIFF];
    wire [FLOAT_SIZE - 1 : 0] inc_depth_w_y   = s_axis_tdata[RASTERIZER_AXIS_INC_DEPTH_W_Y_POS   + FLOAT_SIZE_DIFF +: RASTERIZER_AXIS_VERTEX_ATTRIBUTE_SIZE - FLOAT_SIZE_DIFF];

    // Screen position
    wire [RASTERIZER_AXIS_SCREEN_POS_SIZE - 1 : 0] screen_pos_x = s_axis_tdata[RASTERIZER_AXIS_SCREEN_POS_X_POS +: RASTERIZER_AXIS_SCREEN_POS_SIZE];
    wire [RASTERIZER_AXIS_SCREEN_POS_SIZE - 1 : 0] screen_pos_y = s_axis_tdata[RASTERIZER_AXIS_SCREEN_POS_Y_POS +: RASTERIZER_AXIS_SCREEN_POS_SIZE];

    // Static attributes
    wire [RASTERIZER_AXIS_TRIANGLE_COLOR_SIZE - 1 : 0] triangle_static_color = s_axis_tdata[RASTERIZER_AXIS_TRIANGLE_COLOR_POS +: RASTERIZER_AXIS_TRIANGLE_COLOR_SIZE];
    wire [FRAMEBUFFER_INDEX_WIDTH - 1 : 0] framebuffer_index = s_axis_tdata[RASTERIZER_AXIS_PARAMETER_SIZE +: FRAMEBUFFER_INDEX_WIDTH];

    ////////////////////////////////////////////////////////////////////////////
    // STEP 0 Setup delays for pass through values
    ////////////////////////////////////////////////////////////////////////////
    wire [FRAMEBUFFER_INDEX_WIDTH - 1 : 0] step_0_framebuffer_index; 
    wire [RASTERIZER_AXIS_TRIANGLE_COLOR_SIZE - 1 : 0] step_0_triangle_static_color;
    wire step_0_tvalid;
    wire step_0_tlast;
    ValueDelay #(.VALUE_SIZE(FRAMEBUFFER_INDEX_WIDTH), .DELAY(FRAMEBUFFER_INDEX_DELAY)) 
        step_0_delay_framebuffer_index (.clk(clk), .in(framebuffer_index), .out(step_0_framebuffer_index));
    ValueDelay #(.VALUE_SIZE(RASTERIZER_AXIS_TRIANGLE_COLOR_SIZE), .DELAY(FRAMEBUFFER_INDEX_DELAY)) 
        step_1_delay_triangle_color (.clk(clk), .in(triangle_static_color), .out(step_0_triangle_static_color));
    ValueDelay #(.VALUE_SIZE(1), .DELAY(FRAMEBUFFER_INDEX_DELAY)) 
        step_1_delay_tvalid (.clk(clk), .in(s_axis_tvalid), .out(step_0_tvalid));
    ValueDelay #(.VALUE_SIZE(1), .DELAY(FRAMEBUFFER_INDEX_DELAY)) 
        step_1_delay_tlast (.clk(clk), .in(s_axis_tlast), .out(step_0_tlast));

    ////////////////////////////////////////////////////////////////////////////
    // STEP 1 Convert screen positions integers to float 
    ////////////////////////////////////////////////////////////////////////////
    wire [FLOAT_SIZE - 1 : 0] step_1_screen_pos_x_float;
    wire [FLOAT_SIZE - 1 : 0] step_1_screen_pos_y_float;
    IntToFloat #(.MANTISSA_SIZE(MANTISSA_SIZE), .EXPONENT_SIZE(EXPONENT_SIZE), .INT_SIZE(32))
        intToFloatScreenX (.clk(clk), .in({{INT_32_DIFF{1'b0}}, screen_pos_x}), .out(step_1_screen_pos_x_float));
    IntToFloat #(.MANTISSA_SIZE(MANTISSA_SIZE), .EXPONENT_SIZE(EXPONENT_SIZE), .INT_SIZE(32))
        intToFloatScreenY (.clk(clk), .in({{INT_32_DIFF{1'b0}}, screen_pos_y}), .out(step_1_screen_pos_y_float));   

    wire [FLOAT_SIZE - 1 : 0] step_1_inc_texture_s_x;
    wire [FLOAT_SIZE - 1 : 0] step_1_inc_texture_s_y;
    wire [FLOAT_SIZE - 1 : 0] step_1_inc_texture_t_x;
    wire [FLOAT_SIZE - 1 : 0] step_1_inc_texture_t_y;
    wire [FLOAT_SIZE - 1 : 0] step_1_inc_depth_w_x;
    wire [FLOAT_SIZE - 1 : 0] step_1_inc_depth_w_y;
    ValueDelay #(.VALUE_SIZE(FLOAT_SIZE), .DELAY(4)) step_1_delay_sx (.clk(clk), .in(inc_texture_s_x), .out(step_1_inc_texture_s_x));
    ValueDelay #(.VALUE_SIZE(FLOAT_SIZE), .DELAY(4)) step_1_delay_sy (.clk(clk), .in(inc_texture_s_y), .out(step_1_inc_texture_s_y));
    ValueDelay #(.VALUE_SIZE(FLOAT_SIZE), .DELAY(4)) step_1_delay_tx (.clk(clk), .in(inc_texture_t_x), .out(step_1_inc_texture_t_x));
    ValueDelay #(.VALUE_SIZE(FLOAT_SIZE), .DELAY(4)) step_1_delay_ty (.clk(clk), .in(inc_texture_t_y), .out(step_1_inc_texture_t_y));
    ValueDelay #(.VALUE_SIZE(FLOAT_SIZE), .DELAY(4)) step_1_delay_dx (.clk(clk), .in(inc_depth_w_x), .out(step_1_inc_depth_w_x));
    ValueDelay #(.VALUE_SIZE(FLOAT_SIZE), .DELAY(4)) step_1_delay_dy (.clk(clk), .in(inc_depth_w_y), .out(step_1_inc_depth_w_y));

    ////////////////////////////////////////////////////////////////////////////
    // STEP 2 Multiply screen positions with the vertex attribute increments
    ////////////////////////////////////////////////////////////////////////////
    wire [FLOAT_SIZE - 1 : 0] step_2_inc_texture_s_x;
    wire [FLOAT_SIZE - 1 : 0] step_2_inc_texture_s_y;
    wire [FLOAT_SIZE - 1 : 0] step_2_inc_texture_t_x;
    wire [FLOAT_SIZE - 1 : 0] step_2_inc_texture_t_y;
    wire [FLOAT_SIZE - 1 : 0] step_2_inc_depth_w_x;
    wire [FLOAT_SIZE - 1 : 0] step_2_inc_depth_w_y;
    FloatMul #(.MANTISSA_SIZE(MANTISSA_SIZE), .EXPONENT_SIZE(EXPONENT_SIZE))
        inc_step_s_x(.clk(clk), .facAIn(step_1_screen_pos_x_float), .facBIn(step_1_inc_texture_s_x), .prod(step_2_inc_texture_s_x));
    FloatMul #(.MANTISSA_SIZE(MANTISSA_SIZE), .EXPONENT_SIZE(EXPONENT_SIZE))
        inc_step_s_y(.clk(clk), .facAIn(step_1_screen_pos_y_float), .facBIn(step_1_inc_texture_s_y), .prod(step_2_inc_texture_s_y));
    FloatMul #(.MANTISSA_SIZE(MANTISSA_SIZE), .EXPONENT_SIZE(EXPONENT_SIZE))
        inc_step_t_x(.clk(clk), .facAIn(step_1_screen_pos_x_float), .facBIn(step_1_inc_texture_t_x), .prod(step_2_inc_texture_t_x));
    FloatMul #(.MANTISSA_SIZE(MANTISSA_SIZE), .EXPONENT_SIZE(EXPONENT_SIZE))
        inc_step_t_y(.clk(clk), .facAIn(step_1_screen_pos_y_float), .facBIn(step_1_inc_texture_t_y), .prod(step_2_inc_texture_t_y));
    FloatMul #(.MANTISSA_SIZE(MANTISSA_SIZE), .EXPONENT_SIZE(EXPONENT_SIZE))
        inc_step_depth_x(.clk(clk), .facAIn(step_1_screen_pos_x_float), .facBIn(step_1_inc_depth_w_x), .prod(step_2_inc_depth_w_x));
    FloatMul #(.MANTISSA_SIZE(MANTISSA_SIZE), .EXPONENT_SIZE(EXPONENT_SIZE))
        inc_step_depth_y(.clk(clk), .facAIn(step_1_screen_pos_y_float), .facBIn(step_1_inc_depth_w_y), .prod(step_2_inc_depth_w_y));

    ////////////////////////////////////////////////////////////////////////////
    // STEP 3 Add vertex attributes to the final increment
    ////////////////////////////////////////////////////////////////////////////
    wire [FLOAT_SIZE - 1 : 0] step_3_texture_s_inv;
    wire [FLOAT_SIZE - 1 : 0] step_3_texture_t_inv;
    wire [FLOAT_SIZE - 1 : 0] step_3_depth_w_inv;
    FloatAdd #(.MANTISSA_SIZE(MANTISSA_SIZE), .EXPONENT_SIZE(EXPONENT_SIZE), .ENABLE_OPTIMIZATION(1))
        add_to_final_inc_s (.clk(clk), .aIn(step_2_inc_texture_s_x), .bIn(step_2_inc_texture_s_y), .sum(step_3_texture_s_inv));
    FloatAdd #(.MANTISSA_SIZE(MANTISSA_SIZE), .EXPONENT_SIZE(EXPONENT_SIZE), .ENABLE_OPTIMIZATION(1))
        add_to_final_inc_t (.clk(clk), .aIn(step_2_inc_texture_t_x), .bIn(step_2_inc_texture_t_y), .sum(step_3_texture_t_inv));
    FloatAdd #(.MANTISSA_SIZE(MANTISSA_SIZE), .EXPONENT_SIZE(EXPONENT_SIZE), .ENABLE_OPTIMIZATION(1))
        add_to_final_inc_d (.clk(clk), .aIn(step_2_inc_depth_w_x), .bIn(step_2_inc_depth_w_y), .sum(step_3_depth_w_inv));

    ////////////////////////////////////////////////////////////////////////////
    // STEP 4 Calculate w reciprocal
    ////////////////////////////////////////////////////////////////////////////
    wire [FLOAT_SIZE - 1 : 0] step_4_depth_w;
    FloatFastRecip2 #(.MANTISSA_SIZE(MANTISSA_SIZE))
        recip_depth_w (.clk(clk), .in(step_3_depth_w_inv), .out(step_4_depth_w));

    wire [FLOAT_SIZE - 1 : 0] step_4_texture_s_inv;
    wire [FLOAT_SIZE - 1 : 0] step_4_texture_t_inv;
    ValueDelay #(.VALUE_SIZE(FLOAT_SIZE), .DELAY(12)) step_3_delay_s (.clk(clk), .in(step_3_texture_s_inv), .out(step_4_texture_s_inv));
    ValueDelay #(.VALUE_SIZE(FLOAT_SIZE), .DELAY(12)) step_3_delay_t (.clk(clk), .in(step_3_texture_t_inv), .out(step_4_texture_t_inv));

    ////////////////////////////////////////////////////////////////////////////
    // STEP 5 Calculate final attribute value
    ////////////////////////////////////////////////////////////////////////////
    wire [FLOAT_SIZE - 1 : 0] step_5_texture_s;
    wire [FLOAT_SIZE - 1 : 0] step_5_texture_t;
    FloatMul #(.MANTISSA_SIZE(MANTISSA_SIZE), .EXPONENT_SIZE(EXPONENT_SIZE))
        mul_texture_s_w(.clk(clk), .facAIn(step_4_texture_s_inv), .facBIn(step_4_depth_w), .prod(step_5_texture_s));
    FloatMul #(.MANTISSA_SIZE(MANTISSA_SIZE), .EXPONENT_SIZE(EXPONENT_SIZE))
        mul_texture_t_w(.clk(clk), .facAIn(step_4_texture_t_inv), .facBIn(step_4_depth_w), .prod(step_5_texture_t));

    wire [FLOAT_SIZE - 1 : 0] step_5_depth_w;
    ValueDelay #(.VALUE_SIZE(FLOAT_SIZE), .DELAY(4)) step_4_delay_w (.clk(clk), .in(step_4_depth_w), .out(step_5_depth_w));


    ////////////////////////////////////////////////////////////////////////////
    // STEP 6 Output calculated values
    ////////////////////////////////////////////////////////////////////////////
    assign s_axis_tready = 1;
    assign m_axis_tvalid = step_0_tvalid;
    assign m_axis_tlast = step_0_tlast;
    assign m_axis_tdata = {
        step_0_framebuffer_index,
        {{(AXIS_WORD_WIDTH - RASTERIZER_AXIS_TRIANGLE_COLOR_SIZE){1'b0}}, step_0_triangle_static_color},
        {step_5_depth_w, {FLOAT_SIZE_DIFF{1'b0}}},
        {step_5_texture_t, {FLOAT_SIZE_DIFF{1'b0}}},
        {step_5_texture_s, {FLOAT_SIZE_DIFF{1'b0}}}
    };
endmodule

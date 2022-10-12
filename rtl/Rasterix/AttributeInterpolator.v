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

module AttributeInterpolator #(
    parameter FLOAT_SIZE = 32,

    localparam ATTRIBUTE_SIZE = 32,
    localparam RASTERIZER_AXIS_PARAMETER_SIZE = 3 * ATTRIBUTE_SIZE,
    localparam ATTR_INTERP_AXIS_PARAMETER_SIZE = 10 * ATTRIBUTE_SIZE
)
(
    input wire                              aclk,
    input wire                              resetn,
    
    // Interpolation Control
    output wire                             pixelInPipeline,

    // Pixel Stream
    input  wire                             s_axis_tvalid,
    output wire                             s_axis_tready,
    input  wire                             s_axis_tlast,
    input  wire [RASTERIZER_AXIS_PARAMETER_SIZE - 1 : 0] s_axis_tdata,

    // Pixel Stream Interpolated
    output wire                             m_axis_tvalid,
    input  wire                             m_axis_tready,
    output wire                             m_axis_tlast,
    output wire [ATTR_INTERP_AXIS_PARAMETER_SIZE - 1 : 0] m_axis_tdata,

    // Attributes
    input  wire [ATTRIBUTE_SIZE - 1 : 0]    tex_s,
    input  wire [ATTRIBUTE_SIZE - 1 : 0]    tex_t,
    input  wire [ATTRIBUTE_SIZE - 1 : 0]    tex_q,
    input  wire [ATTRIBUTE_SIZE - 1 : 0]    tex_s_inc_x,
    input  wire [ATTRIBUTE_SIZE - 1 : 0]    tex_t_inc_x,
    input  wire [ATTRIBUTE_SIZE - 1 : 0]    tex_q_inc_x,
    input  wire [ATTRIBUTE_SIZE - 1 : 0]    tex_s_inc_y,
    input  wire [ATTRIBUTE_SIZE - 1 : 0]    tex_t_inc_y,
    input  wire [ATTRIBUTE_SIZE - 1 : 0]    tex_q_inc_y,
    input  wire [ATTRIBUTE_SIZE - 1 : 0]    depth_w,
    input  wire [ATTRIBUTE_SIZE - 1 : 0]    depth_w_inc_x,
    input  wire [ATTRIBUTE_SIZE - 1 : 0]    depth_w_inc_y,
    input  wire [ATTRIBUTE_SIZE - 1 : 0]    depth_z,
    input  wire [ATTRIBUTE_SIZE - 1 : 0]    depth_z_inc_x,
    input  wire [ATTRIBUTE_SIZE - 1 : 0]    depth_z_inc_y,
    input  wire [ATTRIBUTE_SIZE - 1 : 0]    color_r,
    input  wire [ATTRIBUTE_SIZE - 1 : 0]    color_g,
    input  wire [ATTRIBUTE_SIZE - 1 : 0]    color_b,
    input  wire [ATTRIBUTE_SIZE - 1 : 0]    color_a,
    input  wire [ATTRIBUTE_SIZE - 1 : 0]    color_r_inc_x,
    input  wire [ATTRIBUTE_SIZE - 1 : 0]    color_g_inc_x,
    input  wire [ATTRIBUTE_SIZE - 1 : 0]    color_b_inc_x,
    input  wire [ATTRIBUTE_SIZE - 1 : 0]    color_a_inc_x,
    input  wire [ATTRIBUTE_SIZE - 1 : 0]    color_r_inc_y,
    input  wire [ATTRIBUTE_SIZE - 1 : 0]    color_g_inc_y,
    input  wire [ATTRIBUTE_SIZE - 1 : 0]    color_b_inc_y,
    input  wire [ATTRIBUTE_SIZE - 1 : 0]    color_a_inc_y
);
    localparam SCREEN_X_POS = 0;
    localparam SCREEN_Y_POS = 16;
    localparam AXIS_SCREEN_POS_SIZE = 16;
    localparam AXIS_BOUNDING_BOX_POS = 0;
    localparam AXIS_SCREEN_POS = 32;
    localparam AXIS_FRAMEBUFFER_INDEX_POS = 64;

    localparam EXPONENT_SIZE = 8; // Size of a IEEE 754 32 bit float
    localparam MANTISSA_SIZE = FLOAT_SIZE - 1 - EXPONENT_SIZE; // Calculate the mantissa size by substracting from the FLOAT_SIZE the sign and exponent
    localparam FLOAT_SIZE_DIFF = ATTRIBUTE_SIZE - FLOAT_SIZE;

    localparam INT_32_DIFF = ATTRIBUTE_SIZE - AXIS_SCREEN_POS_SIZE;

    localparam RECIP_DELAY = 25;
    localparam FRAMEBUFFER_INDEX_DELAY = 20 + RECIP_DELAY; // 6 steps

    // Selecting the vertex attributes from the axis
    // Convert them from a 32 bit float to a FLOAT_SIZE float. It can easily be done by cutting off bits from the mantissa as long as the exponent keeps it size.
    wire [FLOAT_SIZE - 1 : 0] inc_texture_s   = tex_s[FLOAT_SIZE_DIFF           +: ATTRIBUTE_SIZE - FLOAT_SIZE_DIFF];
    wire [FLOAT_SIZE - 1 : 0] inc_texture_s_x = tex_s_inc_x[FLOAT_SIZE_DIFF     +: ATTRIBUTE_SIZE - FLOAT_SIZE_DIFF];
    wire [FLOAT_SIZE - 1 : 0] inc_texture_s_y = tex_s_inc_y[FLOAT_SIZE_DIFF     +: ATTRIBUTE_SIZE - FLOAT_SIZE_DIFF];
    wire [FLOAT_SIZE - 1 : 0] inc_texture_t   = tex_t[FLOAT_SIZE_DIFF           +: ATTRIBUTE_SIZE - FLOAT_SIZE_DIFF];
    wire [FLOAT_SIZE - 1 : 0] inc_texture_t_x = tex_t_inc_x[FLOAT_SIZE_DIFF     +: ATTRIBUTE_SIZE - FLOAT_SIZE_DIFF];
    wire [FLOAT_SIZE - 1 : 0] inc_texture_t_y = tex_t_inc_y[FLOAT_SIZE_DIFF     +: ATTRIBUTE_SIZE - FLOAT_SIZE_DIFF];
    wire [FLOAT_SIZE - 1 : 0] inc_texture_q   = tex_q[FLOAT_SIZE_DIFF           +: ATTRIBUTE_SIZE - FLOAT_SIZE_DIFF];
    wire [FLOAT_SIZE - 1 : 0] inc_texture_q_x = tex_q_inc_x[FLOAT_SIZE_DIFF     +: ATTRIBUTE_SIZE - FLOAT_SIZE_DIFF];
    wire [FLOAT_SIZE - 1 : 0] inc_texture_q_y = tex_q_inc_y[FLOAT_SIZE_DIFF     +: ATTRIBUTE_SIZE - FLOAT_SIZE_DIFF];
    wire [FLOAT_SIZE - 1 : 0] inc_depth_w     = depth_w[FLOAT_SIZE_DIFF         +: ATTRIBUTE_SIZE - FLOAT_SIZE_DIFF];
    wire [FLOAT_SIZE - 1 : 0] inc_depth_w_x   = depth_w_inc_x[FLOAT_SIZE_DIFF   +: ATTRIBUTE_SIZE - FLOAT_SIZE_DIFF];
    wire [FLOAT_SIZE - 1 : 0] inc_depth_w_y   = depth_w_inc_y[FLOAT_SIZE_DIFF   +: ATTRIBUTE_SIZE - FLOAT_SIZE_DIFF];
    wire [FLOAT_SIZE - 1 : 0] inc_depth_z     = depth_z[FLOAT_SIZE_DIFF         +: ATTRIBUTE_SIZE - FLOAT_SIZE_DIFF];
    wire [FLOAT_SIZE - 1 : 0] inc_depth_z_x   = depth_z_inc_x[FLOAT_SIZE_DIFF   +: ATTRIBUTE_SIZE - FLOAT_SIZE_DIFF];
    wire [FLOAT_SIZE - 1 : 0] inc_depth_z_y   = depth_z_inc_y[FLOAT_SIZE_DIFF   +: ATTRIBUTE_SIZE - FLOAT_SIZE_DIFF];
    wire [FLOAT_SIZE - 1 : 0] inc_color_r     = color_r[FLOAT_SIZE_DIFF         +: ATTRIBUTE_SIZE - FLOAT_SIZE_DIFF];
    wire [FLOAT_SIZE - 1 : 0] inc_color_g     = color_g[FLOAT_SIZE_DIFF         +: ATTRIBUTE_SIZE - FLOAT_SIZE_DIFF];
    wire [FLOAT_SIZE - 1 : 0] inc_color_b     = color_b[FLOAT_SIZE_DIFF         +: ATTRIBUTE_SIZE - FLOAT_SIZE_DIFF];
    wire [FLOAT_SIZE - 1 : 0] inc_color_a     = color_a[FLOAT_SIZE_DIFF         +: ATTRIBUTE_SIZE - FLOAT_SIZE_DIFF];
    wire [FLOAT_SIZE - 1 : 0] inc_color_r_x   = color_r_inc_x[FLOAT_SIZE_DIFF   +: ATTRIBUTE_SIZE - FLOAT_SIZE_DIFF];
    wire [FLOAT_SIZE - 1 : 0] inc_color_g_x   = color_g_inc_x[FLOAT_SIZE_DIFF   +: ATTRIBUTE_SIZE - FLOAT_SIZE_DIFF];
    wire [FLOAT_SIZE - 1 : 0] inc_color_b_x   = color_b_inc_x[FLOAT_SIZE_DIFF   +: ATTRIBUTE_SIZE - FLOAT_SIZE_DIFF];
    wire [FLOAT_SIZE - 1 : 0] inc_color_a_x   = color_a_inc_x[FLOAT_SIZE_DIFF   +: ATTRIBUTE_SIZE - FLOAT_SIZE_DIFF];
    wire [FLOAT_SIZE - 1 : 0] inc_color_r_y   = color_r_inc_y[FLOAT_SIZE_DIFF   +: ATTRIBUTE_SIZE - FLOAT_SIZE_DIFF];
    wire [FLOAT_SIZE - 1 : 0] inc_color_g_y   = color_g_inc_y[FLOAT_SIZE_DIFF   +: ATTRIBUTE_SIZE - FLOAT_SIZE_DIFF];
    wire [FLOAT_SIZE - 1 : 0] inc_color_b_y   = color_b_inc_y[FLOAT_SIZE_DIFF   +: ATTRIBUTE_SIZE - FLOAT_SIZE_DIFF];
    wire [FLOAT_SIZE - 1 : 0] inc_color_a_y   = color_a_inc_y[FLOAT_SIZE_DIFF   +: ATTRIBUTE_SIZE - FLOAT_SIZE_DIFF];

    // Screen position
    wire [AXIS_SCREEN_POS_SIZE - 1 : 0] screen_pos_x = s_axis_tdata[AXIS_SCREEN_POS + SCREEN_X_POS +: AXIS_SCREEN_POS_SIZE];
    wire [AXIS_SCREEN_POS_SIZE - 1 : 0] screen_pos_y = s_axis_tdata[AXIS_SCREEN_POS + SCREEN_Y_POS +: AXIS_SCREEN_POS_SIZE];

    // Bounding box position
    wire [AXIS_SCREEN_POS_SIZE - 1 : 0] bounding_box_pos_x = s_axis_tdata[AXIS_BOUNDING_BOX_POS + SCREEN_X_POS +: AXIS_SCREEN_POS_SIZE];
    wire [AXIS_SCREEN_POS_SIZE - 1 : 0] bounding_box_pos_y = s_axis_tdata[AXIS_BOUNDING_BOX_POS + SCREEN_Y_POS +: AXIS_SCREEN_POS_SIZE];

    // Static attributes
    wire [ATTRIBUTE_SIZE - 1 : 0] framebuffer_index = s_axis_tdata[AXIS_FRAMEBUFFER_INDEX_POS +: ATTRIBUTE_SIZE];

    ////////////////////////////////////////////////////////////////////////////
    // STEP 0 Setup delays for pass through values
    ////////////////////////////////////////////////////////////////////////////
    wire [ATTRIBUTE_SIZE - 1 : 0] step_0_framebuffer_index; 
    wire [AXIS_SCREEN_POS_SIZE - 1 : 0] step_0_screen_pos_x; 
    wire [AXIS_SCREEN_POS_SIZE - 1 : 0] step_0_screen_pos_y; 
    wire step_0_tvalid;
    wire step_0_tlast;
    ValueDelay #(.VALUE_SIZE(ATTRIBUTE_SIZE), .DELAY(FRAMEBUFFER_INDEX_DELAY)) 
        step_0_delay_framebuffer_index (.clk(aclk), .in(framebuffer_index), .out(step_0_framebuffer_index));

    ValueDelay #(.VALUE_SIZE(AXIS_SCREEN_POS_SIZE), .DELAY(FRAMEBUFFER_INDEX_DELAY)) 
        step_0_delay_screen_pos_x (.clk(aclk), .in(screen_pos_x), .out(step_0_screen_pos_x));
    ValueDelay #(.VALUE_SIZE(AXIS_SCREEN_POS_SIZE), .DELAY(FRAMEBUFFER_INDEX_DELAY)) 
        step_0_delay_screen_pos_y (.clk(aclk), .in(screen_pos_y), .out(step_0_screen_pos_y));

    ValueDelay #(.VALUE_SIZE(1), .DELAY(FRAMEBUFFER_INDEX_DELAY)) 
        step_1_delay_tvalid (.clk(aclk), .in(s_axis_tvalid), .out(step_0_tvalid));

    ValueDelay #(.VALUE_SIZE(1), .DELAY(FRAMEBUFFER_INDEX_DELAY)) 
        step_1_delay_tlast (.clk(aclk), .in(s_axis_tlast), .out(step_0_tlast));

    ValueTrack pixelTracker (
        .aclk(aclk),
        .resetn(resetn),
        
        .sigIncommingValue(s_axis_tvalid & s_axis_tready),
        .sigOutgoingValue(m_axis_tvalid & m_axis_tready),
        .valueInPipeline(pixelInPipeline)
    );

    assign s_axis_tready = 1;

    ////////////////////////////////////////////////////////////////////////////
    // STEP 1 Convert bounding box positions integers to float 
    ////////////////////////////////////////////////////////////////////////////
    wire [FLOAT_SIZE - 1 : 0] step_1_bounding_box_pos_x_float;
    wire [FLOAT_SIZE - 1 : 0] step_1_bounding_box_pos_y_float;
    IntToFloat #(.MANTISSA_SIZE(MANTISSA_SIZE), .EXPONENT_SIZE(EXPONENT_SIZE), .INT_SIZE(32))
        intToFloatBBX (.clk(aclk), .in({{INT_32_DIFF{1'b0}}, bounding_box_pos_x}), .out(step_1_bounding_box_pos_x_float));
    IntToFloat #(.MANTISSA_SIZE(MANTISSA_SIZE), .EXPONENT_SIZE(EXPONENT_SIZE), .INT_SIZE(32))
        intToFloatBBY (.clk(aclk), .in({{INT_32_DIFF{1'b0}}, bounding_box_pos_y}), .out(step_1_bounding_box_pos_y_float));   

    ////////////////////////////////////////////////////////////////////////////
    // STEP 2 Multiply bounding box positions with the vertex attribute increments
    ////////////////////////////////////////////////////////////////////////////
    wire [FLOAT_SIZE - 1 : 0] step_2_inc_texture_s_x;
    wire [FLOAT_SIZE - 1 : 0] step_2_inc_texture_s_y;
    wire [FLOAT_SIZE - 1 : 0] step_2_inc_texture_t_x;
    wire [FLOAT_SIZE - 1 : 0] step_2_inc_texture_t_y;
    wire [FLOAT_SIZE - 1 : 0] step_2_inc_texture_q_x;
    wire [FLOAT_SIZE - 1 : 0] step_2_inc_texture_q_y;

    wire [FLOAT_SIZE - 1 : 0] step_2_inc_depth_w_x;
    wire [FLOAT_SIZE - 1 : 0] step_2_inc_depth_w_y;
    wire [FLOAT_SIZE - 1 : 0] step_2_inc_depth_z_x;
    wire [FLOAT_SIZE - 1 : 0] step_2_inc_depth_z_y;

    wire [FLOAT_SIZE - 1 : 0] step_2_inc_color_r_x;
    wire [FLOAT_SIZE - 1 : 0] step_2_inc_color_g_x;
    wire [FLOAT_SIZE - 1 : 0] step_2_inc_color_b_x;
    wire [FLOAT_SIZE - 1 : 0] step_2_inc_color_a_x;
    wire [FLOAT_SIZE - 1 : 0] step_2_inc_color_r_y;
    wire [FLOAT_SIZE - 1 : 0] step_2_inc_color_g_y;
    wire [FLOAT_SIZE - 1 : 0] step_2_inc_color_b_y;
    wire [FLOAT_SIZE - 1 : 0] step_2_inc_color_a_y;

    FloatMul #(.MANTISSA_SIZE(MANTISSA_SIZE), .EXPONENT_SIZE(EXPONENT_SIZE))
        inc_step_s_x(.clk(aclk), .facAIn(step_1_bounding_box_pos_x_float), .facBIn(inc_texture_s_x), .prod(step_2_inc_texture_s_x));
    FloatMul #(.MANTISSA_SIZE(MANTISSA_SIZE), .EXPONENT_SIZE(EXPONENT_SIZE))
        inc_step_s_y(.clk(aclk), .facAIn(step_1_bounding_box_pos_y_float), .facBIn(inc_texture_s_y), .prod(step_2_inc_texture_s_y));
    FloatMul #(.MANTISSA_SIZE(MANTISSA_SIZE), .EXPONENT_SIZE(EXPONENT_SIZE))
        inc_step_t_x(.clk(aclk), .facAIn(step_1_bounding_box_pos_x_float), .facBIn(inc_texture_t_x), .prod(step_2_inc_texture_t_x));
    FloatMul #(.MANTISSA_SIZE(MANTISSA_SIZE), .EXPONENT_SIZE(EXPONENT_SIZE))
        inc_step_t_y(.clk(aclk), .facAIn(step_1_bounding_box_pos_y_float), .facBIn(inc_texture_t_y), .prod(step_2_inc_texture_t_y));
    FloatMul #(.MANTISSA_SIZE(MANTISSA_SIZE), .EXPONENT_SIZE(EXPONENT_SIZE))
        inc_step_tex_q_x(.clk(aclk), .facAIn(step_1_bounding_box_pos_x_float), .facBIn(inc_texture_q_x), .prod(step_2_inc_texture_q_x));
    FloatMul #(.MANTISSA_SIZE(MANTISSA_SIZE), .EXPONENT_SIZE(EXPONENT_SIZE))
        inc_step_tex_q_y(.clk(aclk), .facAIn(step_1_bounding_box_pos_y_float), .facBIn(inc_texture_q_y), .prod(step_2_inc_texture_q_y)); 
    
    FloatMul #(.MANTISSA_SIZE(MANTISSA_SIZE), .EXPONENT_SIZE(EXPONENT_SIZE))
        inc_step_depth_w_x(.clk(aclk), .facAIn(step_1_bounding_box_pos_x_float), .facBIn(inc_depth_w_x), .prod(step_2_inc_depth_w_x));
    FloatMul #(.MANTISSA_SIZE(MANTISSA_SIZE), .EXPONENT_SIZE(EXPONENT_SIZE))
        inc_step_depth_w_y(.clk(aclk), .facAIn(step_1_bounding_box_pos_y_float), .facBIn(inc_depth_w_y), .prod(step_2_inc_depth_w_y)); 
    FloatMul #(.MANTISSA_SIZE(MANTISSA_SIZE), .EXPONENT_SIZE(EXPONENT_SIZE))
        inc_step_depth_z_x(.clk(aclk), .facAIn(step_1_bounding_box_pos_x_float), .facBIn(inc_depth_z_x), .prod(step_2_inc_depth_z_x));
    FloatMul #(.MANTISSA_SIZE(MANTISSA_SIZE), .EXPONENT_SIZE(EXPONENT_SIZE))
        inc_step_depth_z_y(.clk(aclk), .facAIn(step_1_bounding_box_pos_y_float), .facBIn(inc_depth_z_y), .prod(step_2_inc_depth_z_y));

    FloatMul #(.MANTISSA_SIZE(MANTISSA_SIZE), .EXPONENT_SIZE(EXPONENT_SIZE))
        inc_step_color_r_x(.clk(aclk), .facAIn(step_1_bounding_box_pos_x_float), .facBIn(inc_color_r_x), .prod(step_2_inc_color_r_x));
    FloatMul #(.MANTISSA_SIZE(MANTISSA_SIZE), .EXPONENT_SIZE(EXPONENT_SIZE))
        inc_step_color_t_x(.clk(aclk), .facAIn(step_1_bounding_box_pos_x_float), .facBIn(inc_color_g_x), .prod(step_2_inc_color_g_x));
    FloatMul #(.MANTISSA_SIZE(MANTISSA_SIZE), .EXPONENT_SIZE(EXPONENT_SIZE))
        inc_step_color_b_x(.clk(aclk), .facAIn(step_1_bounding_box_pos_x_float), .facBIn(inc_color_b_x), .prod(step_2_inc_color_b_x));
    FloatMul #(.MANTISSA_SIZE(MANTISSA_SIZE), .EXPONENT_SIZE(EXPONENT_SIZE))
        inc_step_color_a_x(.clk(aclk), .facAIn(step_1_bounding_box_pos_x_float), .facBIn(inc_color_a_x), .prod(step_2_inc_color_a_x));
    FloatMul #(.MANTISSA_SIZE(MANTISSA_SIZE), .EXPONENT_SIZE(EXPONENT_SIZE))
        inc_step_color_r_y(.clk(aclk), .facAIn(step_1_bounding_box_pos_y_float), .facBIn(inc_color_r_y), .prod(step_2_inc_color_r_y));
    FloatMul #(.MANTISSA_SIZE(MANTISSA_SIZE), .EXPONENT_SIZE(EXPONENT_SIZE))
        inc_step_color_g_y(.clk(aclk), .facAIn(step_1_bounding_box_pos_y_float), .facBIn(inc_color_g_y), .prod(step_2_inc_color_g_y));
    FloatMul #(.MANTISSA_SIZE(MANTISSA_SIZE), .EXPONENT_SIZE(EXPONENT_SIZE))
        inc_step_color_b_y(.clk(aclk), .facAIn(step_1_bounding_box_pos_y_float), .facBIn(inc_color_b_y), .prod(step_2_inc_color_b_y));
    FloatMul #(.MANTISSA_SIZE(MANTISSA_SIZE), .EXPONENT_SIZE(EXPONENT_SIZE))
        inc_step_color_a_y(.clk(aclk), .facAIn(step_1_bounding_box_pos_y_float), .facBIn(inc_color_a_y), .prod(step_2_inc_color_a_y));

    ////////////////////////////////////////////////////////////////////////////
    // STEP 3 Add vertex attributes to the final increment
    ////////////////////////////////////////////////////////////////////////////
    wire [FLOAT_SIZE - 1 : 0] step_3_texture_s;
    wire [FLOAT_SIZE - 1 : 0] step_3_texture_t;
    wire [FLOAT_SIZE - 1 : 0] step_3_texture_q;
       
    wire [FLOAT_SIZE - 1 : 0] step_3_depth_w;
    wire [FLOAT_SIZE - 1 : 0] step_3_depth_z;

    wire [FLOAT_SIZE - 1 : 0] step_3_color_r;
    wire [FLOAT_SIZE - 1 : 0] step_3_color_g;
    wire [FLOAT_SIZE - 1 : 0] step_3_color_b;
    wire [FLOAT_SIZE - 1 : 0] step_3_color_a;

    FloatAdd #(.MANTISSA_SIZE(MANTISSA_SIZE), .EXPONENT_SIZE(EXPONENT_SIZE), .ENABLE_OPTIMIZATION(1))
        add_to_final_inc_s (.clk(aclk), .aIn(step_2_inc_texture_s_x), .bIn(step_2_inc_texture_s_y), .sum(step_3_texture_s));
    FloatAdd #(.MANTISSA_SIZE(MANTISSA_SIZE), .EXPONENT_SIZE(EXPONENT_SIZE), .ENABLE_OPTIMIZATION(1))
        add_to_final_inc_t (.clk(aclk), .aIn(step_2_inc_texture_t_x), .bIn(step_2_inc_texture_t_y), .sum(step_3_texture_t));
    FloatAdd #(.MANTISSA_SIZE(MANTISSA_SIZE), .EXPONENT_SIZE(EXPONENT_SIZE), .ENABLE_OPTIMIZATION(1))
        add_to_final_inc_q (.clk(aclk), .aIn(step_2_inc_texture_q_x), .bIn(step_2_inc_texture_q_y), .sum(step_3_texture_q));

    FloatAdd #(.MANTISSA_SIZE(MANTISSA_SIZE), .EXPONENT_SIZE(EXPONENT_SIZE), .ENABLE_OPTIMIZATION(1))
        add_to_final_inc_d_w (.clk(aclk), .aIn(step_2_inc_depth_w_x), .bIn(step_2_inc_depth_w_y), .sum(step_3_depth_w));
    FloatAdd #(.MANTISSA_SIZE(MANTISSA_SIZE), .EXPONENT_SIZE(EXPONENT_SIZE), .ENABLE_OPTIMIZATION(1))
        add_to_final_inc_d_z (.clk(aclk), .aIn(step_2_inc_depth_z_x), .bIn(step_2_inc_depth_z_y), .sum(step_3_depth_z));


    FloatAdd #(.MANTISSA_SIZE(MANTISSA_SIZE), .EXPONENT_SIZE(EXPONENT_SIZE), .ENABLE_OPTIMIZATION(1))
        add_to_final_inc_color_r (.clk(aclk), .aIn(step_2_inc_color_r_x), .bIn(step_2_inc_color_r_y), .sum(step_3_color_r));
    FloatAdd #(.MANTISSA_SIZE(MANTISSA_SIZE), .EXPONENT_SIZE(EXPONENT_SIZE), .ENABLE_OPTIMIZATION(1))
        add_to_final_inc_color_g (.clk(aclk), .aIn(step_2_inc_color_g_x), .bIn(step_2_inc_color_g_y), .sum(step_3_color_g));
    FloatAdd #(.MANTISSA_SIZE(MANTISSA_SIZE), .EXPONENT_SIZE(EXPONENT_SIZE), .ENABLE_OPTIMIZATION(1))
        add_to_final_inc_color_b (.clk(aclk), .aIn(step_2_inc_color_b_x), .bIn(step_2_inc_color_b_y), .sum(step_3_color_b));
    FloatAdd #(.MANTISSA_SIZE(MANTISSA_SIZE), .EXPONENT_SIZE(EXPONENT_SIZE), .ENABLE_OPTIMIZATION(1))
        add_to_final_inc_color_a (.clk(aclk), .aIn(step_2_inc_color_a_x), .bIn(step_2_inc_color_a_y), .sum(step_3_color_a));

    ////////////////////////////////////////////////////////////////////////////
    // STEP 4 Add final increment to the base
    ////////////////////////////////////////////////////////////////////////////
    wire [FLOAT_SIZE - 1 : 0] step_4_texture_s_inv;
    wire [FLOAT_SIZE - 1 : 0] step_4_texture_t_inv;
    wire [FLOAT_SIZE - 1 : 0] step_4_texture_q_inv;

    wire [FLOAT_SIZE - 1 : 0] step_4_depth_w_inv;
    wire [FLOAT_SIZE - 1 : 0] step_4_depth_z_inv;

    wire [FLOAT_SIZE - 1 : 0] step_4_color_r_inv;
    wire [FLOAT_SIZE - 1 : 0] step_4_color_g_inv;
    wire [FLOAT_SIZE - 1 : 0] step_4_color_b_inv;
    wire [FLOAT_SIZE - 1 : 0] step_4_color_a_inv;

    FloatAdd #(.MANTISSA_SIZE(MANTISSA_SIZE), .EXPONENT_SIZE(EXPONENT_SIZE), .ENABLE_OPTIMIZATION(1))
        add_to_base_s (.clk(aclk), .aIn(step_3_texture_s), .bIn(inc_texture_s), .sum(step_4_texture_s_inv));
    FloatAdd #(.MANTISSA_SIZE(MANTISSA_SIZE), .EXPONENT_SIZE(EXPONENT_SIZE), .ENABLE_OPTIMIZATION(1))
        add_to_base_t (.clk(aclk), .aIn(step_3_texture_t), .bIn(inc_texture_t), .sum(step_4_texture_t_inv));
    FloatAdd #(.MANTISSA_SIZE(MANTISSA_SIZE), .EXPONENT_SIZE(EXPONENT_SIZE), .ENABLE_OPTIMIZATION(1))
        add_to_base_q (.clk(aclk), .aIn(step_3_texture_q), .bIn(inc_texture_q), .sum(step_4_texture_q_inv));

    FloatAdd #(.MANTISSA_SIZE(MANTISSA_SIZE), .EXPONENT_SIZE(EXPONENT_SIZE), .ENABLE_OPTIMIZATION(1))
        add_to_base_d_w (.clk(aclk), .aIn(step_3_depth_w), .bIn(inc_depth_w), .sum(step_4_depth_w_inv));
    FloatAdd #(.MANTISSA_SIZE(MANTISSA_SIZE), .EXPONENT_SIZE(EXPONENT_SIZE), .ENABLE_OPTIMIZATION(1))
        add_to_base_d_z (.clk(aclk), .aIn(step_3_depth_z), .bIn(inc_depth_z), .sum(step_4_depth_z_inv));

    FloatAdd #(.MANTISSA_SIZE(MANTISSA_SIZE), .EXPONENT_SIZE(EXPONENT_SIZE), .ENABLE_OPTIMIZATION(1))
        add_to_base_color_r (.clk(aclk), .aIn(step_3_color_r), .bIn(inc_color_r), .sum(step_4_color_r_inv));
    FloatAdd #(.MANTISSA_SIZE(MANTISSA_SIZE), .EXPONENT_SIZE(EXPONENT_SIZE), .ENABLE_OPTIMIZATION(1))
        add_to_base_color_g (.clk(aclk), .aIn(step_3_color_g), .bIn(inc_color_g), .sum(step_4_color_g_inv));
    FloatAdd #(.MANTISSA_SIZE(MANTISSA_SIZE), .EXPONENT_SIZE(EXPONENT_SIZE), .ENABLE_OPTIMIZATION(1))
        add_to_base_color_b (.clk(aclk), .aIn(step_3_color_b), .bIn(inc_color_b), .sum(step_4_color_b_inv));
    FloatAdd #(.MANTISSA_SIZE(MANTISSA_SIZE), .EXPONENT_SIZE(EXPONENT_SIZE), .ENABLE_OPTIMIZATION(1))
        add_to_base_color_a (.clk(aclk), .aIn(step_3_color_a), .bIn(inc_color_a), .sum(step_4_color_a_inv));

    ////////////////////////////////////////////////////////////////////////////
    // STEP 5 Calculate w reciprocal
    ////////////////////////////////////////////////////////////////////////////
    wire [FLOAT_SIZE - 1 : 0] step_5_texture_q;
    wire [FLOAT_SIZE - 1 : 0] step_5_depth_w;

    FloatFastRecip2 #(.MANTISSA_SIZE(MANTISSA_SIZE), .ITERATIONS(3))
        recip_tex_q (.clk(aclk), .in(step_4_texture_q_inv), .out(step_5_texture_q));
    
    // Use the cheap reciprocal calculation. It should be sufficient for fog.
    wire [FLOAT_SIZE - 1 : 0] step_5_depth_w_tmp;
    FloatFastRecip #(.MANTISSA_SIZE(MANTISSA_SIZE))
        recip_depth_w (.clk(aclk), .in(step_4_depth_w_inv), .out(step_5_depth_w_tmp));
    ValueDelay #(.VALUE_SIZE(FLOAT_SIZE), .DELAY(RECIP_DELAY - 4)) step_5_delay_d_w (.clk(aclk), .in(step_5_depth_w_tmp), .out(step_5_depth_w));

    wire [FLOAT_SIZE - 1 : 0] step_5_texture_s_inv;
    wire [FLOAT_SIZE - 1 : 0] step_5_texture_t_inv;

    wire [FLOAT_SIZE - 1 : 0] step_5_depth_z_inv;

    wire [FLOAT_SIZE - 1 : 0] step_5_color_r_inv;
    wire [FLOAT_SIZE - 1 : 0] step_5_color_g_inv;
    wire [FLOAT_SIZE - 1 : 0] step_5_color_b_inv;
    wire [FLOAT_SIZE - 1 : 0] step_5_color_a_inv;

    ValueDelay #(.VALUE_SIZE(FLOAT_SIZE), .DELAY(RECIP_DELAY)) step_5_delay_s (.clk(aclk), .in(step_4_texture_s_inv), .out(step_5_texture_s_inv));
    ValueDelay #(.VALUE_SIZE(FLOAT_SIZE), .DELAY(RECIP_DELAY)) step_5_delay_t (.clk(aclk), .in(step_4_texture_t_inv), .out(step_5_texture_t_inv));

    ValueDelay #(.VALUE_SIZE(FLOAT_SIZE), .DELAY(RECIP_DELAY)) step_5_delay_d_z (.clk(aclk), .in(step_4_depth_z_inv), .out(step_5_depth_z_inv));

    ValueDelay #(.VALUE_SIZE(FLOAT_SIZE), .DELAY(RECIP_DELAY)) step_5_delay_color_r (.clk(aclk), .in(step_4_color_r_inv), .out(step_5_color_r_inv));
    ValueDelay #(.VALUE_SIZE(FLOAT_SIZE), .DELAY(RECIP_DELAY)) step_5_delay_color_g (.clk(aclk), .in(step_4_color_g_inv), .out(step_5_color_g_inv));
    ValueDelay #(.VALUE_SIZE(FLOAT_SIZE), .DELAY(RECIP_DELAY)) step_5_delay_color_b (.clk(aclk), .in(step_4_color_b_inv), .out(step_5_color_b_inv));
    ValueDelay #(.VALUE_SIZE(FLOAT_SIZE), .DELAY(RECIP_DELAY)) step_5_delay_color_a (.clk(aclk), .in(step_4_color_a_inv), .out(step_5_color_a_inv));

    ////////////////////////////////////////////////////////////////////////////
    // STEP 6 Calculate final attribute value
    ////////////////////////////////////////////////////////////////////////////
    wire [FLOAT_SIZE - 1 : 0] step_6_texture_s;
    wire [FLOAT_SIZE - 1 : 0] step_6_texture_t;

    wire [FLOAT_SIZE - 1 : 0] step_6_depth_w;
    wire [FLOAT_SIZE - 1 : 0] step_6_depth_z;

    wire [FLOAT_SIZE - 1 : 0] step_6_color_r;
    wire [FLOAT_SIZE - 1 : 0] step_6_color_g;
    wire [FLOAT_SIZE - 1 : 0] step_6_color_b;
    wire [FLOAT_SIZE - 1 : 0] step_6_color_a;

    FloatMul #(.MANTISSA_SIZE(MANTISSA_SIZE), .EXPONENT_SIZE(EXPONENT_SIZE))
        mul_texture_s_w (.clk(aclk), .facAIn(step_5_texture_s_inv), .facBIn(step_5_texture_q), .prod(step_6_texture_s));
    FloatMul #(.MANTISSA_SIZE(MANTISSA_SIZE), .EXPONENT_SIZE(EXPONENT_SIZE))
        mul_texture_t_w (.clk(aclk), .facAIn(step_5_texture_t_inv), .facBIn(step_5_texture_q), .prod(step_6_texture_t));

    ValueDelay #(.VALUE_SIZE(FLOAT_SIZE), .DELAY(4)) step_6_delay_w (.clk(aclk), .in(step_5_depth_w), .out(step_6_depth_w));
    ValueDelay #(.VALUE_SIZE(FLOAT_SIZE), .DELAY(4)) step_6_delay_d_z (.clk(aclk), .in(step_5_depth_z_inv), .out(step_6_depth_z));

    // Enable this to have perspective correct color interpolation
    // Note: When this is enabled, the color must be divided by w (the vertex w) in the Rasterizer.cpp. Afterwards step_5_depth_w can be used.
    // It is right now questionable if this w has enough precision. It is currently not implemented to keep the logic count low.
    // FloatMul #(.MANTISSA_SIZE(MANTISSA_SIZE), .EXPONENT_SIZE(EXPONENT_SIZE))
    //     mul_color_r (.clk(aclk), .facAIn(step_5_color_r_inv), .facBIn(step_5_depth_w), .prod(step_6_color_r));
    // FloatMul #(.MANTISSA_SIZE(MANTISSA_SIZE), .EXPONENT_SIZE(EXPONENT_SIZE))
    //     mul_color_g (.clk(aclk), .facAIn(step_5_color_g_inv), .facBIn(step_5_depth_w), .prod(step_6_color_g));
    // FloatMul #(.MANTISSA_SIZE(MANTISSA_SIZE), .EXPONENT_SIZE(EXPONENT_SIZE))
    //     mul_color_b (.clk(aclk), .facAIn(step_5_color_b_inv), .facBIn(step_5_depth_w), .prod(step_6_color_b));
    // FloatMul #(.MANTISSA_SIZE(MANTISSA_SIZE), .EXPONENT_SIZE(EXPONENT_SIZE))
    //     mul_color_a (.clk(aclk), .facAIn(step_5_color_a_inv), .facBIn(step_5_depth_w), .prod(step_6_color_a));

    ValueDelay #(.VALUE_SIZE(FLOAT_SIZE), .DELAY(4)) step_6_delay_c_r (.clk(aclk), .in(step_5_color_r_inv), .out(step_6_color_r));
    ValueDelay #(.VALUE_SIZE(FLOAT_SIZE), .DELAY(4)) step_6_delay_c_g (.clk(aclk), .in(step_5_color_g_inv), .out(step_6_color_g));
    ValueDelay #(.VALUE_SIZE(FLOAT_SIZE), .DELAY(4)) step_6_delay_c_b (.clk(aclk), .in(step_5_color_b_inv), .out(step_6_color_b));
    ValueDelay #(.VALUE_SIZE(FLOAT_SIZE), .DELAY(4)) step_6_delay_c_a (.clk(aclk), .in(step_5_color_a_inv), .out(step_6_color_a));

    ////////////////////////////////////////////////////////////////////////////
    // STEP 7 Output calculated values
    ////////////////////////////////////////////////////////////////////////////
    assign m_axis_tvalid = step_0_tvalid;
    assign m_axis_tlast = step_0_tlast;
    assign m_axis_tdata = {
        step_0_framebuffer_index,
        {step_0_screen_pos_y, step_0_screen_pos_x},
        {step_6_depth_w, {FLOAT_SIZE_DIFF{1'b0}}},
        {step_6_texture_t, {FLOAT_SIZE_DIFF{1'b0}}},
        {step_6_texture_s, {FLOAT_SIZE_DIFF{1'b0}}},
        {step_6_depth_z, {FLOAT_SIZE_DIFF{1'b0}}},
        {step_6_color_a, {FLOAT_SIZE_DIFF{1'b0}}},
        {step_6_color_b, {FLOAT_SIZE_DIFF{1'b0}}},
        {step_6_color_g, {FLOAT_SIZE_DIFF{1'b0}}},
        {step_6_color_r, {FLOAT_SIZE_DIFF{1'b0}}}
    };

endmodule

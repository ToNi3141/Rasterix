// Rasterix
// https://github.com/ToNi3141/Rasterix
// Copyright (c) 2024 ToNi3141

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

// This module is used to interpolate the triagle attributes based 
// on the data from the rasterizer. It expects numbers in fix point format.
// Pipelined: yes
// Depth: 5 cycles
module AttributeInterpolatorX #(
    parameter INTERNAL_FLOAT_PRECISION = 32,
    parameter INDEX_WIDTH = 32,
    parameter SCREEN_POS_WIDTH = 11,
    parameter ENABLE_LOD_CALC = 1,
    parameter ENABLE_SECOND_TMU = 1,
    parameter SUB_PIXEL_WIDTH = 8,

    localparam ATTRIBUTE_SIZE = 32,
    localparam KEEP_WIDTH = 1,
    localparam FLOAT_SIZE = 32
)
(
    input  wire                             aclk,
    input  wire                             resetn,

    // Pixel Stream
    input  wire                             s_attrb_tvalid,
    input  wire                             s_attrb_tlast,
    input  wire [KEEP_WIDTH - 1 : 0]        s_attrb_tkeep,
    input  wire [SCREEN_POS_WIDTH - 1 : 0]  s_attrb_tspx,
    input  wire [SCREEN_POS_WIDTH - 1 : 0]  s_attrb_tspy,
    input  wire [INDEX_WIDTH - 1 : 0]       s_attrb_tindex,
    input  wire                             s_attrb_tpixel,
    input  wire [ 1 : 0]                    s_attrb_tcmd,


    // Attributes
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0]    tex0_s, // S1.30
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0]    tex0_t, // S1.30
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0]    tex0_q, // S1.30
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0]    tex0_s_inc_x,
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0]    tex0_t_inc_x,
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0]    tex0_q_inc_x,
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0]    tex0_s_inc_y,
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0]    tex0_t_inc_y,
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0]    tex0_q_inc_y,
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0]    tex1_s,
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0]    tex1_t,
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0]    tex1_q,
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0]    tex1_s_inc_x,
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0]    tex1_t_inc_x,
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0]    tex1_q_inc_x,
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0]    tex1_s_inc_y,
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0]    tex1_t_inc_y,
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0]    tex1_q_inc_y,
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0]    depth_w, // S1.30
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0]    depth_w_inc_x, // S1.30
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0]    depth_w_inc_y, // S1.30
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0]    depth_z, // S1.30
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0]    depth_z_inc_x, // S1.30
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0]    depth_z_inc_y, // S1.30
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0]    color_r, // S7.24
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0]    color_g,
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0]    color_b,
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0]    color_a,
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0]    color_r_inc_x,
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0]    color_g_inc_x,
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0]    color_b_inc_x,
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0]    color_a_inc_x,
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0]    color_r_inc_y,
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0]    color_g_inc_y,
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0]    color_b_inc_y,
    input  wire signed [ATTRIBUTE_SIZE - 1 : 0]    color_a_inc_y,

    // Pixel Stream Interpolated
    output wire                             m_attrb_tvalid,
    output wire                             m_attrb_tlast,
    output wire [KEEP_WIDTH - 1 : 0]        m_attrb_tkeep,
    output wire [SCREEN_POS_WIDTH - 1 : 0]  m_attrb_tspx,
    output wire [SCREEN_POS_WIDTH - 1 : 0]  m_attrb_tspy,
    output wire [INDEX_WIDTH - 1 : 0]       m_attrb_tindex,
    output wire [FLOAT_SIZE - 1 : 0]        m_attrb_tdepth_w, // Float
    output wire [31 : 0]                    m_attrb_tdepth_z, // Q16.16
    output wire [31 : 0]                    m_attrb_ttexture0_t, // S16.15
    output wire [31 : 0]                    m_attrb_ttexture0_s, // S16.15
    output wire [31 : 0]                    m_attrb_tmipmap0_t, // S16.15
    output wire [31 : 0]                    m_attrb_tmipmap0_s, // S16.15
    output wire [31 : 0]                    m_attrb_ttexture1_t, // S16.15
    output wire [31 : 0]                    m_attrb_ttexture1_s, // S16.15
    output wire [31 : 0]                    m_attrb_tmipmap1_t, // S16.15
    output wire [31 : 0]                    m_attrb_tmipmap1_s, // S16.15
    output wire [SUB_PIXEL_WIDTH - 1 : 0]   m_attrb_tcolor_a, // Qn
    output wire [SUB_PIXEL_WIDTH - 1 : 0]   m_attrb_tcolor_b, // Qn
    output wire [SUB_PIXEL_WIDTH - 1 : 0]   m_attrb_tcolor_g, // Qn
    output wire [SUB_PIXEL_WIDTH - 1 : 0]   m_attrb_tcolor_r // Qn
);
`include "RasterizerCommands.vh"

    function [15 : 0] truncate16;
        input [31 : 0] in;
        begin
            truncate16 = in[0 +: 16];        
        end
    endfunction

    ////////////////////////////////////////////////////////////////////////////
    // Calculate the increment depending on the command of the rasterizer
    ///////////////////////////////////////////////////////////////////////////
    reg  signed [ATTRIBUTE_SIZE - 1 : 0]    reg_tex0_s;
    reg  signed [ATTRIBUTE_SIZE - 1 : 0]    reg_tex0_t;
    reg  signed [ATTRIBUTE_SIZE - 1 : 0]    reg_tex0_q;
    reg  signed [ATTRIBUTE_SIZE - 1 : 0]    reg_tex0_mipmap_s;
    reg  signed [ATTRIBUTE_SIZE - 1 : 0]    reg_tex0_mipmap_t;
    reg  signed [ATTRIBUTE_SIZE - 1 : 0]    reg_tex0_mipmap_q;
    reg  signed [ATTRIBUTE_SIZE - 1 : 0]    reg_tex1_s;
    reg  signed [ATTRIBUTE_SIZE - 1 : 0]    reg_tex1_t;
    reg  signed [ATTRIBUTE_SIZE - 1 : 0]    reg_tex1_q;
    reg  signed [ATTRIBUTE_SIZE - 1 : 0]    reg_tex1_mipmap_s;
    reg  signed [ATTRIBUTE_SIZE - 1 : 0]    reg_tex1_mipmap_t;
    reg  signed [ATTRIBUTE_SIZE - 1 : 0]    reg_tex1_mipmap_q;
    reg  signed [ATTRIBUTE_SIZE - 1 : 0]    reg_depth_w;
    reg  signed [ATTRIBUTE_SIZE - 1 : 0]    reg_depth_z;
    reg  signed [ATTRIBUTE_SIZE - 1 : 0]    reg_color_r;
    reg  signed [ATTRIBUTE_SIZE - 1 : 0]    reg_color_g;
    reg  signed [ATTRIBUTE_SIZE - 1 : 0]    reg_color_b;
    reg  signed [ATTRIBUTE_SIZE - 1 : 0]    reg_color_a;

    always @(posedge aclk)
    begin
        if (s_attrb_tvalid)
        begin
            case (s_attrb_tcmd)
                RR_CMD_INIT:
                begin
                    reg_tex0_s <= tex0_s;
                    reg_tex0_t <= tex0_t;
                    reg_tex0_q <= tex0_q;
                    if (ENABLE_LOD_CALC)
                    begin
                        reg_tex0_mipmap_s <= tex0_s + tex0_s_inc_x + tex0_s_inc_y;
                        reg_tex0_mipmap_t <= tex0_t + tex0_t_inc_x + tex0_t_inc_y;
                        reg_tex0_mipmap_q <= tex0_q + tex0_q_inc_x + tex0_q_inc_y;
                    end
                    if (ENABLE_SECOND_TMU)
                    begin
                        reg_tex1_s <= tex1_s;
                        reg_tex1_t <= tex1_t;
                        reg_tex1_q <= tex1_q;
                        if (ENABLE_LOD_CALC)
                        begin
                            reg_tex1_mipmap_s <= tex1_s + tex1_s_inc_x + tex1_s_inc_y;
                            reg_tex1_mipmap_t <= tex1_t + tex1_t_inc_x + tex1_t_inc_y;
                            reg_tex1_mipmap_q <= tex1_q + tex1_q_inc_x + tex1_q_inc_y;
                        end
                    end
                    reg_depth_w <= depth_w;
                    reg_depth_z <= depth_z;
                    reg_color_r <= color_r;
                    reg_color_g <= color_g;
                    reg_color_b <= color_b;
                    reg_color_a <= color_a;
                end
                RR_CMD_X_INC:
                begin
                    reg_tex0_s <= reg_tex0_s + tex0_s_inc_x;
                    reg_tex0_t <= reg_tex0_t + tex0_t_inc_x;
                    reg_tex0_q <= reg_tex0_q + tex0_q_inc_x;
                    if (ENABLE_LOD_CALC)
                    begin
                        reg_tex0_mipmap_s <= reg_tex0_mipmap_s + tex0_s_inc_x;
                        reg_tex0_mipmap_t <= reg_tex0_mipmap_t + tex0_t_inc_x;
                        reg_tex0_mipmap_q <= reg_tex0_mipmap_q + tex0_q_inc_x;
                    end
                    if (ENABLE_SECOND_TMU)
                    begin
                        reg_tex1_s <= reg_tex1_s + tex1_s_inc_x;
                        reg_tex1_t <= reg_tex1_t + tex1_t_inc_x;
                        reg_tex1_q <= reg_tex1_q + tex1_q_inc_x;
                        if (ENABLE_LOD_CALC)
                        begin
                            reg_tex1_mipmap_s <= reg_tex1_mipmap_s + tex1_s_inc_x;
                            reg_tex1_mipmap_t <= reg_tex1_mipmap_t + tex1_t_inc_x;
                            reg_tex1_mipmap_q <= reg_tex1_mipmap_q + tex1_q_inc_x;
                        end
                    end
                    reg_depth_w <= reg_depth_w + depth_w_inc_x;
                    reg_depth_z <= reg_depth_z + depth_z_inc_x;
                    reg_color_r <= reg_color_r + color_r_inc_x;
                    reg_color_g <= reg_color_g + color_g_inc_x;
                    reg_color_b <= reg_color_b + color_b_inc_x;
                    reg_color_a <= reg_color_a + color_a_inc_x;
                end
                RR_CMD_X_DEC:
                begin
                    reg_tex0_s <= reg_tex0_s - tex0_s_inc_x;
                    reg_tex0_t <= reg_tex0_t - tex0_t_inc_x;
                    reg_tex0_q <= reg_tex0_q - tex0_q_inc_x;
                    if (ENABLE_LOD_CALC)
                    begin
                        reg_tex0_mipmap_s <= reg_tex0_mipmap_s - tex0_s_inc_x;
                        reg_tex0_mipmap_t <= reg_tex0_mipmap_t - tex0_t_inc_x;
                        reg_tex0_mipmap_q <= reg_tex0_mipmap_q - tex0_q_inc_x;
                    end
                    if (ENABLE_SECOND_TMU)
                    begin
                        reg_tex1_s <= reg_tex1_s - tex1_s_inc_x;
                        reg_tex1_t <= reg_tex1_t - tex1_t_inc_x;
                        reg_tex1_q <= reg_tex1_q - tex1_q_inc_x;
                        if (ENABLE_LOD_CALC)
                        begin
                            reg_tex1_mipmap_s <= reg_tex1_mipmap_s - tex1_s_inc_x;
                            reg_tex1_mipmap_t <= reg_tex1_mipmap_t - tex1_t_inc_x;
                            reg_tex1_mipmap_q <= reg_tex1_mipmap_q - tex1_q_inc_x;
                        end
                    end
                    reg_depth_w <= reg_depth_w - depth_w_inc_x;
                    reg_depth_z <= reg_depth_z - depth_z_inc_x;
                    reg_color_r <= reg_color_r - color_r_inc_x;
                    reg_color_g <= reg_color_g - color_g_inc_x;
                    reg_color_b <= reg_color_b - color_b_inc_x;
                    reg_color_a <= reg_color_a - color_a_inc_x;
                end
                RR_CMD_Y_INC:
                begin
                    reg_tex0_s <= reg_tex0_s + tex0_s_inc_y;
                    reg_tex0_t <= reg_tex0_t + tex0_t_inc_y;
                    reg_tex0_q <= reg_tex0_q + tex0_q_inc_y;
                    if (ENABLE_LOD_CALC)
                    begin
                        reg_tex0_mipmap_s <= reg_tex0_mipmap_s + tex0_s_inc_y;
                        reg_tex0_mipmap_t <= reg_tex0_mipmap_t + tex0_t_inc_y;
                        reg_tex0_mipmap_q <= reg_tex0_mipmap_q + tex0_q_inc_y;
                    end
                    if (ENABLE_SECOND_TMU)
                    begin
                        reg_tex1_s <= reg_tex1_s + tex1_s_inc_y;
                        reg_tex1_t <= reg_tex1_t + tex1_t_inc_y;
                        reg_tex1_q <= reg_tex1_q + tex1_q_inc_y;
                        if (ENABLE_LOD_CALC)
                        begin
                            reg_tex1_mipmap_s <= reg_tex1_mipmap_s + tex1_s_inc_y;
                            reg_tex1_mipmap_t <= reg_tex1_mipmap_t + tex1_t_inc_y;
                            reg_tex1_mipmap_q <= reg_tex1_mipmap_q + tex1_q_inc_y;
                        end
                    end
                    reg_depth_w <= reg_depth_w + depth_w_inc_y;
                    reg_depth_z <= reg_depth_z + depth_z_inc_y;
                    reg_color_r <= reg_color_r + color_r_inc_y;
                    reg_color_g <= reg_color_g + color_g_inc_y;
                    reg_color_b <= reg_color_b + color_b_inc_y;
                    reg_color_a <= reg_color_a + color_a_inc_y;
                end 
                default: 
                begin
                end
            endcase
        end
    end

    ////////////////////////////////////////////////////////////////////////////
    // STEP 1
    // Calculate the reciprocal
    // Clocks: 2
    ///////////////////////////////////////////////////////////////////////////
    localparam RECIP_DELAY = 2;
    wire signed [16 - 1 : 0]                step1_tex0_s;
    wire signed [16 - 1 : 0]                step1_tex0_t;
    wire signed [16 - 1 : 0]                step1_tex0_q; // 0xffffff / reg_tex0_q = step1_tex0_q (Un.24 / U1.15 = U7.9) (clamp to 16 bit)
    wire signed [16 - 1 : 0]                step1_tex0_mipmap_s;
    wire signed [16 - 1 : 0]                step1_tex0_mipmap_t;
    wire signed [16 - 1 : 0]                step1_tex0_mipmap_q;
    wire signed [16 - 1 : 0]                step1_tex1_s;
    wire signed [16 - 1 : 0]                step1_tex1_t;
    wire signed [16 - 1 : 0]                step1_tex1_q;
    wire signed [16 - 1 : 0]                step1_tex1_mipmap_s;
    wire signed [16 - 1 : 0]                step1_tex1_mipmap_t;
    wire signed [16 - 1 : 0]                step1_tex1_mipmap_q;
    wire signed [16 - 1 : 0]                step1_depth_w; // U7.9
    wire        [32 - 1 : 0]                step1_depth_z; // S1.30
    wire        [16 - 1 : 0]                step1_color_r; // S7.8
    wire        [16 - 1 : 0]                step1_color_g;
    wire        [16 - 1 : 0]                step1_color_b;
    wire        [16 - 1 : 0]                step1_color_a;
    wire                                    step1_tvalid;
    wire                                    step1_tlast;
    wire        [KEEP_WIDTH - 1 : 0]        step1_tkeep;
    wire        [SCREEN_POS_WIDTH - 1 : 0]  step1_tspx;
    wire        [SCREEN_POS_WIDTH - 1 : 0]  step1_tspy;
    wire        [INDEX_WIDTH - 1 : 0]       step1_tindex;

    ValueDelay #(.VALUE_SIZE(1), .DELAY(RECIP_DELAY)) 
        step1_tvalid_delay (.clk(aclk), .in(s_attrb_tvalid & s_attrb_tpixel), .out(step1_tvalid));
    ValueDelay #(.VALUE_SIZE(1), .DELAY(RECIP_DELAY)) 
        step1_tlast_delay (.clk(aclk), .in(s_attrb_tlast), .out(step1_tlast));
    ValueDelay #(.VALUE_SIZE(KEEP_WIDTH), .DELAY(RECIP_DELAY)) 
        step1_tkeep_delay (.clk(aclk), .in(s_attrb_tkeep), .out(step1_tkeep));

    ValueDelay #(.VALUE_SIZE(SCREEN_POS_WIDTH), .DELAY(RECIP_DELAY)) 
        step1_tspx_delay (.clk(aclk), .in(s_attrb_tspx), .out(step1_tspx));
    ValueDelay #(.VALUE_SIZE(SCREEN_POS_WIDTH), .DELAY(RECIP_DELAY)) 
        step1_tspy_delay (.clk(aclk), .in(s_attrb_tspy), .out(step1_tspy));

    ValueDelay #(.VALUE_SIZE(INDEX_WIDTH), .DELAY(RECIP_DELAY)) 
        step1_tindex_delay (.clk(aclk), .in(s_attrb_tindex), .out(step1_tindex));
    
    ValueDelay #(
        .VALUE_SIZE(16), 
        .DELAY(RECIP_DELAY)
    ) step1_color_r_delay (
        .clk(aclk), 
        .in(truncate16(reg_color_r >>> 16)), 
        .out(step1_color_r)
    );
    ValueDelay #(
        .VALUE_SIZE(16), 
        .DELAY(RECIP_DELAY)
    ) step1_color_g_delay (
        .clk(aclk), 
        .in(truncate16(reg_color_g >>> 16)), 
        .out(step1_color_g)
    );
    ValueDelay #(
        .VALUE_SIZE(16), 
        .DELAY(RECIP_DELAY)
    ) step1_color_b_delay (
        .clk(aclk), 
        .in(truncate16(reg_color_b >>> 16)), 
        .out(step1_color_b)
    );
    ValueDelay #(
        .VALUE_SIZE(16), 
        .DELAY(RECIP_DELAY)
    ) step1_color_a_delay (
        .clk(aclk), 
        .in(truncate16(reg_color_a >>> 16)), 
        .out(step1_color_a)
    );

    ValueDelay #(
        .VALUE_SIZE(32), 
        .DELAY(RECIP_DELAY)
    ) step1_depth_z_delay (
        .clk(aclk), 
        .in(truncate16(reg_depth_z >> 14)), 
        .out(step1_depth_z)
    );
    Recip #(
        .NUMERATOR(32'hffffff), 
        .NUMBER_WIDTH(16), 
        .LOOKUP_PRECISION(11)
    ) step1_depth_w_recip (
        .aclk(aclk), 
        .x(truncate16(reg_depth_w >> 15)), 
        .y(step1_depth_w)
    );

    ValueDelay #(
        .VALUE_SIZE(16), 
        .DELAY(RECIP_DELAY)
    ) step1_tex0_s_delay (
        .clk(aclk), 
        .in(truncate16(reg_tex0_s >>> 16)), 
        .out(step1_tex0_s)
    );
    ValueDelay #(
        .VALUE_SIZE(16), 
        .DELAY(RECIP_DELAY)
    ) step1_tex0_t_delay (
        .clk(aclk), 
        .in(truncate16(reg_tex0_t >>> 16)), 
        .out(step1_tex0_t)
    );
    Recip #(
        .NUMERATOR(32'hffffff), 
        .NUMBER_WIDTH(16), 
        .LOOKUP_PRECISION(11)
    ) step1_tex0_q_recip (
        .aclk(aclk), 
        // S1.30 >> 15 = U1.15 Clamp to 16 bit and remove sign, because the value is normalized between 1.0 and 0.0
        .x(truncate16(reg_tex0_q >> 15)), 
        .y(step1_tex0_q)
    );
    generate
        if (ENABLE_LOD_CALC)
        begin
            ValueDelay #(
                .VALUE_SIZE(16), 
                .DELAY(RECIP_DELAY)
            ) step1_tex0_mipmap_s_delay (
                .clk(aclk), 
                .in(truncate16(reg_tex0_mipmap_s >>> 16)), 
                .out(step1_tex0_mipmap_s)
            );
            ValueDelay #(
                .VALUE_SIZE(16), 
                .DELAY(RECIP_DELAY)
            ) step1_tex0_mipmap_t_delay (
                .clk(aclk), 
                .in(truncate16(reg_tex0_mipmap_t >>> 16)), 
                .out(step1_tex0_mipmap_t)
            );
            Recip #(
                .NUMERATOR(32'hffffff), 
                .NUMBER_WIDTH(16), 
                .LOOKUP_PRECISION(11)
            ) step1_tex0_mipmap_q_recip (
                .aclk(aclk), 
                .x(truncate16(reg_tex0_mipmap_q >> 15)), 
                .y(step1_tex0_mipmap_q)
            );
        end
    endgenerate

    generate
        if (ENABLE_SECOND_TMU)
        begin
            ValueDelay #(
                .VALUE_SIZE(16), 
                .DELAY(RECIP_DELAY)
            ) step1_tex1_s_delay (
                .clk(aclk), 
                .in(truncate16(reg_tex1_s >>> 16)), 
                .out(step1_tex1_s)
            );
            ValueDelay #(
                .VALUE_SIZE(16), 
                .DELAY(RECIP_DELAY)
            ) step1_tex1_t_delay (
                .clk(aclk), 
                .in(truncate16(reg_tex1_t >>> 16)), 
                .out(step1_tex1_t)
            );
            Recip #(
                .NUMERATOR(32'hffffff), 
                .NUMBER_WIDTH(16), 
                .LOOKUP_PRECISION(11)
            ) step1_tex1_q_recip (
                .aclk(aclk), 
                .x(truncate16(reg_tex1_q >> 15)), 
                .y(step1_tex1_q)
            );
            if (ENABLE_LOD_CALC)
            begin
                ValueDelay #(
                    .VALUE_SIZE(16), 
                    .DELAY(RECIP_DELAY)
                ) step1_tex1_mipmap_s_delay (
                    .clk(aclk), 
                    .in(truncate16(reg_tex1_mipmap_s >>> 16)), 
                    .out(step1_tex1_mipmap_s)
                );
                ValueDelay #(
                    .VALUE_SIZE(16), 
                    .DELAY(RECIP_DELAY)
                ) step1_tex1_mipmap_t_delay (
                    .clk(aclk), 
                    .in(truncate16(reg_tex1_mipmap_t >>> 16)), 
                    .out(step1_tex1_mipmap_t)
                );
                Recip #(
                    .NUMERATOR(32'hffffff), 
                    .NUMBER_WIDTH(16), 
                    .LOOKUP_PRECISION(11)
                ) step1_tex1_mipmap_q_recip (
                    .aclk(aclk), 
                    .x(truncate16(reg_tex1_mipmap_q >> 15)), 
                    .y(step1_tex1_mipmap_q)
                );
            end
        end
    endgenerate

    ////////////////////////////////////////////////////////////////////////////
    // STEP 2
    // Calculate perspective correction
    // Clocks: 4
    ///////////////////////////////////////////////////////////////////////////
    localparam I2F_DELAY = 4;
    wire [32 - 1 : 0]                step2_tex0_s; // S16.15
    wire [32 - 1 : 0]                step2_tex0_t;
    wire [32 - 1 : 0]                step2_tex0_mipmap_s;
    wire [32 - 1 : 0]                step2_tex0_mipmap_t;
    wire [32 - 1 : 0]                step2_tex1_s;
    wire [32 - 1 : 0]                step2_tex1_t;
    wire [32 - 1 : 0]                step2_tex1_mipmap_s;
    wire [32 - 1 : 0]                step2_tex1_mipmap_t;
    wire [FLOAT_SIZE - 1 : 0]        step2_depth_w;
    wire [32 - 1 : 0]                step2_depth_z;
    wire [ 8 - 1 : 0]                step2_color_r;
    wire [ 8 - 1 : 0]                step2_color_g;
    wire [ 8 - 1 : 0]                step2_color_b;
    wire [ 8 - 1 : 0]                step2_color_a;
    wire                             step2_tvalid;
    wire                             step2_tlast;
    wire [KEEP_WIDTH - 1 : 0]        step2_tkeep;
    wire [SCREEN_POS_WIDTH - 1 : 0]  step2_tspx;
    wire [SCREEN_POS_WIDTH - 1 : 0]  step2_tspy;
    wire [INDEX_WIDTH - 1 : 0]       step2_tindex;

    ValueDelay #(.VALUE_SIZE(1), .DELAY(I2F_DELAY)) 
        step2_tvalid_delay (.clk(aclk), .in(step1_tvalid), .out(step2_tvalid));
    ValueDelay #(.VALUE_SIZE(1), .DELAY(I2F_DELAY)) 
        step2_tlast_delay (.clk(aclk), .in(step1_tlast), .out(step2_tlast));
    ValueDelay #(.VALUE_SIZE(KEEP_WIDTH), .DELAY(I2F_DELAY)) 
        step2_tkeep_delay (.clk(aclk), .in(step1_tkeep), .out(step2_tkeep));

    ValueDelay #(.VALUE_SIZE(SCREEN_POS_WIDTH), .DELAY(I2F_DELAY)) 
        step2_tspx_delay (.clk(aclk), .in(step1_tspx), .out(step2_tspx));
    ValueDelay #(.VALUE_SIZE(SCREEN_POS_WIDTH), .DELAY(I2F_DELAY)) 
        step2_tspy_delay (.clk(aclk), .in(step1_tspy), .out(step2_tspy));

    ValueDelay #(.VALUE_SIZE(INDEX_WIDTH), .DELAY(I2F_DELAY)) 
        step2_tindex_delay (.clk(aclk), .in(step1_tindex), .out(step2_tindex));

    ValueDelay #(.VALUE_SIZE(32), .DELAY(I2F_DELAY)) 
        step2_tdepth_z_delay (.clk(aclk), .in(step1_depth_z), .out(step2_depth_z));

    IntToFloat #(.MANTISSA_SIZE(FLOAT_SIZE - 9), .EXPONENT_SIZE(8), .INT_SIZE(32), .EXPONENT_BIAS_OFFSET(-9))
        step2_tdepth_w_i2f (.clk(aclk), .in({16'h0, step1_depth_w}), .out(step2_depth_w));

    reg  [32 - 1 : 0]                step2_tex0_s_reg; // S16.15
    reg  [32 - 1 : 0]                step2_tex0_t_reg;
    reg  [32 - 1 : 0]                step2_tex0_mipmap_s_reg;
    reg  [32 - 1 : 0]                step2_tex0_mipmap_t_reg;
    reg  [32 - 1 : 0]                step2_tex1_s_reg;
    reg  [32 - 1 : 0]                step2_tex1_t_reg;
    reg  [32 - 1 : 0]                step2_tex1_mipmap_s_reg;
    reg  [32 - 1 : 0]                step2_tex1_mipmap_t_reg;
    reg  [ 8 - 1 : 0]                step2_color_r_reg;
    reg  [ 8 - 1 : 0]                step2_color_g_reg;
    reg  [ 8 - 1 : 0]                step2_color_b_reg;
    reg  [ 8 - 1 : 0]                step2_color_a_reg;
    always @(posedge aclk)
    begin
        step2_color_a_reg <= (step1_color_a[15]) ? 0 : (|step1_color_a[8 +: 7]) ? 8'hff : step1_color_a[0 +: 8];
        step2_color_b_reg <= (step1_color_b[15]) ? 0 : (|step1_color_b[8 +: 7]) ? 8'hff : step1_color_b[0 +: 8];
        step2_color_g_reg <= (step1_color_g[15]) ? 0 : (|step1_color_g[8 +: 7]) ? 8'hff : step1_color_g[0 +: 8];
        step2_color_r_reg <= (step1_color_r[15]) ? 0 : (|step1_color_r[8 +: 7]) ? 8'hff : step1_color_r[0 +: 8];

        step2_tex0_s_reg <= (step1_tex0_s * step1_tex0_q) >>> 8; // S7.9 * S1.14 = S8.23 >>> 8 = S16.15
        step2_tex0_t_reg <= (step1_tex0_t * step1_tex0_q) >>> 8;
        if (ENABLE_LOD_CALC)
        begin
            step2_tex0_mipmap_s_reg <= (step1_tex0_mipmap_s * step1_tex0_mipmap_q) >>> 8;
            step2_tex0_mipmap_t_reg <= (step1_tex0_mipmap_t * step1_tex0_mipmap_q) >>> 8;
        end

        if (ENABLE_SECOND_TMU)
        begin
            step2_tex1_s_reg <= (step1_tex1_s * step1_tex1_q) >>> 8;
            step2_tex1_t_reg <= (step1_tex1_t * step1_tex1_q) >>> 8;
            if (ENABLE_LOD_CALC)
            begin
                step2_tex1_mipmap_s_reg <= (step1_tex1_mipmap_s * step1_tex1_mipmap_q) >>> 8;
                step2_tex1_mipmap_t_reg <= (step1_tex1_mipmap_t * step1_tex1_mipmap_q) >>> 8;
            end
        end
    end

    ValueDelay #(.VALUE_SIZE(32), .DELAY(I2F_DELAY - 1)) 
        step2_ttex0_s_delay (.clk(aclk), .in(step2_tex0_s_reg), .out(step2_tex0_s));
    ValueDelay #(.VALUE_SIZE(32), .DELAY(I2F_DELAY - 1)) 
        step2_ttex0_t_delay (.clk(aclk), .in(step2_tex0_t_reg), .out(step2_tex0_t));
    ValueDelay #(.VALUE_SIZE(32), .DELAY(I2F_DELAY - 1)) 
        step2_ttex0_mipmap_s_delay (.clk(aclk), .in(step2_tex0_mipmap_s_reg), .out(step2_tex0_mipmap_s));
    ValueDelay #(.VALUE_SIZE(32), .DELAY(I2F_DELAY - 1)) 
        step2_ttex0_mipmap_t_delay (.clk(aclk), .in(step2_tex0_mipmap_t_reg), .out(step2_tex0_mipmap_t));
    ValueDelay #(.VALUE_SIZE(32), .DELAY(I2F_DELAY - 1)) 
        step2_ttex1_s_delay (.clk(aclk), .in(step2_tex1_s_reg), .out(step2_tex1_s));
    ValueDelay #(.VALUE_SIZE(32), .DELAY(I2F_DELAY - 1)) 
        step2_ttex1_t_delay (.clk(aclk), .in(step2_tex1_t_reg), .out(step2_tex1_t));
    ValueDelay #(.VALUE_SIZE(32), .DELAY(I2F_DELAY - 1)) 
        step2_ttex1_mipmap_s_delay (.clk(aclk), .in(step2_tex1_mipmap_s_reg), .out(step2_tex1_mipmap_s));
    ValueDelay #(.VALUE_SIZE(32), .DELAY(I2F_DELAY - 1)) 
        step2_ttex1_mipmap_t_delay (.clk(aclk), .in(step2_tex1_mipmap_t_reg), .out(step2_tex1_mipmap_t));
    ValueDelay #(.VALUE_SIZE(8), .DELAY(I2F_DELAY - 1)) 
        step2_tcolor_r_delay (.clk(aclk), .in(step2_color_r_reg), .out(step2_color_r));
    ValueDelay #(.VALUE_SIZE(8), .DELAY(I2F_DELAY - 1)) 
        step2_tcolor_g_delay (.clk(aclk), .in(step2_color_g_reg), .out(step2_color_g));
    ValueDelay #(.VALUE_SIZE(8), .DELAY(I2F_DELAY - 1)) 
        step2_tcolor_b_delay (.clk(aclk), .in(step2_color_b_reg), .out(step2_color_b));
    ValueDelay #(.VALUE_SIZE(8), .DELAY(I2F_DELAY - 1)) 
        step2_tcolor_a_delay (.clk(aclk), .in(step2_color_a_reg), .out(step2_color_a));

    ////////////////////////////////////////////////////////////////////////////
    // STEP 3
    // Output data
    // Clocks: 0
    ///////////////////////////////////////////////////////////////////////////
    assign m_attrb_tvalid = step2_tvalid;
    assign m_attrb_tlast = step2_tlast;
    assign m_attrb_tkeep = step2_tkeep;
    assign m_attrb_tspx = step2_tspx;
    assign m_attrb_tspy = step2_tspy;
    assign m_attrb_tindex = step2_tindex;
    assign m_attrb_tdepth_w = step2_depth_w;
    assign m_attrb_tdepth_z = step2_depth_z;

    assign m_attrb_ttexture0_t = step2_tex0_t;
    assign m_attrb_ttexture0_s = step2_tex0_s;
    generate
        if (ENABLE_LOD_CALC)
        begin
            assign m_attrb_tmipmap0_t = step2_tex0_mipmap_t;
            assign m_attrb_tmipmap0_s = step2_tex0_mipmap_s;
        end
        else
        begin
            assign m_attrb_tmipmap0_t = step2_tex0_t;
            assign m_attrb_tmipmap0_s = step2_tex0_s;
        end
    endgenerate

    generate
        if (ENABLE_SECOND_TMU)
        begin
            assign m_attrb_ttexture1_t = step2_tex1_t;
            assign m_attrb_ttexture1_s = step2_tex1_s;
            if (ENABLE_LOD_CALC)
            begin
                assign m_attrb_tmipmap1_t = step2_tex1_mipmap_t;
                assign m_attrb_tmipmap1_s = step2_tex1_mipmap_s;
            end
            else
            begin
                assign m_attrb_tmipmap1_t = step2_tex1_t;
                assign m_attrb_tmipmap1_s = step2_tex1_s;
            end
        end
    endgenerate

    assign m_attrb_tcolor_a = step2_color_a;
    assign m_attrb_tcolor_b = step2_color_b;
    assign m_attrb_tcolor_g = step2_color_g;
    assign m_attrb_tcolor_r = step2_color_r;
endmodule

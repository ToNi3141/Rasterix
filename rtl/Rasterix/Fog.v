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

// Fogs a texel based on a given depth value.
// It uses a fog look up table to calculate the intensity of the fog color based on the depth.
// To see how the LUT is build, @see FunctionInterpolator
// Pipelined: yes
// Depth: 6 cycles
module Fog 
#(
    parameter SUB_PIXEL_WIDTH = 8,
    localparam NUMBER_OF_SUB_PIXEL = 4,
    localparam PIXEL_WIDTH = SUB_PIXEL_WIDTH * NUMBER_OF_SUB_PIXEL,

    localparam FLOAT_SIZE = 32
)
(
    input  wire                         aclk,
    input  wire                         resetn,
    input  wire                         ce,

    // Fog function LUT stream
    input  wire                         s_fog_lut_axis_tvalid,
    output wire                         s_fog_lut_axis_tready,
    input  wire                         s_fog_lut_axis_tlast,
    input  wire [31 : 0]                s_fog_lut_axis_tdata,

    // Fog Color
    input  wire [PIXEL_WIDTH - 1 : 0]   confFogColor,
    input  wire                         confEnable,

    // Fog calculation
    input  wire [FLOAT_SIZE - 1 : 0]    depth,
    input  wire [PIXEL_WIDTH - 1 : 0]   texelColor,
    
    // Fogged texel
    output wire [PIXEL_WIDTH - 1 : 0]   color 
);
`include "RegisterAndDescriptorDefines.vh"

    ////////////////////////////////////////////////////////////////////////////
    // STEP 0
    // Calculate the fog intensity
    // Clocks: 4 
    ////////////////////////////////////////////////////////////////////////////
    wire [23 : 0]               step0_fogIntensity;
    wire [PIXEL_WIDTH - 1 :0]   step0_texelColor;

    ValueDelay #(
        .VALUE_SIZE(PIXEL_WIDTH), 
        .DELAY(4)
    ) step0_texelColorDelay (
        .clk(aclk), 
        .ce(ce),
        .in(texelColor),
        .out(step0_texelColor)
    );

    FunctionInterpolator step0_calculateFogIntensity (
        .aclk(aclk), 
        .resetn(resetn), 
        .ce(ce),
        .x(depth),
        .fx(step0_fogIntensity),
        .s_axis_tvalid(s_fog_lut_axis_tvalid), 
        .s_axis_tready(s_fog_lut_axis_tready), 
        .s_axis_tlast(s_fog_lut_axis_tlast), 
        .s_axis_tdata(s_fog_lut_axis_tdata)
    );

    ////////////////////////////////////////////////////////////////////////////
    // STEP 1
    // Fog color mixing
    // Clocks: 2
    ////////////////////////////////////////////////////////////////////////////
    wire [PIXEL_WIDTH - 1 : 0]  step1_texelColor;
    wire [PIXEL_WIDTH - 1 : 0]  step1_mixedColor;

    ValueDelay #(
        .VALUE_SIZE(PIXEL_WIDTH), 
        .DELAY(2)
    ) step0_texelDelay (
        .clk(aclk),
        .ce(ce),
        .in(step0_texelColor),
        .out(step1_texelColor)
    );

    ColorInterpolator #(
        .SUB_PIXEL_WIDTH(SUB_PIXEL_WIDTH)
    ) fogInterpolator (
        .aclk(aclk),
        .resetn(resetn),
        .ce(ce),

        .intensity((step0_fogIntensity[22]) ? 16'hffff : step0_fogIntensity[22 - 16 +: 16]),
        .colorA(step0_texelColor),
        .colorB(confFogColor),

        .mixedColor(step1_mixedColor)
    );

    ////////////////////////////////////////////////////////////////////////////
    // STEP 2
    // Output fogged texel
    // Clocks: 0
    ////////////////////////////////////////////////////////////////////////////
    assign color = (confEnable) ? {
            step1_mixedColor[COLOR_R_POS +: SUB_PIXEL_WIDTH],
            step1_mixedColor[COLOR_G_POS +: SUB_PIXEL_WIDTH],
            step1_mixedColor[COLOR_B_POS +: SUB_PIXEL_WIDTH],
            step1_texelColor[COLOR_A_POS +: SUB_PIXEL_WIDTH] // Replace alpha, because it is specified that fog does not change the alpha value of a texel
        } : step1_texelColor;

endmodule
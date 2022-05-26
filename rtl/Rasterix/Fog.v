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

module Fog 
#(
    parameter SUB_PIXEL_WIDTH = 8,
    localparam NUMBER_OF_SUB_PIXELS = 4,
    localparam PIXEL_WIDTH = SUB_PIXEL_WIDTH * NUMBER_OF_SUB_PIXELS
)
(
    input  wire                         aclk,
    input  wire                         resetn,

    input  wire [15 : 0]                intensity,
    input  wire [PIXEL_WIDTH - 1 : 0]   texelColor,
    input  wire [PIXEL_WIDTH - 1 : 0]   fogColor,

    output wire [PIXEL_WIDTH - 1 : 0]   color 
);
`include "RegisterAndDescriptorDefines.vh"

    wire [PIXEL_WIDTH - 1 : 0]      mixedColor;
    wire [SUB_PIXEL_WIDTH - 1 : 0]  texelAlpha;

    ValueDelay #(
        .VALUE_SIZE(SUB_PIXEL_WIDTH), 
        .DELAY(2)
    ) alphaDelay (
        .clk(aclk), 
        .in(texelColor[COLOR_A_POS +: SUB_PIXEL_WIDTH]),
        .out(texelAlpha)
    );

    ColorInterpolator #(
        .SUB_PIXEL_WIDTH(SUB_PIXEL_WIDTH)
    ) fogInterpolator (
        .aclk(aclk),
        .resetn(resetn),

        .intensity(intensity),
        .colorA(texelColor),
        .colorB(fogColor),

        .mixedColor(mixedColor)
    );

    assign color = {
        mixedColor[COLOR_R_POS +: SUB_PIXEL_WIDTH],
        mixedColor[COLOR_G_POS +: SUB_PIXEL_WIDTH],
        mixedColor[COLOR_B_POS +: SUB_PIXEL_WIDTH],
        texelAlpha // Replace alpha, because it is specified that fog does not change the alpha value of a texel
    };

endmodule
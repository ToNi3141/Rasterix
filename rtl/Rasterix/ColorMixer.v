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

`include "PixelUtil.vh"

// Mixes four colors together with the following formular 
//  mixedColor = (colorA * colorB) + (colorC * colorD)
// Each color is a vector of four sub colors.
// The mixedColor will be saturated.
// Pipelined: yes
// Depth: 2 cycles
module ColorMixer #(
    parameter SUB_PIXEL_WIDTH = 8,
    localparam NUMBER_OF_SUB_PIXEL = 4,
    localparam PIXEL_WIDTH = SUB_PIXEL_WIDTH * NUMBER_OF_SUB_PIXEL
) (
    input wire                          aclk,
    input wire                          resetn,

    input wire  [PIXEL_WIDTH - 1 : 0]   colorA,
    input wire  [PIXEL_WIDTH - 1 : 0]   colorB,
    input wire  [PIXEL_WIDTH - 1 : 0]   colorC,
    input wire  [PIXEL_WIDTH - 1 : 0]   colorD,

    // mixedColor = (colorA * colorB) + (colorC * colodrD)
    // Colors will be saturated
    output reg  [PIXEL_WIDTH - 1 : 0]   mixedColor
);
    localparam SIGN_WIDTH = 1;
    localparam SUB_PIXEL_SIGNED_WIDTH = SUB_PIXEL_WIDTH + SIGN_WIDTH;
    localparam PIXEL_SIGNED_WIDTH = SUB_PIXEL_SIGNED_WIDTH * NUMBER_OF_SUB_PIXEL;

    localparam SUB_PIXEL_0_POS = SUB_PIXEL_WIDTH * 0;
    localparam SUB_PIXEL_1_POS = SUB_PIXEL_WIDTH * 1;
    localparam SUB_PIXEL_2_POS = SUB_PIXEL_WIDTH * 2;
    localparam SUB_PIXEL_3_POS = SUB_PIXEL_WIDTH * 3;

    localparam SUB_PIXEL_0_SIGNED_POS = SUB_PIXEL_SIGNED_WIDTH * 0;
    localparam SUB_PIXEL_1_SIGNED_POS = SUB_PIXEL_SIGNED_WIDTH * 1;
    localparam SUB_PIXEL_2_SIGNED_POS = SUB_PIXEL_SIGNED_WIDTH * 2;
    localparam SUB_PIXEL_3_SIGNED_POS = SUB_PIXEL_SIGNED_WIDTH * 3;

    `SaturateCastSignedToUnsigned(SaturateCastSignedToUnsigned, SUB_PIXEL_SIGNED_WIDTH);  

    wire [PIXEL_SIGNED_WIDTH - 1 : 0] mixedColorSigned;

    ColorMixerSigned #(
        .SUB_PIXEL_WIDTH(SUB_PIXEL_SIGNED_WIDTH)
    ) colorMixer (
        .aclk(aclk),
        .resetn(resetn),
        
        .colorA({
            1'b0, colorA[SUB_PIXEL_0_POS +: SUB_PIXEL_WIDTH], 
            1'b0, colorA[SUB_PIXEL_1_POS +: SUB_PIXEL_WIDTH], 
            1'b0, colorA[SUB_PIXEL_2_POS +: SUB_PIXEL_WIDTH], 
            1'b0, colorA[SUB_PIXEL_3_POS +: SUB_PIXEL_WIDTH]
        }),
        .colorB({
            1'b0, colorB[SUB_PIXEL_0_POS +: SUB_PIXEL_WIDTH], 
            1'b0, colorB[SUB_PIXEL_1_POS +: SUB_PIXEL_WIDTH], 
            1'b0, colorB[SUB_PIXEL_2_POS +: SUB_PIXEL_WIDTH], 
            1'b0, colorB[SUB_PIXEL_3_POS +: SUB_PIXEL_WIDTH]
        }),
        .colorC({
            1'b0, colorC[SUB_PIXEL_0_POS +: SUB_PIXEL_WIDTH], 
            1'b0, colorC[SUB_PIXEL_1_POS +: SUB_PIXEL_WIDTH], 
            1'b0, colorC[SUB_PIXEL_2_POS +: SUB_PIXEL_WIDTH], 
            1'b0, colorC[SUB_PIXEL_3_POS +: SUB_PIXEL_WIDTH]
        }),
        .colorD({
            1'b0, colorD[SUB_PIXEL_0_POS +: SUB_PIXEL_WIDTH], 
            1'b0, colorD[SUB_PIXEL_1_POS +: SUB_PIXEL_WIDTH], 
            1'b0, colorD[SUB_PIXEL_2_POS +: SUB_PIXEL_WIDTH], 
            1'b0, colorD[SUB_PIXEL_3_POS +: SUB_PIXEL_WIDTH]
        }),

        .mixedColor(mixedColorSigned)
    );

    assign mixedColor = {
        SaturateCastSignedToUnsigned(mixedColorSigned[SUB_PIXEL_0_SIGNED_POS +: SUB_PIXEL_SIGNED_WIDTH]),
        SaturateCastSignedToUnsigned(mixedColorSigned[SUB_PIXEL_1_SIGNED_POS +: SUB_PIXEL_SIGNED_WIDTH]),
        SaturateCastSignedToUnsigned(mixedColorSigned[SUB_PIXEL_2_SIGNED_POS +: SUB_PIXEL_SIGNED_WIDTH]),
        SaturateCastSignedToUnsigned(mixedColorSigned[SUB_PIXEL_3_SIGNED_POS +: SUB_PIXEL_SIGNED_WIDTH])
    };
endmodule
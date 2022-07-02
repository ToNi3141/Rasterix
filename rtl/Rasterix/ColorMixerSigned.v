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
// It expects signed sub pixels.
// Pipelined: yes
// Depth: 2 cycles
module ColorMixerSigned #(
    parameter SUB_PIXEL_WIDTH = 9,
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
    localparam SUB_PIXEL_UNSIGNED_WIDTH = SUB_PIXEL_WIDTH - 1;
    localparam SIGN_WIDTH = 1;
    localparam SUB_PIXEL_WIDTH_BIG = (SUB_PIXEL_UNSIGNED_WIDTH * 2) + SIGN_WIDTH;
    localparam SUB_PIXEL_WIDTH_OVERSIZE = SUB_PIXEL_WIDTH_BIG + 1;
    localparam SUB_PIXEL_0_POS = SUB_PIXEL_WIDTH * 0;
    localparam SUB_PIXEL_1_POS = SUB_PIXEL_WIDTH * 1;
    localparam SUB_PIXEL_2_POS = SUB_PIXEL_WIDTH * 2;
    localparam SUB_PIXEL_3_POS = SUB_PIXEL_WIDTH * 3;

    localparam [SUB_PIXEL_WIDTH_BIG - 1 : 0] ONE_DOT_ZERO = { 1'b0, { SUB_PIXEL_UNSIGNED_WIDTH{1'b0} }, { SUB_PIXEL_UNSIGNED_WIDTH{1'b1} } };
    `ReduceAndSaturateSigned(ReduceAndSaturateSigned, SUB_PIXEL_WIDTH_OVERSIZE, SUB_PIXEL_WIDTH);  

    reg signed [SUB_PIXEL_WIDTH_BIG - 1 : 0] V00;
    reg signed [SUB_PIXEL_WIDTH_BIG - 1 : 0] V01;
    reg signed [SUB_PIXEL_WIDTH_BIG - 1 : 0] V02;
    reg signed [SUB_PIXEL_WIDTH_BIG - 1 : 0] V03;
    reg signed [SUB_PIXEL_WIDTH_BIG - 1 : 0] V10;
    reg signed [SUB_PIXEL_WIDTH_BIG - 1 : 0] V11;
    reg signed [SUB_PIXEL_WIDTH_BIG - 1 : 0] V12;
    reg signed [SUB_PIXEL_WIDTH_BIG - 1 : 0] V13;
    always @(posedge aclk)
    begin : Blending
        reg signed [SUB_PIXEL_WIDTH - 1 : 0] ca0;
        reg signed [SUB_PIXEL_WIDTH - 1 : 0] ca1;
        reg signed [SUB_PIXEL_WIDTH - 1 : 0] ca2;
        reg signed [SUB_PIXEL_WIDTH - 1 : 0] ca3;
        reg signed [SUB_PIXEL_WIDTH - 1 : 0] cb0;
        reg signed [SUB_PIXEL_WIDTH - 1 : 0] cb1;
        reg signed [SUB_PIXEL_WIDTH - 1 : 0] cb2;
        reg signed [SUB_PIXEL_WIDTH - 1 : 0] cb3;
        reg signed [SUB_PIXEL_WIDTH - 1 : 0] cc0;
        reg signed [SUB_PIXEL_WIDTH - 1 : 0] cc1;
        reg signed [SUB_PIXEL_WIDTH - 1 : 0] cc2;
        reg signed [SUB_PIXEL_WIDTH - 1 : 0] cc3;
        reg signed [SUB_PIXEL_WIDTH - 1 : 0] cd0;
        reg signed [SUB_PIXEL_WIDTH - 1 : 0] cd1;
        reg signed [SUB_PIXEL_WIDTH - 1 : 0] cd2;
        reg signed [SUB_PIXEL_WIDTH - 1 : 0] cd3;

        ca0 = colorA[SUB_PIXEL_0_POS +: SUB_PIXEL_WIDTH];
        ca1 = colorA[SUB_PIXEL_1_POS +: SUB_PIXEL_WIDTH];
        ca2 = colorA[SUB_PIXEL_2_POS +: SUB_PIXEL_WIDTH];
        ca3 = colorA[SUB_PIXEL_3_POS +: SUB_PIXEL_WIDTH];

        cb0 = colorB[SUB_PIXEL_0_POS +: SUB_PIXEL_WIDTH];
        cb1 = colorB[SUB_PIXEL_1_POS +: SUB_PIXEL_WIDTH];
        cb2 = colorB[SUB_PIXEL_2_POS +: SUB_PIXEL_WIDTH];
        cb3 = colorB[SUB_PIXEL_3_POS +: SUB_PIXEL_WIDTH];

        cc0 = colorC[SUB_PIXEL_0_POS +: SUB_PIXEL_WIDTH];
        cc1 = colorC[SUB_PIXEL_1_POS +: SUB_PIXEL_WIDTH];
        cc2 = colorC[SUB_PIXEL_2_POS +: SUB_PIXEL_WIDTH];
        cc3 = colorC[SUB_PIXEL_3_POS +: SUB_PIXEL_WIDTH];

        cd0 = colorD[SUB_PIXEL_0_POS +: SUB_PIXEL_WIDTH];
        cd1 = colorD[SUB_PIXEL_1_POS +: SUB_PIXEL_WIDTH];
        cd2 = colorD[SUB_PIXEL_2_POS +: SUB_PIXEL_WIDTH];
        cd3 = colorD[SUB_PIXEL_3_POS +: SUB_PIXEL_WIDTH];

        V00 <= (ca0 * cb0);
        V01 <= (ca1 * cb1);
        V02 <= (ca2 * cb2);
        V03 <= (ca3 * cb3);

        V10 <= (cc0 * cd0);
        V11 <= (cc1 * cd1);
        V12 <= (cc2 * cd2);
        V13 <= (cc3 * cd3);
    end

    always @(posedge aclk)
    begin : Result
        reg signed [SUB_PIXEL_WIDTH_OVERSIZE - 1 : 0] c0;
        reg signed [SUB_PIXEL_WIDTH_OVERSIZE - 1 : 0] c1;
        reg signed [SUB_PIXEL_WIDTH_OVERSIZE - 1 : 0] c2;
        reg signed [SUB_PIXEL_WIDTH_OVERSIZE - 1 : 0] c3;

        c0 = (V00 + V10) + ONE_DOT_ZERO;
        c1 = (V01 + V11) + ONE_DOT_ZERO;
        c2 = (V02 + V12) + ONE_DOT_ZERO;
        c3 = (V03 + V13) + ONE_DOT_ZERO;

        mixedColor <= {
            ReduceAndSaturateSigned(c3),
            ReduceAndSaturateSigned(c2),
            ReduceAndSaturateSigned(c1),
            ReduceAndSaturateSigned(c0)
        };
    end
endmodule
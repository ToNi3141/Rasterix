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

// Interpolates a color with 4 sub color elements together like RGBA4444
// The used caluclation: mixedColor = (intensity * colorA) + ((1.0 - intensity) * colorB)
// Pipelined: yes
// Depth: 2 cycles
module ColorInterpolator #(
    localparam SUB_PIXEL_WIDTH = 8,
    localparam PIXEL_WIDTH = SUB_PIXEL_WIDTH * 4
)
(
    input wire                          aclk,
    input wire                          resetn,

    input wire  [15 : 0]                intensity, // Intensity of color a or 1.0 - intensity of color b. 1.0 == 0xffff, 0.0 == 0x0
    input wire  [PIXEL_WIDTH - 1 : 0]   colorA,
    input wire  [PIXEL_WIDTH - 1 : 0]   colorB,

    output reg  [PIXEL_WIDTH - 1 : 0]   mixedColor
);
`include "RegisterAndDescriptorDefines.vh"

    reg [(SUB_PIXEL_WIDTH * 2) - 1 : 0] V00;
    reg [(SUB_PIXEL_WIDTH * 2) - 1 : 0] V01;
    reg [(SUB_PIXEL_WIDTH * 2) - 1 : 0] V02;
    reg [(SUB_PIXEL_WIDTH * 2) - 1 : 0] V03;
    reg [(SUB_PIXEL_WIDTH * 2) - 1 : 0] V10;
    reg [(SUB_PIXEL_WIDTH * 2) - 1 : 0] V11;
    reg [(SUB_PIXEL_WIDTH * 2) - 1 : 0] V12;
    reg [(SUB_PIXEL_WIDTH * 2) - 1 : 0] V13;
    always @(posedge aclk)
    begin : Blending
        reg [SUB_PIXEL_WIDTH - 1 : 0] ra;
        reg [SUB_PIXEL_WIDTH - 1 : 0] ga;
        reg [SUB_PIXEL_WIDTH - 1 : 0] ba;
        reg [SUB_PIXEL_WIDTH - 1 : 0] aa;
        reg [SUB_PIXEL_WIDTH - 1 : 0] rb;
        reg [SUB_PIXEL_WIDTH - 1 : 0] gb;
        reg [SUB_PIXEL_WIDTH - 1 : 0] bb;
        reg [SUB_PIXEL_WIDTH - 1 : 0] ab;
        reg [(SUB_PIXEL_WIDTH * 2) - 1 : 0] in;

        in = { 8'h0, intensity[16 - SUB_PIXEL_WIDTH +: SUB_PIXEL_WIDTH] };

        ra = colorA[COLOR_R_POS +: SUB_PIXEL_WIDTH];
        ga = colorA[COLOR_G_POS +: SUB_PIXEL_WIDTH];
        ba = colorA[COLOR_B_POS +: SUB_PIXEL_WIDTH];
        aa = colorA[COLOR_A_POS +: SUB_PIXEL_WIDTH];

        rb = colorB[COLOR_R_POS +: SUB_PIXEL_WIDTH];
        gb = colorB[COLOR_G_POS +: SUB_PIXEL_WIDTH];
        bb = colorB[COLOR_B_POS +: SUB_PIXEL_WIDTH];
        ab = colorB[COLOR_A_POS +: SUB_PIXEL_WIDTH];

        V00 <= (in * ra);
        V01 <= (in * ga);
        V02 <= (in * ba);
        V03 <= (in * aa);

        V10 <= ((8'hff - in) * rb);
        V11 <= ((8'hff - in) * gb);
        V12 <= ((8'hff - in) * bb);
        V13 <= ((8'hff - in) * ab);
    end

    always @(posedge aclk)
    begin : Result
        reg [(SUB_PIXEL_WIDTH * 2) : 0] r;
        reg [(SUB_PIXEL_WIDTH * 2) : 0] g;
        reg [(SUB_PIXEL_WIDTH * 2) : 0] b;
        reg [(SUB_PIXEL_WIDTH * 2) : 0] a;

        r = (V00 + V10) + 8'hff;
        g = (V01 + V11) + 8'hff;
        b = (V02 + V12) + 8'hff;
        a = (V03 + V13) + 8'hff;

        mixedColor <= {
            // Saturate colors 
            (r[16]) ? 8'hff : r[15 : 8], 
            (g[16]) ? 8'hff : g[15 : 8], 
            (b[16]) ? 8'hff : b[15 : 8],
            (a[16]) ? 8'hff : a[15 : 8]
        };
    end
endmodule
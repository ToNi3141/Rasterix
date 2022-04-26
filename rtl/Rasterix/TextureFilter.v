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

// This module filters the texture. It gets 4 pixel from the texture buffer inclusive 
// the sub pixel information where the sample lies exactly.
// Pipelined: yes
// Depth: 4 cylces
module TextureFilter #(
    localparam SUB_PIXEL_WIDTH = 4,
    localparam PIXEL_WIDTH = SUB_PIXEL_WIDTH * 4
) 
(
    input  wire                         aclk,
    input  wire                         resetn,

    input  wire [PIXEL_WIDTH - 1 : 0]   texel00,
    input  wire [PIXEL_WIDTH - 1 : 0]   texel01,
    input  wire [PIXEL_WIDTH - 1 : 0]   texel10,
    input  wire [PIXEL_WIDTH - 1 : 0]   texel11,
    input  wire [15 : 0]                texelSubCoordX,
    input  wire [15 : 0]                texelSubCoordY,

    output wire [PIXEL_WIDTH - 1 : 0]   texel
);
    wire [15 : 0]               intensityDelayedY;
    wire [PIXEL_WIDTH - 1 : 0]  mixedColorX0;
    wire [PIXEL_WIDTH - 1 : 0]  mixedColorX1;
    wire [15 : 0]               intensityX;
    wire [15 : 0]               intensityY;

    assign intensityX = 16'hffff - texelSubCoordX;
    assign intensityY = 16'hffff - texelSubCoordY;

    ColorInterpolator interpolatorX0 (
        .aclk(aclk),
        .resetn(resetn),

        .intensity(intensityX),
        .colorA(texel00),
        .colorB(texel01),
        .mixedColor(mixedColorX0)
    );

    ColorInterpolator interpolatorX1 (
        .aclk(aclk),
        .resetn(resetn),

        .intensity(intensityX),
        .colorA(texel10),
        .colorB(texel11),
        .mixedColor(mixedColorX1)
    );

    ColorInterpolator interpolatorY (
        .aclk(aclk),
        .resetn(resetn),

        .intensity(intensityDelayedY),
        .colorA(mixedColorX0),
        .colorB(mixedColorX1),
        .mixedColor(texel)
    );

    // Delay intensityY by 2 clocks (to calculate the X interpolations)
    ValueDelay #(
        .VALUE_SIZE(16), 
        .DELAY(2)
    ) 
    intensityYDelay (
        .clk(aclk), 
        .in(intensityY), 
        .out(intensityDelayedY)
    );

    // ValueDelay #(
    //     .VALUE_SIZE(PIXEL_WIDTH), 
    //     .DELAY(4)
    // ) 
    // texelDelay (
    //     .clk(aclk), 
    //     .in(texel00), 
    //     .out(texel)
    // );

endmodule
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

    input  wire                         enable,

    input  wire [PIXEL_WIDTH - 1 : 0]   texel00,
    input  wire [PIXEL_WIDTH - 1 : 0]   texel01,
    input  wire [PIXEL_WIDTH - 1 : 0]   texel10,
    input  wire [PIXEL_WIDTH - 1 : 0]   texel11,
    input  wire [15 : 0]                texelSubCoordS,
    input  wire [15 : 0]                texelSubCoordT,

    output wire [PIXEL_WIDTH - 1 : 0]   texel
);
    wire [15 : 0]               intensityDelayedT;
    wire [PIXEL_WIDTH - 1 : 0]  mixedColorS0;
    wire [PIXEL_WIDTH - 1 : 0]  mixedColorS1;
    wire [15 : 0]               intensityS;
    wire [15 : 0]               intensityT;
    wire [PIXEL_WIDTH - 1 : 0]  filteredTexel;
    wire [PIXEL_WIDTH - 1 : 0]  unfilteredTexel;

    assign intensityS = 16'hffff - texelSubCoordS;
    assign intensityT = 16'hffff - texelSubCoordT;

    ColorInterpolator interpolatorS0 (
        .aclk(aclk),
        .resetn(resetn),

        .intensity(intensityS),
        .colorA(texel00),
        .colorB(texel01),
        .mixedColor(mixedColorS0)
    );

    ColorInterpolator interpolatorS1 (
        .aclk(aclk),
        .resetn(resetn),

        .intensity(intensityS),
        .colorA(texel10),
        .colorB(texel11),
        .mixedColor(mixedColorS1)
    );

    ColorInterpolator interpolatorT (
        .aclk(aclk),
        .resetn(resetn),

        .intensity(intensityDelayedT),
        .colorA(mixedColorS0),
        .colorB(mixedColorS1),
        .mixedColor(filteredTexel)
    );

    // Delay intensityT by 2 clocks (to calculate the S interpolations)
    ValueDelay #(
        .VALUE_SIZE(16), 
        .DELAY(2)
    ) 
    intensityTDelay (
        .clk(aclk), 
        .in(intensityT), 
        .out(intensityDelayedT)
    );

    // To delay the texel00 to use it as an unfiltered texel
    ValueDelay #(
        .VALUE_SIZE(PIXEL_WIDTH), 
        .DELAY(4)
    ) 
    texelDelay (
        .clk(aclk), 
        .in(texel00), 
        .out(unfilteredTexel)
    );

    assign texel = (enable) ? filteredTexel : unfilteredTexel;

endmodule
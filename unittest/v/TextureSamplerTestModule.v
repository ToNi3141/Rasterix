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

// This is a test module for the TextureSampler and TextureBuffer.
// It instantiates both of them to have with the TextureBuffer a mock for the TextureSampler to ease the testing.
module TextureSamplerTestModule #(
    parameter STREAM_WIDTH = 32,

    localparam TEX_ADDR_WIDTH = 17,
    localparam PIXEL_WIDTH = 32
) 
(
    input  wire                         aclk,
    input  wire                         resetn,

    // Texture size
    // textureSize * 2. 0 equals 1px. 1 equals 2px. 2 equals 4px... Only power of two are allowed.
    input  wire [ 3 : 0]                textureSizeWidth, 
    input  wire [ 3 : 0]                textureSizeHeight,
    input  wire [ 3 : 0]                textureLod,
    input  wire                         enableHalfPixelOffset,
    input  wire [ 3 : 0]                confPixelFormat,

    input  wire [31 : 0]                texelS, // S16.15
    input  wire [31 : 0]                texelT, // S16.15
    input  wire                         clampS,
    input  wire                         clampT,
    output wire [PIXEL_WIDTH - 1 : 0]   texel00, // (0, 0), as (s, t). s and t are switched since the address is constructed like {texelT, texelS}
    output wire [PIXEL_WIDTH - 1 : 0]   texel01, // (1, 0)
    output wire [PIXEL_WIDTH - 1 : 0]   texel10, // (0, 1)
    output wire [PIXEL_WIDTH - 1 : 0]   texel11, // (1, 1)

    // This is basically the faction of the pixel coordinate and has a range from 0.0 (0x0) to 0.999... (0xffff)
    // The integer part is not required, since the integer part only adresses the pixel and we don't care about that.
    // We just care about the coordinates within the texel quad. And if there the coordinate gets >1.0, that means, we
    // are outside of our quad which never happens.
    output wire [15 : 0]                texelSubCoordS, // Q0.16
    output wire [15 : 0]                texelSubCoordT, // Q0.16

    // Texture Write
    input  wire                         s_axis_tvalid,
    output reg                          s_axis_tready,
    input  wire                         s_axis_tlast,
    input  wire [STREAM_WIDTH - 1 : 0]  s_axis_tdata
);

    wire [TEX_ADDR_WIDTH - 1 : 0]   texelAddr00;
    wire [TEX_ADDR_WIDTH - 1 : 0]   texelAddr01;
    wire [TEX_ADDR_WIDTH - 1 : 0]   texelAddr10;
    wire [TEX_ADDR_WIDTH - 1 : 0]   texelAddr11;
    wire [PIXEL_WIDTH - 1 : 0]      texelInput00;
    wire [PIXEL_WIDTH - 1 : 0]      texelInput01;
    wire [PIXEL_WIDTH - 1 : 0]      texelInput10;
    wire [PIXEL_WIDTH - 1 : 0]      texelInput11;

    TextureBuffer #(
        .PIXEL_WIDTH(PIXEL_WIDTH)
    ) texCache (
        .aclk(aclk),
        .resetn(resetn),

        .confPixelFormat(confPixelFormat),

        .texelAddr00(texelAddr00),
        .texelAddr01(texelAddr01),
        .texelAddr10(texelAddr10),
        .texelAddr11(texelAddr11),

        .texelOutput00(texelInput00),
        .texelOutput01(texelInput01),
        .texelOutput10(texelInput10),
        .texelOutput11(texelInput11),

        .s_axis_tvalid(s_axis_tvalid),
        .s_axis_tready(s_axis_tready),
        .s_axis_tlast(s_axis_tlast),
        .s_axis_tdata(s_axis_tdata)
    );

    TextureSampler #(
        .PIXEL_WIDTH(PIXEL_WIDTH)
    ) textureSampler (
        .aclk(aclk),
        .resetn(resetn),

        .textureSizeWidth(textureSizeWidth),
        .textureSizeHeight(textureSizeHeight),
        .textureLod(textureLod),
        .enableHalfPixelOffset(enableHalfPixelOffset),

        .texelAddr00(texelAddr00),
        .texelAddr01(texelAddr01),
        .texelAddr10(texelAddr10),
        .texelAddr11(texelAddr11),
        .texelInput00(texelInput00),
        .texelInput01(texelInput01),
        .texelInput10(texelInput10),
        .texelInput11(texelInput11),

        .texelS(texelS),
        .texelT(texelT),
        .clampS(clampS),
        .clampT(clampT),
        .texel00(texel00),
        .texel01(texel01),
        .texel10(texel10),
        .texel11(texel11),

        .texelSubCoordS(texelSubCoordS),
        .texelSubCoordT(texelSubCoordT)
    );

endmodule
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

// This module calculates the fragment color.
// It samples a texel from the texture memory, filters it and 
// executes the texture environment.
// Pipelined: yes
// Depth: 9 cycles
module TextureMappingUnit
#(
    parameter CMD_STREAM_WIDTH = 64,

    parameter SUB_PIXEL_WIDTH = 8,
    localparam PIXEL_WIDTH = 4 * SUB_PIXEL_WIDTH,

    localparam FLOAT_SIZE = 32,
    localparam ADDR_WIDTH = 16 // Based on the maximum texture size, which is 256x256 (8 bit x 8 bit) in PIXEL_WIDTH word addresses
)
(
    input  wire                         aclk,
    input  wire                         resetn,

    // TMU configurations
    input  wire [ 2 : 0]                confFunc, // See TexEnv for more documentation
    input  wire                         confTextureClampS,
    input  wire                         confTextureClampT,
    input  wire [PIXEL_WIDTH - 1 : 0]   confTextureEnvColor, // CONSTANT
    input  wire [ 7 : 0]                confTextureSizeWidth, 
    input  wire [ 7 : 0]                confTextureSizeHeight,
    input  wire                         confTextureMagFilter,

    // Texture memory access of a texel quad
    output wire [ADDR_WIDTH - 1 : 0]    texelAddr00,
    output wire [ADDR_WIDTH - 1 : 0]    texelAddr01,
    output wire [ADDR_WIDTH - 1 : 0]    texelAddr10,
    output wire [ADDR_WIDTH - 1 : 0]    texelAddr11,
    input  wire [PIXEL_WIDTH - 1 : 0]   texelInput00,
    input  wire [PIXEL_WIDTH - 1 : 0]   texelInput01,
    input  wire [PIXEL_WIDTH - 1 : 0]   texelInput10,
    input  wire [PIXEL_WIDTH - 1 : 0]   texelInput11,

    // Fragment input
    input  wire [PIXEL_WIDTH - 1 : 0]   primaryColor, // PRIMARY_COLOR
    input  wire [31 : 0]                textureS,
    input  wire [31 : 0]                textureT,
    
    input  wire [PIXEL_WIDTH - 1 : 0]   previousColor, // PREVIOUS (currently unused, must be connected to TMUn-1)
    
    // Fragment output
    output wire [PIXEL_WIDTH - 1 : 0]   fragmentColor
);
`include "RegisterAndDescriptorDefines.vh"

    function [15 : 0] clampTexture;
        input [23 : 0] texCoord;
        input [ 0 : 0] mode; 
        begin
            clampTexture = texCoord[0 +: 16];
            if (mode == CLAMP_TO_EDGE)
            begin
                if (texCoord[23]) // Check if it lower than 0 by only checking the sign bit
                begin
                    clampTexture = 0;
                end
                else if ((texCoord >> 15) != 0) // Check if it is greater than one by checking if the integer part is unequal to zero
                begin
                    clampTexture = 16'h7fff;
                end  
            end
        end
    endfunction

    ////////////////////////////////////////////////////////////////////////////
    // STEP 1
    // Request texel from texture buffer and filter it
    // Clocks: 7
    ////////////////////////////////////////////////////////////////////////////
    wire [PIXEL_WIDTH - 1 : 0]  step1_primaryColor;
    wire [PIXEL_WIDTH - 1 : 0]  step1_texel; // TEXTURE

    ValueDelay #(.VALUE_SIZE(PIXEL_WIDTH), .DELAY(7)) 
        step1_primaryColorDelay (.clk(aclk), .in(primaryColor), .out(step1_primaryColor));

    wire [PIXEL_WIDTH - 1 : 0]  step1_texel00Tmp;
    wire [PIXEL_WIDTH - 1 : 0]  step1_texel01Tmp;
    wire [PIXEL_WIDTH - 1 : 0]  step1_texel10Tmp;
    wire [PIXEL_WIDTH - 1 : 0]  step1_texel11Tmp;
    wire [15:0]                 step1_texelSubCoordSTmp;
    wire [15:0]                 step1_texelSubCoordTTmp;
    TextureSampler #(
        .PIXEL_WIDTH(PIXEL_WIDTH)
    ) textureSampler (
        .aclk(aclk),
        .resetn(resetn),

        .textureSizeWidth(confTextureSizeWidth),
        .textureSizeHeight(confTextureSizeHeight),

        .texelAddr00(texelAddr00),
        .texelAddr01(texelAddr01),
        .texelAddr10(texelAddr10),
        .texelAddr11(texelAddr11),
        .texelInput00(texelInput00),
        .texelInput01(texelInput01),
        .texelInput10(texelInput10),
        .texelInput11(texelInput11),

        .texelS(clampTexture(textureS[0 +: 24], confTextureClampS)),
        .texelT(clampTexture(textureT[0 +: 24], confTextureClampT)),
        .clampS(confTextureClampS),
        .clampT(confTextureClampT),
        .texel00(step1_texel00Tmp),
        .texel01(step1_texel01Tmp),
        .texel10(step1_texel10Tmp),
        .texel11(step1_texel11Tmp),

        .texelSubCoordS(step1_texelSubCoordSTmp),
        .texelSubCoordT(step1_texelSubCoordTTmp)
    );

    TextureFilter texFilter (
        .aclk(aclk),
        .resetn(resetn),

        .enable(confTextureMagFilter),

        .texel00(step1_texel00Tmp),
        .texel01(step1_texel01Tmp),
        .texel10(step1_texel10Tmp),
        .texel11(step1_texel11Tmp),
        .texelSubCoordS(step1_texelSubCoordSTmp),
        .texelSubCoordT(step1_texelSubCoordTTmp),

        .texel(step1_texel)
    );

    ////////////////////////////////////////////////////////////////////////////
    // STEP 2
    // Calculate texture environment
    // Clocks: 2
    ////////////////////////////////////////////////////////////////////////////
    wire [PIXEL_WIDTH - 1 : 0]  step2_texel;

    TexEnv #(
        .SUB_PIXEL_WIDTH(SUB_PIXEL_WIDTH)
    ) texEnv (
        .aclk(aclk),
        .resetn(resetn),

        .func(confFunc),

        .texSrcColor(step1_texel),
        .primaryColor(step1_primaryColor),
        .envColor(confTextureEnvColor),

        .color(step2_texel)
    );

    ////////////////////////////////////////////////////////////////////////////
    // STEP 3
    // Output final texel color
    // Clocks: 0
    ////////////////////////////////////////////////////////////////////////////
    assign fragmentColor = step2_texel;

endmodule


 
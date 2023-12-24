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

// This module calculates the fragment color.
// It samples a texel from the texture memory, filters it and 
// executes the texture environment.
// Pipelined: yes
// Depth: 11 cycles
module TextureMappingUnit
#(
    parameter CMD_STREAM_WIDTH = 64,

    parameter SUB_PIXEL_WIDTH = 8,
    localparam PIXEL_WIDTH = 4 * SUB_PIXEL_WIDTH,

    localparam FLOAT_SIZE = 32,
    localparam ADDR_WIDTH = 17 // Based on the maximum texture size, of 256x256 (8 bit x 8 bit) + mipmap levels in PIXEL_WIDTH word addresses
)
(
    input  wire                         aclk,
    input  wire                         resetn,

    // TMU configurations
    input  wire [ 31 : 0]               confFunc, // See TexEnv for more documentation
    input  wire [PIXEL_WIDTH - 1 : 0]   confTextureEnvColor, // CONSTANT
    input  wire [ 31 : 0]               confTextureConfig,
    input  wire                         confEnable,

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
    input  wire [31 : 0]                mipmapS,
    input  wire [31 : 0]                mipmapT,
    
    input  wire [PIXEL_WIDTH - 1 : 0]   previousColor, // PREVIOUS
    
    // Fragment output
    output wire [PIXEL_WIDTH - 1 : 0]   fragmentColor
);
`include "RegisterAndDescriptorDefines.vh"

    ////////////////////////////////////////////////////////////////////////////
    // STEP 1
    // Request texel from texture buffer and filter it
    // Clocks: 8
    ////////////////////////////////////////////////////////////////////////////
    wire [PIXEL_WIDTH - 1 : 0]  step1_primaryColor;
    wire [PIXEL_WIDTH - 1 : 0]  step1_texel; // TEXTURE
    wire [PIXEL_WIDTH - 1 : 0]  step1_previousColor;

    ValueDelay #(.VALUE_SIZE(PIXEL_WIDTH), .DELAY(8)) 
        step1_primaryColorDelay (.clk(aclk), .in(primaryColor), .out(step1_primaryColor));

    ValueDelay #(.VALUE_SIZE(PIXEL_WIDTH), .DELAY(8)) 
        step1_previousColorDelay (.clk(aclk), .in(previousColor), .out(step1_previousColor));

    wire [31 : 0] textureSDly;
    wire [31 : 0] textureTDly;
    ValueDelay #(.VALUE_SIZE(PIXEL_WIDTH), .DELAY(1)) 
        step1_textureSDelay (.clk(aclk), .in(textureS), .out(textureSDly));
    ValueDelay #(.VALUE_SIZE(PIXEL_WIDTH), .DELAY(1)) 
        step1_textureTDelay (.clk(aclk), .in(textureT), .out(textureTDly));

    reg [ 3 : 0] lod;
    LodCalculator lodCalculator (
        .aclk(aclk),
        .resetn(resetn),

        .textureSizeWidth(confTextureConfig[RENDER_CONFIG_TMU_TEXTURE_WIDTH_POS +: RENDER_CONFIG_TMU_TEXTURE_WIDTH_SIZE]),
        .textureSizeHeight(confTextureConfig[RENDER_CONFIG_TMU_TEXTURE_HEIGHT_POS +: RENDER_CONFIG_TMU_TEXTURE_HEIGHT_SIZE]),

        .texelS(textureS),
        .texelT(textureT),

        .texelSxy(mipmapS),
        .texelTxy(mipmapT),

        .lod(lod)
    );

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

        .textureSizeWidth(confTextureConfig[RENDER_CONFIG_TMU_TEXTURE_WIDTH_POS +: RENDER_CONFIG_TMU_TEXTURE_WIDTH_SIZE]),
        .textureSizeHeight(confTextureConfig[RENDER_CONFIG_TMU_TEXTURE_HEIGHT_POS +: RENDER_CONFIG_TMU_TEXTURE_HEIGHT_SIZE]),
        .textureLod(lod),
        .enableHalfPixelOffset(confTextureConfig[RENDER_CONFIG_TMU_TEXTURE_MAG_FILTER_POS +: RENDER_CONFIG_TMU_TEXTURE_MAG_FILTER_SIZE]), 

        .texelAddr00(texelAddr00),
        .texelAddr01(texelAddr01),
        .texelAddr10(texelAddr10),
        .texelAddr11(texelAddr11),
        .texelInput00(texelInput00),
        .texelInput01(texelInput01),
        .texelInput10(texelInput10),
        .texelInput11(texelInput11),

        .texelS(textureSDly),
        .texelT(textureTDly),
        .clampS(confTextureConfig[RENDER_CONFIG_TMU_TEXTURE_CLAMP_S_POS +: RENDER_CONFIG_TMU_TEXTURE_CLAMP_S_SIZE]),
        .clampT(confTextureConfig[RENDER_CONFIG_TMU_TEXTURE_CLAMP_T_POS +: RENDER_CONFIG_TMU_TEXTURE_CLAMP_T_SIZE]),
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

        .enable(confTextureConfig[RENDER_CONFIG_TMU_TEXTURE_MAG_FILTER_POS +: RENDER_CONFIG_TMU_TEXTURE_MAG_FILTER_SIZE]),

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
    // Clocks: 4
    ////////////////////////////////////////////////////////////////////////////
    wire [PIXEL_WIDTH - 1 : 0]  step2_texel;
    wire [PIXEL_WIDTH - 1 : 0]  step2_previousColor;

    ValueDelay #(.VALUE_SIZE(PIXEL_WIDTH), .DELAY(4)) 
        step2_previousColorDelay (.clk(aclk), .in(step1_previousColor), .out(step2_previousColor));

    TexEnv #(
        .SUB_PIXEL_WIDTH(SUB_PIXEL_WIDTH)
    ) texEnv (
        .aclk(aclk),
        .resetn(resetn),

        .conf(confFunc),

        .previousColor(step1_previousColor),
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
    assign fragmentColor = (confEnable) ? step2_texel : step2_previousColor;

endmodule


 
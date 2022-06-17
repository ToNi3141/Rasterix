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
// It reads the texel from the texture memory and clamps it, calculates 
// the fog intensity based on the z value and fog equation (LUT), 
// calculates the equation for the texture environment and applies the
// fog color.
// Pipelined: yes
// Depth: 9 cycles
module TextureMappingUnit
#(
    parameter CMD_STREAM_WIDTH = 64,

    parameter SUB_PIXEL_WIDTH = 8,
    localparam PIXEL_WIDTH = 4 * SUB_PIXEL_WIDTH,

    localparam FLOAT_SIZE = 32
)
(
    input  wire                         aclk,
    input  wire                         resetn,

    // TMU configurations
    input  wire [ 2 : 0]                confFunc, // See TexEnv for more documentation
    input  wire                         confTextureClampS,
    input  wire                         confTextureClampT,
    input  wire [PIXEL_WIDTH - 1 : 0]   confTextureEnvColor, // CONSTANT

    // Texture access
    output wire [15 : 0]                texelS,
    output wire [15 : 0]                texelT,
    input  wire [PIXEL_WIDTH - 1 : 0]   texel, // TEXTURE

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
    // Request texel from texture buffer
    // Clocks: 7
    ////////////////////////////////////////////////////////////////////////////
    wire [PIXEL_WIDTH - 1 : 0]  step1_primaryColor;

    ValueDelay #(.VALUE_SIZE(PIXEL_WIDTH), .DELAY(7)) 
        step1_primaryColorDelay (.clk(aclk), .in(primaryColor), .out(step1_primaryColor));

    assign texelS = clampTexture(textureS[0 +: 24], confTextureClampS);
    assign texelT = clampTexture(textureT[0 +: 24], confTextureClampT);

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

        .texSrcColor(texel),
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


 
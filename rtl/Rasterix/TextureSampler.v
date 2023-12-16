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

// Gets texel coordinates and then samples a texel quad from the texture memory.
// It assumes that a memory with a fixed delay is used. The delay can be configured with MEMORY_DELAY
// It supports texture sizes from 1x1 to 256x256
// Delay: 3 clocks
module TextureSampler #(
    parameter MEMORY_DELAY = 1,
    parameter PIXEL_WIDTH = 32,
    localparam ADDR_WIDTH = 16 // Based on the maximum texture size, which is 256x256 (8 bit x 8 bit) in PIXEL_WIDTH word addresses
)
(
    input  wire                         aclk,
    input  wire                         resetn,

    // Texture size
    // textureSize * 2. 0 equals 1px. 1 equals 2px. 2 equals 4px... Only power of two are allowed.
    input  wire [ 3 : 0]                textureSizeWidth, 
    input  wire [ 3 : 0]                textureSizeHeight,
    input  wire                         enableHalfPixelOffset,

    // Texture memory access of a texel quad
    output reg  [ADDR_WIDTH - 1 : 0]    texelAddr00,
    output reg  [ADDR_WIDTH - 1 : 0]    texelAddr01,
    output reg  [ADDR_WIDTH - 1 : 0]    texelAddr10,
    output reg  [ADDR_WIDTH - 1 : 0]    texelAddr11,
    input  wire [PIXEL_WIDTH - 1 : 0]   texelInput00,
    input  wire [PIXEL_WIDTH - 1 : 0]   texelInput01,
    input  wire [PIXEL_WIDTH - 1 : 0]   texelInput10,
    input  wire [PIXEL_WIDTH - 1 : 0]   texelInput11,

    // Texture Read
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
    output wire [15 : 0]                texelSubCoordT // Q0.16
);
`include "RegisterAndDescriptorDefines.vh"

    function [PIXEL_WIDTH - 1 : 0] texelMux;
        input [ 1 : 0] sel;
        input [PIXEL_WIDTH - 1 : 0] t00;
        input [PIXEL_WIDTH - 1 : 0] t01;
        input [PIXEL_WIDTH - 1 : 0] t10;
        input [PIXEL_WIDTH - 1 : 0] t11;
        begin
            case (sel)
                2'd0: texelMux = t00;
                2'd1: texelMux = t01;
                2'd2: texelMux = t10;
                2'd3: texelMux = t11;
            endcase
        end
    endfunction

    function isPixelOutside;
        input [31 : 0] texCoord;
        input [ 0 : 0] mode;
        begin
            if (mode == CLAMP_TO_EDGE)
            begin
                if (texCoord[31]) // Check if it lower than 0 by only checking the sign bit
                begin
                    isPixelOutside = 1;
                end
                else if ((texCoord >> 15) != 0) // Check if it is greater than one by checking if the integer part is unequal to zero
                begin
                    isPixelOutside = 1;
                end 
                else
                begin
                    isPixelOutside = 0;
                end
            end
            else
            begin
                isPixelOutside = 0;
            end
        end
    endfunction

    // Convert the configured texture size into a 0.5.
    // The texture size comes as a integer value which is intepreted as a normalized 1.0.
    // Assuming a texture size of 4px (16'h100). To normalize this, the point (imaginary) must be moved to the diget bevore the 1.
    // But a 1.0 is different on a 4px texture and on a texture on another size like 16px (16'h10000). Therefore also the 
    // representation of 0.5 changes from 16'h010 to 16'h01000. This is what this function is intended for, to get a normalized
    // 0.5 depending on the size of the texture.
    function [31 : 0] convertToZeroPointFive;
        input [7 : 0] texSize;
        convertToZeroPointFive = 
        {
            1'h0, // sign
            16'h0, // integer part
            texSize[0],
            texSize[1],
            texSize[2],
            texSize[3],
            texSize[4],
            texSize[5],
            texSize[6],
            texSize[7],
            7'h0
        };
    endfunction

    // See convertToZeroPointFive. It is basically the same just for 1.0
    function [31 : 0] convertToOnePointZero;
        input [7 : 0] texSize;
        convertToOnePointZero = 
        {
            1'h0, // sign
            15'h0, // integer part
            texSize[0], // integer part
            texSize[1], // fraction part
            texSize[2],
            texSize[3],
            texSize[4],
            texSize[5],
            texSize[6],
            texSize[7],
            8'h0
        };
    endfunction

    //////////////////////////////////////////////
    // STEP 0
    // Build RAM adresses
    // Clocks: 1
    //////////////////////////////////////////////
    reg             step0_texelS0Valid;
    reg             step0_texelS1Valid;
    reg             step0_texelT0Valid;
    reg             step0_texelT1Valid;
    reg  [15 : 0]   step0_subCoordS; // Q0.16
    reg  [15 : 0]   step0_subCoordT; // Q0.16
    
    // Build the RAM adress of the given (s, t) coordinate and also generate the address of 
    // the texel quad
    always @(posedge aclk)
    begin : TexAddrCalc
        reg [ 7 : 0]    addrT0;
        reg [ 7 : 0]    addrT1;
        reg [31 : 0]    texelS0WithOffset; // S16.15
        reg [31 : 0]    texelS1WithOffset; // S16.15
        reg [31 : 0]    texelT0WithOffset; // S16.15
        reg [31 : 0]    texelT1WithOffset; // S16.15
        reg [14 : 0]    step0_texelS0; // Q0.15
        reg [14 : 0]    step0_texelS1; // Q0.15
        reg [14 : 0]    step0_texelT0; // Q0.15
        reg [14 : 0]    step0_texelT1; // Q0.15

        if (enableHalfPixelOffset)
        begin
            texelS0WithOffset = texelS - convertToZeroPointFive(1 << textureSizeWidth);
            texelS1WithOffset = texelS + convertToZeroPointFive(1 << textureSizeWidth);
            texelT0WithOffset = texelT - convertToZeroPointFive(1 << textureSizeHeight);
            texelT1WithOffset = texelT + convertToZeroPointFive(1 << textureSizeHeight);
        end
        else
        begin
            texelS0WithOffset = texelS;
            texelS1WithOffset = texelS + convertToOnePointZero(1 << textureSizeWidth);
            texelT0WithOffset = texelT;
            texelT1WithOffset = texelT + convertToOnePointZero(1 << textureSizeHeight);
        end

        step0_texelT0 = texelT0WithOffset[0 +: 15];
        step0_texelS0 = texelS0WithOffset[0 +: 15];
        step0_texelT1 = texelT1WithOffset[0 +: 15];
        step0_texelS1 = texelS1WithOffset[0 +: 15];

        step0_texelT0Valid <= !isPixelOutside(texelT0WithOffset, clampT);
        step0_texelS0Valid <= !isPixelOutside(texelS0WithOffset, clampS);
        step0_texelT1Valid <= !isPixelOutside(texelT1WithOffset, clampT);
        step0_texelS1Valid <= !isPixelOutside(texelS1WithOffset, clampS);

        addrT0 = step0_texelT0[7 +: 8] >> (8 - textureSizeHeight);
        addrT1 = step0_texelT1[7 +: 8] >> (8 - textureSizeHeight);

        texelAddr00 <= ({ 8'h0, addrT0 } << textureSizeWidth) | ({ 8'h0, step0_texelS0[7 +: 8] } >> (8 - textureSizeWidth));
        texelAddr01 <= ({ 8'h0, addrT0 } << textureSizeWidth) | ({ 8'h0, step0_texelS1[7 +: 8] } >> (8 - textureSizeWidth));
        texelAddr10 <= ({ 8'h0, addrT1 } << textureSizeWidth) | ({ 8'h0, step0_texelS0[7 +: 8] } >> (8 - textureSizeWidth));
        texelAddr11 <= ({ 8'h0, addrT1 } << textureSizeWidth) | ({ 8'h0, step0_texelS1[7 +: 8] } >> (8 - textureSizeWidth));
        step0_subCoordS <= { step0_texelS0[0 +: 15], 1'b0 } << textureSizeWidth;
        step0_subCoordT <= { step0_texelT0[0 +: 15], 1'b0 } << textureSizeHeight;
    end

    //////////////////////////////////////////////
    // STEP 1
    // Wait for data
    // Clocks: 1
    //////////////////////////////////////////////
    wire            step1_texelS0Valid;
    wire            step1_texelS1Valid;
    wire            step1_texelT0Valid;
    wire            step1_texelT1Valid;
    wire [15 : 0]   step1_subCoordS; // Q0.16
    wire [15 : 0]   step1_subCoordT; // Q0.16

    ValueDelay #( .VALUE_SIZE(16), .DELAY(MEMORY_DELAY)) 
        step1_subCoordSDelay ( .clk(aclk), .in(step0_subCoordS), .out(step1_subCoordS));

    ValueDelay #( .VALUE_SIZE(16), .DELAY(MEMORY_DELAY)) 
        step1_subCoordTDelay ( .clk(aclk), .in(step0_subCoordT), .out(step1_subCoordT));

    ValueDelay #( .VALUE_SIZE(1), .DELAY(MEMORY_DELAY)) 
        step1_texelS0ValidDelay ( .clk(aclk), .in(step0_texelS0Valid), .out(step1_texelS0Valid));

    ValueDelay #( .VALUE_SIZE(1), .DELAY(MEMORY_DELAY)) 
        step1_texelS1ValidDelay ( .clk(aclk), .in(step0_texelS1Valid), .out(step1_texelS1Valid));

    ValueDelay #( .VALUE_SIZE(1), .DELAY(MEMORY_DELAY)) 
        step1_texelT0ValidDelay ( .clk(aclk), .in(step0_texelT0Valid), .out(step1_texelT0Valid));

    ValueDelay #( .VALUE_SIZE(1), .DELAY(MEMORY_DELAY)) 
        step1_texelT1ValidDelay ( .clk(aclk), .in(step0_texelT1Valid), .out(step1_texelT1Valid));

    //////////////////////////////////////////////
    // STEP 2
    // Clamp texel quad
    // Clocks: 1
    //////////////////////////////////////////////
    reg  [15 : 0]               step2_subCoordS; // Q0.16
    reg  [15 : 0]               step2_subCoordT; // Q0.16
    reg  [PIXEL_WIDTH - 1 : 0]  step2_texel00;
    reg  [PIXEL_WIDTH - 1 : 0]  step2_texel01; 
    reg  [PIXEL_WIDTH - 1 : 0]  step2_texel10; 
    reg  [PIXEL_WIDTH - 1 : 0]  step2_texel11; 

    wire            step2_texelS0Valid;
    wire            step2_texelS1Valid;
    wire            step2_texelT0Valid;
    wire            step2_texelT1Valid;

    always @(posedge aclk)
    begin : ClampTexelQuad
        reg         t00;
        reg         t01;
        reg         t10;
        reg         t11;
        reg [1 : 0] ti00;
        reg [1 : 0] ti01;
        reg [1 : 0] ti10;
        reg [1 : 0] ti11;

        t00 = step1_texelS0Valid & step1_texelT0Valid;
        t01 = step1_texelS1Valid & step1_texelT0Valid;
        t10 = step1_texelS0Valid & step1_texelT1Valid;
        t11 = step1_texelS1Valid & step1_texelT1Valid;
    
        casez ({t11, t10, t01, t00})
            // Imagine a pixel quad as
            // 0 1 
            // 2 3
            // Lets observe 0. Now whe following cases can exist:
            // 0 z : in all cases where 0 is valid -> use 0
            // z z 
            4'bzzz1: ti00 = 0; // Pixel of the quad is in bounds
            // shift out in x direction
            // o|1 : 0 is outside, clamp s requires to use the right side -> use 1
            // o|3 
            4'b0110: ti00 = 1; // Quad is shiftet out in x direction
            // shift out in y direction
            // o o : 0 is outside, clamp t requires to use the bottom -> use 2
            // ---
            // 2 3 
            4'b1100: ti00 = 2; // Quad is shiftet out in y direction
            // shift out on top left corner
            // o|o : 0 is outside only 3 is inside -> use 3
            // ---
            // o|3 
            4'b1000: ti00 = 3; // Quad is on the corner
            // shift out on top right corner
            // o|o : 0 is outside only 2 is inside -> use 2
            // ---
            // 2|o 
            4'b0100: ti00 = 2; // Quad is on the corner
            // shift out on bottom left corner
            // o|1 : 0 is outside only 1 is inside -> use 1
            // ---
            // o|o 
            4'b0010: ti00 = 1; // Quad is on the corner
            // There should never be a case where this default is required.
            // There should be always a valid texel in the quad and they should always clamp on multiple of 90°
            default: ti00 = 0; // Default all invalid cases
        endcase

        casez ({t11, t10, t01, t00})
            4'bzz1z: ti01 = 1; // Pixel of the quad is in bounds
            4'b0101: ti01 = 0; // Quad is shiftet out in x direction
            4'b1100: ti01 = 3; // Quad is shiftet out in y direction
            4'b1000: ti01 = 3; // Quad is on the corner
            4'b0100: ti01 = 2; // Quad is on the corner
            4'b0001: ti01 = 0; // Quad is on the corner
            default: ti01 = 1; // Default all invalid cases
        endcase

        casez ({t11, t10, t01, t00})
            4'bz1zz: ti10 = 2; // Pixel of the quad is in bounds
            4'b1010: ti10 = 3; // Quad is shiftet out in x direction
            4'b0011: ti10 = 0; // Quad is shiftet out in y direction
            4'b1000: ti10 = 3; // Quad is on the corner
            4'b0010: ti10 = 1; // Quad is on the corner
            4'b0001: ti10 = 0; // Quad is on the corner
            default: ti10 = 2; // Default all invalid cases
        endcase

        casez ({t11, t10, t01, t00})
            4'b1zzz: ti11 = 3; // Pixel of the quad is in bounds
            4'b0101: ti11 = 2; // Quad is shiftet out in x direction
            4'b0011: ti11 = 1; // Quad is shiftet out in y direction
            4'b0100: ti11 = 2; // Quad is on the corner
            4'b0010: ti11 = 1; // Quad is on the corner
            4'b0001: ti11 = 0; // Quad is on the corner
            default: ti11 = 3; // Default all invalid cases
        endcase

        // Clamp texel quad
        step2_texel00 <= texelMux(ti00, texelInput00, texelInput01, texelInput10, texelInput11);
        step2_texel01 <= texelMux(ti01, texelInput00, texelInput01, texelInput10, texelInput11);
        step2_texel10 <= texelMux(ti10, texelInput00, texelInput01, texelInput10, texelInput11);
        step2_texel11 <= texelMux(ti11, texelInput00, texelInput01, texelInput10, texelInput11);

        step2_subCoordS <= step1_subCoordS;
        step2_subCoordT <= step1_subCoordT;
    end

    //////////////////////////////////////////////
    // STEP 3
    // Output
    // Clocks: 0
    //////////////////////////////////////////////
    assign texel00 = step2_texel00;
    assign texel01 = step2_texel01;
    assign texel10 = step2_texel10;
    assign texel11 = step2_texel11;

    assign texelSubCoordS = step2_subCoordS;
    assign texelSubCoordT = step2_subCoordT;

endmodule 
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
    input  wire [ 7 : 0]                textureSizeWidth, 
    input  wire [ 7 : 0]                textureSizeHeight,
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
    
    function [15 : 0] clampTexture;
        input [31 : 0] texCoord;
        input [ 0 : 0] mode; 
        begin
            clampTexture = texCoord[0 +: 16];
            if (mode == CLAMP_TO_EDGE)
            begin
                if (texCoord[31]) // Check if it lower than 0 by only checking the sign bit
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
            1'h0,
            texSize[0],
            texSize[1],
            texSize[2],
            texSize[3],
            texSize[4],
            texSize[5],
            texSize[6],
            texSize[7],
            6'h0
        };
    endfunction

    // See convertToZeroPointFive. It is basically the same just for 1.0
    function [31 : 0] convertToOnePointZero;
        input [7 : 0] texSize;
        convertToOnePointZero = 
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

    //////////////////////////////////////////////
    // STEP 0
    // Build RAM adresses
    // Clocks: 1
    //////////////////////////////////////////////
    reg             step0_clampS;
    reg             step0_clampT;
    reg  [15 : 0]   step0_texelS0; // Q1.15
    reg  [15 : 0]   step0_texelS1; // Q1.15
    reg  [15 : 0]   step0_texelT0; // Q1.15
    reg  [15 : 0]   step0_texelT1; // Q1.15
    reg  [15 : 0]   step0_subCoordS; // Q0.16
    reg  [15 : 0]   step0_subCoordT; // Q0.16
    
    // Build the RAM adress of the given (s, t) coordinate and also generate the address of 
    // the texel quad
    always @(posedge aclk)
    begin : TexAddrCalc
        reg [ 7 : 0] addrT0;
        reg [ 7 : 0] addrT1;
        reg [31 : 0] texelS0WithOffset;
        reg [31 : 0] texelS1WithOffset;
        reg [31 : 0] texelT0WithOffset;
        reg [31 : 0] texelT1WithOffset;

        step0_clampS <= clampS;
        step0_clampT <= clampT;

        if (enableHalfPixelOffset)
        begin
            texelS0WithOffset = texelS - convertToZeroPointFive(textureSizeWidth);
            texelS1WithOffset = texelS + convertToZeroPointFive(textureSizeWidth);
            texelT0WithOffset = texelT - convertToZeroPointFive(textureSizeHeight);
            texelT1WithOffset = texelT + convertToZeroPointFive(textureSizeHeight);
        end
        else
        begin
            texelS0WithOffset = texelS;
            texelS1WithOffset = texelS + convertToOnePointZero(textureSizeWidth);
            texelT0WithOffset = texelT;
            texelT1WithOffset = texelT + convertToOnePointZero(textureSizeHeight);
        end

        step0_texelT0 = clampTexture(texelT0WithOffset, clampT);
        step0_texelS0 = clampTexture(texelS0WithOffset, clampS);
        step0_texelT1 = clampTexture(texelT1WithOffset, clampT);
        step0_texelS1 = clampTexture(texelS1WithOffset, clampS);

        // Select Y coordinate
        case (textureSizeHeight)
            8'b00000001: // 2px
            begin
                addrT0 = {7'h0, step0_texelT0[14 +: 1]};
                addrT1 = {7'h0, step0_texelT1[14 +: 1]};
                step0_subCoordT <= {step0_texelT0[0 +: 14], 2'h0};
            end
            8'b00000010: // 4px
            begin
                addrT0 = {6'h0, step0_texelT0[13 +: 2]};
                addrT1 = {6'h0, step0_texelT1[13 +: 2]};
                step0_subCoordT <= {step0_texelT0[0 +: 13], 3'h0};
            end
            8'b00000100: // 8px
            begin
                addrT0 = {5'h0, step0_texelT0[12 +: 3]};
                addrT1 = {5'h0, step0_texelT1[12 +: 3]};
                step0_subCoordT <= {step0_texelT0[0 +: 12], 4'h0};
            end
            8'b00001000: // 16px
            begin
                addrT0 = {4'h0, step0_texelT0[11 +: 4]};
                addrT1 = {4'h0, step0_texelT1[11 +: 4]};
                step0_subCoordT <= {step0_texelT0[0 +: 11], 5'h0};
            end
            8'b00010000: // 32px
            begin  
                addrT0 = {3'h0, step0_texelT0[10 +: 5]};
                addrT1 = {3'h0, step0_texelT1[10 +: 5]};
                step0_subCoordT <= {step0_texelT0[0 +: 10], 6'h0};
            end
            8'b00100000: // 64px
            begin
                addrT0 = {2'h0, step0_texelT0[9 +: 6]};
                addrT1 = {2'h0, step0_texelT1[9 +: 6]};
                step0_subCoordT <= {step0_texelT0[0 +:  9], 7'h0};
            end
            8'b01000000: // 128px
            begin
                addrT0 = {1'h0, step0_texelT0[ 8 +: 7]};
                addrT1 = {1'h0, step0_texelT1[ 8 +: 7]};
                step0_subCoordT <= {step0_texelT0[0 +:  8], 8'h0};
            end
            8'b10000000: // 256px
            begin
                addrT0 = {      step0_texelT0[ 7 +: 8]};
                addrT1 = {      step0_texelT1[ 7 +: 8]};
                step0_subCoordT <= {step0_texelT0[0 +:  7], 9'h0};
            end
            default: // 1px
            begin   
                step0_texelT1 = step0_texelT0;
                addrT0 = 0;
                addrT1 = 0;
                step0_subCoordT <= 16'h0;
            end
        endcase

        // Select X coordinate
        case (textureSizeWidth)
            8'b00000001: // 2px
            begin
                texelAddr00 <= {7'h0, addrT0, step0_texelS0[14 +: 1]};
                texelAddr01 <= {7'h0, addrT0, step0_texelS1[14 +: 1]};
                texelAddr10 <= {7'h0, addrT1, step0_texelS0[14 +: 1]};
                texelAddr11 <= {7'h0, addrT1, step0_texelS1[14 +: 1]};
                step0_subCoordS <= {step0_texelS0[0 +: 14], 2'h0};
            end
            8'b00000010: // 4px
            begin
                texelAddr00 <= {6'h0, addrT0, step0_texelS0[13 +: 2]};
                texelAddr01 <= {6'h0, addrT0, step0_texelS1[13 +: 2]};
                texelAddr10 <= {6'h0, addrT1, step0_texelS0[13 +: 2]};
                texelAddr11 <= {6'h0, addrT1, step0_texelS1[13 +: 2]};
                step0_subCoordS <= {step0_texelS0[0 +: 13], 3'h0};
            end
            8'b00000100: // 8px
            begin
                texelAddr00 <= {5'h0, addrT0, step0_texelS0[12 +: 3]};
                texelAddr01 <= {5'h0, addrT0, step0_texelS1[12 +: 3]};
                texelAddr10 <= {5'h0, addrT1, step0_texelS0[12 +: 3]};
                texelAddr11 <= {5'h0, addrT1, step0_texelS1[12 +: 3]};
                step0_subCoordS <= {step0_texelS0[0 +: 12], 4'h0};
            end
            8'b00001000: // 16px
            begin
                texelAddr00 <= {4'h0, addrT0, step0_texelS0[11 +: 4]};
                texelAddr01 <= {4'h0, addrT0, step0_texelS1[11 +: 4]};
                texelAddr10 <= {4'h0, addrT1, step0_texelS0[11 +: 4]};
                texelAddr11 <= {4'h0, addrT1, step0_texelS1[11 +: 4]};
                step0_subCoordS <= {step0_texelS0[0 +: 11], 5'h0};
            end
            8'b00010000: // 32px
            begin
                texelAddr00 <= {3'h0, addrT0, step0_texelS0[10 +: 5]};
                texelAddr01 <= {3'h0, addrT0, step0_texelS1[10 +: 5]};
                texelAddr10 <= {3'h0, addrT1, step0_texelS0[10 +: 5]};
                texelAddr11 <= {3'h0, addrT1, step0_texelS1[10 +: 5]};
                step0_subCoordS <= {step0_texelS0[0 +: 10], 6'h0};
            end
            8'b00100000: // 64px
            begin
                texelAddr00 <= {2'h0, addrT0, step0_texelS0[ 9 +: 6]};
                texelAddr01 <= {2'h0, addrT0, step0_texelS1[ 9 +: 6]};
                texelAddr10 <= {2'h0, addrT1, step0_texelS0[ 9 +: 6]};
                texelAddr11 <= {2'h0, addrT1, step0_texelS1[ 9 +: 6]};
                step0_subCoordS <= {step0_texelS0[0 +:  9], 7'h0};
            end
            8'b01000000: // 128px
            begin
                texelAddr00 <= {1'h0, addrT0, step0_texelS0[ 8 +: 7]};
                texelAddr01 <= {1'h0, addrT0, step0_texelS1[ 8 +: 7]};
                texelAddr10 <= {1'h0, addrT1, step0_texelS0[ 8 +: 7]};
                texelAddr11 <= {1'h0, addrT1, step0_texelS1[ 8 +: 7]};
                step0_subCoordS <= {step0_texelS0[0 +:  8], 8'h0};
            end
            8'b10000000: // 256px
            begin
                texelAddr00 <= {      addrT0, step0_texelS0[ 7 +: 8]};
                texelAddr01 <= {      addrT0, step0_texelS1[ 7 +: 8]};
                texelAddr10 <= {      addrT1, step0_texelS0[ 7 +: 8]};
                texelAddr11 <= {      addrT1, step0_texelS1[ 7 +: 8]};
                step0_subCoordS <= {step0_texelS0[0 +:  7], 9'h0};
            end
            default: // 1px
            begin
                step0_texelS1 = step0_texelS0;
                texelAddr00 <= {8'h0, addrT0};
                texelAddr01 <= {8'h0, addrT0};
                texelAddr10 <= {8'h0, addrT1};
                texelAddr11 <= {8'h0, addrT1};
                step0_subCoordS <= 16'h0;
            end
        endcase
    end

    //////////////////////////////////////////////
    // STEP 1
    // Wait for data
    // Clocks: 1
    //////////////////////////////////////////////
    wire            step1_clampS;
    wire            step1_clampT;
    wire [15 : 0]   step1_subCoordS; // Q0.16
    wire [15 : 0]   step1_subCoordT; // Q0.16

    // Check if we have to clamp
    // Check if the texel coordinate is smaller than texel+1. If so, we have an overflow and we have to clamp.
    // OR, since the texel coordinate is a Q1.15 number, we need a dedicated check for the integer part. Could be, 
    // that just the fraction part overflows but not the whole variable. Therefor also check for it by checking the
    // most significant bit.
    wire step1_clampSCalc = step0_clampS && ((step0_texelS0 > step0_texelS1) || (!step0_texelS0[15] && step0_texelS1[15]));
    wire step1_clampTCalc = step0_clampT && ((step0_texelT0 > step0_texelT1) || (!step0_texelT0[15] && step0_texelT1[15]));

    ValueDelay #( .VALUE_SIZE(16), .DELAY(MEMORY_DELAY)) 
        step1_subCoordSDelay ( .clk(aclk), .in(step0_subCoordS), .out(step1_subCoordS));

    ValueDelay #( .VALUE_SIZE(16), .DELAY(MEMORY_DELAY)) 
        step1_subCoordTDelay ( .clk(aclk), .in(step0_subCoordT), .out(step1_subCoordT));

    ValueDelay #( .VALUE_SIZE(1), .DELAY(MEMORY_DELAY)) 
        step1_clampSDelay ( .clk(aclk), .in(step1_clampSCalc), .out(step1_clampS));

    ValueDelay #( .VALUE_SIZE(1), .DELAY(MEMORY_DELAY)) 
        step1_clampTDelay ( .clk(aclk), .in(step1_clampTCalc), .out(step1_clampT));

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

    always @(posedge aclk)
    begin
        step2_subCoordS <= step1_subCoordS;
        step2_subCoordT <= step1_subCoordT;

        // Clamp texel quad
        step2_texel00 <= texelInput00;
        step2_texel01 <= (step1_clampS) ? texelInput00 
                                        : texelInput01;
        step2_texel10 <= (step1_clampT) ? texelInput00 
                                        : texelInput10;
        step2_texel11 <= (step1_clampS) ? (step1_clampT) ? texelInput00 
                                                         : texelInput10
                                        : (step1_clampT) ? texelInput01 
                                                         : texelInput11;
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
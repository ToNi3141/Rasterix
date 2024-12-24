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

// Calculates the lod level of a texel. It expects on additional texel
// which is incremented by x and y in screenspace
//
//  -------------------------
// | this pixel |            | Pixel used for the rendeing
//  -------------------------
// |            | this pixel | Pixel only used for mipmap calculation
//  -------------------------
//
// Pipelined: yes
// Depth: 1 cycles
module LodCalculator
(
    input  wire                         aclk,
    input  wire                         resetn,
    input  wire                         ce,

    input  wire                         confEnable,

    input  wire [ 3 : 0]                textureSizeWidth, 
    input  wire [ 3 : 0]                textureSizeHeight,

    input  wire [31 : 0]                texelS, // S16.15
    input  wire [31 : 0]                texelT, // S16.15

    input  wire [31 : 0]                texelSxy, // S16.15
    input  wire [31 : 0]                texelTxy, // S16.15

    output reg  [ 3 : 0]                lod
);
    wire [31 : 0] diffS = texelS - texelSxy;
    wire [31 : 0] diffT = texelT - texelTxy;
    wire [14 : 0] diffUnsignedS = diffS[31] ? ~diffS[0 +: 15] + 1 : diffS[0 +: 15];
    wire [14 : 0] diffUnsignedT = diffT[31] ? ~diffT[0 +: 15] + 1 : diffT[0 +: 15];
    wire [ 7 : 0] diffU = diffUnsignedS[7 +: 8] >> (8 - textureSizeWidth);
    wire [ 7 : 0] diffV = diffUnsignedT[7 +: 8] >> (8 - textureSizeHeight);
    wire [ 7 : 0] diffMax = diffV | diffU;
    always @(posedge aclk)
    if (ce) begin : Mux
        reg [ 3 : 0] lodReg;
        casez (diffMax)
            8'b00000000: lodReg = 0;
            8'b00000001: lodReg = 1;
            8'b0000001?: lodReg = 2;
            8'b000001??: lodReg = 3;
            8'b00001???: lodReg = 4;
            8'b0001????: lodReg = 5;
            8'b001?????: lodReg = 6;
            8'b01??????: lodReg = 7;
            8'b1???????: lodReg = 8;
        endcase

        if (confEnable)
        begin
            lod <= lodReg;
        end
        else
        begin
            lod <= 0;
        end
    end
endmodule
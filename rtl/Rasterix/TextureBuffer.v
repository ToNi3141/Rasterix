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

module TextureBuffer #(
    // Width of the write port
    parameter STREAM_WIDTH = 32,

    // Size in bytes in power of two
    parameter SIZE = 14,
    
    localparam SUB_PIXEL_WIDTH = 4,
    localparam PIXEL_WIDTH = SUB_PIXEL_WIDTH * 4,
    localparam SIZE_IN_WORDS = SIZE - $clog2(PIXEL_WIDTH / 8),
    localparam ADDR_WIDTH = SIZE_IN_WORDS - $clog2(STREAM_WIDTH / PIXEL_WIDTH),
    localparam ADDR_WIDTH_DIFF = SIZE_IN_WORDS - ADDR_WIDTH
)
(
    input  wire                         clk,
    input  wire                         reset,

    // Texture size
    input  wire [ 7 : 0]                textureSizeX,
    input  wire [ 7 : 0]                textureSizeY,

    // Texture Read
    output wire [PIXEL_WIDTH - 1 : 0]   texel,
    input  wire [15 : 0]                texelX,
    input  wire [15 : 0]                texelY,

    // Texture Write
    input  wire                         s_axis_tvalid,
    output reg                          s_axis_tready,
    input  wire                         s_axis_tlast,
    input  wire [STREAM_WIDTH - 1 : 0]  s_axis_tdata
);
`include "RegisterAndDescriptorDefines.vh"

    reg  [ADDR_WIDTH - 1 : 0]       memWriteAddr = 0;
    wire [STREAM_WIDTH - 1 : 0]     memReadData;
    wire [ADDR_WIDTH - 1 : 0]       memReadAddr;
    reg  [SIZE_IN_WORDS - 1 : 0]    texelIndexConf;
    reg  [SIZE_IN_WORDS - 1 : 0]    texelIndexConfDelay;

    DualPortRam #(
        .MEM_SIZE_BYTES(SIZE),
        .MEM_WIDTH(STREAM_WIDTH),
        .WRITE_STROBE_WIDTH(PIXEL_WIDTH)
        //.MEMORY_PRIMITIVE("distributed")
    ) texCache 
    (
        .clk(clk),
        .reset(reset),

        .writeData(s_axis_tdata),
        .writeCs(1),
        .write(s_axis_tvalid),
        .writeAddr(memWriteAddr),
        .writeMask({(STREAM_WIDTH / PIXEL_WIDTH){1'b1}}),

        .readData(memReadData),
        .readCs(1),
        .readAddr(memReadAddr)
    );

    assign memReadAddr = texelIndexConf[ADDR_WIDTH_DIFF +: ADDR_WIDTH];

    // Note: The memReadData is one clock cycle delayed, therefore we have to use the delayed texel index
    assign texel = memReadData[texelIndexConfDelay[0 +: ADDR_WIDTH_DIFF] * PIXEL_WIDTH +: PIXEL_WIDTH];

    always @*
    begin : TexAddrCalc
        reg [7 : 0] addrY;
        casez (textureSizeY)
            8'b???????1:
                addrY = {7'h0, texelY[14 +: 1]};
            8'b??????1?:
                addrY = {6'h0, texelY[13 +: 2]};
            8'b?????1??:
                addrY = {5'h0, texelY[12 +: 3]};
            8'b????1???:
                addrY = {4'h0, texelY[11 +: 4]};
            8'b???1????:
                addrY = {3'h0, texelY[10 +: 5]};
            8'b??1?????:
                addrY = {2'h0, texelY[ 9 +: 6]};
            8'b?1??????:
                addrY = {1'h0, texelY[ 8 +: 7]};
            8'b1???????:
                addrY = {      texelY[ 7 +: 8]};
            default:
                addrY = 0;
        endcase

        casez (textureSizeX)
            8'b???????1:
                texelIndexConf = {7'h0, addrY, texelX[14 +: 1]};
            8'b??????1?:
                texelIndexConf = {6'h0, addrY, texelX[13 +: 2]};
            8'b?????1??:
                texelIndexConf = {5'h0, addrY, texelX[12 +: 3]};
            8'b????1???:
                texelIndexConf = {4'h0, addrY, texelX[11 +: 4]};
            8'b???1????:
                texelIndexConf = {3'h0, addrY, texelX[10 +: 5]};
            8'b??1?????:
                texelIndexConf = {2'h0, addrY, texelX[ 9 +: 6]};
            8'b?1??????:
                texelIndexConf = {1'h0, addrY, texelX[ 8 +: 7]};
            8'b1???????:
                texelIndexConf = {      addrY, texelX[ 7 +: 8]};
            default:
                texelIndexConf = {8'h0, addrY};
        endcase
    end

    always @(posedge clk)
    begin
        texelIndexConfDelay <= texelIndexConf;
        if (reset)
        begin
            memWriteAddr <= 0;
            s_axis_tready <= 1;
        end
        else
        begin
            if (s_axis_tvalid)
            begin
                if (s_axis_tlast)
                begin
                    memWriteAddr <= 0;
                end
                else
                begin
                    memWriteAddr <= memWriteAddr + 1;
                end
            end
        end
    end

endmodule 
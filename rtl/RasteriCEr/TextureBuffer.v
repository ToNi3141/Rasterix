// RasteriCEr
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
    parameter STREAM_WIDTH = 16,

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

    // Texture mode
    // 4'b0001 32x32
    // 4'b0010 64x64
    // 4'b0100 128x128
    // 4'b1000 256x256 (right now not supported)
    input  wire [ 3 : 0]                mode,

    // Texture Read
    output wire [PIXEL_WIDTH - 1 : 0]   texel,
    input  wire [31 : 0]                texelIndex,

    // Texture Write
    input  wire                         s_axis_tvalid,
    output reg                          s_axis_tready,
    input  wire                         s_axis_tlast,
    input  wire [STREAM_WIDTH - 1 : 0]  s_axis_tdata
);
`ifdef UP5K
`define RAM_MODULE SinglePortRam32k
`else
`define RAM_MODULE DualPortRam
`endif
`include "RegisterAndDescriptorDefines.vh"

    reg  [ADDR_WIDTH - 1 : 0]       memWriteAddr = 0;
    wire [STREAM_WIDTH - 1 : 0]     memReadData;
    wire [ADDR_WIDTH - 1 : 0]       memReadAddr;
    reg  [SIZE_IN_WORDS - 1 : 0]    texelIndexConf;
    reg  [SIZE_IN_WORDS - 1 : 0]    texelIndexConfDelay;


    `RAM_MODULE #(
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

    generate
        if (STREAM_WIDTH == 16)
        begin
            assign memReadAddr = texelIndexConf;
            assign texel = memReadData;
        end
        else
        begin
            assign memReadAddr = texelIndexConf[ADDR_WIDTH_DIFF +: ADDR_WIDTH];

            // Note: The memReadData is one clock cycle delayed, therefore we have to use the delayed texel index
            assign texel = memReadData[texelIndexConfDelay[0 +: ADDR_WIDTH_DIFF] * PIXEL_WIDTH +: PIXEL_WIDTH];
        end
    endgenerate


    always @*
    begin
        case (mode)
            `OP_TEXTURE_STREAM_MODE_32x32: // 32x32px texture
                texelIndexConf = {4'b0, texelIndex[26 +: 5], texelIndex[10 +: 5]};
            `OP_TEXTURE_STREAM_MODE_64x64: // 64x64px texture
                texelIndexConf = {2'b0, texelIndex[25 +: 6], texelIndex[9 +: 6]};
            `OP_TEXTURE_STREAM_MODE_128x128: // 128x128px texture
                texelIndexConf = {texelIndex[24 +: 7], texelIndex[8 +: 7]}; 
            // `OP_TEXTURE_STREAM_MODE_256x256: // 256x256px texture (right now not supported)
            //     texelIndexConf = {texelIndex[23 +: 8], texelIndex[7 +: 8]}; 
            default:
                texelIndexConf = 0;
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
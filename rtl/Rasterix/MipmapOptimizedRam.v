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

`include "PixelUtil.vh"

// RAM which is optimized for the memory layout of a mipmap
// Pipelined: yes
// Depth: 1 cycle
module MipmapOptimizedRam #(
    parameter MEM_WIDTH = 16, // Memory width in bits
    parameter WRITE_STROBE_WIDTH = MEM_WIDTH, // Write strobe in bits
    parameter MEMORY_PRIMITIVE = "block", // "distribute" or "block"
    parameter ADDR_WIDTH = 8,
    localparam WRITE_MASK_SIZE = MEM_WIDTH / WRITE_STROBE_WIDTH
)
(
    input  wire                             clk,
    input  wire                             reset,

    // Write interface. When write is 0, writeDataOut can be used to read from this channel.
    input  wire [MEM_WIDTH - 1 : 0]         writeData,
    input  wire                             write,
    input  wire [ADDR_WIDTH - 1 : 0]        writeAddr,
    input  wire [WRITE_MASK_SIZE - 1 : 0]   writeMask,
    output wire [MEM_WIDTH - 1 : 0]         writeDataOut, 

    // Read interface
    output wire [MEM_WIDTH - 1 : 0]         readData,
    input  wire [ADDR_WIDTH - 1 : 0]        readAddr
);
    wire [MEM_WIDTH - 1 : 0] memWriteDataOut[0 : 1]; 
    wire [MEM_WIDTH - 1 : 0] memReadData[0 : 1];

    reg  [ADDR_WIDTH - 1 : 0] readAddrReg;
    reg  [ADDR_WIDTH - 1 : 0] writeAddrReg;

    TrueDualPortRam #(
        .ADDR_WIDTH(ADDR_WIDTH - 1),
        .MEM_WIDTH(MEM_WIDTH),
        .WRITE_STROBE_WIDTH(WRITE_STROBE_WIDTH),
        .MEMORY_PRIMITIVE(MEMORY_PRIMITIVE)
    ) ram0 (
        .clk(clk),
        .reset(reset),

        .writeData(writeData),
        .write(!writeAddr[ADDR_WIDTH - 1] && write),
        .writeAddr(writeAddr[0 +: ADDR_WIDTH - 1]),
        .writeMask(writeMask),
        .writeDataOut(memWriteDataOut[0]),

        .readData(memReadData[0]),
        .readAddr(readAddr[0 +: ADDR_WIDTH - 1])
    );

    TrueDualPortRam #(
        .ADDR_WIDTH(ADDR_WIDTH - 2),
        .MEM_WIDTH(MEM_WIDTH),
        .WRITE_STROBE_WIDTH(WRITE_STROBE_WIDTH),
        .MEMORY_PRIMITIVE(MEMORY_PRIMITIVE)
    ) ram1 (
        .clk(clk),
        .reset(reset),

        .writeData(writeData),
        .write(writeAddr[ADDR_WIDTH - 1] && write),
        .writeAddr(writeAddr[0 +: ADDR_WIDTH - 2]),
        .writeMask(writeMask),
        .writeDataOut(memWriteDataOut[1]),

        .readData(memReadData[1]),
        .readAddr(readAddr[0 +: ADDR_WIDTH - 2])
    );

    always @(posedge clk)
    begin
        readAddrReg <= readAddr;
        writeAddrReg <= writeAddr;
    end


    assign readData = (readAddrReg[ADDR_WIDTH - 1]) ? memReadData[1] : memReadData[0];
    assign writeDataOut = (writeAddrReg[ADDR_WIDTH - 1]) ? memWriteDataOut[1] : memWriteDataOut[0];

endmodule 
// RasteriCEr
// https://github.com/ToNi3141/RasteriCEr
// Copyright (c) 2021 ToNi3141

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

module FunctionInterpolator #(
    // Width of the write port
    parameter STREAM_WIDTH = 64,

    localparam FLOAT_WIDTH = 32,
    localparam INT_WIDTH = 24 // TODO: Use this parameter in the code
)
(
    input  wire                         aclk,
    input  wire                         reset,

    input  wire [FLOAT_WIDTH - 1 : 0]   x, // IEEE 754 32bit float
    output wire signed [INT_WIDTH - 1 : 0] fx,

    // LUT data
    input  wire                         s_axis_tvalid,
    output reg                          s_axis_tready,
    input  wire                         s_axis_tlast,
    input  wire [STREAM_WIDTH - 1 : 0]  s_axis_tdata
);
    localparam LUT_ENTRY_WIDTH = 64;
    localparam LUT_SIZE_BYTES = (2**8) * (LUT_ENTRY_WIDTH / 8);
    localparam LUT_SIZE_ADDR_DIFF = $clog2(STREAM_WIDTH) - $clog2(LUT_ENTRY_WIDTH);
    

    localparam FLOAT_EXP_SIZE = 8;
    localparam FLOAT_EXP_POS = 23;
    localparam FLOAT_MANTISSA_SIZE = 23;
    localparam FLOAT_MANTISSA_POS = 0;

    reg  [32 - 1 : 0]                       lutLowerBound = 0;
    reg  [32 - 1 : 0]                       lutUpperBound = 0;
    reg                                                 writeLutBounds = 1;

    wire [FLOAT_EXP_SIZE - 1 : 0]                       floatExp = x[FLOAT_EXP_POS +: FLOAT_EXP_SIZE] - 127; // Todo: Check how we can handle this bias and reduce the lut memory (more than the half of the lut entries are not required)
    wire [FLOAT_MANTISSA_SIZE - 1 : 0]                  floatMantissa = x[FLOAT_MANTISSA_POS +: FLOAT_MANTISSA_SIZE];

    reg  [FLOAT_EXP_SIZE - LUT_SIZE_ADDR_DIFF - 1 : 0]  memWriteAddr = 0;
    wire [STREAM_WIDTH - 1 : 0]                         memReadData;
    wire [FLOAT_EXP_SIZE - LUT_SIZE_ADDR_DIFF - 1 : 0]  memReadAddr = floatExp[LUT_SIZE_ADDR_DIFF +: FLOAT_EXP_SIZE - LUT_SIZE_ADDR_DIFF];
    wire [LUT_ENTRY_WIDTH - 1 : 0]                      lutEntry = (LUT_SIZE_ADDR_DIFF > 0)
                                                                 ? memReadData[LUT_ENTRY_WIDTH * floatExp[0 +: (LUT_SIZE_ADDR_DIFF == 0) ? 1 // Resolves compile errors...
                                                                                                                                         : LUT_SIZE_ADDR_DIFF] +: LUT_ENTRY_WIDTH]
                                                                 : memReadData[0 +: LUT_ENTRY_WIDTH];


    wire signed [31 : 0]                                m = lutEntry[0  +: 32];
    wire signed [31 : 0]                                b = lutEntry[32 +: 32];
    wire signed [23 : 0]                                m24 = m[8 +: 24];
    wire signed [23 : 0]                                b24 = b[8 +: 24];
    wire [23 : 0]                                       x24 = {floatMantissa, 1'b0};
    wire [ 7 : 0]                                       x8 = x24[16 +: 8];
    wire signed [(24 + 8) - 1 : 0]                      mx32 = m24 * x8;
    wire signed [23 : 0]                                mxb24 = mx32[0 +: 24] + b24;
    assign fx = (x <= lutLowerBound) ? {1'b0, 1'b1, 22'h0}  
                                     : (x >= lutUpperBound) ? 0
                                                            : mxb24[24 - INT_WIDTH +: INT_WIDTH];
    

    DualPortRam #(
        .MEM_SIZE_BYTES($clog2(LUT_SIZE_BYTES)),
        .MEM_WIDTH(STREAM_WIDTH),
        .WRITE_STROBE_WIDTH(STREAM_WIDTH),
        .MEMORY_PRIMITIVE("distributed")
    ) lut 
    (
        .clk(aclk),
        .reset(reset),

        .writeData(s_axis_tdata),
        .writeCs(1),
        .write(s_axis_tvalid && !writeLutBounds),
        .writeAddr(memWriteAddr),
        .writeMask(1'b1),

        .readData(memReadData),
        .readCs(1),
        .readAddr(memReadAddr)
    );


    always @(posedge aclk)
    begin
        if (reset)
        begin
            memWriteAddr <= 0;
            writeLutBounds <= 1;
            s_axis_tready <= 1;
        end
        else
        begin
            if (s_axis_tvalid)
            begin
                if (s_axis_tlast)
                begin
                    memWriteAddr <= 0;
                    writeLutBounds <= 1;
                end
                else
                begin
                    if (writeLutBounds)
                    begin
                        lutLowerBound <= s_axis_tdata[0 +: 32];
                        lutUpperBound <= s_axis_tdata[32 +: 32];
                        writeLutBounds <= 0;
                    end
                    else 
                    begin
                        memWriteAddr <= memWriteAddr + 1;
                    end
                end
            end
        end
    end
endmodule

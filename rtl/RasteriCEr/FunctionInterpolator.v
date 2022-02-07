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

// The function interpolater will interpolate a mathematical function based on a LUT
// The used interpolation equation is: f(x) = m*x + b
// where:
// x: User input, mantissa is of this value is used as multiplicant for m
// m: slope stored in the LUT and is accessed via the x's exponent - 127
// b: offset stored in the LUT and is accessed via the x's exponent - 127
// Since the LUT is accessed via the exponent of x, the entries in the LUT have a exponential
// distribution. Means entry 0 is for the float value 1.0, entry 1 for 2.0, entry 2 for 4.0, entry 3 for 8.0 ...
// There is a limitation with values below 1.0. The limitation comes from the LUT access based 
// on the exponent. The function interpolator substracts 127 from the exponent. This means that
// 1.0 is the smallest number. This is not really necessary, but in the usual use case,
// this function interpolator is used for values over one. To also cover values below one,
// we would require 128 additional LUT entries which I want to save. As work around for values
// smaller one, one can set the lower bound to 1.0. For values lower 1.0 the function 
// interpolator will output fx = 1.0.
// The function interpolator has 32 entries to cover input values from 1.0 to 4294967296.0
// Dataformat
// Beat 0 are the LUT bounds
// lower bound = tdata[0 : 32] as float. If x is lower, fx will be 1.0
// upper bound = tdata[32 : 32] as float. If x is higher, fx will be 0.0
// Beat 1 .. 33 
// m = tdata[0  +: 32] as S1.30
// b = tdata[32  +: 32] as S1.30
// In case STREAM_WIDTH is bigger than 64 bit, bits [n : 64] are ignored
module FunctionInterpolator #(
    // Width of the write port
    parameter STREAM_WIDTH = 64,
    localparam FLOAT_WIDTH = 32,
    localparam INT_WIDTH = 24
)
(
    input  wire                         aclk,
    input  wire                         reset,

    input  wire [FLOAT_WIDTH - 1 : 0]   x, // IEEE 754 32bit float
    output wire signed [INT_WIDTH - 1 : 0] fx, // 24bit signed fixpoint S1.22 number

    // LUT data
    input  wire                         s_axis_tvalid,
    output reg                          s_axis_tready,
    input  wire                         s_axis_tlast,
    input  wire [STREAM_WIDTH - 1 : 0]  s_axis_tdata
);
    localparam LUT_ENTRIES = 32;

    localparam LUT_ENTRY_WIDTH = 64;

    localparam FLOAT_EXP_SIZE = 8;
    localparam FLOAT_EXP_POS = 23;
    localparam FLOAT_MANTISSA_SIZE = 23;
    localparam FLOAT_MANTISSA_POS = 0;

    // LUT bounds
    reg  [31 : 0]                                       lutLowerBound = 0;
    reg  [31 : 0]                                       lutUpperBound = 0;
    reg  [LUT_ENTRY_WIDTH - 1 : 0]                      lut[0 : LUT_ENTRIES - 1];

    // LUT memory access
    reg                                                 writeLutBounds = 1;
    reg  [$clog2(LUT_ENTRIES) - 1 : 0]                  memWriteAddr = 0;

    // Float unpacking
    wire [FLOAT_EXP_SIZE - 1 : 0]                       floatExp = x[FLOAT_EXP_POS +: FLOAT_EXP_SIZE] - 127; // Todo: Check how we can handle this bias and reduce the lut memory (more than the half of the lut entries are not required)
    wire [FLOAT_MANTISSA_SIZE - 1 : 0]                  floatMantissa = x[FLOAT_MANTISSA_POS +: FLOAT_MANTISSA_SIZE];

    // Memory access
    wire [LUT_ENTRY_WIDTH - 1 : 0]                      lutEntry = lut[floatExp[0 +: $clog2(LUT_ENTRIES)]];

    // Interpolation
    wire signed [31 : 0]                                m = lutEntry[0  +: 32];
    wire signed [31 : 0]                                b = lutEntry[32 +: 32];
    wire signed [23 : 0]                                m24 = m[8 +: 24];
    wire signed [23 : 0]                                b24 = b[8 +: 24];
    wire [23 : 0]                                       x24 = {floatMantissa, 1'b0};
    wire [ 7 : 0]                                       x8 = x24[16 +: 8];
    wire signed [(24 + 8) - 1 : 0]                      mx32 = m24 * x8;
    wire signed [23 : 0]                                mxb24 = mx32[0 +: 24] + b24;
    assign fx = (x <= lutLowerBound) ? {1'b0, 1'b1, {(INT_WIDTH - 2){1'b0}}}  
                                     : (x >= lutUpperBound) ? 0
                                                            : mxb24[24 - INT_WIDTH +: INT_WIDTH];

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
                        lut[memWriteAddr] <= s_axis_tdata[0 +: LUT_ENTRY_WIDTH];
                        memWriteAddr <= memWriteAddr + 1;
                    end
                end
            end
        end
    end
endmodule

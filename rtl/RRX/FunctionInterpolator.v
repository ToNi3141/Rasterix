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

// The function interpolator will interpolate a mathematical function based on a LUT
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
// Beat 0 Lower bound
// tdata as float. If x is lower, fx will be 1.0
// Beat 1 Upper bound
// tdata as float. If x is higher, fx will be 0.0
// Beat 2 .. 66 
// Even beat: m = tdata as S1.30
// Odd beat b = tdata as S1.30
//
// This module is pipelined. It requires 4 clock cycles until it outputs the calculated value.
module FunctionInterpolator #(
    localparam LUT_ENTRY_FIELD_WIDTH = 32,
    localparam FLOAT_WIDTH = LUT_ENTRY_FIELD_WIDTH,
    localparam INT_WIDTH = 24
)
(
    input  wire                         aclk,
    input  wire                         resetn,
    input  wire                         ce,

    input  wire [FLOAT_WIDTH - 1 : 0]   x, // IEEE 754 32bit float
    output reg  signed [INT_WIDTH - 1 : 0] fx, // 24bit signed fixpoint S1.22 number

    // LUT data
    input  wire                         s_axis_tvalid,
    output reg                          s_axis_tready,
    input  wire                         s_axis_tlast,
    input  wire [31 : 0]                s_axis_tdata
);
    localparam LUT_INTERPOLATION_STEPS = 8; // Defines the steps between two LUT entries. The range between x and x + 1 will be divided by pow(2, LUT_INTERPOLATION_STEPS) 
    localparam LUT_ENTRIES = 32;

    localparam FLOAT_EXP_SIZE = 8;
    localparam FLOAT_EXP_POS = 23;
    localparam FLOAT_MANTISSA_SIZE = 23;
    localparam FLOAT_MANTISSA_POS = 0;
    localparam FLOAT_EXP_BIAS = 127;

    localparam STATE_WRITE_LOWER_BOUND = 0;
    localparam STATE_WRITE_UPPER_BOUND = 1;
    localparam STATE_WRITE_LUT_M = 2;
    localparam STATE_WRITE_LUT_B = 3;

    // LUT bounds
    reg  [FLOAT_WIDTH - 1 : 0]  lutLowerBound = 0;
    reg  [FLOAT_WIDTH - 1 : 0]  lutUpperBound = 0;
    reg  [INT_WIDTH - 1 : 0]    lutM[0 : LUT_ENTRIES - 1];
    reg  [INT_WIDTH - 1 : 0]    lutB[0 : LUT_ENTRIES - 1];

    // LUT writer
    always @(posedge aclk)
    begin : LutWriter
        // LUT memory access
        reg  [1 : 0]                        writeState;
        reg  [$clog2(LUT_ENTRIES) - 1 : 0]  memWriteAddr;

        if (!resetn)
        begin
            memWriteAddr <= 0;
            writeState <= STATE_WRITE_LOWER_BOUND;
            s_axis_tready <= 1;
        end
        else
        begin
            if (s_axis_tvalid)
            begin
                case (writeState)
                STATE_WRITE_LOWER_BOUND:
                begin
                    lutLowerBound <= s_axis_tdata[0 +: FLOAT_WIDTH];
                    writeState <= STATE_WRITE_UPPER_BOUND;
                end
                STATE_WRITE_UPPER_BOUND:
                begin
                    lutUpperBound <= s_axis_tdata[0 +: FLOAT_WIDTH];
                    writeState <= STATE_WRITE_LUT_M;
                end
                STATE_WRITE_LUT_M:
                begin
                    lutM[memWriteAddr] <= s_axis_tdata[LUT_ENTRY_FIELD_WIDTH - INT_WIDTH +: INT_WIDTH];
                    writeState <= STATE_WRITE_LUT_B;
                end
                STATE_WRITE_LUT_B:
                begin
                    lutB[memWriteAddr] <= s_axis_tdata[LUT_ENTRY_FIELD_WIDTH - INT_WIDTH +: INT_WIDTH];
                    memWriteAddr <= memWriteAddr + 1;
                    writeState <= STATE_WRITE_LUT_M;
                end
                endcase
            end
            if (s_axis_tlast)
            begin
                memWriteAddr <= 0;
                writeState <= STATE_WRITE_LOWER_BOUND;
            end
        end
    end

    // Interpolation
    always @(posedge aclk)
    if (ce) begin : Interpolation
        // x
        reg [FLOAT_WIDTH - 1 : 0]   xVal[0 : 3];

        // Float unpacking
        reg  [FLOAT_EXP_SIZE - 1 : 0]       floatExp;
        reg  [FLOAT_MANTISSA_SIZE - 1 : 0]  floatMantissa[0 : 3];

        // Interpolation
        reg  signed [INT_WIDTH - 1 : 0]                             m;
        reg  signed [INT_WIDTH - 1 : 0]                             b;
        reg  [INT_WIDTH - 1 : 0]                                    xi;
        reg  [LUT_INTERPOLATION_STEPS - 1 : 0]                      xs;
        reg  signed [(INT_WIDTH + LUT_INTERPOLATION_STEPS) - 1 : 0] mx;
        reg  signed [INT_WIDTH - 1 : 0]                             mxb;

        // Float unpacking and rebias
        // Rebiasing is done, because we care usually about the integer part. Half of the values from a float lies between 0..1.
        // so we are cutting this off
        floatExp <= x[FLOAT_EXP_POS +: FLOAT_EXP_SIZE] - FLOAT_EXP_BIAS;
        floatMantissa[0] <= x[FLOAT_MANTISSA_POS +: FLOAT_MANTISSA_SIZE];

        xVal[0] <= x;

        // LUT access
        m <= lutM[floatExp[0 +: $clog2(LUT_ENTRIES)]];
        b <= lutB[floatExp[0 +: $clog2(LUT_ENTRIES)]];

        xVal[1] <= xVal[0];
        floatMantissa[1] <= floatMantissa[0];

        // Calculate (interpolation)
        xi = {floatMantissa[1][FLOAT_MANTISSA_SIZE - (INT_WIDTH - 1) +: INT_WIDTH - 1], 1'b0};
        xs = xi[INT_WIDTH - LUT_INTERPOLATION_STEPS +: LUT_INTERPOLATION_STEPS];
        mx = m * xs;
        mxb <= mx[0 +: INT_WIDTH] + b;

        xVal[2] <= xVal[1];
        floatMantissa[2] <= floatMantissa[1];

        // Clamp to bounds
        if (xVal[2] <= lutLowerBound) 
        begin
            fx <= {1'b0, 1'b1, {(INT_WIDTH - 2){1'b0}}};
        end
        else if (xVal[2] >= lutUpperBound)
        begin
            fx <= 0;
        end
        else
        begin
            fx <= mxb;
        end
    end
endmodule

// RasterIX
// https://github.com/ToNi3141/RasterIX
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

// This module implements the following OPs
// OP:
//  KEEP = 0
//  ZERO = 1
//  REPLACE = 2
//  INCR = 3
//  INCR_WRAP = 4
//  DECR = 5
//  DECR_WRAP = 6 
//  INVERT = 7
// Pipelined: n/a
// Depth: 1 cycle
module StencilOp 
#(
    parameter STENCIL_WIDTH = 4
)
(
    input  wire                         aclk,
    input  wire                         resetn,
    input  wire                         ce,
    
    input  wire [ 2 : 0]                opZFail,
    input  wire [ 2 : 0]                opZPass,
    input  wire [ 2 : 0]                opFail,
    input  wire [STENCIL_WIDTH - 1 : 0] refVal,
    input  wire                         enable,

    input  wire                         zTest,
    input  wire                         sTest,
    input  wire [STENCIL_WIDTH - 1 : 0] sValIn,

    output reg  [STENCIL_WIDTH - 1 : 0] sValOut,
    output reg                          write
);
`include "RegisterAndDescriptorDefines.vh"
    localparam MAX_VAL = { STENCIL_WIDTH {1'b1} };

    always @(posedge aclk)
    if (ce) begin : TestFunc
        reg [2 : 0] op;
        if (!sTest)
        begin
            op = opFail;
        end 
        else if (!zTest)
        begin
            op = opZFail;
        end
        else
        begin
            op = opZPass;
        end

        case (op)
            STENCIL_OP_KEEP:
                sValOut <= sValIn;
            STENCIL_OP_ZERO:
                sValOut <= 0;
            STENCIL_OP_REPLACE:
                sValOut <= refVal;
            STENCIL_OP_INCR:
                if (sValIn < MAX_VAL)
                    sValOut <= sValIn + 1;
                else
                    sValOut <= sValIn;
            STENCIL_OP_INCR_WRAP:
                sValOut <= sValIn + 1;
            STENCIL_OP_DECR:
                if (sValIn > 0)
                    sValOut <= sValIn - 1;
                else
                    sValOut <= sValIn;
            STENCIL_OP_DECR_WRAP:
                sValOut <= sValIn - 1;
            STENCIL_OP_INVERT:
                sValOut <= ~sValIn;
        endcase

        write <= enable;
    end
endmodule
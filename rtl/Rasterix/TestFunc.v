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

// This module implements various test functions, used for the alpha and depth test.
// func:
//  ALWAYS = 0
//  NEVER = 1
//  LESS = 2
//  EQUAL = 3
//  LEQUAL = 4
//  GREATER = 5
//  NOTEQUAL = 6
//  GEQUAL = 7
// Pipelined: n/a
// Depth: 1 cycle
module TestFunc 
#(
    parameter SUB_PIXEL_WIDTH = 8
)
(
    input  wire                             aclk,
    input  wire                             resetn,
    
    input  wire [ 2 : 0]                    func,
    input  wire [SUB_PIXEL_WIDTH - 1 : 0]   refVal,
    input  wire                             confEnable,

    input  wire [SUB_PIXEL_WIDTH - 1 : 0]   val,

    output reg                              success
);
`include "RegisterAndDescriptorDefines.vh"

    always @(posedge aclk)
    begin : TestFunc
        reg less;
        reg greater;
        reg equal;
        reg check;
        less =    val <  refVal;
        greater = val >  refVal;
        equal =   val == refVal;
        case (func)
            ALWAYS:
                check = 1;
            NEVER:
                check = 0;
            LESS:
                check = less;
            EQUAL:
                check = equal;
            LEQUAL:
                check = less | equal;
            GREATER:
                check = greater;
            NOTEQUAL:
                check = !equal;
            GEQUAL:
                check = greater | equal;
            default: 
                check = 1;
        endcase

        success <= !confEnable || check;
    end

endmodule
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

module TestFunc 
#(
    parameter SUB_PIXEL_WIDTH = 8
)
(
    input  wire                             aclk,
    input  wire                             resetn,
    
    input  wire [ 2 : 0]                    func,
    input  wire [SUB_PIXEL_WIDTH - 1 : 0]   refVal,

    input  wire [SUB_PIXEL_WIDTH - 1 : 0]   val,

    output reg                              success
);
`include "RegisterAndDescriptorDefines.vh"

    always @(posedge aclk)
    begin : TestFunc
        reg less;
        reg greater;
        reg equal;
        less =    val <  refVal;
        greater = val >  refVal;
        equal =   val == refVal;
        case (func)
            ALWAYS:
                success <= 1;
            NEVER:
                success <= 0;
            LESS:
                success <= less;
            EQUAL:
                success <= equal;
            LEQUAL:
                success <= less | equal;
            GREATER:
                success <= greater;
            NOTEQUAL:
                success <= !equal;
            GEQUAL:
                success <= greater | equal;
            default: 
                success <= 1;
        endcase
    end

endmodule
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

module FramebufferWriterStrobeGen #(
    parameter STRB_WIDTH = 16,
    parameter MASK_WIDTH = 4,
    localparam INDEX_COUNT = STRB_WIDTH / MASK_WIDTH,
    localparam INDEX_WIDTH = $clog2(INDEX_COUNT)
) (
    input  wire [MASK_WIDTH - 1 : 0]    mask,
    input  wire [INDEX_WIDTH - 1 : 0]   val,
    output reg  [STRB_WIDTH - 1 : 0]    strobe
);
    generate
        genvar i;
        for (i = 0; i < INDEX_COUNT; i = i + 1)
        always_latch begin
            if (val == i)
            begin
                strobe = { 
                    { STRB_WIDTH - ((i + 1) * MASK_WIDTH) { 1'b0 } }, 
                    mask, 
                    { i * MASK_WIDTH { 1'b0 } } 
                };
            end
        end
    endgenerate
endmodule
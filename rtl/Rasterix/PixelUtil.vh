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

`ifndef PIXEL_UTIL_VH
`define PIXEL_UTIL_VH

module PixelUtil #(
    parameter SUB_PIXEL_WIDTH = 4,
    parameter CONV_SUB_PIXEL_WIDTH = 8,
    parameter NUMBER_OF_SUB_PIXELS = 4
) ();
    localparam PIXEL_WIDTH = SUB_PIXEL_WIDTH * NUMBER_OF_SUB_PIXELS;
    localparam CONV_PIXEL_WIDTH = CONV_SUB_PIXEL_WIDTH * NUMBER_OF_SUB_PIXELS;
    localparam DIFF_SUB_PIXEL_WIDTH = CONV_SUB_PIXEL_WIDTH - SUB_PIXEL_WIDTH;

    function [SUB_PIXEL_WIDTH - 1 : 0] ClampSubPixel;
        input [SUB_PIXEL_WIDTH * 2 : 0] subpixel;
        ClampSubPixel = (subpixel[SUB_PIXEL_WIDTH * 2]) ? {SUB_PIXEL_WIDTH{1'b1}} 
                                                        : subpixel[(SUB_PIXEL_WIDTH * 2) - 1 : SUB_PIXEL_WIDTH];
    endfunction

    function [CONV_PIXEL_WIDTH - 1 : 0] Expand;
        input [PIXEL_WIDTH - 1 : 0] pixel;
        integer i;
        if (DIFF_SUB_PIXEL_WIDTH == 0)
        begin
            assign Expand = pixel; 
        end
        else
        begin
            for (i = 0; i < NUMBER_OF_SUB_PIXELS; i = i + 1)
            begin
                assign Expand[i * CONV_SUB_PIXEL_WIDTH +: CONV_SUB_PIXEL_WIDTH] = { 
                    pixel[i * SUB_PIXEL_WIDTH +: SUB_PIXEL_WIDTH], 
                    // Mirror the subpixel in the reminder.
                    // Assume, expanded sub pixel width is 6 bit and sub pixel width is 4 bit
                    // Then the expanded sub pixel width will ec[5 : 0] = { c[3 : 0], c[3 : 2] }
                    pixel[(i * SUB_PIXEL_WIDTH) + (SUB_PIXEL_WIDTH - DIFF_SUB_PIXEL_WIDTH) +: DIFF_SUB_PIXEL_WIDTH]
                };
            end
        end
    endfunction

    function [PIXEL_WIDTH - 1 : 0] Reduce;
        input [CONV_PIXEL_WIDTH - 1 : 0] pixel;
        integer i;
        if (DIFF_SUB_PIXEL_WIDTH == 0)
        begin
            assign Reduce = pixel; 
        end
        else
        begin
            for (i = 0; i < NUMBER_OF_SUB_PIXELS; i = i + 1)
            begin
                assign Reduce[i * SUB_PIXEL_WIDTH +: SUB_PIXEL_WIDTH] = { 
                    pixel[(CONV_SUB_PIXEL_WIDTH * i) + DIFF_SUB_PIXEL_WIDTH +: SUB_PIXEL_WIDTH] 
                };
            end
        end
    endfunction

endmodule 

`endif // PIXEL_UTIL_VH
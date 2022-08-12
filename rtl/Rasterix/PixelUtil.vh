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

`define Saturate(FuncName, SubPixelWidth) \
    function [SubPixelWidth - 1 : 0] FuncName; \
        input [SubPixelWidth * 2 : 0] subpixel; \
        FuncName = (subpixel[SubPixelWidth * 2])    ? {SubPixelWidth{1'b1}} \
                                                    : subpixel[(SubPixelWidth * 2) - 1 : SubPixelWidth]; \
    endfunction

// Gets an signed integer S1.X and clamps and satureates it to a S0.Y number.
`define ReduceAndSaturateSigned(FuncName, SubPixelWidthIn, SubPixelWidthOut) \
    function [SubPixelWidthOut - 1 : 0] FuncName; \
        input [SubPixelWidthIn - 1 : 0] subpixel; \
        localparam Diff = SubPixelWidthIn - SubPixelWidthOut; \
        if (subpixel[SubPixelWidthIn - 1]) // check sign \
        begin \
            // In the negative case, clamp to 'b10000... \
            FuncName = (&subpixel[SubPixelWidthOut - 1 +: Diff])    ? { subpixel[SubPixelWidthIn - 1], subpixel[0 +: SubPixelWidthOut - 1] } \
                                                                    : { subpixel[SubPixelWidthIn - 1], { (SubPixelWidthOut - 1){1'b0} } }; \
        end \
        else \
        begin \
            // In the positive case, clamp to 'b01111... \
            FuncName = (|subpixel[SubPixelWidthOut - 1 +: Diff])    ? { subpixel[SubPixelWidthIn - 1], { (SubPixelWidthOut - 1){1'b1} } } \
                                                                    : { subpixel[SubPixelWidthIn - 1], subpixel[0 +: SubPixelWidthOut - 1] }; \
        end \
    endfunction


// Expands the size of a signed integer.
`define ExpandSigned(FuncName, SubPixelWidthIn, SubPixelWidthOut) \
    function [SubPixelWidthOut - 1 : 0] FuncName; \
        input [SubPixelWidthIn - 1 : 0] subpixel; \
        FuncName = { { (SubPixelWidthOut - SubPixelWidthIn){subpixel[SubPixelWidthIn - 1]} }, subpixel}; \
    endfunction

// Casts a signed integer to an unsigned by removing the sign.
// In case the integer is negative, it clamps it to zero.
`define SaturateCastSignedToUnsigned(FuncName, SubPixelWidthSigned) \
    function [SubPixelWidthSigned - 2 : 0] FuncName; // with removed sign \
        input [SubPixelWidthSigned - 1 : 0] subpixel; \
        if (subpixel[SubPixelWidthSigned - 1]) // check sign \
        begin \
            // in case the value is negative, clamp to zero \
            FuncName = 0; \
        end \
        else \
        begin \
            // In the positive case, just remove the sign \
            FuncName = subpixel[0 +: SubPixelWidthSigned - 1]; \
        end \
    endfunction

`define Expand(FuncName, SubPixelWidth, ConvSubPixelWidth, NumberOfSubPixels) \
    function [(ConvSubPixelWidth * NumberOfSubPixels) - 1 : 0] FuncName; \
        input [(SubPixelWidth * NumberOfSubPixels) - 1 : 0] pixel; \
        localparam PIXEL_WIDTH = SubPixelWidth * NumberOfSubPixels; \
        localparam DIFF_SUB_PIXEL_WIDTH = ConvSubPixelWidth - SubPixelWidth; \
        integer i; \
        if (SubPixelWidth == ConvSubPixelWidth) \
        begin \
            FuncName[0 +: PIXEL_WIDTH] = pixel[0 +: PIXEL_WIDTH];  \
        end \
        else \
        begin \
            for (i = 0; i < NumberOfSubPixels; i = i + 1) \
            begin \
                FuncName[i * ConvSubPixelWidth +: ConvSubPixelWidth] = {  \
                    pixel[i * SubPixelWidth +: SubPixelWidth],  \
                    // Mirror the subpixel in the reminder. \
                    // Assume, expanded sub pixel width is 6 bit and sub pixel width is 4 bit \
                    // Then the expanded sub pixel width will ec[5 : 0] = { c[3 : 0], c[3 : 2] } \
                    pixel[(i * SubPixelWidth) + (SubPixelWidth - DIFF_SUB_PIXEL_WIDTH) +: DIFF_SUB_PIXEL_WIDTH] \
                }; \
            end \
        end \
    endfunction

`define Reduce(FuncName, SubPixelWidth, ConvSubPixelWidth, NumberOfSubPixels) \
    function [(SubPixelWidth * NumberOfSubPixels) - 1 : 0] FuncName; \
        input [(ConvSubPixelWidth * NumberOfSubPixels) - 1 : 0] pixel; \
        localparam PIXEL_WIDTH = SubPixelWidth * NumberOfSubPixels; \
        localparam DIFF_SUB_PIXEL_WIDTH = ConvSubPixelWidth - SubPixelWidth; \
        integer i; \
        if (SubPixelWidth == ConvSubPixelWidth) \
        begin \
            FuncName[0 +: PIXEL_WIDTH] = pixel[0 +: PIXEL_WIDTH];  \
        end \
        else \
        begin \
            for (i = 0; i < NumberOfSubPixels; i = i + 1) \
            begin \
                FuncName[i * SubPixelWidth +: SubPixelWidth] = {  \
                    pixel[(ConvSubPixelWidth * i) + DIFF_SUB_PIXEL_WIDTH +: SubPixelWidth]  \
                }; \
            end \
        end \
    endfunction

`endif // PIXEL_UTIL_VH
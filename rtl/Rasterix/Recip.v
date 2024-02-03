// Rasterix
// https://github.com/ToNi3141/Rasterix
// Copyright (c) 2024 ToNi3141

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

// Calculates the reciprocal of a number
// Pipelined: yes
// Depth: 2 cycles
module Recip #(
    // Numerator in fix point representation
    parameter NUMERATOR = 32'hffffff, 
    // Width of the input and output number. Care must be taken if the NUMERATOR has more bits than NUMBER_WIDTH.
    // In this case, some of the output numbers will overflow, but numbers which can be fit (for instance smaller numbers)
    // will still have a correct interpolation
    parameter NUMBER_WIDTH = 16, 
    // Size of the lookup table. Smaller value here will safe blockram but will also degrease the precision because
    // a widther range has to be interpolated. The amout of blockram which is consumed LOCKUP_PRECISION * NUMBER_WIDTH * 2.
    parameter LOOKUP_PRECISION = 11
)
(
    input  wire                        aclk,
    input  wire [NUMBER_WIDTH - 1 : 0] x, // Unsigned input
    output reg  [NUMBER_WIDTH - 1 : 0] y  // Unsigned output
);
    localparam INTERPOLATION_WIDTH = NUMBER_WIDTH - LOOKUP_PRECISION;
    localparam LOOKUP_SIZE = 2 ** LOOKUP_PRECISION;
    // Lookup tables for the interpolation
    reg [NUMBER_WIDTH - 1 : 0]          lutB [0 : LOOKUP_SIZE - 1];
    reg [NUMBER_WIDTH - 1 : 0]          lutM [0 : LOOKUP_SIZE - 1];

    wire [NUMBER_WIDTH - 1 : 0]         mulA = {{LOOKUP_PRECISION{1'b0}}, x[0 +: INTERPOLATION_WIDTH]};
    wire [NUMBER_WIDTH - 1 : 0]         mulB = lutM[x[INTERPOLATION_WIDTH +: LOOKUP_PRECISION]];
    wire [(NUMBER_WIDTH * 2) - 1 : 0]   prod;
    reg [NUMBER_WIDTH - 1 : 0]         b;
    reg [NUMBER_WIDTH - 1 : 0]         mx;

    assign prod = mulA * mulB;
    // Calculate the linear interpolated value
    always @(posedge aclk)
    begin
        b <= lutB[x[INTERPOLATION_WIDTH +: LOOKUP_PRECISION]];
        mx <= prod[INTERPOLATION_WIDTH +: NUMBER_WIDTH]; // Negate the product because of the negative slope

        y <= -mx + b; 
    end

    // Initialize the lookup tables
    integer j, nextB, currentB, deltaB;
    initial 
    begin
        // The lookup tables must contain evenly distributed samples to interpolate between these values. The interpolation width is the
        // configured NUMBER_WIDTH substracted with the LOOKUP_WIDTH (INTERPOLATION_WIDTH).
        // We have now to options how to write this loop to configure the LUT. We can increment j with INTERPOLATION_WIDTH and divide NUMERATOR by j.
        // We would easily get the result for this sample. Another way is to just increment j by one and then divide NUMERATOR by j and then divide 
        // the result by INTERPOLATION_WIDTH. This would result in the same (sometimes in better) values, the above approach, because of rounding issues. 
        // The following example will illustrate that:
        // For instance, we want for the NUMERATOR (for instance 1000) evenly distributed points for a lookup table with the size LOOKUP_SIZE. 
        // The expected lookup table will contain the following values: inf, 100, 50, 33, 25, 20, 17, 14, 13, 11. 
        // If we divde by j and increment j at each cycle by one, we would get the following table: inf, 1000, 500, 333, 250, 200, 166, 142, 125, 111.
        // This table contains values which are obviously ten times or INTERPOLATION_WIDTH times higher, than the values for the expected LUT.
        // For that reason, we divide each value by INTERPOLATION_WIDTH and we will get the expected results.
        for (j = 0; j < LOOKUP_SIZE; j = j + 1)
        begin
            if (j == 0)
            begin
                // Manually handle the division by zero case
                currentB = ~0; // Initialize the first element with the highest possible number
                nextB    = (((NUMERATOR / (j + 1))) + (1 << (INTERPOLATION_WIDTH - 1))) >> INTERPOLATION_WIDTH;
            end
            else
            begin
                currentB = (((NUMERATOR / j))       + (1 << (INTERPOLATION_WIDTH - 1))) >> INTERPOLATION_WIDTH;
                nextB    = (((NUMERATOR / (j + 1))) + (1 << (INTERPOLATION_WIDTH - 1))) >> INTERPOLATION_WIDTH;
            end
            deltaB   = currentB - nextB;
            lutB[j]  = currentB[0 +: NUMBER_WIDTH];
            lutM[j]  = deltaB[0 +: NUMBER_WIDTH];
        end

        // For Debugging
        // for (j = 0; j < LOOKUP_SIZE / 4; j = j + 1)
        // begin
        //     $display("%d: lutB[j] = %d, lutM[j] = %d", j, lutB[j], lutM[j]);
        // end
    end

endmodule

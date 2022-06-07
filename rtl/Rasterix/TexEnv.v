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

// This module calculates the texture environment 
// func:
//  DISABLE = 0
//  REPLACE = 1
//  MODULATE = 2
//  DECAL = 3
//  BLEND = 4
//  ADD = 5
// Pipelined: yes
// Depth: 2 cycles
module TexEnv 
#(
    parameter SUB_PIXEL_WIDTH = 8,

    localparam NUMBER_OF_SUB_PIXEL = 4,
    localparam PIXEL_WIDTH = SUB_PIXEL_WIDTH * NUMBER_OF_SUB_PIXEL
)
(
    input  wire                         aclk,
    input  wire                         resetn,
    
    input  wire [ 2 : 0]                func,

    input  wire [PIXEL_WIDTH - 1 : 0]   texSrcColor, // Cs 
    input  wire [PIXEL_WIDTH - 1 : 0]   primaryColor, // Cf or Cp 
    input  wire [PIXEL_WIDTH - 1 : 0]   envColor, // Cc

    output wire [PIXEL_WIDTH - 1 : 0]   color
);
`include "RegisterAndDescriptorDefines.vh"

    parameter [SUB_PIXEL_WIDTH - 1 : 0] ONE_DOT_ZERO = { SUB_PIXEL_WIDTH{1'b1} };

    reg [SUB_PIXEL_WIDTH - 1 : 0] v00;
    reg [SUB_PIXEL_WIDTH - 1 : 0] v01;
    reg [SUB_PIXEL_WIDTH - 1 : 0] v02;
    reg [SUB_PIXEL_WIDTH - 1 : 0] v03;
    reg [SUB_PIXEL_WIDTH - 1 : 0] v10;
    reg [SUB_PIXEL_WIDTH - 1 : 0] v11;
    reg [SUB_PIXEL_WIDTH - 1 : 0] v12;
    reg [SUB_PIXEL_WIDTH - 1 : 0] v13;
    reg [SUB_PIXEL_WIDTH - 1 : 0] v20;
    reg [SUB_PIXEL_WIDTH - 1 : 0] v21;
    reg [SUB_PIXEL_WIDTH - 1 : 0] v22;
    reg [SUB_PIXEL_WIDTH - 1 : 0] v23;
    reg [SUB_PIXEL_WIDTH - 1 : 0] v30;
    reg [SUB_PIXEL_WIDTH - 1 : 0] v31;
    reg [SUB_PIXEL_WIDTH - 1 : 0] v32;
    reg [SUB_PIXEL_WIDTH - 1 : 0] v33;

    always @*
    begin : SelectColor
        reg [SUB_PIXEL_WIDTH - 1 : 0] rs;
        reg [SUB_PIXEL_WIDTH - 1 : 0] gs;
        reg [SUB_PIXEL_WIDTH - 1 : 0] bs;
        reg [SUB_PIXEL_WIDTH - 1 : 0] as;
        reg [SUB_PIXEL_WIDTH - 1 : 0] rp;
        reg [SUB_PIXEL_WIDTH - 1 : 0] gp;
        reg [SUB_PIXEL_WIDTH - 1 : 0] bp;
        reg [SUB_PIXEL_WIDTH - 1 : 0] ap;
        reg [SUB_PIXEL_WIDTH - 1 : 0] rc;
        reg [SUB_PIXEL_WIDTH - 1 : 0] gc;
        reg [SUB_PIXEL_WIDTH - 1 : 0] bc;
        reg [SUB_PIXEL_WIDTH - 1 : 0] ac;

        // Texture source color (Cs)
        rs = texSrcColor[COLOR_R_POS +: SUB_PIXEL_WIDTH];
        gs = texSrcColor[COLOR_G_POS +: SUB_PIXEL_WIDTH];
        bs = texSrcColor[COLOR_B_POS +: SUB_PIXEL_WIDTH];
        as = texSrcColor[COLOR_A_POS +: SUB_PIXEL_WIDTH];

        // Input of texture unit n (or in case of texture unit 0 it is the primary color of the triangle (Cf))
        // Currently we only have one texture unit, so Cp is Cf
        rp = primaryColor[COLOR_R_POS +: SUB_PIXEL_WIDTH];
        gp = primaryColor[COLOR_G_POS +: SUB_PIXEL_WIDTH];
        bp = primaryColor[COLOR_B_POS +: SUB_PIXEL_WIDTH];
        ap = primaryColor[COLOR_A_POS +: SUB_PIXEL_WIDTH];

        // Texture environment color (Cc)
        rc = envColor[COLOR_R_POS +: SUB_PIXEL_WIDTH];
        gc = envColor[COLOR_G_POS +: SUB_PIXEL_WIDTH];
        bc = envColor[COLOR_B_POS +: SUB_PIXEL_WIDTH];
        ac = envColor[COLOR_A_POS +: SUB_PIXEL_WIDTH];
                
        case (func)
            DISABLE:
            begin
                // Disables the tex env, so just use the triangle color instread of the texture
                // (The opposite of the GL_REPLACE)

                v00 = rp;
                v01 = gp;
                v02 = bp;
                v03 = ap;

                v10 = ONE_DOT_ZERO;
                v11 = ONE_DOT_ZERO;
                v12 = ONE_DOT_ZERO;
                v13 = ONE_DOT_ZERO;

                v20 = 0;
                v21 = 0;
                v22 = 0;
                v23 = 0;

                v30 = 0;
                v31 = 0;
                v32 = 0;
                v33 = 0;
            end
            REPLACE:
            begin
                // (s * 1.0) + (0.0 * 0.0)
                // (as * 1.0) + (0.0 * 0.0)

                v00 = rs;
                v01 = gs;
                v02 = bs;
                v03 = as;

                v10 = ONE_DOT_ZERO;
                v11 = ONE_DOT_ZERO;
                v12 = ONE_DOT_ZERO;
                v13 = ONE_DOT_ZERO;

                v20 = 0;
                v21 = 0;
                v22 = 0;
                v23 = 0;

                v30 = 0;
                v31 = 0;
                v32 = 0;
                v33 = 0;
            end
            BLEND:
            begin
                // (p * (1.0 - s) + (c * s)
                // (ap * as) + (0.0 * 0.0)      
                
                v00 = rp;
                v01 = gp;
                v02 = bp;
                v03 = ap;

                v10 = ONE_DOT_ZERO - rs;
                v11 = ONE_DOT_ZERO - gs;
                v12 = ONE_DOT_ZERO - bs;
                v13 = as;

                v20 = rc;
                v21 = gc;
                v22 = bc;
                v23 = 0;

                v30 = rs;
                v31 = gs;
                v32 = bs;
                v33 = 0; 
            end 
            DECAL:
            begin
                // (p * (1.0 - as)) + (s * as)
                // (ap * 1.0) + (0.0 * 0.0)

                v00 = rp;
                v01 = gp;
                v02 = bp;
                v03 = ap;

                v10 = (ONE_DOT_ZERO - as);
                v11 = (ONE_DOT_ZERO - as);
                v12 = (ONE_DOT_ZERO - as);
                v13 = ONE_DOT_ZERO;
                
                v20 = rs;
                v21 = gs;
                v22 = bs;
                v23 = 0;

                v30 = as;
                v31 = as;
                v32 = as;
                v33 = 0;
                
            end
            MODULATE:
            begin
                // (p * s) + (0 * 0)
                // (ap * as) + (0.0 * 0.0)

                v00 = rp;
                v01 = gp;
                v02 = bp;
                v03 = ap;

                v10 = rs;
                v11 = gs;
                v12 = bs;
                v13 = as;

                v20 = 0;
                v21 = 0;
                v22 = 0;
                v23 = 0;

                v30 = 0;
                v31 = 0;
                v32 = 0;
                v33 = 0;
            end
            ADD:
            begin 
                // (p * 1.0) + (s * 1.0)
                // (ap * as) + (0.0 * 0.0)

                v00 = rp;
                v01 = gp;
                v02 = bp;
                v03 = ap;

                v10 = ONE_DOT_ZERO;
                v11 = ONE_DOT_ZERO;
                v12 = ONE_DOT_ZERO;
                v13 = as;

                v20 = rs;
                v21 = gs;
                v22 = bs;
                v23 = 0;

                v30 = ONE_DOT_ZERO;
                v31 = ONE_DOT_ZERO;
                v32 = ONE_DOT_ZERO;
                v33 = 0;
            end
            default:
            begin
                
            end 
        endcase
    end

    ColorMixer #(
        .SUB_PIXEL_WIDTH(SUB_PIXEL_WIDTH),
        .NUMBER_OF_SUB_PIXEL(NUMBER_OF_SUB_PIXEL)
    ) colorMixer (
        .aclk(aclk),
        .resetn(resetn),

        .colorA({
            v00,
            v01,
            v02,
            v03
        }),
        .colorB({
            v10,
            v11,
            v12,
            v13
        }),
        .colorC({
            v20,
            v21,
            v22,
            v23
        }),
        .colorD({
            v30,
            v31,
            v32,
            v33
        }),

        .mixedColor(color)
    );

endmodule
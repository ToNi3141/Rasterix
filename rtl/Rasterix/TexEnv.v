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
// combineRgb:
//  REPLACE = 0;
//  MODULATE = 1;
//  ADD = 2;
//  ADD_SIGNED = 3;
//  INTERPOLATE = 4;
//  SUBTRACT = 5;
//  DOT3_RGB = 6;
//  DOT3_RGBA = 7;
// combineAlpha:
//  REPLACE = 0;
//  MODULATE = 1;
//  ADD = 2;
//  ADD_SIGNED = 3;
//  INTERPOLATE = 4;
//  SUBTRACT = 5;
// srcRegRgbx
//  SRC_TEXTURE = 0;
//  SRC_CONSTANT = 1;
//  SRC_PRIMARY_COLOR = 2;
//  SRC_PREVIOUS = 3;
// srcRegAlphax
//  SRC_TEXTURE = 0;
//  SRC_CONSTANT = 1;
//  SRC_PRIMARY_COLOR = 2;
//  SRC_PREVIOUS = 3;
// operandRgbx
//  OPERAND_RGB_SRC_ALPHA = 0;
//  OPERAND_RGB_ONE_MINUS_SRC_ALPHA = 1;
//  OPERAND_RGB_SRC_COLOR = 2;
//  OPERAND_RGB_ONE_MINUS_SRC_COLOR = 3;
// operandAlphax
//  OPERAND_ALPHA_SRC_ALPHA = 0;
//  OPERAND_ALPHA_ONE_MINUS_SRC_ALPHA = 1;
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
    
    input  wire [ 2 : 0]                combineRgb,
    input  wire [ 2 : 0]                combineAlpha,
    input  wire [ 1 : 0]                srcRegRgb0,
    input  wire [ 1 : 0]                srcRegRgb1,
    input  wire [ 1 : 0]                srcRegRgb2,
    input  wire [ 1 : 0]                srcRegAlpha0,
    input  wire [ 1 : 0]                srcRegAlpha1,
    input  wire [ 1 : 0]                srcRegAlpha2,
    input  wire [ 1 : 0]                operandRgb0,
    input  wire [ 1 : 0]                operandRgb1,
    input  wire [ 1 : 0]                operandRgb2,
    input  wire [ 0 : 0]                operandAlpha0,
    input  wire [ 0 : 0]                operandAlpha1,
    input  wire [ 0 : 0]                operandAlpha2,

    input  wire [PIXEL_WIDTH - 1 : 0]   previousColor,
    input  wire [PIXEL_WIDTH - 1 : 0]   texSrcColor, // Cs 
    input  wire [PIXEL_WIDTH - 1 : 0]   primaryColor, // Cf (in case of tex unit 0) or Cp (output color of tex unit n-1) 
    input  wire [PIXEL_WIDTH - 1 : 0]   envColor, // Cc

    output wire [PIXEL_WIDTH - 1 : 0]   color
);
`include "RegisterAndDescriptorDefines.vh"
    parameter [SUB_PIXEL_WIDTH - 1 : 0] ONE_DOT_ZERO = { SUB_PIXEL_WIDTH{1'b1} };
    parameter [SUB_PIXEL_WIDTH - 1 : 0] ZERO_DOT_FIVE = { 1'b0, ONE_DOT_ZERO[0 +: SUB_PIXEL_WIDTH - 1] };

    function [SUB_PIXEL_WIDTH - 1 : 0] SelectRgbOperand;
        input [ 1 : 0]                  conf;
        input [SUB_PIXEL_WIDTH - 1 : 0] subPixel;
        input [SUB_PIXEL_WIDTH - 1 : 0] alpha;
        
        case (conf)
            OPERAND_RGB_SRC_COLOR:
            begin
                SelectRgbOperand = subPixel;
            end
            OPERAND_RGB_ONE_MINUS_SRC_COLOR:
            begin
                SelectRgbOperand = ONE_DOT_ZERO - subPixel;
            end
            OPERAND_RGB_SRC_ALPHA:
            begin
                SelectRgbOperand = alpha;
            end
            OPERAND_RGB_ONE_MINUS_SRC_ALPHA:
            begin
                SelectRgbOperand = ONE_DOT_ZERO - alpha;
            end
        endcase
    endfunction

    function [SUB_PIXEL_WIDTH - 1 : 0] SelectAlphaOperand;
        input [ 0 : 0]                  conf;
        input [SUB_PIXEL_WIDTH - 1 : 0] alpha;
        
        case (conf)
            OPERAND_ALPHA_SRC_ALPHA:
            begin
                SelectAlphaOperand = alpha;
            end
            OPERAND_ALPHA_ONE_MINUS_SRC_ALPHA:
            begin
                SelectAlphaOperand = ONE_DOT_ZERO - alpha;
            end
        endcase
    endfunction

    function [SUB_PIXEL_WIDTH - 1 : 0] SelectSrcColor;
        input [ 1 : 0]                  conf;
        input [SUB_PIXEL_WIDTH - 1 : 0] subPixelTexture;
        input [SUB_PIXEL_WIDTH - 1 : 0] subPixelConstant;
        input [SUB_PIXEL_WIDTH - 1 : 0] subPixelPrimaryColor;
        input [SUB_PIXEL_WIDTH - 1 : 0] subPixelPrevious;

        case (conf)
            SRC_TEXTURE:
            begin
                SelectSrcColor = subPixelTexture;
            end
            SRC_CONSTANT:
            begin
                SelectSrcColor = subPixelConstant;
            end
            SRC_PRIMARY_COLOR:
            begin
                SelectSrcColor = subPixelPrimaryColor;
            end
            SRC_PREVIOUS:
            begin
                SelectSrcColor = subPixelPrevious;
            end
        endcase
    endfunction
    

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
        reg [SUB_PIXEL_WIDTH - 1 : 0] rt;
        reg [SUB_PIXEL_WIDTH - 1 : 0] gt;
        reg [SUB_PIXEL_WIDTH - 1 : 0] bt;
        reg [SUB_PIXEL_WIDTH - 1 : 0] at;
        reg [SUB_PIXEL_WIDTH - 1 : 0] rc;
        reg [SUB_PIXEL_WIDTH - 1 : 0] gc;
        reg [SUB_PIXEL_WIDTH - 1 : 0] bc;
        reg [SUB_PIXEL_WIDTH - 1 : 0] ac;
        reg [SUB_PIXEL_WIDTH - 1 : 0] rpc;
        reg [SUB_PIXEL_WIDTH - 1 : 0] gpc;
        reg [SUB_PIXEL_WIDTH - 1 : 0] bpc;
        reg [SUB_PIXEL_WIDTH - 1 : 0] apc;
        reg [SUB_PIXEL_WIDTH - 1 : 0] rp;
        reg [SUB_PIXEL_WIDTH - 1 : 0] gp;
        reg [SUB_PIXEL_WIDTH - 1 : 0] bp;
        reg [SUB_PIXEL_WIDTH - 1 : 0] ap;

        reg [SUB_PIXEL_WIDTH - 1 : 0] r0;
        reg [SUB_PIXEL_WIDTH - 1 : 0] g0;
        reg [SUB_PIXEL_WIDTH - 1 : 0] b0;
        reg [SUB_PIXEL_WIDTH - 1 : 0] a0;
        reg [SUB_PIXEL_WIDTH - 1 : 0] r1;
        reg [SUB_PIXEL_WIDTH - 1 : 0] g1;
        reg [SUB_PIXEL_WIDTH - 1 : 0] b1;
        reg [SUB_PIXEL_WIDTH - 1 : 0] a1;
        reg [SUB_PIXEL_WIDTH - 1 : 0] r2;
        reg [SUB_PIXEL_WIDTH - 1 : 0] g2;
        reg [SUB_PIXEL_WIDTH - 1 : 0] b2;
        reg [SUB_PIXEL_WIDTH - 1 : 0] a2;

        rt = texSrcColor[COLOR_R_POS +: SUB_PIXEL_WIDTH];
        gt = texSrcColor[COLOR_G_POS +: SUB_PIXEL_WIDTH];
        bt = texSrcColor[COLOR_B_POS +: SUB_PIXEL_WIDTH];
        at = texSrcColor[COLOR_A_POS +: SUB_PIXEL_WIDTH];

        rc = envColor[COLOR_R_POS +: SUB_PIXEL_WIDTH];
        gc = envColor[COLOR_G_POS +: SUB_PIXEL_WIDTH];
        bc = envColor[COLOR_B_POS +: SUB_PIXEL_WIDTH];
        ac = envColor[COLOR_A_POS +: SUB_PIXEL_WIDTH];

        rpc = primaryColor[COLOR_R_POS +: SUB_PIXEL_WIDTH];
        gpc = primaryColor[COLOR_G_POS +: SUB_PIXEL_WIDTH];
        bpc = primaryColor[COLOR_B_POS +: SUB_PIXEL_WIDTH];
        apc = primaryColor[COLOR_A_POS +: SUB_PIXEL_WIDTH];

        rp = previousColor[COLOR_R_POS +: SUB_PIXEL_WIDTH];
        gp = previousColor[COLOR_G_POS +: SUB_PIXEL_WIDTH];
        bp = previousColor[COLOR_B_POS +: SUB_PIXEL_WIDTH];
        ap = previousColor[COLOR_A_POS +: SUB_PIXEL_WIDTH];

        r0 = SelectRgbOperand(operandRgb0, 
                              SelectSrcColor(srcRegRgb0, rt, rc, rpc, rp), 
                              SelectSrcColor(srcRegRgb0, at, ac, apc, ap));

        r1 = SelectRgbOperand(operandRgb1, 
                              SelectSrcColor(srcRegRgb1, rt, rc, rpc, rp), 
                              SelectSrcColor(srcRegRgb1, at, ac, apc, ap));

        r2 = SelectRgbOperand(operandRgb2, 
                              SelectSrcColor(srcRegRgb2, rt, rc, rpc, rp), 
                              SelectSrcColor(srcRegRgb2, at, ac, apc, ap));

        g0 = SelectRgbOperand(operandRgb0, 
                              SelectSrcColor(srcRegRgb0, gt, gc, gpc, gp), 
                              SelectSrcColor(srcRegRgb0, at, ac, apc, ap));

        g1 = SelectRgbOperand(operandRgb1, 
                              SelectSrcColor(srcRegRgb1, gt, gc, gpc, gp), 
                              SelectSrcColor(srcRegRgb1, at, ac, apc, ap));

        g2 = SelectRgbOperand(operandRgb2, 
                              SelectSrcColor(srcRegRgb2, gt, gc, gpc, gp), 
                              SelectSrcColor(srcRegRgb2, at, ac, apc, ap));

        b0 = SelectRgbOperand(operandRgb0, 
                              SelectSrcColor(srcRegRgb0, bt, bc, bpc, bp), 
                              SelectSrcColor(srcRegRgb0, at, ac, apc, ap));

        b1 = SelectRgbOperand(operandRgb1, 
                              SelectSrcColor(srcRegRgb1, bt, bc, bpc, bp), 
                              SelectSrcColor(srcRegRgb1, at, ac, apc, ap));

        b2 = SelectRgbOperand(operandRgb2, 
                              SelectSrcColor(srcRegRgb2, bt, bc, bpc, bp), 
                              SelectSrcColor(srcRegRgb2, at, ac, apc, ap));

        a0 = SelectAlphaOperand(operandAlpha0, 
                                SelectSrcColor(srcRegAlpha0, at, ac, apc, ap));

        a1 = SelectAlphaOperand(operandAlpha1, 
                                SelectSrcColor(srcRegAlpha1, at, ac, apc, ap));

        a2 = SelectAlphaOperand(operandAlpha2,
                                SelectSrcColor(srcRegAlpha2, at, ac, apc, ap));
                
        case (combineRgb)
            REPLACE:
            begin
                // (x0 * 1.0) + (0.0 * 0.0)

                v00 = r0;
                v01 = g0;
                v02 = b0;

                v10 = ONE_DOT_ZERO;
                v11 = ONE_DOT_ZERO;
                v12 = ONE_DOT_ZERO;

                v20 = 0;
                v21 = 0;
                v22 = 0;

                v30 = 0;
                v31 = 0;
                v32 = 0;
            end
            MODULATE:
            begin
                // (x0 * x1) + (0 * 0)

                v00 = r0;
                v01 = g0;
                v02 = b0;

                v10 = r1;
                v11 = g1;
                v12 = b1;

                v20 = 0;
                v21 = 0;
                v22 = 0;

                v30 = 0;
                v31 = 0;
                v32 = 0;
            end
            ADD:
            begin 
                // (x0 * 1.0) + (x1 * 1.0)

                v00 = r0;
                v01 = g0;
                v02 = b0;

                v10 = ONE_DOT_ZERO;
                v11 = ONE_DOT_ZERO;
                v12 = ONE_DOT_ZERO;

                v20 = r1;
                v21 = g1;
                v22 = b1;

                v30 = ONE_DOT_ZERO;
                v31 = ONE_DOT_ZERO;
                v32 = ONE_DOT_ZERO;

            end
            ADD_SIGNED:
            begin 
                // (x0 * 1.0) + ((x1 - 0.5) * 1.0)

                v00 = r0;
                v01 = g0;
                v02 = b0;

                v10 = ONE_DOT_ZERO;
                v11 = ONE_DOT_ZERO;
                v12 = ONE_DOT_ZERO;

                v20 = r1 - ZERO_DOT_FIVE;
                v21 = g1 - ZERO_DOT_FIVE;
                v22 = b1 - ZERO_DOT_FIVE;

                v30 = ONE_DOT_ZERO;
                v31 = ONE_DOT_ZERO;
                v32 = ONE_DOT_ZERO;

            end
            INTERPOLATE:
            begin
                // (x0 * x2) + (x1 * (1.0 - x2))   
                
                v00 = r0;
                v01 = g0;
                v02 = b0;

                v10 = r2;
                v11 = g2;
                v12 = b2;

                v20 = r1;
                v21 = g1;
                v22 = b1;

                v30 = ONE_DOT_ZERO - r2;
                v31 = ONE_DOT_ZERO - g2;
                v32 = ONE_DOT_ZERO - b2;
            end 
            SUBTRACT:
            begin
                // (x0 * 1.0) + (x1 * -1.0)

                v00 = r0;
                v01 = g0;
                v02 = b0;

                v10 = ONE_DOT_ZERO;
                v11 = ONE_DOT_ZERO;
                v12 = ONE_DOT_ZERO;

                v20 = r1;
                v21 = g1;
                v22 = b1;

                v30 = -ONE_DOT_ZERO;
                v31 = -ONE_DOT_ZERO;
                v32 = -ONE_DOT_ZERO;
            end
            DOT3_RGB:
            begin
                // ((x0 - 0.5) * (x1 - 0.5)) + (0 * 0)

                v00 = r0 - ZERO_DOT_FIVE;
                v01 = g0 - ZERO_DOT_FIVE;
                v02 = b0 - ZERO_DOT_FIVE;

                v10 = r1 - ZERO_DOT_FIVE;
                v11 = g1 - ZERO_DOT_FIVE;
                v12 = b1 - ZERO_DOT_FIVE;

                v20 = 0;
                v21 = 0;
                v22 = 0;

                v30 = 0;
                v31 = 0;
                v32 = 0;
            end
            DOT3_RGBA:
            begin
                // ((x0 - 0.5) * (x1 - 0.5)) + (0 * 0)

                v00 = r0 - ZERO_DOT_FIVE;
                v01 = g0 - ZERO_DOT_FIVE;
                v02 = b0 - ZERO_DOT_FIVE;

                v10 = r1 - ZERO_DOT_FIVE;
                v11 = g1 - ZERO_DOT_FIVE;
                v12 = b1 - ZERO_DOT_FIVE;

                v20 = 0;
                v21 = 0;
                v22 = 0;

                v30 = 0;
                v31 = 0;
                v32 = 0;
            end
            default:
            begin
                
            end 
        endcase

        case (combineAlpha)
            REPLACE:
            begin
                // (x0 * 1.0) + (0.0 * 0.0)

                v03 = a0;
                v13 = ONE_DOT_ZERO;
                v23 = 0;
                v33 = 0;
            end
            MODULATE:
            begin
                // (x0 * x1) + (0 * 0)

                v03 = a0;
                v13 = a1;
                v23 = 0;
                v33 = 0;
            end
            ADD:
            begin 
                // (x0 * 1.0) + (x1 * 1.0)

                v03 = a0;
                v13 = ONE_DOT_ZERO;
                v23 = a1;
                v33 = ONE_DOT_ZERO;

            end
            ADD_SIGNED:
            begin 
                // (x0 * 1.0) + ((x1 - 0.5) * 1.0)

                v03 = a0;
                v13 = ONE_DOT_ZERO;
                v23 = a1 - ZERO_DOT_FIVE;
                v33 = ONE_DOT_ZERO;

            end
            INTERPOLATE:
            begin
                // (x0 * x2) + (x1 * (1.0 - x2))   
                
                v03 = a0;
                v13 = a2;
                v23 = a1;
                v33 = ONE_DOT_ZERO - a2;
            end 
            SUBTRACT:
            begin
                // (x0 * 1.0) + (x1 * -1.0)

                v03 = a0;
                v13 = ONE_DOT_ZERO;
                v23 = a1;
                v33 = -ONE_DOT_ZERO;
            end
            default:
            begin
                
            end 
        endcase
    end

    ColorMixer #(
        .SUB_PIXEL_WIDTH(SUB_PIXEL_WIDTH)
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

        .mixedColor(color) // TODO: The summation of the color components in the DOT3 case is not yet implemented
    );

endmodule
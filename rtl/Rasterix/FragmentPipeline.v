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

// This module calculates the fragment color.
// It reads the texel from the texture memory and clamps it, calculates 
// the fog intensity based on the z value and fog equation (LUT), 
// calculates the equation for the texture environment and applies the
// fog color.
// Pipelined: yes
// Depth: 11 cycles
module FragmentPipeline
#(
    parameter CMD_STREAM_WIDTH = 64,

    parameter SUB_PIXEL_WIDTH = 8,
    localparam PIXEL_WIDTH = 4 * SUB_PIXEL_WIDTH,

    localparam FLOAT_SIZE = 32
)
(
    input  wire                         aclk,
    input  wire                         resetn,

    // Fog function LUT stream
    input  wire                         s_fog_lut_axis_tvalid,
    output wire                         s_fog_lut_axis_tready,
    input  wire                         s_fog_lut_axis_tlast,
    input  wire [CMD_STREAM_WIDTH - 1 : 0] s_fog_lut_axis_tdata,

    // Shader configurations
    input  wire [31 : 0]                confReg1,
    input  wire [31 : 0]                confReg2,
    input  wire                         confTextureClampS,
    input  wire                         confTextureClampT,
    input  wire [PIXEL_WIDTH - 1 : 0]   confTextureEnvColor,
    input  wire [PIXEL_WIDTH - 1 : 0]   confFogColor,

    // Texture access
    output reg  [15 : 0]                texelS,
    output reg  [15 : 0]                texelT,
    input  wire [PIXEL_WIDTH - 1 : 0]   texel,

    // Fragment input
    input  wire [PIXEL_WIDTH - 1 : 0]   triangleColor,
    input  wire [FLOAT_SIZE - 1 : 0]    depth,
    input  wire [31 : 0]                textureS,
    input  wire [31 : 0]                textureT,
    
    // Fragment output
    output wire [PIXEL_WIDTH - 1 : 0]   fragmentColor
);
`include "RegisterAndDescriptorDefines.vh"

    function [15 : 0] clampTexture;
        input [23 : 0] texCoord;
        input [ 0 : 0] mode; 
        begin
            clampTexture = texCoord[0 +: 16];
            if (mode == CLAMP_TO_EDGE)
            begin
                if (texCoord[23]) // Check if it lower than 0 by only checking the sign bit
                begin
                    clampTexture = 0;
                end
                else if ((texCoord >> 15) != 0) // Check if it is greater than one by checking if the integer part is unequal to zero
                begin
                    clampTexture = 16'h7fff;
                end  
            end
        end
    endfunction

    ////////////////////////////////////////////////////////////////////////////
    // STEP 0
    // Request texel from texture buffer
    // Clocks: 1 (only the request, the response requires 6 clocks, see STEP 2)
    ////////////////////////////////////////////////////////////////////////////
    reg [PIXEL_WIDTH - 1 : 0]   step0_triangleColor;
    reg [FLOAT_SIZE - 1 : 0]    step0_depth;
    always @(posedge aclk)
    begin
        texelS <= clampTexture(textureS[0 +: 24], confTextureClampS);
        texelT <= clampTexture(textureT[0 +: 24], confTextureClampT);
        step0_triangleColor <= triangleColor;
        step0_depth <= depth;
    end

    ////////////////////////////////////////////////////////////////////////////
    // STEP 1
    // Calculate the fog intensity
    // Clocks: 6 (the calculation takes only 4 clocks, but we also wait two 
    //              additional clocks for the texture)
    ////////////////////////////////////////////////////////////////////////////
    wire [15 : 0]               step1_fogIntensity;
    wire [PIXEL_WIDTH - 1 : 0]  step1_triangleColor;

    // This step requires 4 clocks, add after that a ValueDelay to append two additional clocks.
    wire [23 : 0]               step1_fogIntensityTmp;
    FunctionInterpolator #(
        .STREAM_WIDTH(CMD_STREAM_WIDTH)
    )
    convert_fog_intensity (
        .aclk(aclk), 
        .resetn(resetn), 
        .x(step0_depth),
        .fx(step1_fogIntensityTmp),
        .s_axis_tvalid(s_fog_lut_axis_tvalid), 
        .s_axis_tready(s_fog_lut_axis_tready), 
        .s_axis_tlast(s_fog_lut_axis_tlast), 
        .s_axis_tdata(s_fog_lut_axis_tdata)
    );

    ValueDelay #(.VALUE_SIZE(PIXEL_WIDTH), .DELAY(6)) 
        step1_triangleColorDelay (.clk(aclk), .in(step0_triangleColor), .out(step1_triangleColor));

    ValueDelay #(
        .VALUE_SIZE(16), 
        .DELAY(2)
    ) step1_fogIntensityDelay (
        .clk(aclk), 
        .in((step1_fogIntensityTmp[22]) ? 16'hffff : step1_fogIntensityTmp[22 - 16 +: 16]), 
        .out(step1_fogIntensity)
    );

    ////////////////////////////////////////////////////////////////////////////
    // STEP 2
    // Calculate texture environment
    // Clocks: 2
    ////////////////////////////////////////////////////////////////////////////
    wire [PIXEL_WIDTH - 1 : 0]  step2_texel;
    wire [15 : 0]               step2_fogIntensity;

    TexEnv #(
        .SUB_PIXEL_WIDTH(SUB_PIXEL_WIDTH)
    ) texEnv (
        .aclk(aclk),
        .resetn(resetn),

        .func(confReg2[REG2_TEX_ENV_FUNC_POS +: REG2_TEX_ENV_FUNC_SIZE]),

        .texSrcColor(texel),
        .primaryColor(step1_triangleColor),
        .envColor(confTextureEnvColor),

        .color(step2_texel)
    );

    ValueDelay #(.VALUE_SIZE(16), .DELAY(2)) 
        step2_fogIntensityDelay (.clk(aclk), .in(step1_fogIntensity), .out(step2_fogIntensity));

    ////////////////////////////////////////////////////////////////////////////
    // STEP 3
    // Calculate fog
    // Clocks: 2
    ////////////////////////////////////////////////////////////////////////////
    wire [PIXEL_WIDTH - 1 : 0]  step3_texel;

    Fog #(
        .SUB_PIXEL_WIDTH(SUB_PIXEL_WIDTH)
    ) fog (
        .aclk(aclk),
        .resetn(resetn),

        .intensity(step2_fogIntensity),
        .texelColor(step2_texel),
        .fogColor(confFogColor),

        .color(step3_texel)
    );

    ////////////////////////////////////////////////////////////////////////////
    // STEP 4
    // Output final texel color
    // Clocks: 0
    ////////////////////////////////////////////////////////////////////////////
    assign fragmentColor = step3_texel;

endmodule


 
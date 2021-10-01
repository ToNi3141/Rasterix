// RasteriCEr
// https://github.com/ToNi3141/RasteriCEr
// Copyright (c) 2021 ToNi3141

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

// This module adds bubble cycles into the rendering pipeline. With the assumption, that the pipeline
// contains an even number of steps between read of data and write of data, it is possible by
// adding one bubble cycle, that also the iCE40 version can render every other clock one pixel 
// with just a single port memory, and it is possible to reuse the pipelined shader
// Example shows the collision: 
//  ParamIn     ReadReq     ReadWait    ColorCalc   WriteBack
//  a           -           -           -           -
//  b           a           -           -           -
//  c           b           a           -           -
//  d           c           b           a           -
//  e          _d_          c           b          _a_     <- Collision between d and a if we use a single port mmeory
// Example shows the resolved collision: 
//  ParamIn     ReadReq     ReadWait    ColorCalc   WriteBack
//  a           -           -           -           -
//  -           a           -           -           -
//  b           -           a           -           -
//  -           b           -           a           -
//  c           -           b           -           a      <- No Collision, because during write back no read request is issued
// It is important that the pipeline contains an even number of steps between ReadReq and WriteBack

module FragmentPipelineIce40Wrapper
#(
    parameter FRAMEBUFFER_INDEX_WIDTH = 14
)
(
    input  wire        clk,
    input  wire        reset,
    output wire        pixelInPipeline,

    // Shader configurations
    input  wire [15:0] confReg1,
    input  wire [15:0] confReg2,
    input  wire [15:0] confTextureEnvColor,

    // Fragment Stream
    input  wire        s_axis_tvalid,
    output reg         s_axis_tready,
    input  wire        s_axis_tlast,
    input  wire [FRAMEBUFFER_INDEX_WIDTH + RASTERIZER_AXIS_PARAMETER_SIZE - 1 : 0] s_axis_tdata,

    // Texture access
    output wire [31:0] texelIndex,
    input  wire [15:0] texel,

    // Frame buffer access
    // Read
    output wire [FRAMEBUFFER_INDEX_WIDTH - 1 : 0] colorIndexRead,
    input  wire [15:0] colorIn,
    // Write
    output wire [FRAMEBUFFER_INDEX_WIDTH - 1 : 0] colorIndexWrite,
    output wire        colorWriteEnable,
    output wire [15:0] colorOut,

    // ZBuffer buffer access
    // Read
    output wire [FRAMEBUFFER_INDEX_WIDTH - 1 : 0] depthIndexRead,
    input  wire [15:0] depthIn,
    // Write
    output wire [FRAMEBUFFER_INDEX_WIDTH - 1 : 0] depthIndexWrite,
    output wire        depthWriteEnable,
    output wire [15:0] depthOut
);
`include "RasterizerDefines.vh"
`include "RegisterAndDescriptorDefines.vh"
    reg enablePipeline = 0;
    wire shaderFreeInst;
    reg [FRAMEBUFFER_INDEX_WIDTH - 1 : 0] fbIndexShaderInst;
    reg shaderRequestInst;

    reg                            s_tmp_axis_tvalid;
    wire                           s_tmp_axis_tready;
    reg                            s_tmp_axis_tlast;
    reg  [FRAMEBUFFER_INDEX_WIDTH + RASTERIZER_AXIS_PARAMETER_SIZE - 1 : 0] s_tmp_axis_tdata;

    // assign m_tmp_axis_tvalid = (enablePipeline) ? shaderFreeInst : 0;
    // wire shaderRequestInst = (enablePipeline) ? shadeRequest : 0; 

    always @(posedge clk)
    begin
        if (enablePipeline)
        begin
            s_tmp_axis_tdata <= s_axis_tdata;
            s_tmp_axis_tvalid <= s_axis_tvalid;
            s_tmp_axis_tlast <= s_axis_tlast;
            s_axis_tready <= s_tmp_axis_tready;

            enablePipeline <= 0;
        end
        else
        begin
            s_tmp_axis_tvalid <= 0;
            s_tmp_axis_tlast <= 0;
            s_axis_tready <= 0;

            enablePipeline <= 1;
        end  
    end

    FragmentPipeline fragmentPipeline (    
        .clk(clk),
        .reset(reset),
        .pixelInPipeline(pixelInPipeline),

        .s_axis_tvalid(s_tmp_axis_tvalid),
        .s_axis_tready(s_tmp_axis_tready),
        .s_axis_tlast(s_tmp_axis_tlast),
        .s_axis_tdata(s_tmp_axis_tdata),

        .texelIndex(texelIndex),
        .texel(texel),

        .colorIndexRead(colorIndexRead),
        .colorIn(colorIn),
        .colorIndexWrite(colorIndexWrite),
        .colorWriteEnable(colorWriteEnable),
        .colorOut(colorOut),

        .depthIndexRead(depthIndexRead),
        .depthIn(depthIn),
        .depthIndexWrite(depthIndexWrite),
        .depthWriteEnable(depthWriteEnable),
        .depthOut(depthOut),

        .confReg1(confReg1),
        .confReg2(confReg2),
        .confTextureEnvColor(confTextureEnvColor)
    );
    defparam fragmentPipeline.FRAMEBUFFER_INDEX_WIDTH = FRAMEBUFFER_INDEX_WIDTH;

endmodule
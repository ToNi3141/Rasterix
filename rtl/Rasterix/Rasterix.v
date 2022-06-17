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

`include "PixelUtil.vh"

module Rasterix #(
    // The resolution of the whole screen
    parameter X_RESOLUTION = 128,
    parameter Y_RESOLUTION = 128,
    // The resolution of a subpart of the screen. The whole screen is constructed of 1 to n subparts.
    parameter Y_LINE_RESOLUTION = Y_RESOLUTION,

    // The bit width of the command stream interface
    // Allowed values: 32, 64, 128, 256 bit
    parameter CMD_STREAM_WIDTH = 16,

    // The bit width of the framebuffer stream interface
    parameter FRAMEBUFFER_STREAM_WIDTH = 16,

    // The size of the texture in bytes in power of two
    parameter TEXTURE_BUFFER_SIZE = 15
)
(
    input  wire         aclk,
    input  wire         resetn,

    // AXI Stream command interface
    input  wire         s_cmd_axis_tvalid,
    output wire         s_cmd_axis_tready,
    input  wire         s_cmd_axis_tlast,
    input  wire [CMD_STREAM_WIDTH - 1 : 0]  s_cmd_axis_tdata,

    // Framebuffer output
    // AXI Stream master interface
    output wire         m_framebuffer_axis_tvalid,
    input  wire         m_framebuffer_axis_tready,
    output wire         m_framebuffer_axis_tlast,
    output wire [FRAMEBUFFER_STREAM_WIDTH - 1 : 0]  m_framebuffer_axis_tdata,
    
    // Debug
    output wire [ 3:0]  dbgStreamState,
    output wire         dbgRasterizerRunning
);
`include "RasterizerDefines.vh"
`include "RegisterAndDescriptorDefines.vh"
`include "AttributeInterpolatorDefines.vh"

    localparam MEMORY_SUB_PIXEL_WIDTH = 4;
    localparam TEX_ADDR_WIDTH = 16;

    // The width of the frame buffer index (it would me nice if we could query the frame buffer instance directly ...)
    localparam FRAMEBUFFER_INDEX_WIDTH = $clog2(X_RESOLUTION * Y_LINE_RESOLUTION);

    // The bit width of the texture stream
    localparam TEXTURE_STREAM_WIDTH = CMD_STREAM_WIDTH;

    `Expand(Expand, MEMORY_SUB_PIXEL_WIDTH, COLOR_SUB_PIXEL_WIDTH, COLOR_NUMBER_OF_SUB_PIXEL);
    `Reduce(Reduce, MEMORY_SUB_PIXEL_WIDTH, COLOR_SUB_PIXEL_WIDTH, COLOR_NUMBER_OF_SUB_PIXEL);

    ///////////////////////////
    // Regs and wires
    ///////////////////////////
    // Texture access
    wire [ 7:0]                     confTextureSizeWidth;
    wire [ 7:0]                     confTextureSizeHeight;
    wire                            confTextureClampS;
    wire                            confTextureClampT;
    wire                            confTextureMagFilter;
    wire [TEX_ADDR_WIDTH - 1 : 0]   texelAddr00;
    wire [TEX_ADDR_WIDTH - 1 : 0]   texelAddr01;
    wire [TEX_ADDR_WIDTH - 1 : 0]   texelAddr10;
    wire [TEX_ADDR_WIDTH - 1 : 0]   texelAddr11;
    wire [15 : 0]                   texelInput00;
    wire [15 : 0]                   texelInput01;
    wire [15 : 0]                   texelInput10;
    wire [15 : 0]                   texelInput11;

    // Color buffer access
    wire [FRAMEBUFFER_INDEX_WIDTH - 1 : 0] colorIndexRead;
    wire [FRAMEBUFFER_INDEX_WIDTH - 1 : 0] colorIndexWrite;
    wire        colorWriteEnable;
    wire [15:0] colorIn;
    wire [31:0] colorOut;

    // Depth buffer access
    wire [FRAMEBUFFER_INDEX_WIDTH - 1 : 0] depthIndexRead;
    wire [FRAMEBUFFER_INDEX_WIDTH - 1 : 0] depthIndexWrite;
    wire        depthWriteEnable;
    wire [15:0] depthIn;
    wire [15:0] depthOut;

    wire pixelInPipelineInterpolator;
    wire pixelInPipelineShader;
    wire pixelInPipeline = pixelInPipelineInterpolator || pixelInPipelineShader;
    wire startRendering;

   
    // Control
    wire        rasterizerRunning;
    wire        s_rasterizer_axis_tvalid;
    wire        s_rasterizer_axis_tready;
    wire        s_rasterizer_axis_tlast;
    wire [CMD_STREAM_WIDTH - 1 : 0] s_rasterizer_axis_tdata;

    // Memory
    wire        colorBufferApply;
    wire        colorBufferApplied;
    wire        colorBufferCmdCommit;
    wire        colorBufferCmdMemset;
    wire [31:0] confColorBufferClearColor;
    wire        depthBufferApply;
    wire        depthBufferApplied;
    wire        depthBufferCmdCommit;
    wire        depthBufferCmdMemset;
    wire [15:0] confDepthBufferClearDepth;

    // Texture memory AXIS
    wire        s_texture_axis_tvalid;
    wire        s_texture_axis_tready;
    wire        s_texture_axis_tlast;
    wire [TEXTURE_STREAM_WIDTH - 1 : 0] s_texture_axis_tdata;

    // Attribute interpolator
    wire        m_attr_inter_axis_tvalid;
    wire        m_attr_inter_axis_tready;
    wire        m_attr_inter_axis_tlast;
    wire [ATTR_INTERP_AXIS_PARAMETER_SIZE - 1 : 0] m_attr_inter_axis_tdata;
    wire [31:0] confReg1;
    wire [31:0] confReg2;
    wire [31:0] confTextureEnvColor;
    wire [31:0] confFogColor;

    // Rasterizer
    wire        m_rasterizer_axis_tvalid;
    wire        m_rasterizer_axis_tready;
    wire        m_rasterizer_axis_tlast;
    wire [RASTERIZER_AXIS_PARAMETER_SIZE - 1 : 0] m_rasterizer_axis_tdata;

    // Fog LUT
    wire        s_fog_lut_axis_tvalid;
    wire        s_fog_lut_axis_tready;
    wire        s_fog_lut_axis_tlast;
    wire [CMD_STREAM_WIDTH - 1 : 0] s_fog_lut_axis_tdata;

    // Register bank
    wire [(PARAM_SIZE * `GET_TRIANGLE_SIZE_FOR_BUS_WIDTH(CMD_STREAM_WIDTH)) - 1 : 0] triangleParams;

    assign dbgRasterizerRunning = rasterizerRunning;

    CommandParser commandParser(
        .aclk(aclk),
        .resetn(resetn),

        // AXI Stream command interface
        .s_cmd_axis_tvalid(s_cmd_axis_tvalid),
        .s_cmd_axis_tready(s_cmd_axis_tready),
        .s_cmd_axis_tlast(s_cmd_axis_tlast),
        .s_cmd_axis_tdata(s_cmd_axis_tdata),

        // Fog LUT
        .m_fog_lut_axis_tvalid(s_fog_lut_axis_tvalid),
        .m_fog_lut_axis_tready(s_fog_lut_axis_tready),
        .m_fog_lut_axis_tlast(s_fog_lut_axis_tlast),
        .m_fog_lut_axis_tdata(s_fog_lut_axis_tdata),

        // Rasterizer
        // Configs
        .confTextureSizeWidth(confTextureSizeWidth),
        .confTextureSizeHeight(confTextureSizeHeight),
        .confTextureClampS(confTextureClampS),
        .confTextureClampT(confTextureClampT),
        .confTextureMagFilter(confTextureMagFilter),
        .confReg1(confReg1),
        .confReg2(confReg2),
        .confTextureEnvColor(confTextureEnvColor),
        .confFogColor(confFogColor),
        // Control
        .rasterizerRunning(rasterizerRunning),
        .startRendering(startRendering),
        .pixelInPipeline(pixelInPipeline),
        .m_rasterizer_axis_tvalid(s_rasterizer_axis_tvalid),
        .m_rasterizer_axis_tready(s_rasterizer_axis_tready),
        .m_rasterizer_axis_tlast(s_rasterizer_axis_tlast),
        .m_rasterizer_axis_tdata(s_rasterizer_axis_tdata),

        // applied
        .colorBufferApply(colorBufferApply),
        .colorBufferApplied(colorBufferApplied),
        .colorBufferCmdCommit(colorBufferCmdCommit),
        .colorBufferCmdMemset(colorBufferCmdMemset),
        .confColorBufferClearColor(confColorBufferClearColor),
        .depthBufferApply(depthBufferApply),
        .depthBufferApplied(depthBufferApplied),
        .depthBufferCmdCommit(depthBufferCmdCommit),
        .depthBufferCmdMemset(depthBufferCmdMemset),
        .confDepthBufferClearDepth(confDepthBufferClearDepth),

        // Texture AXIS interface
        .m_texture_axis_tvalid(s_texture_axis_tvalid),
        .m_texture_axis_tready(s_texture_axis_tready),
        .m_texture_axis_tlast(s_texture_axis_tlast),
        .m_texture_axis_tdata(s_texture_axis_tdata),

        // Debug
        .dbgStreamState(dbgStreamState)
    );
    defparam commandParser.CMD_STREAM_WIDTH = CMD_STREAM_WIDTH;
    defparam commandParser.TEXTURE_STREAM_WIDTH = TEXTURE_STREAM_WIDTH;

    ///////////////////////////
    // Modul Instantiation and wiring
    ///////////////////////////
    RegisterBank regBank (
        .aclk(aclk),
        .resetn(resetn),

        .s_axis_tvalid(s_rasterizer_axis_tvalid),
        .s_axis_tready(s_rasterizer_axis_tready),
        .s_axis_tlast(s_rasterizer_axis_tlast),
        .s_axis_tdata(s_rasterizer_axis_tdata),

        .registers(triangleParams)
    );
    defparam regBank.BANK_SIZE = `GET_TRIANGLE_SIZE_FOR_BUS_WIDTH(CMD_STREAM_WIDTH);
    defparam regBank.CMD_STREAM_WIDTH = CMD_STREAM_WIDTH;

    TextureBuffer texCache (
        .aclk(aclk),
        .resetn(resetn),

        .texelAddr00(texelAddr00),
        .texelAddr01(texelAddr01),
        .texelAddr10(texelAddr10),
        .texelAddr11(texelAddr11),

        .texelOutput00(texelInput00),
        .texelOutput01(texelInput01),
        .texelOutput10(texelInput10),
        .texelOutput11(texelInput11),

        .s_axis_tvalid(s_texture_axis_tvalid),
        .s_axis_tready(s_texture_axis_tready),
        .s_axis_tlast(s_texture_axis_tlast),
        .s_axis_tdata(s_texture_axis_tdata)
    );
    defparam texCache.STREAM_WIDTH = TEXTURE_STREAM_WIDTH;
    defparam texCache.SIZE = TEXTURE_BUFFER_SIZE;

    FrameBuffer depthBuffer (  
        .clk(aclk),
        .reset(!resetn),

        .fragIndexRead(depthIndexRead),
        .fragOut(depthIn),
        .fragIndexWrite(depthIndexWrite),
        .fragIn(depthOut),
        .fragWriteEnable(depthWriteEnable),
        .fragMask(confReg1[REG1_DEPTH_MASK_POS +: REG1_DEPTH_MASK_SIZE]),
        
        .apply(depthBufferApply),
        .applied(depthBufferApplied),
        .cmdCommit(depthBufferCmdCommit),
        .cmdMemset(depthBufferCmdMemset),

        .m_axis_tvalid(),
        .m_axis_tready(1'b1),
        .m_axis_tlast(),
        .m_axis_tdata(),

        .clearColor(confDepthBufferClearDepth)
    );
    defparam depthBuffer.FRAME_SIZE = X_RESOLUTION * Y_LINE_RESOLUTION;
    defparam depthBuffer.STREAM_WIDTH = FRAMEBUFFER_STREAM_WIDTH;
    defparam depthBuffer.NUMBER_OF_SUB_PIXELS = 1;
    defparam depthBuffer.SUB_PIXEL_WIDTH = 16;

    FrameBuffer colorBuffer (  
        .clk(aclk),
        .reset(!resetn),

        .fragIndexRead(colorIndexRead),
        .fragOut(colorIn),
        .fragIndexWrite(colorIndexWrite),
        .fragIn(Reduce(colorOut)),
        .fragWriteEnable(colorWriteEnable),
        .fragMask({ confReg1[REG1_COLOR_MASK_R_POS +: REG1_COLOR_MASK_R_SIZE], 
                    confReg1[REG1_COLOR_MASK_G_POS +: REG1_COLOR_MASK_G_SIZE], 
                    confReg1[REG1_COLOR_MASK_B_POS +: REG1_COLOR_MASK_B_SIZE], 
                    confReg1[REG1_COLOR_MASK_A_POS +: REG1_COLOR_MASK_A_SIZE]}),
        
        .apply(colorBufferApply),
        .applied(colorBufferApplied),
        .cmdCommit(colorBufferCmdCommit),
        .cmdMemset(colorBufferCmdMemset),
        .clearColor(Reduce(confColorBufferClearColor)),

        .m_axis_tvalid(m_framebuffer_axis_tvalid),
        .m_axis_tready(m_framebuffer_axis_tready),
        .m_axis_tlast(m_framebuffer_axis_tlast),
        .m_axis_tdata(m_framebuffer_axis_tdata)
    );
    defparam colorBuffer.FRAME_SIZE = X_RESOLUTION * Y_LINE_RESOLUTION;
    defparam colorBuffer.STREAM_WIDTH = FRAMEBUFFER_STREAM_WIDTH;
    defparam colorBuffer.NUMBER_OF_SUB_PIXELS = COLOR_NUMBER_OF_SUB_PIXEL;
    defparam colorBuffer.SUB_PIXEL_WIDTH = MEMORY_SUB_PIXEL_WIDTH;

    Rasterizer rop (
        .clk(aclk), 
        .reset(!resetn), 

        .rasterizerRunning(rasterizerRunning),
        .startRendering(startRendering),

        .m_axis_tvalid(m_rasterizer_axis_tvalid),
        .m_axis_tready(m_rasterizer_axis_tready),
        .m_axis_tlast(m_rasterizer_axis_tlast),
        .m_axis_tdata(m_rasterizer_axis_tdata),

        .bbStart(triangleParams[BB_START * PARAM_SIZE +: PARAM_SIZE]),
        .bbEnd(triangleParams[BB_END * PARAM_SIZE +: PARAM_SIZE]),
        .w0(triangleParams[INC_W0 * PARAM_SIZE +: PARAM_SIZE]),
        .w1(triangleParams[INC_W1 * PARAM_SIZE +: PARAM_SIZE]),
        .w2(triangleParams[INC_W2 * PARAM_SIZE +: PARAM_SIZE]),
        .w0IncX(triangleParams[INC_W0_X * PARAM_SIZE +: PARAM_SIZE]),
        .w1IncX(triangleParams[INC_W1_X * PARAM_SIZE +: PARAM_SIZE]),
        .w2IncX(triangleParams[INC_W2_X * PARAM_SIZE +: PARAM_SIZE]),
        .w0IncY(triangleParams[INC_W0_Y * PARAM_SIZE +: PARAM_SIZE]),
        .w1IncY(triangleParams[INC_W1_Y * PARAM_SIZE +: PARAM_SIZE]),
        .w2IncY(triangleParams[INC_W2_Y * PARAM_SIZE +: PARAM_SIZE])
    );
    defparam rop.X_RESOLUTION = X_RESOLUTION;
    defparam rop.Y_RESOLUTION = Y_RESOLUTION;
    defparam rop.Y_LINE_RESOLUTION = Y_LINE_RESOLUTION;
    defparam rop.FRAMEBUFFER_INDEX_WIDTH = FRAMEBUFFER_INDEX_WIDTH;
    defparam rop.CMD_STREAM_WIDTH = CMD_STREAM_WIDTH;

    AttributeInterpolator attributeInterpolator (
        .aclk(aclk),
        .resetn(resetn),
        .pixelInPipeline(pixelInPipelineInterpolator),

        .s_axis_tvalid(m_rasterizer_axis_tvalid),
        .s_axis_tready(m_rasterizer_axis_tready),
        .s_axis_tlast(m_rasterizer_axis_tlast),
        .s_axis_tdata(m_rasterizer_axis_tdata),

        .m_axis_tvalid(m_attr_inter_axis_tvalid),
        .m_axis_tready(m_attr_inter_axis_tready),
        .m_axis_tlast(m_attr_inter_axis_tlast),
        .m_axis_tdata(m_attr_inter_axis_tdata),

        .tex_s(triangleParams[INC_TEX_S * PARAM_SIZE +: PARAM_SIZE]),
        .tex_t(triangleParams[INC_TEX_T * PARAM_SIZE +: PARAM_SIZE]),
        .tex_s_inc_x(triangleParams[INC_TEX_S_X * PARAM_SIZE +: PARAM_SIZE]),
        .tex_t_inc_x(triangleParams[INC_TEX_T_X * PARAM_SIZE +: PARAM_SIZE]),
        .tex_s_inc_y(triangleParams[INC_TEX_S_Y * PARAM_SIZE +: PARAM_SIZE]),
        .tex_t_inc_y(triangleParams[INC_TEX_T_Y * PARAM_SIZE +: PARAM_SIZE]),
        .depth_w(triangleParams[INC_DEPTH_W * PARAM_SIZE +: PARAM_SIZE]),
        .depth_w_inc_x(triangleParams[INC_DEPTH_W_X * PARAM_SIZE +: PARAM_SIZE]),
        .depth_w_inc_y(triangleParams[INC_DEPTH_W_Y * PARAM_SIZE +: PARAM_SIZE]),
        .depth_z(triangleParams[INC_DEPTH_Z * PARAM_SIZE +: PARAM_SIZE]),
        .depth_z_inc_x(triangleParams[INC_DEPTH_Z_X * PARAM_SIZE +: PARAM_SIZE]),
        .depth_z_inc_y(triangleParams[INC_DEPTH_Z_Y * PARAM_SIZE +: PARAM_SIZE]),
        .color_r(triangleParams[INC_COLOR_R * PARAM_SIZE +: PARAM_SIZE]),
        .color_r_inc_x(triangleParams[INC_COLOR_R_X * PARAM_SIZE +: PARAM_SIZE]),
        .color_r_inc_y(triangleParams[INC_COLOR_R_Y * PARAM_SIZE +: PARAM_SIZE]),
        .color_g(triangleParams[INC_COLOR_G * PARAM_SIZE +: PARAM_SIZE]),
        .color_g_inc_x(triangleParams[INC_COLOR_G_X * PARAM_SIZE +: PARAM_SIZE]),
        .color_g_inc_y(triangleParams[INC_COLOR_G_Y * PARAM_SIZE +: PARAM_SIZE]),
        .color_b(triangleParams[INC_COLOR_B * PARAM_SIZE +: PARAM_SIZE]),
        .color_b_inc_x(triangleParams[INC_COLOR_B_X * PARAM_SIZE +: PARAM_SIZE]),
        .color_b_inc_y(triangleParams[INC_COLOR_B_Y * PARAM_SIZE +: PARAM_SIZE]),
        .color_a(triangleParams[INC_COLOR_A * PARAM_SIZE +: PARAM_SIZE]),
        .color_a_inc_x(triangleParams[INC_COLOR_A_X * PARAM_SIZE +: PARAM_SIZE]),
        .color_a_inc_y(triangleParams[INC_COLOR_A_Y * PARAM_SIZE +: PARAM_SIZE])
    );

    PixelPipeline pixelPipeline (    
        .aclk(aclk),
        .resetn(resetn),
        .pixelInPipeline(pixelInPipelineShader),

        .s_fog_lut_axis_tvalid(s_fog_lut_axis_tvalid),
        .s_fog_lut_axis_tready(s_fog_lut_axis_tready),
        .s_fog_lut_axis_tlast(s_fog_lut_axis_tlast),
        .s_fog_lut_axis_tdata(s_fog_lut_axis_tdata),

        .confReg1(confReg1),
        .confReg2(confReg2),
        .confTextureClampS(confTextureClampS),
        .confTextureClampT(confTextureClampT),
        .confTextureEnvColor(confTextureEnvColor),
        .confFogColor(confFogColor),
        .confTextureSizeWidth(confTextureSizeWidth),
        .confTextureSizeHeight(confTextureSizeHeight),
        .confTextureMagFilter(confTextureMagFilter),

        .s_axis_tvalid(m_attr_inter_axis_tvalid),
        .s_axis_tready(m_attr_inter_axis_tready),
        .s_axis_tlast(m_attr_inter_axis_tlast),
        .s_axis_tdata(m_attr_inter_axis_tdata),

        .texelAddr00(texelAddr00),
        .texelAddr01(texelAddr01),
        .texelAddr10(texelAddr10),
        .texelAddr11(texelAddr11),

        .texelInput00(Expand(texelInput00)),
        .texelInput01(Expand(texelInput01)),
        .texelInput10(Expand(texelInput10)),
        .texelInput11(Expand(texelInput11)),

        .colorIndexRead(colorIndexRead),
        .colorIn(Expand(colorIn)),
        .colorIndexWrite(colorIndexWrite),
        .colorWriteEnable(colorWriteEnable),
        .colorOut(colorOut),

        .depthIndexRead(depthIndexRead),
        .depthIn(depthIn),
        .depthIndexWrite(depthIndexWrite),
        .depthWriteEnable(depthWriteEnable),
        .depthOut(depthOut)
    );
    defparam pixelPipeline.FRAMEBUFFER_INDEX_WIDTH = FRAMEBUFFER_INDEX_WIDTH;
    defparam pixelPipeline.CMD_STREAM_WIDTH = CMD_STREAM_WIDTH;
    defparam pixelPipeline.SUB_PIXEL_WIDTH = COLOR_SUB_PIXEL_WIDTH;

endmodule
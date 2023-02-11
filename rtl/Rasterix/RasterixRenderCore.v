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

module RasterixRenderCore #(
    // The size of the internal framebuffer (in power of two)
    parameter FRAMEBUFFER_SIZE_BYTES = 18,

    // This is the color depth of the framebuffer. Note: This setting has no influence on the framebuffer stream. This steam will
    // stay at RGB565. It changes the internal representation and might be used to reduce the memory footprint.
    // Lower depth will result in color banding.
    parameter FRAMEBUFFER_SUB_PIXEL_WIDTH = 6,
    // This enables the alpha channel of the framebuffer. Requires additional memory.
    parameter FRAMEBUFFER_ENABLE_ALPHA_CHANNEL = 0,

    // Number of TMUs. Currently supported values: 1 and 2
    parameter TMU_COUNT = 2,
    
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
    // AXI Stream master interface (RGB565)
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
    localparam FRAMEBUFFER_NUMBER_OF_SUB_PIXELS = (FRAMEBUFFER_ENABLE_ALPHA_CHANNEL == 0) ? 3 : 4;

    localparam TEX_ADDR_WIDTH = 16;

    localparam FRAMEBUFFER_INDEX_WIDTH = FRAMEBUFFER_SIZE_BYTES - 1;

    // The bit width of the texture stream
    localparam TEXTURE_STREAM_WIDTH = CMD_STREAM_WIDTH;

    initial
    begin
        if (TMU_COUNT > 2)
        begin
            $error("Only a maximum of 2 TMUs are supported");
        end
        if (TMU_COUNT < 1)
        begin
            $error("At least one TMU must be enabled");
        end
    end

    localparam ENABLE_SECOND_TMU = TMU_COUNT == 2;
    
    ///////////////////////////
    // Regs and wires
    ///////////////////////////
    // Texture access TMU0
    wire [TEX_ADDR_WIDTH - 1 : 0]   texel0Addr00;
    wire [TEX_ADDR_WIDTH - 1 : 0]   texel0Addr01;
    wire [TEX_ADDR_WIDTH - 1 : 0]   texel0Addr10;
    wire [TEX_ADDR_WIDTH - 1 : 0]   texel0Addr11;
    wire [31 : 0]                   texel0Input00;
    wire [31 : 0]                   texel0Input01;
    wire [31 : 0]                   texel0Input10;
    wire [31 : 0]                   texel0Input11;
    // Texture access TMU1
    wire [TEX_ADDR_WIDTH - 1 : 0]   texel1Addr00;
    wire [TEX_ADDR_WIDTH - 1 : 0]   texel1Addr01;
    wire [TEX_ADDR_WIDTH - 1 : 0]   texel1Addr10;
    wire [TEX_ADDR_WIDTH - 1 : 0]   texel1Addr11;
    wire [31 : 0]                   texel1Input00;
    wire [31 : 0]                   texel1Input01;
    wire [31 : 0]                   texel1Input10;
    wire [31 : 0]                   texel1Input11;

    // Color buffer access
    wire [FRAMEBUFFER_INDEX_WIDTH - 1 : 0] colorIndexRead;
    wire [FRAMEBUFFER_INDEX_WIDTH - 1 : 0] colorIndexWrite;
    wire            colorWriteEnable;
    wire [31 : 0]   colorIn;
    wire [31 : 0]   colorOut;
    wire [ATTR_INTERP_AXIS_SCREEN_POS_SIZE - 1 : 0] colorOutScreenPosX;
    wire [ATTR_INTERP_AXIS_SCREEN_POS_SIZE - 1 : 0] colorOutScreenPosY;

    // Depth buffer access
    wire [FRAMEBUFFER_INDEX_WIDTH - 1 : 0] depthIndexRead;
    wire [FRAMEBUFFER_INDEX_WIDTH - 1 : 0] depthIndexWrite;
    wire            depthWriteEnable;
    wire [15 : 0]   depthIn;
    wire [15 : 0]   depthOut;
    wire [ATTR_INTERP_AXIS_SCREEN_POS_SIZE - 1 : 0] depthOutScreenPosX;
    wire [ATTR_INTERP_AXIS_SCREEN_POS_SIZE - 1 : 0] depthOutScreenPosY;

    wire pixelInPipelineInterpolator;
    wire pixelInPipelineShader;
    wire pixelInPipeline = pixelInPipelineInterpolator || pixelInPipelineShader;
    wire startRendering;

   
    // Control
    wire            rasterizerRunning;

    // Memory
    wire            colorBufferApply;
    wire            colorBufferApplied;
    wire            colorBufferCmdCommit;
    wire            colorBufferCmdMemset;
    wire [31 : 0]   confColorBufferClearColor;
    wire            depthBufferApply;
    wire            depthBufferApplied;
    wire            depthBufferCmdCommit;
    wire            depthBufferCmdMemset;
    wire [31 : 0]   confDepthBufferClearDepth;

    // Attribute interpolator
    wire            m_attr_inter_axis_tvalid;
    wire            m_attr_inter_axis_tready;
    wire            m_attr_inter_axis_tlast;
    wire [ATTR_INTERP_AXIS_PARAMETER_SIZE - 1 : 0] m_attr_inter_axis_tdata;

    // Rasterizer
    wire            m_rasterizer_axis_tvalid;
    wire            m_rasterizer_axis_tready;
    wire            m_rasterizer_axis_tlast;
    wire [RASTERIZER_AXIS_PARAMETER_SIZE - 1 : 0] m_rasterizer_axis_tdata;

    // Steams
    wire [CMD_STREAM_WIDTH - 1 : 0]  s_cmd_xxx_axis_tdata;
    wire [ 3 : 0]    s_cmd_xxx_axis_tuser;
    wire             s_cmd_xxx_axis_tlast;
    wire             s_cmd_fog_axis_tvalid;
    wire             s_cmd_rasterizer_axis_tvalid;
    wire             s_cmd_tmu0_axis_tvalid;
    wire             s_cmd_tmu1_axis_tvalid;
    wire             s_cmd_config_axis_tvalid;
    wire             s_cmd_fog_axis_tready;
    wire             s_cmd_rasterizer_axis_tready;
    wire             s_cmd_tmu0_axis_tready;
    wire             s_cmd_tmu1_axis_tready;
    wire             s_cmd_config_axis_tready;


    // Register bank
    wire [(TRIANGLE_STREAM_PARAM_SIZE * `GET_TRIANGLE_SIZE_FOR_BUS_WIDTH(CMD_STREAM_WIDTH)) - 1 : 0] triangleParams;
    wire [(OP_RENDER_CONFIG_REG_WIDTH * OP_RENDER_CONFIG_NUMBER_OR_REGS) - 1 : 0] renderConfigs;

    // Configs
    wire [OP_RENDER_CONFIG_REG_WIDTH - 1 : 0]   confFeatureEnable;
    wire [OP_RENDER_CONFIG_REG_WIDTH - 1 : 0]   confFragmentPipelineConfig;
    wire [OP_RENDER_CONFIG_REG_WIDTH - 1 : 0]   confFragmentPipelineFogColor;
    wire [OP_RENDER_CONFIG_REG_WIDTH - 1 : 0]   confTMU0TexEnvConfig;
    wire [OP_RENDER_CONFIG_REG_WIDTH - 1 : 0]   confTMU0TextureConfig;
    wire [OP_RENDER_CONFIG_REG_WIDTH - 1 : 0]   confTMU0TexEnvColor;
    wire [OP_RENDER_CONFIG_REG_WIDTH - 1 : 0]   confScissorStartXY;
    wire [OP_RENDER_CONFIG_REG_WIDTH - 1 : 0]   confScissorEndXY;
    wire [OP_RENDER_CONFIG_REG_WIDTH - 1 : 0]   confYOffset;
    wire [OP_RENDER_CONFIG_REG_WIDTH - 1 : 0]   confRenderResolution;
    wire [OP_RENDER_CONFIG_REG_WIDTH - 1 : 0]   confTMU1TexEnvConfig;
    wire [OP_RENDER_CONFIG_REG_WIDTH - 1 : 0]   confTMU1TextureConfig;
    wire [OP_RENDER_CONFIG_REG_WIDTH - 1 : 0]   confTMU1TexEnvColor;

    assign confFeatureEnable = renderConfigs[OP_RENDER_CONFIG_FEATURE_ENABLE +: OP_RENDER_CONFIG_REG_WIDTH];
    assign confColorBufferClearColor = renderConfigs[OP_RENDER_CONFIG_COLOR_BUFFER_CLEAR_COLOR +: OP_RENDER_CONFIG_REG_WIDTH];
    assign confDepthBufferClearDepth = renderConfigs[OP_RENDER_CONFIG_DEPTH_BUFFER_CLEAR_DEPTH +: OP_RENDER_CONFIG_REG_WIDTH];
    assign confFragmentPipelineConfig = renderConfigs[OP_RENDER_CONFIG_FRAGMENT_PIPELINE +: OP_RENDER_CONFIG_REG_WIDTH];
    assign confFragmentPipelineFogColor = renderConfigs[OP_RENDER_CONFIG_FRAGMENT_FOG_COLOR +: OP_RENDER_CONFIG_REG_WIDTH];
    assign confTMU0TexEnvConfig = renderConfigs[OP_RENDER_CONFIG_TMU0_TEX_ENV +: OP_RENDER_CONFIG_REG_WIDTH];
    assign confTMU0TextureConfig = renderConfigs[OP_RENDER_CONFIG_TMU0_TEXTURE_CONFIG +: OP_RENDER_CONFIG_REG_WIDTH];
    assign confTMU0TexEnvColor = renderConfigs[OP_RENDER_CONFIG_TMU0_TEX_ENV_COLOR +: OP_RENDER_CONFIG_REG_WIDTH];
    assign confTMU1TexEnvConfig = renderConfigs[OP_RENDER_CONFIG_TMU1_TEX_ENV +: OP_RENDER_CONFIG_REG_WIDTH];
    assign confTMU1TextureConfig = renderConfigs[OP_RENDER_CONFIG_TMU1_TEXTURE_CONFIG +: OP_RENDER_CONFIG_REG_WIDTH];
    assign confTMU1TexEnvColor = renderConfigs[OP_RENDER_CONFIG_TMU1_TEX_ENV_COLOR +: OP_RENDER_CONFIG_REG_WIDTH];
    assign confScissorStartXY = renderConfigs[OP_RENDER_CONFIG_SCISSOR_START_XY +: OP_RENDER_CONFIG_REG_WIDTH];
    assign confScissorEndXY = renderConfigs[OP_RENDER_CONFIG_SCISSOR_END_XY +: OP_RENDER_CONFIG_REG_WIDTH];
    assign confYOffset = renderConfigs[OP_RENDER_CONFIG_Y_OFFSET +: OP_RENDER_CONFIG_REG_WIDTH];
    assign confRenderResolution = renderConfigs[OP_RENDER_CONFIG_RENDER_RESOLUTION +: OP_RENDER_CONFIG_REG_WIDTH];

    assign dbgRasterizerRunning = rasterizerRunning;

    CommandParser commandParser(
        .aclk(aclk),
        .resetn(resetn),

        // AXI Stream command interface
        .s_cmd_axis_tvalid(s_cmd_axis_tvalid),
        .s_cmd_axis_tready(s_cmd_axis_tready),
        .s_cmd_axis_tlast(s_cmd_axis_tlast),
        .s_cmd_axis_tdata(s_cmd_axis_tdata),

        .m_cmd_xxx_axis_tdata(s_cmd_xxx_axis_tdata),
        .m_cmd_xxx_axis_tuser(s_cmd_xxx_axis_tuser),
        .m_cmd_xxx_axis_tlast(s_cmd_xxx_axis_tlast),
        .m_cmd_fog_axis_tvalid(s_cmd_fog_axis_tvalid),
        .m_cmd_rasterizer_axis_tvalid(s_cmd_rasterizer_axis_tvalid),
        .m_cmd_tmu0_axis_tvalid(s_cmd_tmu0_axis_tvalid),
        .m_cmd_tmu1_axis_tvalid(s_cmd_tmu1_axis_tvalid),
        .m_cmd_config_axis_tvalid(s_cmd_config_axis_tvalid),
        .m_cmd_fog_axis_tready(s_cmd_fog_axis_tready),
        .m_cmd_rasterizer_axis_tready(s_cmd_rasterizer_axis_tready),
        .m_cmd_tmu0_axis_tready(s_cmd_tmu0_axis_tready),
        .m_cmd_tmu1_axis_tready(s_cmd_tmu1_axis_tready),
        .m_cmd_config_axis_tready(s_cmd_config_axis_tready),

        // Rasterizer
        // Control
        .rasterizerRunning(rasterizerRunning),
        .startRendering(startRendering),
        .pixelInPipeline(pixelInPipeline),

        // applied
        .colorBufferApply(colorBufferApply),
        .colorBufferApplied(colorBufferApplied),
        .colorBufferCmdCommit(colorBufferCmdCommit),
        .colorBufferCmdMemset(colorBufferCmdMemset),
        .depthBufferApply(depthBufferApply),
        .depthBufferApplied(depthBufferApplied),
        .depthBufferCmdCommit(depthBufferCmdCommit),
        .depthBufferCmdMemset(depthBufferCmdMemset),

        // Debug
        .dbgStreamState(dbgStreamState)
    );
    defparam commandParser.CMD_STREAM_WIDTH = CMD_STREAM_WIDTH;
    defparam commandParser.TEXTURE_STREAM_WIDTH = TEXTURE_STREAM_WIDTH;

    ///////////////////////////
    // Modul Instantiation and wiring
    ///////////////////////////
    RegisterBank triangleParameters (
        .aclk(aclk),
        .resetn(resetn),

        .s_axis_tvalid(s_cmd_rasterizer_axis_tvalid),
        .s_axis_tready(s_cmd_rasterizer_axis_tready),
        .s_axis_tlast(s_cmd_xxx_axis_tlast),
        .s_axis_tdata(s_cmd_xxx_axis_tdata),
        .s_axis_tuser(0),

        .registers(triangleParams)
    );
    defparam triangleParameters.BANK_SIZE = `GET_TRIANGLE_SIZE_FOR_BUS_WIDTH(CMD_STREAM_WIDTH);
    defparam triangleParameters.CMD_STREAM_WIDTH = CMD_STREAM_WIDTH;

    RegisterBank renderConfigsRegBank (
        .aclk(aclk),
        .resetn(resetn),

        .s_axis_tvalid(s_cmd_config_axis_tvalid),
        .s_axis_tready(s_cmd_config_axis_tready),
        .s_axis_tlast(s_cmd_xxx_axis_tlast),
        .s_axis_tdata(s_cmd_xxx_axis_tdata),
        .s_axis_tuser(s_cmd_xxx_axis_tuser),

        .registers(renderConfigs)
    );
    defparam renderConfigsRegBank.BANK_SIZE = OP_RENDER_CONFIG_NUMBER_OR_REGS;
    defparam renderConfigsRegBank.CMD_STREAM_WIDTH = CMD_STREAM_WIDTH;
    defparam renderConfigsRegBank.COMPRESSED = 0;

    TextureBuffer textureBufferTMU0 (
        .aclk(aclk),
        .resetn(resetn),

        .confPixelFormat(confTMU0TextureConfig[RENDER_CONFIG_TMU_TEXTURE_PIXEL_FORMAT_POS +: RENDER_CONFIG_TMU_TEXTURE_PIXEL_FORMAT_SIZE]),

        .texelAddr00(texel0Addr00),
        .texelAddr01(texel0Addr01),
        .texelAddr10(texel0Addr10),
        .texelAddr11(texel0Addr11),

        .texelOutput00(texel0Input00),
        .texelOutput01(texel0Input01),
        .texelOutput10(texel0Input10),
        .texelOutput11(texel0Input11),

        .s_axis_tvalid(s_cmd_tmu0_axis_tvalid),
        .s_axis_tready(s_cmd_tmu0_axis_tready),
        .s_axis_tlast(s_cmd_xxx_axis_tlast),
        .s_axis_tdata(s_cmd_xxx_axis_tdata)
    );
    defparam textureBufferTMU0.STREAM_WIDTH = TEXTURE_STREAM_WIDTH;
    defparam textureBufferTMU0.SIZE = TEXTURE_BUFFER_SIZE;
    defparam textureBufferTMU0.PIXEL_WIDTH = COLOR_NUMBER_OF_SUB_PIXEL * COLOR_SUB_PIXEL_WIDTH;

    generate
        if (ENABLE_SECOND_TMU)
        begin
            TextureBuffer textureBufferTMU1 (
                .aclk(aclk),
                .resetn(resetn),

                .confPixelFormat(confTMU1TextureConfig[RENDER_CONFIG_TMU_TEXTURE_PIXEL_FORMAT_POS +: RENDER_CONFIG_TMU_TEXTURE_PIXEL_FORMAT_SIZE]),

                .texelAddr00(texel1Addr00),
                .texelAddr01(texel1Addr01),
                .texelAddr10(texel1Addr10),
                .texelAddr11(texel1Addr11),

                .texelOutput00(texel1Input00),
                .texelOutput01(texel1Input01),
                .texelOutput10(texel1Input10),
                .texelOutput11(texel1Input11),

                .s_axis_tvalid(s_cmd_tmu1_axis_tvalid),
                .s_axis_tready(s_cmd_tmu1_axis_tready),
                .s_axis_tlast(s_cmd_xxx_axis_tlast),
                .s_axis_tdata(s_cmd_xxx_axis_tdata)
            );
            defparam textureBufferTMU1.STREAM_WIDTH = TEXTURE_STREAM_WIDTH;
            defparam textureBufferTMU1.SIZE = TEXTURE_BUFFER_SIZE;
            defparam textureBufferTMU1.PIXEL_WIDTH = COLOR_NUMBER_OF_SUB_PIXEL * COLOR_SUB_PIXEL_WIDTH;
        end
        else
        begin
            assign texel1Input00 = 0;
            assign texel1Input01 = 0;
            assign texel1Input10 = 0;
            assign texel1Input11 = 0;
            assign s_cmd_tmu1_axis_tready = 1;
        end
    endgenerate

    FrameBuffer depthBuffer (  
        .clk(aclk),
        .reset(!resetn),

        .confClearColor(confDepthBufferClearDepth[RENDER_CONFIG_CLEAR_DEPTH_POS +: RENDER_CONFIG_CLEAR_DEPTH_SIZE]),
        .confEnableScissor(confFeatureEnable[RENDER_CONFIG_FEATURE_ENABLE_SCISSOR_POS]),
        .confScissorStartX(confScissorStartXY[RENDER_CONFIG_X_POS +: RENDER_CONFIG_X_SIZE]),
        .confScissorStartY(confScissorStartXY[RENDER_CONFIG_Y_POS +: RENDER_CONFIG_Y_SIZE]),
        .confScissorEndX(confScissorEndXY[RENDER_CONFIG_X_POS +: RENDER_CONFIG_X_SIZE]),
        .confScissorEndY(confScissorEndXY[RENDER_CONFIG_Y_POS +: RENDER_CONFIG_Y_SIZE]),
        .confYOffset(confYOffset[RENDER_CONFIG_Y_POS +: RENDER_CONFIG_Y_SIZE]),
        .confXResolution(confRenderResolution[RENDER_CONFIG_X_POS +: RENDER_CONFIG_X_SIZE]),
        .confYResolution(confRenderResolution[RENDER_CONFIG_Y_POS +: RENDER_CONFIG_Y_SIZE]),

        .fragIndexRead(depthIndexRead),
        .fragOut(depthIn),
        .fragIndexWrite(depthIndexWrite),
        .fragIn(depthOut),
        .fragWriteEnable(depthWriteEnable),
        .fragMask(confFragmentPipelineConfig[RENDER_CONFIG_FRAGMENT_DEPTH_MASK_POS +: RENDER_CONFIG_FRAGMENT_DEPTH_MASK_SIZE]),
        .screenPosX(depthOutScreenPosX),
        .screenPosY(depthOutScreenPosY),

        .apply(depthBufferApply),
        .applied(depthBufferApplied),
        .cmdCommit(depthBufferCmdCommit),
        .cmdMemset(depthBufferCmdMemset),

        .m_axis_tvalid(),
        .m_axis_tready(1'b1),
        .m_axis_tlast(),
        .m_axis_tdata()
    );
    defparam depthBuffer.STREAM_WIDTH = FRAMEBUFFER_STREAM_WIDTH;
    defparam depthBuffer.NUMBER_OF_SUB_PIXELS = 1;
    defparam depthBuffer.NUMBER_OF_SUB_PIXELS_INTERNAL = 1;
    defparam depthBuffer.SUB_PIXEL_WIDTH_INTERNAL = 16;
    defparam depthBuffer.SUB_PIXEL_WIDTH = 16;
    defparam depthBuffer.X_BIT_WIDTH = RENDER_CONFIG_X_SIZE;
    defparam depthBuffer.Y_BIT_WIDTH = RENDER_CONFIG_Y_SIZE;
    defparam depthBuffer.FRAMEBUFFER_SIZE_BYTES = FRAMEBUFFER_SIZE_BYTES;

    FrameBuffer colorBuffer (  
        .clk(aclk),
        .reset(!resetn),

        .confClearColor(confColorBufferClearColor),
        .confEnableScissor(confFeatureEnable[RENDER_CONFIG_FEATURE_ENABLE_SCISSOR_POS]),
        .confScissorStartX(confScissorStartXY[RENDER_CONFIG_X_POS +: RENDER_CONFIG_X_SIZE]),
        .confScissorStartY(confScissorStartXY[RENDER_CONFIG_Y_POS +: RENDER_CONFIG_Y_SIZE]),
        .confScissorEndX(confScissorEndXY[RENDER_CONFIG_X_POS +: RENDER_CONFIG_X_SIZE]),
        .confScissorEndY(confScissorEndXY[RENDER_CONFIG_Y_POS +: RENDER_CONFIG_Y_SIZE]),
        .confYOffset(confYOffset[RENDER_CONFIG_Y_POS +: RENDER_CONFIG_Y_SIZE]),
        .confXResolution(confRenderResolution[RENDER_CONFIG_X_POS +: RENDER_CONFIG_X_SIZE]),
        .confYResolution(confRenderResolution[RENDER_CONFIG_Y_POS +: RENDER_CONFIG_Y_SIZE]),

        .fragIndexRead(colorIndexRead),
        .fragOut(colorIn),
        .fragIndexWrite(colorIndexWrite),
        .fragIn(colorOut),
        .fragWriteEnable(colorWriteEnable),
        .fragMask({ confFragmentPipelineConfig[RENDER_CONFIG_FRAGMENT_COLOR_MASK_R_POS +: RENDER_CONFIG_FRAGMENT_COLOR_MASK_R_SIZE], 
                    confFragmentPipelineConfig[RENDER_CONFIG_FRAGMENT_COLOR_MASK_G_POS +: RENDER_CONFIG_FRAGMENT_COLOR_MASK_G_SIZE], 
                    confFragmentPipelineConfig[RENDER_CONFIG_FRAGMENT_COLOR_MASK_B_POS +: RENDER_CONFIG_FRAGMENT_COLOR_MASK_B_SIZE], 
                    confFragmentPipelineConfig[RENDER_CONFIG_FRAGMENT_COLOR_MASK_A_POS +: RENDER_CONFIG_FRAGMENT_COLOR_MASK_A_SIZE]}),
        .screenPosX(colorOutScreenPosX),
        .screenPosY(colorOutScreenPosY),
        
        .apply(colorBufferApply),
        .applied(colorBufferApplied),
        .cmdCommit(colorBufferCmdCommit),
        .cmdMemset(colorBufferCmdMemset),

        .m_axis_tvalid(m_framebuffer_axis_tvalid),
        .m_axis_tready(m_framebuffer_axis_tready),
        .m_axis_tlast(m_framebuffer_axis_tlast),
        .m_axis_tdata(m_framebuffer_axis_tdata)
    );
    defparam colorBuffer.STREAM_WIDTH = FRAMEBUFFER_STREAM_WIDTH;
    defparam colorBuffer.NUMBER_OF_SUB_PIXELS = COLOR_NUMBER_OF_SUB_PIXEL;
    defparam colorBuffer.NUMBER_OF_SUB_PIXELS_INTERNAL = FRAMEBUFFER_NUMBER_OF_SUB_PIXELS;
    defparam colorBuffer.SUB_PIXEL_WIDTH_INTERNAL = FRAMEBUFFER_SUB_PIXEL_WIDTH;
    defparam colorBuffer.SUB_PIXEL_WIDTH = COLOR_SUB_PIXEL_WIDTH;
    defparam colorBuffer.X_BIT_WIDTH = RENDER_CONFIG_X_SIZE;
    defparam colorBuffer.Y_BIT_WIDTH = RENDER_CONFIG_Y_SIZE;
    defparam colorBuffer.FRAMEBUFFER_SIZE_BYTES = FRAMEBUFFER_SIZE_BYTES;

    Rasterizer rop (
        .clk(aclk), 
        .reset(!resetn), 

        .rasterizerRunning(rasterizerRunning),
        .startRendering(startRendering),

        .yOffset(confYOffset[RENDER_CONFIG_Y_POS +: RENDER_CONFIG_Y_SIZE]),
        .xResolution(confRenderResolution[RENDER_CONFIG_X_POS +: RENDER_CONFIG_X_SIZE]),
        .yResolution(confRenderResolution[RENDER_CONFIG_Y_POS +: RENDER_CONFIG_Y_SIZE]),

        .m_axis_tvalid(m_rasterizer_axis_tvalid),
        .m_axis_tready(m_rasterizer_axis_tready),
        .m_axis_tlast(m_rasterizer_axis_tlast),
        .m_axis_tdata(m_rasterizer_axis_tdata),

        .bbStart(triangleParams[TRIANGLE_STREAM_BB_START * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
        .bbEnd(triangleParams[TRIANGLE_STREAM_BB_END * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
        .w0(triangleParams[TRIANGLE_STREAM_INC_W0 * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
        .w1(triangleParams[TRIANGLE_STREAM_INC_W1 * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
        .w2(triangleParams[TRIANGLE_STREAM_INC_W2 * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
        .w0IncX(triangleParams[TRIANGLE_STREAM_INC_W0_X * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
        .w1IncX(triangleParams[TRIANGLE_STREAM_INC_W1_X * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
        .w2IncX(triangleParams[TRIANGLE_STREAM_INC_W2_X * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
        .w0IncY(triangleParams[TRIANGLE_STREAM_INC_W0_Y * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
        .w1IncY(triangleParams[TRIANGLE_STREAM_INC_W1_Y * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
        .w2IncY(triangleParams[TRIANGLE_STREAM_INC_W2_Y * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE])
    );
    defparam rop.X_BIT_WIDTH = RENDER_CONFIG_X_SIZE;
    defparam rop.Y_BIT_WIDTH = RENDER_CONFIG_Y_SIZE;
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

        .tex0_s(triangleParams[TRIANGLE_STREAM_INC_TEX0_S * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
        .tex0_t(triangleParams[TRIANGLE_STREAM_INC_TEX0_T * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
        .tex0_q(triangleParams[TRIANGLE_STREAM_INC_TEX0_Q * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
        .tex0_s_inc_x(triangleParams[TRIANGLE_STREAM_INC_TEX0_S_X * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
        .tex0_t_inc_x(triangleParams[TRIANGLE_STREAM_INC_TEX0_T_X * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
        .tex0_q_inc_x(triangleParams[TRIANGLE_STREAM_INC_TEX0_Q_X * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
        .tex0_s_inc_y(triangleParams[TRIANGLE_STREAM_INC_TEX0_S_Y * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
        .tex0_t_inc_y(triangleParams[TRIANGLE_STREAM_INC_TEX0_T_Y * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
        .tex0_q_inc_y(triangleParams[TRIANGLE_STREAM_INC_TEX0_Q_Y * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
        .tex1_s(triangleParams[TRIANGLE_STREAM_INC_TEX1_S * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
        .tex1_t(triangleParams[TRIANGLE_STREAM_INC_TEX1_T * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
        .tex1_q(triangleParams[TRIANGLE_STREAM_INC_TEX1_Q * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
        .tex1_s_inc_x(triangleParams[TRIANGLE_STREAM_INC_TEX1_S_X * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
        .tex1_t_inc_x(triangleParams[TRIANGLE_STREAM_INC_TEX1_T_X * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
        .tex1_q_inc_x(triangleParams[TRIANGLE_STREAM_INC_TEX1_Q_X * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
        .tex1_s_inc_y(triangleParams[TRIANGLE_STREAM_INC_TEX1_S_Y * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
        .tex1_t_inc_y(triangleParams[TRIANGLE_STREAM_INC_TEX1_T_Y * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
        .tex1_q_inc_y(triangleParams[TRIANGLE_STREAM_INC_TEX1_Q_Y * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
        .depth_w(triangleParams[TRIANGLE_STREAM_INC_DEPTH_W * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
        .depth_w_inc_x(triangleParams[TRIANGLE_STREAM_INC_DEPTH_W_X * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
        .depth_w_inc_y(triangleParams[TRIANGLE_STREAM_INC_DEPTH_W_Y * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
        .depth_z(triangleParams[TRIANGLE_STREAM_INC_DEPTH_Z * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
        .depth_z_inc_x(triangleParams[TRIANGLE_STREAM_INC_DEPTH_Z_X * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
        .depth_z_inc_y(triangleParams[TRIANGLE_STREAM_INC_DEPTH_Z_Y * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
        .color_r(triangleParams[TRIANGLE_STREAM_INC_COLOR_R * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
        .color_r_inc_x(triangleParams[TRIANGLE_STREAM_INC_COLOR_R_X * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
        .color_r_inc_y(triangleParams[TRIANGLE_STREAM_INC_COLOR_R_Y * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
        .color_g(triangleParams[TRIANGLE_STREAM_INC_COLOR_G * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
        .color_g_inc_x(triangleParams[TRIANGLE_STREAM_INC_COLOR_G_X * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
        .color_g_inc_y(triangleParams[TRIANGLE_STREAM_INC_COLOR_G_Y * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
        .color_b(triangleParams[TRIANGLE_STREAM_INC_COLOR_B * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
        .color_b_inc_x(triangleParams[TRIANGLE_STREAM_INC_COLOR_B_X * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
        .color_b_inc_y(triangleParams[TRIANGLE_STREAM_INC_COLOR_B_Y * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
        .color_a(triangleParams[TRIANGLE_STREAM_INC_COLOR_A * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
        .color_a_inc_x(triangleParams[TRIANGLE_STREAM_INC_COLOR_A_X * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),
        .color_a_inc_y(triangleParams[TRIANGLE_STREAM_INC_COLOR_A_Y * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE])
    );

    PixelPipeline pixelPipeline (    
        .aclk(aclk),
        .resetn(resetn),
        .pixelInPipeline(pixelInPipelineShader),

        .s_fog_lut_axis_tvalid(s_cmd_fog_axis_tvalid),
        .s_fog_lut_axis_tready(s_cmd_fog_axis_tready),
        .s_fog_lut_axis_tlast(s_cmd_xxx_axis_tlast),
        .s_fog_lut_axis_tdata(s_cmd_xxx_axis_tdata),

        .confFeatureEnable(confFeatureEnable),
        .confFragmentPipelineConfig(confFragmentPipelineConfig),
        .confFragmentPipelineFogColor(confFragmentPipelineFogColor),
        .confTMU0TexEnvConfig(confTMU0TexEnvConfig),
        .confTMU0TextureConfig(confTMU0TextureConfig),
        .confTMU0TexEnvColor(confTMU0TexEnvColor),
        .confTMU1TexEnvConfig(confTMU1TexEnvConfig),
        .confTMU1TextureConfig(confTMU1TextureConfig),
        .confTMU1TexEnvColor(confTMU1TexEnvColor),

        .s_axis_tvalid(m_attr_inter_axis_tvalid),
        .s_axis_tready(m_attr_inter_axis_tready),
        .s_axis_tlast(m_attr_inter_axis_tlast),
        .s_axis_tdata(m_attr_inter_axis_tdata),

        .texel0Addr00(texel0Addr00),
        .texel0Addr01(texel0Addr01),
        .texel0Addr10(texel0Addr10),
        .texel0Addr11(texel0Addr11),

        .texel0Input00(texel0Input00),
        .texel0Input01(texel0Input01),
        .texel0Input10(texel0Input10),
        .texel0Input11(texel0Input11),

        .texel1Addr00(texel1Addr00),
        .texel1Addr01(texel1Addr01),
        .texel1Addr10(texel1Addr10),
        .texel1Addr11(texel1Addr11),

        .texel1Input00(texel1Input00),
        .texel1Input01(texel1Input01),
        .texel1Input10(texel1Input10),
        .texel1Input11(texel1Input11),

        .colorIndexRead(colorIndexRead),
        .colorIn(colorIn),
        .colorIndexWrite(colorIndexWrite),
        .colorWriteEnable(colorWriteEnable),
        .colorOut(colorOut),
        .colorOutScreenPosX(colorOutScreenPosX),
        .colorOutScreenPosY(colorOutScreenPosY),

        .depthIndexRead(depthIndexRead),
        .depthIn(depthIn),
        .depthIndexWrite(depthIndexWrite),
        .depthWriteEnable(depthWriteEnable),
        .depthOut(depthOut),
        .depthOutScreenPosX(depthOutScreenPosX),
        .depthOutScreenPosY(depthOutScreenPosY)
    );
    defparam pixelPipeline.FRAMEBUFFER_INDEX_WIDTH = FRAMEBUFFER_INDEX_WIDTH;
    defparam pixelPipeline.CMD_STREAM_WIDTH = CMD_STREAM_WIDTH;
    defparam pixelPipeline.SUB_PIXEL_WIDTH = COLOR_SUB_PIXEL_WIDTH;
    defparam pixelPipeline.ENABLE_SECOND_TMU = ENABLE_SECOND_TMU;

endmodule
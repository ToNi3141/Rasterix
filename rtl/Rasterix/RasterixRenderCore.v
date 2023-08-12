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

`include "PixelUtil.vh"

module RasterixRenderCore #(
    // The size of the internal framebuffer (in power of two)
    // The access is in words, not bytes! A word can have the size of at least 1 bit to n bits
    parameter INDEX_WIDTH = 17,

    // Number of TMUs. Currently supported values: 1 and 2
    parameter TMU_COUNT = 2,
    
    // The bit width of the command stream interface
    // Allowed values: 32, 64, 128, 256 bit
    parameter CMD_STREAM_WIDTH = 16,

    // The size of the texture in bytes in power of two
    parameter TEXTURE_BUFFER_SIZE = 15,

    // The size of a sub pixel
    localparam SUB_PIXEL_WIDTH = 8,

    // The number of sub pixel used for a pixel
    localparam NUMBER_OF_SUB_PIXELS = 4,

    // Pixel width
    localparam PIXEL_WIDTH = NUMBER_OF_SUB_PIXELS * SUB_PIXEL_WIDTH,

    // Stencil width
    localparam STENCIL_WIDTH = 4,

    // Depth width
    localparam DEPTH_WIDTH = 16,

    // Screen pos width
    localparam SCREEN_POS_WIDTH = 11
)
(
    input  wire         aclk,
    input  wire         resetn,

    // AXI Stream command interface
    input  wire         s_cmd_axis_tvalid,
    output wire         s_cmd_axis_tready,
    input  wire         s_cmd_axis_tlast,
    input  wire [CMD_STREAM_WIDTH - 1 : 0]  s_cmd_axis_tdata,

    output wire                                             framebufferParamEnableScissor,
    output wire [SCREEN_POS_WIDTH - 1 : 0]                  framebufferParamScissorStartX,
    output wire [SCREEN_POS_WIDTH - 1 : 0]                  framebufferParamScissorStartY,
    output wire [SCREEN_POS_WIDTH - 1 : 0]                  framebufferParamScissorEndX,
    output wire [SCREEN_POS_WIDTH - 1 : 0]                  framebufferParamScissorEndY,
    output wire [SCREEN_POS_WIDTH - 1 : 0]                  framebufferParamYOffset,
    output wire [SCREEN_POS_WIDTH - 1 : 0]                  framebufferParamXResolution,
    output wire [SCREEN_POS_WIDTH - 1 : 0]                  framebufferParamYResolution,

    // Color buffer access
    output wire [PIXEL_WIDTH - 1 : 0]                       colorBufferClearColor,
    output wire                                             colorBufferApply,
    input  wire                                             colorBufferApplied,
    output wire                                             colorBufferCmdCommit,
    output wire                                             colorBufferCmdMemset,
    output wire [INDEX_WIDTH - 1 : 0]                       colorIndexRead,
    output wire [INDEX_WIDTH - 1 : 0]                       colorIndexWrite,
    output wire                                             colorWriteEnable,
    input  wire [PIXEL_WIDTH - 1 : 0]                       colorIn,
    output wire [PIXEL_WIDTH - 1 : 0]                       colorOut,
    output wire [NUMBER_OF_SUB_PIXELS - 1 : 0]              colorMask,
    output wire [SCREEN_POS_WIDTH - 1 : 0]                  colorOutScreenPosX,
    output wire [SCREEN_POS_WIDTH - 1 : 0]                  colorOutScreenPosY,

    // Depth buffer access
    output wire [DEPTH_WIDTH - 1 : 0]                       depthBufferClearDepth,
    output wire                                             depthBufferApply,
    input  wire                                             depthBufferApplied,
    output wire                                             depthBufferCmdCommit,
    output wire                                             depthBufferCmdMemset,
    output wire [INDEX_WIDTH - 1 : 0]                       depthIndexRead,
    output wire [INDEX_WIDTH - 1 : 0]                       depthIndexWrite,
    output wire                                             depthWriteEnable,
    input  wire [DEPTH_WIDTH - 1 : 0]                       depthIn,
    output wire [DEPTH_WIDTH - 1 : 0]                       depthOut,
    output wire                                             depthMask,
    output wire [SCREEN_POS_WIDTH - 1 : 0]                  depthOutScreenPosX,
    output wire [SCREEN_POS_WIDTH - 1 : 0]                  depthOutScreenPosY,

    // Stencil buffer access
    output wire [STENCIL_WIDTH - 1 : 0]                     stencilBufferClearStencil,
    output wire                                             stencilBufferApply,
    input  wire                                             stencilBufferApplied,
    output wire                                             stencilBufferCmdCommit,
    output wire                                             stencilBufferCmdMemset,
    output wire [INDEX_WIDTH - 1 : 0]                       stencilIndexRead,
    output wire [INDEX_WIDTH - 1 : 0]                       stencilIndexWrite,
    output wire                                             stencilWriteEnable,
    input  wire [STENCIL_WIDTH - 1 : 0]                     stencilIn,
    output wire [STENCIL_WIDTH - 1 : 0]                     stencilOut,
    output wire [STENCIL_WIDTH - 1 : 0]                     stencilMask,
    output wire [SCREEN_POS_WIDTH - 1 : 0]                  stencilOutScreenPosX,
    output wire [SCREEN_POS_WIDTH - 1 : 0]                  stencilOutScreenPosY,

    // Debug
    output wire [ 3:0]  dbgStreamState,
    output wire         dbgRasterizerRunning
);
`include "RasterizerDefines.vh"
`include "RegisterAndDescriptorDefines.vh"
`include "AttributeInterpolatorDefines.vh"


    localparam TEX_ADDR_WIDTH = 16;

    

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

        if ((SCREEN_POS_WIDTH != RENDER_CONFIG_X_SIZE) || (SCREEN_POS_WIDTH != RENDER_CONFIG_Y_SIZE))
        begin
            $error("Screen size width in the RasterixRenderCore and RegisterAndDescriptorDefines are different");
        end

        if (COLOR_SUB_PIXEL_WIDTH != SUB_PIXEL_WIDTH)
        begin
            $error("The sub pixel width in the RasterixRenderCore and RegisterAndDescriptorDefines are different");
        end

        if (COLOR_NUMBER_OF_SUB_PIXEL != NUMBER_OF_SUB_PIXELS)
        begin
            $error("The number of sub pixels in the RasterixRenderCore and RegisterAndDescriptorDefines are different");
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


    // Per fragment wires
    wire [(COLOR_SUB_PIXEL_WIDTH * 4) - 1 : 0]      framebuffer_fragmentColor;
    wire [INDEX_WIDTH - 1 : 0]                      framebuffer_index;
    wire [SCREEN_POS_WIDTH - 1 : 0]                 framebuffer_screenPosX;
    wire [SCREEN_POS_WIDTH - 1 : 0]                 framebuffer_screenPosY;
    wire [31 : 0]                                   framebuffer_depth;
    wire                                            framebuffer_valid;

    wire pixelInPipelineInterpolator;
    wire pixelInPipelineShader;
    wire pixelInPipeline = pixelInPipelineInterpolator || pixelInPipelineShader;
    wire startRendering;

   
    // Control
    wire            rasterizerRunning;

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
    wire [OP_RENDER_CONFIG_REG_WIDTH - 1 : 0]   confStencilBufferConfig;

    assign confFeatureEnable = renderConfigs[OP_RENDER_CONFIG_FEATURE_ENABLE +: OP_RENDER_CONFIG_REG_WIDTH];
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
    assign confStencilBufferConfig = renderConfigs[OP_RENDER_CONFIG_STENCIL_BUFFER +: OP_RENDER_CONFIG_REG_WIDTH];

    assign colorBufferClearColor = renderConfigs[OP_RENDER_CONFIG_COLOR_BUFFER_CLEAR_COLOR +: OP_RENDER_CONFIG_REG_WIDTH];
    assign depthBufferClearDepth = renderConfigs[OP_RENDER_CONFIG_DEPTH_BUFFER_CLEAR_DEPTH +: RENDER_CONFIG_CLEAR_DEPTH_SIZE - (RENDER_CONFIG_CLEAR_DEPTH_SIZE - DEPTH_WIDTH)];
    assign stencilBufferClearStencil = confStencilBufferConfig[RENDER_CONFIG_STENCIL_BUFFER_CLEAR_STENICL_POS +: RENDER_CONFIG_STENCIL_BUFFER_CLEAR_STENICL_SIZE - (RENDER_CONFIG_STENCIL_BUFFER_CLEAR_STENICL_SIZE - STENCIL_WIDTH)];

    assign framebufferParamEnableScissor = confFeatureEnable[RENDER_CONFIG_FEATURE_ENABLE_SCISSOR_POS];
    assign framebufferParamScissorStartX = confScissorStartXY[RENDER_CONFIG_X_POS +: RENDER_CONFIG_X_SIZE];
    assign framebufferParamScissorStartY = confScissorStartXY[RENDER_CONFIG_Y_POS +: RENDER_CONFIG_Y_SIZE];
    assign framebufferParamScissorEndX = confScissorEndXY[RENDER_CONFIG_X_POS +: RENDER_CONFIG_X_SIZE];
    assign framebufferParamScissorEndY = confScissorEndXY[RENDER_CONFIG_Y_POS +: RENDER_CONFIG_Y_SIZE];
    assign framebufferParamYOffset = confYOffset[RENDER_CONFIG_Y_POS +: RENDER_CONFIG_Y_SIZE];
    assign framebufferParamXResolution = confRenderResolution[RENDER_CONFIG_X_POS +: RENDER_CONFIG_X_SIZE];
    assign framebufferParamYResolution = confRenderResolution[RENDER_CONFIG_Y_POS +: RENDER_CONFIG_Y_SIZE];
    assign depthMask = confFragmentPipelineConfig[RENDER_CONFIG_FRAGMENT_DEPTH_MASK_POS +: RENDER_CONFIG_FRAGMENT_DEPTH_MASK_SIZE];
    assign colorMask = { 
                    confFragmentPipelineConfig[RENDER_CONFIG_FRAGMENT_COLOR_MASK_R_POS +: RENDER_CONFIG_FRAGMENT_COLOR_MASK_R_SIZE], 
                    confFragmentPipelineConfig[RENDER_CONFIG_FRAGMENT_COLOR_MASK_G_POS +: RENDER_CONFIG_FRAGMENT_COLOR_MASK_G_SIZE], 
                    confFragmentPipelineConfig[RENDER_CONFIG_FRAGMENT_COLOR_MASK_B_POS +: RENDER_CONFIG_FRAGMENT_COLOR_MASK_B_SIZE], 
                    confFragmentPipelineConfig[RENDER_CONFIG_FRAGMENT_COLOR_MASK_A_POS +: RENDER_CONFIG_FRAGMENT_COLOR_MASK_A_SIZE] };
    assign stencilMask = confStencilBufferConfig[RENDER_CONFIG_STENCIL_BUFFER_STENICL_MASK_POS +: RENDER_CONFIG_STENCIL_BUFFER_STENICL_MASK_SIZE - (RENDER_CONFIG_STENCIL_BUFFER_STENICL_MASK_SIZE - STENCIL_WIDTH)];


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
        .stencilBufferApply(stencilBufferApply),
        .stencilBufferApplied(stencilBufferApplied),
        .stencilBufferCmdCommit(stencilBufferCmdCommit),
        .stencilBufferCmdMemset(stencilBufferCmdMemset),

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

    ////////////////////////////////////////////////////////////////////////////
    // STEP 1
    // Rasterizing the triangle
    // Clocks: n/a
    ////////////////////////////////////////////////////////////////////////////
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
    defparam rop.FRAMEBUFFER_INDEX_WIDTH = INDEX_WIDTH;
    defparam rop.CMD_STREAM_WIDTH = CMD_STREAM_WIDTH;

    wire fragmentProcessed;
    ValueTrack pixelTracker (
        .aclk(aclk),
        .resetn(resetn),
        
        .sigIncommingValue(m_attr_inter_axis_tvalid & m_attr_inter_axis_tready),
        .sigOutgoingValue(fragmentProcessed),
        .valueInPipeline(pixelInPipelineShader)
    );
    
    ////////////////////////////////////////////////////////////////////////////
    // STEP 2
    // Interpolation of attributes
    // Clocks: 45
    ////////////////////////////////////////////////////////////////////////////
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

    ////////////////////////////////////////////////////////////////////////////
    // STEP 3
    // Texturing triangle, fogging
    // Clocks: 32
    ////////////////////////////////////////////////////////////////////////////
    // TODO: Es wird aktuell noch nicht tlast durch geroutet
    PixelPipeline pixelPipeline (    
        .aclk(aclk),
        .resetn(resetn),

        .s_fog_lut_axis_tvalid(s_cmd_fog_axis_tvalid),
        .s_fog_lut_axis_tready(s_cmd_fog_axis_tready),
        .s_fog_lut_axis_tlast(s_cmd_xxx_axis_tlast),
        .s_fog_lut_axis_tdata(s_cmd_xxx_axis_tdata),

        .confFeatureEnable(confFeatureEnable),
        .confFragmentPipelineConfig(confFragmentPipelineConfig),
        .confFragmentPipelineFogColor(confFragmentPipelineFogColor),
        .confStencilBufferConfig(confStencilBufferConfig),
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

        .framebuffer_valid(framebuffer_valid),
        .framebuffer_fragmentColor(framebuffer_fragmentColor),
        .framebuffer_depth(framebuffer_depth),
        .framebuffer_index(framebuffer_index),
        .framebuffer_screenPosX(framebuffer_screenPosX),
        .framebuffer_screenPosY(framebuffer_screenPosY)
    );
    defparam pixelPipeline.FRAMEBUFFER_INDEX_WIDTH = INDEX_WIDTH;
    defparam pixelPipeline.CMD_STREAM_WIDTH = CMD_STREAM_WIDTH;
    defparam pixelPipeline.SUB_PIXEL_WIDTH = COLOR_SUB_PIXEL_WIDTH;
    defparam pixelPipeline.ENABLE_SECOND_TMU = ENABLE_SECOND_TMU;
    defparam pixelPipeline.STENCIL_WIDTH = STENCIL_WIDTH;
    defparam pixelPipeline.DEPTH_WIDTH = DEPTH_WIDTH;
    defparam pixelPipeline.SCREEN_POS_WIDTH = SCREEN_POS_WIDTH;

    ////////////////////////////////////////////////////////////////////////////
    // STEP 4
    // Access framebuffer, blend, test and save pixel in framebuffer
    // Clocks: 5
    ////////////////////////////////////////////////////////////////////////////
    PerFragmentPipeline perFragmentPipeline (
        .aclk(aclk),
        .resetn(resetn),

        .conf(confFragmentPipelineConfig),
        .confFeatureEnable(confFeatureEnable),
        .confStencilBufferConfig(confStencilBufferConfig),

        .valid(framebuffer_valid),
        .fragmentColor(framebuffer_fragmentColor),
        .depth(framebuffer_depth),
        .index(framebuffer_index),
        .screenPosX(framebuffer_screenPosX),
        .screenPosY(framebuffer_screenPosY),

        .fragmentProcessed(fragmentProcessed),

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
        .depthOutScreenPosY(depthOutScreenPosY),

        .stencilIndexRead(stencilIndexRead),
        .stencilIn(stencilIn),
        .stencilIndexWrite(stencilIndexWrite),
        .stencilWriteEnable(stencilWriteEnable),
        .stencilOut(stencilOut),
        .stencilOutScreenPosX(stencilOutScreenPosX),
        .stencilOutScreenPosY(stencilOutScreenPosY)
    );
    defparam perFragmentPipeline.FRAMEBUFFER_INDEX_WIDTH = INDEX_WIDTH;
    defparam perFragmentPipeline.SCREEN_POS_WIDTH = SCREEN_POS_WIDTH;
    defparam perFragmentPipeline.DEPTH_WIDTH = DEPTH_WIDTH;
    defparam perFragmentPipeline.STENCIL_WIDTH = STENCIL_WIDTH;
    defparam perFragmentPipeline.SUB_PIXEL_WIDTH = COLOR_SUB_PIXEL_WIDTH;
endmodule
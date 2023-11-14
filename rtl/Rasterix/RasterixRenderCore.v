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

    // The width of the address channel
    parameter ADDR_WIDTH = 32,

    // Number of TMUs. Currently supported values: 1 and 2
    parameter TMU_COUNT = 2,
    
    // The bit width of the command stream interface
    // Allowed values: 32, 64, 128, 256 bit
    parameter CMD_STREAM_WIDTH = 16,

    // The size of the texture in bytes in power of two
    parameter TEXTURE_BUFFER_SIZE = 15,

    // Enables the flow control. Disabling can safe logic resources.
    parameter ENABLE_FLOW_CTRL = 1,

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
    localparam SCREEN_POS_WIDTH = 11,

    // Number of pixel the pipeline can maximal contain until a stall event occures in power of two
    localparam MAX_NUMBER_OF_PIXELS_LG = 7
)
(
    input  wire                                 aclk,
    input  wire                                 resetn,

    // AXI Stream command interface
    input  wire                                 s_cmd_axis_tvalid,
    output wire                                 s_cmd_axis_tready,
    input  wire                                 s_cmd_axis_tlast,
    input  wire [CMD_STREAM_WIDTH - 1 : 0]      s_cmd_axis_tdata,

    output wire                                 framebufferParamEnableScissor,
    output wire [SCREEN_POS_WIDTH - 1 : 0]      framebufferParamScissorStartX,
    output wire [SCREEN_POS_WIDTH - 1 : 0]      framebufferParamScissorStartY,
    output wire [SCREEN_POS_WIDTH - 1 : 0]      framebufferParamScissorEndX,
    output wire [SCREEN_POS_WIDTH - 1 : 0]      framebufferParamScissorEndY,
    output wire [SCREEN_POS_WIDTH - 1 : 0]      framebufferParamYOffset,
    output wire [SCREEN_POS_WIDTH - 1 : 0]      framebufferParamXResolution,
    output wire [SCREEN_POS_WIDTH - 1 : 0]      framebufferParamYResolution,

    // Color buffer access
    output wire [PIXEL_WIDTH - 1 : 0]           colorBufferClearColor,
    output wire [ADDR_WIDTH - 1 : 0]            colorBufferAddr,
    output wire                                 colorBufferApply,
    input  wire                                 colorBufferApplied,
    output wire                                 colorBufferCmdCommit,
    output wire                                 colorBufferCmdMemset,
    output wire                                 colorBufferEnable,
    output wire [NUMBER_OF_SUB_PIXELS - 1 : 0]  colorBufferMask,
    input  wire                                 m_color_arready,
    output wire                                 m_color_arvalid,
    output wire                                 m_color_arlast,
    output wire [INDEX_WIDTH - 1 : 0]           m_color_araddr,
    output wire                                 m_color_rready,
    input  wire                                 m_color_rvalid,
    input  wire [PIXEL_WIDTH - 1 : 0]           m_color_rdata,
    output wire [INDEX_WIDTH - 1 : 0]           m_color_waddr,
    output wire                                 m_color_wvalid,
    input  wire                                 m_color_wready,
    output wire                                 m_color_wlast,
    output wire [PIXEL_WIDTH - 1 : 0]           m_color_wdata,
    output wire                                 m_color_wstrb,
    output wire [SCREEN_POS_WIDTH - 1 : 0]      m_color_wscreenPosX,
    output wire [SCREEN_POS_WIDTH - 1 : 0]      m_color_wscreenPosY,

    // Depth buffer access
    output wire [DEPTH_WIDTH - 1 : 0]           depthBufferClearDepth,
    output wire [ADDR_WIDTH - 1 : 0]            depthBufferAddr,
    output wire                                 depthBufferApply,
    input  wire                                 depthBufferApplied,
    output wire                                 depthBufferCmdCommit,
    output wire                                 depthBufferCmdMemset,
    output wire                                 depthBufferEnable,
    output wire                                 depthBufferMask,
    input  wire                                 m_depth_arready,
    output wire                                 m_depth_arvalid,
    output wire                                 m_depth_arlast,
    output wire [INDEX_WIDTH - 1 : 0]           m_depth_araddr,
    output wire                                 m_depth_rready,
    input  wire                                 m_depth_rvalid,
    input  wire [DEPTH_WIDTH - 1 : 0]           m_depth_rdata,
    output wire [INDEX_WIDTH - 1 : 0]           m_depth_waddr,
    output wire                                 m_depth_wvalid,
    input  wire                                 m_depth_wready,
    output wire                                 m_depth_wlast,
    output wire [DEPTH_WIDTH - 1 : 0]           m_depth_wdata,
    output wire                                 m_depth_wstrb,
    output wire [SCREEN_POS_WIDTH - 1 : 0]      m_depth_wscreenPosX,
    output wire [SCREEN_POS_WIDTH - 1 : 0]      m_depth_wscreenPosY,

    // Stencil buffer access
    output wire [STENCIL_WIDTH - 1 : 0]         stencilBufferClearStencil,
    output wire [ADDR_WIDTH - 1 : 0]            stencilBufferAddr,
    output wire                                 stencilBufferApply,
    input  wire                                 stencilBufferApplied,
    output wire                                 stencilBufferCmdCommit,
    output wire                                 stencilBufferCmdMemset,
    output wire                                 stencilBufferEnable,
    output wire [STENCIL_WIDTH - 1 : 0]         stencilBufferMask,
    input  wire                                 m_stencil_arready,
    output wire                                 m_stencil_arvalid,
    output wire                                 m_stencil_arlast,
    output wire [INDEX_WIDTH - 1 : 0]           m_stencil_araddr,
    output wire                                 m_stencil_rready,
    input  wire                                 m_stencil_rvalid,
    input  wire [STENCIL_WIDTH - 1 : 0]         m_stencil_rdata,
    output wire [INDEX_WIDTH - 1 : 0]           m_stencil_waddr,
    output wire                                 m_stencil_wvalid,
    input  wire                                 m_stencil_wready,
    output wire                                 m_stencil_wlast,
    output wire [STENCIL_WIDTH - 1 : 0]         m_stencil_wdata,
    output wire                                 m_stencil_wstrb,
    output wire [SCREEN_POS_WIDTH - 1 : 0]      m_stencil_wscreenPosX,
    output wire [SCREEN_POS_WIDTH - 1 : 0]      m_stencil_wscreenPosY,

    // Debug
    output wire [ 3 : 0]                        dbgStreamState,
    output wire                                 dbgRasterizerRunning
);
`include "RegisterAndDescriptorDefines.vh"

    localparam TEX_ADDR_WIDTH = 16;
    localparam ATTRIBUTE_SIZE = 32;
    
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
    
    ////////////////////////////////////////////////////////////////////////////
    // Command Parser
    // Execution unit for commands from the command interface
    // Clocks: n/a
    ////////////////////////////////////////////////////////////////////////////
    wire [CMD_STREAM_WIDTH - 1 : 0]  cmd_xxx_axis_tdata;
    wire [ 4 : 0]    cmd_xxx_axis_tuser;
    wire             cmd_xxx_axis_tlast;
    wire             cmd_fog_axis_tvalid;
    wire             cmd_rasterizer_axis_tvalid;
    wire             cmd_tmu0_axis_tvalid;
    wire             cmd_tmu1_axis_tvalid;
    wire             cmd_config_axis_tvalid;
    wire             cmd_fog_axis_tready;
    wire             cmd_rasterizer_axis_tready;
    wire             cmd_tmu0_axis_tready;
    wire             cmd_tmu1_axis_tready;
    wire             cmd_config_axis_tready;
    // Control
    wire             pipelineEmpty;
    wire             startRendering;
    wire             color_fifo_empty;
    wire             depth_fifo_empty;
    wire             stencil_fifo_empty;
    wire             rasterizerRunning;
    CommandParser commandParser(
        .aclk(aclk),
        .resetn(resetn),

        // AXI Stream command interface
        .s_cmd_axis_tvalid(s_cmd_axis_tvalid),
        .s_cmd_axis_tready(s_cmd_axis_tready),
        .s_cmd_axis_tlast(s_cmd_axis_tlast),
        .s_cmd_axis_tdata(s_cmd_axis_tdata),

        .m_cmd_xxx_axis_tdata(cmd_xxx_axis_tdata),
        .m_cmd_xxx_axis_tuser(cmd_xxx_axis_tuser),
        .m_cmd_xxx_axis_tlast(cmd_xxx_axis_tlast),
        .m_cmd_fog_axis_tvalid(cmd_fog_axis_tvalid),
        .m_cmd_rasterizer_axis_tvalid(cmd_rasterizer_axis_tvalid),
        .m_cmd_tmu0_axis_tvalid(cmd_tmu0_axis_tvalid),
        .m_cmd_tmu1_axis_tvalid(cmd_tmu1_axis_tvalid),
        .m_cmd_config_axis_tvalid(cmd_config_axis_tvalid),
        .m_cmd_fog_axis_tready(cmd_fog_axis_tready),
        .m_cmd_rasterizer_axis_tready(cmd_rasterizer_axis_tready),
        .m_cmd_tmu0_axis_tready(cmd_tmu0_axis_tready),
        .m_cmd_tmu1_axis_tready(cmd_tmu1_axis_tready),
        .m_cmd_config_axis_tready(cmd_config_axis_tready),

        // Rasterizer
        // Control
        .rasterizerRunning(rasterizerRunning),
        .startRendering(startRendering),
        .pixelInPipeline(!pipelineEmpty || !color_fifo_empty || !depth_fifo_empty || !stencil_fifo_empty),

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
    assign dbgRasterizerRunning = rasterizerRunning;

    ////////////////////////////////////////////////////////////////////////////
    // Register Bank for the triangle parameters
    // Clocks: n/a
    ////////////////////////////////////////////////////////////////////////////
    wire [(TRIANGLE_STREAM_PARAM_SIZE * `GET_TRIANGLE_SIZE_FOR_BUS_WIDTH(CMD_STREAM_WIDTH)) - 1 : 0] triangleParams;
    RegisterBank triangleParameters (
        .aclk(aclk),
        .resetn(resetn),

        .s_axis_tvalid(cmd_rasterizer_axis_tvalid),
        .s_axis_tready(cmd_rasterizer_axis_tready),
        .s_axis_tlast(cmd_xxx_axis_tlast),
        .s_axis_tdata(cmd_xxx_axis_tdata),
        .s_axis_tuser(0),

        .registers(triangleParams)
    );
    defparam triangleParameters.BANK_SIZE = `GET_TRIANGLE_SIZE_FOR_BUS_WIDTH(CMD_STREAM_WIDTH);
    defparam triangleParameters.CMD_STREAM_WIDTH = CMD_STREAM_WIDTH;
    
    ////////////////////////////////////////////////////////////////////////////
    // Register Bank for the render configs
    // Clocks: n/a
    ////////////////////////////////////////////////////////////////////////////
    wire [(OP_RENDER_CONFIG_REG_WIDTH * OP_RENDER_CONFIG_NUMBER_OR_REGS) - 1 : 0] renderConfigs;
    RegisterBank renderConfigsRegBank (
        .aclk(aclk),
        .resetn(resetn),

        .s_axis_tvalid(cmd_config_axis_tvalid),
        .s_axis_tready(cmd_config_axis_tready),
        .s_axis_tlast(cmd_xxx_axis_tlast),
        .s_axis_tdata(cmd_xxx_axis_tdata),
        .s_axis_tuser(cmd_xxx_axis_tuser),

        .registers(renderConfigs)
    );
    defparam renderConfigsRegBank.BANK_SIZE = OP_RENDER_CONFIG_NUMBER_OR_REGS;
    defparam renderConfigsRegBank.CMD_STREAM_WIDTH = CMD_STREAM_WIDTH;
    defparam renderConfigsRegBank.COMPRESSED = 0;

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
    assign colorBufferAddr = renderConfigs[OP_RENDER_CONFIG_COLOR_BUFFER_ADDR +: OP_RENDER_CONFIG_REG_WIDTH];
    assign depthBufferAddr = renderConfigs[OP_RENDER_CONFIG_DEPTH_BUFFER_ADDR +: OP_RENDER_CONFIG_REG_WIDTH];
    assign stencilBufferAddr = renderConfigs[OP_RENDER_CONFIG_STENCIL_BUFFER_ADDR +: OP_RENDER_CONFIG_REG_WIDTH];
    assign framebufferParamEnableScissor = confFeatureEnable[RENDER_CONFIG_FEATURE_ENABLE_SCISSOR_POS];
    assign framebufferParamScissorStartX = confScissorStartXY[RENDER_CONFIG_X_POS +: RENDER_CONFIG_X_SIZE];
    assign framebufferParamScissorStartY = confScissorStartXY[RENDER_CONFIG_Y_POS +: RENDER_CONFIG_Y_SIZE];
    assign framebufferParamScissorEndX = confScissorEndXY[RENDER_CONFIG_X_POS +: RENDER_CONFIG_X_SIZE];
    assign framebufferParamScissorEndY = confScissorEndXY[RENDER_CONFIG_Y_POS +: RENDER_CONFIG_Y_SIZE];
    assign framebufferParamYOffset = confYOffset[RENDER_CONFIG_Y_POS +: RENDER_CONFIG_Y_SIZE];
    assign framebufferParamXResolution = confRenderResolution[RENDER_CONFIG_X_POS +: RENDER_CONFIG_X_SIZE];
    assign framebufferParamYResolution = confRenderResolution[RENDER_CONFIG_Y_POS +: RENDER_CONFIG_Y_SIZE];
    assign depthBufferEnable = confFeatureEnable[RENDER_CONFIG_FEATURE_ENABLE_DEPTH_TEST_POS];
    assign depthBufferMask = confFragmentPipelineConfig[RENDER_CONFIG_FRAGMENT_DEPTH_MASK_POS +: RENDER_CONFIG_FRAGMENT_DEPTH_MASK_SIZE];
    assign colorBufferEnable = 1'b1;
    assign colorBufferMask = { 
                    confFragmentPipelineConfig[RENDER_CONFIG_FRAGMENT_COLOR_MASK_R_POS +: RENDER_CONFIG_FRAGMENT_COLOR_MASK_R_SIZE], 
                    confFragmentPipelineConfig[RENDER_CONFIG_FRAGMENT_COLOR_MASK_G_POS +: RENDER_CONFIG_FRAGMENT_COLOR_MASK_G_SIZE], 
                    confFragmentPipelineConfig[RENDER_CONFIG_FRAGMENT_COLOR_MASK_B_POS +: RENDER_CONFIG_FRAGMENT_COLOR_MASK_B_SIZE], 
                    confFragmentPipelineConfig[RENDER_CONFIG_FRAGMENT_COLOR_MASK_A_POS +: RENDER_CONFIG_FRAGMENT_COLOR_MASK_A_SIZE] };
    assign stencilBufferEnable = confFeatureEnable[RENDER_CONFIG_FEATURE_ENABLE_STENCIL_TEST_POS];
    assign stencilBufferMask = confStencilBufferConfig[RENDER_CONFIG_STENCIL_BUFFER_STENICL_MASK_POS +: RENDER_CONFIG_STENCIL_BUFFER_STENICL_MASK_SIZE - (RENDER_CONFIG_STENCIL_BUFFER_STENICL_MASK_SIZE - STENCIL_WIDTH)];

    ////////////////////////////////////////////////////////////////////////////
    // Texture Mapping Unit Buffer 0
    // Memory area where the texture is stored
    // Clocks: n/a
    ////////////////////////////////////////////////////////////////////////////
    wire [TEX_ADDR_WIDTH - 1 : 0]   texel0Addr00;
    wire [TEX_ADDR_WIDTH - 1 : 0]   texel0Addr01;
    wire [TEX_ADDR_WIDTH - 1 : 0]   texel0Addr10;
    wire [TEX_ADDR_WIDTH - 1 : 0]   texel0Addr11;
    wire [31 : 0]                   texel0Input00;
    wire [31 : 0]                   texel0Input01;
    wire [31 : 0]                   texel0Input10;
    wire [31 : 0]                   texel0Input11;
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

        .s_axis_tvalid(cmd_tmu0_axis_tvalid),
        .s_axis_tready(cmd_tmu0_axis_tready),
        .s_axis_tlast(cmd_xxx_axis_tlast),
        .s_axis_tdata(cmd_xxx_axis_tdata)
    );
    defparam textureBufferTMU0.STREAM_WIDTH = TEXTURE_STREAM_WIDTH;
    defparam textureBufferTMU0.SIZE = TEXTURE_BUFFER_SIZE;
    defparam textureBufferTMU0.PIXEL_WIDTH = COLOR_NUMBER_OF_SUB_PIXEL * COLOR_SUB_PIXEL_WIDTH;

    ////////////////////////////////////////////////////////////////////////////
    // Texture Mapping Unit Buffer 1
    // Memory area where the texture is stored
    // Clocks: n/a
    ////////////////////////////////////////////////////////////////////////////
    wire [TEX_ADDR_WIDTH - 1 : 0]   texel1Addr00;
    wire [TEX_ADDR_WIDTH - 1 : 0]   texel1Addr01;
    wire [TEX_ADDR_WIDTH - 1 : 0]   texel1Addr10;
    wire [TEX_ADDR_WIDTH - 1 : 0]   texel1Addr11;
    wire [31 : 0]                   texel1Input00;
    wire [31 : 0]                   texel1Input01;
    wire [31 : 0]                   texel1Input10;
    wire [31 : 0]                   texel1Input11;
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

                .s_axis_tvalid(cmd_tmu1_axis_tvalid),
                .s_axis_tready(cmd_tmu1_axis_tready),
                .s_axis_tlast(cmd_xxx_axis_tlast),
                .s_axis_tdata(cmd_xxx_axis_tdata)
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
            assign cmd_tmu1_axis_tready = 1;
        end
    endgenerate

    ////////////////////////////////////////////////////////////////////////////
    // STEP 0
    // Rasterizing the triangle
    // Clocks: n/a
    ////////////////////////////////////////////////////////////////////////////
    wire                            rasterizer_tvalid;
    wire                            rasterizer_tready;
    wire                            rasterizer_tlast;
    wire                            rasterizer_tkeep;
    wire [SCREEN_POS_WIDTH - 1 : 0] rasterizer_tbbx;
    wire [SCREEN_POS_WIDTH - 1 : 0] rasterizer_tbby;
    wire [SCREEN_POS_WIDTH - 1 : 0] rasterizer_tspx;
    wire [SCREEN_POS_WIDTH - 1 : 0] rasterizer_tspy;
    wire [INDEX_WIDTH - 1 : 0]      rasterizer_tindex;

    Rasterizer rop (
        .clk(aclk), 
        .reset(!resetn), 

        .rasterizerRunning(rasterizerRunning),
        .startRendering(startRendering),

        .yOffset(confYOffset[RENDER_CONFIG_Y_POS +: RENDER_CONFIG_Y_SIZE]),
        .xResolution(confRenderResolution[RENDER_CONFIG_X_POS +: SCREEN_POS_WIDTH]),
        .yResolution(confRenderResolution[RENDER_CONFIG_Y_POS +: SCREEN_POS_WIDTH]),

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
        .w2IncY(triangleParams[TRIANGLE_STREAM_INC_W2_Y * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),

        .m_rr_tvalid(rasterizer_tvalid),
        .m_rr_tready(rasterizer_tready),
        .m_rr_tlast(rasterizer_tlast),
        .m_rr_tkeep(rasterizer_tkeep),
        .m_rr_tbbx(rasterizer_tbbx),
        .m_rr_tbby(rasterizer_tbby),
        .m_rr_tspx(rasterizer_tspx),
        .m_rr_tspy(rasterizer_tspy),
        .m_rr_tindex(rasterizer_tindex)
    );
    defparam rop.X_BIT_WIDTH = SCREEN_POS_WIDTH;
    defparam rop.Y_BIT_WIDTH = SCREEN_POS_WIDTH;
    defparam rop.INDEX_WIDTH = INDEX_WIDTH;
    defparam rop.CMD_STREAM_WIDTH = CMD_STREAM_WIDTH;

    ////////////////////////////////////////////////////////////////////////////
    // STEP 1
    // Implementation of the flow control via a stream semaphore and stream barrier
    // In combination with the StreamBarrier and StreamSemaphore, it prevents 
    // overflows of the write FIFOs by ensuring that the pipeline contains a maximum  
    // of MAX_NUMBER_OF_PIXELS_LG pixels. The StreamBarrier starts to stall,
    // as soon as the fill level of FIFOs are exceeding MAX_NUMBER_OF_PIXELS_LG.
    // Both in combination ensuring that only a maximum of MAX_NUMBER_OF_PIXELS_LG + 1
    // can be on the fly. That means for the write FIFOs: They require a minimum 
    // size of MAX_NUMBER_OF_PIXELS_LG + 1.
    // Clocks: 2
    ////////////////////////////////////////////////////////////////////////////
    wire                            rasterizer_sem_tvalid;
    wire                            rasterizer_sem_tlast;
    wire                            rasterizer_sem_tkeep;
    wire                            rasterizer_sem_tready;
    wire [SCREEN_POS_WIDTH - 1 : 0] rasterizer_sem_tbbx;
    wire [SCREEN_POS_WIDTH - 1 : 0] rasterizer_sem_tbby;
    wire [SCREEN_POS_WIDTH - 1 : 0] rasterizer_sem_tspx;
    wire [SCREEN_POS_WIDTH - 1 : 0] rasterizer_sem_tspy;
    wire [INDEX_WIDTH - 1 : 0]      rasterizer_sem_tindex;
    wire                            rasterizer_sbr_tvalid;
    wire                            rasterizer_sbr_tready;
    wire                            rasterizer_sbr_tlast;
    wire                            rasterizer_sbr_tkeep;
    wire [SCREEN_POS_WIDTH - 1 : 0] rasterizer_sbr_tbbx;
    wire [SCREEN_POS_WIDTH - 1 : 0] rasterizer_sbr_tbby;
    wire [SCREEN_POS_WIDTH - 1 : 0] rasterizer_sbr_tspx;
    wire [SCREEN_POS_WIDTH - 1 : 0] rasterizer_sbr_tspy;
    wire [INDEX_WIDTH - 1 : 0]      rasterizer_sbr_tindex;

    wire [MAX_NUMBER_OF_PIXELS_LG + 1 : 0]  color_fifo_fill;
    wire [MAX_NUMBER_OF_PIXELS_LG + 1 : 0]  depth_fifo_fill;
    wire [MAX_NUMBER_OF_PIXELS_LG + 1 : 0]  stencil_fifo_fill;
    wire                                    fifosAlmostFull;
    wire                                    fragmentProcessed;

    StreamSemaphore ssem (
        .aclk(aclk),
        .resetn(resetn),

        .s_axis_tvalid(rasterizer_tvalid),
        .s_axis_tready(rasterizer_tready),
        .s_axis_tlast(rasterizer_tlast),
        .s_axis_tdata({
            rasterizer_tbbx,
            rasterizer_tbby,
            rasterizer_tspx,
            rasterizer_tspy,
            rasterizer_tindex
        }),
        .s_axis_tkeep(rasterizer_tkeep),

        .m_axis_tvalid(rasterizer_sem_tvalid),
        .m_axis_tready(rasterizer_sem_tready),
        .m_axis_tlast(rasterizer_sem_tlast),
        .m_axis_tdata({
            rasterizer_sem_tbbx,
            rasterizer_sem_tbby,
            rasterizer_sem_tspx,
            rasterizer_sem_tspy,
            rasterizer_sem_tindex
        }),
        .m_axis_tkeep(rasterizer_sem_tkeep),

        .sigRelease(fragmentProcessed),
        .released(pipelineEmpty)
    );
    defparam ssem.MAX_NUMBER_OF_ELEMENTS = 2 ** MAX_NUMBER_OF_PIXELS_LG;
    defparam ssem.STREAM_WIDTH = (4 * SCREEN_POS_WIDTH) + INDEX_WIDTH;
    defparam ssem.KEEP_WIDTH = 1;

    generate
        if (ENABLE_FLOW_CTRL)
        begin
            assign fifosAlmostFull = (color_fifo_fill >= (2 ** MAX_NUMBER_OF_PIXELS_LG))
                || (depth_fifo_fill >= (2 ** MAX_NUMBER_OF_PIXELS_LG))
                || (stencil_fifo_fill >= (2 ** MAX_NUMBER_OF_PIXELS_LG));

            // Used to prevent overflows of the write fifos
            StreamBarrier sbr (
                .aclk(aclk),
                .resetn(resetn),

                .s_axis_tvalid(rasterizer_sem_tvalid),
                .s_axis_tready(rasterizer_sem_tready),
                .s_axis_tlast(rasterizer_sem_tlast),
                .s_axis_tdata({
                    rasterizer_sem_tbbx,
                    rasterizer_sem_tbby,
                    rasterizer_sem_tspx,
                    rasterizer_sem_tspy,
                    rasterizer_sem_tindex
                }),
                .s_axis_tkeep(rasterizer_sem_tkeep),

                .m_axis_tvalid(rasterizer_sbr_tvalid),
                .m_axis_tready(rasterizer_sbr_tready),
                .m_axis_tlast(rasterizer_sbr_tlast),
                .m_axis_tdata({
                    rasterizer_sbr_tbbx,
                    rasterizer_sbr_tbby,
                    rasterizer_sbr_tspx,
                    rasterizer_sbr_tspy,
                    rasterizer_sbr_tindex
                }),
                .m_axis_tkeep(rasterizer_sbr_tkeep),

                .stall(fifosAlmostFull)
            );
            defparam sbr.STREAM_WIDTH = (4 * SCREEN_POS_WIDTH) + INDEX_WIDTH;
            defparam sbr.KEEP_WIDTH = 1;
        end
        else
        begin
            assign rasterizer_sbr_tvalid = rasterizer_sem_tvalid;
            assign rasterizer_sem_tready = rasterizer_sbr_tready;
            assign rasterizer_sbr_tlast = rasterizer_sem_tlast;
            assign rasterizer_sbr_tbbx = rasterizer_sem_tbbx;
            assign rasterizer_sbr_tbby = rasterizer_sem_tbby;
            assign rasterizer_sbr_tspx = rasterizer_sem_tspx;
            assign rasterizer_sbr_tspy = rasterizer_sem_tspy;
            assign rasterizer_sbr_tindex = rasterizer_sem_tindex;
            assign rasterizer_sbr_tkeep = rasterizer_sem_tkeep;
        end
    endgenerate
    
    ////////////////////////////////////////////////////////////////////////////
    // STEP 2
    // Broadcaster to broadcast the stream from the rasterizer to the attribute
    // interpolator and to all framebuffers
    // Clocks: n
    ////////////////////////////////////////////////////////////////////////////
    wire [(SCREEN_POS_WIDTH * 4) - 1 : 0]   bc_bbx;
    wire [(SCREEN_POS_WIDTH * 4) - 1 : 0]   bc_bby;
    wire [(SCREEN_POS_WIDTH * 4) - 1 : 0]   bc_spx;
    wire [(SCREEN_POS_WIDTH * 4) - 1 : 0]   bc_spy;
    wire [(INDEX_WIDTH * 4) - 1 : 0]        bc_index;
    wire [ 3 : 0]                           bc_valid;
    wire [ 3 : 0]                           bc_last;
    wire [ 3 : 0]                           bc_keep;
    wire                                    mem_arready_attrib;
    assign m_color_araddr     = bc_index[INDEX_WIDTH * 3 +: INDEX_WIDTH];
    assign m_depth_araddr     = bc_index[INDEX_WIDTH * 2 +: INDEX_WIDTH];
    assign m_stencil_araddr   = bc_index[INDEX_WIDTH * 1 +: INDEX_WIDTH];
    assign m_color_arvalid    = bc_valid[3] & colorBufferEnable;
    assign m_depth_arvalid    = bc_valid[2] & depthBufferEnable;
    assign m_stencil_arvalid  = bc_valid[1] & stencilBufferEnable;
    assign m_color_arlast     = bc_last[3];
    assign m_depth_arlast     = bc_last[2];
    assign m_stencil_arlast   = bc_last[1];

    axis_broadcast axisBroadcast (
        .clk(aclk),
        .rst(!resetn),

        .s_axis_tdata({
            rasterizer_sbr_tbbx,
            rasterizer_sbr_tbby,
            rasterizer_sbr_tspx,
            rasterizer_sbr_tspy,
            rasterizer_sbr_tindex
        }),
        .s_axis_tlast(rasterizer_sbr_tlast),
        .s_axis_tvalid(rasterizer_sbr_tvalid),
        .s_axis_tready(rasterizer_sbr_tready),
        .s_axis_tkeep(rasterizer_sbr_tkeep),
        .s_axis_tid(),
        .s_axis_tdest(),
        .s_axis_tuser(),

        .m_axis_tdata({
            bc_bbx[3 * SCREEN_POS_WIDTH +: SCREEN_POS_WIDTH],
            bc_bby[3 * SCREEN_POS_WIDTH +: SCREEN_POS_WIDTH],
            bc_spx[3 * SCREEN_POS_WIDTH +: SCREEN_POS_WIDTH],
            bc_spy[3 * SCREEN_POS_WIDTH +: SCREEN_POS_WIDTH],
            bc_index[3 * INDEX_WIDTH +: INDEX_WIDTH],
            bc_bbx[2 * SCREEN_POS_WIDTH +: SCREEN_POS_WIDTH],
            bc_bby[2 * SCREEN_POS_WIDTH +: SCREEN_POS_WIDTH],
            bc_spx[2 * SCREEN_POS_WIDTH +: SCREEN_POS_WIDTH],
            bc_spy[2 * SCREEN_POS_WIDTH +: SCREEN_POS_WIDTH],
            bc_index[2 * INDEX_WIDTH +: INDEX_WIDTH],
            bc_bbx[1 * SCREEN_POS_WIDTH +: SCREEN_POS_WIDTH],
            bc_bby[1 * SCREEN_POS_WIDTH +: SCREEN_POS_WIDTH],
            bc_spx[1 * SCREEN_POS_WIDTH +: SCREEN_POS_WIDTH],
            bc_spy[1 * SCREEN_POS_WIDTH +: SCREEN_POS_WIDTH],
            bc_index[1 * INDEX_WIDTH +: INDEX_WIDTH],
            bc_bbx[0 * SCREEN_POS_WIDTH +: SCREEN_POS_WIDTH],
            bc_bby[0 * SCREEN_POS_WIDTH +: SCREEN_POS_WIDTH],
            bc_spx[0 * SCREEN_POS_WIDTH +: SCREEN_POS_WIDTH],
            bc_spy[0 * SCREEN_POS_WIDTH +: SCREEN_POS_WIDTH],
            bc_index[0 * INDEX_WIDTH +: INDEX_WIDTH]
        }),
        .m_axis_tvalid(bc_valid),
        .m_axis_tready({ 
            m_color_arready | !colorBufferEnable, 
            m_depth_arready | !depthBufferEnable, 
            m_stencil_arready | !stencilBufferEnable, 
            mem_arready_attrib 
        }),
        .m_axis_tlast(bc_last),
        .m_axis_tkeep(bc_keep),
        .m_axis_tid(),
        .m_axis_tdest(),
        .m_axis_tuser()
    );
    defparam axisBroadcast.M_COUNT = 4;
    defparam axisBroadcast.DATA_WIDTH = (4 * SCREEN_POS_WIDTH) + INDEX_WIDTH;
    defparam axisBroadcast.KEEP_ENABLE = 1;
    defparam axisBroadcast.KEEP_WIDTH = 1;
    defparam axisBroadcast.LAST_ENABLE = 1;
    defparam axisBroadcast.ID_ENABLE = 0;
    defparam axisBroadcast.DEST_ENABLE = 0;
    defparam axisBroadcast.USER_ENABLE = 0;

    ////////////////////////////////////////////////////////////////////////////
    // STEP 3
    // Interpolation of attributes
    // Clocks: 41
    ////////////////////////////////////////////////////////////////////////////
    wire                            alrp_tvalid;
    wire                            alrp_tready;
    wire                            alrp_tlast;
    wire                            alrp_tkeep;
    wire [SCREEN_POS_WIDTH - 1 : 0] alrp_tspx;
    wire [SCREEN_POS_WIDTH - 1 : 0] alrp_tspy;
    wire [INDEX_WIDTH - 1 : 0]      alrp_tindex;
    wire [ATTRIBUTE_SIZE - 1 : 0]   alrp_tdepth_w;
    wire [ATTRIBUTE_SIZE - 1 : 0]   alrp_tdepth_z;
    wire [ATTRIBUTE_SIZE - 1 : 0]   alrp_ttexture0_t;
    wire [ATTRIBUTE_SIZE - 1 : 0]   alrp_ttexture0_s;
    wire [ATTRIBUTE_SIZE - 1 : 0]   alrp_ttexture1_t;
    wire [ATTRIBUTE_SIZE - 1 : 0]   alrp_ttexture1_s;
    wire [ATTRIBUTE_SIZE - 1 : 0]   alrp_tcolor_a;
    wire [ATTRIBUTE_SIZE - 1 : 0]   alrp_tcolor_b;
    wire [ATTRIBUTE_SIZE - 1 : 0]   alrp_tcolor_g;
    wire [ATTRIBUTE_SIZE - 1 : 0]   alrp_tcolor_r;

    AttributeInterpolator attributeInterpolator (
        .aclk(aclk),
        .resetn(resetn),
        .pixelInPipeline(),

        .s_attrb_tvalid(bc_valid[0]),
        .s_attrb_tready(mem_arready_attrib),
        .s_attrb_tlast(bc_last[0]),
        .s_attrb_tkeep(bc_keep[0]),
        .s_attrb_tbbx(bc_bbx[0 * SCREEN_POS_WIDTH +: SCREEN_POS_WIDTH]),
        .s_attrb_tbby(bc_bby[0 * SCREEN_POS_WIDTH +: SCREEN_POS_WIDTH]),
        .s_attrb_tspx(bc_spx[0 * SCREEN_POS_WIDTH +: SCREEN_POS_WIDTH]),
        .s_attrb_tspy(bc_spy[0 * SCREEN_POS_WIDTH +: SCREEN_POS_WIDTH]),
        .s_attrb_tindex(bc_index[0 * INDEX_WIDTH +: INDEX_WIDTH]),

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
        .color_a_inc_y(triangleParams[TRIANGLE_STREAM_INC_COLOR_A_Y * TRIANGLE_STREAM_PARAM_SIZE +: TRIANGLE_STREAM_PARAM_SIZE]),

        .m_attrb_tvalid(alrp_tvalid),
        .m_attrb_tready(alrp_tready),
        .m_attrb_tlast(alrp_tlast),
        .m_attrb_tkeep(alrp_tkeep),
        .m_attrb_tspx(alrp_tspx),
        .m_attrb_tspy(alrp_tspy),
        .m_attrb_tindex(alrp_tindex),
        .m_attrb_tdepth_w(alrp_tdepth_w),
        .m_attrb_tdepth_z(alrp_tdepth_z),
        .m_attrb_ttexture0_t(alrp_ttexture0_t),
        .m_attrb_ttexture0_s(alrp_ttexture0_s),
        .m_attrb_ttexture1_t(alrp_ttexture1_t),
        .m_attrb_ttexture1_s(alrp_ttexture1_s),
        .m_attrb_tcolor_a(alrp_tcolor_a),
        .m_attrb_tcolor_b(alrp_tcolor_b),
        .m_attrb_tcolor_g(alrp_tcolor_g),
        .m_attrb_tcolor_r(alrp_tcolor_r)
    );
    defparam attributeInterpolator.FLOAT_SIZE = 24;
    defparam attributeInterpolator.INDEX_WIDTH = INDEX_WIDTH;
    defparam attributeInterpolator.SCREEN_POS_WIDTH = SCREEN_POS_WIDTH;

    ////////////////////////////////////////////////////////////////////////////
    // STEP 4
    // Texturing triangle, fogging
    // Clocks: 30
    ////////////////////////////////////////////////////////////////////////////
    wire [(COLOR_SUB_PIXEL_WIDTH * 4) - 1 : 0]  framebuffer_fragmentColor;
    wire [INDEX_WIDTH - 1 : 0]                  framebuffer_index;
    wire [SCREEN_POS_WIDTH - 1 : 0]             framebuffer_screenPosX;
    wire [SCREEN_POS_WIDTH - 1 : 0]             framebuffer_screenPosY;
    wire [31 : 0]                               framebuffer_depth;
    wire                                        framebuffer_valid;
    wire                                        framebuffer_last;
    wire                                        framebuffer_keep;

    PixelPipeline pixelPipeline (    
        .aclk(aclk),
        .resetn(resetn),

        .s_fog_lut_axis_tvalid(cmd_fog_axis_tvalid),
        .s_fog_lut_axis_tready(cmd_fog_axis_tready),
        .s_fog_lut_axis_tlast(cmd_xxx_axis_tlast),
        .s_fog_lut_axis_tdata(cmd_xxx_axis_tdata),

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

        .s_attrb_tvalid(alrp_tvalid),
        .s_attrb_tready(alrp_tready),
        .s_attrb_tlast(alrp_tlast),
        .s_attrb_tkeep(alrp_tkeep),
        .s_attrb_tspx(alrp_tspx),
        .s_attrb_tspy(alrp_tspy),
        .s_attrb_tindex(alrp_tindex),
        .s_attrb_tdepth_w(alrp_tdepth_w),
        .s_attrb_tdepth_z(alrp_tdepth_z),
        .s_attrb_ttexture0_t(alrp_ttexture0_t),
        .s_attrb_ttexture0_s(alrp_ttexture0_s),
        .s_attrb_ttexture1_t(alrp_ttexture1_t),
        .s_attrb_ttexture1_s(alrp_ttexture1_s),
        .s_attrb_tcolor_a(alrp_tcolor_a),
        .s_attrb_tcolor_b(alrp_tcolor_b),
        .s_attrb_tcolor_g(alrp_tcolor_g),
        .s_attrb_tcolor_r(alrp_tcolor_r),

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

        .m_framebuffer_valid(framebuffer_valid),
        .m_framebuffer_last(framebuffer_last),
        .m_framebuffer_keep(framebuffer_keep),
        .m_framebuffer_fragmentColor(framebuffer_fragmentColor),
        .m_framebuffer_depth(framebuffer_depth),
        .m_framebuffer_index(framebuffer_index),
        .m_framebuffer_screenPosX(framebuffer_screenPosX),
        .m_framebuffer_screenPosY(framebuffer_screenPosY)
    );
    defparam pixelPipeline.INDEX_WIDTH = INDEX_WIDTH;
    defparam pixelPipeline.CMD_STREAM_WIDTH = CMD_STREAM_WIDTH;
    defparam pixelPipeline.SUB_PIXEL_WIDTH = COLOR_SUB_PIXEL_WIDTH;
    defparam pixelPipeline.ENABLE_SECOND_TMU = ENABLE_SECOND_TMU;
    defparam pixelPipeline.STENCIL_WIDTH = STENCIL_WIDTH;
    defparam pixelPipeline.DEPTH_WIDTH = DEPTH_WIDTH;
    defparam pixelPipeline.SCREEN_POS_WIDTH = SCREEN_POS_WIDTH;

    ////////////////////////////////////////////////////////////////////////////
    // STEP 5
    // Concatinates the stream from the pixel pipeline and from the framebuffers
    // Clocks: n/a
    ////////////////////////////////////////////////////////////////////////////
    localparam FRAGMENT_STREAM_WIDTH = (COLOR_SUB_PIXEL_WIDTH * 4) + 32 + INDEX_WIDTH + (SCREEN_POS_WIDTH * 2) + 1 + 1;

    wire fragment_stream_out_tvalid;
    wire fragment_stream_out_tready;
    wire [(FRAGMENT_STREAM_WIDTH + PIXEL_WIDTH + DEPTH_WIDTH + STENCIL_WIDTH) - 1 : 0] fragment_stream_out_tdata;

    StreamConcatFifo streamConcatFifo (
        .aclk(aclk),
        .resetn(resetn),

        .s_stream0_tenable(1'b1),
        .s_stream0_tvalid(framebuffer_valid),
        .s_stream0_tdata({ 
            framebuffer_keep,
            framebuffer_last,
            framebuffer_screenPosY,
            framebuffer_screenPosX,
            framebuffer_index,
            framebuffer_depth, 
            framebuffer_fragmentColor 
        }),
        .s_stream0_tready(),

        .s_stream1_tenable(colorBufferEnable),
        .s_stream1_tvalid(m_color_rvalid),
        .s_stream1_tdata(m_color_rdata),
        .s_stream1_tready(m_color_rready),

        .s_stream2_tenable(depthBufferEnable),
        .s_stream2_tvalid(m_depth_rvalid),
        .s_stream2_tdata(m_depth_rdata),
        .s_stream2_tready(m_depth_rready),

        .s_stream3_tenable(stencilBufferEnable),
        .s_stream3_tvalid(m_stencil_rvalid),
        .s_stream3_tdata(m_stencil_rdata),
        .s_stream3_tready(m_stencil_rready),

        .m_stream_tvalid(fragment_stream_out_tvalid),
        .m_stream_tdata(fragment_stream_out_tdata),
        .m_stream_tready(fragment_stream_out_tready)
    );
    defparam streamConcatFifo.FIFO_DEPTH0_POW2 = (ENABLE_FLOW_CTRL) ? MAX_NUMBER_OF_PIXELS_LG : 0;
    defparam streamConcatFifo.FIFO_DEPTH1_POW2 = MAX_NUMBER_OF_PIXELS_LG;
    defparam streamConcatFifo.FIFO_DEPTH2_POW2 = MAX_NUMBER_OF_PIXELS_LG;
    defparam streamConcatFifo.FIFO_DEPTH3_POW2 = MAX_NUMBER_OF_PIXELS_LG;
    defparam streamConcatFifo.STREAM0_WIDTH = FRAGMENT_STREAM_WIDTH;
    defparam streamConcatFifo.STREAM1_WIDTH = PIXEL_WIDTH;
    defparam streamConcatFifo.STREAM2_WIDTH = DEPTH_WIDTH;
    defparam streamConcatFifo.STREAM3_WIDTH = STENCIL_WIDTH;

    ////////////////////////////////////////////////////////////////////////////
    // STEP 6
    // Access framebuffer, blend, test and save pixel in framebuffer
    // Clocks: 4
    ////////////////////////////////////////////////////////////////////////////
    wire [INDEX_WIDTH - 1 : 0]      color_fifo_waddr;
    wire                            color_fifo_wvalid;
    wire                            color_fifo_wlast;
    wire [PIXEL_WIDTH - 1 : 0]      color_fifo_wdata;
    wire                            color_fifo_wstrb;
    wire [SCREEN_POS_WIDTH - 1 : 0] color_fifo_wscreenPosX;
    wire [SCREEN_POS_WIDTH - 1 : 0] color_fifo_wscreenPosY;

    wire [INDEX_WIDTH - 1 : 0]      depth_fifo_waddr;
    wire                            depth_fifo_wvalid;
    wire                            depth_fifo_wlast;
    wire [DEPTH_WIDTH - 1 : 0]      depth_fifo_wdata;
    wire                            depth_fifo_wstrb;
    wire [SCREEN_POS_WIDTH - 1 : 0] depth_fifo_wscreenPosX;
    wire [SCREEN_POS_WIDTH - 1 : 0] depth_fifo_wscreenPosY;

    wire [INDEX_WIDTH - 1 : 0]      stencil_fifo_waddr;
    wire                            stencil_fifo_wvalid;
    wire                            stencil_fifo_wlast;
    wire [STENCIL_WIDTH - 1 : 0]    stencil_fifo_wdata;
    wire                            stencil_fifo_wstrb;
    wire [SCREEN_POS_WIDTH - 1 : 0] stencil_fifo_wscreenPosX;
    wire [SCREEN_POS_WIDTH - 1 : 0] stencil_fifo_wscreenPosY;

    PerFragmentPipeline perFragmentPipeline (
        .aclk(aclk),
        .resetn(resetn),

        .conf(confFragmentPipelineConfig),
        .confFeatureEnable(confFeatureEnable),
        .confStencilBufferConfig(confStencilBufferConfig),

        .s_frag_tready(fragment_stream_out_tready),
        .s_frag_tlast(fragment_stream_out_tdata[(COLOR_SUB_PIXEL_WIDTH * 4) + 32 + INDEX_WIDTH + (SCREEN_POS_WIDTH * 2) +: 1]),
        .s_frag_tkeep(fragment_stream_out_tdata[(COLOR_SUB_PIXEL_WIDTH * 4) + 32 + INDEX_WIDTH + (SCREEN_POS_WIDTH * 2) + 1 +: 1]),
        .s_frag_tvalid(fragment_stream_out_tvalid),
        .s_frag_tcolor(fragment_stream_out_tdata[0 +: (COLOR_SUB_PIXEL_WIDTH * 4)]),
        .s_frag_tdepth(fragment_stream_out_tdata[(COLOR_SUB_PIXEL_WIDTH * 4) +: 32]),
        .s_frag_tindex(fragment_stream_out_tdata[(COLOR_SUB_PIXEL_WIDTH * 4) + 32 +: INDEX_WIDTH]),
        .s_frag_tscreenPosX(fragment_stream_out_tdata[(COLOR_SUB_PIXEL_WIDTH * 4) + 32 + INDEX_WIDTH +: SCREEN_POS_WIDTH]),
        .s_frag_tscreenPosY(fragment_stream_out_tdata[(COLOR_SUB_PIXEL_WIDTH * 4) + 32 + INDEX_WIDTH + SCREEN_POS_WIDTH +: SCREEN_POS_WIDTH]),

        .fragmentProcessed(fragmentProcessed),

        .s_color_rready(),
        .s_color_rvalid(fragment_stream_out_tvalid),
        .s_color_rdata(fragment_stream_out_tdata[FRAGMENT_STREAM_WIDTH +: PIXEL_WIDTH]),
        .m_color_waddr(color_fifo_waddr),
        .m_color_wvalid(color_fifo_wvalid),
        .m_color_wlast(color_fifo_wlast),
        .m_color_wdata(color_fifo_wdata),
        .m_color_wstrb(color_fifo_wstrb),
        .m_color_wscreenPosX(color_fifo_wscreenPosX),
        .m_color_wscreenPosY(color_fifo_wscreenPosY),

        .s_depth_rready(),
        .s_depth_rvalid(fragment_stream_out_tvalid),
        .s_depth_rdata(fragment_stream_out_tdata[FRAGMENT_STREAM_WIDTH + PIXEL_WIDTH +: DEPTH_WIDTH]),
        .m_depth_waddr(depth_fifo_waddr),
        .m_depth_wvalid(depth_fifo_wvalid),
        .m_depth_wlast(depth_fifo_wlast),
        .m_depth_wdata(depth_fifo_wdata),
        .m_depth_wstrb(depth_fifo_wstrb),
        .m_depth_wscreenPosX(depth_fifo_wscreenPosX),
        .m_depth_wscreenPosY(depth_fifo_wscreenPosY),

        .s_stencil_rready(),
        .s_stencil_rvalid(fragment_stream_out_tvalid),
        .s_stencil_rdata(fragment_stream_out_tdata[FRAGMENT_STREAM_WIDTH + PIXEL_WIDTH + DEPTH_WIDTH +: STENCIL_WIDTH]),
        .m_stencil_waddr(stencil_fifo_waddr),
        .m_stencil_wvalid(stencil_fifo_wvalid),
        .m_stencil_wlast(stencil_fifo_wlast),
        .m_stencil_wdata(stencil_fifo_wdata),
        .m_stencil_wstrb(stencil_fifo_wstrb),
        .m_stencil_wscreenPosX(stencil_fifo_wscreenPosX),
        .m_stencil_wscreenPosY(stencil_fifo_wscreenPosY)
    );
    defparam perFragmentPipeline.FRAMEBUFFER_INDEX_WIDTH = INDEX_WIDTH;
    defparam perFragmentPipeline.SCREEN_POS_WIDTH = SCREEN_POS_WIDTH;
    defparam perFragmentPipeline.DEPTH_WIDTH = DEPTH_WIDTH;
    defparam perFragmentPipeline.STENCIL_WIDTH = STENCIL_WIDTH;
    defparam perFragmentPipeline.SUB_PIXEL_WIDTH = COLOR_SUB_PIXEL_WIDTH;

    ////////////////////////////////////////////////////////////////////////////
    // STEP 7
    // FIFOs for the flow control on the write channel
    // Clocks: 1
    ////////////////////////////////////////////////////////////////////////////
    localparam WRITE_FIFO_SIZE = MAX_NUMBER_OF_PIXELS_LG + 1;

    generate
        if (ENABLE_FLOW_CTRL)
        begin
            localparam COLOR_FIFO_WIDTH = 1 + 1 + SCREEN_POS_WIDTH + SCREEN_POS_WIDTH + INDEX_WIDTH + PIXEL_WIDTH;
            sfifo colorWriteFifo (
                .i_clk(aclk),
                .i_reset(!resetn),

                .i_wr(color_fifo_wvalid),
                .i_data({ 
                    color_fifo_wlast, 
                    color_fifo_wstrb, 
                    color_fifo_wscreenPosY, 
                    color_fifo_wscreenPosX, 
                    color_fifo_waddr, 
                    color_fifo_wdata 
                }),
                .o_full(),
                .o_fill(color_fifo_fill),

                .i_rd(m_color_wready),
                .o_data({
                    m_color_wlast,
                    m_color_wstrb,
                    m_color_wscreenPosY,
                    m_color_wscreenPosX,
                    m_color_waddr,
                    m_color_wdata
                }),
                .o_empty(color_fifo_empty)    
            );
            defparam colorWriteFifo.BW = COLOR_FIFO_WIDTH;
            defparam colorWriteFifo.LGFLEN = WRITE_FIFO_SIZE;
            defparam colorWriteFifo.OPT_ASYNC_READ = 0;

            assign m_color_wvalid = !color_fifo_empty;
        end
        else
        begin
            assign m_color_wdata = color_fifo_wdata;
            assign m_color_waddr = color_fifo_waddr;
            assign m_color_wscreenPosX = color_fifo_wscreenPosX;
            assign m_color_wscreenPosY = color_fifo_wscreenPosY;
            assign m_color_wstrb = color_fifo_wstrb;
            assign m_color_wlast = color_fifo_wlast;
            assign m_color_wvalid = color_fifo_wvalid;

            assign color_fifo_fill = 0;
            assign color_fifo_empty = 1;
        end
    endgenerate

    generate
        if (ENABLE_FLOW_CTRL)
        begin
            localparam DEPTH_FIFO_WIDTH = 1 + 1 + SCREEN_POS_WIDTH + SCREEN_POS_WIDTH + INDEX_WIDTH + DEPTH_WIDTH;
            sfifo depthWriteFifo (
                .i_clk(aclk),
                .i_reset(!resetn),

                .i_wr(depth_fifo_wvalid),
                .i_data({ 
                    depth_fifo_wlast, 
                    depth_fifo_wstrb, 
                    depth_fifo_wscreenPosY, 
                    depth_fifo_wscreenPosX, 
                    depth_fifo_waddr, 
                    depth_fifo_wdata 
                }),
                .o_full(),
                .o_fill(depth_fifo_fill),

                .i_rd(m_depth_wready),
                .o_data({
                    m_depth_wlast,
                    m_depth_wstrb,
                    m_depth_wscreenPosY,
                    m_depth_wscreenPosX,
                    m_depth_waddr,
                    m_depth_wdata
                }),
                .o_empty(depth_fifo_empty)    
            );
            defparam depthWriteFifo.BW = DEPTH_FIFO_WIDTH;
            defparam depthWriteFifo.LGFLEN = WRITE_FIFO_SIZE;
            defparam depthWriteFifo.OPT_ASYNC_READ = 0;

            assign m_depth_wvalid = !depth_fifo_empty;
        end
        else
        begin
            assign m_depth_wdata = depth_fifo_wdata;
            assign m_depth_waddr = depth_fifo_waddr;
            assign m_depth_wscreenPosX = depth_fifo_wscreenPosX;
            assign m_depth_wscreenPosY = depth_fifo_wscreenPosY;
            assign m_depth_wstrb = depth_fifo_wstrb;
            assign m_depth_wlast = depth_fifo_wlast;
            assign m_depth_wvalid = depth_fifo_wvalid;

            assign depth_fifo_fill = 0;
            assign depth_fifo_empty = 1;
        end
    endgenerate
    
    generate
        if (ENABLE_FLOW_CTRL)
        begin
            localparam STENCIL_FIFO_WIDTH = 1 + 1 + SCREEN_POS_WIDTH + SCREEN_POS_WIDTH + INDEX_WIDTH + STENCIL_WIDTH;
            sfifo stencilWriteFifo (
                .i_clk(aclk),
                .i_reset(!resetn),

                .i_wr(stencil_fifo_wvalid),
                .i_data({ 
                    stencil_fifo_wlast, 
                    stencil_fifo_wstrb, 
                    stencil_fifo_wscreenPosY, 
                    stencil_fifo_wscreenPosX, 
                    stencil_fifo_waddr, 
                    stencil_fifo_wdata 
                }),
                .o_full(),
                .o_fill(stencil_fifo_fill),

                .i_rd(m_stencil_wready),
                .o_data({
                    m_stencil_wlast,
                    m_stencil_wstrb,
                    m_stencil_wscreenPosY,
                    m_stencil_wscreenPosX,
                    m_stencil_waddr,
                    m_stencil_wdata
                }),
                .o_empty(stencil_fifo_empty)    
            );
            defparam stencilWriteFifo.BW = STENCIL_FIFO_WIDTH;
            defparam stencilWriteFifo.LGFLEN = WRITE_FIFO_SIZE;
            defparam stencilWriteFifo.OPT_ASYNC_READ = 0;

            assign m_stencil_wvalid = !stencil_fifo_empty;
        end
        else
        begin
            assign m_stencil_wdata = stencil_fifo_wdata;
            assign m_stencil_waddr = stencil_fifo_waddr;
            assign m_stencil_wscreenPosX = stencil_fifo_wscreenPosX;
            assign m_stencil_wscreenPosY = stencil_fifo_wscreenPosY;
            assign m_stencil_wstrb = stencil_fifo_wstrb;
            assign m_stencil_wlast = stencil_fifo_wlast;
            assign m_stencil_wvalid = stencil_fifo_wvalid;

            assign stencil_fifo_fill = 0;
            assign stencil_fifo_empty = 1;
        end
    endgenerate
endmodule
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

module RasterixIF #(
    // The size of the internal framebuffer (in power of two)
    // Depth buffer word size: 16 bit
    // Color buffer word size: FRAMEBUFFER_SUB_PIXEL_WIDTH * (FRAMEBUFFER_ENABLE_ALPHA_CHANNEL ? 4 : 3)
    parameter FRAMEBUFFER_SIZE_IN_WORDS = 17,

    // This is the color depth of the framebuffer. Note: This setting has no influence on the framebuffer stream. This steam will
    // stay at RGB565. It changes the internal representation and might be used to reduce the memory footprint.
    // Lower depth will result in color banding.
    parameter FRAMEBUFFER_SUB_PIXEL_WIDTH = 6,
    // This enables the alpha channel of the framebuffer. Requires additional memory.
    parameter FRAMEBUFFER_ENABLE_ALPHA_CHANNEL = 0,
    // The number of sub pixels in the framebuffer
    localparam FRAMEBUFFER_NUMBER_OF_SUB_PIXELS = (FRAMEBUFFER_ENABLE_ALPHA_CHANNEL == 0) ? 3 : 4,
    // The sub pixel with in the framebuffer
    localparam PIXEL_WIDTH_FRAMEBUFFER = FRAMEBUFFER_NUMBER_OF_SUB_PIXELS * FRAMEBUFFER_SUB_PIXEL_WIDTH,

    // The width of the stencil buffer
    localparam STENCIL_WIDTH = 4,

    // The width of the depth buffer
    localparam DEPTH_WIDTH = 16,

    // This enables the 4 bit stencil buffer
    parameter ENABLE_STENCIL_BUFFER = 1,

    // Number of TMUs. Currently supported values: 1 and 2
    parameter TMU_COUNT = 2,
    
    // The bit width of the command stream interface and memory interface
    // Allowed values: 32, 64, 128, 256 bit
    parameter CMD_STREAM_WIDTH = 16,

    // The width of the frame buffer stream
    parameter FRAMEBUFFER_STREAM_WIDTH = CMD_STREAM_WIDTH,

    // The size of the texture in bytes in power of two
    parameter TEXTURE_BUFFER_SIZE = 15,

    // Memory address witdth
    parameter ADDR_WIDTH = 24,
    // Memory ID width
    parameter ID_WIDTH = 8,
    // Memory strobe width
    parameter STRB_WIDTH = CMD_STREAM_WIDTH / 8
)
(
    input  wire                             aclk,
    input  wire                             resetn,

    // AXI Stream command interface
    input  wire                             s_cmd_axis_tvalid,
    output wire                             s_cmd_axis_tready,
    input  wire                             s_cmd_axis_tlast,
    input  wire [CMD_STREAM_WIDTH - 1 : 0]  s_cmd_axis_tdata,

    // Framebuffer output
    // AXI Stream master interface (RGB565)
    output wire                             m_framebuffer_axis_tvalid,
    input  wire                             m_framebuffer_axis_tready,
    output wire                             m_framebuffer_axis_tlast,
    output wire [CMD_STREAM_WIDTH - 1 : 0]  m_framebuffer_axis_tdata,

    // Memory interface
    output wire [ID_WIDTH - 1 : 0]          m_mem_axi_awid,
    output wire [ADDR_WIDTH - 1 : 0]        m_mem_axi_awaddr,
    output wire [ 7 : 0]                    m_mem_axi_awlen, // How many beats are in this transaction
    output wire [ 2 : 0]                    m_mem_axi_awsize, // The increment during one cycle. Means, 0 incs addr by 1, 2 by 4 and so on
    output wire [ 1 : 0]                    m_mem_axi_awburst, // 0 fixed, 1 incr, 2 wrappig
    output wire                             m_mem_axi_awlock,
    output wire [ 3 : 0]                    m_mem_axi_awcache,
    output wire [ 2 : 0]                    m_mem_axi_awprot, 
    output wire                             m_mem_axi_awvalid,
    input  wire                             m_mem_axi_awready,

    output wire [CMD_STREAM_WIDTH - 1 : 0]  m_mem_axi_wdata,
    output wire [STRB_WIDTH - 1 : 0]        m_mem_axi_wstrb,
    output wire                             m_mem_axi_wlast,
    output wire                             m_mem_axi_wvalid,
    input  wire                             m_mem_axi_wready,

    input  wire [ID_WIDTH - 1 : 0]          m_mem_axi_bid,
    input  wire [ 1 : 0]                    m_mem_axi_bresp,
    input  wire                             m_mem_axi_bvalid,
    output wire                             m_mem_axi_bready,

    output wire [ID_WIDTH - 1 : 0]          m_mem_axi_arid,
    output wire [ADDR_WIDTH - 1 : 0]        m_mem_axi_araddr,
    output wire [ 7 : 0]                    m_mem_axi_arlen,
    output wire [ 2 : 0]                    m_mem_axi_arsize,
    output wire [ 1 : 0]                    m_mem_axi_arburst,
    output wire                             m_mem_axi_arlock,
    output wire [ 3 : 0]                    m_mem_axi_arcache,
    output wire [ 2 : 0]                    m_mem_axi_arprot,
    output wire                             m_mem_axi_arvalid,
    input  wire                             m_mem_axi_arready,

    input  wire [ID_WIDTH - 1 : 0]          m_mem_axi_rid,
    input  wire [CMD_STREAM_WIDTH - 1 : 0]  m_mem_axi_rdata,
    input  wire [ 1 : 0]                    m_mem_axi_rresp,
    input  wire                             m_mem_axi_rlast,
    input  wire                             m_mem_axi_rvalid,
    output wire                             m_mem_axi_rready
);
`include "RegisterAndDescriptorDefines.vh"
    localparam DEFAULT_ALPHA_VAL = 0;
    localparam SCREEN_POS_WIDTH = 11;
    localparam PIXEL_WIDTH_STREAM = 16;
    localparam PIXEL_PER_BEAT = FRAMEBUFFER_STREAM_WIDTH / PIXEL_WIDTH_STREAM;
    localparam PIPELINE_PIXEL_WIDTH = COLOR_SUB_PIXEL_WIDTH * COLOR_NUMBER_OF_SUB_PIXEL;
    // This is used to configure, if it is required to reduce / expand a vector or not. This is done by the offset:
    // When the offset is set to number of pixels, then the reduce / expand function will just copy the line
    // without removing or adding something.
    // If it is set to a lower value, then the functions will start to remove or add new pixels.
    localparam SUB_PIXEL_OFFSET = (COLOR_NUMBER_OF_SUB_PIXEL == FRAMEBUFFER_NUMBER_OF_SUB_PIXELS) ? COLOR_NUMBER_OF_SUB_PIXEL : COLOR_A_POS; 
    `ReduceVec(ColorBufferReduceVec, COLOR_SUB_PIXEL_WIDTH, COLOR_NUMBER_OF_SUB_PIXEL, SUB_PIXEL_OFFSET, COLOR_NUMBER_OF_SUB_PIXEL, FRAMEBUFFER_NUMBER_OF_SUB_PIXELS)
    `ReduceVec(ColorBufferReduceMask, 1, COLOR_NUMBER_OF_SUB_PIXEL, SUB_PIXEL_OFFSET, COLOR_NUMBER_OF_SUB_PIXEL, FRAMEBUFFER_NUMBER_OF_SUB_PIXELS)
    `ExpandVec(ColorBufferExpandVec, COLOR_SUB_PIXEL_WIDTH, FRAMEBUFFER_NUMBER_OF_SUB_PIXELS, SUB_PIXEL_OFFSET, COLOR_NUMBER_OF_SUB_PIXEL, COLOR_NUMBER_OF_SUB_PIXEL)
    `Expand(ColorBufferExpand, FRAMEBUFFER_SUB_PIXEL_WIDTH, COLOR_SUB_PIXEL_WIDTH, FRAMEBUFFER_NUMBER_OF_SUB_PIXELS)
    `Reduce(ColorBufferReduce, FRAMEBUFFER_SUB_PIXEL_WIDTH, COLOR_SUB_PIXEL_WIDTH, FRAMEBUFFER_NUMBER_OF_SUB_PIXELS)

    wire                             m_cmd_axis_tvalid;
    wire                             m_cmd_axis_tready;
    wire                             m_cmd_axis_tlast;
    wire [CMD_STREAM_WIDTH - 1 : 0]  m_cmd_axis_tdata;

    wire                             s_framebuffer_axis_tvalid;
    wire                             s_framebuffer_axis_tready;
    wire                             s_framebuffer_axis_tlast;
    wire [CMD_STREAM_WIDTH - 1 : 0]  s_framebuffer_axis_tdata;

    DmaStreamEngine #(
        .STREAM_WIDTH(CMD_STREAM_WIDTH),
        .ADDR_WIDTH(ADDR_WIDTH)
    ) dma (
        .aclk(aclk),
        .resetn(resetn),

        .m_st1_axis_tvalid(m_cmd_axis_tvalid),
        .m_st1_axis_tready(m_cmd_axis_tready),
        .m_st1_axis_tlast(m_cmd_axis_tlast),
        .m_st1_axis_tdata(m_cmd_axis_tdata),

        .s_st1_axis_tvalid(s_framebuffer_axis_tvalid),
        .s_st1_axis_tready(s_framebuffer_axis_tready),
        .s_st1_axis_tlast(s_framebuffer_axis_tlast),
        .s_st1_axis_tdata(s_framebuffer_axis_tdata),

        .m_st0_axis_tvalid(m_framebuffer_axis_tvalid),
        .m_st0_axis_tready(m_framebuffer_axis_tready),
        .m_st0_axis_tlast(m_framebuffer_axis_tlast),
        .m_st0_axis_tdata(m_framebuffer_axis_tdata),

        .s_st0_axis_tvalid(s_cmd_axis_tvalid),
        .s_st0_axis_tready(s_cmd_axis_tready),
        .s_st0_axis_tlast(s_cmd_axis_tlast),
        .s_st0_axis_tdata(s_cmd_axis_tdata),

        .m_mem_axi_awid(m_mem_axi_awid),
        .m_mem_axi_awaddr(m_mem_axi_awaddr),
        .m_mem_axi_awlen(m_mem_axi_awlen), 
        .m_mem_axi_awsize(m_mem_axi_awsize), 
        .m_mem_axi_awburst(m_mem_axi_awburst), 
        .m_mem_axi_awlock(m_mem_axi_awlock), 
        .m_mem_axi_awcache(m_mem_axi_awcache), 
        .m_mem_axi_awprot(m_mem_axi_awprot), 
        .m_mem_axi_awvalid(m_mem_axi_awvalid),
        .m_mem_axi_awready(m_mem_axi_awready),

        .m_mem_axi_wdata(m_mem_axi_wdata),
        .m_mem_axi_wstrb(m_mem_axi_wstrb),
        .m_mem_axi_wlast(m_mem_axi_wlast),
        .m_mem_axi_wvalid(m_mem_axi_wvalid),
        .m_mem_axi_wready(m_mem_axi_wready),

        .m_mem_axi_bid(m_mem_axi_bid),
        .m_mem_axi_bresp(m_mem_axi_bresp),
        .m_mem_axi_bvalid(m_mem_axi_bvalid),
        .m_mem_axi_bready(m_mem_axi_bready),

        .m_mem_axi_arid(m_mem_axi_arid),
        .m_mem_axi_araddr(m_mem_axi_araddr),
        .m_mem_axi_arlen(m_mem_axi_arlen),
        .m_mem_axi_arsize(m_mem_axi_arsize),
        .m_mem_axi_arburst(m_mem_axi_arburst),
        .m_mem_axi_arlock(m_mem_axi_arlock),
        .m_mem_axi_arcache(m_mem_axi_arcache),
        .m_mem_axi_arprot(m_mem_axi_arprot),
        .m_mem_axi_arvalid(m_mem_axi_arvalid),
        .m_mem_axi_arready(m_mem_axi_arready),

        .m_mem_axi_rid(m_mem_axi_rid),
        .m_mem_axi_rdata(m_mem_axi_rdata),
        .m_mem_axi_rresp(m_mem_axi_rresp),
        .m_mem_axi_rlast(m_mem_axi_rlast),
        .m_mem_axi_rvalid(m_mem_axi_rvalid),
        .m_mem_axi_rready(m_mem_axi_rready)
    );

    wire                                             framebufferParamEnableScissor;
    wire [SCREEN_POS_WIDTH - 1 : 0]                  framebufferParamScissorStartX;
    wire [SCREEN_POS_WIDTH - 1 : 0]                  framebufferParamScissorStartY;
    wire [SCREEN_POS_WIDTH - 1 : 0]                  framebufferParamScissorEndX;
    wire [SCREEN_POS_WIDTH - 1 : 0]                  framebufferParamScissorEndY;
    wire [SCREEN_POS_WIDTH - 1 : 0]                  framebufferParamYOffset;
    wire [SCREEN_POS_WIDTH - 1 : 0]                  framebufferParamXResolution;
    wire [SCREEN_POS_WIDTH - 1 : 0]                  framebufferParamYResolution;

    // Color buffer access
    wire [PIPELINE_PIXEL_WIDTH - 1 : 0]              colorBufferClearColor;
    wire                                             colorBufferApply;
    wire                                             colorBufferApplied;
    wire                                             colorBufferCmdCommit;
    wire                                             colorBufferCmdMemset;
    wire                                             colorBufferEnable;
    wire [3 : 0]                                     colorBufferMask;
    wire                                             color_arvalid;
    wire                                             color_arlast;
    wire                                             color_rvalid;
    wire                                             color_rlast;
    wire [FRAMEBUFFER_SIZE_IN_WORDS - 1 : 0]         color_araddr;
    wire [FRAMEBUFFER_SIZE_IN_WORDS - 1 : 0]         color_waddr;
    wire                                             color_wvalid;
    wire [PIXEL_WIDTH_FRAMEBUFFER - 1 : 0]           color_rdata;
    wire [PIPELINE_PIXEL_WIDTH - 1 : 0]              color_wdata;
    wire                                             color_wstrb;
    wire [SCREEN_POS_WIDTH - 1 : 0]                  color_wscreenPosX;
    wire [SCREEN_POS_WIDTH - 1 : 0]                  color_wscreenPosY;

    // Depth buffer access
    wire [DEPTH_WIDTH - 1 : 0]                       depthBufferClearDepth;
    wire                                             depthBufferApply;
    wire                                             depthBufferApplied;
    wire                                             depthBufferCmdCommit;
    wire                                             depthBufferCmdMemset;
    wire                                             depthBufferEnable;
    wire                                             depthBufferMask;
    wire                                             depth_arvalid;
    wire                                             depth_arlast;
    wire                                             depth_rvalid;
    wire                                             depth_rlast;
    wire [FRAMEBUFFER_SIZE_IN_WORDS - 1 : 0]         depth_araddr;
    wire [FRAMEBUFFER_SIZE_IN_WORDS - 1 : 0]         depth_waddr;
    wire                                             depth_wvalid;
    wire [DEPTH_WIDTH - 1 : 0]                       depth_rdata;
    wire [DEPTH_WIDTH - 1 : 0]                       depth_wdata;
    wire                                             depth_wstrb;
    wire [SCREEN_POS_WIDTH - 1 : 0]                  depth_wscreenPosX;
    wire [SCREEN_POS_WIDTH - 1 : 0]                  depth_wscreenPosY;

    // Stencil buffer access
    wire [STENCIL_WIDTH - 1 : 0]                     stencilBufferClearStencil;
    wire                                             stencilBufferApply;
    wire                                             stencilBufferApplied;
    wire                                             stencilBufferCmdCommit;
    wire                                             stencilBufferCmdMemset;
    wire                                             stencilBufferEnable;
    wire [STENCIL_WIDTH - 1 : 0]                     stencilBufferMask;
    wire                                             stencil_arvalid;
    wire                                             stencil_arlast;
    wire                                             stencil_rvalid;
    wire                                             stencil_rlast;
    wire [FRAMEBUFFER_SIZE_IN_WORDS - 1 : 0]         stencil_araddr;
    wire [FRAMEBUFFER_SIZE_IN_WORDS - 1 : 0]         stencil_waddr;
    wire                                             stencil_wvalid;
    wire [STENCIL_WIDTH - 1 : 0]                     stencil_rdata;
    wire [STENCIL_WIDTH - 1 : 0]                     stencil_wdata;
    wire                                             stencil_wstrb;
    wire [SCREEN_POS_WIDTH - 1 : 0]                  stencil_wscreenPosX;
    wire [SCREEN_POS_WIDTH - 1 : 0]                  stencil_wscreenPosY;

    FrameBuffer depthBuffer (  
        .clk(aclk),
        .reset(!resetn),

        .confEnable(depthBufferEnable),
        .confClearColor(depthBufferClearDepth),
        .confEnableScissor(framebufferParamEnableScissor),
        .confScissorStartX(framebufferParamScissorStartX),
        .confScissorStartY(framebufferParamScissorStartY),
        .confScissorEndX(framebufferParamScissorEndX),
        .confScissorEndY(framebufferParamScissorEndY),
        .confYOffset(framebufferParamYOffset),
        .confXResolution(framebufferParamXResolution),
        .confYResolution(framebufferParamYResolution),
        .confMask(depthBufferMask),

        .araddr(depth_araddr),
        .arvalid(depth_arvalid),
        .arlast(depth_arlast),
        .rvalid(depth_rvalid),
        .rlast(depth_rlast),
        .rdata(depth_rdata),
        .waddr(depth_waddr),
        .wdata(depth_wdata),
        .wvalid(depth_wvalid),
        .wstrb(depth_wstrb),
        .wscreenPosX(depth_wscreenPosX),
        .wscreenPosY(depth_wscreenPosY),

        .apply(depthBufferApply),
        .applied(depthBufferApplied),
        .cmdCommit(depthBufferCmdCommit),
        .cmdMemset(depthBufferCmdMemset),

        .m_axis_tvalid(),
        .m_axis_tready(1'b1),
        .m_axis_tlast(),
        .m_axis_tdata()
    );
    defparam depthBuffer.NUMBER_OF_PIXELS_PER_BEAT = PIXEL_PER_BEAT;
    defparam depthBuffer.NUMBER_OF_SUB_PIXELS = 1;
    defparam depthBuffer.SUB_PIXEL_WIDTH = 16;
    defparam depthBuffer.X_BIT_WIDTH = RENDER_CONFIG_X_SIZE;
    defparam depthBuffer.Y_BIT_WIDTH = RENDER_CONFIG_Y_SIZE;
    defparam depthBuffer.FRAMEBUFFER_SIZE_IN_WORDS = FRAMEBUFFER_SIZE_IN_WORDS;

    wire [(PIXEL_WIDTH_FRAMEBUFFER * PIXEL_PER_BEAT) - 1 : 0] s_framebuffer_unconverted_axis_tdata;
    FrameBuffer colorBuffer (  
        .clk(aclk),
        .reset(!resetn),

        .confEnable(colorBufferEnable),
        .confClearColor(ColorBufferReduce(ColorBufferReduceVec(colorBufferClearColor))),
        .confEnableScissor(framebufferParamEnableScissor),
        .confScissorStartX(framebufferParamScissorStartX),
        .confScissorStartY(framebufferParamScissorStartY),
        .confScissorEndX(framebufferParamScissorEndX),
        .confScissorEndY(framebufferParamScissorEndY),
        .confYOffset(framebufferParamYOffset),
        .confXResolution(framebufferParamXResolution),
        .confYResolution(framebufferParamYResolution),
        .confMask(ColorBufferReduceMask(colorBufferMask)),

        .araddr(color_araddr),
        .arvalid(color_arvalid),
        .arlast(color_arlast),
        .rvalid(color_rvalid),
        .rlast(color_rlast),
        .rdata(color_rdata),
        .waddr(color_waddr),
        .wdata(ColorBufferReduce(ColorBufferReduceVec(color_wdata))),
        .wvalid(color_wvalid),
        .wstrb(color_wstrb),
        .wscreenPosX(color_wscreenPosX),
        .wscreenPosY(color_wscreenPosY),
        
        .apply(colorBufferApply),
        .applied(colorBufferApplied),
        .cmdCommit(colorBufferCmdCommit),
        .cmdMemset(colorBufferCmdMemset),

        .m_axis_tvalid(s_framebuffer_axis_tvalid),
        .m_axis_tready(s_framebuffer_axis_tready),
        .m_axis_tlast(s_framebuffer_axis_tlast),
        .m_axis_tdata(s_framebuffer_unconverted_axis_tdata)
    );
    defparam colorBuffer.NUMBER_OF_PIXELS_PER_BEAT = PIXEL_PER_BEAT; 
    defparam colorBuffer.NUMBER_OF_SUB_PIXELS = FRAMEBUFFER_NUMBER_OF_SUB_PIXELS;
    defparam colorBuffer.SUB_PIXEL_WIDTH = FRAMEBUFFER_SUB_PIXEL_WIDTH;
    defparam colorBuffer.X_BIT_WIDTH = RENDER_CONFIG_X_SIZE;
    defparam colorBuffer.Y_BIT_WIDTH = RENDER_CONFIG_Y_SIZE;
    defparam colorBuffer.FRAMEBUFFER_SIZE_IN_WORDS = FRAMEBUFFER_SIZE_IN_WORDS;

    // Conversion of the internal pixel representation the exnternal one required for the AXIS interface
    generate
        `XXX2RGB565(XXX2RGB565, COLOR_SUB_PIXEL_WIDTH, PIXEL_PER_BEAT);
        `Expand(ExpandFramebufferStream, FRAMEBUFFER_SUB_PIXEL_WIDTH, COLOR_SUB_PIXEL_WIDTH, PIXEL_PER_BEAT * 3);
        if (FRAMEBUFFER_NUMBER_OF_SUB_PIXELS == 4)
        begin
            `ReduceVec(ReduceVecFramebufferStream, FRAMEBUFFER_SUB_PIXEL_WIDTH, PIXEL_PER_BEAT * COLOR_NUMBER_OF_SUB_PIXEL, COLOR_A_POS, COLOR_NUMBER_OF_SUB_PIXEL, PIXEL_PER_BEAT * 3);
            assign s_framebuffer_axis_tdata = XXX2RGB565(ExpandFramebufferStream(ReduceVecFramebufferStream(s_framebuffer_unconverted_axis_tdata)));
        end
        else
        begin
            assign s_framebuffer_axis_tdata = XXX2RGB565(ExpandFramebufferStream(s_framebuffer_unconverted_axis_tdata));
        end
    endgenerate

    generate 
        if (ENABLE_STENCIL_BUFFER)
        begin
            FrameBuffer stencilBuffer (  
                .clk(aclk),
                .reset(!resetn),

                .confEnable(stencilBufferEnable),
                .confClearColor(stencilBufferClearStencil),
                .confEnableScissor(framebufferParamEnableScissor),
                .confScissorStartX(framebufferParamScissorStartX),
                .confScissorStartY(framebufferParamScissorStartY),
                .confScissorEndX(framebufferParamScissorEndX),
                .confScissorEndY(framebufferParamScissorEndY),
                .confYOffset(framebufferParamYOffset),
                .confXResolution(framebufferParamXResolution),
                .confYResolution(framebufferParamYResolution),
                .confMask(stencilBufferMask),

                .araddr(stencil_araddr),
                .arvalid(stencil_arvalid),
                .arlast(stencil_arlast),
                .rvalid(stencil_rvalid),
                .rlast(stencil_rlast),
                .rdata(stencil_rdata),
                .waddr(stencil_waddr),
                .wdata(stencil_wdata),
                .wvalid(stencil_wvalid),
                .wstrb(stencil_wstrb),
                .wscreenPosX(stencil_wscreenPosX),
                .wscreenPosY(stencil_wscreenPosY),

                .apply(stencilBufferApply),
                .applied(stencilBufferApplied),
                .cmdCommit(stencilBufferCmdCommit),
                .cmdMemset(stencilBufferCmdMemset),

                .m_axis_tvalid(),
                .m_axis_tready(1'b1),
                .m_axis_tlast(),
                .m_axis_tdata()
            );
            defparam stencilBuffer.NUMBER_OF_PIXELS_PER_BEAT = PIXEL_PER_BEAT;
            defparam stencilBuffer.NUMBER_OF_SUB_PIXELS = STENCIL_WIDTH;
            defparam stencilBuffer.SUB_PIXEL_WIDTH = 1;
            defparam stencilBuffer.X_BIT_WIDTH = RENDER_CONFIG_X_SIZE;
            defparam stencilBuffer.Y_BIT_WIDTH = RENDER_CONFIG_Y_SIZE;
            defparam stencilBuffer.FRAMEBUFFER_SIZE_IN_WORDS = FRAMEBUFFER_SIZE_IN_WORDS;
        end
        else
        begin
            assign stencil_rdata = 0;
            assign stencilBufferApplied = 1;
        end
    endgenerate

    RasterixRenderCore #(
        .INDEX_WIDTH(FRAMEBUFFER_SIZE_IN_WORDS),
        .CMD_STREAM_WIDTH(CMD_STREAM_WIDTH),
        .TEXTURE_BUFFER_SIZE(TEXTURE_BUFFER_SIZE),
        .TMU_COUNT(TMU_COUNT),
        .ENABLE_FLOW_CTRL(0)
    ) graphicCore (
        .aclk(aclk),
        .resetn(resetn),
        
        .s_cmd_axis_tvalid(m_cmd_axis_tvalid),
        .s_cmd_axis_tready(m_cmd_axis_tready),
        .s_cmd_axis_tlast(m_cmd_axis_tlast),
        .s_cmd_axis_tdata(m_cmd_axis_tdata),

        .framebufferParamEnableScissor(framebufferParamEnableScissor),
        .framebufferParamScissorStartX(framebufferParamScissorStartX),
        .framebufferParamScissorStartY(framebufferParamScissorStartY),
        .framebufferParamScissorEndX(framebufferParamScissorEndX),
        .framebufferParamScissorEndY(framebufferParamScissorEndY),
        .framebufferParamYOffset(framebufferParamYOffset),
        .framebufferParamXResolution(framebufferParamXResolution),
        .framebufferParamYResolution(framebufferParamYResolution),

        .colorBufferClearColor(colorBufferClearColor),
        .colorBufferApply(colorBufferApply),
        .colorBufferApplied(colorBufferApplied),
        .colorBufferCmdCommit(colorBufferCmdCommit),
        .colorBufferCmdMemset(colorBufferCmdMemset),
        .colorBufferEnable(colorBufferEnable),
        .colorBufferMask(colorBufferMask),
        .color_arready(1),
        .color_arlast(),
        .color_arvalid(color_arvalid),
        .color_araddr(color_araddr),
        .color_rready(),
        .color_rdata(ColorBufferExpandVec(ColorBufferExpand(color_rdata), DEFAULT_ALPHA_VAL)),
        .color_rvalid(color_rvalid),
        .color_waddr(color_waddr),
        .color_wvalid(color_wvalid),
        .color_wready(1),
        .color_wdata(color_wdata),
        .color_wstrb(color_wstrb),
        .color_wlast(),
        .color_wscreenPosX(color_wscreenPosX),
        .color_wscreenPosY(color_wscreenPosY),

        .depthBufferClearDepth(depthBufferClearDepth),
        .depthBufferApply(depthBufferApply),
        .depthBufferApplied(depthBufferApplied),
        .depthBufferCmdCommit(depthBufferCmdCommit),
        .depthBufferCmdMemset(depthBufferCmdMemset),
        .depthBufferEnable(depthBufferEnable),
        .depthBufferMask(depthBufferMask),
        .depth_arready(1),
        .depth_arlast(),
        .depth_arvalid(depth_arvalid),
        .depth_araddr(depth_araddr),
        .depth_rready(),
        .depth_rdata(depth_rdata),
        .depth_rvalid(depth_rvalid),
        .depth_waddr(depth_waddr),
        .depth_wvalid(depth_wvalid),
        .depth_wready(1),
        .depth_wdata(depth_wdata),
        .depth_wstrb(depth_wstrb),
        .depth_wlast(),
        .depth_wscreenPosX(depth_wscreenPosX),
        .depth_wscreenPosY(depth_wscreenPosY),

        .stencilBufferClearStencil(stencilBufferClearStencil),
        .stencilBufferApply(stencilBufferApply),
        .stencilBufferApplied(stencilBufferApplied),
        .stencilBufferCmdCommit(stencilBufferCmdCommit),
        .stencilBufferCmdMemset(stencilBufferCmdMemset),
        .stencilBufferEnable(stencilBufferEnable),
        .stencilBufferMask(stencilBufferMask),
        .stencil_arready(1),
        .stencil_arlast(),
        .stencil_arvalid(stencil_arvalid),
        .stencil_araddr(stencil_araddr),
        .stencil_rready(),
        .stencil_rdata(stencil_rdata),
        .stencil_rvalid(stencil_rvalid),
        .stencil_waddr(stencil_waddr),
        .stencil_wvalid(stencil_wvalid),
        .stencil_wready(1),
        .stencil_wdata(stencil_wdata),
        .stencil_wstrb(stencil_wstrb),
        .stencil_wlast(),
        .stencil_wscreenPosX(stencil_wscreenPosX),
        .stencil_wscreenPosY(stencil_wscreenPosY)
    );

endmodule
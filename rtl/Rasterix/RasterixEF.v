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

module RasterixEF #(
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
    localparam PIXEL_WIDTH_FRAMEBUFFER = 16,

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
    parameter ADDR_WIDTH = 32,

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
    output wire                             swap_fb,
    input  wire                             fb_swapped,

    // Common memory interface
    output wire [ID_WIDTH - 1 : 0]          m_common_axi_awid,
    output wire [ADDR_WIDTH - 1 : 0]        m_common_axi_awaddr,
    output wire [ 7 : 0]                    m_common_axi_awlen,
    output wire [ 2 : 0]                    m_common_axi_awsize,
    output wire [ 1 : 0]                    m_common_axi_awburst,
    output wire                             m_common_axi_awlock,
    output wire [ 3 : 0]                    m_common_axi_awcache,
    output wire [ 2 : 0]                    m_common_axi_awprot, 
    output wire                             m_common_axi_awvalid,
    input  wire                             m_common_axi_awready,

    output wire [CMD_STREAM_WIDTH - 1 : 0]  m_common_axi_wdata,
    output wire [STRB_WIDTH - 1 : 0]        m_common_axi_wstrb,
    output wire                             m_common_axi_wlast,
    output wire                             m_common_axi_wvalid,
    input  wire                             m_common_axi_wready,

    input  wire [ID_WIDTH - 1 : 0]          m_common_axi_bid,
    input  wire [ 1 : 0]                    m_common_axi_bresp,
    input  wire                             m_common_axi_bvalid,
    output wire                             m_common_axi_bready,

    output wire [ID_WIDTH - 1 : 0]          m_common_axi_arid,
    output wire [ADDR_WIDTH - 1 : 0]        m_common_axi_araddr,
    output wire [ 7 : 0]                    m_common_axi_arlen,
    output wire [ 2 : 0]                    m_common_axi_arsize,
    output wire [ 1 : 0]                    m_common_axi_arburst,
    output wire                             m_common_axi_arlock,
    output wire [ 3 : 0]                    m_common_axi_arcache,
    output wire [ 2 : 0]                    m_common_axi_arprot,
    output wire                             m_common_axi_arvalid,
    input  wire                             m_common_axi_arready,

    input  wire [ID_WIDTH - 1 : 0]          m_common_axi_rid,
    input  wire [CMD_STREAM_WIDTH - 1 : 0]  m_common_axi_rdata,
    input  wire [ 1 : 0]                    m_common_axi_rresp,
    input  wire                             m_common_axi_rlast,
    input  wire                             m_common_axi_rvalid,
    output wire                             m_common_axi_rready,

    // Color Buffer
    output wire [ID_WIDTH - 1 : 0]          m_color_axi_awid,
    output wire [ADDR_WIDTH - 1 : 0]        m_color_axi_awaddr,
    output wire [ 7 : 0]                    m_color_axi_awlen,
    output wire [ 2 : 0]                    m_color_axi_awsize,
    output wire [ 1 : 0]                    m_color_axi_awburst,
    output wire                             m_color_axi_awlock,
    output wire [ 3 : 0]                    m_color_axi_awcache,
    output wire [ 2 : 0]                    m_color_axi_awprot, 
    output wire                             m_color_axi_awvalid,
    input  wire                             m_color_axi_awready,

    output wire [CMD_STREAM_WIDTH - 1 : 0]  m_color_axi_wdata,
    output wire [STRB_WIDTH - 1 : 0]        m_color_axi_wstrb,
    output wire                             m_color_axi_wlast,
    output wire                             m_color_axi_wvalid,
    input  wire                             m_color_axi_wready,

    input  wire [ID_WIDTH - 1 : 0]          m_color_axi_bid,
    input  wire [ 1 : 0]                    m_color_axi_bresp,
    input  wire                             m_color_axi_bvalid,
    output wire                             m_color_axi_bready,

    output wire [ID_WIDTH - 1 : 0]          m_color_axi_arid,
    output wire [ADDR_WIDTH - 1 : 0]        m_color_axi_araddr,
    output wire [ 7 : 0]                    m_color_axi_arlen,
    output wire [ 2 : 0]                    m_color_axi_arsize,
    output wire [ 1 : 0]                    m_color_axi_arburst,
    output wire                             m_color_axi_arlock,
    output wire [ 3 : 0]                    m_color_axi_arcache,
    output wire [ 2 : 0]                    m_color_axi_arprot,
    output wire                             m_color_axi_arvalid,
    input  wire                             m_color_axi_arready,

    input  wire [ID_WIDTH - 1 : 0]          m_color_axi_rid,
    input  wire [CMD_STREAM_WIDTH - 1 : 0]  m_color_axi_rdata,
    input  wire [ 1 : 0]                    m_color_axi_rresp,
    input  wire                             m_color_axi_rlast,
    input  wire                             m_color_axi_rvalid,
    output wire                             m_color_axi_rready,

    // Depth Buffer
    output wire [ID_WIDTH - 1 : 0]          m_depth_axi_awid,
    output wire [ADDR_WIDTH - 1 : 0]        m_depth_axi_awaddr,
    output wire [ 7 : 0]                    m_depth_axi_awlen,
    output wire [ 2 : 0]                    m_depth_axi_awsize,
    output wire [ 1 : 0]                    m_depth_axi_awburst,
    output wire                             m_depth_axi_awlock,
    output wire [ 3 : 0]                    m_depth_axi_awcache,
    output wire [ 2 : 0]                    m_depth_axi_awprot, 
    output wire                             m_depth_axi_awvalid,
    input  wire                             m_depth_axi_awready,

    output wire [CMD_STREAM_WIDTH - 1 : 0]  m_depth_axi_wdata,
    output wire [STRB_WIDTH - 1 : 0]        m_depth_axi_wstrb,
    output wire                             m_depth_axi_wlast,
    output wire                             m_depth_axi_wvalid,
    input  wire                             m_depth_axi_wready,

    input  wire [ID_WIDTH - 1 : 0]          m_depth_axi_bid,
    input  wire [ 1 : 0]                    m_depth_axi_bresp,
    input  wire                             m_depth_axi_bvalid,
    output wire                             m_depth_axi_bready,

    output wire [ID_WIDTH - 1 : 0]          m_depth_axi_arid,
    output wire [ADDR_WIDTH - 1 : 0]        m_depth_axi_araddr,
    output wire [ 7 : 0]                    m_depth_axi_arlen,
    output wire [ 2 : 0]                    m_depth_axi_arsize,
    output wire [ 1 : 0]                    m_depth_axi_arburst,
    output wire                             m_depth_axi_arlock,
    output wire [ 3 : 0]                    m_depth_axi_arcache,
    output wire [ 2 : 0]                    m_depth_axi_arprot,
    output wire                             m_depth_axi_arvalid,
    input  wire                             m_depth_axi_arready,

    input  wire [ID_WIDTH - 1 : 0]          m_depth_axi_rid,
    input  wire [CMD_STREAM_WIDTH - 1 : 0]  m_depth_axi_rdata,
    input  wire [ 1 : 0]                    m_depth_axi_rresp,
    input  wire                             m_depth_axi_rlast,
    input  wire                             m_depth_axi_rvalid,
    output wire                             m_depth_axi_rready,

    // Stencil Buffer
    output wire [ID_WIDTH - 1 : 0]          m_stencil_axi_awid,
    output wire [ADDR_WIDTH - 1 : 0]        m_stencil_axi_awaddr,
    output wire [ 7 : 0]                    m_stencil_axi_awlen,
    output wire [ 2 : 0]                    m_stencil_axi_awsize,
    output wire [ 1 : 0]                    m_stencil_axi_awburst,
    output wire                             m_stencil_axi_awlock,
    output wire [ 3 : 0]                    m_stencil_axi_awcache,
    output wire [ 2 : 0]                    m_stencil_axi_awprot, 
    output wire                             m_stencil_axi_awvalid,
    input  wire                             m_stencil_axi_awready,

    output wire [CMD_STREAM_WIDTH - 1 : 0]  m_stencil_axi_wdata,
    output wire [STRB_WIDTH - 1 : 0]        m_stencil_axi_wstrb,
    output wire                             m_stencil_axi_wlast,
    output wire                             m_stencil_axi_wvalid,
    input  wire                             m_stencil_axi_wready,

    input  wire [ID_WIDTH - 1 : 0]          m_stencil_axi_bid,
    input  wire [ 1 : 0]                    m_stencil_axi_bresp,
    input  wire                             m_stencil_axi_bvalid,
    output wire                             m_stencil_axi_bready,

    output wire [ID_WIDTH - 1 : 0]          m_stencil_axi_arid,
    output wire [ADDR_WIDTH - 1 : 0]        m_stencil_axi_araddr,
    output wire [ 7 : 0]                    m_stencil_axi_arlen,
    output wire [ 2 : 0]                    m_stencil_axi_arsize,
    output wire [ 1 : 0]                    m_stencil_axi_arburst,
    output wire                             m_stencil_axi_arlock,
    output wire [ 3 : 0]                    m_stencil_axi_arcache,
    output wire [ 2 : 0]                    m_stencil_axi_arprot,
    output wire                             m_stencil_axi_arvalid,
    input  wire                             m_stencil_axi_arready,

    input  wire [ID_WIDTH - 1 : 0]          m_stencil_axi_rid,
    input  wire [CMD_STREAM_WIDTH - 1 : 0]  m_stencil_axi_rdata,
    input  wire [ 1 : 0]                    m_stencil_axi_rresp,
    input  wire                             m_stencil_axi_rlast,
    input  wire                             m_stencil_axi_rvalid,
    output wire                             m_stencil_axi_rready
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
    `ReduceVec(ColorBufferReduceVec, COLOR_SUB_PIXEL_WIDTH, COLOR_NUMBER_OF_SUB_PIXEL, SUB_PIXEL_OFFSET, COLOR_NUMBER_OF_SUB_PIXEL, FRAMEBUFFER_NUMBER_OF_SUB_PIXELS);
    `ReduceVec(ColorBufferReduceMask, 1, COLOR_NUMBER_OF_SUB_PIXEL, SUB_PIXEL_OFFSET, COLOR_NUMBER_OF_SUB_PIXEL, FRAMEBUFFER_NUMBER_OF_SUB_PIXELS);
    `ExpandVec(ColorBufferExpandVec, COLOR_SUB_PIXEL_WIDTH, FRAMEBUFFER_NUMBER_OF_SUB_PIXELS, SUB_PIXEL_OFFSET, COLOR_NUMBER_OF_SUB_PIXEL, COLOR_NUMBER_OF_SUB_PIXEL);
    `Reduce(ColorBufferReduce, FRAMEBUFFER_SUB_PIXEL_WIDTH, COLOR_SUB_PIXEL_WIDTH, FRAMEBUFFER_NUMBER_OF_SUB_PIXELS);
    `XXX2RGB565(XXX2RGB565, COLOR_SUB_PIXEL_WIDTH, 1);
    `RGB5652XXX(RGB5652XXX, COLOR_SUB_PIXEL_WIDTH, 1);

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
        .ADDR_WIDTH(28),
        .ID_WIDTH(ID_WIDTH)
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

        .m_mem_axi_awid(m_common_axi_awid),
        .m_mem_axi_awaddr(m_common_axi_awaddr),
        .m_mem_axi_awlen(m_common_axi_awlen), 
        .m_mem_axi_awsize(m_common_axi_awsize), 
        .m_mem_axi_awburst(m_common_axi_awburst), 
        .m_mem_axi_awlock(m_common_axi_awlock), 
        .m_mem_axi_awcache(m_common_axi_awcache), 
        .m_mem_axi_awprot(m_common_axi_awprot), 
        .m_mem_axi_awvalid(m_common_axi_awvalid),
        .m_mem_axi_awready(m_common_axi_awready),

        .m_mem_axi_wdata(m_common_axi_wdata),
        .m_mem_axi_wstrb(m_common_axi_wstrb),
        .m_mem_axi_wlast(m_common_axi_wlast),
        .m_mem_axi_wvalid(m_common_axi_wvalid),
        .m_mem_axi_wready(m_common_axi_wready),

        .m_mem_axi_bid(m_common_axi_bid),
        .m_mem_axi_bresp(m_common_axi_bresp),
        .m_mem_axi_bvalid(m_common_axi_bvalid),
        .m_mem_axi_bready(m_common_axi_bready),

        .m_mem_axi_arid(m_common_axi_arid),
        .m_mem_axi_araddr(m_common_axi_araddr),
        .m_mem_axi_arlen(m_common_axi_arlen),
        .m_mem_axi_arsize(m_common_axi_arsize),
        .m_mem_axi_arburst(m_common_axi_arburst),
        .m_mem_axi_arlock(m_common_axi_arlock),
        .m_mem_axi_arcache(m_common_axi_arcache),
        .m_mem_axi_arprot(m_common_axi_arprot),
        .m_mem_axi_arvalid(m_common_axi_arvalid),
        .m_mem_axi_arready(m_common_axi_arready),

        .m_mem_axi_rid(m_common_axi_rid),
        .m_mem_axi_rdata(m_common_axi_rdata),
        .m_mem_axi_rresp(m_common_axi_rresp),
        .m_mem_axi_rlast(m_common_axi_rlast),
        .m_mem_axi_rvalid(m_common_axi_rvalid),
        .m_mem_axi_rready(m_common_axi_rready)
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
    wire [ADDR_WIDTH - 1 : 0]                        colorBufferAddr;
    wire                                             colorBufferApply;
    wire                                             colorBufferApplied;
    wire                                             colorBufferCmdCommit;
    wire                                             colorBufferCmdMemset;
    wire                                             colorBufferEnable;
    wire [3 : 0]                                     colorBufferMask;
    wire                                             color_arvalid;
    wire                                             color_arlast;
    wire                                             color_arready;
    wire                                             color_rvalid;
    wire                                             color_rready;
    wire                                             color_rlast;
    wire [FRAMEBUFFER_SIZE_IN_WORDS - 1 : 0]         color_araddr;
    wire [FRAMEBUFFER_SIZE_IN_WORDS - 1 : 0]         color_waddr;
    wire                                             color_wvalid;
    wire                                             color_wready;
    wire [PIXEL_WIDTH_FRAMEBUFFER - 1 : 0]           color_rdata;
    wire [PIPELINE_PIXEL_WIDTH - 1 : 0]              color_wdata;
    wire                                             color_wstrb;
    wire                                             color_wlast;
    wire [SCREEN_POS_WIDTH - 1 : 0]                  color_wscreenPosX;
    wire [SCREEN_POS_WIDTH - 1 : 0]                  color_wscreenPosY;

    // Depth buffer access
    wire [DEPTH_WIDTH - 1 : 0]                       depthBufferClearDepth;
    wire [ADDR_WIDTH - 1 : 0]                        depthBufferAddr;
    wire                                             depthBufferApply;
    wire                                             depthBufferApplied;
    wire                                             depthBufferCmdCommit;
    wire                                             depthBufferCmdMemset;
    wire                                             depthBufferEnable;
    wire                                             depthBufferMask;
    wire                                             depth_arvalid;
    wire                                             depth_arlast;
    wire                                             depth_arready;
    wire                                             depth_rvalid;
    wire                                             depth_rready;
    wire                                             depth_rlast;
    wire [FRAMEBUFFER_SIZE_IN_WORDS - 1 : 0]         depth_araddr;
    wire [FRAMEBUFFER_SIZE_IN_WORDS - 1 : 0]         depth_waddr;
    wire                                             depth_wvalid;
    wire                                             depth_wready;
    wire [DEPTH_WIDTH - 1 : 0]                       depth_rdata;
    wire [DEPTH_WIDTH - 1 : 0]                       depth_wdata;
    wire                                             depth_wstrb;
    wire                                             depth_wlast;
    wire [SCREEN_POS_WIDTH - 1 : 0]                  depth_wscreenPosX;
    wire [SCREEN_POS_WIDTH - 1 : 0]                  depth_wscreenPosY;

    // Stencil buffer access
    wire [STENCIL_WIDTH - 1 : 0]                     stencilBufferClearStencil;
    wire [ADDR_WIDTH - 1 : 0]                        stencilBufferAddr;
    wire                                             stencilBufferApply;
    wire                                             stencilBufferApplied;
    wire                                             stencilBufferCmdCommit;
    wire                                             stencilBufferCmdMemset;
    wire                                             stencilBufferEnable;
    wire [STENCIL_WIDTH - 1 : 0]                     stencilBufferMask;
    wire                                             stencil_arvalid;
    wire                                             stencil_arlast;
    wire                                             stencil_arready;
    wire                                             stencil_rvalid;
    wire                                             stencil_rready;
    wire                                             stencil_rlast;
    wire [FRAMEBUFFER_SIZE_IN_WORDS - 1 : 0]         stencil_araddr;
    wire [FRAMEBUFFER_SIZE_IN_WORDS - 1 : 0]         stencil_waddr;
    wire                                             stencil_wvalid;
    wire                                             stencil_wready;
    wire [STENCIL_WIDTH - 1 : 0]                     stencil_rdata;
    wire [STENCIL_WIDTH - 1 : 0]                     stencil_wdata;
    wire                                             stencil_wstrb;
    wire                                             stencil_wlast;
    wire [SCREEN_POS_WIDTH - 1 : 0]                  stencil_wscreenPosX;
    wire [SCREEN_POS_WIDTH - 1 : 0]                  stencil_wscreenPosY;

    StreamFramebuffer #(
        .STREAM_WIDTH(CMD_STREAM_WIDTH),
        .ADDR_WIDTH(ADDR_WIDTH),
        .ID_WIDTH(ID_WIDTH),
        .X_BIT_WIDTH(RENDER_CONFIG_X_SIZE),
        .Y_BIT_WIDTH(RENDER_CONFIG_Y_SIZE),
        .PIXEL_WIDTH(DEPTH_WIDTH)
    ) depthBuffer (
        .aclk(aclk),
        .resetn(resetn),

        .confAddr(depthBufferAddr),
        .confEnable(depthBufferEnable),
        .confEnableScissor(framebufferParamEnableScissor),
        .confScissorStartX(framebufferParamScissorStartX),
        .confScissorStartY(framebufferParamScissorStartY),
        .confScissorEndX(framebufferParamScissorEndX),
        .confScissorEndY(framebufferParamScissorEndY),
        .confXResolution(framebufferParamXResolution),
        .confYResolution(framebufferParamYResolution),
        .confMask({ depthBufferMask, depthBufferMask }),
        .confClearColor(depthBufferClearDepth),

        .apply(depthBufferApply & depthBufferCmdMemset),
        .applied(depthBufferApplied),

        .fetch_arvalid(depth_arvalid),
        .fetch_arlast(depth_arlast),
        .fetch_arready(depth_arready),
        .fetch_araddr(depth_araddr),

        .frag_rvalid(depth_rvalid),
        .frag_rready(depth_rready),
        .frag_rdata(depth_rdata),
        .frag_rlast(depth_rlast),

        .frag_wvalid(depth_wvalid),
        .frag_wlast(depth_wlast),
        .frag_wready(depth_wready),
        .frag_wdata(depth_wdata),
        .frag_wstrb(depth_wstrb),
        .frag_waddr(depth_waddr),
        .frag_wxpos(depth_wscreenPosX),
        .frag_wypos(depth_wscreenPosY),

        .m_mem_axi_awid(m_depth_axi_awid),
        .m_mem_axi_awaddr(m_depth_axi_awaddr),
        .m_mem_axi_awlen(m_depth_axi_awlen), 
        .m_mem_axi_awsize(m_depth_axi_awsize), 
        .m_mem_axi_awburst(m_depth_axi_awburst), 
        .m_mem_axi_awlock(m_depth_axi_awlock), 
        .m_mem_axi_awcache(m_depth_axi_awcache), 
        .m_mem_axi_awprot(m_depth_axi_awprot), 
        .m_mem_axi_awvalid(m_depth_axi_awvalid),
        .m_mem_axi_awready(m_depth_axi_awready),

        .m_mem_axi_wdata(m_depth_axi_wdata),
        .m_mem_axi_wstrb(m_depth_axi_wstrb),
        .m_mem_axi_wlast(m_depth_axi_wlast),
        .m_mem_axi_wvalid(m_depth_axi_wvalid),
        .m_mem_axi_wready(m_depth_axi_wready),

        .m_mem_axi_bid(m_depth_axi_bid),
        .m_mem_axi_bresp(m_depth_axi_bresp),
        .m_mem_axi_bvalid(m_depth_axi_bvalid),
        .m_mem_axi_bready(m_depth_axi_bready),

        .m_mem_axi_arid(m_depth_axi_arid),
        .m_mem_axi_araddr(m_depth_axi_araddr),
        .m_mem_axi_arlen(m_depth_axi_arlen),
        .m_mem_axi_arsize(m_depth_axi_arsize),
        .m_mem_axi_arburst(m_depth_axi_arburst),
        .m_mem_axi_arlock(m_depth_axi_arlock),
        .m_mem_axi_arcache(m_depth_axi_arcache),
        .m_mem_axi_arprot(m_depth_axi_arprot),
        .m_mem_axi_arvalid(m_depth_axi_arvalid),
        .m_mem_axi_arready(m_depth_axi_arready),

        .m_mem_axi_rid(m_depth_axi_rid),
        .m_mem_axi_rdata(m_depth_axi_rdata),
        .m_mem_axi_rresp(m_depth_axi_rresp),
        .m_mem_axi_rlast(m_depth_axi_rlast),
        .m_mem_axi_rvalid(m_depth_axi_rvalid),
        .m_mem_axi_rready(m_depth_axi_rready)
    );

    StreamFramebuffer #(
        .STREAM_WIDTH(CMD_STREAM_WIDTH),
        .ADDR_WIDTH(ADDR_WIDTH),
        .ID_WIDTH(ID_WIDTH),
        .X_BIT_WIDTH(RENDER_CONFIG_X_SIZE),
        .Y_BIT_WIDTH(RENDER_CONFIG_Y_SIZE),
        .PIXEL_WIDTH(PIXEL_WIDTH_STREAM)
    ) colorBuffer (
        .aclk(aclk),
        .resetn(resetn),

        .confAddr(colorBufferAddr),
        .confEnable(colorBufferEnable),
        .confEnableScissor(framebufferParamEnableScissor),
        .confScissorStartX(framebufferParamScissorStartX),
        .confScissorStartY(framebufferParamScissorStartY),
        .confScissorEndX(framebufferParamScissorEndX),
        .confScissorEndY(framebufferParamScissorEndY),
        .confXResolution(framebufferParamXResolution),
        .confYResolution(framebufferParamYResolution),
        .confMask(ColorBufferReduceMask(colorBufferMask)),
        .confClearColor(XXX2RGB565(ColorBufferReduceVec(colorBufferClearColor))),

        .apply(colorBufferApply & colorBufferCmdMemset),
        .applied(colorBufferApplied),

        .fetch_arvalid(color_arvalid),
        .fetch_arlast(color_arlast),
        .fetch_arready(color_arready),
        .fetch_araddr(color_araddr),

        .frag_rvalid(color_rvalid),
        .frag_rready(color_rready),
        .frag_rdata(color_rdata),
        .frag_rlast(color_rlast),

        .frag_wvalid(color_wvalid),
        .frag_wlast(color_wlast),
        .frag_wready(color_wready),
        .frag_wdata(XXX2RGB565(ColorBufferReduceVec(color_wdata))),
        .frag_wstrb(color_wstrb),
        .frag_waddr(color_waddr),
        .frag_wxpos(color_wscreenPosX),
        .frag_wypos(color_wscreenPosY),

        .m_mem_axi_awid(m_color_axi_awid),
        .m_mem_axi_awaddr(m_color_axi_awaddr),
        .m_mem_axi_awlen(m_color_axi_awlen), 
        .m_mem_axi_awsize(m_color_axi_awsize), 
        .m_mem_axi_awburst(m_color_axi_awburst), 
        .m_mem_axi_awlock(m_color_axi_awlock), 
        .m_mem_axi_awcache(m_color_axi_awcache), 
        .m_mem_axi_awprot(m_color_axi_awprot), 
        .m_mem_axi_awvalid(m_color_axi_awvalid),
        .m_mem_axi_awready(m_color_axi_awready),

        .m_mem_axi_wdata(m_color_axi_wdata),
        .m_mem_axi_wstrb(m_color_axi_wstrb),
        .m_mem_axi_wlast(m_color_axi_wlast),
        .m_mem_axi_wvalid(m_color_axi_wvalid),
        .m_mem_axi_wready(m_color_axi_wready),

        .m_mem_axi_bid(m_color_axi_bid),
        .m_mem_axi_bresp(m_color_axi_bresp),
        .m_mem_axi_bvalid(m_color_axi_bvalid),
        .m_mem_axi_bready(m_color_axi_bready),

        .m_mem_axi_arid(m_color_axi_arid),
        .m_mem_axi_araddr(m_color_axi_araddr),
        .m_mem_axi_arlen(m_color_axi_arlen),
        .m_mem_axi_arsize(m_color_axi_arsize),
        .m_mem_axi_arburst(m_color_axi_arburst),
        .m_mem_axi_arlock(m_color_axi_arlock),
        .m_mem_axi_arcache(m_color_axi_arcache),
        .m_mem_axi_arprot(m_color_axi_arprot),
        .m_mem_axi_arvalid(m_color_axi_arvalid),
        .m_mem_axi_arready(m_color_axi_arready),

        .m_mem_axi_rid(m_color_axi_rid),
        .m_mem_axi_rdata(m_color_axi_rdata),
        .m_mem_axi_rresp(m_color_axi_rresp),
        .m_mem_axi_rlast(m_color_axi_rlast),
        .m_mem_axi_rvalid(m_color_axi_rvalid),
        .m_mem_axi_rready(m_color_axi_rready)
    );

    generate 
        if (ENABLE_STENCIL_BUFFER)
        begin
            StreamFramebuffer #(
                .STREAM_WIDTH(CMD_STREAM_WIDTH),
                .ADDR_WIDTH(ADDR_WIDTH),
                .ID_WIDTH(ID_WIDTH),
                .X_BIT_WIDTH(RENDER_CONFIG_X_SIZE),
                .Y_BIT_WIDTH(RENDER_CONFIG_Y_SIZE),
                .PIXEL_WIDTH(8)
            ) stencilBuffer (
                .aclk(aclk),
                .resetn(resetn),

                .confAddr(stencilBufferAddr),
                .confEnable(stencilBufferEnable),
                .confEnableScissor(framebufferParamEnableScissor),
                .confScissorStartX(framebufferParamScissorStartX),
                .confScissorStartY(framebufferParamScissorStartY),
                .confScissorEndX(framebufferParamScissorEndX),
                .confScissorEndY(framebufferParamScissorEndY),
                .confXResolution(framebufferParamXResolution),
                .confYResolution(framebufferParamYResolution),
                .confMask(|stencilBufferMask),
                .confClearColor(stencilBufferClearStencil),

                .apply(stencilBufferApply & stencilBufferCmdMemset),
                .applied(stencilBufferApplied),

                .fetch_arvalid(stencil_arvalid),
                .fetch_arlast(stencil_arlast),
                .fetch_arready(stencil_arready),
                .fetch_araddr(stencil_araddr),

                .frag_rvalid(stencil_rvalid),
                .frag_rready(stencil_rready),
                .frag_rdata(stencil_rdata),
                .frag_rlast(stencil_rlast),

                .frag_wvalid(stencil_wvalid),
                .frag_wlast(stencil_wlast),
                .frag_wready(stencil_wready),
                .frag_wdata(stencil_wdata),
                .frag_wstrb(stencil_wstrb),
                .frag_waddr(stencil_waddr),
                .frag_wxpos(stencil_wscreenPosX),
                .frag_wypos(stencil_wscreenPosY),

                .m_mem_axi_awid(m_stencil_axi_awid),
                .m_mem_axi_awaddr(m_stencil_axi_awaddr),
                .m_mem_axi_awlen(m_stencil_axi_awlen), 
                .m_mem_axi_awsize(m_stencil_axi_awsize), 
                .m_mem_axi_awburst(m_stencil_axi_awburst), 
                .m_mem_axi_awlock(m_stencil_axi_awlock), 
                .m_mem_axi_awcache(m_stencil_axi_awcache), 
                .m_mem_axi_awprot(m_stencil_axi_awprot), 
                .m_mem_axi_awvalid(m_stencil_axi_awvalid),
                .m_mem_axi_awready(m_stencil_axi_awready),

                .m_mem_axi_wdata(m_stencil_axi_wdata),
                .m_mem_axi_wstrb(m_stencil_axi_wstrb),
                .m_mem_axi_wlast(m_stencil_axi_wlast),
                .m_mem_axi_wvalid(m_stencil_axi_wvalid),
                .m_mem_axi_wready(m_stencil_axi_wready),

                .m_mem_axi_bid(m_stencil_axi_bid),
                .m_mem_axi_bresp(m_stencil_axi_bresp),
                .m_mem_axi_bvalid(m_stencil_axi_bvalid),
                .m_mem_axi_bready(m_stencil_axi_bready),

                .m_mem_axi_arid(m_stencil_axi_arid),
                .m_mem_axi_araddr(m_stencil_axi_araddr),
                .m_mem_axi_arlen(m_stencil_axi_arlen),
                .m_mem_axi_arsize(m_stencil_axi_arsize),
                .m_mem_axi_arburst(m_stencil_axi_arburst),
                .m_mem_axi_arlock(m_stencil_axi_arlock),
                .m_mem_axi_arcache(m_stencil_axi_arcache),
                .m_mem_axi_arprot(m_stencil_axi_arprot),
                .m_mem_axi_arvalid(m_stencil_axi_arvalid),
                .m_mem_axi_arready(m_stencil_axi_arready),

                .m_mem_axi_rid(m_stencil_axi_rid),
                .m_mem_axi_rdata(m_stencil_axi_rdata),
                .m_mem_axi_rresp(m_stencil_axi_rresp),
                .m_mem_axi_rlast(m_stencil_axi_rlast),
                .m_mem_axi_rvalid(m_stencil_axi_rvalid),
                .m_mem_axi_rready(m_stencil_axi_rready)
            );
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
        .TMU_COUNT(TMU_COUNT)
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
        .colorBufferAddr(colorBufferAddr),
        .colorBufferApply(colorBufferApply),
        .colorBufferApplied(colorBufferApplied && fb_swapped),
        .colorBufferCmdCommit(colorBufferCmdCommit),
        .colorBufferCmdMemset(colorBufferCmdMemset),
        .colorBufferEnable(colorBufferEnable),
        .colorBufferMask(colorBufferMask),
        .color_arready(color_arready),
        .color_arlast(color_arlast),
        .color_arvalid(color_arvalid),
        .color_araddr(color_araddr),
        .color_rready(color_rready),
        .color_rdata(ColorBufferExpandVec(RGB5652XXX(color_rdata), DEFAULT_ALPHA_VAL)),
        .color_rvalid(color_rvalid),
        .color_waddr(color_waddr),
        .color_wvalid(color_wvalid),
        .color_wready(color_wready),
        .color_wdata(color_wdata),
        .color_wstrb(color_wstrb),
        .color_wlast(color_wlast),
        .color_wscreenPosX(color_wscreenPosX),
        .color_wscreenPosY(color_wscreenPosY),

        .depthBufferClearDepth(depthBufferClearDepth),
        .depthBufferAddr(depthBufferAddr),
        .depthBufferApply(depthBufferApply),
        .depthBufferApplied(depthBufferApplied),
        .depthBufferCmdCommit(depthBufferCmdCommit),
        .depthBufferCmdMemset(depthBufferCmdMemset),
        .depthBufferEnable(depthBufferEnable),
        .depthBufferMask(depthBufferMask),
        .depth_arready(depth_arready),
        .depth_arlast(depth_arlast),
        .depth_arvalid(depth_arvalid),
        .depth_araddr(depth_araddr),
        .depth_rready(depth_rready),
        .depth_rdata(depth_rdata),
        .depth_rvalid(depth_rvalid),
        .depth_waddr(depth_waddr),
        .depth_wvalid(depth_wvalid),
        .depth_wready(depth_wready),
        .depth_wdata(depth_wdata),
        .depth_wstrb(depth_wstrb),
        .depth_wlast(depth_wlast),
        .depth_wscreenPosX(depth_wscreenPosX),
        .depth_wscreenPosY(depth_wscreenPosY),

        .stencilBufferClearStencil(stencilBufferClearStencil),
        .stencilBufferAddr(stencilBufferAddr),
        .stencilBufferApply(stencilBufferApply),
        .stencilBufferApplied(stencilBufferApplied),
        .stencilBufferCmdCommit(stencilBufferCmdCommit),
        .stencilBufferCmdMemset(stencilBufferCmdMemset),
        .stencilBufferEnable(stencilBufferEnable),
        .stencilBufferMask(stencilBufferMask),
        .stencil_arready(stencil_arready),
        .stencil_arlast(stencil_arlast),
        .stencil_arvalid(stencil_arvalid),
        .stencil_araddr(stencil_araddr),
        .stencil_rready(stencil_rready),
        .stencil_rdata(stencil_rdata),
        .stencil_rvalid(stencil_rvalid),
        .stencil_waddr(stencil_waddr),
        .stencil_wvalid(stencil_wvalid),
        .stencil_wready(stencil_wready),
        .stencil_wdata(stencil_wdata),
        .stencil_wstrb(stencil_wstrb),
        .stencil_wlast(stencil_wlast),
        .stencil_wscreenPosX(stencil_wscreenPosX),
        .stencil_wscreenPosY(stencil_wscreenPosY)
    );

    assign swap_fb = colorBufferApply && colorBufferCmdCommit;

endmodule
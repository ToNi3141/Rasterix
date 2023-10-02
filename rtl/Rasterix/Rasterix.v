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

module Rasterix #(
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
    `ReduceVec(ColorBufferReduceVec, COLOR_SUB_PIXEL_WIDTH, COLOR_NUMBER_OF_SUB_PIXEL, SUB_PIXEL_OFFSET, COLOR_NUMBER_OF_SUB_PIXEL, FRAMEBUFFER_NUMBER_OF_SUB_PIXELS);
    `ReduceVec(ColorBufferReduceMask, 1, COLOR_NUMBER_OF_SUB_PIXEL, SUB_PIXEL_OFFSET, COLOR_NUMBER_OF_SUB_PIXEL, FRAMEBUFFER_NUMBER_OF_SUB_PIXELS);
    `ExpandVec(ColorBufferExpandVec, COLOR_SUB_PIXEL_WIDTH, FRAMEBUFFER_NUMBER_OF_SUB_PIXELS, SUB_PIXEL_OFFSET, COLOR_NUMBER_OF_SUB_PIXEL, COLOR_NUMBER_OF_SUB_PIXEL);
    `Expand(ColorBufferExpand, FRAMEBUFFER_SUB_PIXEL_WIDTH, COLOR_SUB_PIXEL_WIDTH, FRAMEBUFFER_NUMBER_OF_SUB_PIXELS);
    `Reduce(ColorBufferReduce, FRAMEBUFFER_SUB_PIXEL_WIDTH, COLOR_SUB_PIXEL_WIDTH, FRAMEBUFFER_NUMBER_OF_SUB_PIXELS);

    wire                             m_cmd_axis_tvalid;
    wire                             m_cmd_axis_tready;
    wire                             m_cmd_axis_tlast;
    wire [CMD_STREAM_WIDTH - 1 : 0]  m_cmd_axis_tdata;

    wire                             s_framebuffer_axis_tvalid;
    wire                             s_framebuffer_axis_tready;
    wire                             s_framebuffer_axis_tlast;
    wire [CMD_STREAM_WIDTH - 1 : 0]  s_framebuffer_axis_tdata;

    localparam NRS = 4;

    wire [(NRS * ID_WIDTH) - 1 : 0]         s_xbar_axi_awid;
    wire [ADDR_WIDTH - 1 : 0]               s_xbar_axi_awaddr;
    wire [(NRS * 8) : 0]                    s_xbar_axi_awlen; 
    wire [(NRS * 3) : 0]                    s_xbar_axi_awsize;
    wire [(NRS * 2) : 0]                    s_xbar_axi_awburst;
    wire [NRS - 1 : 0]                      s_xbar_axi_awlock;
    wire [(NRS * 4) - 1 : 0]                s_xbar_axi_awcache;
    wire [(NRS * 3) - 1 : 0]                s_xbar_axi_awprot; 
    wire [NRS - 1 : 0]                      s_xbar_axi_awvalid;
    wire [NRS - 1 : 0]                      s_xbar_axi_awready;

    wire [(NRS * CMD_STREAM_WIDTH) - 1 : 0] s_xbar_axi_wdata;
    wire [(NRS * STRB_WIDTH) - 1 : 0]       s_xbar_axi_wstrb;
    wire [NRS - 1 : 0]                      s_xbar_axi_wlast;
    wire [NRS - 1 : 0]                      s_xbar_axi_wvalid;
    wire [NRS - 1 : 0]                      s_xbar_axi_wready;

    wire [(NRS * ID_WIDTH) - 1 : 0]         s_xbar_axi_bid;
    wire [NRS - 1 : 0]                      s_xbar_axi_bresp;
    wire [NRS - 1 : 0]                      s_xbar_axi_bvalid;
    wire [NRS - 1 : 0]                      s_xbar_axi_bready;

    wire [(NRS * ID_WIDTH) - 1 : 0]         s_xbar_axi_arid;
    wire [(NRS * ADDR_WIDTH) - 1 : 0]       s_xbar_axi_araddr;
    wire [(NRS * 8) - 1 : 0]                s_xbar_axi_arlen;
    wire [(NRS * 3) - 1 : 0]                s_xbar_axi_arsize;
    wire [(NRS * 2) - 1 : 0]                s_xbar_axi_arburst;
    wire [NRS - 1 : 0]                      s_xbar_axi_arlock;
    wire [(NRS * 4) - 1 : 0]                s_xbar_axi_arcache;
    wire [(NRS * 3) - 1 : 0]                s_xbar_axi_arprot;
    wire [NRS - 1 : 0]                      s_xbar_axi_arvalid;
    wire [NRS - 1 : 0]                      s_xbar_axi_arready;

    wire [(NRS * ID_WIDTH) - 1 : 0]         s_xbar_axi_rid;
    wire [(NRS * CMD_STREAM_WIDTH) - 1 : 0] s_xbar_axi_rdata;
    wire [(NRS * 2) - 1 : 0]                s_xbar_axi_rresp;
    wire [NRS - 1 : 0]                      s_xbar_axi_rlast;
    wire [NRS - 1 : 0]                      s_xbar_axi_rvalid;
    wire [NRS - 1 : 0]                      s_xbar_axi_rready;

    axixbar #(
        .C_AXI_DATA_WIDTH(CMD_STREAM_WIDTH),
        .C_AXI_ADDR_WIDTH(ADDR_WIDTH),
        .C_AXI_ID_WIDTH(ID_WIDTH),
        .NM(1),
        .NS(NRS)
    ) mainXBar (
        .S_AXI_ACLK(aclk),
        .S_AXI_ARESETN(resetn),

        .S_AXI_AWID(s_xbar_axi_awid),
        .S_AXI_AWADDR(s_xbar_axi_awaddr),
        .S_AXI_AWLEN(s_xbar_axi_awlen),
        .S_AXI_AWSIZE(s_xbar_axi_awsize),
        .S_AXI_AWBURST(s_xbar_axi_awburst),
        .S_AXI_AWLOCK(s_xbar_axi_awlock),
        .S_AXI_AWCACHE(s_xbar_axi_awcache),
        .S_AXI_AWPROT(s_xbar_axi_awprot),
        .S_AXI_AWQOS(0),
        .S_AXI_AWVALID(s_xbar_axi_awvalid),
        .S_AXI_AWREADY(s_xbar_axi_awready),

        .S_AXI_WDATA(s_xbar_axi_wdata),
        .S_AXI_WSTRB(s_xbar_axi_wstrb),
        .S_AXI_WLAST(s_xbar_axi_wlast),
        .S_AXI_WVALID(s_xbar_axi_wvalid),
        .S_AXI_WREADY(s_xbar_axi_wready),

        .S_AXI_BID(s_xbar_axi_bid),
        .S_AXI_BRESP(s_xbar_axi_bresp),
        .S_AXI_BVALID(s_xbar_axi_bvalid),
        .S_AXI_BREADY(s_xbar_axi_bready),

        .S_AXI_ARID(s_xbar_axi_arid),
        .S_AXI_ARADDR(s_xbar_axi_araddr),
        .S_AXI_ARLEN(s_xbar_axi_arlen),
        .S_AXI_ARSIZE(s_xbar_axi_arsize),
        .S_AXI_ARBURST(s_xbar_axi_arburst),
        .S_AXI_ARLOCK(s_xbar_axi_arlock),
        .S_AXI_ARCACHE(s_xbar_axi_arcache),
        .S_AXI_ARPROT(s_xbar_axi_arprot),
        .S_AXI_ARQOS(0),
        .S_AXI_ARVALID(s_xbar_axi_arvalid),
        .S_AXI_ARREADY(s_xbar_axi_arready),

        .S_AXI_RID(s_xbar_axi_rid),
        .S_AXI_RDATA(s_xbar_axi_rdata),
        .S_AXI_RRESP(s_xbar_axi_rresp),
        .S_AXI_RLAST(s_xbar_axi_rlast),
        .S_AXI_RVALID(s_xbar_axi_rvalid),
        .S_AXI_RREADY(s_xbar_axi_rready),

        .M_AXI_AWID(m_mem_axi_awid),
        .M_AXI_AWADDR(m_mem_axi_awaddr),
        .M_AXI_AWLEN(m_mem_axi_awlen),
        .M_AXI_AWSIZE(m_mem_axi_awsize),
        .M_AXI_AWBURST(m_mem_axi_awburst),
        .M_AXI_AWLOCK(m_mem_axi_awlock),
        .M_AXI_AWCACHE(m_mem_axi_awcache),
        .M_AXI_AWPROT(m_mem_axi_awprot),
        .M_AXI_AWQOS(),
        .M_AXI_AWVALID(m_mem_axi_awvalid),
        .M_AXI_AWREADY(m_mem_axi_awready),

        .M_AXI_WDATA(m_mem_axi_wdata),
        .M_AXI_WSTRB(m_mem_axi_wstrb),
        .M_AXI_WLAST(m_mem_axi_wlast),
        .M_AXI_WVALID(m_mem_axi_wvalid),
        .M_AXI_WREADY(m_mem_axi_wready),

        .M_AXI_BID(m_mem_axi_bid),
        .M_AXI_BRESP(m_mem_axi_bresp),
        .M_AXI_BVALID(m_mem_axi_bvalid),
        .M_AXI_BREADY(m_mem_axi_bready),

        .M_AXI_ARID(m_mem_axi_arid),
        .M_AXI_ARADDR(m_mem_axi_araddr),
        .M_AXI_ARLEN(m_mem_axi_arlen),
        .M_AXI_ARSIZE(m_mem_axi_arsize),
        .M_AXI_ARBURST(m_mem_axi_arburst),
        .M_AXI_ARLOCK(m_mem_axi_arlock),
        .M_AXI_ARCACHE(m_mem_axi_arcache),
        .M_AXI_ARQOS(),
        .M_AXI_ARPROT(m_mem_axi_arprot),
        .M_AXI_ARVALID(m_mem_axi_arvalid),
        .M_AXI_ARREADY(m_mem_axi_arready),

        .M_AXI_RID(m_mem_axi_rid),
        .M_AXI_RDATA(m_mem_axi_rdata),
        .M_AXI_RRESP(m_mem_axi_rresp),
        .M_AXI_RLAST(m_mem_axi_rlast),
        .M_AXI_RVALID(m_mem_axi_rvalid),
        .M_AXI_RREADY(m_mem_axi_rready)
    );

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

        .m_mem_axi_awid(s_xbar_axi_awid[0 * ID_WIDTH +: ID_WIDTH]),
        .m_mem_axi_awaddr(s_xbar_axi_awaddr[0 * ADDR_WIDTH +: ADDR_WIDTH]),
        .m_mem_axi_awlen(s_xbar_axi_awlen[0 * 8 +: 8]), 
        .m_mem_axi_awsize(s_xbar_axi_awsize[0 * 3 +: 3]), 
        .m_mem_axi_awburst(s_xbar_axi_awburst[0 * 2 +: 2]), 
        .m_mem_axi_awlock(s_xbar_axi_awlock[0 * 1 +: 1]), 
        .m_mem_axi_awcache(s_xbar_axi_awcache[0 * 4 +: 4]), 
        .m_mem_axi_awprot(s_xbar_axi_awprot[0 * 3 +: 3]), 
        .m_mem_axi_awvalid(s_xbar_axi_awvalid[0 * 1 +: 1]),
        .m_mem_axi_awready(s_xbar_axi_awready[0 * 1 +: 1]),

        .m_mem_axi_wdata(s_xbar_axi_wdata[0 * CMD_STREAM_WIDTH +: CMD_STREAM_WIDTH]),
        .m_mem_axi_wstrb(s_xbar_axi_wstrb[0 * STRB_WIDTH +: STRB_WIDTH]),
        .m_mem_axi_wlast(s_xbar_axi_wlast[0 * 1 +: 1]),
        .m_mem_axi_wvalid(s_xbar_axi_wvalid[0 * 1 +: 1]),
        .m_mem_axi_wready(s_xbar_axi_wready[0 * 1 +: 1]),

        .m_mem_axi_bid(s_xbar_axi_bid[0 * ID_WIDTH +: ID_WIDTH]),
        .m_mem_axi_bresp(s_xbar_axi_bresp[0 * 2 +: 2]),
        .m_mem_axi_bvalid(s_xbar_axi_bvalid[0 * 1 +: 1]),
        .m_mem_axi_bready(s_xbar_axi_bready[0 * 1 +: 1]),

        .m_mem_axi_arid(s_xbar_axi_arid[0 * ID_WIDTH +: ID_WIDTH]),
        .m_mem_axi_araddr(s_xbar_axi_araddr[0 * ADDR_WIDTH +: ADDR_WIDTH]),
        .m_mem_axi_arlen(s_xbar_axi_arlen[0 * 8 +: 8]),
        .m_mem_axi_arsize(s_xbar_axi_arsize[0 * 3 +: 3]),
        .m_mem_axi_arburst(s_xbar_axi_arburst[0 * 2 +: 2]),
        .m_mem_axi_arlock(s_xbar_axi_arlock[0 * 1 +: 1]),
        .m_mem_axi_arcache(s_xbar_axi_arcache[0 * 4 +: 4]),
        .m_mem_axi_arprot(s_xbar_axi_arprot[0 * 3 +: 3]),
        .m_mem_axi_arvalid(s_xbar_axi_arvalid[0 * 1 +: 1]),
        .m_mem_axi_arready(s_xbar_axi_arready[0 * 1 +: 1]),

        .m_mem_axi_rid(s_xbar_axi_rid[0 * ID_WIDTH +: ID_WIDTH]),
        .m_mem_axi_rdata(s_xbar_axi_rdata[0 * CMD_STREAM_WIDTH +: CMD_STREAM_WIDTH]),
        .m_mem_axi_rresp(s_xbar_axi_rresp[0 * 2 +: 2]),
        .m_mem_axi_rlast(s_xbar_axi_rlast[0 * 1 +: 1]),
        .m_mem_axi_rvalid(s_xbar_axi_rvalid[0 * 1 +: 1]),
        .m_mem_axi_rready(s_xbar_axi_rready[0 * 1 +: 1])
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

    wire [(PIXEL_WIDTH_STREAM * PIXEL_PER_BEAT) - 1 : 0] s_framebuffer_unconverted_axis_tdata;
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
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

    localparam NRS = 4;

    wire [(NRS * ID_WIDTH) - 1 : 0]         s_xbar_axi_awid;
    wire [(NRS * ADDR_WIDTH) - 1 : 0]       s_xbar_axi_awaddr;
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
    wire [(NRS * 2) - 1 : 0]                s_xbar_axi_bresp;
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

    axi_crossbar #(
        .DATA_WIDTH(CMD_STREAM_WIDTH),
        .ADDR_WIDTH(ADDR_WIDTH),
        .S_ID_WIDTH(2),
        .S_COUNT(NRS),
        .M_COUNT(1),
        .M_ADDR_WIDTH({1{{1{32'd25}}}})
    ) mainXBar (
        .clk(aclk),
        .rst(!resetn),

        .s_axi_awid(s_xbar_axi_awid),
        .s_axi_awaddr(s_xbar_axi_awaddr),
        .s_axi_awlen(s_xbar_axi_awlen),
        .s_axi_awsize(s_xbar_axi_awsize),
        .s_axi_awburst(s_xbar_axi_awburst),
        .s_axi_awlock(s_xbar_axi_awlock),
        .s_axi_awcache(s_xbar_axi_awcache),
        .s_axi_awprot(s_xbar_axi_awprot),
        .s_axi_awqos(0),
        .s_axi_awvalid(s_xbar_axi_awvalid),
        .s_axi_awready(s_xbar_axi_awready),

        .s_axi_wdata(s_xbar_axi_wdata),
        .s_axi_wstrb(s_xbar_axi_wstrb),
        .s_axi_wlast(s_xbar_axi_wlast),
        .s_axi_wvalid(s_xbar_axi_wvalid),
        .s_axi_wready(s_xbar_axi_wready),

        .s_axi_bid(s_xbar_axi_bid),
        .s_axi_bresp(s_xbar_axi_bresp),
        .s_axi_bvalid(s_xbar_axi_bvalid),
        .s_axi_bready(s_xbar_axi_bready),

        .s_axi_arid(s_xbar_axi_arid),
        .s_axi_araddr(s_xbar_axi_araddr),
        .s_axi_arlen(s_xbar_axi_arlen),
        .s_axi_arsize(s_xbar_axi_arsize),
        .s_axi_arburst(s_xbar_axi_arburst),
        .s_axi_arlock(s_xbar_axi_arlock),
        .s_axi_arcache(s_xbar_axi_arcache),
        .s_axi_arprot(s_xbar_axi_arprot),
        .s_axi_arqos(0),
        .s_axi_arvalid(s_xbar_axi_arvalid),
        .s_axi_arready(s_xbar_axi_arready),

        .s_axi_rid(s_xbar_axi_rid),
        .s_axi_rdata(s_xbar_axi_rdata),
        .s_axi_rresp(s_xbar_axi_rresp),
        .s_axi_rlast(s_xbar_axi_rlast),
        .s_axi_rvalid(s_xbar_axi_rvalid),
        .s_axi_rready(s_xbar_axi_rready),

        .m_axi_awid(m_mem_axi_awid),
        .m_axi_awaddr(m_mem_axi_awaddr),
        .m_axi_awlen(m_mem_axi_awlen),
        .m_axi_awsize(m_mem_axi_awsize),
        .m_axi_awburst(m_mem_axi_awburst),
        .m_axi_awlock(m_mem_axi_awlock),
        .m_axi_awcache(m_mem_axi_awcache),
        .m_axi_awprot(m_mem_axi_awprot),
        .m_axi_awqos(),
        .m_axi_awvalid(m_mem_axi_awvalid),
        .m_axi_awready(m_mem_axi_awready),

        .m_axi_wdata(m_mem_axi_wdata),
        .m_axi_wstrb(m_mem_axi_wstrb),
        .m_axi_wlast(m_mem_axi_wlast),
        .m_axi_wvalid(m_mem_axi_wvalid),
        .m_axi_wready(m_mem_axi_wready),

        .m_axi_bid(m_mem_axi_bid),
        .m_axi_bresp(m_mem_axi_bresp),
        .m_axi_bvalid(m_mem_axi_bvalid),
        .m_axi_bready(m_mem_axi_bready),

        .m_axi_arid(m_mem_axi_arid),
        .m_axi_araddr(m_mem_axi_araddr),
        .m_axi_arlen(m_mem_axi_arlen),
        .m_axi_arsize(m_mem_axi_arsize),
        .m_axi_arburst(m_mem_axi_arburst),
        .m_axi_arlock(m_mem_axi_arlock),
        .m_axi_arcache(m_mem_axi_arcache),
        .m_axi_arqos(),
        .m_axi_arprot(m_mem_axi_arprot),
        .m_axi_arvalid(m_mem_axi_arvalid),
        .m_axi_arready(m_mem_axi_arready),

        .m_axi_rid(m_mem_axi_rid),
        .m_axi_rdata(m_mem_axi_rdata),
        .m_axi_rresp(m_mem_axi_rresp),
        .m_axi_rlast(m_mem_axi_rlast),
        .m_axi_rvalid(m_mem_axi_rvalid),
        .m_axi_rready(m_mem_axi_rready)
    );

    DmaStreamEngine #(
        .STREAM_WIDTH(CMD_STREAM_WIDTH),
        .ADDR_WIDTH(28)
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

        //         .m_mem_axi_awid(s_xbar_axi_awid[1 * ID_WIDTH +: ID_WIDTH]),
        // .m_mem_axi_awaddr(s_xbar_axi_awaddr[1 * ADDR_WIDTH +: ADDR_WIDTH]),
        // .m_mem_axi_awlen(s_xbar_axi_awlen[1 * 8 +: 8]), 
        // .m_mem_axi_awsize(s_xbar_axi_awsize[1 * 3 +: 3]), 
        // .m_mem_axi_awburst(s_xbar_axi_awburst[1 * 2 +: 2]), 
        // .m_mem_axi_awlock(s_xbar_axi_awlock[1 * 1 +: 1]), 
        // .m_mem_axi_awcache(s_xbar_axi_awcache[1 * 4 +: 4]), 
        // .m_mem_axi_awprot(s_xbar_axi_awprot[1 * 3 +: 3]), 
        // .m_mem_axi_awvalid(s_xbar_axi_awvalid[1 * 1 +: 1]),
        // .m_mem_axi_awready(s_xbar_axi_awready[1 * 1 +: 1]),

        // .m_mem_axi_wdata(s_xbar_axi_wdata[1 * CMD_STREAM_WIDTH +: CMD_STREAM_WIDTH]),
        // .m_mem_axi_wstrb(s_xbar_axi_wstrb[1 * STRB_WIDTH +: STRB_WIDTH]),
        // .m_mem_axi_wlast(s_xbar_axi_wlast[1 * 1 +: 1]),
        // .m_mem_axi_wvalid(s_xbar_axi_wvalid[1 * 1 +: 1]),
        // .m_mem_axi_wready(s_xbar_axi_wready[1 * 1 +: 1]),

        // .m_mem_axi_bid(s_xbar_axi_bid[1 * ID_WIDTH +: ID_WIDTH]),
        // .m_mem_axi_bresp(s_xbar_axi_bresp[1 * 2 +: 2]),
        // .m_mem_axi_bvalid(s_xbar_axi_bvalid[1 * 1 +: 1]),
        // .m_mem_axi_bready(s_xbar_axi_bready[1 * 1 +: 1]),

        // .m_mem_axi_arid(s_xbar_axi_arid[1 * ID_WIDTH +: ID_WIDTH]),
        // .m_mem_axi_araddr(s_xbar_axi_araddr[1 * ADDR_WIDTH +: ADDR_WIDTH]),
        // .m_mem_axi_arlen(s_xbar_axi_arlen[1 * 8 +: 8]),
        // .m_mem_axi_arsize(s_xbar_axi_arsize[1 * 3 +: 3]),
        // .m_mem_axi_arburst(s_xbar_axi_arburst[1 * 2 +: 2]),
        // .m_mem_axi_arlock(s_xbar_axi_arlock[1 * 1 +: 1]),
        // .m_mem_axi_arcache(s_xbar_axi_arcache[1 * 4 +: 4]),
        // .m_mem_axi_arprot(s_xbar_axi_arprot[1 * 3 +: 3]),
        // .m_mem_axi_arvalid(s_xbar_axi_arvalid[1 * 1 +: 1]),
        // .m_mem_axi_arready(s_xbar_axi_arready[1 * 1 +: 1]),

        // .m_mem_axi_rid(s_xbar_axi_rid[1 * ID_WIDTH +: ID_WIDTH]),
        // .m_mem_axi_rdata(s_xbar_axi_rdata[1 * CMD_STREAM_WIDTH +: CMD_STREAM_WIDTH]),
        // .m_mem_axi_rresp(s_xbar_axi_rresp[1 * 2 +: 2]),
        // .m_mem_axi_rlast(s_xbar_axi_rlast[1 * 1 +: 1]),
        // .m_mem_axi_rvalid(s_xbar_axi_rvalid[1 * 1 +: 1]),
        // .m_mem_axi_rready(s_xbar_axi_rready[1 * 1 +: 1])
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

    // FrameBuffer depthBuffer (  
    //     .clk(aclk),
    //     .reset(!resetn),

    //     .confEnable(depthBufferEnable),
    //     .confClearColor(depthBufferClearDepth),
    //     .confEnableScissor(framebufferParamEnableScissor),
    //     .confScissorStartX(framebufferParamScissorStartX),
    //     .confScissorStartY(framebufferParamScissorStartY),
    //     .confScissorEndX(framebufferParamScissorEndX),
    //     .confScissorEndY(framebufferParamScissorEndY),
    //     .confYOffset(framebufferParamYOffset),
    //     .confXResolution(framebufferParamXResolution),
    //     .confYResolution(framebufferParamYResolution),
    //     .confMask(depthBufferMask),

    //     .araddr(depth_araddr),
    //     .arvalid(depth_arvalid),
    //     .arlast(depth_arlast),
    //     .rvalid(depth_rvalid),
    //     .rlast(depth_rlast),
    //     .rdata(depth_rdata),
    //     .waddr(depth_waddr),
    //     .wdata(depth_wdata),
    //     .wvalid(depth_wvalid),
    //     .wstrb(depth_wstrb),
    //     .wscreenPosX(depth_wscreenPosX),
    //     .wscreenPosY(depth_wscreenPosY),

    //     .apply(depthBufferApply),
    //     .applied(depthBufferApplied),
    //     .cmdCommit(depthBufferCmdCommit),
    //     .cmdMemset(depthBufferCmdMemset),

    //     .m_axis_tvalid(),
    //     .m_axis_tready(1'b1),
    //     .m_axis_tlast(),
    //     .m_axis_tdata()
    // );
    // defparam depthBuffer.NUMBER_OF_PIXELS_PER_BEAT = PIXEL_PER_BEAT;
    // defparam depthBuffer.NUMBER_OF_SUB_PIXELS = 1;
    // defparam depthBuffer.SUB_PIXEL_WIDTH = 16;
    // defparam depthBuffer.X_BIT_WIDTH = RENDER_CONFIG_X_SIZE;
    // defparam depthBuffer.Y_BIT_WIDTH = RENDER_CONFIG_Y_SIZE;
    // defparam depthBuffer.FRAMEBUFFER_SIZE_IN_WORDS = FRAMEBUFFER_SIZE_IN_WORDS;

    StreamFramebuffer #(
        .STREAM_WIDTH(CMD_STREAM_WIDTH),
        .ADDR_WIDTH(ADDR_WIDTH),
        .ID_WIDTH(ID_WIDTH),
        .X_BIT_WIDTH(RENDER_CONFIG_X_SIZE),
        .Y_BIT_WIDTH(RENDER_CONFIG_Y_SIZE),
        .PIXEL_WIDTH(16)
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

        .m_mem_axi_awid(s_xbar_axi_awid[2 * ID_WIDTH +: ID_WIDTH]),
        .m_mem_axi_awaddr(s_xbar_axi_awaddr[2 * ADDR_WIDTH +: ADDR_WIDTH]),
        .m_mem_axi_awlen(s_xbar_axi_awlen[2 * 8 +: 8]), 
        .m_mem_axi_awsize(s_xbar_axi_awsize[2 * 3 +: 3]), 
        .m_mem_axi_awburst(s_xbar_axi_awburst[2 * 2 +: 2]), 
        .m_mem_axi_awlock(s_xbar_axi_awlock[2 * 1 +: 1]), 
        .m_mem_axi_awcache(s_xbar_axi_awcache[2 * 4 +: 4]), 
        .m_mem_axi_awprot(s_xbar_axi_awprot[2 * 3 +: 3]), 
        .m_mem_axi_awvalid(s_xbar_axi_awvalid[2 * 1 +: 1]),
        .m_mem_axi_awready(s_xbar_axi_awready[2 * 1 +: 1]),

        .m_mem_axi_wdata(s_xbar_axi_wdata[2 * CMD_STREAM_WIDTH +: CMD_STREAM_WIDTH]),
        .m_mem_axi_wstrb(s_xbar_axi_wstrb[2 * STRB_WIDTH +: STRB_WIDTH]),
        .m_mem_axi_wlast(s_xbar_axi_wlast[2 * 1 +: 1]),
        .m_mem_axi_wvalid(s_xbar_axi_wvalid[2 * 1 +: 1]),
        .m_mem_axi_wready(s_xbar_axi_wready[2 * 1 +: 1]),

        .m_mem_axi_bid(s_xbar_axi_bid[2 * ID_WIDTH +: ID_WIDTH]),
        .m_mem_axi_bresp(s_xbar_axi_bresp[2 * 2 +: 2]),
        .m_mem_axi_bvalid(s_xbar_axi_bvalid[2 * 1 +: 1]),
        .m_mem_axi_bready(s_xbar_axi_bready[2 * 1 +: 1]),

        .m_mem_axi_arid(s_xbar_axi_arid[2 * ID_WIDTH +: ID_WIDTH]),
        .m_mem_axi_araddr(s_xbar_axi_araddr[2 * ADDR_WIDTH +: ADDR_WIDTH]),
        .m_mem_axi_arlen(s_xbar_axi_arlen[2 * 8 +: 8]),
        .m_mem_axi_arsize(s_xbar_axi_arsize[2 * 3 +: 3]),
        .m_mem_axi_arburst(s_xbar_axi_arburst[2 * 2 +: 2]),
        .m_mem_axi_arlock(s_xbar_axi_arlock[2 * 1 +: 1]),
        .m_mem_axi_arcache(s_xbar_axi_arcache[2 * 4 +: 4]),
        .m_mem_axi_arprot(s_xbar_axi_arprot[2 * 3 +: 3]),
        .m_mem_axi_arvalid(s_xbar_axi_arvalid[2 * 1 +: 1]),
        .m_mem_axi_arready(s_xbar_axi_arready[2 * 1 +: 1]),

        .m_mem_axi_rid(s_xbar_axi_rid[2 * ID_WIDTH +: ID_WIDTH]),
        .m_mem_axi_rdata(s_xbar_axi_rdata[2 * CMD_STREAM_WIDTH +: CMD_STREAM_WIDTH]),
        .m_mem_axi_rresp(s_xbar_axi_rresp[2 * 2 +: 2]),
        .m_mem_axi_rlast(s_xbar_axi_rlast[2 * 1 +: 1]),
        .m_mem_axi_rvalid(s_xbar_axi_rvalid[2 * 1 +: 1]),
        .m_mem_axi_rready(s_xbar_axi_rready[2 * 1 +: 1])
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

        .m_mem_axi_awid(s_xbar_axi_awid[1 * ID_WIDTH +: ID_WIDTH]),
        .m_mem_axi_awaddr(s_xbar_axi_awaddr[1 * ADDR_WIDTH +: ADDR_WIDTH]),
        .m_mem_axi_awlen(s_xbar_axi_awlen[1 * 8 +: 8]), 
        .m_mem_axi_awsize(s_xbar_axi_awsize[1 * 3 +: 3]), 
        .m_mem_axi_awburst(s_xbar_axi_awburst[1 * 2 +: 2]), 
        .m_mem_axi_awlock(s_xbar_axi_awlock[1 * 1 +: 1]), 
        .m_mem_axi_awcache(s_xbar_axi_awcache[1 * 4 +: 4]), 
        .m_mem_axi_awprot(s_xbar_axi_awprot[1 * 3 +: 3]), 
        .m_mem_axi_awvalid(s_xbar_axi_awvalid[1 * 1 +: 1]),
        .m_mem_axi_awready(s_xbar_axi_awready[1 * 1 +: 1]),

        .m_mem_axi_wdata(s_xbar_axi_wdata[1 * CMD_STREAM_WIDTH +: CMD_STREAM_WIDTH]),
        .m_mem_axi_wstrb(s_xbar_axi_wstrb[1 * STRB_WIDTH +: STRB_WIDTH]),
        .m_mem_axi_wlast(s_xbar_axi_wlast[1 * 1 +: 1]),
        .m_mem_axi_wvalid(s_xbar_axi_wvalid[1 * 1 +: 1]),
        .m_mem_axi_wready(s_xbar_axi_wready[1 * 1 +: 1]),

        .m_mem_axi_bid(s_xbar_axi_bid[1 * ID_WIDTH +: ID_WIDTH]),
        .m_mem_axi_bresp(s_xbar_axi_bresp[1 * 2 +: 2]),
        .m_mem_axi_bvalid(s_xbar_axi_bvalid[1 * 1 +: 1]),
        .m_mem_axi_bready(s_xbar_axi_bready[1 * 1 +: 1]),

        .m_mem_axi_arid(s_xbar_axi_arid[1 * ID_WIDTH +: ID_WIDTH]),
        .m_mem_axi_araddr(s_xbar_axi_araddr[1 * ADDR_WIDTH +: ADDR_WIDTH]),
        .m_mem_axi_arlen(s_xbar_axi_arlen[1 * 8 +: 8]),
        .m_mem_axi_arsize(s_xbar_axi_arsize[1 * 3 +: 3]),
        .m_mem_axi_arburst(s_xbar_axi_arburst[1 * 2 +: 2]),
        .m_mem_axi_arlock(s_xbar_axi_arlock[1 * 1 +: 1]),
        .m_mem_axi_arcache(s_xbar_axi_arcache[1 * 4 +: 4]),
        .m_mem_axi_arprot(s_xbar_axi_arprot[1 * 3 +: 3]),
        .m_mem_axi_arvalid(s_xbar_axi_arvalid[1 * 1 +: 1]),
        .m_mem_axi_arready(s_xbar_axi_arready[1 * 1 +: 1]),

        .m_mem_axi_rid(s_xbar_axi_rid[1 * ID_WIDTH +: ID_WIDTH]),
        .m_mem_axi_rdata(s_xbar_axi_rdata[1 * CMD_STREAM_WIDTH +: CMD_STREAM_WIDTH]),
        .m_mem_axi_rresp(s_xbar_axi_rresp[1 * 2 +: 2]),
        .m_mem_axi_rlast(s_xbar_axi_rlast[1 * 1 +: 1]),
        .m_mem_axi_rvalid(s_xbar_axi_rvalid[1 * 1 +: 1]),
        .m_mem_axi_rready(s_xbar_axi_rready[1 * 1 +: 1])


        // .m_mem_axi_awid(s_xbar_axi_awid[0 * ID_WIDTH +: ID_WIDTH]),
        // .m_mem_axi_awaddr(s_xbar_axi_awaddr[0 * ADDR_WIDTH +: ADDR_WIDTH]),
        // .m_mem_axi_awlen(s_xbar_axi_awlen[0 * 8 +: 8]), 
        // .m_mem_axi_awsize(s_xbar_axi_awsize[0 * 3 +: 3]), 
        // .m_mem_axi_awburst(s_xbar_axi_awburst[0 * 2 +: 2]), 
        // .m_mem_axi_awlock(s_xbar_axi_awlock[0 * 1 +: 1]), 
        // .m_mem_axi_awcache(s_xbar_axi_awcache[0 * 4 +: 4]), 
        // .m_mem_axi_awprot(s_xbar_axi_awprot[0 * 3 +: 3]), 
        // .m_mem_axi_awvalid(s_xbar_axi_awvalid[0 * 1 +: 1]),
        // .m_mem_axi_awready(s_xbar_axi_awready[0 * 1 +: 1]),

        // .m_mem_axi_wdata(s_xbar_axi_wdata[0 * CMD_STREAM_WIDTH +: CMD_STREAM_WIDTH]),
        // .m_mem_axi_wstrb(s_xbar_axi_wstrb[0 * STRB_WIDTH +: STRB_WIDTH]),
        // .m_mem_axi_wlast(s_xbar_axi_wlast[0 * 1 +: 1]),
        // .m_mem_axi_wvalid(s_xbar_axi_wvalid[0 * 1 +: 1]),
        // .m_mem_axi_wready(s_xbar_axi_wready[0 * 1 +: 1]),

        // .m_mem_axi_bid(s_xbar_axi_bid[0 * ID_WIDTH +: ID_WIDTH]),
        // .m_mem_axi_bresp(s_xbar_axi_bresp[0 * 2 +: 2]),
        // .m_mem_axi_bvalid(s_xbar_axi_bvalid[0 * 1 +: 1]),
        // .m_mem_axi_bready(s_xbar_axi_bready[0 * 1 +: 1]),

        // .m_mem_axi_arid(s_xbar_axi_arid[0 * ID_WIDTH +: ID_WIDTH]),
        // .m_mem_axi_araddr(s_xbar_axi_araddr[0 * ADDR_WIDTH +: ADDR_WIDTH]),
        // .m_mem_axi_arlen(s_xbar_axi_arlen[0 * 8 +: 8]),
        // .m_mem_axi_arsize(s_xbar_axi_arsize[0 * 3 +: 3]),
        // .m_mem_axi_arburst(s_xbar_axi_arburst[0 * 2 +: 2]),
        // .m_mem_axi_arlock(s_xbar_axi_arlock[0 * 1 +: 1]),
        // .m_mem_axi_arcache(s_xbar_axi_arcache[0 * 4 +: 4]),
        // .m_mem_axi_arprot(s_xbar_axi_arprot[0 * 3 +: 3]),
        // .m_mem_axi_arvalid(s_xbar_axi_arvalid[0 * 1 +: 1]),
        // .m_mem_axi_arready(s_xbar_axi_arready[0 * 1 +: 1]),

        // .m_mem_axi_rid(s_xbar_axi_rid[0 * ID_WIDTH +: ID_WIDTH]),
        // .m_mem_axi_rdata(s_xbar_axi_rdata[0 * CMD_STREAM_WIDTH +: CMD_STREAM_WIDTH]),
        // .m_mem_axi_rresp(s_xbar_axi_rresp[0 * 2 +: 2]),
        // .m_mem_axi_rlast(s_xbar_axi_rlast[0 * 1 +: 1]),
        // .m_mem_axi_rvalid(s_xbar_axi_rvalid[0 * 1 +: 1]),
        // .m_mem_axi_rready(s_xbar_axi_rready[0 * 1 +: 1])
    );

    // wire [(PIXEL_WIDTH_STREAM * PIXEL_PER_BEAT) - 1 : 0] s_framebuffer_unconverted_axis_tdata;
    // FrameBuffer colorBuffer (  
    //     .clk(aclk),
    //     .reset(!resetn),

    //     .confEnable(colorBufferEnable),
    //     .confClearColor(ColorBufferReduce(ColorBufferReduceVec(colorBufferClearColor))),
    //     .confEnableScissor(framebufferParamEnableScissor),
    //     .confScissorStartX(framebufferParamScissorStartX),
    //     .confScissorStartY(framebufferParamScissorStartY),
    //     .confScissorEndX(framebufferParamScissorEndX),
    //     .confScissorEndY(framebufferParamScissorEndY),
    //     .confYOffset(framebufferParamYOffset),
    //     .confXResolution(framebufferParamXResolution),
    //     .confYResolution(framebufferParamYResolution),
    //     .confMask(ColorBufferReduceMask(colorBufferMask)),

    //     .araddr(color_araddr),
    //     .arvalid(color_arvalid),
    //     .arlast(color_arlast),
    //     .rvalid(color_rvalid),
    //     .rlast(color_rlast),
    //     .rdata(color_rdata),
    //     .waddr(color_waddr),
    //     .wdata(ColorBufferReduce(ColorBufferReduceVec(color_wdata))),
    //     .wvalid(color_wvalid),
    //     .wstrb(color_wstrb),
    //     .wscreenPosX(color_wscreenPosX),
    //     .wscreenPosY(color_wscreenPosY),
        
    //     .apply(colorBufferApply),
    //     .applied(colorBufferApplied),
    //     .cmdCommit(colorBufferCmdCommit),
    //     .cmdMemset(colorBufferCmdMemset),

    //     .m_axis_tvalid(s_framebuffer_axis_tvalid),
    //     .m_axis_tready(s_framebuffer_axis_tready),
    //     .m_axis_tlast(s_framebuffer_axis_tlast),
    //     .m_axis_tdata(s_framebuffer_unconverted_axis_tdata)
    // );
    // defparam colorBuffer.NUMBER_OF_PIXELS_PER_BEAT = PIXEL_PER_BEAT; 
    // defparam colorBuffer.NUMBER_OF_SUB_PIXELS = FRAMEBUFFER_NUMBER_OF_SUB_PIXELS;
    // defparam colorBuffer.SUB_PIXEL_WIDTH = FRAMEBUFFER_SUB_PIXEL_WIDTH;
    // defparam colorBuffer.X_BIT_WIDTH = RENDER_CONFIG_X_SIZE;
    // defparam colorBuffer.Y_BIT_WIDTH = RENDER_CONFIG_Y_SIZE;
    // defparam colorBuffer.FRAMEBUFFER_SIZE_IN_WORDS = FRAMEBUFFER_SIZE_IN_WORDS;

    // Conversion of the internal pixel representation the exnternal one required for the AXIS interface
    // generate
    //     `XXX2RGB565(XXX2RGB565, COLOR_SUB_PIXEL_WIDTH, PIXEL_PER_BEAT);
    //     `Expand(ExpandFramebufferStream, FRAMEBUFFER_SUB_PIXEL_WIDTH, COLOR_SUB_PIXEL_WIDTH, PIXEL_PER_BEAT * 3);
    //     if (FRAMEBUFFER_NUMBER_OF_SUB_PIXELS == 4)
    //     begin
    //         `ReduceVec(ReduceVecFramebufferStream, FRAMEBUFFER_SUB_PIXEL_WIDTH, PIXEL_PER_BEAT * COLOR_NUMBER_OF_SUB_PIXEL, COLOR_A_POS, COLOR_NUMBER_OF_SUB_PIXEL, PIXEL_PER_BEAT * 3);
    //         assign s_framebuffer_axis_tdata = XXX2RGB565(ExpandFramebufferStream(ReduceVecFramebufferStream(s_framebuffer_unconverted_axis_tdata)));
    //     end
    //     else
    //     begin
    //         assign s_framebuffer_axis_tdata = XXX2RGB565(ExpandFramebufferStream(s_framebuffer_unconverted_axis_tdata));
    //     end
    // endgenerate

    generate 
        if (ENABLE_STENCIL_BUFFER)
        begin
            // FrameBuffer stencilBuffer (  
            //     .clk(aclk),
            //     .reset(!resetn),

            //     .confEnable(stencilBufferEnable),
            //     .confClearColor(stencilBufferClearStencil),
            //     .confEnableScissor(framebufferParamEnableScissor),
            //     .confScissorStartX(framebufferParamScissorStartX),
            //     .confScissorStartY(framebufferParamScissorStartY),
            //     .confScissorEndX(framebufferParamScissorEndX),
            //     .confScissorEndY(framebufferParamScissorEndY),
            //     .confYOffset(framebufferParamYOffset),
            //     .confXResolution(framebufferParamXResolution),
            //     .confYResolution(framebufferParamYResolution),
            //     .confMask(stencilBufferMask),

            //     .araddr(stencil_araddr),
            //     .arvalid(stencil_arvalid),
            //     .arlast(stencil_arlast),
            //     .rvalid(stencil_rvalid),
            //     .rlast(stencil_rlast),
            //     .rdata(stencil_rdata),
            //     .waddr(stencil_waddr),
            //     .wdata(stencil_wdata),
            //     .wvalid(stencil_wvalid),
            //     .wstrb(stencil_wstrb),
            //     .wscreenPosX(stencil_wscreenPosX),
            //     .wscreenPosY(stencil_wscreenPosY),

            //     .apply(stencilBufferApply),
            //     .applied(stencilBufferApplied),
            //     .cmdCommit(stencilBufferCmdCommit),
            //     .cmdMemset(stencilBufferCmdMemset),

            //     .m_axis_tvalid(),
            //     .m_axis_tready(1'b1),
            //     .m_axis_tlast(),
            //     .m_axis_tdata()
            // );
            // defparam stencilBuffer.NUMBER_OF_PIXELS_PER_BEAT = PIXEL_PER_BEAT;
            // defparam stencilBuffer.NUMBER_OF_SUB_PIXELS = STENCIL_WIDTH;
            // defparam stencilBuffer.SUB_PIXEL_WIDTH = 1;
            // defparam stencilBuffer.X_BIT_WIDTH = RENDER_CONFIG_X_SIZE;
            // defparam stencilBuffer.Y_BIT_WIDTH = RENDER_CONFIG_Y_SIZE;
            // defparam stencilBuffer.FRAMEBUFFER_SIZE_IN_WORDS = FRAMEBUFFER_SIZE_IN_WORDS;

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

                .m_mem_axi_awid(s_xbar_axi_awid[3 * ID_WIDTH +: ID_WIDTH]),
                .m_mem_axi_awaddr(s_xbar_axi_awaddr[3 * ADDR_WIDTH +: ADDR_WIDTH]),
                .m_mem_axi_awlen(s_xbar_axi_awlen[3 * 8 +: 8]), 
                .m_mem_axi_awsize(s_xbar_axi_awsize[3 * 3 +: 3]), 
                .m_mem_axi_awburst(s_xbar_axi_awburst[3 * 2 +: 2]), 
                .m_mem_axi_awlock(s_xbar_axi_awlock[3 * 1 +: 1]), 
                .m_mem_axi_awcache(s_xbar_axi_awcache[3 * 4 +: 4]), 
                .m_mem_axi_awprot(s_xbar_axi_awprot[3 * 3 +: 3]), 
                .m_mem_axi_awvalid(s_xbar_axi_awvalid[3 * 1 +: 1]),
                .m_mem_axi_awready(s_xbar_axi_awready[3 * 1 +: 1]),

                .m_mem_axi_wdata(s_xbar_axi_wdata[3 * CMD_STREAM_WIDTH +: CMD_STREAM_WIDTH]),
                .m_mem_axi_wstrb(s_xbar_axi_wstrb[3 * STRB_WIDTH +: STRB_WIDTH]),
                .m_mem_axi_wlast(s_xbar_axi_wlast[3 * 1 +: 1]),
                .m_mem_axi_wvalid(s_xbar_axi_wvalid[3 * 1 +: 1]),
                .m_mem_axi_wready(s_xbar_axi_wready[3 * 1 +: 1]),

                .m_mem_axi_bid(s_xbar_axi_bid[3 * ID_WIDTH +: ID_WIDTH]),
                .m_mem_axi_bresp(s_xbar_axi_bresp[3 * 2 +: 2]),
                .m_mem_axi_bvalid(s_xbar_axi_bvalid[3 * 1 +: 1]),
                .m_mem_axi_bready(s_xbar_axi_bready[3 * 1 +: 1]),

                .m_mem_axi_arid(s_xbar_axi_arid[3 * ID_WIDTH +: ID_WIDTH]),
                .m_mem_axi_araddr(s_xbar_axi_araddr[3 * ADDR_WIDTH +: ADDR_WIDTH]),
                .m_mem_axi_arlen(s_xbar_axi_arlen[3 * 8 +: 8]),
                .m_mem_axi_arsize(s_xbar_axi_arsize[3 * 3 +: 3]),
                .m_mem_axi_arburst(s_xbar_axi_arburst[3 * 2 +: 2]),
                .m_mem_axi_arlock(s_xbar_axi_arlock[3 * 1 +: 1]),
                .m_mem_axi_arcache(s_xbar_axi_arcache[3 * 4 +: 4]),
                .m_mem_axi_arprot(s_xbar_axi_arprot[3 * 3 +: 3]),
                .m_mem_axi_arvalid(s_xbar_axi_arvalid[3 * 1 +: 1]),
                .m_mem_axi_arready(s_xbar_axi_arready[3 * 1 +: 1]),

                .m_mem_axi_rid(s_xbar_axi_rid[3 * ID_WIDTH +: ID_WIDTH]),
                .m_mem_axi_rdata(s_xbar_axi_rdata[3 * CMD_STREAM_WIDTH +: CMD_STREAM_WIDTH]),
                .m_mem_axi_rresp(s_xbar_axi_rresp[3 * 2 +: 2]),
                .m_mem_axi_rlast(s_xbar_axi_rlast[3 * 1 +: 1]),
                .m_mem_axi_rvalid(s_xbar_axi_rvalid[3 * 1 +: 1]),
                .m_mem_axi_rready(s_xbar_axi_rready[3 * 1 +: 1])
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
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

module top #(
    parameter CMD_STREAM_WIDTH = 64
)
(
    input  wire         aclk,
    input  wire         resetn,

    // AXI Stream command interface
    input  wire         s_cmd_axis_tvalid,
    output wire         s_cmd_axis_tready,
    input  wire         s_cmd_axis_tlast,
    input  wire [CMD_STREAM_WIDTH - 1 : 0]  s_cmd_axis_tdata,

    // AXI Stream framebuffer
    output wire         m_framebuffer_axis_tvalid,
    input  wire         m_framebuffer_axis_tready,
    output wire         m_framebuffer_axis_tlast,
    output wire [CMD_STREAM_WIDTH - 1 : 0]  m_framebuffer_axis_tdata
);
    parameter FRAMEBUFFER_SIZE_IN_WORDS = `FRAMEBUFFER_SIZE_IN_WORDS;
    parameter TEXTURE_BUFFER_SIZE = 17;

    initial
    begin
        $dumpfile("rr.vcd");
        $dumpvars();
    end

    localparam ID_WIDTH = 8;
    localparam ID_WIDTH_LOC = ID_WIDTH / 4;
    localparam ADDR_WIDTH = 25;
    localparam DATA_WIDTH = CMD_STREAM_WIDTH;
    localparam STRB_WIDTH = CMD_STREAM_WIDTH / 8;
    wire [ID_WIDTH-1:0]    m_mem_axi_awid;
    wire [ADDR_WIDTH-1:0]  m_mem_axi_awaddr;
    wire [7:0]             m_mem_axi_awlen;
    wire [2:0]             m_mem_axi_awsize;
    wire [1:0]             m_mem_axi_awburst;
    wire                   m_mem_axi_awlock;
    wire [3:0]             m_mem_axi_awcache;
    wire [2:0]             m_mem_axi_awprot;
    wire                   m_mem_axi_awvalid;
    wire                   m_mem_axi_awready;
    wire [DATA_WIDTH-1:0]  m_mem_axi_wdata;
    wire [STRB_WIDTH-1:0]  m_mem_axi_wstrb;
    wire                   m_mem_axi_wlast;
    wire                   m_mem_axi_wvalid;
    wire                   m_mem_axi_wready;
    wire [ID_WIDTH-1:0]    m_mem_axi_bid;
    wire [1:0]             m_mem_axi_bresp;
    wire                   m_mem_axi_bvalid;
    wire                   m_mem_axi_bready;
    wire [ID_WIDTH-1:0]    m_mem_axi_arid;
    wire [ADDR_WIDTH-1:0]  m_mem_axi_araddr;
    wire [7:0]             m_mem_axi_arlen;
    wire [2:0]             m_mem_axi_arsize;
    wire [1:0]             m_mem_axi_arburst;
    wire                   m_mem_axi_arlock;
    wire [3:0]             m_mem_axi_arcache;
    wire [2:0]             m_mem_axi_arprot;
    wire                   m_mem_axi_arvalid;
    wire                   m_mem_axi_arready;
    wire [ID_WIDTH-1:0]    m_mem_axi_rid;
    wire [DATA_WIDTH-1:0]  m_mem_axi_rdata;
    wire [1:0]             m_mem_axi_rresp;
    wire                   m_mem_axi_rlast;
    wire                   m_mem_axi_rvalid;
    wire                   m_mem_axi_rready;

    localparam NRS = 4;

    wire [(NRS * ID_WIDTH_LOC) - 1 : 0]     s_xbar_axi_awid;
    wire [(NRS * ADDR_WIDTH) - 1 : 0]       s_xbar_axi_awaddr;
    wire [(NRS * 8) - 1 : 0]                s_xbar_axi_awlen; 
    wire [(NRS * 3) - 1 : 0]                s_xbar_axi_awsize;
    wire [(NRS * 2) - 1 : 0]                s_xbar_axi_awburst;
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

    wire [(NRS * ID_WIDTH_LOC) - 1 : 0]     s_xbar_axi_bid;
    wire [(NRS * 2) - 1 : 0]                s_xbar_axi_bresp;
    wire [NRS - 1 : 0]                      s_xbar_axi_bvalid;
    wire [NRS - 1 : 0]                      s_xbar_axi_bready;

    wire [(NRS * ID_WIDTH_LOC) - 1 : 0]     s_xbar_axi_arid;
    wire [(NRS * ADDR_WIDTH) - 1 : 0]       s_xbar_axi_araddr;
    wire [(NRS * 8) - 1 : 0]                s_xbar_axi_arlen;
    wire [(NRS * 3) - 1 : 0]                s_xbar_axi_arsize;
    wire [(NRS * 2) - 1 : 0]                s_xbar_axi_arburst;
    wire [NRS - 1 : 0]                      s_xbar_axi_arlock;
    wire [(NRS * 4) - 1 : 0]                s_xbar_axi_arcache;
    wire [(NRS * 3) - 1 : 0]                s_xbar_axi_arprot;
    wire [NRS - 1 : 0]                      s_xbar_axi_arvalid;
    wire [NRS - 1 : 0]                      s_xbar_axi_arready;

    wire [(NRS * ID_WIDTH_LOC) - 1 : 0]     s_xbar_axi_rid;
    wire [(NRS * CMD_STREAM_WIDTH) - 1 : 0] s_xbar_axi_rdata;
    wire [(NRS * 2) - 1 : 0]                s_xbar_axi_rresp;
    wire [NRS - 1 : 0]                      s_xbar_axi_rlast;
    wire [NRS - 1 : 0]                      s_xbar_axi_rvalid;
    wire [NRS - 1 : 0]                      s_xbar_axi_rready;


    axi_ram #(
        .DATA_WIDTH(CMD_STREAM_WIDTH),
        .ADDR_WIDTH(ADDR_WIDTH)
    ) ram (
        .rst(!resetn),
        .clk(aclk),

        .s_axi_awid(m_mem_axi_awid),
        .s_axi_awaddr(m_mem_axi_awaddr),
        .s_axi_awlen(m_mem_axi_awlen),
        .s_axi_awsize(m_mem_axi_awsize),
        .s_axi_awburst(m_mem_axi_awburst),
        .s_axi_awlock(m_mem_axi_awlock),
        .s_axi_awcache(m_mem_axi_awcache),
        .s_axi_awprot(m_mem_axi_awprot),
        .s_axi_awvalid(m_mem_axi_awvalid),
        .s_axi_awready(m_mem_axi_awready),
        .s_axi_wdata(m_mem_axi_wdata),
        .s_axi_wstrb(m_mem_axi_wstrb),
        .s_axi_wlast(m_mem_axi_wlast),
        .s_axi_wvalid(m_mem_axi_wvalid),
        .s_axi_wready(m_mem_axi_wready),
        .s_axi_bid(m_mem_axi_bid),
        .s_axi_bresp(m_mem_axi_bresp),
        .s_axi_bvalid(m_mem_axi_bvalid),
        .s_axi_bready(m_mem_axi_bready),
        .s_axi_arid(m_mem_axi_arid),
        .s_axi_araddr(m_mem_axi_araddr),
        .s_axi_arlen(m_mem_axi_arlen),
        .s_axi_arsize(m_mem_axi_arsize),
        .s_axi_arburst(m_mem_axi_arburst),
        .s_axi_arlock(m_mem_axi_arlock),
        .s_axi_arcache(m_mem_axi_arcache),
        .s_axi_arprot(m_mem_axi_arprot),
        .s_axi_arvalid(m_mem_axi_arvalid),
        .s_axi_arready(m_mem_axi_arready),
        .s_axi_rid(m_mem_axi_rid),
        .s_axi_rdata(m_mem_axi_rdata),
        .s_axi_rresp(m_mem_axi_rresp),
        .s_axi_rlast(m_mem_axi_rlast),
        .s_axi_rvalid(m_mem_axi_rvalid),
        .s_axi_rready(m_mem_axi_rready)
    );

    axi_crossbar #(
        .DATA_WIDTH(CMD_STREAM_WIDTH),
        .ADDR_WIDTH(ADDR_WIDTH),
        .S_ID_WIDTH(ID_WIDTH_LOC),
        .S_COUNT(NRS),
        .M_COUNT(1),
        .M_ADDR_WIDTH({ 1 { { 1 { 32'd25 } } } })
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

    Rasterix #(
        .FRAMEBUFFER_SIZE_IN_WORDS(FRAMEBUFFER_SIZE_IN_WORDS),
        .CMD_STREAM_WIDTH(CMD_STREAM_WIDTH),
        .ENABLE_STENCIL_BUFFER(1),
        .TEXTURE_BUFFER_SIZE(TEXTURE_BUFFER_SIZE),
        .FRAMEBUFFER_SUB_PIXEL_WIDTH(5),
        .TMU_COUNT(2),
        .ADDR_WIDTH(FRAMEBUFFER_SIZE_IN_WORDS),
        .ID_WIDTH(ID_WIDTH_LOC)
    ) rasterix (
        .aclk(aclk),
        .resetn(resetn),
        
        .s_cmd_axis_tvalid(s_cmd_axis_tvalid),
        .s_cmd_axis_tready(s_cmd_axis_tready),
        .s_cmd_axis_tlast(s_cmd_axis_tlast),
        .s_cmd_axis_tdata(s_cmd_axis_tdata),

        .m_framebuffer_axis_tvalid(m_framebuffer_axis_tvalid),
        .m_framebuffer_axis_tready(m_framebuffer_axis_tready),
        .m_framebuffer_axis_tlast(m_framebuffer_axis_tlast),
        .m_framebuffer_axis_tdata(m_framebuffer_axis_tdata),

        .swap_fb(),
        .fb_swapped(1),

        .m_common_axi_awid(s_xbar_axi_awid[0 * ID_WIDTH_LOC +: ID_WIDTH_LOC]),
        .m_common_axi_awaddr(s_xbar_axi_awaddr[0 * ADDR_WIDTH +: ADDR_WIDTH]),
        .m_common_axi_awlen(s_xbar_axi_awlen[0 * 8 +: 8]),
        .m_common_axi_awsize(s_xbar_axi_awsize[0 * 3 +: 3]),
        .m_common_axi_awburst(s_xbar_axi_awburst[0 * 2 +: 2]),
        .m_common_axi_awlock(s_xbar_axi_awlock[0 * 1 +: 1]),
        .m_common_axi_awcache(s_xbar_axi_awcache[0 * 4 +: 4]),
        .m_common_axi_awprot(s_xbar_axi_awprot[0 * 3 +: 3]), 
        .m_common_axi_awvalid(s_xbar_axi_awvalid[0 * 1 +: 1]),
        .m_common_axi_awready(s_xbar_axi_awready[0 * 1 +: 1]),

        .m_common_axi_wdata(s_xbar_axi_wdata[0 * CMD_STREAM_WIDTH +: CMD_STREAM_WIDTH]),
        .m_common_axi_wstrb(s_xbar_axi_wstrb[0 * STRB_WIDTH +: STRB_WIDTH]),
        .m_common_axi_wlast(s_xbar_axi_wlast[0 * 1 +: 1]),
        .m_common_axi_wvalid(s_xbar_axi_wvalid[0 * 1 +: 1]),
        .m_common_axi_wready(s_xbar_axi_wready[0 * 1 +: 1]),

        .m_common_axi_bid(s_xbar_axi_bid[0 * ID_WIDTH_LOC +: ID_WIDTH_LOC]),
        .m_common_axi_bresp(s_xbar_axi_bresp[0 * 2 +: 2]),
        .m_common_axi_bvalid(s_xbar_axi_bvalid[0 * 1 +: 1]),
        .m_common_axi_bready(s_xbar_axi_bready[0 * 1 +: 1]),

        .m_common_axi_arid(s_xbar_axi_arid[0 * ID_WIDTH_LOC +: ID_WIDTH_LOC]),
        .m_common_axi_araddr(s_xbar_axi_araddr[0 * ADDR_WIDTH +: ADDR_WIDTH]),
        .m_common_axi_arlen(s_xbar_axi_arlen[0 * 8 +: 8]),
        .m_common_axi_arsize(s_xbar_axi_arsize[0 * 3 +: 3]),
        .m_common_axi_arburst(s_xbar_axi_arburst[0 * 2 +: 2]),
        .m_common_axi_arlock(s_xbar_axi_arlock[0 * 1 +: 1]),
        .m_common_axi_arcache(s_xbar_axi_arcache[0 * 4 +: 4]),
        .m_common_axi_arprot(s_xbar_axi_arprot[0 * 3 +: 3]),
        .m_common_axi_arvalid(s_xbar_axi_arvalid[0 * 1 +: 1]),
        .m_common_axi_arready(s_xbar_axi_arready[0 * 1 +: 1]),

        .m_common_axi_rid(s_xbar_axi_rid[0 * ID_WIDTH_LOC +: ID_WIDTH_LOC]),
        .m_common_axi_rdata(s_xbar_axi_rdata[0 * CMD_STREAM_WIDTH +: CMD_STREAM_WIDTH]),
        .m_common_axi_rresp(s_xbar_axi_rresp[0 * 2 +: 2]),
        .m_common_axi_rlast(s_xbar_axi_rlast[0 * 1 +: 1]),
        .m_common_axi_rvalid(s_xbar_axi_rvalid[0 * 1 +: 1]),
        .m_common_axi_rready(s_xbar_axi_rready[0 * 1 +: 1]),

        .m_color_axi_awid(s_xbar_axi_awid[1 * ID_WIDTH_LOC +: ID_WIDTH_LOC]),
        .m_color_axi_awaddr(s_xbar_axi_awaddr[1 * ADDR_WIDTH +: ADDR_WIDTH]),
        .m_color_axi_awlen(s_xbar_axi_awlen[1 * 8 +: 8]),
        .m_color_axi_awsize(s_xbar_axi_awsize[1 * 3 +: 3]),
        .m_color_axi_awburst(s_xbar_axi_awburst[1 * 2 +: 2]),
        .m_color_axi_awlock(s_xbar_axi_awlock[1 * 1 +: 1]),
        .m_color_axi_awcache(s_xbar_axi_awcache[1 * 4 +: 4]),
        .m_color_axi_awprot(s_xbar_axi_awprot[1 * 3 +: 3]), 
        .m_color_axi_awvalid(s_xbar_axi_awvalid[1 * 1 +: 1]),
        .m_color_axi_awready(s_xbar_axi_awready[1 * 1 +: 1]),

        .m_color_axi_wdata(s_xbar_axi_wdata[1 * CMD_STREAM_WIDTH +: CMD_STREAM_WIDTH]),
        .m_color_axi_wstrb(s_xbar_axi_wstrb[1 * STRB_WIDTH +: STRB_WIDTH]),
        .m_color_axi_wlast(s_xbar_axi_wlast[1 * 1 +: 1]),
        .m_color_axi_wvalid(s_xbar_axi_wvalid[1 * 1 +: 1]),
        .m_color_axi_wready(s_xbar_axi_wready[1 * 1 +: 1]),

        .m_color_axi_bid(s_xbar_axi_bid[1 * ID_WIDTH_LOC +: ID_WIDTH_LOC]),
        .m_color_axi_bresp(s_xbar_axi_bresp[1 * 2 +: 2]),
        .m_color_axi_bvalid(s_xbar_axi_bvalid[1 * 1 +: 1]),
        .m_color_axi_bready(s_xbar_axi_bready[1 * 1 +: 1]),

        .m_color_axi_arid(s_xbar_axi_arid[1 * ID_WIDTH_LOC +: ID_WIDTH_LOC]),
        .m_color_axi_araddr(s_xbar_axi_araddr[1 * ADDR_WIDTH +: ADDR_WIDTH]),
        .m_color_axi_arlen(s_xbar_axi_arlen[1 * 8 +: 8]),
        .m_color_axi_arsize(s_xbar_axi_arsize[1 * 3 +: 3]),
        .m_color_axi_arburst(s_xbar_axi_arburst[1 * 2 +: 2]),
        .m_color_axi_arlock(s_xbar_axi_arlock[1 * 1 +: 1]),
        .m_color_axi_arcache(s_xbar_axi_arcache[1 * 4 +: 4]),
        .m_color_axi_arprot(s_xbar_axi_arprot[1 * 3 +: 3]),
        .m_color_axi_arvalid(s_xbar_axi_arvalid[1 * 1 +: 1]),
        .m_color_axi_arready(s_xbar_axi_arready[1 * 1 +: 1]),

        .m_color_axi_rid(s_xbar_axi_rid[1 * ID_WIDTH_LOC +: ID_WIDTH_LOC]),
        .m_color_axi_rdata(s_xbar_axi_rdata[1 * CMD_STREAM_WIDTH +: CMD_STREAM_WIDTH]),
        .m_color_axi_rresp(s_xbar_axi_rresp[1 * 2 +: 2]),
        .m_color_axi_rlast(s_xbar_axi_rlast[1 * 1 +: 1]),
        .m_color_axi_rvalid(s_xbar_axi_rvalid[1 * 1 +: 1]),
        .m_color_axi_rready(s_xbar_axi_rready[1 * 1 +: 1]),

        .m_depth_axi_awid(s_xbar_axi_awid[2 * ID_WIDTH_LOC +: ID_WIDTH_LOC]),
        .m_depth_axi_awaddr(s_xbar_axi_awaddr[2 * ADDR_WIDTH +: ADDR_WIDTH]),
        .m_depth_axi_awlen(s_xbar_axi_awlen[2 * 8 +: 8]),
        .m_depth_axi_awsize(s_xbar_axi_awsize[2 * 3 +: 3]),
        .m_depth_axi_awburst(s_xbar_axi_awburst[2 * 2 +: 2]),
        .m_depth_axi_awlock(s_xbar_axi_awlock[2 * 1 +: 1]),
        .m_depth_axi_awcache(s_xbar_axi_awcache[2 * 4 +: 4]),
        .m_depth_axi_awprot(s_xbar_axi_awprot[2 * 3 +: 3]), 
        .m_depth_axi_awvalid(s_xbar_axi_awvalid[2 * 1 +: 1]),
        .m_depth_axi_awready(s_xbar_axi_awready[2 * 1 +: 1]),

        .m_depth_axi_wdata(s_xbar_axi_wdata[2 * CMD_STREAM_WIDTH +: CMD_STREAM_WIDTH]),
        .m_depth_axi_wstrb(s_xbar_axi_wstrb[2 * STRB_WIDTH +: STRB_WIDTH]),
        .m_depth_axi_wlast(s_xbar_axi_wlast[2 * 1 +: 1]),
        .m_depth_axi_wvalid(s_xbar_axi_wvalid[2 * 1 +: 1]),
        .m_depth_axi_wready(s_xbar_axi_wready[2 * 1 +: 1]),

        .m_depth_axi_bid(s_xbar_axi_bid[2 * ID_WIDTH_LOC +: ID_WIDTH_LOC]),
        .m_depth_axi_bresp(s_xbar_axi_bresp[2 * 2 +: 2]),
        .m_depth_axi_bvalid(s_xbar_axi_bvalid[2 * 1 +: 1]),
        .m_depth_axi_bready(s_xbar_axi_bready[2 * 1 +: 1]),

        .m_depth_axi_arid(s_xbar_axi_arid[2 * ID_WIDTH_LOC +: ID_WIDTH_LOC]),
        .m_depth_axi_araddr(s_xbar_axi_araddr[2 * ADDR_WIDTH +: ADDR_WIDTH]),
        .m_depth_axi_arlen(s_xbar_axi_arlen[2 * 8 +: 8]),
        .m_depth_axi_arsize(s_xbar_axi_arsize[2 * 3 +: 3]),
        .m_depth_axi_arburst(s_xbar_axi_arburst[2 * 2 +: 2]),
        .m_depth_axi_arlock(s_xbar_axi_arlock[2 * 1 +: 1]),
        .m_depth_axi_arcache(s_xbar_axi_arcache[2 * 4 +: 4]),
        .m_depth_axi_arprot(s_xbar_axi_arprot[2 * 3 +: 3]),
        .m_depth_axi_arvalid(s_xbar_axi_arvalid[2 * 1 +: 1]),
        .m_depth_axi_arready(s_xbar_axi_arready[2 * 1 +: 1]),

        .m_depth_axi_rid(s_xbar_axi_rid[2 * ID_WIDTH_LOC +: ID_WIDTH_LOC]),
        .m_depth_axi_rdata(s_xbar_axi_rdata[2 * CMD_STREAM_WIDTH +: CMD_STREAM_WIDTH]),
        .m_depth_axi_rresp(s_xbar_axi_rresp[2 * 2 +: 2]),
        .m_depth_axi_rlast(s_xbar_axi_rlast[2 * 1 +: 1]),
        .m_depth_axi_rvalid(s_xbar_axi_rvalid[2 * 1 +: 1]),
        .m_depth_axi_rready(s_xbar_axi_rready[2 * 1 +: 1]),

        .m_stencil_axi_awid(s_xbar_axi_awid[3 * ID_WIDTH_LOC +: ID_WIDTH_LOC]),
        .m_stencil_axi_awaddr(s_xbar_axi_awaddr[3 * ADDR_WIDTH +: ADDR_WIDTH]),
        .m_stencil_axi_awlen(s_xbar_axi_awlen[3 * 8 +: 8]),
        .m_stencil_axi_awsize(s_xbar_axi_awsize[3 * 3 +: 3]),
        .m_stencil_axi_awburst(s_xbar_axi_awburst[3 * 2 +: 2]),
        .m_stencil_axi_awlock(s_xbar_axi_awlock[3 * 1 +: 1]),
        .m_stencil_axi_awcache(s_xbar_axi_awcache[3 * 4 +: 4]),
        .m_stencil_axi_awprot(s_xbar_axi_awprot[3 * 3 +: 3]), 
        .m_stencil_axi_awvalid(s_xbar_axi_awvalid[3 * 1 +: 1]),
        .m_stencil_axi_awready(s_xbar_axi_awready[3 * 1 +: 1]),

        .m_stencil_axi_wdata(s_xbar_axi_wdata[3 * CMD_STREAM_WIDTH +: CMD_STREAM_WIDTH]),
        .m_stencil_axi_wstrb(s_xbar_axi_wstrb[3 * STRB_WIDTH +: STRB_WIDTH]),
        .m_stencil_axi_wlast(s_xbar_axi_wlast[3 * 1 +: 1]),
        .m_stencil_axi_wvalid(s_xbar_axi_wvalid[3 * 1 +: 1]),
        .m_stencil_axi_wready(s_xbar_axi_wready[3 * 1 +: 1]),

        .m_stencil_axi_bid(s_xbar_axi_bid[3 * ID_WIDTH_LOC +: ID_WIDTH_LOC]),
        .m_stencil_axi_bresp(s_xbar_axi_bresp[3 * 2 +: 2]),
        .m_stencil_axi_bvalid(s_xbar_axi_bvalid[3 * 1 +: 1]),
        .m_stencil_axi_bready(s_xbar_axi_bready[3 * 1 +: 1]),

        .m_stencil_axi_arid(s_xbar_axi_arid[3 * ID_WIDTH_LOC +: ID_WIDTH_LOC]),
        .m_stencil_axi_araddr(s_xbar_axi_araddr[3 * ADDR_WIDTH +: ADDR_WIDTH]),
        .m_stencil_axi_arlen(s_xbar_axi_arlen[3 * 8 +: 8]),
        .m_stencil_axi_arsize(s_xbar_axi_arsize[3 * 3 +: 3]),
        .m_stencil_axi_arburst(s_xbar_axi_arburst[3 * 2 +: 2]),
        .m_stencil_axi_arlock(s_xbar_axi_arlock[3 * 1 +: 1]),
        .m_stencil_axi_arcache(s_xbar_axi_arcache[3 * 4 +: 4]),
        .m_stencil_axi_arprot(s_xbar_axi_arprot[3 * 3 +: 3]),
        .m_stencil_axi_arvalid(s_xbar_axi_arvalid[3 * 1 +: 1]),
        .m_stencil_axi_arready(s_xbar_axi_arready[3 * 1 +: 1]),

        .m_stencil_axi_rid(s_xbar_axi_rid[3 * ID_WIDTH_LOC +: ID_WIDTH_LOC]),
        .m_stencil_axi_rdata(s_xbar_axi_rdata[3 * CMD_STREAM_WIDTH +: CMD_STREAM_WIDTH]),
        .m_stencil_axi_rresp(s_xbar_axi_rresp[3 * 2 +: 2]),
        .m_stencil_axi_rlast(s_xbar_axi_rlast[3 * 1 +: 1]),
        .m_stencil_axi_rvalid(s_xbar_axi_rvalid[3 * 1 +: 1]),
        .m_stencil_axi_rready(s_xbar_axi_rready[3 * 1 +: 1])
    );
endmodule
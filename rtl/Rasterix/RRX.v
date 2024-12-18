// Rasterix
// https://github.com/ToNi3141/Rasterix
// Copyright (c) 2024 ToNi3141

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

module RRX #(
    // The size of the internal framebuffer (in power of two)
    // Depth buffer word size: 16 bit
    // Color buffer word size: FRAMEBUFFER_SUB_PIXEL_WIDTH * (FRAMEBUFFER_ENABLE_ALPHA_CHANNEL ? 4 : 3)
    parameter FRAMEBUFFER_SIZE_IN_WORDS = 16,

    // This is the color depth of the framebuffer. Note: This setting has no influence on the framebuffer stream. This steam will
    // stay at RGB565. It changes the internal representation and might be used to reduce the memory footprint.
    // Lower depth will result in color banding.
    parameter FRAMEBUFFER_SUB_PIXEL_WIDTH = 5,
    // This enables the alpha channel of the framebuffer. Requires additional memory.
    parameter FRAMEBUFFER_ENABLE_ALPHA_CHANNEL = 0,

    // This enables the 4 bit stencil buffer
    parameter ENABLE_STENCIL_BUFFER = 1,

    // Number of TMUs. Currently supported values: 1 and 2
    parameter TMU_COUNT = 2,
    parameter ENABLE_MIPMAPPING = 1,
    parameter TMU_PAGE_SIZE = 4096,
    
    // The size of the texture in bytes
    parameter TEXTURE_BUFFER_SIZE = 17, // 128kB enough for 256x256px textures

    // Memory address width
    parameter ADDR_WIDTH = 32,
    // Memory ID width
    parameter ID_WIDTH = 8,
    // Memory data width
    parameter DATA_WIDTH = 32,
    // Memory strobe width
    parameter STRB_WIDTH = DATA_WIDTH / 8,

    // Configures the precision of the float calculations (interpolation of textures, depth, ...)
    // A lower value can significant reduce the logic consumption but can cause visible 
    // distortions in the rendered image.
    // 4 bit reducing can safe around 1k LUTs.
    // For compatibility reasons, it only cuts of the mantissa. By default it uses a 25x25 multiplier (for floatMul)
    // If you have a FPGA with only 18 bit native multipliers, reduce this value to 26.
    parameter RASTERIZER_FLOAT_PRECISION = 32,
    // When RASTERIZER_ENABLE_FLOAT_INTERPOLATION is 0, then this configures the width of the multipliers for the fix point
    // calculations. A value of 25 will instantiate signed 25 bit multipliers. The 25 already including the sign bit.
    // Lower values can lead to distortions of the fog and texels.
    parameter RASTERIZER_FIXPOINT_PRECISION = 25,
    // Enables the floating point interpolation. If this is disabled, it falls back
    // to the fix point interpolation
    parameter RASTERIZER_ENABLE_FLOAT_INTERPOLATION = 0,


    localparam CMD_STREAM_WIDTH = 32
)
(
    input  wire                             aclk,
    input  wire                             resetn,

    // AXI Stream command interface
    input  wire                             s_cmd_axis_tvalid,
    output wire                             s_cmd_axis_tready,
    input  wire                             s_cmd_axis_tlast,
    input  wire [CMD_STREAM_WIDTH - 1 : 0]  s_cmd_axis_tdata,

    // AXI Stream framebuffer
    output wire                             m_framebuffer_axis_tvalid,
    input  wire                             m_framebuffer_axis_tready,
    output wire                             m_framebuffer_axis_tlast,
    output wire [CMD_STREAM_WIDTH - 1 : 0]  m_framebuffer_axis_tdata,
    // Framebuffer
    output wire                             swap_fb,
    output wire [ADDR_WIDTH - 1 : 0]        fb_addr,
    input  wire                             fb_swapped,

    // Memory Interface
    output wire [ID_WIDTH - 1 : 0]          m_axi_awid,
    output wire [ADDR_WIDTH - 1 : 0]        m_axi_awaddr,
    output wire [ 7 : 0]                    m_axi_awlen,
    output wire [ 2 : 0]                    m_axi_awsize,
    output wire [ 1 : 0]                    m_axi_awburst,
    output wire                             m_axi_awlock,
    output wire [ 3 : 0]                    m_axi_awcache,
    output wire [ 2 : 0]                    m_axi_awprot,
    output wire                             m_axi_awvalid,
    input  wire                             m_axi_awready,
    output wire [DATA_WIDTH - 1 : 0]        m_axi_wdata,
    output wire [STRB_WIDTH - 1 : 0]        m_axi_wstrb,
    output wire                             m_axi_wlast,
    output wire                             m_axi_wvalid,
    input  wire                             m_axi_wready,
    input  wire [ID_WIDTH - 1 : 0]          m_axi_bid,
    input  wire [ 1 : 0]                    m_axi_bresp,
    input  wire                             m_axi_bvalid,
    output wire                             m_axi_bready,
    output wire [ID_WIDTH - 1 : 0]          m_axi_arid,
    output wire [ADDR_WIDTH - 1 : 0]        m_axi_araddr,
    output wire [ 7 : 0]                    m_axi_arlen,
    output wire [ 2 : 0]                    m_axi_arsize,
    output wire [ 1 : 0]                    m_axi_arburst,
    output wire                             m_axi_arlock,
    output wire [ 3 : 0]                    m_axi_arcache,
    output wire [ 2 : 0]                    m_axi_arprot,
    output wire                             m_axi_arvalid,
    input  wire                             m_axi_arready,
    input  wire [ID_WIDTH - 1 : 0]          m_axi_rid,
    input  wire [DATA_WIDTH - 1 : 0]        m_axi_rdata,
    input  wire [ 1 : 0]                    m_axi_rresp,
    input  wire                             m_axi_rlast,
    input  wire                             m_axi_rvalid,
    output wire                             m_axi_rready
);
    localparam ID_WIDTH_LOC = ID_WIDTH - 2;
    localparam NRS = 3;

    wire [(NRS * ID_WIDTH_LOC) - 1 : 0]     xbar_axi_awid;
    wire [(NRS * ADDR_WIDTH) - 1 : 0]       xbar_axi_awaddr;
    wire [(NRS * 8) - 1 : 0]                xbar_axi_awlen; 
    wire [(NRS * 3) - 1 : 0]                xbar_axi_awsize;
    wire [(NRS * 2) - 1 : 0]                xbar_axi_awburst;
    wire [NRS - 1 : 0]                      xbar_axi_awlock;
    wire [(NRS * 4) - 1 : 0]                xbar_axi_awcache;
    wire [(NRS * 3) - 1 : 0]                xbar_axi_awprot; 
    wire [NRS - 1 : 0]                      xbar_axi_awvalid;
    wire [NRS - 1 : 0]                      xbar_axi_awready;

    wire [(NRS * DATA_WIDTH) - 1 : 0]       xbar_axi_wdata;
    wire [(NRS * STRB_WIDTH) - 1 : 0]       xbar_axi_wstrb;
    wire [NRS - 1 : 0]                      xbar_axi_wlast;
    wire [NRS - 1 : 0]                      xbar_axi_wvalid;
    wire [NRS - 1 : 0]                      xbar_axi_wready;

    wire [(NRS * ID_WIDTH_LOC) - 1 : 0]     xbar_axi_bid;
    wire [(NRS * 2) - 1 : 0]                xbar_axi_bresp;
    wire [NRS - 1 : 0]                      xbar_axi_bvalid;
    wire [NRS - 1 : 0]                      xbar_axi_bready;

    wire [(NRS * ID_WIDTH_LOC) - 1 : 0]     xbar_axi_arid;
    wire [(NRS * ADDR_WIDTH) - 1 : 0]       xbar_axi_araddr;
    wire [(NRS * 8) - 1 : 0]                xbar_axi_arlen;
    wire [(NRS * 3) - 1 : 0]                xbar_axi_arsize;
    wire [(NRS * 2) - 1 : 0]                xbar_axi_arburst;
    wire [NRS - 1 : 0]                      xbar_axi_arlock;
    wire [(NRS * 4) - 1 : 0]                xbar_axi_arcache;
    wire [(NRS * 3) - 1 : 0]                xbar_axi_arprot;
    wire [NRS - 1 : 0]                      xbar_axi_arvalid;
    wire [NRS - 1 : 0]                      xbar_axi_arready;

    wire [(NRS * ID_WIDTH_LOC) - 1 : 0]     xbar_axi_rid;
    wire [(NRS * DATA_WIDTH) - 1 : 0]       xbar_axi_rdata;
    wire [(NRS * 2) - 1 : 0]                xbar_axi_rresp;
    wire [NRS - 1 : 0]                      xbar_axi_rlast;
    wire [NRS - 1 : 0]                      xbar_axi_rvalid;
    wire [NRS - 1 : 0]                      xbar_axi_rready;

    axi_crossbar #(
        .DATA_WIDTH(DATA_WIDTH),
        .ADDR_WIDTH(ADDR_WIDTH),
        .S_ID_WIDTH(ID_WIDTH_LOC),
        .S_COUNT(NRS),
        .M_COUNT(1),
        .M_ID_WIDTH(ID_WIDTH),
        .M_ADDR_WIDTH( ADDR_WIDTH[0 +: 32])
    ) mainXBar (
        .clk(aclk),
        .rst(!resetn),

        .s_axi_awid(xbar_axi_awid),
        .s_axi_awaddr(xbar_axi_awaddr),
        .s_axi_awlen(xbar_axi_awlen),
        .s_axi_awsize(xbar_axi_awsize),
        .s_axi_awburst(xbar_axi_awburst),
        .s_axi_awlock(xbar_axi_awlock),
        .s_axi_awcache(xbar_axi_awcache),
        .s_axi_awprot(xbar_axi_awprot),
        .s_axi_awqos(0),
        .s_axi_awuser(0),
        .s_axi_awvalid(xbar_axi_awvalid),
        .s_axi_awready(xbar_axi_awready),

        .s_axi_wdata(xbar_axi_wdata),
        .s_axi_wstrb(xbar_axi_wstrb),
        .s_axi_wlast(xbar_axi_wlast),
        .s_axi_wuser(0),
        .s_axi_wvalid(xbar_axi_wvalid),
        .s_axi_wready(xbar_axi_wready),

        .s_axi_bid(xbar_axi_bid),
        .s_axi_bresp(xbar_axi_bresp),
        .s_axi_buser(),
        .s_axi_bvalid(xbar_axi_bvalid),
        .s_axi_bready(xbar_axi_bready),

        .s_axi_arid(xbar_axi_arid),
        .s_axi_araddr(xbar_axi_araddr),
        .s_axi_arlen(xbar_axi_arlen),
        .s_axi_arsize(xbar_axi_arsize),
        .s_axi_arburst(xbar_axi_arburst),
        .s_axi_arlock(xbar_axi_arlock),
        .s_axi_arcache(xbar_axi_arcache),
        .s_axi_arprot(xbar_axi_arprot),
        .s_axi_arqos(0),
        .s_axi_aruser(0),
        .s_axi_arvalid(xbar_axi_arvalid),
        .s_axi_arready(xbar_axi_arready),

        .s_axi_rid(xbar_axi_rid),
        .s_axi_rdata(xbar_axi_rdata),
        .s_axi_rresp(xbar_axi_rresp),
        .s_axi_rlast(xbar_axi_rlast),
        .s_axi_ruser(),
        .s_axi_rvalid(xbar_axi_rvalid),
        .s_axi_rready(xbar_axi_rready),

        .m_axi_awid(m_axi_awid),
        .m_axi_awaddr(m_axi_awaddr),
        .m_axi_awlen(m_axi_awlen),
        .m_axi_awsize(m_axi_awsize),
        .m_axi_awburst(m_axi_awburst),
        .m_axi_awlock(m_axi_awlock),
        .m_axi_awcache(m_axi_awcache),
        .m_axi_awprot(m_axi_awprot),
        .m_axi_awqos(),
        .m_axi_awregion(),
        .m_axi_awuser(),
        .m_axi_awvalid(m_axi_awvalid),
        .m_axi_awready(m_axi_awready),

        .m_axi_wdata(m_axi_wdata),
        .m_axi_wstrb(m_axi_wstrb),
        .m_axi_wlast(m_axi_wlast),
        .m_axi_wuser(),
        .m_axi_wvalid(m_axi_wvalid),
        .m_axi_wready(m_axi_wready),

        .m_axi_bid(m_axi_bid),
        .m_axi_bresp(m_axi_bresp),
        .m_axi_buser(0),
        .m_axi_bvalid(m_axi_bvalid),
        .m_axi_bready(m_axi_bready),

        .m_axi_arid(m_axi_arid),
        .m_axi_araddr(m_axi_araddr),
        .m_axi_arlen(m_axi_arlen),
        .m_axi_arsize(m_axi_arsize),
        .m_axi_arburst(m_axi_arburst),
        .m_axi_arlock(m_axi_arlock),
        .m_axi_arcache(m_axi_arcache),
        .m_axi_arqos(),
        .m_axi_arregion(),
        .m_axi_aruser(),
        .m_axi_arprot(m_axi_arprot),
        .m_axi_arvalid(m_axi_arvalid),
        .m_axi_arready(m_axi_arready),

        .m_axi_rid(m_axi_rid),
        .m_axi_rdata(m_axi_rdata),
        .m_axi_rresp(m_axi_rresp),
        .m_axi_rlast(m_axi_rlast),
        .m_axi_ruser(0),
        .m_axi_rvalid(m_axi_rvalid),
        .m_axi_rready(m_axi_rready)
    );

    localparam CMD_STREAM_STRB_WIDTH = CMD_STREAM_WIDTH / 8;
    wire [ID_WIDTH_LOC - 1 : 0]             common_axi_awid;
    wire [ADDR_WIDTH - 1 : 0]               common_axi_awaddr;
    wire [ 7 : 0]                           common_axi_awlen;
    wire [ 2 : 0]                           common_axi_awsize;
    wire [ 1 : 0]                           common_axi_awburst;
    wire                                    common_axi_awlock;
    wire [ 3 : 0]                           common_axi_awcache;
    wire [ 2 : 0]                           common_axi_awprot;
    wire                                    common_axi_awvalid;
    wire                                    common_axi_awready;
    wire [CMD_STREAM_WIDTH - 1 : 0]         common_axi_wdata;
    wire [CMD_STREAM_STRB_WIDTH - 1 : 0]    common_axi_wstrb;
    wire                                    common_axi_wlast;
    wire                                    common_axi_wvalid;
    wire                                    common_axi_wready;
    wire [ID_WIDTH_LOC - 1 : 0]             common_axi_bid;
    wire [ 1 : 0]                           common_axi_bresp;
    wire                                    common_axi_bvalid;
    wire                                    common_axi_bready;
    wire [ID_WIDTH_LOC - 1 : 0]             common_axi_arid;
    wire [ADDR_WIDTH - 1 : 0]               common_axi_araddr;
    wire [ 7 : 0]                           common_axi_arlen;
    wire [ 2 : 0]                           common_axi_arsize;
    wire [ 1 : 0]                           common_axi_arburst;
    wire                                    common_axi_arlock;
    wire [ 3 : 0]                           common_axi_arcache;
    wire [ 2 : 0]                           common_axi_arprot;
    wire                                    common_axi_arvalid;
    wire                                    common_axi_arready;
    wire [ID_WIDTH_LOC - 1 : 0]             common_axi_rid;
    wire [CMD_STREAM_WIDTH - 1 : 0]         common_axi_rdata;
    wire [ 1 : 0]                           common_axi_rresp;
    wire                                    common_axi_rlast;
    wire                                    common_axi_rvalid;
    wire                                    common_axi_rready;

    axi_adapter #(
        .ADDR_WIDTH(ADDR_WIDTH),
        .M_DATA_WIDTH(DATA_WIDTH),
        .M_STRB_WIDTH(STRB_WIDTH),
        .S_DATA_WIDTH(CMD_STREAM_WIDTH),
        .S_STRB_WIDTH(CMD_STREAM_STRB_WIDTH),
        .ID_WIDTH(ID_WIDTH_LOC)
    ) commonAxiAdapter
    (
        .clk(aclk),
        .rst(!resetn),

        .m_axi_awid(xbar_axi_awid[0 * ID_WIDTH_LOC +: ID_WIDTH_LOC]),
        .m_axi_awaddr(xbar_axi_awaddr[0 * ADDR_WIDTH +: ADDR_WIDTH]),
        .m_axi_awlen(xbar_axi_awlen[0 * 8 +: 8]),
        .m_axi_awsize(xbar_axi_awsize[0 * 3 +: 3]),
        .m_axi_awburst(xbar_axi_awburst[0 * 2 +: 2]),
        .m_axi_awlock(xbar_axi_awlock[0 * 1 +: 1]),
        .m_axi_awcache(xbar_axi_awcache[0 * 4 +: 4]),
        .m_axi_awprot(xbar_axi_awprot[0 * 3 +: 3]),
        .m_axi_awqos(),
        .m_axi_awregion(),
        .m_axi_awuser(),
        .m_axi_awvalid(xbar_axi_awvalid[0 * 1 +: 1]),
        .m_axi_awready(xbar_axi_awready[0 * 1 +: 1]),
        .m_axi_wdata(xbar_axi_wdata[0 * DATA_WIDTH +: DATA_WIDTH]),
        .m_axi_wstrb(xbar_axi_wstrb[0 * STRB_WIDTH +: STRB_WIDTH]),
        .m_axi_wlast(xbar_axi_wlast[0 * 1 +: 1]),
        .m_axi_wuser(),
        .m_axi_wvalid(xbar_axi_wvalid[0 * 1 +: 1]),
        .m_axi_wready(xbar_axi_wready[0 * 1 +: 1]),
        .m_axi_bid(xbar_axi_bid[0 * ID_WIDTH_LOC +: ID_WIDTH_LOC]),
        .m_axi_bresp(xbar_axi_bresp[0 * 2 +: 2]),
        .m_axi_buser(0),
        .m_axi_bvalid(xbar_axi_bvalid[0 * 1 +: 1]),
        .m_axi_bready(xbar_axi_bready[0 * 1 +: 1]),
        .m_axi_arid(xbar_axi_arid[0 * ID_WIDTH_LOC +: ID_WIDTH_LOC]),
        .m_axi_araddr(xbar_axi_araddr[0 * ADDR_WIDTH +: ADDR_WIDTH]),
        .m_axi_arlen(xbar_axi_arlen[0 * 8 +: 8]),
        .m_axi_arsize(xbar_axi_arsize[0 * 3 +: 3]),
        .m_axi_arburst(xbar_axi_arburst[0 * 2 +: 2]),
        .m_axi_arlock(xbar_axi_arlock[0 * 1 +: 1]),
        .m_axi_arcache(xbar_axi_arcache[0 * 4 +: 4]),
        .m_axi_arprot(xbar_axi_arprot[0 * 3 +: 3]),
        .m_axi_arqos(),
        .m_axi_arregion(),
        .m_axi_aruser(),
        .m_axi_arvalid(xbar_axi_arvalid[0 * 1 +: 1]),
        .m_axi_arready(xbar_axi_arready[0 * 1 +: 1]),
        .m_axi_rid(xbar_axi_rid[0 * ID_WIDTH_LOC +: ID_WIDTH_LOC]),
        .m_axi_rdata(xbar_axi_rdata[0 * DATA_WIDTH +: DATA_WIDTH]),
        .m_axi_rresp(xbar_axi_rresp[0 * 2 +: 2]),
        .m_axi_rlast(xbar_axi_rlast[0 * 1 +: 1]),
        .m_axi_ruser(0),
        .m_axi_rvalid(xbar_axi_rvalid[0 * 1 +: 1]),
        .m_axi_rready(xbar_axi_rready[0 * 1 +: 1]),

        .s_axi_awid(common_axi_awid),
        .s_axi_awaddr(common_axi_awaddr),
        .s_axi_awlen(common_axi_awlen),
        .s_axi_awsize(common_axi_awsize),
        .s_axi_awburst(common_axi_awburst),
        .s_axi_awlock(common_axi_awlock),
        .s_axi_awcache(common_axi_awcache),
        .s_axi_awprot(common_axi_awprot),
        .s_axi_awqos(0),
        .s_axi_awregion(0),
        .s_axi_awuser(0),
        .s_axi_awvalid(common_axi_awvalid),
        .s_axi_awready(common_axi_awready),
        .s_axi_wdata(common_axi_wdata),
        .s_axi_wstrb(common_axi_wstrb),
        .s_axi_wlast(common_axi_wlast),
        .s_axi_wuser(0),
        .s_axi_wvalid(common_axi_wvalid),
        .s_axi_wready(common_axi_wready),
        .s_axi_bid(common_axi_bid),
        .s_axi_bresp(common_axi_bresp),
        .s_axi_buser(),
        .s_axi_bvalid(common_axi_bvalid),
        .s_axi_bready(common_axi_bready),
        .s_axi_arid(common_axi_arid),
        .s_axi_araddr(common_axi_araddr),
        .s_axi_arlen(common_axi_arlen),
        .s_axi_arsize(common_axi_arsize),
        .s_axi_arburst(common_axi_arburst),
        .s_axi_arlock(common_axi_arlock),
        .s_axi_arcache(common_axi_arcache),
        .s_axi_arprot(common_axi_arprot),
        .s_axi_arqos(0),
        .s_axi_arregion(0),
        .s_axi_aruser(0),
        .s_axi_arvalid(common_axi_arvalid),
        .s_axi_arready(common_axi_arready),
        .s_axi_rid(common_axi_rid),
        .s_axi_rdata(common_axi_rdata),
        .s_axi_rresp(common_axi_rresp),
        .s_axi_rlast(common_axi_rlast),
        .s_axi_ruser(),
        .s_axi_rvalid(common_axi_rvalid),
        .s_axi_rready(common_axi_rready)
    );

    RasterixIF #(
        .FRAMEBUFFER_SIZE_IN_WORDS(FRAMEBUFFER_SIZE_IN_WORDS),
        .FRAMEBUFFER_ENABLE_ALPHA_CHANNEL(FRAMEBUFFER_ENABLE_ALPHA_CHANNEL),
        .TMU_PAGE_SIZE(TMU_PAGE_SIZE),
        .ADDR_WIDTH(ADDR_WIDTH),
        .ID_WIDTH(ID_WIDTH_LOC),
        .DATA_WIDTH(DATA_WIDTH),
        .ENABLE_STENCIL_BUFFER(ENABLE_STENCIL_BUFFER),
        .TEXTURE_BUFFER_SIZE(TEXTURE_BUFFER_SIZE),
        .ENABLE_MIPMAPPING(ENABLE_MIPMAPPING),
        .FRAMEBUFFER_SUB_PIXEL_WIDTH(FRAMEBUFFER_SUB_PIXEL_WIDTH),
        .TMU_COUNT(TMU_COUNT),
        .RASTERIZER_ENABLE_FLOAT_INTERPOLATION(RASTERIZER_ENABLE_FLOAT_INTERPOLATION),
        .RASTERIZER_FLOAT_PRECISION(RASTERIZER_FLOAT_PRECISION),
        .RASTERIZER_FIXPOINT_PRECISION(RASTERIZER_FIXPOINT_PRECISION)
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

        .swap_fb(swap_fb),
        .fb_addr(fb_addr),
        .fb_swapped(fb_swapped),

        .m_common_axi_awid(common_axi_awid),
        .m_common_axi_awaddr(common_axi_awaddr),
        .m_common_axi_awlen(common_axi_awlen),
        .m_common_axi_awsize(common_axi_awsize),
        .m_common_axi_awburst(common_axi_awburst),
        .m_common_axi_awlock(common_axi_awlock),
        .m_common_axi_awcache(common_axi_awcache),
        .m_common_axi_awprot(common_axi_awprot), 
        .m_common_axi_awvalid(common_axi_awvalid),
        .m_common_axi_awready(common_axi_awready),

        .m_common_axi_wdata(common_axi_wdata),
        .m_common_axi_wstrb(common_axi_wstrb),
        .m_common_axi_wlast(common_axi_wlast),
        .m_common_axi_wvalid(common_axi_wvalid),
        .m_common_axi_wready(common_axi_wready),

        .m_common_axi_bid(common_axi_bid),
        .m_common_axi_bresp(common_axi_bresp),
        .m_common_axi_bvalid(common_axi_bvalid),
        .m_common_axi_bready(common_axi_bready),

        .m_common_axi_arid(common_axi_arid),
        .m_common_axi_araddr(common_axi_araddr),
        .m_common_axi_arlen(common_axi_arlen),
        .m_common_axi_arsize(common_axi_arsize),
        .m_common_axi_arburst(common_axi_arburst),
        .m_common_axi_arlock(common_axi_arlock),
        .m_common_axi_arcache(common_axi_arcache),
        .m_common_axi_arprot(common_axi_arprot),
        .m_common_axi_arvalid(common_axi_arvalid),
        .m_common_axi_arready(common_axi_arready),

        .m_common_axi_rid(common_axi_rid),
        .m_common_axi_rdata(common_axi_rdata),
        .m_common_axi_rresp(common_axi_rresp),
        .m_common_axi_rlast(common_axi_rlast),
        .m_common_axi_rvalid(common_axi_rvalid),
        .m_common_axi_rready(common_axi_rready),

        .m_tmu0_axi_arid(xbar_axi_arid[1 * ID_WIDTH_LOC +: ID_WIDTH_LOC]),
        .m_tmu0_axi_araddr(xbar_axi_araddr[1 * ADDR_WIDTH +: ADDR_WIDTH]),
        .m_tmu0_axi_arlen(xbar_axi_arlen[1 * 8 +: 8]),
        .m_tmu0_axi_arsize(xbar_axi_arsize[1 * 3 +: 3]),
        .m_tmu0_axi_arburst(xbar_axi_arburst[1 * 2 +: 2]),
        .m_tmu0_axi_arlock(xbar_axi_arlock[1 * 1 +: 1]),
        .m_tmu0_axi_arcache(xbar_axi_arcache[1 * 4 +: 4]),
        .m_tmu0_axi_arprot(xbar_axi_arprot[1 * 3 +: 3]),
        .m_tmu0_axi_arvalid(xbar_axi_arvalid[1 * 1 +: 1]),
        .m_tmu0_axi_arready(xbar_axi_arready[1 * 1 +: 1]),

        .m_tmu0_axi_rid(xbar_axi_rid[1 * ID_WIDTH_LOC +: ID_WIDTH_LOC]),
        .m_tmu0_axi_rdata(xbar_axi_rdata[1 * DATA_WIDTH +: DATA_WIDTH]),
        .m_tmu0_axi_rresp(xbar_axi_rresp[1 * 2 +: 2]),
        .m_tmu0_axi_rlast(xbar_axi_rlast[1 * 1 +: 1]),
        .m_tmu0_axi_rvalid(xbar_axi_rvalid[1 * 1 +: 1]),
        .m_tmu0_axi_rready(xbar_axi_rready[1 * 1 +: 1]),

        .m_tmu1_axi_arid(xbar_axi_arid[2 * ID_WIDTH_LOC +: ID_WIDTH_LOC]),
        .m_tmu1_axi_araddr(xbar_axi_araddr[2 * ADDR_WIDTH +: ADDR_WIDTH]),
        .m_tmu1_axi_arlen(xbar_axi_arlen[2 * 8 +: 8]),
        .m_tmu1_axi_arsize(xbar_axi_arsize[2 * 3 +: 3]),
        .m_tmu1_axi_arburst(xbar_axi_arburst[2 * 2 +: 2]),
        .m_tmu1_axi_arlock(xbar_axi_arlock[2 * 1 +: 1]),
        .m_tmu1_axi_arcache(xbar_axi_arcache[2 * 4 +: 4]),
        .m_tmu1_axi_arprot(xbar_axi_arprot[2 * 3 +: 3]),
        .m_tmu1_axi_arvalid(xbar_axi_arvalid[2 * 1 +: 1]),
        .m_tmu1_axi_arready(xbar_axi_arready[2 * 1 +: 1]),

        .m_tmu1_axi_rid(xbar_axi_rid[2 * ID_WIDTH_LOC +: ID_WIDTH_LOC]),
        .m_tmu1_axi_rdata(xbar_axi_rdata[2 * DATA_WIDTH +: DATA_WIDTH]),
        .m_tmu1_axi_rresp(xbar_axi_rresp[2 * 2 +: 2]),
        .m_tmu1_axi_rlast(xbar_axi_rlast[2 * 1 +: 1]),
        .m_tmu1_axi_rvalid(xbar_axi_rvalid[2 * 1 +: 1]),
        .m_tmu1_axi_rready(xbar_axi_rready[2 * 1 +: 1])
    );

    // Directly assign a value does not work in verilator.
    // This is an work around for the internal error: verilator Internal Error: ../V3Gate.cpp:1008: Can't replace lvalue assignments with const var
    reg tmpOne = 1;
    reg tmpZero = 0;

    assign xbar_axi_awid[1 * ID_WIDTH_LOC +: ID_WIDTH_LOC] = { ID_WIDTH_LOC { tmpZero } };
    assign xbar_axi_awaddr[1 * ADDR_WIDTH +: ADDR_WIDTH] = { ADDR_WIDTH { tmpZero } };
    assign xbar_axi_awlen[1 * 8 +: 8] = { 8 { tmpZero } };
    assign xbar_axi_awsize[1 * 3 +: 3] = { 3 { tmpZero } };
    assign xbar_axi_awburst[1 * 2 +: 2] = { 2 { tmpZero } };
    assign xbar_axi_awlock[1 * 1 +: 1] = tmpZero;
    assign xbar_axi_awcache[1 * 4 +: 4] = { 4 { tmpZero } };
    assign xbar_axi_awprot[1 * 3 +: 3] = { 3 { tmpZero } };
    assign xbar_axi_awvalid[1 * 1 +: 1] = tmpZero;
    assign xbar_axi_wdata[1 * DATA_WIDTH +: DATA_WIDTH] = { DATA_WIDTH { tmpZero } };
    assign xbar_axi_wstrb[1 * STRB_WIDTH +: STRB_WIDTH] = { STRB_WIDTH { tmpZero } };
    assign xbar_axi_wlast[1 * 1 +: 1] = tmpZero;
    assign xbar_axi_wvalid[1 * 1 +: 1] = tmpZero;
    assign xbar_axi_bready[1 * 1 +: 1] = tmpZero;

    assign xbar_axi_awid[2 * ID_WIDTH_LOC +: ID_WIDTH_LOC] = { ID_WIDTH_LOC { tmpZero } };
    assign xbar_axi_awaddr[2 * ADDR_WIDTH +: ADDR_WIDTH] = { ADDR_WIDTH { tmpZero } };
    assign xbar_axi_awlen[2 * 8 +: 8] = { 8 { tmpZero } };
    assign xbar_axi_awsize[2 * 3 +: 3] = { 3 { tmpZero } };
    assign xbar_axi_awburst[2 * 2 +: 2] = { 2 { tmpZero } };
    assign xbar_axi_awlock[2 * 1 +: 1] = tmpZero;
    assign xbar_axi_awcache[2 * 4 +: 4] = { 4 { tmpZero } };
    assign xbar_axi_awprot[2 * 3 +: 3] = { 3 { tmpZero } };
    assign xbar_axi_awvalid[2 * 1 +: 1] = tmpZero;
    assign xbar_axi_wdata[2 * DATA_WIDTH +: DATA_WIDTH] = { DATA_WIDTH { tmpZero } };
    assign xbar_axi_wstrb[2 * STRB_WIDTH +: STRB_WIDTH] = { STRB_WIDTH { tmpZero } };
    assign xbar_axi_wlast[2 * 1 +: 1] = tmpZero;
    assign xbar_axi_wvalid[2 * 1 +: 1] = tmpZero;
    assign xbar_axi_bready[2 * 1 +: 1] = tmpZero;

endmodule
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

module Rasterix #(
    // The size of the internal framebuffer (in power of two)
    parameter FRAMEBUFFER_SIZE_BYTES = 18,

    // This is the color depth of the framebuffer. Note: This setting has no influence on the framebuffer stream. This steam will
    // stay at RGB565. It changes the internal representation and might be used to reduce the memory footprint.
    // Lower depth will result in color banding.
    parameter FRAMEBUFFER_SUB_PIXEL_WIDTH = 6,
    // This enables the alpha channel of the framebuffer. Requires additional memory.
    parameter FRAMEBUFFER_ENABLE_ALPHA_CHANNEL = 0,
    
    // The bit width of the command stream interface and memory interface
    // Allowed values: 32, 64, 128, 256 bit
    parameter CMD_STREAM_WIDTH = 16,

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

    RasterixRenderCore #(
        .FRAMEBUFFER_SIZE_BYTES(FRAMEBUFFER_SIZE_BYTES),
        .FRAMEBUFFER_SUB_PIXEL_WIDTH(FRAMEBUFFER_SUB_PIXEL_WIDTH),
        .FRAMEBUFFER_ENABLE_ALPHA_CHANNEL(FRAMEBUFFER_ENABLE_ALPHA_CHANNEL),
        .CMD_STREAM_WIDTH(CMD_STREAM_WIDTH),
        .FRAMEBUFFER_STREAM_WIDTH(CMD_STREAM_WIDTH),
        .TEXTURE_BUFFER_SIZE(TEXTURE_BUFFER_SIZE)
    ) graphicCore (
        .aclk(aclk),
        .resetn(resetn),
        
        .s_cmd_axis_tvalid(m_cmd_axis_tvalid),
        .s_cmd_axis_tready(m_cmd_axis_tready),
        .s_cmd_axis_tlast(m_cmd_axis_tlast),
        .s_cmd_axis_tdata(m_cmd_axis_tdata),

        .m_framebuffer_axis_tvalid(s_framebuffer_axis_tvalid),
        .m_framebuffer_axis_tready(s_framebuffer_axis_tready),
        .m_framebuffer_axis_tlast(s_framebuffer_axis_tlast),
        .m_framebuffer_axis_tdata(s_framebuffer_axis_tdata)
    );

endmodule
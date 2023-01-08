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
    parameter CMD_STREAM_WIDTH = 64,
    parameter FRAMEBUFFER_STREAM_WIDTH = 64
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
    output wire [FRAMEBUFFER_STREAM_WIDTH - 1 : 0]  m_framebuffer_axis_tdata
);
    parameter X_RESOLUTION = `X_RESOLUTION;
    parameter Y_RESOLUTION = `Y_RESOLUTION;
    parameter Y_LINE_RESOLUTION = `Y_LINE_RESOLUTION;
    parameter TEXTURE_BUFFER_SIZE = 17;

    wire m_cmd_axis_tvalid;
    wire m_cmd_axis_tready;
    wire m_cmd_axis_tlast;
    wire [CMD_STREAM_WIDTH - 1 : 0] m_cmd_axis_tdata;

    localparam ID_WIDTH = 8;
    localparam ADDR_WIDTH = 24;
    localparam DATA_WIDTH = CMD_STREAM_WIDTH;
    localparam STRB_WIDTH = CMD_STREAM_WIDTH / 8;
    wire [ID_WIDTH-1:0]    s_axi_awid;
    wire [ADDR_WIDTH-1:0]  s_axi_awaddr;
    wire [7:0]             s_axi_awlen;
    wire [2:0]             s_axi_awsize;
    wire [1:0]             s_axi_awburst;
    wire                   s_axi_awlock;
    wire [3:0]             s_axi_awcache;
    wire [2:0]             s_axi_awprot;
    wire                   s_axi_awvalid;
    wire                   s_axi_awready;
    wire [DATA_WIDTH-1:0]  s_axi_wdata;
    wire [STRB_WIDTH-1:0]  s_axi_wstrb;
    wire                   s_axi_wlast;
    wire                   s_axi_wvalid;
    wire                   s_axi_wready;
    wire [ID_WIDTH-1:0]    s_axi_bid;
    wire [1:0]             s_axi_bresp;
    wire                   s_axi_bvalid;
    wire                   s_axi_bready;
    wire [ID_WIDTH-1:0]    s_axi_arid;
    wire [ADDR_WIDTH-1:0]  s_axi_araddr;
    wire [7:0]             s_axi_arlen;
    wire [2:0]             s_axi_arsize;
    wire [1:0]             s_axi_arburst;
    wire                   s_axi_arlock;
    wire [3:0]             s_axi_arcache;
    wire [2:0]             s_axi_arprot;
    wire                   s_axi_arvalid;
    wire                   s_axi_arready;
    wire [ID_WIDTH-1:0]    s_axi_rid;
    wire [DATA_WIDTH-1:0]  s_axi_rdata;
    wire [1:0]             s_axi_rresp;
    wire                   s_axi_rlast;
    wire                   s_axi_rvalid;
    wire                   s_axi_rready;

    axi_ram #(
        .DATA_WIDTH(CMD_STREAM_WIDTH),
        .ADDR_WIDTH(ADDR_WIDTH)
    ) ram (
        .rst(!resetn),
        .clk(aclk),

        .s_axi_awid(s_axi_awid),
        .s_axi_awaddr(s_axi_awaddr),
        .s_axi_awlen(s_axi_awlen),
        .s_axi_awsize(s_axi_awsize),
        .s_axi_awburst(s_axi_awburst),
        .s_axi_awlock(s_axi_awlock),
        .s_axi_awcache(s_axi_awcache),
        .s_axi_awprot(s_axi_awprot),
        .s_axi_awvalid(s_axi_awvalid),
        .s_axi_awready(s_axi_awready),
        .s_axi_wdata(s_axi_wdata),
        .s_axi_wstrb(s_axi_wstrb),
        .s_axi_wlast(s_axi_wlast),
        .s_axi_wvalid(s_axi_wvalid),
        .s_axi_wready(s_axi_wready),
        .s_axi_bid(s_axi_bid),
        .s_axi_bresp(s_axi_bresp),
        .s_axi_bvalid(s_axi_bvalid),
        .s_axi_bready(s_axi_bready),
        .s_axi_arid(s_axi_arid),
        .s_axi_araddr(s_axi_araddr),
        .s_axi_arlen(s_axi_arlen),
        .s_axi_arsize(s_axi_arsize),
        .s_axi_arburst(s_axi_arburst),
        .s_axi_arlock(s_axi_arlock),
        .s_axi_arcache(s_axi_arcache),
        .s_axi_arprot(s_axi_arprot),
        .s_axi_arvalid(s_axi_arvalid),
        .s_axi_arready(s_axi_arready),
        .s_axi_rid(s_axi_rid),
        .s_axi_rdata(s_axi_rdata),
        .s_axi_rresp(s_axi_rresp),
        .s_axi_rlast(s_axi_rlast),
        .s_axi_rvalid(s_axi_rvalid),
        .s_axi_rready(s_axi_rready)
    );

    DmaStreamEngine #(
        .STREAM_WIDTH(CMD_STREAM_WIDTH),
        .ADDR_WIDTH(ADDR_WIDTH)
    ) parser (
        .aclk(aclk),
        .resetn(resetn),

        .m_cmd_axis_tvalid(m_cmd_axis_tvalid),
        .m_cmd_axis_tready(m_cmd_axis_tready),
        .m_cmd_axis_tlast(m_cmd_axis_tlast),
        .m_cmd_axis_tdata(m_cmd_axis_tdata),
        
        .s_cmd_axis_tvalid(s_cmd_axis_tvalid),
        .s_cmd_axis_tready(s_cmd_axis_tready),
        .s_cmd_axis_tlast(s_cmd_axis_tlast),
        .s_cmd_axis_tdata(s_cmd_axis_tdata),

        .m_mem_axi_awid(s_axi_awid),
        .m_mem_axi_awaddr(s_axi_awaddr),
        .m_mem_axi_awlen(s_axi_awlen), 
        .m_mem_axi_awsize(s_axi_awsize), 
        .m_mem_axi_awburst(s_axi_awburst), 
        .m_mem_axi_awlock(s_axi_awlock), 
        .m_mem_axi_awcache(s_axi_awcache), 
        .m_mem_axi_awprot(s_axi_awprot), 
        .m_mem_axi_awvalid(s_axi_awvalid),
        .m_mem_axi_awready(s_axi_awready),

        .m_mem_axi_wdata(s_axi_wdata),
        .m_mem_axi_wstrb(s_axi_wstrb),
        .m_mem_axi_wlast(s_axi_wlast),
        .m_mem_axi_wvalid(s_axi_wvalid),
        .m_mem_axi_wready(s_axi_wready),

        .m_mem_axi_bid(s_axi_bid),
        .m_mem_axi_bresp(s_axi_bresp),
        .m_mem_axi_bvalid(s_axi_bvalid),
        .m_mem_axi_bready(s_axi_bready),

        .m_mem_axi_arid(s_axi_arid),
        .m_mem_axi_araddr(s_axi_araddr),
        .m_mem_axi_arlen(s_axi_arlen),
        .m_mem_axi_arsize(s_axi_arsize),
        .m_mem_axi_arburst(s_axi_arburst),
        .m_mem_axi_arlock(s_axi_arlock),
        .m_mem_axi_arcache(s_axi_arcache),
        .m_mem_axi_arprot(s_axi_arprot),
        .m_mem_axi_arvalid(s_axi_arvalid),
        .m_mem_axi_arready(s_axi_arready),

        .m_mem_axi_rid(s_axi_rid),
        .m_mem_axi_rdata(s_axi_rdata),
        .m_mem_axi_rresp(s_axi_rresp),
        .m_mem_axi_rlast(s_axi_rlast),
        .m_mem_axi_rvalid(s_axi_rvalid),
        .m_mem_axi_rready(s_axi_rready)
    );

    Rasterix #(.X_RESOLUTION(X_RESOLUTION),
                 .Y_RESOLUTION(Y_RESOLUTION),
                 .Y_LINE_RESOLUTION(Y_LINE_RESOLUTION),
                 .CMD_STREAM_WIDTH(CMD_STREAM_WIDTH),
                 .FRAMEBUFFER_STREAM_WIDTH(FRAMEBUFFER_STREAM_WIDTH),
                 .TEXTURE_BUFFER_SIZE(TEXTURE_BUFFER_SIZE)) rasteriCEr(
        .aclk(aclk),
        .resetn(resetn),
        
        .s_cmd_axis_tvalid(m_cmd_axis_tvalid),
        .s_cmd_axis_tready(m_cmd_axis_tready),
        .s_cmd_axis_tlast(m_cmd_axis_tlast),
        .s_cmd_axis_tdata(m_cmd_axis_tdata),

        .m_framebuffer_axis_tvalid(m_framebuffer_axis_tvalid),
        .m_framebuffer_axis_tready(m_framebuffer_axis_tready),
        .m_framebuffer_axis_tlast(m_framebuffer_axis_tlast),
        .m_framebuffer_axis_tdata(m_framebuffer_axis_tdata),

        // Debug
        .dbgStreamState(),
        .dbgRasterizerRunning()
    );
endmodule
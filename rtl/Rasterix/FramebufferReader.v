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

module FramebufferReader #(
    // Width of the axi interfaces
    parameter STREAM_WIDTH = 32,
    // Width of address bus in bits
    parameter ADDR_WIDTH = 32,
    // Width of wstrb (width of data bus in words)
    parameter STRB_WIDTH = (STREAM_WIDTH / 8),
    // Width of ID signal
    parameter ID_WIDTH = 8,

    // Size of the pixels
    localparam PIXEL_WIDTH = 16

) (
    input  wire                             aclk,
    input  wire                             resetn,

    /////////////////////////
    // Configs
    /////////////////////////
    input  wire [ADDR_WIDTH - 1 : 0]        confAddr,

    /////////////////////////
    // Fragment Interface
    /////////////////////////

    // Fetch interface
    input  wire                             s_fetch_axis_tvalid,
    input  wire                             s_fetch_axis_tlast,
    output wire                             s_fetch_axis_tready,
    input  wire [ADDR_WIDTH - 1 : 0]        s_fetch_axis_tdest,
    input  wire                             s_fetch_axis_tkeep,

    // Framebuffer Interface
    output wire                             m_frag_axis_tvalid,
    input  wire                             m_frag_axis_tready,
    output wire [PIXEL_WIDTH - 1 : 0]       m_frag_axis_tdata,
    output wire [ADDR_WIDTH - 1 : 0]        m_frag_axis_tdest,
    output wire                             m_frag_axis_tkeep,
    output wire                             m_frag_axis_tlast,

    /////////////////////////
    // Memory Interface
    /////////////////////////

    // Address channel
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

    // Data channel
    input  wire [ID_WIDTH - 1 : 0]          m_mem_axi_rid,
    input  wire [STREAM_WIDTH - 1 : 0]      m_mem_axi_rdata,
    input  wire [ 1 : 0]                    m_mem_axi_rresp,
    input  wire                             m_mem_axi_rlast,
    input  wire                             m_mem_axi_rvalid,
    output wire                             m_mem_axi_rready
);
    localparam FETCH_FIFO_LEN = $clog2(128);

    wire [ 1 : 0]                   bc_fetch_tvalid;
    wire [ 1 : 0]                   bc_fetch_tlast;
    wire [ 1 : 0]                   bc_fetch_tready;
    wire [(ADDR_WIDTH * 2) - 1 : 0] bc_fetch_tdest;
    wire [ 1 : 0]                   bc_fetch_tkeep;

    wire                            fifo_fetch_tvalid;
    wire                            fifo_fetch_tlast;
    wire                            fifo_fetch_tready;
    wire [ADDR_WIDTH - 1 : 0]       fifo_fetch_tdest;
    wire                            fifo_fetch_tkeep;
    wire                            fifo_fetch_tempty;
    wire [ADDR_WIDTH + 2 - 1 : 0]   fifo_fetch_tdata;
    wire                            fifo_full;
    assign fifo_fetch_tvalid = !fifo_fetch_tempty;
    assign fifo_fetch_tlast = fifo_fetch_tdata[ADDR_WIDTH + 1 +: 1];
    assign fifo_fetch_tkeep = fifo_fetch_tdata[ADDR_WIDTH +: 1];
    assign fifo_fetch_tdest = fifo_fetch_tdata[0 +: ADDR_WIDTH];

    assign bc_fetch_tready[1] = !fifo_full; 

    axis_broadcast #(
        .M_COUNT(2),
        .DATA_WIDTH(ADDR_WIDTH),
        .KEEP_ENABLE(1),
        .KEEP_WIDTH(1),
        .LAST_ENABLE(1),
        .ID_ENABLE(0),
        .DEST_ENABLE(0),
        .USER_ENABLE(0)
    ) axisBroadcast (
        .clk(aclk),
        .rst(!resetn),

        .s_axis_tdata(s_fetch_axis_tdest),
        .s_axis_tlast(s_fetch_axis_tlast),
        .s_axis_tvalid(s_fetch_axis_tvalid),
        .s_axis_tready(s_fetch_axis_tready),
        .s_axis_tkeep(s_fetch_axis_tkeep),
        .s_axis_tid(),
        .s_axis_tdest(),
        .s_axis_tuser(),

        .m_axis_tdata(bc_fetch_tdest),
        .m_axis_tvalid(bc_fetch_tvalid),
        .m_axis_tready(bc_fetch_tready),
        .m_axis_tlast(bc_fetch_tlast),
        .m_axis_tkeep(bc_fetch_tkeep),
        .m_axis_tid(),
        .m_axis_tdest(),
        .m_axis_tuser()
    );

    sfifo #(
        .BW(ADDR_WIDTH + 1 + 1),
        .LGFLEN(FETCH_FIFO_LEN)
    ) fetchAddrFifo (
        .i_clk(aclk),
        .i_reset(!resetn),

        .i_wr(bc_fetch_tvalid[1]),
        .i_data({ bc_fetch_tlast[1], bc_fetch_tkeep[1], bc_fetch_tdest[ADDR_WIDTH +: ADDR_WIDTH] }),
        .o_full(fifo_full),
        .o_fill(),

        .i_rd(fifo_fetch_tready),
        .o_data(fifo_fetch_tdata),
        .o_empty(fifo_fetch_tempty)
    );

    MemoryReadRequestGenerator #(
        .STREAM_WIDTH(STREAM_WIDTH), 
        .ADDR_WIDTH(ADDR_WIDTH), 
        .ID_WIDTH(ID_WIDTH), 
        .PIXEL_WIDTH(PIXEL_WIDTH)
    ) mrrg (
        .aclk(aclk),
        .resetn(resetn),

        .confAddr(confAddr),
        
        .s_fetch_axis_tvalid(bc_fetch_tvalid[0]),
        .s_fetch_axis_tlast(bc_fetch_tlast[0]),
        .s_fetch_axis_tready(bc_fetch_tready[0]),
        .s_fetch_axis_tdest(bc_fetch_tdest[0 +: ADDR_WIDTH]),

        .m_mem_axi_arid(m_mem_axi_arid),
        .m_mem_axi_araddr(m_mem_axi_araddr),
        .m_mem_axi_arlen(m_mem_axi_arlen),
        .m_mem_axi_arsize(m_mem_axi_arsize),
        .m_mem_axi_arburst(m_mem_axi_arburst),
        .m_mem_axi_arlock(m_mem_axi_arlock),
        .m_mem_axi_arcache(m_mem_axi_arcache),
        .m_mem_axi_arprot(m_mem_axi_arprot),
        .m_mem_axi_arvalid(m_mem_axi_arvalid),
        .m_mem_axi_arready(m_mem_axi_arready)
    );

    FramebufferSerializer #(
        .STREAM_WIDTH(STREAM_WIDTH),
        .ADDR_WIDTH(ADDR_WIDTH),
        .ID_WIDTH(ID_WIDTH),
        .PIXEL_WIDTH(PIXEL_WIDTH),
        .KEEP_WIDTH(1)
    ) fbs (
        .aclk(aclk),
        .resetn(resetn),

        .m_frag_axis_tvalid(m_frag_axis_tvalid),
        .m_frag_axis_tready(m_frag_axis_tready),
        .m_frag_axis_tkeep(m_frag_axis_tkeep),
        .m_frag_axis_tdata(m_frag_axis_tdata),
        .m_frag_axis_tdest(m_frag_axis_tdest),
        .m_frag_axis_tlast(m_frag_axis_tlast),

        .s_fetch_axis_tvalid(fifo_fetch_tvalid),
        .s_fetch_axis_tlast(fifo_fetch_tlast),
        .s_fetch_axis_tready(fifo_fetch_tready),
        .s_fetch_axis_tdest(fifo_fetch_tdest),
        .s_fetch_axis_tkeep(fifo_fetch_tkeep),

        .m_mem_axi_rid(m_mem_axi_rid),
        .m_mem_axi_rdata(m_mem_axi_rdata),
        .m_mem_axi_rresp(m_mem_axi_rresp),
        .m_mem_axi_rlast(m_mem_axi_rlast),
        .m_mem_axi_rvalid(m_mem_axi_rvalid),
        .m_mem_axi_rready(m_mem_axi_rready)
    );

endmodule
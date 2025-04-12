// RasterIX
// https://github.com/ToNi3141/RasterIX
// Copyright (c) 2025 ToNi3141

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

// Receives a stream from a framebuffer and writes it into the memory
module AxisFramebufferWriter #(
    // Width of the axi interfaces
    parameter DATA_WIDTH = 32,
    // Width of address bus in bits
    parameter ADDR_WIDTH = 32,
    // Width of wstrb (width of data bus in words)
    parameter STRB_WIDTH = 4,
    // Width of ID signal
    parameter ID_WIDTH = 8,

    localparam FB_SIZE_IN_PIXEL_LG = 20
) (
    input  wire                                 aclk,
    input  wire                                 resetn,

    input  wire                                 commit_fb,
    input  wire [ADDR_WIDTH - 1 : 0]            fb_addr,
    input  wire [FB_SIZE_IN_PIXEL_LG - 1 : 0]   fb_size,
    output reg                                  fb_committed,

    // Display port
    input  wire                                 s_disp_axis_tvalid,
    output wire                                 s_disp_axis_tready,
    input  wire                                 s_disp_axis_tlast,
    input  wire [DATA_WIDTH - 1 : 0]            s_disp_axis_tdata,

    // Memory port
    output wire [ID_WIDTH - 1 : 0]              m_mem_axi_awid,
    output wire [ADDR_WIDTH - 1 : 0]            m_mem_axi_awaddr,
    output wire [ 7 : 0]                        m_mem_axi_awlen,
    output wire [ 2 : 0]                        m_mem_axi_awsize,
    output wire [ 1 : 0]                        m_mem_axi_awburst,
    output wire                                 m_mem_axi_awlock,
    output wire [ 3 : 0]                        m_mem_axi_awcache,
    output wire [ 2 : 0]                        m_mem_axi_awprot, 
    output wire                                 m_mem_axi_awvalid,
    input  wire                                 m_mem_axi_awready,

    output wire [DATA_WIDTH - 1 : 0]            m_mem_axi_wdata,
    output wire [(STRB_WIDTH - 1) : 0]          m_mem_axi_wstrb,
    output wire                                 m_mem_axi_wlast,
    output wire                                 m_mem_axi_wvalid,
    input  wire                                 m_mem_axi_wready,

    input  wire [ID_WIDTH - 1 : 0]              m_mem_axi_bid,
    input  wire [ 1 : 0]                        m_mem_axi_bresp,
    input  wire                                 m_mem_axi_bvalid,
    output wire                                 m_mem_axi_bready
);
    reg                         st0_axis_tvalid;
    wire                        st0_axis_tready;
    reg                         st0_axis_tlast;
    reg [DATA_WIDTH - 1 : 0]    st0_axis_tdata;

    localparam STATE_CMD = 0;
    localparam STATE_ADDR = 1;
    localparam STATE_WAIT_DSE = 2;
    reg [ 1 : 0] state;

    DmaStreamEngine #(
        .STREAM_WIDTH(DATA_WIDTH),
        .ADDR_WIDTH(ADDR_WIDTH),
        .STRB_WIDTH(STRB_WIDTH),
        .ID_WIDTH(ID_WIDTH)
    ) dse (
        .aclk(aclk),
        .resetn(resetn),
        
        .s_st1_axis_tvalid(s_disp_axis_tvalid),
        .s_st1_axis_tready(s_disp_axis_tready),
        .s_st1_axis_tlast(s_disp_axis_tlast),
        .s_st1_axis_tdata(s_disp_axis_tdata),

        .s_st0_axis_tvalid(st0_axis_tvalid),
        .s_st0_axis_tready(st0_axis_tready),
        .s_st0_axis_tlast(st0_axis_tlast),
        .s_st0_axis_tdata(st0_axis_tdata),

        .m_st1_axis_tvalid(),
        .m_st1_axis_tready(0),
        .m_st1_axis_tlast(),
        .m_st1_axis_tdata(),

        .m_st0_axis_tvalid(),
        .m_st0_axis_tready(0),
        .m_st0_axis_tlast(),
        .m_st0_axis_tdata(),

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

        .m_mem_axi_arid(),
        .m_mem_axi_araddr(),
        .m_mem_axi_arlen(),
        .m_mem_axi_arsize(),
        .m_mem_axi_arburst(),
        .m_mem_axi_arlock(),
        .m_mem_axi_arcache(),
        .m_mem_axi_arprot(),
        .m_mem_axi_arvalid(),
        .m_mem_axi_arready(0),

        .m_mem_axi_rid(0),
        .m_mem_axi_rdata(0),
        .m_mem_axi_rresp(0),
        .m_mem_axi_rlast(0),
        .m_mem_axi_rvalid(0),
        .m_mem_axi_rready()
    );

    always @(posedge aclk)
    begin
        if (!resetn)
        begin
            state <= STATE_CMD;
            fb_committed <= 1;
            st0_axis_tvalid <= 0;
            st0_axis_tlast <= 0;
        end
        else
        begin
            case (state)
                STATE_CMD:
                begin
                    if (commit_fb)
                    begin
                        st0_axis_tdata <= { 2'h3, 2'h2, 7'h0, fb_size, 1'h0 }; // fb_size is the size of the FB in 16 bit pixel
                        st0_axis_tvalid <= 1;
                        st0_axis_tlast <= 0;
                        fb_committed <= 0;
                        state <= STATE_ADDR;
                    end
                end
                STATE_ADDR: 
                begin
                    if (st0_axis_tready)
                    begin
                        st0_axis_tdata <= fb_addr;
                        st0_axis_tlast <= 1;
                        fb_committed <= 1; // Early acknowledge to the framebuffer
                        state <= STATE_WAIT_DSE;
                    end
                end
                STATE_WAIT_DSE:
                begin
                    if (st0_axis_tready)
                    begin
                        st0_axis_tvalid <= 0;
                        st0_axis_tlast <= 0;
                        state <= STATE_CMD;
                    end
                end
                default: 
                begin
                end
            endcase
        end
    end
endmodule
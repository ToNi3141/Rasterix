// RasterIX
// https://github.com/ToNi3141/RasterIX
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

// Async SRAM controller for 10ns 8 bit SRAMs.
// It reaches ~40MB/s with random access patterns, and ~50MB/s with bursts.
// aclk requires a 100MHz clock. Lower frequencies are possible, but will
// lower the throughput.
// Cache signals are not supported.
// Only INCR bursts are supported.
// Only axsize with $log2(DATA_WIDTH / 2) are supported
module AsyncSramController
#(
    parameter DATA_WIDTH = 32,
    parameter ADDR_WIDTH = 19,
    parameter STRB_WIDTH = ADDR_WIDTH / 4,
    parameter ID_WIDTH = 10,
    parameter MEM_WIDTH = 8
)
(
    input  wire                         aclk,
    input  wire                         resetn,

    output reg  [ADDR_WIDTH - 1 : 0]    mem_addr,
    input  wire [MEM_WIDTH - 1 : 0]     mem_data_i,
    output reg  [MEM_WIDTH - 1 : 0]     mem_data_o,
    output reg  [MEM_WIDTH - 1 : 0]     mem_data_t,
    output reg                          mem_noe,
    output reg                          mem_nce,
    output reg                          mem_nwe,

    input  wire [ID_WIDTH - 1 : 0]      s_axi_awid,
    input  wire [ADDR_WIDTH - 1 : 0]    s_axi_awaddr,
    input  wire [ 7 : 0]                s_axi_awlen,
    input  wire [ 2 : 0]                s_axi_awsize,
    input  wire [ 1 : 0]                s_axi_awburst,
    input  wire                         s_axi_awlock,
    input  wire [ 3 : 0]                s_axi_awcache,
    input  wire [ 2 : 0]                s_axi_awprot,
    input  wire                         s_axi_awvalid,
    output reg                          s_axi_awready,

    input  wire [DATA_WIDTH - 1 : 0]    s_axi_wdata,
    input  wire [STRB_WIDTH - 1 : 0]    s_axi_wstrb,
    input  wire                         s_axi_wlast,
    input  wire                         s_axi_wvalid,
    output reg                          s_axi_wready,

    output reg  [ID_WIDTH - 1 : 0]      s_axi_bid,
    output reg  [ 1 : 0]                s_axi_bresp,
    output reg                          s_axi_bvalid,
    input  wire                         s_axi_bready,
    input  wire [ID_WIDTH - 1 : 0]      s_axi_arid,

    input  wire [ADDR_WIDTH - 1 : 0]    s_axi_araddr,
    input  wire [ 7 : 0]                s_axi_arlen,
    input  wire [ 2 : 0]                s_axi_arsize,
    input  wire [ 1 : 0]                s_axi_arburst,
    input  wire                         s_axi_arlock,
    input  wire [ 3 : 0]                s_axi_arcache,
    input  wire [ 2 : 0]                s_axi_arprot,
    input  wire                         s_axi_arvalid,
    output reg                          s_axi_arready,

    output reg  [ID_WIDTH - 1 : 0]      s_axi_rid,
    output reg  [DATA_WIDTH - 1 : 0]    s_axi_rdata,
    output reg  [ 1 : 0]                s_axi_rresp,
    output reg                          s_axi_rlast,
    output reg                          s_axi_rvalid,
    input  wire                         s_axi_rready
);
    localparam WIT = DATA_WIDTH / MEM_WIDTH;
    localparam WITM = WIT - 1;

    localparam IDLE = 0;
    localparam WRITE_ADDR = 1;
    localparam WRITE_DATA = 2;
    localparam READ_ADDR = 3;
    localparam READ_DATA = 4;
    localparam FINISH = 5;

    reg [ 2 : 0]                state;
    reg [$clog2(WIT) + 7 : 0]   axlen;
    reg [ID_WIDTH - 1 : 0]      axid;
    reg [ 2 : 0]                axsize;
    reg [10 : 0]                i;
    reg [ADDR_WIDTH - 1 : 0]    axaddr;

    always @(posedge aclk)
    begin
        if (!resetn)
        begin
            s_axi_rvalid <= 0;
            s_axi_arready <= 0;

            s_axi_wready <= 0;
            s_axi_awready <= 0;

            s_axi_bvalid <= 0;
            s_axi_bresp <= 0;

            s_axi_rresp <= 0;

            mem_noe <= 1;
            mem_nce <= 1;
            mem_nwe <= 1;

            i <= 0;

            state <= IDLE;
        end
        else
        begin
            if (s_axi_bvalid && s_axi_bready)
            begin
                s_axi_bvalid <= 0;
            end
            case (state)
                IDLE:
                begin
                    if (s_axi_awvalid)
                    begin
                        i <= 0;

                        axid <= s_axi_awid;

                        mem_noe <= 1;
                        mem_nce <= 0;
                        axaddr <= s_axi_awaddr;

                        axlen <= { s_axi_awlen, { { $clog2(WIT) { 1'b1 } } } };

                        s_axi_awready <= 1;

                        state <= WRITE_ADDR;
                    end
                    else if (s_axi_arvalid)
                    begin
                        i <= 0;

                        s_axi_rid <= s_axi_arid;

                        mem_noe <= 0;
                        mem_nce <= 0;
                        mem_data_t <= ~0;
                        mem_addr <= s_axi_araddr;
                        axaddr <= s_axi_araddr;
                        
                        axlen <= { s_axi_arlen, { { $clog2(WIT) { 1'b1 } } } };

                        s_axi_arready <= 1;

                        state <= READ_ADDR;
                    end
                end
                WRITE_ADDR:
                begin
                    s_axi_awready <= 0;
                    if (s_axi_wvalid)
                    begin
                        mem_addr <= axaddr + i;
                        mem_data_o <= s_axi_wdata[(i & WITM) * MEM_WIDTH +: MEM_WIDTH];
                        mem_data_t <= 0;
                        mem_nwe <= !s_axi_wstrb[i & WITM];
                        s_axi_wready <= (i & WITM) == WITM;
                        state <= WRITE_DATA;
                    end
                end
                WRITE_DATA:
                begin
                    s_axi_wready <= 0;
                    mem_nwe <= 1;
                    if (i == axlen)
                    begin
                        s_axi_bvalid <= 1;
                        s_axi_bid <= axid;
                        state <= FINISH;
                    end
                    else
                    begin
                        i <= i + 1;
                        state <= WRITE_ADDR;
                    end
                end
                READ_ADDR:
                begin
                    s_axi_rvalid <= 0;
                    s_axi_arready <= 0;
                    
                    state <= READ_DATA;
                end
                READ_DATA:
                begin : ReadData
                    reg valid;
                    valid = (i & WITM) == WITM;
                    s_axi_rvalid <= valid;
                    s_axi_rdata[(i & WITM) * MEM_WIDTH +: MEM_WIDTH] <= mem_data_i;
                    s_axi_rlast <= (i == axlen);
                    if (s_axi_rready || !valid)
                    begin
                        if (i == axlen)
                        begin
                            state <= FINISH;
                        end
                        else
                        begin
                            i <= i + 1;
                            mem_addr <= axaddr + i + 1;
                            state <= READ_ADDR;
                        end
                        if (s_axi_rvalid)
                        begin
                            s_axi_rvalid <= 0;
                        end
                    end
                end
                FINISH:
                begin
                    s_axi_rvalid <= 0;
                    mem_noe <= 1;
                    mem_nce <= 1;
                    mem_nwe <= 1;
                    mem_data_t <= ~0;
                    state <= IDLE;
                end
                default:
                begin
                end
            endcase
        end
    end
endmodule
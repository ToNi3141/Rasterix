// RasteriCEr
// https://github.com/ToNi3141/RasteriCEr
// Copyright (c) 2021 ToNi3141

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

module PreCommandParser #(
    parameter STREAM_WIDTH = 32,
    // Width of address bus in bits
    parameter ADDR_WIDTH = 16,
    // Width of wstrb (width of data bus in words)
    parameter STRB_WIDTH = (STREAM_WIDTH / 8),
    // Width of ID signal
    parameter ID_WIDTH = 8
) (
    input  wire                         aclk,
    input  wire                         resetn,

    // Stream interface
    output reg                          m_axis_tvalid,
    input  wire                         m_axis_tready,
    output reg                          m_axis_tlast,
    output reg  [STREAM_WIDTH - 1 : 0]  m_axis_tdata,

    input  wire                         s_axis_tvalid,
    output reg                          s_axis_tready,
    input  wire                         s_axis_tlast,
    input  wire [STREAM_WIDTH - 1 : 0]  s_axis_tdata,

    // Memory interface
    output reg  [ID_WIDTH - 1 : 0]      m_axi_awid,
    output reg  [ADDR_WIDTH - 1 : 0]    m_axi_awaddr,
    output reg  [ 7 : 0]                m_axi_awlen, // How many cycles are in this transaction
    output reg  [ 2 : 0]                m_axi_awsize, // The increment during one cycle. Means, 0 incs addr by 1, 2 by 4 and so on
    output reg  [ 1 : 0]                m_axi_awburst, // 0 fixed, 1 incr, 2 wrappig
    output reg                          m_axi_awlock, //
    output reg  [ 3 : 0]                m_axi_awcache, //
    output reg  [ 2 : 0]                m_axi_awprot, //
    output reg                          m_axi_awvalid,
    input  wire                         m_axi_awready,

    output reg  [STREAM_WIDTH - 1 : 0]  m_axi_wdata,
    output reg  [STRB_WIDTH - 1 : 0]    m_axi_wstrb,
    output reg                          m_axi_wlast,
    output reg                          m_axi_wvalid,
    input  wire                         m_axi_wready,

    input  wire [ID_WIDTH - 1 : 0]      m_axi_bid,
    input  wire [ 1 : 0]                m_axi_bresp,
    input  wire                         m_axi_bvalid,
    output reg                          m_axi_bready,

    output reg  [ID_WIDTH - 1 : 0]      m_axi_arid,
    output reg  [ADDR_WIDTH - 1 : 0]    m_axi_araddr,
    output reg  [ 7 : 0]                m_axi_arlen,
    output reg  [ 2 : 0]                m_axi_arsize,
    output reg  [ 1 : 0]                m_axi_arburst,
    output reg                          m_axi_arlock,
    output reg  [ 3 : 0]                m_axi_arcache,
    output reg  [ 2 : 0]                m_axi_arprot,
    output reg                          m_axi_arvalid,
    input  wire                         m_axi_arready,

    input  wire [ID_WIDTH - 1 : 0]      m_axi_rid,
    input  wire [STREAM_WIDTH - 1 : 0]  m_axi_rdata,
    input  wire [ 1 : 0]                m_axi_rresp,
    input  wire                         m_axi_rlast,
    input  wire                         m_axi_rvalid,
    output reg                          m_axi_rready
);
    localparam BYTES_TO_BEATS_SHIFT = $clog2(STREAM_WIDTH / 8);

    localparam OP_POS = 28;
    localparam OP_SIZE = 4;
    localparam OP_IMM_POS = 0;
    localparam OP_IMM_SIZE = 28;

    localparam OP_NOP = 0;
    localparam OP_STORE = 1;
    localparam OP_LOAD = 2;
    localparam OP_MEMSET = 3;
    localparam OP_STREAM = 4;

    localparam IDLE = 0;
    localparam COMMAND = 1;
    localparam STORE = 2;
    localparam LOAD = 3;
    localparam MEMSET = 4;
    localparam STREAM = 5;

    reg [ 5 : 0] state;
    reg [OP_IMM_SIZE - 1 : 0] counter;

    reg         streamLast;
    reg [ STREAM_WIDTH - 1 : 0] streamLastTData;
    reg         streamLastTValid;
    reg         first;

    always @(posedge aclk)
    begin
        if (resetn == 0)
        begin
            m_axis_tvalid <= 0;
            m_axis_tlast <= 0;
            s_axis_tready <= 0;

            state <= IDLE;
        end
        else 
        begin
            case (state)
            IDLE:
            begin
                m_axis_tlast <= 0;
                m_axis_tvalid <= 0;

                s_axis_tready <= 1;
                state <= COMMAND;
            end
            COMMAND:
            begin
                if (s_axis_tvalid)
                begin
                    case (s_axis_tdata[OP_POS +: OP_SIZE])
                    OP_NOP:
                    begin
                        s_axis_tready <= 0;
                        state <= IDLE;
                    end
                    OP_STORE:
                    begin
                        
                    end
                    OP_LOAD:
                    begin
                        
                    end
                    OP_MEMSET:
                    begin
                        
                    end
                    OP_STREAM:
                    begin
                        first <= 1;
                        counter <= s_axis_tdata[OP_IMM_POS +: OP_IMM_SIZE] >> BYTES_TO_BEATS_SHIFT;
                        state <= STREAM;
                    end
                    endcase
                end
            end
            STORE:
            begin

            end
            LOAD:
            begin
                
            end
            MEMSET:
            begin
                
            end
            STREAM:
            begin
                // Write the next beat when the destination is ready
                // if the destination is not ready, preload one entry till the output is valid
                if (m_axis_tready || !m_axis_tvalid)
                begin
                    // Copy data from the source to the destination
                    m_axis_tvalid <= s_axis_tvalid;
                    m_axis_tdata <= s_axis_tdata;  
                    m_axis_tlast <= counter == 1;

                    // If the counter reaches 1, we don't want to fetch new data, except
                    // we have no valid data, then we try to fetch till we have valid data.
                    if ((counter == 1) && s_axis_tvalid)
                    begin
                        s_axis_tready <= 0;
                    end
                    else 
                    begin
                        // As long as the destination is ready or we have no preloaded value
                        // tell the source that we can read data
                        s_axis_tready <= (m_axis_tready || !s_axis_tvalid);
                    end

                    // Valid data signals what we have fetched one beat and decrement the counter
                    if (s_axis_tvalid)
                    begin
                        counter <= counter - 1;
                    end
                end

                // If the counter is zero and we have transfered the last beat, then we can 
                // go back to idle
                if ((counter == 0) && m_axis_tready)
                begin
                    m_axis_tvalid <= 0;
                    s_axis_tready <= 0;
                    m_axis_tlast <= 0;
                    state <= IDLE;
                end  
            end
            endcase 
        end
    end

endmodule
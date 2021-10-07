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
    output wire                         m_cmd_axis_tvalid,
    input  wire                         m_cmd_axis_tready,
    output wire                         m_cmd_axis_tlast,
    output wire [STREAM_WIDTH - 1 : 0]  m_cmd_axis_tdata,

    input  wire                         s_cmd_axis_tvalid,
    output wire                         s_cmd_axis_tready,
    input  wire                         s_cmd_axis_tlast,
    input  wire [STREAM_WIDTH - 1 : 0]  s_cmd_axis_tdata,

    // Memory interface
    output reg  [ID_WIDTH - 1 : 0]      m_mem_axi_awid,
    output reg  [ADDR_WIDTH - 1 : 0]    m_mem_axi_awaddr,
    output reg  [ 7 : 0]                m_mem_axi_awlen, // How many cycles are in this transaction
    output reg  [ 2 : 0]                m_mem_axi_awsize, // The increment during one cycle. Means, 0 incs addr by 1, 2 by 4 and so on
    output reg  [ 1 : 0]                m_mem_axi_awburst, // 0 fixed, 1 incr, 2 wrappig
    output reg                          m_mem_axi_awlock, //
    output reg  [ 3 : 0]                m_mem_axi_awcache, //
    output reg  [ 2 : 0]                m_mem_axi_awprot, //
    output reg                          m_mem_axi_awvalid,
    input  wire                         m_mem_axi_awready,

    output wire [STREAM_WIDTH - 1 : 0]  m_mem_axi_wdata,
    output reg  [STRB_WIDTH - 1 : 0]    m_mem_axi_wstrb,
    output wire                         m_mem_axi_wlast,
    output wire                         m_mem_axi_wvalid,
    input  wire                         m_mem_axi_wready,

    input  wire [ID_WIDTH - 1 : 0]      m_mem_axi_bid,
    input  wire [ 1 : 0]                m_mem_axi_bresp,
    input  wire                         m_mem_axi_bvalid,
    output reg                          m_mem_axi_bready,

    output reg  [ID_WIDTH - 1 : 0]      m_mem_axi_arid,
    output reg  [ADDR_WIDTH - 1 : 0]    m_mem_axi_araddr,
    output reg  [ 7 : 0]                m_mem_axi_arlen,
    output reg  [ 2 : 0]                m_mem_axi_arsize,
    output reg  [ 1 : 0]                m_mem_axi_arburst,
    output reg                          m_mem_axi_arlock,
    output reg  [ 3 : 0]                m_mem_axi_arcache,
    output reg  [ 2 : 0]                m_mem_axi_arprot,
    output reg                          m_mem_axi_arvalid,
    input  wire                         m_mem_axi_arready,

    input  wire [ID_WIDTH - 1 : 0]      m_mem_axi_rid,
    input  wire [STREAM_WIDTH - 1 : 0]  m_mem_axi_rdata,
    input  wire [ 1 : 0]                m_mem_axi_rresp,
    input  wire                         m_mem_axi_rlast,
    input  wire                         m_mem_axi_rvalid,
    output wire                         m_mem_axi_rready
);
    localparam BYTES_PER_BEAT = STREAM_WIDTH / 8;
    localparam BYTES_TO_BEATS_SHIFT = $clog2(BYTES_PER_BEAT);

    localparam OP_POS = 28;
    localparam OP_SIZE = 4;
    localparam OP_IMM_POS = 0;
    localparam OP_IMM_SIZE = 28;

    localparam OP_NOP = 0;
    localparam OP_STORE = 5;
    localparam OP_LOAD = 6;
    localparam OP_MEMSET = 7;
    localparam OP_STREAM = 8;

    localparam IDLE = 0;
    localparam COMMAND = 1;
    localparam STORE = 2;
    localparam LOAD = 3;
    localparam STORE_ADDR = 4;
    localparam LOAD_ADDR = 5;
    localparam MEMSET = 6;
    localparam MEMSET_ADDR = 7;
    localparam MEMSET_VAL = 8;
    localparam STREAM = 9;
    localparam STREAM_PAUSED = 10;

    reg [ 5 : 0]                state;
    reg [ 5 : 0]                mux;
    reg [OP_IMM_SIZE - 1 : 0]   counter;
    reg [31 : 0]                memsetVal;
    reg [ADDR_WIDTH - 1 : 0]    addr;
    reg                         enableWriteChannel;
    reg [ADDR_WIDTH - 1 : 0]    addrLast;

    reg                         m_axis_tvalid = 0;
    reg                         m_axis_tready;
    reg                         m_axis_tlast = 0;
    reg  [STREAM_WIDTH - 1 : 0] m_axis_tdata = 0;

    reg                         s_axis_tvalid;
    reg                         s_axis_tready;
    reg                         s_axis_tlast;
    reg [STREAM_WIDTH - 1 : 0]  s_axis_tdata;

    reg                         m_axis_tlast_last;
    reg [STREAM_WIDTH - 1 : 0]  m_axis_tdata_last;

    initial 
    begin
        m_mem_axi_awid = 0;
        m_mem_axi_awlen = 0;
        m_mem_axi_awsize = BYTES_TO_BEATS_SHIFT[0 +: 3];
        m_mem_axi_awburst = 1;
        m_mem_axi_awlock = 0;
        m_mem_axi_awcache = 0; 
        m_mem_axi_awprot = 0;
        m_mem_axi_awvalid = 0;

        m_mem_axi_arid = 0;
        m_mem_axi_arlen = 0;
        m_mem_axi_arsize = BYTES_TO_BEATS_SHIFT[0 +: 3];
        m_mem_axi_arburst = 1;
        m_mem_axi_arlock = 0;
        m_mem_axi_arcache = 0;
        m_mem_axi_arprot = 0;

        m_mem_axi_wstrb = {STRB_WIDTH{1'b1}};

        m_mem_axi_bready = 1;
    end

    // Master Channel Muxes
    assign m_cmd_axis_tvalid    = (mux == STREAM) ? m_axis_tvalid
                                : (mux == LOAD) ? m_axis_tvalid 
                                : 0;

    assign m_mem_axi_wvalid     = (mux == STORE) ? m_axis_tvalid 
                                : (mux == MEMSET) ? m_axis_tvalid 
                                : 0;

    assign m_cmd_axis_tlast = (mux == STREAM) ? m_axis_tlast 
                            : (mux == LOAD) ? m_axis_tlast 
                            : 0;

    assign m_mem_axi_wlast  = 1;
                            
    assign m_cmd_axis_tdata = (mux == STREAM) ? m_axis_tdata 
                            : (mux == LOAD) ? m_axis_tdata 
                            : 0;

    assign m_mem_axi_wdata  = (mux == STORE) ? m_axis_tdata 
                            : (mux == MEMSET) ? m_axis_tdata 
                            : 0;

    // Slave Channel Muxes
    assign s_cmd_axis_tready    = (mux == STREAM) ? s_axis_tready
                                : (mux == STORE) ? s_axis_tready
                                : (mux == COMMAND) ? s_axis_tready
                                : 0;

    assign m_mem_axi_rready     = (mux == LOAD) ? s_axis_tready
                                : 0;

    always @(posedge aclk)
    begin
        // Master Channel Muxes
        m_axis_tready   = (mux == STREAM) ? m_cmd_axis_tready
                        : (mux == LOAD) ? m_cmd_axis_tready
                        : (mux == STORE) ? m_mem_axi_wready
                        : (mux == MEMSET) ? m_mem_axi_wready
                        : 0;

        // Slave Channel Muxes
        s_axis_tvalid   = (mux == STREAM) ? s_cmd_axis_tvalid 
                        : (mux == STORE) ? s_cmd_axis_tvalid
                        : (mux == COMMAND) ? s_cmd_axis_tvalid
                        : (mux == LOAD) ? m_mem_axi_rvalid
                        : (mux == MEMSET) ? 1 
                        : 0;

        s_axis_tlast    = (mux == STREAM) ? s_cmd_axis_tlast 
                        : (mux == STORE) ? s_cmd_axis_tlast
                        : (mux == LOAD) ? m_mem_axi_rlast
                        : (mux == MEMSET) ? 0 
                        : 0;

        s_axis_tdata    = (mux == STREAM) ? s_cmd_axis_tdata
                        : (mux == STORE) ? s_cmd_axis_tdata
                        : (mux == COMMAND) ? s_cmd_axis_tdata
                        : (mux == LOAD) ? m_mem_axi_rdata
                        : (mux == MEMSET) ? memsetVal
                        : 0;

        if (resetn == 0)
        begin
            m_axis_tvalid <= 0;
            m_axis_tlast <= 0;
            s_axis_tready <= 0;

            m_mem_axi_arvalid <= 0;
            m_mem_axi_awvalid <= 0;

            addrLast <= 0;
            addr <= 0;

            state <= IDLE;
            mux <= IDLE;
        end
        else 
        begin
            case (state)
            IDLE:
            begin
                m_axis_tlast <= 0;
                m_axis_tvalid <= 0;

                s_axis_tready <= 1;

                mux <= COMMAND;
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
                        counter <= s_axis_tdata[OP_IMM_POS +: OP_IMM_SIZE] >> BYTES_TO_BEATS_SHIFT;
                        state <= STORE_ADDR;
                    end
                    OP_LOAD:
                    begin
                        counter <= s_axis_tdata[OP_IMM_POS +: OP_IMM_SIZE] >> BYTES_TO_BEATS_SHIFT;
                        state <= LOAD_ADDR;
                    end
                    OP_MEMSET:
                    begin
                        counter <= s_axis_tdata[OP_IMM_POS +: OP_IMM_SIZE] >> BYTES_TO_BEATS_SHIFT;
                        state <= MEMSET_ADDR;
                    end
                    OP_STREAM:
                    begin
                        counter <= s_axis_tdata[OP_IMM_POS +: OP_IMM_SIZE] >> BYTES_TO_BEATS_SHIFT;
                        mux <= STREAM;
                        state <= STREAM;
                    end
                    endcase
                end
            end
            STORE_ADDR:
            begin
                if (s_axis_tvalid)
                begin
                    enableWriteChannel <= 1;
                    addr <= s_axis_tdata[0 +: ADDR_WIDTH];
                    addrLast <= s_axis_tdata[0 +: ADDR_WIDTH] + (counter[0 +: ADDR_WIDTH] << BYTES_TO_BEATS_SHIFT) - BYTES_PER_BEAT;
                    mux <= STORE;
                    state <= STREAM;
                end
            end
            LOAD_ADDR:
            begin
                if (s_axis_tvalid)
                begin
                    enableWriteChannel <= 0;
                    addr <= s_axis_tdata[0 +: ADDR_WIDTH];
                    addrLast <= s_axis_tdata[0 +: ADDR_WIDTH] + (counter[0 +: ADDR_WIDTH] << BYTES_TO_BEATS_SHIFT) - BYTES_PER_BEAT;
                    mux <= LOAD;
                    state <= STREAM;
                end
            end
            MEMSET_ADDR:
            begin
                if (s_axis_tvalid)
                begin
                    enableWriteChannel <= 1;
                    addr <= s_axis_tdata[0 +: ADDR_WIDTH];
                    addrLast <= s_axis_tdata[0 +: ADDR_WIDTH] + (counter[0 +: ADDR_WIDTH] << BYTES_TO_BEATS_SHIFT) - BYTES_PER_BEAT;
                    state <= MEMSET_VAL;
                end
            end
            MEMSET_VAL:
            begin
                if (s_axis_tvalid)
                begin
                    memsetVal <= s_axis_tdata[0 +: 32];
                    mux <= MEMSET;
                    state <= STREAM;
                end
            end
            STREAM:
            begin
                // Optional addr channel
                if (enableWriteChannel)
                begin
                    if (addr != addrLast)
                    begin
                        if (m_mem_axi_awready && m_mem_axi_awvalid) 
                        begin
                            addr <= addr + BYTES_PER_BEAT;
                            m_mem_axi_awaddr <= addr + BYTES_PER_BEAT;
                        end
                        else 
                        begin
                            m_mem_axi_awaddr <= addr;
                        end
                        m_mem_axi_awvalid <= 1;
                    end

                    if (m_mem_axi_awready && (addr == addrLast))
                    begin
                        m_mem_axi_awvalid <= 0;
                    end
                end
                else 
                begin
                    if (addr != addrLast)
                    begin
                        if (m_mem_axi_arready && m_mem_axi_arvalid) 
                        begin
                            addr <= addr + BYTES_PER_BEAT;
                            m_mem_axi_araddr <= addr + BYTES_PER_BEAT;
                        end
                        else 
                        begin
                            m_mem_axi_araddr <= addr;
                        end
                        m_mem_axi_arvalid <= 1;
                    end

                    if (m_mem_axi_arready && (addr == addrLast))
                    begin
                        m_mem_axi_arvalid <= 0;
                    end
                end


                // Write the next beat when the destination is ready
                // if the destination is not ready, preload one entry till the output is valid
                if ((m_axis_tready || !m_axis_tvalid) && (counter != 0))
                begin
                    // Copy data from the source to the destination
                    m_axis_tvalid <= s_axis_tvalid;
                    m_axis_tdata <= s_axis_tdata;
                    m_axis_tlast <= counter == 1;

                    if ((counter != 1) || !s_axis_tvalid)
                    begin
                        s_axis_tready <= 1;
                    end
                    else
                    begin
                        s_axis_tready <= 0;
                    end

                    // Valid data signals what we have fetched one beat and decrement the counter
                    if (s_axis_tvalid)
                    begin
                        counter <= counter - 1;
                    end
                end
                else
                begin
                    s_axis_tready <= 0;
                    if ((counter > 0) && !m_axis_tready && s_axis_tvalid)
                    begin
                        m_axis_tlast_last <= counter == 1;
                        m_axis_tdata_last <= s_axis_tdata;
                        state <= STREAM_PAUSED;
                    end
                end

                // If the counter is zero and we have transfered the last beat, then we can 
                // go back to idle
                if ((counter == 0) && m_axis_tready)
                begin
                    m_axis_tvalid <= 0;
                    s_axis_tready <= 0;
                    m_axis_tlast <= 0;
                end

                if ((counter == 0) && m_axis_tready && !m_mem_axi_awvalid && !m_mem_axi_arvalid)
                begin
                    state <= IDLE;
                end
            end
            STREAM_PAUSED:
            begin
                if (m_axis_tready)
                begin
                    
                    if (counter != 1)
                    begin
                        s_axis_tready <= 1;
                    end
                    
                    // Skid buffer
                    m_axis_tdata <= m_axis_tdata_last;
                    m_axis_tlast <= m_axis_tlast_last;
                    counter <= counter - 1;
                    state <= STREAM;
                end
            end
            endcase 
        end
    end

endmodule
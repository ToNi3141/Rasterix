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

    // Memory transfers have to be 128 bytes aligned.
    // 128 because: The zynq has 64 bit axi3 ports, which means one beat contains 8 bytes.
    // Max beats of an axi3 port are 16.
    // 128 / 8 = 16. So we will be axi3 compliant.
    localparam BEATS_PER_TRANSFER = 128 / BYTES_PER_BEAT;

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
    reg                         enableAddressChannel;

    reg                         axisDestValid;
    reg                         axisDestReady;
    reg                         axisDestLast;
    reg                         axiDestLast;
    reg  [STREAM_WIDTH - 1 : 0] axisDestData;

    reg                         axisSourceValid;
    reg                         axisSourceReady;
    reg                         axisSourceLast;
    reg [STREAM_WIDTH - 1 : 0]  axisSourceData;

    reg                         axisSourceLastNext;
    reg                         axiSourceLastNext;
    reg [STREAM_WIDTH - 1 : 0]  axisSourceDataNext;

    initial 
    begin
        m_mem_axi_awid = 0;
        m_mem_axi_awlen = BEATS_PER_TRANSFER - 1;
        m_mem_axi_awsize = BYTES_TO_BEATS_SHIFT[0 +: 3];
        m_mem_axi_awburst = 1;
        m_mem_axi_awlock = 0;
        m_mem_axi_awcache = 0; 
        m_mem_axi_awprot = 0;
        m_mem_axi_awvalid = 0;

        m_mem_axi_arid = 0;
        m_mem_axi_arlen = BEATS_PER_TRANSFER - 1;
        m_mem_axi_arsize = BYTES_TO_BEATS_SHIFT[0 +: 3];
        m_mem_axi_arburst = 1;
        m_mem_axi_arlock = 0;
        m_mem_axi_arcache = 0;
        m_mem_axi_arprot = 0;

        m_mem_axi_wstrb = {STRB_WIDTH{1'b1}};

        m_mem_axi_bready = 1;
    end

    // Master Channel Muxes output ports
    assign m_cmd_axis_tvalid    = (mux == STREAM) ? axisDestValid
                                : (mux == LOAD) ? axisDestValid 
                                : 0;

    assign m_mem_axi_wvalid     = (mux == STORE) ? axisDestValid 
                                : (mux == MEMSET) ? axisDestValid 
                                : 0;

    assign m_cmd_axis_tlast = (mux == STREAM) ? axisDestLast 
                            : (mux == LOAD) ? axisDestLast 
                            : 0;

    assign m_mem_axi_wlast  = (mux == STORE) ? axiDestLast 
                            : (mux == MEMSET) ? axiDestLast 
                            : 0;
                            
    assign m_cmd_axis_tdata = (mux == STREAM) ? axisDestData 
                            : (mux == LOAD) ? axisDestData 
                            : 0;

    assign m_mem_axi_wdata  = (mux == STORE) ? axisDestData 
                            : (mux == MEMSET) ? axisDestData 
                            : 0;

    // Slave Channel Muxes input ports
    assign s_cmd_axis_tready    = (mux == STREAM) ? axisSourceReady
                                : (mux == STORE) ? axisSourceReady
                                : (mux == COMMAND) ? axisSourceReady
                                : 0;

    assign m_mem_axi_rready     = (mux == LOAD) ? axisSourceReady
                                : 0;

    always @(posedge aclk)
    begin
        // Master Channel Muxes input ports
        axisDestReady   = (mux == STREAM) ? m_cmd_axis_tready
                        : (mux == LOAD) ? m_cmd_axis_tready
                        : (mux == STORE) ? m_mem_axi_wready
                        : (mux == MEMSET) ? m_mem_axi_wready
                        : 0;

        // Slave Channel Muxes output ports
        axisSourceValid   = (mux == STREAM) ? s_cmd_axis_tvalid 
                        : (mux == STORE) ? s_cmd_axis_tvalid
                        : (mux == COMMAND) ? s_cmd_axis_tvalid
                        : (mux == LOAD) ? m_mem_axi_rvalid
                        : (mux == MEMSET) ? 1 
                        : 0;

        axisSourceLast    = (mux == STREAM) ? s_cmd_axis_tlast 
                        : (mux == STORE) ? s_cmd_axis_tlast
                        : (mux == LOAD) ? m_mem_axi_rlast
                        : (mux == MEMSET) ? 0 
                        : 0;

        axisSourceData    = (mux == STREAM) ? s_cmd_axis_tdata
                        : (mux == STORE) ? s_cmd_axis_tdata
                        : (mux == COMMAND) ? s_cmd_axis_tdata
                        : (mux == LOAD) ? m_mem_axi_rdata
                        : (mux == MEMSET) ? memsetVal
                        : 0;

        if (resetn == 0)
        begin
            axisDestValid <= 0;
            axisDestLast <= 0;
            axiDestLast <= 0;
            axisSourceReady <= 0;

            m_mem_axi_arvalid <= 0;
            m_mem_axi_awvalid <= 0;

            addrLast <= 0;
            addr <= 0;

            enableAddressChannel <= 0;

            state <= IDLE;
            mux <= IDLE;
        end
        else 
        begin
            case (state)
            IDLE:
            begin
                axisDestLast <= 0;
                axiDestLast <= 0;
                axisDestValid <= 0;

                axisSourceReady <= 1;

                mux <= COMMAND;
                state <= COMMAND;
            end
            COMMAND:
            begin
                if (axisSourceValid)
                begin
                    case (axisSourceData[OP_POS +: OP_SIZE])
                    OP_NOP:
                    begin
                        axisSourceReady <= 0;
                        state <= IDLE;
                    end
                    OP_STORE:
                    begin
                        counter <= axisSourceData[OP_IMM_POS +: OP_IMM_SIZE] >> BYTES_TO_BEATS_SHIFT;
                        state <= STORE_ADDR;
                    end
                    OP_LOAD:
                    begin
                        counter <= axisSourceData[OP_IMM_POS +: OP_IMM_SIZE] >> BYTES_TO_BEATS_SHIFT;
                        state <= LOAD_ADDR;
                    end
                    OP_MEMSET:
                    begin
                        counter <= axisSourceData[OP_IMM_POS +: OP_IMM_SIZE] >> BYTES_TO_BEATS_SHIFT;
                        state <= MEMSET_ADDR;
                    end
                    OP_STREAM:
                    begin
                        counter <= axisSourceData[OP_IMM_POS +: OP_IMM_SIZE] >> BYTES_TO_BEATS_SHIFT;
                        mux <= STREAM;
                        state <= STREAM;
                    end
                    endcase
                end
            end
            STORE_ADDR:
            begin
                if (axisSourceValid)
                begin
                    enableWriteChannel <= 1;
                    addr <= axisSourceData[0 +: ADDR_WIDTH];
                    addrLast <= axisSourceData[0 +: ADDR_WIDTH] + (counter[0 +: ADDR_WIDTH] << BYTES_TO_BEATS_SHIFT) - (BYTES_PER_BEAT * BEATS_PER_TRANSFER);
                    enableAddressChannel <= 1;
                    mux <= STORE;
                    state <= STREAM;
                end
            end
            LOAD_ADDR:
            begin
                if (axisSourceValid)
                begin
                    enableWriteChannel <= 0;
                    addr <= axisSourceData[0 +: ADDR_WIDTH];
                    addrLast <= axisSourceData[0 +: ADDR_WIDTH] + (counter[0 +: ADDR_WIDTH] << BYTES_TO_BEATS_SHIFT) - (BYTES_PER_BEAT * BEATS_PER_TRANSFER);
                    enableAddressChannel <= 1;
                    mux <= LOAD;
                    state <= STREAM;
                end
            end
            MEMSET_ADDR:
            begin
                if (axisSourceValid)
                begin
                    enableWriteChannel <= 1;
                    addr <= axisSourceData[0 +: ADDR_WIDTH];
                    addrLast <= axisSourceData[0 +: ADDR_WIDTH] + (counter[0 +: ADDR_WIDTH] << BYTES_TO_BEATS_SHIFT) - (BYTES_PER_BEAT * BEATS_PER_TRANSFER);
                    state <= MEMSET_VAL;
                end
            end
            MEMSET_VAL:
            begin
                if (axisSourceValid)
                begin
                    memsetVal <= axisSourceData[0 +: 32];
                    enableAddressChannel <= 1;
                    mux <= MEMSET;
                    state <= STREAM;
                end
            end
            STREAM:
            begin
                // Write the next beat when the destination is ready
                // if the destination is not ready, preload one entry till the output is valid
                if ((axisDestReady || !axisDestValid) && (counter != 0))
                begin
                    // Copy data from the source to the destination
                    axisDestValid <= axisSourceValid;
                    axisDestData <= axisSourceData;
                    axisDestLast <= counter == 1;
                    axiDestLast <= counter[0 +: $clog2(BEATS_PER_TRANSFER)] == 1;

                    // Enable the data source as long as we have to receive data.
                    // If the counter is 1 (which means last beat) but we didn't got valid data, keep the source active.
                    if ((counter != 1) || !axisSourceValid)
                    begin
                        axisSourceReady <= 1;
                    end
                    else
                    begin
                        axisSourceReady <= 0;
                    end

                    // Valid data signals what we have fetched one beat and decrement the counter
                    if (axisSourceValid)
                    begin
                        counter <= counter - 1;
                    end
                end
                else
                begin
                    // Wait till new data are received.
                    // If our destination was not ready, we have to save the data and to disable our data source
                    if ((counter > 0) && !axisDestReady && axisSourceValid)
                    begin
                        axisSourceReady <= 0;

                        axisSourceLastNext <= counter == 1;
                        axiSourceLastNext <= counter[0 +: $clog2(BEATS_PER_TRANSFER)] == 1;
                        axisSourceDataNext <= axisSourceData;

                        counter <= counter - 1;

                        // Pause stream till our destination is ready
                        state <= STREAM_PAUSED;
                    end
                end

                // If the counter is zero and we have transfered the last beat, then we can 
                // go back to idle
                if ((counter == 0) && axisDestReady)
                begin
                    axisDestValid <= 0;
                    axisSourceReady <= 0;
                    axisDestLast <= 0;

                    // Synchronize with the address channel
                    if (!enableAddressChannel)
                    begin
                        state <= IDLE;
                    end
                end
            end
            STREAM_PAUSED:
            begin
                if (axisDestReady)
                begin
                    // Check if the counter is greater than zero, if so, we still have to fetch data
                    if (counter > 0)
                    begin
                        axisSourceReady <= 1;
                    end

                    // Output the next data
                    axisDestData <= axisSourceDataNext;
                    axisDestLast <= axisSourceLastNext;
                    axiDestLast <= axiSourceLastNext;
                    
                    // Reenable the stream
                    state <= STREAM;
                end
            end
            endcase 
        end

        // Optional addr channel
        if (enableAddressChannel)
        begin
            if (enableWriteChannel)
            begin
                if (addr != addrLast)
                begin
                    if (m_mem_axi_awready && m_mem_axi_awvalid) 
                    begin
                        addr <= addr + (BYTES_PER_BEAT * BEATS_PER_TRANSFER);
                        m_mem_axi_awaddr <= addr + (BYTES_PER_BEAT * BEATS_PER_TRANSFER);
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
                    enableAddressChannel <= 0;
                end
            end
            else 
            begin
                if (addr != addrLast)
                begin
                    if (m_mem_axi_arready && m_mem_axi_arvalid) 
                    begin
                        addr <= addr + (BYTES_PER_BEAT * BEATS_PER_TRANSFER);
                        m_mem_axi_araddr <= addr + (BYTES_PER_BEAT * BEATS_PER_TRANSFER);
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
                    enableAddressChannel <= 0;
                end
            end
        end
    end
endmodule
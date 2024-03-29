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

// Reads stream with a vector of pixels from the memory interface. Based on
// the index from the fetch interface, it serializes the vector into pixels
// available on the fragment interface.
// Performance: 1 pixel per cycle
module FramebufferSerializer #(
    // Width of the axi interfaces
    parameter DATA_WIDTH = 32,
    // Width of address bus in bits
    parameter ADDR_WIDTH = 32,
    // Width of ID signal
    parameter ID_WIDTH = 8,

    // Sets the width of a pixel to fix the memory alignment
    parameter PIXEL_WIDTH = 16
) (
    input  wire                             aclk,
    input  wire                             resetn,

    /////////////////////////
    // Fragment Interface
    /////////////////////////

    // Framebuffer Interface
    output reg                              m_frag_tvalid,
    input  wire                             m_frag_tready,
    output reg  [PIXEL_WIDTH - 1 : 0]       m_frag_tdata,
    output reg  [ADDR_WIDTH - 1 : 0]        m_frag_taddr,
    output reg                              m_frag_tlast,

    // Fetch interface
    input  wire                             s_fetch_tvalid,
    input  wire                             s_fetch_tlast,
    output reg                              s_fetch_tready,
    input  wire [ADDR_WIDTH - 1 : 0]        s_fetch_taddr,

    /////////////////////////
    // Memory Interface
    /////////////////////////

    // Data channel
    input  wire [ID_WIDTH - 1 : 0]          m_mem_axi_rid,
    input  wire [DATA_WIDTH - 1 : 0]        m_mem_axi_rdata,
    input  wire [ 1 : 0]                    m_mem_axi_rresp,
    input  wire                             m_mem_axi_rlast,
    input  wire                             m_mem_axi_rvalid,
    output reg                              m_mem_axi_rready
);
    localparam ADDR_BYTE_POS_POS = 0;
    localparam ADDR_BYTE_POS_WIDTH = $clog2(DATA_WIDTH / PIXEL_WIDTH);
    localparam ADDR_TAG_POS = ADDR_BYTE_POS_POS + ADDR_BYTE_POS_WIDTH;
    localparam ADDR_TAG_WIDTH = ADDR_WIDTH - ADDR_TAG_POS;

    reg [ADDR_TAG_WIDTH - 1 : 0]    addrTag;
    reg [ADDR_WIDTH - 1 : 0]        addrSkid;
    reg                             tlastSkid;
    reg                             memoryBubbleCycleRequired;
    reg                             stateSkid;
    reg [DATA_WIDTH - 1 : 0]        cacheLine;
    

    always @(posedge aclk)
    begin
        if (!resetn)
        begin
            m_frag_tvalid <= 0;
            m_frag_tlast <= 0;

            s_fetch_tready <= 1;

            m_mem_axi_rready <= 0;

            addrTag <= ~0;

            memoryBubbleCycleRequired <= 0;
            stateSkid <= 0;
        end
        else
        begin : Fetch
            reg [ADDR_TAG_WIDTH - 1 : 0]        tag;
            reg [ADDR_BYTE_POS_WIDTH  - 1 : 0]  bytePos;
            reg                                 tlast;

            reg firstFrag;
            reg tagValid;
            reg leaveSkid;
            reg normalOperation;

            if (memoryBubbleCycleRequired)
            begin
                memoryBubbleCycleRequired <= 0;
            end

            if (stateSkid)
            begin
                tag = addrSkid[ADDR_TAG_POS +: ADDR_TAG_WIDTH];
                bytePos = addrSkid[ADDR_BYTE_POS_POS +: ADDR_BYTE_POS_WIDTH];
                tlast = tlastSkid;
            end
            else
            begin
                tag = s_fetch_taddr[ADDR_TAG_POS +: ADDR_TAG_WIDTH];
                bytePos = s_fetch_taddr[ADDR_BYTE_POS_POS +: ADDR_BYTE_POS_WIDTH];
                tlast = s_fetch_tlast;
            end
            firstFrag = addrTag == { ADDR_TAG_WIDTH { 1'b1 } };
            tagValid = (addrTag == tag) && !firstFrag;
            leaveSkid = (m_frag_tready || firstFrag) && (tagValid || m_mem_axi_rvalid) && stateSkid;
            normalOperation = (m_frag_tready || firstFrag) && s_fetch_tvalid && !stateSkid;

            if (normalOperation || leaveSkid)
            begin
                // For normal operation (includes leaving the skid state)
                // This means that source data is available and the destination is also capable of receiving data.
                // Only an invalid memory access can cause a stall here.

                if (leaveSkid)
                begin
                    stateSkid <= 0;
                end

                if (tagValid)
                begin
                    // Tag is valid, everything ok
                    m_mem_axi_rready <= 0;
                    m_frag_taddr <= { tag, bytePos };
                    m_frag_tdata <= cacheLine[bytePos * PIXEL_WIDTH +: PIXEL_WIDTH];
                    m_frag_tvalid <= 1;
                    m_frag_tlast <= tlast;
                    if (tlast)
                    begin
                        addrTag <= ~0;
                    end
                end    
                else if (m_mem_axi_rvalid)
                begin
                    if (memoryBubbleCycleRequired)
                    begin
                        // Slow fetch, because a fast fetch was already used (where the rready siganl is exploited).
                        // It is now required to wait for one clock cycle to read the new data. This extra clock cycle 
                        // results in a bubble cycle.
                        stateSkid <= 1;
                        m_mem_axi_rready <= 0;
                        s_fetch_tready <= 0;
                        addrSkid <= { tag, bytePos };
                        tlastSkid <= tlast;
                        m_frag_tvalid <= 0;
                    end
                    else
                    begin
                        // Fast fetch, tag invalid but valid data from memory is available. rready is set to true, but normally
                        // it is now required to wait for one clock cycle. But this is not a problem if no memory fetch is executed
                        // in the next clock cycle. In case a fetch is executed in the next clock cycle, memoryBubbleCycleRequired will
                        // be true and is handled in the code above.
                        // Load data and stream it out
                        m_mem_axi_rready <= 1;
                        memoryBubbleCycleRequired <= 1; // Skid is only required when rready was not one, otherwise we can burst
                        s_fetch_tready <= 1;
                        cacheLine <= m_mem_axi_rdata;
                        m_frag_tdata <= m_mem_axi_rdata[bytePos * PIXEL_WIDTH +: PIXEL_WIDTH];
                        m_frag_taddr <= { tag, bytePos };
                        m_frag_tvalid <= 1;
                        m_frag_tlast <= tlast;
                        if (tlast)
                        begin
                            addrTag <= ~0;
                        end
                        else
                        begin
                            addrTag <= tag;
                        end
                    end
                end
                else
                begin
                    // Tag is not valid and the memory does not contain any data -> pause stream and skid.
                    stateSkid <= 1;
                    m_mem_axi_rready <= 0;
                    s_fetch_tready <= 0;
                    addrSkid <= { tag, bytePos };
                    tlastSkid <= tlast;
                    m_frag_tvalid <= 0;
                end
            end
            else
            begin
                // Case when no source data is available or the destination is not capable of receiving data
                if (stateSkid)
                begin
                    if (m_frag_tready)
                    begin
                        // Invalidate the data if the current state is skid, because during the skid state, no new data will arrive
                        m_frag_tvalid <= 0;
                    end
                end
                else
                begin
                    if (!m_frag_tready)
                    begin
                        // The destination is busy. Only skid when the source has new data. Avoid to load nothing into the skid buffer.
                        if (s_fetch_tvalid)
                        begin
                            stateSkid <= 1;
                            m_mem_axi_rready <= 0;
                            s_fetch_tready <= 0;
                            addrSkid <= { tag, bytePos };
                            tlastSkid <= tlast;
                        end
                    end
                    else if (!s_fetch_tvalid)
                    begin
                        // No new data from the source. It's enough to just set the tvalid to zero till new data is available
                        m_frag_tvalid <= 0;
                    end

                    if (m_mem_axi_rvalid && m_mem_axi_rready)
                    begin
                        m_mem_axi_rready <= 0;
                    end
                end
            end
        end
    end 
endmodule
// Rasterix
// https://github.com/ToNi3141/Rasterix
// Copyright (c) 2022 ToNi3141

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

module FrameBuffer
#(
    parameter FRAME_SIZE = 128 * 128, // 128px * 128px. Used for abort the memset and commit phase when all pixels are transferred without processing the padding pixel
    parameter STREAM_WIDTH = 16,
    localparam SIZE = $clog2(FRAME_SIZE * 2), // The size of the frame buffer as bytes in power of two
    localparam ADDR_WIDTH = SIZE - 1 // Convert SIZE from 8 bit bytes to 16 bit pixels
)
(
    input   wire        clk,
    input   wire        reset,

    // Framebuffer Interface
    input  wire [ADDR_WIDTH - 1 : 0] fragIndexRead,
    output wire [15:0]  fragOut,
    input  wire [ADDR_WIDTH - 1 : 0] fragIndexWrite,
    input  wire [15:0]  fragIn,
    input  wire         fragWriteEnable,
    input  wire [ 3:0]  fragMask,
    
    // Control
    input   wire        apply, // This will apply and clear the cache
    output  reg         applied,
    input   wire        cmdCommit,
    input   wire        cmdMemset,

    // AXI Stream master interface
    output reg          m_axis_tvalid,
    input  wire         m_axis_tready,
    output reg          m_axis_tlast,
    output wire [STREAM_WIDTH - 1 : 0] m_axis_tdata,

    // Clear color
    input  wire [15:0]  clearColor
);
`ifdef UP5K
`define RAM_MODULE SinglePortRam32k
`else
`define RAM_MODULE DualPortRam
`endif
    localparam NUMBER_OF_SUB_PIXELS = 4;
    localparam SUB_PIXEL_WIDTH = 4;
    localparam PIXEL_WIDTH = NUMBER_OF_SUB_PIXELS * SUB_PIXEL_WIDTH;
    localparam PIXEL_PER_BEAT = STREAM_WIDTH / PIXEL_WIDTH;
    localparam STROBES_PER_BEAT = STREAM_WIDTH / SUB_PIXEL_WIDTH;
    localparam PIXEL_PER_BEAT_LOG2 = $clog2(PIXEL_PER_BEAT);
    localparam FRAMEBUFFER_FRAME_SIZE_IN_BEATS = FRAME_SIZE / PIXEL_PER_BEAT;
    localparam MEM_ADDR_WIDTH = ADDR_WIDTH - PIXEL_PER_BEAT_LOG2;
    localparam TILECONTROL_WAIT_FOR_COMMAND = 0;
    localparam TILECONTROL_MEMCPY = 1;
    localparam TILECONTROL_MEMSET = 2;
    localparam TILECONTROL_MEMCPY_INIT = 3;

    // Tile Control
    reg [5:0] stateTileControl;

    wire [STROBES_PER_BEAT - 1 : 0] writeStrobe; 
    wire [MEM_ADDR_WIDTH - 1 : 0]   fragAddrWrite;
    wire [STREAM_WIDTH - 1 : 0]     fragValIn; 
    wire [MEM_ADDR_WIDTH - 1 : 0]   fragAddrRead;
    wire [STREAM_WIDTH - 1 : 0]     fragValOut;
    reg  [ADDR_WIDTH - 1 : 0]       fragAddrReadDelay;
    
    reg                             commandRunning;
    reg  [MEM_ADDR_WIDTH - 1 : 0]   counter;
    wire [MEM_ADDR_WIDTH - 1 : 0]   counterNext = counter + 1;
    wire [MEM_ADDR_WIDTH - 1 : 0]   commitAddr = (m_axis_tready && m_axis_tvalid) ? counterNext : counter;
    wire [MEM_ADDR_WIDTH - 1 : 0]   memsetWriteAddr = (m_axis_tready && (stateTileControl == TILECONTROL_MEMCPY)) ? counterNext : counter;

    reg                             fbWr;
    wire [MEM_ADDR_WIDTH - 1 : 0]   fbAddrBusWrite  = (commandRunning) ? commitAddr : fragAddrWrite;
    wire [MEM_ADDR_WIDTH - 1 : 0]   fbAddrBusRead   = (commandRunning) ? memsetWriteAddr : fragAddrRead;
    wire [STREAM_WIDTH - 1 : 0]     fbDataInBus     = (commandRunning) ? {PIXEL_PER_BEAT{clearColor}} : fragValIn;
    wire                            fbWrBus         = (commandRunning) ? fbWr : fragWriteEnable;
    wire [STROBES_PER_BEAT - 1 : 0] fbWrMaskBus     = (commandRunning) ? {PIXEL_PER_BEAT{fragMask}} : writeStrobe;

    genvar i;
    generate
        if (PIXEL_PER_BEAT == 1)
        begin
            assign fragAddrWrite = fragIndexWrite;
            assign fragValIn = fragIn;
            assign writeStrobe = fragMask;
            assign fragAddrRead = fragIndexRead;
            assign fragOut = fragValOut;
        end
        else
        begin
            for (i = 0; i < PIXEL_PER_BEAT; i = i + 1)
            begin
                assign writeStrobe[(i * NUMBER_OF_SUB_PIXELS) + 0] = (fragIndexWrite[0 +: PIXEL_PER_BEAT_LOG2] == i) & fragMask[0];
                assign writeStrobe[(i * NUMBER_OF_SUB_PIXELS) + 1] = (fragIndexWrite[0 +: PIXEL_PER_BEAT_LOG2] == i) & fragMask[1];
                assign writeStrobe[(i * NUMBER_OF_SUB_PIXELS) + 2] = (fragIndexWrite[0 +: PIXEL_PER_BEAT_LOG2] == i) & fragMask[2];
                assign writeStrobe[(i * NUMBER_OF_SUB_PIXELS) + 3] = (fragIndexWrite[0 +: PIXEL_PER_BEAT_LOG2] == i) & fragMask[3];
            end
            assign fragAddrWrite = fragIndexWrite[PIXEL_PER_BEAT_LOG2 +: MEM_ADDR_WIDTH];
            assign fragValIn = {PIXEL_PER_BEAT{fragIn}};
            assign fragAddrRead = fragIndexRead[PIXEL_PER_BEAT_LOG2 +: MEM_ADDR_WIDTH];

            assign fragOut = fragValOut[fragAddrReadDelay[0 +: PIXEL_PER_BEAT_LOG2] * PIXEL_WIDTH +: PIXEL_WIDTH];
        end
    endgenerate
    assign m_axis_tdata = fragValOut;

    `RAM_MODULE ramTile (
        .clk(clk),
        .reset(reset),

        .writeData(fbDataInBus),
        .write(fbWrBus),
        .writeAddr(fbAddrBusWrite),
        .writeMask(fbWrMaskBus),

        .readData(fragValOut),
        .readAddr(fbAddrBusRead)
    );
    defparam ramTile.MEM_SIZE_BYTES = SIZE;
    defparam ramTile.MEM_WIDTH = STREAM_WIDTH;
    defparam ramTile.WRITE_STROBE_WIDTH = SUB_PIXEL_WIDTH;

    always @(posedge clk)
    begin
        if (reset)
        begin
            stateTileControl <= TILECONTROL_WAIT_FOR_COMMAND;
            commandRunning <= 0;
            m_axis_tvalid <= 0;
            m_axis_tlast <= 0;
            fbWr <= 0;
            applied <= 1;
        end
        else
        begin
            fragAddrReadDelay <= fragIndexRead;
            case (stateTileControl)
            TILECONTROL_WAIT_FOR_COMMAND:
            begin
                counter <= 0;
                if (apply)
                begin
                    applied <= 0;
                    commandRunning <= 1;
                    if (cmdMemset) 
                    begin
                        fbWr <= 1;
                        stateTileControl <= TILECONTROL_MEMSET;
                    end

                    // Commits have priority over a clear.
                    // When both are activated, the user probably wants first to commit and then to clear it.
                    if (cmdCommit)
                    begin
                        fbWr <= 0;
                        stateTileControl <= TILECONTROL_MEMCPY_INIT;
                    end
                end
                else 
                begin
                    applied <= 1;
                    commandRunning <= 0;
                end
            end
            TILECONTROL_MEMCPY_INIT:
            begin
                m_axis_tvalid <= 1;
                stateTileControl <= TILECONTROL_MEMCPY;
            end
            TILECONTROL_MEMCPY:
            begin
                if (m_axis_tready)
                begin
                    counter <= counterNext;
                
                    if (counterNext == (FRAMEBUFFER_FRAME_SIZE_IN_BEATS[0 +: MEM_ADDR_WIDTH] - 1))
                    begin
                        m_axis_tlast <= 1;
                    end

                    // Check if we reached the end of the copy process
                    if (counterNext == FRAMEBUFFER_FRAME_SIZE_IN_BEATS[0 +: MEM_ADDR_WIDTH])
                    begin
                        m_axis_tvalid <= 0; 
                        m_axis_tlast <= 0;

                        // Continue with memset if it is activated
                        if (cmdMemset) 
                        begin
                            counter <= 0;
                            fbWr <= 1;
                            stateTileControl <= TILECONTROL_MEMSET;
                        end
                        else
                        begin
                            stateTileControl <= TILECONTROL_WAIT_FOR_COMMAND;
                        end
                    end
                end
            end
            TILECONTROL_MEMSET:
            begin
                if (counterNext == FRAMEBUFFER_FRAME_SIZE_IN_BEATS[0 +: MEM_ADDR_WIDTH])
                begin
                    fbWr <= 0;
                    stateTileControl <= TILECONTROL_WAIT_FOR_COMMAND;
                end
                counter <= counterNext;
            end
            endcase
        end
    end
endmodule

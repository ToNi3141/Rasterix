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

`include "PixelUtil.vh"

// The framebuffer is used to store the frame.
//
// This module uses an AXIS interface to stream out the framebuffer contents.
// To execute a command, set apply to 1. The framebuffer will then start sampling the command bits and starts executing.
// During execution, applied is set to 0. Is the execution finished, applied is set to 1.
// cmdCommit: This command will start streaming the content of the framebuffer via the AXIS interface.
// cmdMemset: This command will initialize the memory with the color in clearColor
//
// The fragment interface can be used to access single fragments from the framebuffer
//
// Pipelined: n/a
// Depth: 1 cycle
module FrameBuffer
#(
    parameter FRAME_SIZE = 128 * 128, // 128px * 128px. Used for abort the memset and commit phase when all pixels are transferred without processing the padding pixel
    
    parameter STREAM_WIDTH = 16,
    
    // Number of sub pixels / channels the framebuffer can manage (important for the write mask)
    parameter NUMBER_OF_SUB_PIXELS = 4,

    parameter SUB_PIXEL_WIDTH = 4,

    localparam PIXEL_WIDTH = NUMBER_OF_SUB_PIXELS * SUB_PIXEL_WIDTH,
    localparam SIZE = $clog2(FRAME_SIZE * (PIXEL_WIDTH / 8)), // The size of the frame buffer as bytes in power of two
    localparam ADDR_WIDTH = SIZE - 1 // Convert SIZE from 8 bit bytes to 16 bit pixels
)
(
    input   wire                        clk,
    input   wire                        reset,

    /////////////////////////
    // Fragment interface
    /////////////////////////

    // Framebuffer Interface
    input  wire [ADDR_WIDTH - 1 : 0]    fragIndexRead,
    output wire [PIXEL_WIDTH - 1 : 0]   fragOut,
    input  wire [ADDR_WIDTH - 1 : 0]    fragIndexWrite,
    input  wire [PIXEL_WIDTH - 1 : 0]   fragIn,
    input  wire                         fragWriteEnable,
    input  wire [NUMBER_OF_SUB_PIXELS - 1 : 0]  fragMask,
    
    /////////////////////////
    // Control
    /////////////////////////

    // Cmd interface
    input   wire                        apply, // This will apply and clear the cache
    output  reg                         applied,
    input   wire                        cmdCommit,
    input   wire                        cmdMemset,
    input  wire [PIXEL_WIDTH - 1 : 0]   clearColor,

    // AXI Stream master interface
        output reg                          m_axis_tvalid,
        input  wire                         m_axis_tready,
        output reg                          m_axis_tlast,
        output wire [STREAM_WIDTH - 1 : 0]  m_axis_tdata
    
);
    localparam PIXEL_PER_BEAT = STREAM_WIDTH / PIXEL_WIDTH;
    localparam STROBES_PER_BEAT = STREAM_WIDTH / SUB_PIXEL_WIDTH;
    localparam PIXEL_PER_BEAT_LOG2 = $clog2(PIXEL_PER_BEAT);
    localparam FRAMEBUFFER_FRAME_SIZE_IN_BEATS = FRAME_SIZE / PIXEL_PER_BEAT;
    localparam MEM_ADDR_WIDTH = ADDR_WIDTH - PIXEL_PER_BEAT_LOG2;

    // Stream states
    localparam COMMAND_WAIT_FOR_COMMAND = 0;
    localparam COMMAND_MEMCPY = 1;
    localparam COMMAND_MEMSET = 2;
    localparam COMMAND_MEMCPY_INIT = 3;

    // Tile Control
    reg [5:0] commandState;

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
    wire [MEM_ADDR_WIDTH - 1 : 0]   memsetWriteAddr = (m_axis_tready && (commandState == COMMAND_MEMCPY)) ? counterNext : counter;

    reg                             fbWr;
    wire [MEM_ADDR_WIDTH - 1 : 0]   fbAddrBusWrite  = (commandRunning) ? commitAddr : fragAddrWrite;
    wire [MEM_ADDR_WIDTH - 1 : 0]   fbAddrBusRead   = (commandRunning) ? memsetWriteAddr : fragAddrRead;
    wire [STREAM_WIDTH - 1 : 0]     fbDataInBus     = (commandRunning) ? {PIXEL_PER_BEAT{clearColor}} : fragValIn;
    wire                            fbWrBus         = (commandRunning) ? fbWr : fragWriteEnable;
    wire [STROBES_PER_BEAT - 1 : 0] fbWrMaskBus     = (commandRunning) ? {PIXEL_PER_BEAT{fragMask}} : writeStrobe;

    reg                             cmdMemsetPending = 0;
    reg  [PIXEL_WIDTH - 1 : 0]      clearColorPending = 0;

    generate
        genvar i, j;
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
                for (j = 0; j < NUMBER_OF_SUB_PIXELS; j = j + 1)
                begin
                    assign writeStrobe[(i * NUMBER_OF_SUB_PIXELS) + j] = (fragIndexWrite[0 +: PIXEL_PER_BEAT_LOG2] == i) & fragMask[j];
                end
            end
            assign fragAddrWrite = fragIndexWrite[PIXEL_PER_BEAT_LOG2 +: MEM_ADDR_WIDTH];
            assign fragValIn = {PIXEL_PER_BEAT{fragIn}};
            assign fragAddrRead = fragIndexRead[PIXEL_PER_BEAT_LOG2 +: MEM_ADDR_WIDTH];

            assign fragOut = fragValOut[fragAddrReadDelay[0 +: PIXEL_PER_BEAT_LOG2] * PIXEL_WIDTH +: PIXEL_WIDTH];
        end
    endgenerate

    assign m_axis_tdata = fragValOut;

    DualPortRam ramTile (
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
            commandState <= COMMAND_WAIT_FOR_COMMAND;
            commandRunning <= 0;
            m_axis_tvalid <= 0;
            m_axis_tlast <= 0;
            fbWr <= 0;
            applied <= 1;
        end
        else
        begin
            fragAddrReadDelay <= fragIndexRead;
            case (commandState)
            COMMAND_WAIT_FOR_COMMAND:
            begin
                counter <= 0;
                if (apply)
                begin
                    applied <= 0;
                    commandRunning <= 1;

                    cmdMemsetPending <= cmdMemset;
                    clearColorPending <= clearColor;

                    if (cmdMemset) 
                    begin
                        fbWr <= 1;
                        commandState <= COMMAND_MEMSET;
                    end

                    // Commits have priority over a clear.
                    // When both are activated, the user probably wants first to commit and then to clear it.
                    if (cmdCommit)
                    begin
                        fbWr <= 0;
                        commandState <= COMMAND_MEMCPY_INIT;
                    end
                end
                else 
                begin
                    applied <= 1;
                    commandRunning <= 0;
                end
            end
            COMMAND_MEMCPY_INIT:
            begin
                m_axis_tvalid <= 1;
                commandState <= COMMAND_MEMCPY;
            end
            COMMAND_MEMCPY:
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
                        if (cmdMemsetPending) 
                        begin
                            counter <= 0;
                            fbWr <= 1;
                            commandState <= COMMAND_MEMSET;
                        end
                        else
                        begin
                            commandState <= COMMAND_WAIT_FOR_COMMAND;
                        end
                    end
                end
            end
            COMMAND_MEMSET:
            begin
                if (counterNext == FRAMEBUFFER_FRAME_SIZE_IN_BEATS[0 +: MEM_ADDR_WIDTH])
                begin
                    fbWr <= 0;
                    commandState <= COMMAND_WAIT_FOR_COMMAND;
                end
                counter <= counterNext;
            end
            endcase
        end
    end
endmodule

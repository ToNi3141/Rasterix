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
// Improvements: The scrissor is not optimized. When memset is called, it will set the whole memory except
// the scissor area. A improved version will just set the scissor area.
// It is not implemented, because it requires additional logic for the index calculation (multiplier and so on).
//
// Pipelined: n/a
// Depth: 1 cycle
module FrameBuffer
#(
    
    parameter STREAM_WIDTH = 16,
    
    // Number of sub pixels / channels the framebuffer can manage (important for the write mask)
    parameter NUMBER_OF_SUB_PIXELS = 4,

    parameter SUB_PIXEL_WIDTH = 4,

    parameter X_RESOLUTION = 1024,
    parameter Y_RESOLUTION = 600,
    parameter Y_LINE_RESOLUTION = 48,
    localparam FRAME_SIZE = X_RESOLUTION * Y_LINE_RESOLUTION, // 128px * 128px. Used for abort the memset and commit phase when all pixels are transferred without processing the padding pixel

    localparam PIXEL_WIDTH = NUMBER_OF_SUB_PIXELS * SUB_PIXEL_WIDTH,
    localparam SIZE = $clog2(FRAME_SIZE * (PIXEL_WIDTH / 8)), // The size of the frame buffer as bytes in power of two
    localparam ADDR_WIDTH = SIZE - 1, // Convert SIZE from 8 bit bytes to 16 bit pixels
    parameter SCREEN_POS_WIDTH = 16
)
(
    input   wire                        clk,
    input   wire                        reset,

    /////////////////////////
    // Configs
    /////////////////////////
    input  wire                             confEnableScissor,
    input  wire [SCREEN_POS_WIDTH - 1 : 0]  confScissorStartX,
    input  wire [SCREEN_POS_WIDTH - 1 : 0]  confScissorStartY,
    input  wire [SCREEN_POS_WIDTH - 1 : 0]  confScissorEndX,
    input  wire [SCREEN_POS_WIDTH - 1 : 0]  confScissorEndY,
    input  wire [11 : 0]                    confYOffset,

    /////////////////////////
    // Fragment interface
    /////////////////////////

    // Framebuffer Interface
    input  wire [ADDR_WIDTH - 1 : 0]    fragIndexRead,
    output reg  [PIXEL_WIDTH - 1 : 0]   fragOut,
    input  wire [ADDR_WIDTH - 1 : 0]    fragIndexWrite,
    input  wire [PIXEL_WIDTH - 1 : 0]   fragIn,
    input  wire                         fragWriteEnable,
    input  wire [NUMBER_OF_SUB_PIXELS - 1 : 0]  fragMask,
    input  wire [SCREEN_POS_WIDTH - 1 : 0]  screenPosX,
    input  wire [SCREEN_POS_WIDTH - 1 : 0]  screenPosY,
    
    /////////////////////////
    // Control
    /////////////////////////

    // Cmd interface
    input  wire                         apply, // This will apply and clear the cache
    output reg                          applied,
    input  wire                         cmdCommit,
    input  wire                         cmdMemset,
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
    localparam MEM_ADDR_WIDTH = ADDR_WIDTH - PIXEL_PER_BEAT_LOG2;
    localparam FRAMEBUFFER_FRAME_SIZE_IN_BEATS_TMP = (FRAME_SIZE / PIXEL_PER_BEAT);
    localparam [MEM_ADDR_WIDTH - 1 : 0] FRAMEBUFFER_FRAME_SIZE_IN_BEATS = FRAMEBUFFER_FRAME_SIZE_IN_BEATS_TMP[MEM_ADDR_WIDTH - 1 : 0];

    // Stream states
    localparam COMMAND_WAIT_FOR_COMMAND = 0;
    localparam COMMAND_MEMCPY = 1;
    localparam COMMAND_MEMSET = 2;
    localparam COMMAND_MEMCPY_INIT = 3;

    function [0 : 0] scissorFunc;
        input           enable;
        input [15 : 0]  startX;
        input [15 : 0]  startY;
        input [15 : 0]  endX;
        input [15 : 0]  endY;
        input [15 : 0]  screenX;
        input [15 : 0]  screenY;
        begin
            scissorFunc = !enable || ((screenX >= startX) && (screenX < endX) && (screenY >= startY) && (screenY < endY));
        end
    endfunction

    // Tile Control
    
    reg  [STROBES_PER_BEAT - 1 : 0] memsetScissor;

    wire [STROBES_PER_BEAT - 1 : 0] fragWriteMask; 
    wire [MEM_ADDR_WIDTH - 1 : 0]   fragAddrWrite;
    wire [MEM_ADDR_WIDTH - 1 : 0]   fragAddrRead;
    reg  [ADDR_WIDTH - 1 : 0]       fragAddrReadDelay;
    wire [STREAM_WIDTH - 1 : 0]     fragDataIn; 
    wire                            fragScissorTest;
    
    
    reg  [MEM_ADDR_WIDTH - 1 : 0]   cmdIndex;
    wire [MEM_ADDR_WIDTH - 1 : 0]   cmdIndexNext = cmdIndex + 1;
    reg                             cmdRunning;
    reg  [5 : 0]                    cmdState;
    reg                             cmdWrite;
    wire [MEM_ADDR_WIDTH - 1 : 0]   cmdMemsetAddr = cmdIndex;
    wire [MEM_ADDR_WIDTH - 1 : 0]   cmdMemcpyAddr = (m_axis_tready && (cmdState == COMMAND_MEMCPY)) ? cmdIndexNext : cmdIndex;

    reg  [15 : 0]                   cmdMemsetX;
    wire [15 : 0]                   cmdMemsetXNext = cmdMemsetX + PIXEL_PER_BEAT;
    reg  [15 : 0]                   cmdMemsetY;
    wire [15 : 0]                   cmdMemsetYNext = cmdMemsetY - 1;
    
    wire [STROBES_PER_BEAT - 1 : 0] cmdMemsetScissorMask;
    wire [PIXEL_PER_BEAT - 1 : 0]   cmdMemsetScissor;
    
    genvar i, j;
    generate
        if (PIXEL_PER_BEAT == 1)
        begin : MemsetScissorSingle
            assign cmdMemsetScissor = scissorFunc(confEnableScissor, confScissorStartX, confScissorStartY, confScissorEndX, confScissorEndY, cmdMemsetX, cmdMemsetY);
            assign cmdMemsetScissorMask = { NUMBER_OF_SUB_PIXELS { cmdMemsetScissor } };
        end
        else
        begin
            for (i = 0; i < PIXEL_PER_BEAT; i = i + 1)
            begin
                assign cmdMemsetScissor[i] = scissorFunc(confEnableScissor, confScissorStartX, confScissorStartY, confScissorEndX, confScissorEndY, cmdMemsetX + i, cmdMemsetY);
                for (j = 0; j < NUMBER_OF_SUB_PIXELS; j = j + 1)
                begin
                    assign cmdMemsetScissorMask[(i * NUMBER_OF_SUB_PIXELS) + j] = cmdMemsetScissor[i];
                end
            end
        end
    endgenerate
    
    wire [STREAM_WIDTH - 1 : 0]     memBusDataOut;
    wire [MEM_ADDR_WIDTH - 1 : 0]   memBusAddrWrite = (cmdRunning) ? cmdMemsetAddr                                          : fragAddrWrite;
    wire [MEM_ADDR_WIDTH - 1 : 0]   memBusAddrRead  = (cmdRunning) ? cmdMemcpyAddr                                          : fragAddrRead;
    wire [STREAM_WIDTH - 1 : 0]     memBusDataIn    = (cmdRunning) ? { PIXEL_PER_BEAT { clearColor } }                      : fragDataIn;
    wire                            memBusWrite     = (cmdRunning) ? cmdWrite                                               : fragWriteEnable;
    wire [STROBES_PER_BEAT - 1 : 0] memBusWriteMask = (cmdRunning) ? { PIXEL_PER_BEAT { fragMask } } & cmdMemsetScissorMask : fragWriteMask;

    reg                             cmdMemsetPending = 0;
    reg  [PIXEL_WIDTH - 1 : 0]      clearColorPending = 0;

    wire [PIXEL_WIDTH - 1 : 0]      fragOutTmp;

    assign fragScissorTest = scissorFunc(confEnableScissor, confScissorStartX, confScissorStartY, confScissorEndX, confScissorEndY, screenPosX, screenPosY);

    generate
        if (PIXEL_PER_BEAT == 1)
        begin
            assign fragAddrWrite = fragIndexWrite;
            assign fragDataIn = fragIn;
            assign fragWriteMask = fragMask & {NUMBER_OF_SUB_PIXELS{fragScissorTest}};
            assign fragAddrRead = fragIndexRead;
            assign fragOutTmp = memBusDataOut;
        end
        else
        begin
            for (i = 0; i < PIXEL_PER_BEAT; i = i + 1)
            begin
                for (j = 0; j < NUMBER_OF_SUB_PIXELS; j = j + 1)
                begin
                    assign fragWriteMask[(i * NUMBER_OF_SUB_PIXELS) + j] = (fragIndexWrite[0 +: PIXEL_PER_BEAT_LOG2] == i) & fragMask[j] & fragScissorTest;
                end
            end
            assign fragAddrWrite = fragIndexWrite[PIXEL_PER_BEAT_LOG2 +: MEM_ADDR_WIDTH];
            assign fragDataIn = { PIXEL_PER_BEAT { fragIn } };
            assign fragAddrRead = fragIndexRead[PIXEL_PER_BEAT_LOG2 +: MEM_ADDR_WIDTH];

            assign fragOutTmp = memBusDataOut[fragAddrReadDelay[0 +: PIXEL_PER_BEAT_LOG2] * PIXEL_WIDTH +: PIXEL_WIDTH];
        end
    endgenerate

    always @(posedge clk)
    begin
        fragOut <= fragOutTmp;
    end

    assign m_axis_tdata = memBusDataOut;

    DualPortRam ramTile (
        .clk(clk),
        .reset(reset),

        .writeData(memBusDataIn),
        .write(memBusWrite),
        .writeAddr(memBusAddrWrite),
        .writeMask(memBusWriteMask),

        .readData(memBusDataOut),
        .readAddr(memBusAddrRead)
    );
    defparam ramTile.MEM_SIZE_BYTES = SIZE;
    defparam ramTile.MEM_WIDTH = STREAM_WIDTH;
    defparam ramTile.WRITE_STROBE_WIDTH = SUB_PIXEL_WIDTH;

    always @(posedge clk)
    begin
        if (reset)
        begin
            cmdState <= COMMAND_WAIT_FOR_COMMAND;
            cmdRunning <= 0;
            m_axis_tvalid <= 0;
            m_axis_tlast <= 0;
            cmdWrite <= 0;
            applied <= 1;
        end
        else
        begin
            fragAddrReadDelay <= fragIndexRead;
            case (cmdState)
            COMMAND_WAIT_FOR_COMMAND:
            begin
                cmdIndex <= 0;
                cmdMemsetX <= 0;

                // Here is a missmatch between the RAM adresses and the OpenGL coordiate system.
                // OpenGL starts at the lower left corner. But this a fairly high address in the RAM.
                // The cmdIndex starts at zero. This is basically in OpenGL the position (0, Y_LINE_RESOLUTION - 1)
                cmdMemsetY <= confYOffset + Y_LINE_RESOLUTION - 1;

                if (apply)
                begin
                    applied <= 0;
                    cmdRunning <= 1;

                    cmdMemsetPending <= cmdMemset;
                    clearColorPending <= clearColor;

                    if (cmdMemset) 
                    begin
                        cmdWrite <= 1;
                        cmdState <= COMMAND_MEMSET;
                    end

                    // Commits have priority over a clear.
                    // When both are activated, the user probably wants first to commit and then to clear it.
                    if (cmdCommit)
                    begin
                        cmdWrite <= 0;
                        cmdState <= COMMAND_MEMCPY_INIT;
                    end
                end
                else 
                begin
                    applied <= 1;
                    cmdRunning <= 0;
                end
            end
            COMMAND_MEMCPY_INIT:
            begin
                m_axis_tvalid <= 1;
                cmdState <= COMMAND_MEMCPY;
            end
            COMMAND_MEMCPY:
            begin
                if (m_axis_tready)
                begin
                    cmdIndex <= cmdIndexNext;
                
                    if (cmdIndexNext == (FRAMEBUFFER_FRAME_SIZE_IN_BEATS - 1))
                    begin
                        m_axis_tlast <= 1;
                    end

                    // Check if we reached the end of the copy process
                    if (cmdIndexNext == FRAMEBUFFER_FRAME_SIZE_IN_BEATS)
                    begin
                        m_axis_tvalid <= 0; 
                        m_axis_tlast <= 0;

                        // Continue with memset if it is activated
                        if (cmdMemsetPending) 
                        begin
                            cmdIndex <= 0;
                            cmdWrite <= 1;
                            cmdState <= COMMAND_MEMSET;
                        end
                        else
                        begin
                            cmdState <= COMMAND_WAIT_FOR_COMMAND;
                        end
                    end
                end
            end
            COMMAND_MEMSET:
            begin
                if (cmdIndexNext == FRAMEBUFFER_FRAME_SIZE_IN_BEATS)
                begin
                    cmdWrite <= 0;
                    cmdState <= COMMAND_WAIT_FOR_COMMAND;
                end
                cmdIndex <= cmdIndexNext;

                if (cmdMemsetXNext == X_RESOLUTION)
                begin
                    cmdMemsetX <= 0;
                    cmdMemsetY <= cmdMemsetYNext;
                end
                else
                begin
                    cmdMemsetX <= cmdMemsetXNext;
                end
            end
            endcase
        end
    end
endmodule

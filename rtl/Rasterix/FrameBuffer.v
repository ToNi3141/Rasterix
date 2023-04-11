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

// The framebuffer is used to store the frame.
//
// This module uses an AXIS interface to stream out the framebuffer contents.
// To execute a command, set apply to 1. The framebuffer will then start sampling the command bits and starts executing.
// During execution, applied is set to 0. Is the execution finished, applied is set to 1.
// cmdCommit: This command will start streaming the content of the framebuffer via the AXIS interface.
// cmdMemset: This command will initialize the memory with the color in confClearColor
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
    // Number of pixels a stream beat contains
    parameter NUMBER_OF_PIXELS_PER_BEAT = 1,
    
    // Number of sub pixels the interface of this module containts
    parameter NUMBER_OF_SUB_PIXELS = 4,
    // Number of bits of each sub pixel containts
    parameter SUB_PIXEL_WIDTH = 8,

    // The maximum size of the screen in power of two
    parameter X_BIT_WIDTH = 11,
    parameter Y_BIT_WIDTH = 11,
    parameter FRAMEBUFFER_SIZE_IN_WORDS = 18, // Framebuffer size in power of two words (PIXEL_WIDTH)

    // Size of the pixels
    localparam PIXEL_WIDTH = NUMBER_OF_SUB_PIXELS * SUB_PIXEL_WIDTH,

    // Size of the internal memory
    localparam ADDR_WIDTH = FRAMEBUFFER_SIZE_IN_WORDS,

    // Width of the AXIS interface with the frame buffer content
    localparam STREAM_WIDTH = NUMBER_OF_PIXELS_PER_BEAT * PIXEL_WIDTH
)
(
    input   wire                            clk,
    input   wire                            reset,

    /////////////////////////
    // Configs
    /////////////////////////
    input  wire [PIXEL_WIDTH - 1 : 0]       confClearColor,
    input  wire                             confEnableScissor,
    input  wire [X_BIT_WIDTH - 1 : 0]       confScissorStartX,
    input  wire [Y_BIT_WIDTH - 1 : 0]       confScissorStartY,
    input  wire [X_BIT_WIDTH - 1 : 0]       confScissorEndX,
    input  wire [Y_BIT_WIDTH - 1 : 0]       confScissorEndY,
    input  wire [Y_BIT_WIDTH - 1 : 0]       confYOffset,
    input  wire [X_BIT_WIDTH - 1 : 0]       confXResolution,
    input  wire [Y_BIT_WIDTH - 1 : 0]       confYResolution,

    /////////////////////////
    // Fragment interface
    /////////////////////////

    // Framebuffer Interface
    input  wire [ADDR_WIDTH - 1 : 0]        fragIndexRead,
    output reg  [PIXEL_WIDTH - 1 : 0]       fragOut,
    input  wire [ADDR_WIDTH - 1 : 0]        fragIndexWrite,
    input  wire [PIXEL_WIDTH - 1 : 0]       fragIn,
    input  wire                             fragWriteEnable,
    input  wire [NUMBER_OF_SUB_PIXELS - 1 : 0]  fragMask,
    input  wire [X_BIT_WIDTH - 1 : 0]       screenPosX,
    input  wire [Y_BIT_WIDTH - 1 : 0]       screenPosY,
    
    /////////////////////////
    // Control
    /////////////////////////

    // Cmd interface
    input  wire                             apply, // This start a command 
    output reg                              applied, // This marks if the commands have been applied.
    input  wire                             cmdCommit, // Starts to stream the memory content via the AXIS interface
    input  wire                             cmdMemset, // Applies the confClearColor (with respect to the scissor) to the memory

    // AXI Stream master interface
    output reg                              m_axis_tvalid,
    input  wire                             m_axis_tready,
    output reg                              m_axis_tlast,
    output wire [STREAM_WIDTH - 1 : 0]      m_axis_tdata
    
);
    // Number of pixels a AXIS beat or a memory line can contain
    localparam PIXEL_PER_BEAT_LOG2 = $clog2(NUMBER_OF_PIXELS_PER_BEAT);

    // Size constrains of the internal memory
    localparam MEM_PIXEL_WIDTH = NUMBER_OF_SUB_PIXELS * SUB_PIXEL_WIDTH;
    localparam MEM_MASK_WIDTH = NUMBER_OF_PIXELS_PER_BEAT * NUMBER_OF_SUB_PIXELS;
    localparam MEM_WIDTH = MEM_MASK_WIDTH * SUB_PIXEL_WIDTH;
    localparam MEM_ADDR_WIDTH = ADDR_WIDTH - PIXEL_PER_BEAT_LOG2;

    // Stream states
    localparam COMMAND_WAIT_FOR_COMMAND = 0;
    localparam COMMAND_MEMCPY = 1;
    localparam COMMAND_MEMSET = 2;
    localparam COMMAND_MEMCPY_INIT = 3;

    // Scissor function
    function [0 : 0] scissorFunc;
        input                       enable;
        input [X_BIT_WIDTH - 1 : 0] startX;
        input [Y_BIT_WIDTH - 1 : 0] startY;
        input [X_BIT_WIDTH - 1 : 0] endX;
        input [Y_BIT_WIDTH - 1 : 0] endY;
        input [X_BIT_WIDTH - 1 : 0] screenX;
        input [Y_BIT_WIDTH - 1 : 0] screenY;
        begin
            scissorFunc = !enable || ((screenX >= startX) && (screenX < endX) && (screenY >= startY) && (screenY < endY));
        end
    endfunction

    // Interface to the memory when accessing a fragment. This interface has already the size required for the internal memory.
    wire [MEM_MASK_WIDTH - 1 : 0]   memWriteMask; 
    wire [MEM_ADDR_WIDTH - 1 : 0]   memAddrWrite;
    wire [MEM_ADDR_WIDTH - 1 : 0]   memAddrRead;
    reg  [ADDR_WIDTH - 1 : 0]       memAddrReadDelay;
    wire [MEM_WIDTH - 1 : 0]        memDataIn; 
    wire [MEM_PIXEL_WIDTH - 1 : 0]  memDataOut;
    wire [MEM_MASK_WIDTH - 1 : 0]   memMask = { NUMBER_OF_PIXELS_PER_BEAT { fragMask } };
    wire                            memScissorTest;
    wire                            memWriteEnable = fragWriteEnable;
    
    // State variables for managing the memory (memset and memcpy)
    reg  [MEM_ADDR_WIDTH - 1 : 0]   cmdIndex;
    wire [MEM_ADDR_WIDTH - 1 : 0]   cmdIndexNext = cmdIndex + 1;
    reg                             cmdRunning;
    reg  [5 : 0]                    cmdState;
    reg                             cmdWrite;
    wire [MEM_MASK_WIDTH - 1 : 0]   cmdMask = { NUMBER_OF_PIXELS_PER_BEAT { fragMask } };
    reg  [MEM_ADDR_WIDTH - 1 : 0]   cmdFbSizeInBeats;

    // Memcpy address
    wire [MEM_ADDR_WIDTH - 1 : 0]   cmdMemcpyAddr = (m_axis_tready && (cmdState == COMMAND_MEMCPY)) ? cmdIndexNext : cmdIndex;
    
    // State variables for the memset
    wire [MEM_ADDR_WIDTH - 1 : 0]   cmdMemsetAddr = cmdIndex;
    reg  [X_BIT_WIDTH - 1 : 0]      cmdMemsetX;
    wire [X_BIT_WIDTH - 1 : 0]      cmdMemsetXNext = cmdMemsetX + NUMBER_OF_PIXELS_PER_BEAT;
    reg  [Y_BIT_WIDTH - 1 : 0]      cmdMemsetY;
    wire [Y_BIT_WIDTH - 1 : 0]      cmdMemsetYNext = cmdMemsetY - 1;
    wire [MEM_MASK_WIDTH - 1 : 0]   cmdMemsetScissorMask;
    wire [NUMBER_OF_PIXELS_PER_BEAT - 1 : 0]   cmdMemsetScissor;
    reg                             cmdMemsetPending = 0;

    // Multiplexed interface to the internal memory. It multiplexs between the commands and the fragmend access.
    wire [MEM_WIDTH - 1 : 0]        memBusDataOut;
    wire [MEM_ADDR_WIDTH - 1 : 0]   memBusAddrWrite = (cmdRunning) ? cmdMemsetAddr                          : memAddrWrite;
    wire [MEM_ADDR_WIDTH - 1 : 0]   memBusAddrRead  = (cmdRunning) ? cmdMemcpyAddr                          : memAddrRead;
    wire [MEM_WIDTH - 1 : 0]        memBusDataIn    = (cmdRunning) ? { NUMBER_OF_PIXELS_PER_BEAT { confClearColor } }  : memDataIn;
    wire                            memBusWrite     = (cmdRunning) ? cmdWrite                               : memWriteEnable;
    wire [MEM_MASK_WIDTH - 1 : 0]   memBusWriteMask = (cmdRunning) ? cmdMask & cmdMemsetScissorMask         : memWriteMask;

    // Instance of the internal memory
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
    defparam ramTile.ADDR_WIDTH = MEM_ADDR_WIDTH;
    defparam ramTile.MEM_WIDTH = MEM_WIDTH;
    defparam ramTile.WRITE_STROBE_WIDTH = SUB_PIXEL_WIDTH;

    genvar i, j;

    // Scissor check for the memset command 
    generate
        if (NUMBER_OF_PIXELS_PER_BEAT == 1)
        begin 
            assign cmdMemsetScissor = scissorFunc(confEnableScissor, confScissorStartX, confScissorStartY, confScissorEndX, confScissorEndY, cmdMemsetX, cmdMemsetY);
            assign cmdMemsetScissorMask = { NUMBER_OF_SUB_PIXELS { cmdMemsetScissor } };
        end
        else
        begin
            for (i = 0; i < NUMBER_OF_PIXELS_PER_BEAT; i = i + 1)
            begin
                assign cmdMemsetScissor[i] = scissorFunc(confEnableScissor, confScissorStartX, confScissorStartY, confScissorEndX, confScissorEndY, cmdMemsetX + i, cmdMemsetY);
                for (j = 0; j < NUMBER_OF_SUB_PIXELS; j = j + 1)
                begin
                    assign cmdMemsetScissorMask[(i * NUMBER_OF_SUB_PIXELS) + j] = cmdMemsetScissor[i];
                end
            end
        end
    endgenerate

    // Fragment access including the scissor check
    generate
        assign memScissorTest = scissorFunc(confEnableScissor, confScissorStartX, confScissorStartY, confScissorEndX, confScissorEndY, screenPosX, screenPosY);
        if (NUMBER_OF_PIXELS_PER_BEAT == 1)
        begin
            assign memAddrWrite = fragIndexWrite;
            assign memDataIn = fragIn;
            assign memWriteMask = memMask & { NUMBER_OF_SUB_PIXELS { memScissorTest } };
            assign memAddrRead = fragIndexRead;
            assign memDataOut = memBusDataOut;
        end
        else
        begin
            for (i = 0; i < NUMBER_OF_PIXELS_PER_BEAT; i = i + 1)
            begin
                for (j = 0; j < NUMBER_OF_SUB_PIXELS; j = j + 1)
                begin
                    assign memWriteMask[(i * NUMBER_OF_SUB_PIXELS) + j] = (fragIndexWrite[0 +: PIXEL_PER_BEAT_LOG2] == i) & memMask[j] & memScissorTest;
                end
            end
            assign memAddrWrite = fragIndexWrite[PIXEL_PER_BEAT_LOG2 +: MEM_ADDR_WIDTH];
            assign memDataIn = { NUMBER_OF_PIXELS_PER_BEAT { fragIn } };
            assign memAddrRead = fragIndexRead[PIXEL_PER_BEAT_LOG2 +: MEM_ADDR_WIDTH];
            assign memDataOut = memBusDataOut[memAddrReadDelay[0 +: PIXEL_PER_BEAT_LOG2] * MEM_PIXEL_WIDTH +: MEM_PIXEL_WIDTH];
        end
    endgenerate

    always @(posedge clk)
    begin
        fragOut <= memDataOut;
    end
            
    assign m_axis_tdata = memBusDataOut;

    // Command execution
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
            memAddrReadDelay <= fragIndexRead;
            case (cmdState)
            COMMAND_WAIT_FOR_COMMAND:
            begin : waitForCommand
                reg [X_BIT_WIDTH + Y_BIT_WIDTH -1 : 0] fbSize;
                cmdIndex <= 0;
                cmdMemsetX <= 0;

                // Here is a missmatch between the RAM adresses and the OpenGL coordiate system.
                // OpenGL starts at the lower left corner. But this is a fairly high address in the RAM.
                // The cmdIndex starts at zero. This is basically in OpenGL the position (0, confYResolution - 1)
                cmdMemsetY <= confYOffset + confYResolution - 1;

                fbSize = confYResolution * confXResolution;
                cmdFbSizeInBeats <= fbSize[PIXEL_PER_BEAT_LOG2 +: MEM_ADDR_WIDTH];

                if (apply)
                begin
                    applied <= 0;
                    cmdRunning <= 1;

                    cmdMemsetPending <= cmdMemset;

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
                
                    if (cmdIndexNext == (cmdFbSizeInBeats - 1))
                    begin
                        m_axis_tlast <= 1;
                    end

                    // Check if we reached the end of the copy process
                    if (cmdIndexNext == cmdFbSizeInBeats)
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
                if (cmdIndexNext == cmdFbSizeInBeats)
                begin
                    cmdWrite <= 0;
                    cmdState <= COMMAND_WAIT_FOR_COMMAND;
                end
                cmdIndex <= cmdIndexNext;

                if (cmdMemsetXNext == confXResolution)
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

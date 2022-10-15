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
// cmdMemset: This command will initialize the memory with the color in confClearColor
//
// The fragment interface can be used to access single fragments from the framebuffer
//
// To optimize the memory utilization, this module has a internal and external pixel configuration. It automatically
// converts between the internal and external. With that it is possible to have unified interface to this module,
// (for instance 32 bit colors) but with a internal representation as RGB444 or RGBA4444 or RGB666 and so on to optimize
// the utilization.
//
// Improvements: The scrissor is not optimized. When memset is called, it will set the whole memory except
// the scissor area. A improved version will just set the scissor area.
// It is not implemented, because it requires additional logic for the index calculation (multiplier and so on).
//
// Pipelined: n/a
// Depth: 1 cycle
module FrameBuffer
#(
    // Width of the AXIS interface with the frame buffer content
    parameter STREAM_WIDTH = 16,
    
    // Number of sub pixels the interface of this module containts
    parameter NUMBER_OF_SUB_PIXELS = 4,
    // Number of sub pixels the internal memory contains (when there is a missmatch between the module interface, the module will then convert the colors)
    parameter NUMBER_OF_SUB_PIXELS_INTERNAL = 4,
    // Number of bits of each sub pixel containts
    parameter SUB_PIXEL_WIDTH = 8,
    // Internal size of a sub pixel (will be converted if there is a missmatch between SUB_PIXEL_WIDTH)
    parameter SUB_PIXEL_WIDTH_INTERNAL = 4,

    // Resolution of the frame buffer (and the frame buffer lines)
    parameter X_RESOLUTION = 1024,
    parameter Y_RESOLUTION = 600,
    parameter Y_LINE_RESOLUTION = 48,
    parameter SCREEN_POS_WIDTH = 16,

    // Size of the frame line
    localparam FRAME_SIZE_IN_PX = X_RESOLUTION * Y_LINE_RESOLUTION, // 128px * 128px. Used for abort the memset and commit phase when all pixels are transferred without processing the padding pixel

    // Size of the pixels
    localparam PIXEL_WIDTH = NUMBER_OF_SUB_PIXELS * SUB_PIXEL_WIDTH,

    // Size of the internal memory
    localparam MEM_SIZE = $clog2(FRAME_SIZE_IN_PX * (PIXEL_WIDTH / 8)), // The size of the frame buffer as bytes in power of two
    localparam ADDR_WIDTH = MEM_SIZE - 1 // Convert MEM_SIZE from 8 bit bytes to 16 bit pixels
)
(
    input   wire                            clk,
    input   wire                            reset,

    /////////////////////////
    // Configs
    /////////////////////////
    input  wire [PIXEL_WIDTH - 1 : 0]       confClearColor,
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
    input  wire [ADDR_WIDTH - 1 : 0]        fragIndexRead,
    output reg  [PIXEL_WIDTH - 1 : 0]       fragOut,
    input  wire [ADDR_WIDTH - 1 : 0]        fragIndexWrite,
    input  wire [PIXEL_WIDTH - 1 : 0]       fragIn,
    input  wire                             fragWriteEnable,
    input  wire [NUMBER_OF_SUB_PIXELS - 1 : 0]  fragMask,
    input  wire [SCREEN_POS_WIDTH - 1 : 0]  screenPosX,
    input  wire [SCREEN_POS_WIDTH - 1 : 0]  screenPosY,
    
    /////////////////////////
    // Control
    /////////////////////////

    // Cmd interface
    input  wire                             apply, // This will apply and clear the cache
    output reg                              applied,
    input  wire                             cmdCommit,
    input  wire                             cmdMemset,

    // AXI Stream master interface
    output reg                              m_axis_tvalid,
    input  wire                             m_axis_tready,
    output reg                              m_axis_tlast,
    output wire [STREAM_WIDTH - 1 : 0]      m_axis_tdata
    
);
    // Number of pixels a AXIS beat or a memory line can contain
    localparam PIXEL_PER_BEAT = STREAM_WIDTH / 16;
    localparam PIXEL_PER_BEAT_LOG2 = $clog2(PIXEL_PER_BEAT);

    // Number of AXIS beats to stream or initialize the internal memory
    localparam FRAMEBUFFER_FRAME_SIZE_IN_BEATS_TMP = (FRAME_SIZE_IN_PX / PIXEL_PER_BEAT);
    localparam [MEM_ADDR_WIDTH - 1 : 0] FRAMEBUFFER_FRAME_SIZE_IN_BEATS = FRAMEBUFFER_FRAME_SIZE_IN_BEATS_TMP[MEM_ADDR_WIDTH - 1 : 0];

    // Size constrains of the internal memory
    localparam MEM_PIXEL_WIDTH = NUMBER_OF_SUB_PIXELS_INTERNAL * SUB_PIXEL_WIDTH_INTERNAL;
    localparam MEM_MASK = PIXEL_PER_BEAT * NUMBER_OF_SUB_PIXELS_INTERNAL;
    localparam MEM_WIDTH = MEM_MASK * SUB_PIXEL_WIDTH_INTERNAL;
    localparam MEM_ADDR_WIDTH = ADDR_WIDTH - PIXEL_PER_BEAT_LOG2;

    // Used for expanding pixels to set default values
    localparam ALPHA_POS = 0;
    localparam DEFAULT_ALPHA_VAL = 0;

    `Expand(Expand, SUB_PIXEL_WIDTH_INTERNAL, SUB_PIXEL_WIDTH, NUMBER_OF_SUB_PIXELS_INTERNAL);
    `Reduce(Reduce, SUB_PIXEL_WIDTH_INTERNAL, SUB_PIXEL_WIDTH, NUMBER_OF_SUB_PIXELS_INTERNAL);

    // This is used to configure, if it is required to reduce / expand a vector or not. This is done by the offset:
    // When the offset is set to number of pixels, then the reduce / expand function will just copy the while line
    // without removing or adding somethin
    // If it is set to a lower value, then the functions will start to remove or add new pixels.
    localparam SUB_PIXEL_OFFSET = (NUMBER_OF_SUB_PIXELS == NUMBER_OF_SUB_PIXELS_INTERNAL) ? NUMBER_OF_SUB_PIXELS : ALPHA_POS; 
    `ReduceVec(ReduceVec, SUB_PIXEL_WIDTH, NUMBER_OF_SUB_PIXELS, SUB_PIXEL_OFFSET, NUMBER_OF_SUB_PIXELS, NUMBER_OF_SUB_PIXELS_INTERNAL);
    `ReduceVec(ReduceVecMask, 1, NUMBER_OF_SUB_PIXELS, SUB_PIXEL_OFFSET, NUMBER_OF_SUB_PIXELS, NUMBER_OF_SUB_PIXELS_INTERNAL);
    `ExpandVec(ExpandVec, SUB_PIXEL_WIDTH, NUMBER_OF_SUB_PIXELS_INTERNAL, SUB_PIXEL_OFFSET, NUMBER_OF_SUB_PIXELS, NUMBER_OF_SUB_PIXELS);

    // Stream states
    localparam COMMAND_WAIT_FOR_COMMAND = 0;
    localparam COMMAND_MEMCPY = 1;
    localparam COMMAND_MEMSET = 2;
    localparam COMMAND_MEMCPY_INIT = 3;

    // Scissor function
    function [0 : 0] scissorFunc;
        input           enable;
        input [SCREEN_POS_WIDTH - 1 : 0]  startX;
        input [SCREEN_POS_WIDTH - 1 : 0]  startY;
        input [SCREEN_POS_WIDTH - 1 : 0]  endX;
        input [SCREEN_POS_WIDTH - 1 : 0]  endY;
        input [SCREEN_POS_WIDTH - 1 : 0]  screenX;
        input [SCREEN_POS_WIDTH - 1 : 0]  screenY;
        begin
            scissorFunc = !enable || ((screenX >= startX) && (screenX < endX) && (screenY >= startY) && (screenY < endY));
        end
    endfunction

    // Interface to the memory when accessing a fragment. This interface has already the size required for the internal memory.
    wire [MEM_MASK - 1 : 0]         memWriteMask; 
    wire [MEM_ADDR_WIDTH - 1 : 0]   memAddrWrite;
    wire [MEM_ADDR_WIDTH - 1 : 0]   memAddrRead;
    reg  [ADDR_WIDTH - 1 : 0]       memAddrReadDelay;
    wire [MEM_WIDTH - 1 : 0]        memDataIn; 
    wire [MEM_PIXEL_WIDTH - 1 : 0]  memDataOut;
    wire [MEM_MASK - 1 : 0]         memMask = { PIXEL_PER_BEAT { ReduceVecMask(fragMask) } };
    wire                            memScissorTest;
    wire                            memWriteEnable = fragWriteEnable;
    
    // State variables for managing the memory (memset and memcpy)
    reg  [MEM_ADDR_WIDTH - 1 : 0]   cmdIndex;
    wire [MEM_ADDR_WIDTH - 1 : 0]   cmdIndexNext = cmdIndex + 1;
    reg                             cmdRunning;
    reg  [5 : 0]                    cmdState;
    reg                             cmdWrite;
    wire [MEM_MASK - 1 : 0]         cmdMask = { PIXEL_PER_BEAT { ReduceVecMask(fragMask) } };

    // Memcpy address
    wire [MEM_ADDR_WIDTH - 1 : 0]   cmdMemcpyAddr = (m_axis_tready && (cmdState == COMMAND_MEMCPY)) ? cmdIndexNext : cmdIndex;
    
    // State variables for the memset
    wire [MEM_ADDR_WIDTH - 1 : 0]   cmdMemsetAddr = cmdIndex;
    reg  [SCREEN_POS_WIDTH - 1 : 0] cmdMemsetX;
    wire [SCREEN_POS_WIDTH - 1 : 0] cmdMemsetXNext = cmdMemsetX + PIXEL_PER_BEAT;
    reg  [SCREEN_POS_WIDTH - 1 : 0] cmdMemsetY;
    wire [SCREEN_POS_WIDTH - 1 : 0] cmdMemsetYNext = cmdMemsetY - 1;
    wire [MEM_MASK - 1 : 0]         cmdMemsetScissorMask;
    wire [PIXEL_PER_BEAT - 1 : 0]   cmdMemsetScissor;
    reg                             cmdMemsetPending = 0;

    // Multiplexed interface to the internal memory. It multiplexs between the commands and the fragmend access.
    wire [MEM_WIDTH - 1 : 0]        memBusDataOut;
    wire [MEM_ADDR_WIDTH - 1 : 0]   memBusAddrWrite = (cmdRunning) ? cmdMemsetAddr                                          : memAddrWrite;
    wire [MEM_ADDR_WIDTH - 1 : 0]   memBusAddrRead  = (cmdRunning) ? cmdMemcpyAddr                                          : memAddrRead;
    wire [MEM_WIDTH - 1 : 0]        memBusDataIn    = (cmdRunning) ? { PIXEL_PER_BEAT { Reduce(ReduceVec(confClearColor)) } }   : memDataIn;
    wire                            memBusWrite     = (cmdRunning) ? cmdWrite                                               : memWriteEnable;
    wire [MEM_MASK - 1 : 0]         memBusWriteMask = (cmdRunning) ? cmdMask & cmdMemsetScissorMask                         : memWriteMask;

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
    defparam ramTile.MEM_SIZE_BYTES = MEM_SIZE;
    defparam ramTile.MEM_WIDTH = MEM_WIDTH;
    defparam ramTile.WRITE_STROBE_WIDTH = SUB_PIXEL_WIDTH_INTERNAL;

    genvar i, j;

    // Scissor check for the memset command 
    generate
        if (PIXEL_PER_BEAT == 1)
        begin 
            assign cmdMemsetScissor = scissorFunc(confEnableScissor, confScissorStartX, confScissorStartY, confScissorEndX, confScissorEndY, cmdMemsetX, cmdMemsetY);
            assign cmdMemsetScissorMask = { NUMBER_OF_SUB_PIXELS_INTERNAL { cmdMemsetScissor } };
        end
        else
        begin
            for (i = 0; i < PIXEL_PER_BEAT; i = i + 1)
            begin
                assign cmdMemsetScissor[i] = scissorFunc(confEnableScissor, confScissorStartX, confScissorStartY, confScissorEndX, confScissorEndY, cmdMemsetX + i, cmdMemsetY);
                for (j = 0; j < NUMBER_OF_SUB_PIXELS_INTERNAL; j = j + 1)
                begin
                    assign cmdMemsetScissorMask[(i * NUMBER_OF_SUB_PIXELS_INTERNAL) + j] = cmdMemsetScissor[i];
                end
            end
        end
    endgenerate

    // Fragment access including the scissor check
    generate
        assign memScissorTest = scissorFunc(confEnableScissor, confScissorStartX, confScissorStartY, confScissorEndX, confScissorEndY, screenPosX, screenPosY);
        if (PIXEL_PER_BEAT == 1)
        begin
            assign memAddrWrite = fragIndexWrite;
            assign memDataIn = Reduce(ReduceVec(fragIn));
            assign memWriteMask = memMask & { NUMBER_OF_SUB_PIXELS_INTERNAL { memScissorTest } };
            assign memAddrRead = fragIndexRead;
            assign memDataOut = memBusDataOut;
        end
        else
        begin
            for (i = 0; i < PIXEL_PER_BEAT; i = i + 1)
            begin
                for (j = 0; j < NUMBER_OF_SUB_PIXELS_INTERNAL; j = j + 1)
                begin
                    assign memWriteMask[(i * NUMBER_OF_SUB_PIXELS_INTERNAL) + j] = (fragIndexWrite[0 +: PIXEL_PER_BEAT_LOG2] == i) & memMask[j] & memScissorTest;
                end
            end
            assign memAddrWrite = fragIndexWrite[PIXEL_PER_BEAT_LOG2 +: MEM_ADDR_WIDTH];
            assign memDataIn = { PIXEL_PER_BEAT { Reduce(ReduceVec(fragIn)) } };
            assign memAddrRead = fragIndexRead[PIXEL_PER_BEAT_LOG2 +: MEM_ADDR_WIDTH];
            assign memDataOut = memBusDataOut[memAddrReadDelay[0 +: PIXEL_PER_BEAT_LOG2] * MEM_PIXEL_WIDTH +: MEM_PIXEL_WIDTH];
        end
    endgenerate

    always @(posedge clk)
    begin
        fragOut <= ExpandVec(Expand(memDataOut), DEFAULT_ALPHA_VAL);
    end

    // Conversion of the internal pixel representation the the exnternal one required for the AXIS interface
    generate
        `XXX2RGB565(XXX2RGB565, SUB_PIXEL_WIDTH, PIXEL_PER_BEAT);
        `Expand(ExpandFramebufferStream, SUB_PIXEL_WIDTH_INTERNAL, SUB_PIXEL_WIDTH, PIXEL_PER_BEAT * 3);
        if (NUMBER_OF_SUB_PIXELS_INTERNAL == 4)
        begin
            `ReduceVec(ReduceVecFramebufferStream, SUB_PIXEL_WIDTH_INTERNAL, PIXEL_PER_BEAT * NUMBER_OF_SUB_PIXELS, ALPHA_POS, NUMBER_OF_SUB_PIXELS, PIXEL_PER_BEAT * 3);
            assign m_axis_tdata = XXX2RGB565(ExpandFramebufferStream(ReduceVecFramebufferStream(memBusDataOut)));
        end
        else if (NUMBER_OF_SUB_PIXELS_INTERNAL == 3)
        begin
            assign m_axis_tdata = XXX2RGB565(ExpandFramebufferStream(memBusDataOut));
        end
        else 
        begin
            assign m_axis_tdata = memBusDataOut; // Fallback
        end
    endgenerate

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

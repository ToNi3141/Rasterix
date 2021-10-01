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

`timescale 1ns / 1ps
module DisplayControllerSpi #(
    parameter PIXEL = (128*128),
    parameter CLOCK_DIV = 2 // Divides clk to slowdown sck. 0 means nohting, 1 means half the clock, 2 only one quarter and so on.
) (
    input   wire        reset,
    input   wire        clk,

    // Serial out
    output  reg         mosi,
    output  wire        sck,

    // AXI Stream slave interface
    input  wire         s_axis_tvalid,
    output reg          s_axis_tready,
    input  wire         s_axis_tlast,
    input  wire [15:0]  s_axis_tdata,

    // External enable or disable of the serializer
    input   wire        startTransfer,
    output  reg         transferRunning
    );
`ifdef UP5K
`define RAM_MODULE SinglePortRam32k
`else
`define RAM_MODULE DualPortRam
`endif

    localparam COLOR_R_POS = 12;
    localparam COLOR_G_POS = 8;
    localparam COLOR_B_POS = 4;
    localparam COLOR_A_POS = 0;
    localparam COLOR_SUB_PIXEL_WIDTH = 4;

    // If the Colormode is 12 Bit, then the controller will send the pixel encoded in an RGB444 stream to the display.
    // If this mode is not selected, then the controller will convert the RGB444 color into an RGB565 color and will
    // then send a RGB565 stream to the display.
`ifdef STREAM_COLORMODE_12BIT
    localparam SERIALIZER_WORDWIDH = 12;
`else
    localparam SERIALIZER_WORDWIDH = 16;
`endif
   
    localparam WAIT_FOR_MEM_REQ = 0;
    localparam WAIT_FOR_BUS = 1;
    localparam READ_DATA = 2;
    localparam WAIT_FOR_FIRST_BYTE = 3;

    localparam AXIS_WAIT_FOR_START = 0;
    localparam AXIS_TRANSFER = 1;

    localparam WAIT_FOR_START = 0;
    localparam WAIT_FOR_DATA = 1;
    localparam SERIALIZE_DATA = 2;

    reg [3:0] stateBufferReq;
    reg [3:0] stateAxis;

    reg [SERIALIZER_WORDWIDH - 1:0] serializerCache; // While the data is scanned out, this buffer is used to fetch new data
    reg [SERIALIZER_WORDWIDH - 1:0] serializerCacheWorking; // This is the buffer where the data is scanned out
    reg serializerCacheEmpty; // Signal that data is copied from the fbCache to serializerCacheWorking so that a new memory request can be started
    
    reg [3:0] stateSerializer;
    reg [6:0] serCount;
    reg [$clog2(PIXEL * 2):0] pixelCount;
    reg [$clog2(PIXEL * 2):0] streamAddr;

    reg regSck;
    reg [CLOCK_DIV - 1 : 0] serClockDiv;
    reg enableSck;

    wire [15:0]             memOut;
    wire [15:0]             memIn = s_axis_tdata;
    reg                     memWr;
    wire [$clog2(PIXEL) - 1 : 0]  memAddr = (memWr) ? streamAddr : pixelCount;

    `RAM_MODULE mem (
        .clk(clk),
        .reset(reset),

        .writeData(memIn),
        .writeCs(1),
        .write(memWr), 
        .writeAddr(memAddr),
        .writeMask(4'hf),

        .readData(memOut),
        .readCs(1),
        .readAddr(memAddr)
    );
    defparam mem.MEM_SIZE_BYTES = $clog2(PIXEL * 2); // Round size to the next bigger number of two size
    defparam mem.MEM_WIDTH = 16;

    wire bufferClean = pixelCount == PIXEL;

    if (CLOCK_DIV == 0)
    begin
        assign sck = (enableSck) ? !clk : 0;
    end
    else
    begin
        assign sck = regSck;
    end
    
    always @(posedge clk)
    begin
        if (reset)
        begin
            serializerCacheEmpty <= 0;
            serializerCacheWorking <= 0;
            stateBufferReq <= WAIT_FOR_MEM_REQ;
            stateSerializer <= WAIT_FOR_START;
            stateAxis <= AXIS_WAIT_FOR_START;
            pixelCount <= PIXEL;
            mosi <= 0;
            enableSck <= 0;
            regSck <= 0;
            if (CLOCK_DIV != 0) 
            begin
                serClockDiv <= 0;    
            end
        end
        else
        begin
            if (CLOCK_DIV != 0) 
            begin
                serClockDiv <= serClockDiv + 1;   
            end
            
            // Write buffer interface
            case (stateAxis)
                AXIS_WAIT_FOR_START:
                begin
                    s_axis_tready <= 0; 
                    if (startTransfer && bufferClean)
                    begin
                        s_axis_tready <= 1;
                        streamAddr <= 0; 
                        memWr <= 1;
                        stateAxis <= AXIS_TRANSFER;
                    end
                end
                AXIS_TRANSFER:
                begin
                    if (s_axis_tvalid)
                    begin
                        streamAddr <= streamAddr + 1;
                        if (s_axis_tlast)
                        begin
                            pixelCount <= 0; // Reset pixelCount to signal, that the serializer can continue with serializing
                            memWr <= 0;
                            stateAxis <= AXIS_WAIT_FOR_START;
                        end
                    end
                end
            endcase

            // Read buffer interface
            case (stateBufferReq)
                WAIT_FOR_MEM_REQ: // Wait for mem req
                begin
                    if (serializerCacheEmpty && !bufferClean)
                    begin
                        stateBufferReq <= WAIT_FOR_FIRST_BYTE;
                    end
                end 
                WAIT_FOR_FIRST_BYTE:
                begin
                    stateBufferReq <= READ_DATA;
                end
                READ_DATA: // Get the mem data
                begin
`ifdef STREAM_COLORMODE_12BIT
                    serializerCache <= {memOut[COLOR_R_POS +: COLOR_SUB_PIXEL_WIDTH], 
                                        memOut[COLOR_G_POS +: COLOR_SUB_PIXEL_WIDTH], 
                                        memOut[COLOR_B_POS +: COLOR_SUB_PIXEL_WIDTH]};
`else
                    serializerCache <= {memOut[COLOR_R_POS +: COLOR_SUB_PIXEL_WIDTH], 1'b0, 
                                        memOut[COLOR_G_POS +: COLOR_SUB_PIXEL_WIDTH], 2'b00, 
                                        memOut[COLOR_B_POS +: COLOR_SUB_PIXEL_WIDTH], 1'b0};
`endif
                    pixelCount <= pixelCount + 1;
                    serializerCacheEmpty <= 0;
                    stateBufferReq <= WAIT_FOR_MEM_REQ;
                end
                default:
                begin
                end 
            endcase

            // Serializer
            case (stateSerializer)
                WAIT_FOR_START:
                begin
                    if (startTransfer)
                    begin
                        serializerCacheEmpty <= 1;
                        transferRunning <= 1;
                        serCount <= 0;
                        stateSerializer <= WAIT_FOR_DATA;
                    end
                end
                WAIT_FOR_DATA:
                begin
                    if ((CLOCK_DIV == 0) || (serClockDiv == 0))
                    begin
                        regSck <= 0;
                        if (!serializerCacheEmpty)
                        begin
                            enableSck <= 1;
                            serializerCacheWorking <= serializerCache;
                            serCount <= 1; // It is one because it is pushing now also one bit out
                            mosi <= serializerCache[SERIALIZER_WORDWIDH - 1];
                            
                            serializerCacheEmpty <= 1; // Start the next request
                            stateSerializer <= SERIALIZE_DATA;
                        end
                    end
                end
                SERIALIZE_DATA:
                begin
                    if (CLOCK_DIV == 0)
                    begin
                        mosi <= serializerCacheWorking[(SERIALIZER_WORDWIDH - 1) - serCount];
                        serCount <= serCount + 1;

                        if (serCount == SERIALIZER_WORDWIDH)
                        begin
                            enableSck <= 0;
                            stateSerializer <= WAIT_FOR_DATA;
                        end
                    end
                    else
                    begin
                        if (serClockDiv == 0)
                        begin
                            regSck <= 0;
                            mosi <= serializerCacheWorking[(SERIALIZER_WORDWIDH - 1) - serCount];
                            serCount <= serCount + 1;
                        end
                        else if (serClockDiv[CLOCK_DIV - 1])
                        begin
                            regSck <= 1;
                            
                            if (serCount == SERIALIZER_WORDWIDH)
                            begin
                                stateSerializer <= WAIT_FOR_DATA;
                            end
                        end
                    end
                end
                default:
                begin
                end
            endcase
        end
    end
endmodule

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

// Texture buffer which stores a whole texture. When reading a texel, the texture buffer
// reads a texel quad with the neigbored texels. Additionally it returns the sub pixel 
// coordinates which later can be used for texture filtering
// Pipelined: n/a
// Depth: 1 cycle
module TextureBuffer #(
    // Width of the write port
    parameter STREAM_WIDTH = 32,

    // Size in bytes in power of two
    parameter SIZE = 14,
    
    localparam STREAM_WIDTH_HALF = STREAM_WIDTH / 2,

    localparam SUB_PIXEL_WIDTH = 4,
    localparam PIXEL_WIDTH = SUB_PIXEL_WIDTH * 4,
    localparam SIZE_IN_WORDS = SIZE - $clog2(PIXEL_WIDTH / 8),
    localparam ADDR_WIDTH = SIZE_IN_WORDS - $clog2(STREAM_WIDTH / PIXEL_WIDTH),
    localparam ADDR_WIDTH_DIFF = SIZE_IN_WORDS - ADDR_WIDTH
)
(
    input  wire                         aclk,
    input  wire                         resetn,

    // Texture size
    input  wire [ 7 : 0]                textureSizeX,
    input  wire [ 7 : 0]                textureSizeY,

    // Texture Read
    input  wire [15 : 0]                texelX,
    input  wire [15 : 0]                texelY,
    output wire [PIXEL_WIDTH - 1 : 0]   texel00, // (0, 0), as (x, y). X and Y are switched since the address is constructed like {texelY, texelX}
    output wire [PIXEL_WIDTH - 1 : 0]   texel01, // (1, 0)
    output wire [PIXEL_WIDTH - 1 : 0]   texel10, // (0, 1)
    output wire [PIXEL_WIDTH - 1 : 0]   texel11, // (1, 1)

    // This is basically the faction of the pixel coordinate and has a range from 0.0 (0x0) to 0.999... (0xffff)
    // The integer part is not required, since the integer part only adresses the pixel and we don't care about that.
    // We just care about the coordinates within the texel quad. And if there the coordinate gets >1.0, that means, we
    // are outside of our quad which never happens.
    output reg  [15 : 0]                texelSubCoordX, 
    output reg  [15 : 0]                texelSubCoordY, 

    // Texture Write
    input  wire                         s_axis_tvalid,
    output reg                          s_axis_tready,
    input  wire                         s_axis_tlast,
    input  wire [STREAM_WIDTH - 1 : 0]  s_axis_tdata
);
`include "RegisterAndDescriptorDefines.vh"

    reg  [ADDR_WIDTH - 1 : 0]       memWriteAddr = 0;
    reg  [15 : 0]                   texelAddrForDecoding00;
    reg  [15 : 0]                   texelAddrForDecoding01;
    reg  [15 : 0]                   texelAddrForDecoding10;
    reg  [15 : 0]                   texelAddrForDecoding11;

    reg  [15 : 0]                   texelAddr00;
    reg  [15 : 0]                   texelAddr01;
    reg  [15 : 0]                   texelAddr10;
    reg  [15 : 0]                   texelAddr11;

    wire [ADDR_WIDTH - 1 : 0]       memReadAddrEven0;
    wire [ADDR_WIDTH - 1 : 0]       memReadAddrOdd0;
    wire [ADDR_WIDTH - 1 : 0]       memReadAddrEven1;
    wire [ADDR_WIDTH - 1 : 0]       memReadAddrOdd1;

    wire [STREAM_WIDTH_HALF - 1 : 0] memReadDataEven0;
    wire [STREAM_WIDTH_HALF - 1 : 0] memReadDataOdd0;
    wire [STREAM_WIDTH_HALF - 1 : 0] memReadDataEven1;
    wire [STREAM_WIDTH_HALF - 1 : 0] memReadDataOdd1;

    wire [STREAM_WIDTH_HALF - 1 : 0] tdataEvenX;
    wire [STREAM_WIDTH_HALF - 1 : 0] tdataOddX;

    reg  [15 : 0]                   texelSubCoordXReg;
    reg  [15 : 0]                   texelSubCoordYReg;

    TrueDualPortRam #(
        .MEM_SIZE_BYTES(SIZE - 1),
        .MEM_WIDTH(STREAM_WIDTH_HALF),
        .WRITE_STROBE_WIDTH(PIXEL_WIDTH)
        //.MEMORY_PRIMITIVE("distributed")
    ) texCacheEvenX
    (
        .clk(aclk),
        .reset(!resetn),

        .writeData(tdataEvenX),
        .write(s_axis_tvalid),
        .writeAddr((s_axis_tvalid) ? memWriteAddr : memReadAddrEven1),
        .writeMask({(STREAM_WIDTH_HALF / PIXEL_WIDTH){1'b1}}),
        .writeDataOut(memReadDataEven1),

        .readData(memReadDataEven0),
        .readAddr(memReadAddrEven0)
    );

    TrueDualPortRam #(
        .MEM_SIZE_BYTES(SIZE - 1),
        .MEM_WIDTH(STREAM_WIDTH_HALF),
        .WRITE_STROBE_WIDTH(PIXEL_WIDTH)
        //.MEMORY_PRIMITIVE("distributed")
    ) texCacheOddX
    (
        .clk(aclk),
        .reset(!resetn),

        .writeData(tdataOddX),
        .write(s_axis_tvalid),
        .writeAddr((s_axis_tvalid) ? memWriteAddr : memReadAddrOdd1),
        .writeMask({(STREAM_WIDTH_HALF / PIXEL_WIDTH){1'b1}}),
        .writeDataOut(memReadDataOdd1),

        .readData(memReadDataOdd0),
        .readAddr(memReadAddrOdd0)
    );

    // Check if the pixel (0, 0) from our pixel quad is on a even or uneven address. 
    // Depending in the out come we have to query the even or uneven RAM.
    assign memReadAddrEven0 = (texelAddr00[0]) ? texelAddr01[ADDR_WIDTH_DIFF +: ADDR_WIDTH] : texelAddr00[ADDR_WIDTH_DIFF +: ADDR_WIDTH];
    assign memReadAddrOdd0  = (texelAddr00[0]) ? texelAddr00[ADDR_WIDTH_DIFF +: ADDR_WIDTH] : texelAddr01[ADDR_WIDTH_DIFF +: ADDR_WIDTH];
    assign memReadAddrEven1 = (texelAddr10[0]) ? texelAddr11[ADDR_WIDTH_DIFF +: ADDR_WIDTH] : texelAddr10[ADDR_WIDTH_DIFF +: ADDR_WIDTH];
    assign memReadAddrOdd1  = (texelAddr10[0]) ? texelAddr10[ADDR_WIDTH_DIFF +: ADDR_WIDTH] : texelAddr11[ADDR_WIDTH_DIFF +: ADDR_WIDTH];

    generate
        if (STREAM_WIDTH == 32)
        begin
            assign texel00 = (texelAddrForDecoding00[0])    ? memReadDataOdd0
                                                            : memReadDataEven0;

            assign texel01 = (texelAddrForDecoding01[0])    ? memReadDataOdd0
                                                            : memReadDataEven0;

            assign texel10 = (texelAddrForDecoding10[0])    ? memReadDataOdd1
                                                            : memReadDataEven1;

            assign texel11 = (texelAddrForDecoding11[0])    ? memReadDataOdd1
                                                            : memReadDataEven1;
        end
        else 
        begin
            // Bit zero is used to check, if we have to select the RAM with the even or unevent pixel adresses (see also the multiplexing of the memReadAddr*)
            // Since bit zero is already used from the ADDR_WIDTH_DIFF to select the even or uneven ram, we can use the rest of the
            // bits to select the pixel from the vector. Therefor we start at position 1 and select one bit less from ADDR_WIDTH_DIFF to keep the selection in bound.
            assign texel00 = (texelAddrForDecoding00[0])    ? memReadDataOdd0[texelAddrForDecoding00[1 +: ADDR_WIDTH_DIFF - 1] * PIXEL_WIDTH +: PIXEL_WIDTH]
                                                            : memReadDataEven0[texelAddrForDecoding00[1 +: ADDR_WIDTH_DIFF - 1] * PIXEL_WIDTH +: PIXEL_WIDTH];

            assign texel01 = (texelAddrForDecoding01[0])    ? memReadDataOdd0[texelAddrForDecoding01[1 +: ADDR_WIDTH_DIFF - 1] * PIXEL_WIDTH +: PIXEL_WIDTH]
                                                            : memReadDataEven0[texelAddrForDecoding01[1 +: ADDR_WIDTH_DIFF - 1] * PIXEL_WIDTH +: PIXEL_WIDTH];

            assign texel10 = (texelAddrForDecoding10[0])    ? memReadDataOdd1[texelAddrForDecoding10[1 +: ADDR_WIDTH_DIFF - 1] * PIXEL_WIDTH +: PIXEL_WIDTH]
                                                            : memReadDataEven1[texelAddrForDecoding10[1 +: ADDR_WIDTH_DIFF - 1] * PIXEL_WIDTH +: PIXEL_WIDTH];

            assign texel11 = (texelAddrForDecoding11[0])    ? memReadDataOdd1[texelAddrForDecoding11[1 +: ADDR_WIDTH_DIFF - 1] * PIXEL_WIDTH +: PIXEL_WIDTH]
                                                            : memReadDataEven1[texelAddrForDecoding11[1 +: ADDR_WIDTH_DIFF - 1] * PIXEL_WIDTH +: PIXEL_WIDTH];
        end
    endgenerate

    always @*
    begin : TexAddrCalc
        reg [7 : 0] addrY0;
        reg [7 : 0] addrY1;

        reg [15 : 0] texelY0;
        reg [15 : 0] texelY1;
        reg [15 : 0] texelX0;
        reg [15 : 0] texelX1;
        
        texelY0 = texelY;
        texelX0 = texelX;

        // Select Y coordinate
        case (textureSizeY)
            8'b00000001: // 2px
            begin
                texelY1 = texelY + (1 << 14);
                addrY0 = {7'h0, texelY0[14 +: 1]};
                addrY1 = {7'h0, texelY1[14 +: 1]};
                texelSubCoordYReg = {texelY0[0 +: 14], 2'h0};
            end
            8'b00000010: // 4px
            begin
                texelY1 = texelY + (1 << 13);
                addrY0 = {6'h0, texelY0[13 +: 2]};
                addrY1 = {6'h0, texelY1[13 +: 2]};
                texelSubCoordYReg = {texelY0[0 +: 13], 3'h0};
            end
            8'b00000100: // 8px
            begin
                texelY1 = texelY + (1 << 12);
                addrY0 = {5'h0, texelY0[12 +: 3]};
                addrY1 = {5'h0, texelY1[12 +: 3]};
                texelSubCoordYReg = {texelY0[0 +: 12], 4'h0};
            end
            8'b00001000: // 16px
            begin
                texelY1 = texelY + (1 << 11);
                addrY0 = {4'h0, texelY0[11 +: 4]};
                addrY1 = {4'h0, texelY1[11 +: 4]};
                texelSubCoordYReg = {texelY0[0 +: 11], 5'h0};
            end
            8'b00010000: // 32px
            begin   
                texelY1 = texelY + (1 << 10);
                addrY0 = {3'h0, texelY0[10 +: 5]};
                addrY1 = {3'h0, texelY1[10 +: 5]};
                texelSubCoordYReg = {texelY0[0 +: 10], 6'h0};
            end
            8'b00100000: // 64px
            begin   
                texelY1 = texelY + (1 << 9);
                addrY0 = {2'h0, texelY0[9 +: 6]};
                addrY1 = {2'h0, texelY1[9 +: 6]};
                texelSubCoordYReg = {texelY0[0 +:  9], 7'h0};
            end
            8'b01000000: // 128px
            begin 
                texelY1 = texelY + (1 << 8);
                addrY0 = {1'h0, texelY0[ 8 +: 7]};
                addrY1 = {1'h0, texelY1[ 8 +: 7]};
                texelSubCoordYReg = {texelY0[0 +:  8], 8'h0};
            end
            8'b10000000: // 256px
            begin
                texelY1 = texelY + (1 << 7);
                addrY0 = {      texelY0[ 7 +: 8]};
                addrY1 = {      texelY1[ 7 +: 8]};
                texelSubCoordYReg = {texelY0[0 +:  7], 9'h0};
            end
            default: // 1px
            begin   
                texelY1 = texelY;
                addrY0 = 0;
                addrY1 = 0;
                texelSubCoordYReg = 16'h0;
            end
        endcase

        // Select X coordinate
        case (textureSizeX)
            8'b00000001: // 2px
            begin
                texelX1 = texelX + (1 << 14);
                texelAddr00 = {7'h0, addrY0, texelX0[14 +: 1]};
                texelAddr01 = {7'h0, addrY0, texelX1[14 +: 1]};
                texelAddr10 = {7'h0, addrY1, texelX0[14 +: 1]};
                texelAddr11 = {7'h0, addrY1, texelX1[14 +: 1]};
                texelSubCoordXReg = {texelX0[0 +: 14], 2'h0};
            end
            8'b00000010: // 4px
            begin
                texelX1 = texelX + (1 << 13);
                texelAddr00 = {6'h0, addrY0, texelX0[13 +: 2]};
                texelAddr01 = {6'h0, addrY0, texelX1[13 +: 2]};
                texelAddr10 = {6'h0, addrY1, texelX0[13 +: 2]};
                texelAddr11 = {6'h0, addrY1, texelX1[13 +: 2]};
                texelSubCoordXReg = {texelX0[0 +: 13], 3'h0};
            end
            8'b00000100: // 8px
            begin
                texelX1 = texelX + (1 << 12);
                texelAddr00 = {5'h0, addrY0, texelX0[12 +: 3]};
                texelAddr01 = {5'h0, addrY0, texelX1[12 +: 3]};
                texelAddr10 = {5'h0, addrY1, texelX0[12 +: 3]};
                texelAddr11 = {5'h0, addrY1, texelX1[12 +: 3]};
                texelSubCoordXReg = {texelX0[0 +: 12], 4'h0};
            end
            8'b00001000: // 16px
            begin
                texelX1 = texelX + (1 << 11);
                texelAddr00 = {4'h0, addrY0, texelX0[11 +: 4]};
                texelAddr01 = {4'h0, addrY0, texelX1[11 +: 4]};
                texelAddr10 = {4'h0, addrY1, texelX0[11 +: 4]};
                texelAddr11 = {4'h0, addrY1, texelX1[11 +: 4]};
                texelSubCoordXReg = {texelX0[0 +: 11], 5'h0};
            end
            8'b00010000: // 32px
            begin
                texelX1 = texelX + (1 << 10);
                texelAddr00 = {3'h0, addrY0, texelX0[10 +: 5]};
                texelAddr01 = {3'h0, addrY0, texelX1[10 +: 5]};
                texelAddr10 = {3'h0, addrY1, texelX0[10 +: 5]};
                texelAddr11 = {3'h0, addrY1, texelX1[10 +: 5]};
                texelSubCoordXReg = {texelX0[0 +: 10], 6'h0};
            end
            8'b00100000: // 64px
            begin
                texelX1 = texelX + (1 << 9);
                texelAddr00 = {2'h0, addrY0, texelX0[ 9 +: 6]};
                texelAddr01 = {2'h0, addrY0, texelX1[ 9 +: 6]};
                texelAddr10 = {2'h0, addrY1, texelX0[ 9 +: 6]};
                texelAddr11 = {2'h0, addrY1, texelX1[ 9 +: 6]};
                texelSubCoordXReg = {texelX0[0 +:  9], 7'h0};
            end
            8'b01000000: // 128px
            begin
                texelX1 = texelX + (1 << 8);
                texelAddr00 = {1'h0, addrY0, texelX0[ 8 +: 7]};
                texelAddr01 = {1'h0, addrY0, texelX1[ 8 +: 7]};
                texelAddr10 = {1'h0, addrY1, texelX0[ 8 +: 7]};
                texelAddr11 = {1'h0, addrY1, texelX1[ 8 +: 7]};
                texelSubCoordXReg = {texelX0[0 +:  8], 8'h0};
            end
            8'b10000000: // 256px
            begin
                texelX1 = texelX + (1 << 7);
                texelAddr00 = {      addrY0, texelX0[ 7 +: 8]};
                texelAddr01 = {      addrY0, texelX1[ 7 +: 8]};
                texelAddr10 = {      addrY1, texelX0[ 7 +: 8]};
                texelAddr11 = {      addrY1, texelX1[ 7 +: 8]};
                texelSubCoordXReg = {texelX0[0 +:  7], 9'h0};
            end
            default: // 1px
            begin
                texelX1 = texelX;
                texelAddr00 = {8'h0, addrY0};
                texelAddr01 = {8'h0, addrY0};
                texelAddr10 = {8'h0, addrY1};
                texelAddr11 = {8'h0, addrY1};
                texelSubCoordXReg = 16'h0;
            end
        endcase
    end

    always @(posedge aclk)
    begin
        // Save decoding information to select the right word from the memory read vector
        texelAddrForDecoding00 <= texelAddr00;
        texelAddrForDecoding01 <= texelAddr01;
        texelAddrForDecoding10 <= texelAddr10;
        texelAddrForDecoding11 <= texelAddr11;

        // Output the pixel intensity
        texelSubCoordX <= texelSubCoordXReg;
        texelSubCoordY <= texelSubCoordYReg;
    end

    // Memory interface to write data from the AXIS to the buffer
    always @(posedge aclk)
    begin
        if (!resetn)
        begin
            memWriteAddr <= 0;
            s_axis_tready <= 1;
        end
        else
        begin
            if (s_axis_tvalid)
            begin
                if (s_axis_tlast)
                begin
                    memWriteAddr <= 0;
                end
                else
                begin
                    memWriteAddr <= memWriteAddr + 1;
                end
            end
        end
    end

    generate 
    begin
        // Stride the incoming data. All even pixel on the X coordinate have to go to the even RAM
        // All uneven pixel on the X coordinate have to go in the odd RAM.
        genvar i;

        // Stride for the even RAM
        for (i = 0; i < STREAM_WIDTH_HALF / PIXEL_WIDTH; i = i + 1)
        begin
            localparam ii = i * (PIXEL_WIDTH * 2);
            localparam jj = i * PIXEL_WIDTH;
            assign tdataEvenX[jj +: PIXEL_WIDTH] = s_axis_tdata[ii +: PIXEL_WIDTH];
        end

        // Stride for the uneven RAM
        for (i = 0; i < STREAM_WIDTH_HALF / PIXEL_WIDTH; i = i + 1)
        begin
            localparam ii = (i * (PIXEL_WIDTH * 2)) + PIXEL_WIDTH;
            localparam jj = i * PIXEL_WIDTH;
            assign tdataOddX[jj +: PIXEL_WIDTH] = s_axis_tdata[ii +: PIXEL_WIDTH];
        end
    end
    endgenerate

endmodule 
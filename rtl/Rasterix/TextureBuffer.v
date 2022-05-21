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
// Pipelined: yes
// Depth: 2 cycle
module TextureBuffer #(
    // Width of the write port
    parameter STREAM_WIDTH = 32,

    // Size in bytes in power of two
    parameter SIZE = 14,

    // Size of the subpixel
    parameter SUB_PIXEL_WIDTH = 4,

    localparam PIXEL_WIDTH = NUMBER_OF_SUB_PIXELS * SUB_PIXEL_WIDTH,

    localparam STREAM_WIDTH_HALF = STREAM_WIDTH / 2,
    localparam NUMBER_OF_SUB_PIXELS = 4,

    localparam SIZE_IN_WORDS = SIZE - $clog2(PIXEL_WIDTH / 8),
    localparam ADDR_WIDTH = SIZE_IN_WORDS - $clog2(STREAM_WIDTH / PIXEL_WIDTH),
    localparam ADDR_WIDTH_DIFF = SIZE_IN_WORDS - ADDR_WIDTH
)
(
    input  wire                         aclk,
    input  wire                         resetn,

    // Texture size
    // textureSize * 2. 0 equals 1px. 1 equals 2px. 2 equals 4px... Only power of two are allowed.
    input  wire [ 7 : 0]                textureSizeWidth, 
    input  wire [ 7 : 0]                textureSizeHeight,

    // Texture Read
    input  wire [15 : 0]                texelS, // Q1.15
    input  wire [15 : 0]                texelT, // Q1.15
    input  wire                         clampS,
    input  wire                         clampT,
    output reg  [PIXEL_WIDTH - 1 : 0]   texel00, // (0, 0), as (s, t). s and t are switched since the address is constructed like {texelT, texelS}
    output reg  [PIXEL_WIDTH - 1 : 0]   texel01, // (1, 0)
    output reg  [PIXEL_WIDTH - 1 : 0]   texel10, // (0, 1)
    output reg  [PIXEL_WIDTH - 1 : 0]   texel11, // (1, 1)

    // This is basically the faction of the pixel coordinate and has a range from 0.0 (0x0) to 0.999... (0xffff)
    // The integer part is not required, since the integer part only adresses the pixel and we don't care about that.
    // We just care about the coordinates within the texel quad. And if there the coordinate gets >1.0, that means, we
    // are outside of our quad which never happens.
    output reg  [15 : 0]                texelSubCoordS, // Q0.16
    output reg  [15 : 0]                texelSubCoordT, // Q0.16

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

    reg  [15 : 0]                   texelT0; // Q1.15
    reg  [15 : 0]                   texelT1; // Q1.15
    reg  [15 : 0]                   texelS0; // Q1.15
    reg  [15 : 0]                   texelS1; // Q1.15

    wire [ADDR_WIDTH - 1 : 0]       memReadAddrEven0;
    wire [ADDR_WIDTH - 1 : 0]       memReadAddrOdd0;
    wire [ADDR_WIDTH - 1 : 0]       memReadAddrEven1;
    wire [ADDR_WIDTH - 1 : 0]       memReadAddrOdd1;

    wire [STREAM_WIDTH_HALF - 1 : 0] memReadDataEven0;
    wire [STREAM_WIDTH_HALF - 1 : 0] memReadDataOdd0;
    wire [STREAM_WIDTH_HALF - 1 : 0] memReadDataEven1;
    wire [STREAM_WIDTH_HALF - 1 : 0] memReadDataOdd1;

    wire [STREAM_WIDTH_HALF - 1 : 0] tdataEvenS;
    wire [STREAM_WIDTH_HALF - 1 : 0] tdataOddS;

    reg  [15 : 0]                   texelSubCoordSReg; // Q0.16
    reg  [15 : 0]                   texelSubCoordTReg; // Q0.16

    reg  [15 : 0]                   texelSubCoordSRegOut; // Q0.16
    reg  [15 : 0]                   texelSubCoordTRegOut; // Q0.16

    reg                             texelClampS;
    reg                             texelClampT;

    wire [PIXEL_WIDTH - 1 : 0]      texelSelect00;
    wire [PIXEL_WIDTH - 1 : 0]      texelSelect01;
    wire [PIXEL_WIDTH - 1 : 0]      texelSelect10;
    wire [PIXEL_WIDTH - 1 : 0]      texelSelect11;

    wire [PIXEL_WIDTH - 1 : 0]      texel00Out;
    wire [PIXEL_WIDTH - 1 : 0]      texel01Out; 
    wire [PIXEL_WIDTH - 1 : 0]      texel10Out; 
    wire [PIXEL_WIDTH - 1 : 0]      texel11Out; 

    TrueDualPortRam #(
        .MEM_SIZE_BYTES(SIZE - 1),
        .MEM_WIDTH(STREAM_WIDTH_HALF),
        .WRITE_STROBE_WIDTH(PIXEL_WIDTH)
        //.MEMORY_PRIMITIVE("distributed")
    ) texCacheEvenS
    (
        .clk(aclk),
        .reset(!resetn),

        .writeData(tdataEvenS),
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
    ) texCacheOddS
    (
        .clk(aclk),
        .reset(!resetn),

        .writeData(tdataOddS),
        .write(s_axis_tvalid),
        .writeAddr((s_axis_tvalid) ? memWriteAddr : memReadAddrOdd1),
        .writeMask({(STREAM_WIDTH_HALF / PIXEL_WIDTH){1'b1}}),
        .writeDataOut(memReadDataOdd1),

        .readData(memReadDataOdd0),
        .readAddr(memReadAddrOdd0)
    );
    
    //////////////////////////////////////////////
    // CLK 0 Build RAM adresses
    //////////////////////////////////////////////

    // Build the RAM adress of the given (s, t) coordinate and also generate the address of 
    // the texel quad
    always @*
    begin : TexAddrCalc
        reg [7 : 0] addrT0;
        reg [7 : 0] addrT1;

        texelT0 = texelT;
        texelS0 = texelS;

        // Select Y coordinate
        case (textureSizeHeight)
            8'b00000001: // 2px
            begin
                texelT1 = texelT + (1 << 14);
                addrT0 = {7'h0, texelT0[14 +: 1]};
                addrT1 = {7'h0, texelT1[14 +: 1]};
                texelSubCoordTReg = {texelT0[0 +: 14], 2'h0};
            end
            8'b00000010: // 4px
            begin
                texelT1 = texelT + (1 << 13);
                addrT0 = {6'h0, texelT0[13 +: 2]};
                addrT1 = {6'h0, texelT1[13 +: 2]};
                texelSubCoordTReg = {texelT0[0 +: 13], 3'h0};
            end
            8'b00000100: // 8px
            begin
                texelT1 = texelT + (1 << 12);
                addrT0 = {5'h0, texelT0[12 +: 3]};
                addrT1 = {5'h0, texelT1[12 +: 3]};
                texelSubCoordTReg = {texelT0[0 +: 12], 4'h0};
            end
            8'b00001000: // 16px
            begin
                texelT1 = texelT + (1 << 11);
                addrT0 = {4'h0, texelT0[11 +: 4]};
                addrT1 = {4'h0, texelT1[11 +: 4]};
                texelSubCoordTReg = {texelT0[0 +: 11], 5'h0};
            end
            8'b00010000: // 32px
            begin   
                texelT1 = texelT + (1 << 10);
                addrT0 = {3'h0, texelT0[10 +: 5]};
                addrT1 = {3'h0, texelT1[10 +: 5]};
                texelSubCoordTReg = {texelT0[0 +: 10], 6'h0};
            end
            8'b00100000: // 64px
            begin   
                texelT1 = texelT + (1 << 9);
                addrT0 = {2'h0, texelT0[9 +: 6]};
                addrT1 = {2'h0, texelT1[9 +: 6]};
                texelSubCoordTReg = {texelT0[0 +:  9], 7'h0};
            end
            8'b01000000: // 128px
            begin 
                texelT1 = texelT + (1 << 8);
                addrT0 = {1'h0, texelT0[ 8 +: 7]};
                addrT1 = {1'h0, texelT1[ 8 +: 7]};
                texelSubCoordTReg = {texelT0[0 +:  8], 8'h0};
            end
            8'b10000000: // 256px
            begin
                texelT1 = texelT + (1 << 7);
                addrT0 = {      texelT0[ 7 +: 8]};
                addrT1 = {      texelT1[ 7 +: 8]};
                texelSubCoordTReg = {texelT0[0 +:  7], 9'h0};
            end
            default: // 1px
            begin   
                texelT1 = texelT;
                addrT0 = 0;
                addrT1 = 0;
                texelSubCoordTReg = 16'h0;
            end
        endcase

        // Select X coordinate
        case (textureSizeWidth)
            8'b00000001: // 2px
            begin
                texelS1 = texelS + (1 << 14);
                texelAddr00 = {7'h0, addrT0, texelS0[14 +: 1]};
                texelAddr01 = {7'h0, addrT0, texelS1[14 +: 1]};
                texelAddr10 = {7'h0, addrT1, texelS0[14 +: 1]};
                texelAddr11 = {7'h0, addrT1, texelS1[14 +: 1]};
                texelSubCoordSReg = {texelS0[0 +: 14], 2'h0};
            end
            8'b00000010: // 4px
            begin
                texelS1 = texelS + (1 << 13);
                texelAddr00 = {6'h0, addrT0, texelS0[13 +: 2]};
                texelAddr01 = {6'h0, addrT0, texelS1[13 +: 2]};
                texelAddr10 = {6'h0, addrT1, texelS0[13 +: 2]};
                texelAddr11 = {6'h0, addrT1, texelS1[13 +: 2]};
                texelSubCoordSReg = {texelS0[0 +: 13], 3'h0};
            end
            8'b00000100: // 8px
            begin
                texelS1 = texelS + (1 << 12);
                texelAddr00 = {5'h0, addrT0, texelS0[12 +: 3]};
                texelAddr01 = {5'h0, addrT0, texelS1[12 +: 3]};
                texelAddr10 = {5'h0, addrT1, texelS0[12 +: 3]};
                texelAddr11 = {5'h0, addrT1, texelS1[12 +: 3]};
                texelSubCoordSReg = {texelS0[0 +: 12], 4'h0};
            end
            8'b00001000: // 16px
            begin
                texelS1 = texelS + (1 << 11);
                texelAddr00 = {4'h0, addrT0, texelS0[11 +: 4]};
                texelAddr01 = {4'h0, addrT0, texelS1[11 +: 4]};
                texelAddr10 = {4'h0, addrT1, texelS0[11 +: 4]};
                texelAddr11 = {4'h0, addrT1, texelS1[11 +: 4]};
                texelSubCoordSReg = {texelS0[0 +: 11], 5'h0};
            end
            8'b00010000: // 32px
            begin
                texelS1 = texelS + (1 << 10);
                texelAddr00 = {3'h0, addrT0, texelS0[10 +: 5]};
                texelAddr01 = {3'h0, addrT0, texelS1[10 +: 5]};
                texelAddr10 = {3'h0, addrT1, texelS0[10 +: 5]};
                texelAddr11 = {3'h0, addrT1, texelS1[10 +: 5]};
                texelSubCoordSReg = {texelS0[0 +: 10], 6'h0};
            end
            8'b00100000: // 64px
            begin
                texelS1 = texelS + (1 << 9);
                texelAddr00 = {2'h0, addrT0, texelS0[ 9 +: 6]};
                texelAddr01 = {2'h0, addrT0, texelS1[ 9 +: 6]};
                texelAddr10 = {2'h0, addrT1, texelS0[ 9 +: 6]};
                texelAddr11 = {2'h0, addrT1, texelS1[ 9 +: 6]};
                texelSubCoordSReg = {texelS0[0 +:  9], 7'h0};
            end
            8'b01000000: // 128px
            begin
                texelS1 = texelS + (1 << 8);
                texelAddr00 = {1'h0, addrT0, texelS0[ 8 +: 7]};
                texelAddr01 = {1'h0, addrT0, texelS1[ 8 +: 7]};
                texelAddr10 = {1'h0, addrT1, texelS0[ 8 +: 7]};
                texelAddr11 = {1'h0, addrT1, texelS1[ 8 +: 7]};
                texelSubCoordSReg = {texelS0[0 +:  8], 8'h0};
            end
            8'b10000000: // 256px
            begin
                texelS1 = texelS + (1 << 7);
                texelAddr00 = {      addrT0, texelS0[ 7 +: 8]};
                texelAddr01 = {      addrT0, texelS1[ 7 +: 8]};
                texelAddr10 = {      addrT1, texelS0[ 7 +: 8]};
                texelAddr11 = {      addrT1, texelS1[ 7 +: 8]};
                texelSubCoordSReg = {texelS0[0 +:  7], 9'h0};
            end
            default: // 1px
            begin
                texelS1 = texelS;
                texelAddr00 = {8'h0, addrT0};
                texelAddr01 = {8'h0, addrT0};
                texelAddr10 = {8'h0, addrT1};
                texelAddr11 = {8'h0, addrT1};
                texelSubCoordSReg = 16'h0;
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

        // Check if we have to clamp
        // Check if the texel coordinate is smaller than texel+1. If so, we have an overflow and we have to clamp.
        // OR, since the texel coordinate is a Q1.15 number, we need a dedicated check for the integer part. Could be, 
        // that just the fraction part overflows but not the whole variable. Therefor also check for it by checking the
        // most significant bit.
        texelClampS <= clampS && ((texelS0 > texelS1) || (!texelS0[15] && texelS1[15]));
        texelClampT <= clampT && ((texelT0 > texelT1) || (!texelT0[15] && texelT1[15]));

        // Output the pixel intensity
        texelSubCoordSRegOut <= texelSubCoordSReg;
        texelSubCoordTRegOut <= texelSubCoordTReg;
    end

    // Muxing of the RAM access to query the texels from the even and odd RAMs.
    // The odd RAM only contains the texels of the odd s coordinates. The even only the texels of an even s
    assign memReadAddrEven0 = (texelAddr00[0]) ? texelAddr01[ADDR_WIDTH_DIFF +: ADDR_WIDTH] : texelAddr00[ADDR_WIDTH_DIFF +: ADDR_WIDTH];
    assign memReadAddrOdd0  = (texelAddr00[0]) ? texelAddr00[ADDR_WIDTH_DIFF +: ADDR_WIDTH] : texelAddr01[ADDR_WIDTH_DIFF +: ADDR_WIDTH];
    assign memReadAddrEven1 = (texelAddr10[0]) ? texelAddr11[ADDR_WIDTH_DIFF +: ADDR_WIDTH] : texelAddr10[ADDR_WIDTH_DIFF +: ADDR_WIDTH];
    assign memReadAddrOdd1  = (texelAddr10[0]) ? texelAddr10[ADDR_WIDTH_DIFF +: ADDR_WIDTH] : texelAddr11[ADDR_WIDTH_DIFF +: ADDR_WIDTH];

    //////////////////////////////////////////////
    // CLK 1 Demux RAM adress and expand pixels
    //////////////////////////////////////////////

    // Demux the RAM access and access the texels in the read vector
    generate
        if (STREAM_WIDTH == 32)
        begin
            assign texelSelect00 = (texelAddrForDecoding00[0])  ? memReadDataOdd0
                                                                : memReadDataEven0;

            assign texelSelect01 = (texelAddrForDecoding01[0])  ? memReadDataOdd0
                                                                : memReadDataEven0;

            assign texelSelect10 = (texelAddrForDecoding10[0])  ? memReadDataOdd1
                                                                : memReadDataEven1;

            assign texelSelect11 = (texelAddrForDecoding11[0])  ? memReadDataOdd1
                                                                : memReadDataEven1;
        end
        else 
        begin
            // Bit zero is used to check, if we have to select the RAM with the even or unevent pixel adresses (see also the multiplexing of the memReadAddr*)
            // Since bit zero is already used from the ADDR_WIDTH_DIFF to select the even or uneven ram, we can use the rest of the
            // bits to select the pixel from the vector. Therefor we start at position 1 and select one bit less from ADDR_WIDTH_DIFF to keep the selection in bound.
            assign texelSelect00 = (texelAddrForDecoding00[0])  ? memReadDataOdd0[texelAddrForDecoding00[1 +: ADDR_WIDTH_DIFF - 1] * PIXEL_WIDTH +: PIXEL_WIDTH]
                                                                : memReadDataEven0[texelAddrForDecoding00[1 +: ADDR_WIDTH_DIFF - 1] * PIXEL_WIDTH +: PIXEL_WIDTH];

            assign texelSelect01 = (texelAddrForDecoding01[0])  ? memReadDataOdd0[texelAddrForDecoding01[1 +: ADDR_WIDTH_DIFF - 1] * PIXEL_WIDTH +: PIXEL_WIDTH]
                                                                : memReadDataEven0[texelAddrForDecoding01[1 +: ADDR_WIDTH_DIFF - 1] * PIXEL_WIDTH +: PIXEL_WIDTH];

            assign texelSelect10 = (texelAddrForDecoding10[0])  ? memReadDataOdd1[texelAddrForDecoding10[1 +: ADDR_WIDTH_DIFF - 1] * PIXEL_WIDTH +: PIXEL_WIDTH]
                                                                : memReadDataEven1[texelAddrForDecoding10[1 +: ADDR_WIDTH_DIFF - 1] * PIXEL_WIDTH +: PIXEL_WIDTH];

            assign texelSelect11 = (texelAddrForDecoding11[0])  ? memReadDataOdd1[texelAddrForDecoding11[1 +: ADDR_WIDTH_DIFF - 1] * PIXEL_WIDTH +: PIXEL_WIDTH]
                                                                : memReadDataEven1[texelAddrForDecoding11[1 +: ADDR_WIDTH_DIFF - 1] * PIXEL_WIDTH +: PIXEL_WIDTH];
        end
    endgenerate

    // Clamp texel quad
    assign texel00Out = texelSelect00;
    assign texel01Out = (texelClampS)   ? texelSelect00 
                                        : texelSelect01;
    assign texel10Out = (texelClampT)   ? texelSelect00 
                                        : texelSelect10;
    assign texel11Out = (texelClampS)   ? texel10Out
                                        : (texelClampT) ? texelSelect01 
                                                        : texelSelect11;

    always @(posedge aclk)
    begin
        // Output the texel
        texel00 <= texel00Out;
        texel01 <= texel01Out;
        texel10 <= texel10Out;
        texel11 <= texel11Out;


        // Output the sub coords
        texelSubCoordS <= texelSubCoordSRegOut;
        texelSubCoordT <= texelSubCoordTRegOut;
    end

    //////////////////////////////////////////////
    // AXIS Interface
    //////////////////////////////////////////////

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
            assign tdataEvenS[jj +: PIXEL_WIDTH] = s_axis_tdata[ii +: PIXEL_WIDTH];
        end

        // Stride for the uneven RAM
        for (i = 0; i < STREAM_WIDTH_HALF / PIXEL_WIDTH; i = i + 1)
        begin
            localparam ii = (i * (PIXEL_WIDTH * 2)) + PIXEL_WIDTH;
            localparam jj = i * PIXEL_WIDTH;
            assign tdataOddS[jj +: PIXEL_WIDTH] = s_axis_tdata[ii +: PIXEL_WIDTH];
        end
    end
    endgenerate

endmodule 
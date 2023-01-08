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

`include "PixelUtil.vh"

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

    localparam NUMBER_OF_SUB_PIXELS = 4,

    parameter PIXEL_WIDTH = 32,
    localparam SUB_PIXEL_WIDTH = PIXEL_WIDTH / NUMBER_OF_SUB_PIXELS,

    localparam PIXEL_WIDTH_INT = 16,
    localparam SUB_PIXEL_WIDTH_INT = PIXEL_WIDTH_INT / NUMBER_OF_SUB_PIXELS,

    localparam STREAM_WIDTH_HALF = STREAM_WIDTH / 2,

    localparam SIZE_IN_WORDS = SIZE - $clog2(PIXEL_WIDTH_INT / 8),
    localparam ADDR_WIDTH = SIZE_IN_WORDS - $clog2(STREAM_WIDTH / PIXEL_WIDTH_INT),
    localparam ADDR_WIDTH_DIFF = SIZE_IN_WORDS - ADDR_WIDTH,

    localparam TEX_ADDR_WIDTH = 16
)
(
    input  wire                             aclk,
    input  wire                             resetn,

    input  wire [ 3 : 0]                    confPixelFormat,

    // Texture Read
    input  wire [TEX_ADDR_WIDTH - 1 : 0]    texelAddr00,
    input  wire [TEX_ADDR_WIDTH - 1 : 0]    texelAddr01,
    input  wire [TEX_ADDR_WIDTH - 1 : 0]    texelAddr10,
    input  wire [TEX_ADDR_WIDTH - 1 : 0]    texelAddr11,
    output wire [PIXEL_WIDTH - 1 : 0]       texelOutput00,
    output wire [PIXEL_WIDTH - 1 : 0]       texelOutput01,
    output wire [PIXEL_WIDTH - 1 : 0]       texelOutput10,
    output wire [PIXEL_WIDTH - 1 : 0]       texelOutput11,

    // Texture Write
    input  wire                             s_axis_tvalid,
    output reg                              s_axis_tready,
    input  wire                             s_axis_tlast,
    input  wire [STREAM_WIDTH - 1 : 0]      s_axis_tdata
);
`include "RegisterAndDescriptorDefines.vh"
    initial 
    begin
        if (PIXEL_WIDTH != 32)
        begin
            $error("PIXEL_WIDTH must be 32. Otherwise the conversions from the internal format to the external will not work.");
            $finish;
        end
        if (PIXEL_WIDTH_INT != 16)
        begin
            $error("PIXEL_WIDTH_INT must be 16. Otherwise the conversions from the internal format to the external will not work.");
            $finish;
        end
        if (COLOR_A_POS != 0)
        begin
            $error("The COLOR_A_POS is expected to be at position 0. Otherwise the conversions from the internal format to the external will not work.");
            $finish;
        end
        if (RENDER_CONFIG_TMU_TEXTURE_PIXEL_FORMAT_SIZE != 4)
        begin
            $error("RENDER_CONFIG_TMU_TEXTURE_PIXEL_FORMAT_SIZE must be 4. If not, adapt confPixelFormat.");
            $finish;
        end
    end

    function [PIXEL_WIDTH - 1 : 0] RGBA5551TO8888; 
        input [PIXEL_WIDTH_INT - 1 : 0] pixels; 
        begin
            RGBA5551TO8888[0  +: SUB_PIXEL_WIDTH] = { 8 { pixels[0] } }; 
            RGBA5551TO8888[8  +: SUB_PIXEL_WIDTH] = { pixels[1  +: 5], pixels[2  +: 3] }; 
            RGBA5551TO8888[16 +: SUB_PIXEL_WIDTH] = { pixels[6  +: 5], pixels[7  +: 3] }; 
            RGBA5551TO8888[24 +: SUB_PIXEL_WIDTH] = { pixels[11 +: 5], pixels[12 +: 3] }; 
        end
    endfunction

    function [PIXEL_WIDTH - 1 : 0] RGB565TO8888; 
        input [PIXEL_WIDTH_INT - 1 : 0] pixels; 
        begin
            RGB565TO8888[0  +: SUB_PIXEL_WIDTH] = 8'hff; 
            RGB565TO8888[8  +: SUB_PIXEL_WIDTH] = { pixels[0  +: 5], pixels[2  +: 3] }; 
            RGB565TO8888[16 +: SUB_PIXEL_WIDTH] = { pixels[5  +: 6], pixels[9  +: 2] }; 
            RGB565TO8888[24 +: SUB_PIXEL_WIDTH] = { pixels[11 +: 5], pixels[13 +: 3] }; 
        end
    endfunction

    `Expand(Expand, SUB_PIXEL_WIDTH_INT, SUB_PIXEL_WIDTH, NUMBER_OF_SUB_PIXELS);

    reg  [ADDR_WIDTH - 1 : 0]           memWriteAddr = 0;
    reg  [TEX_ADDR_WIDTH - 1 : 0]       texelAddrForDecoding00;
    reg  [TEX_ADDR_WIDTH - 1 : 0]       texelAddrForDecoding01;
    reg  [TEX_ADDR_WIDTH - 1 : 0]       texelAddrForDecoding10;
    reg  [TEX_ADDR_WIDTH - 1 : 0]       texelAddrForDecoding11;

    wire [ADDR_WIDTH - 1 : 0]           memReadAddrEven0;
    wire [ADDR_WIDTH - 1 : 0]           memReadAddrOdd0;
    wire [ADDR_WIDTH - 1 : 0]           memReadAddrEven1;
    wire [ADDR_WIDTH - 1 : 0]           memReadAddrOdd1;

    wire [STREAM_WIDTH_HALF - 1 : 0]    memReadDataEven0;
    wire [STREAM_WIDTH_HALF - 1 : 0]    memReadDataOdd0;
    wire [STREAM_WIDTH_HALF - 1 : 0]    memReadDataEven1;
    wire [STREAM_WIDTH_HALF - 1 : 0]    memReadDataOdd1;

    wire [STREAM_WIDTH_HALF - 1 : 0]    tdataEvenS;
    wire [STREAM_WIDTH_HALF - 1 : 0]    tdataOddS;

    wire [PIXEL_WIDTH_INT - 1 : 0]      texelSelect00;
    wire [PIXEL_WIDTH_INT - 1 : 0]      texelSelect01;
    wire [PIXEL_WIDTH_INT - 1 : 0]      texelSelect10;
    wire [PIXEL_WIDTH_INT - 1 : 0]      texelSelect11;

    TrueDualPortRam #(
        .ADDR_WIDTH(ADDR_WIDTH),
        .MEM_WIDTH(STREAM_WIDTH_HALF),
        .WRITE_STROBE_WIDTH(PIXEL_WIDTH_INT)
        //.MEMORY_PRIMITIVE("distributed")
    ) texCacheEvenS
    (
        .clk(aclk),
        .reset(!resetn),

        .writeData(tdataEvenS),
        .write(s_axis_tvalid),
        .writeAddr((s_axis_tvalid) ? memWriteAddr : memReadAddrEven1),
        .writeMask({(STREAM_WIDTH_HALF / PIXEL_WIDTH_INT){1'b1}}),
        .writeDataOut(memReadDataEven1),

        .readData(memReadDataEven0),
        .readAddr(memReadAddrEven0)
    );

    TrueDualPortRam #(
        .ADDR_WIDTH(ADDR_WIDTH),
        .MEM_WIDTH(STREAM_WIDTH_HALF),
        .WRITE_STROBE_WIDTH(PIXEL_WIDTH_INT)
        //.MEMORY_PRIMITIVE("distributed")
    ) texCacheOddS
    (
        .clk(aclk),
        .reset(!resetn),

        .writeData(tdataOddS),
        .write(s_axis_tvalid),
        .writeAddr((s_axis_tvalid) ? memWriteAddr : memReadAddrOdd1),
        .writeMask({(STREAM_WIDTH_HALF / PIXEL_WIDTH_INT){1'b1}}),
        .writeDataOut(memReadDataOdd1),

        .readData(memReadDataOdd0),
        .readAddr(memReadAddrOdd0)
    );
    
    //////////////////////////////////////////////
    //  Build RAM adresses
    //////////////////////////////////////////////

    // Muxing of the RAM access to query the texels from the even and odd RAMs.
    // The odd RAM only contains the texels of the odd s coordinates. The even only the texels of an even s
    assign memReadAddrEven0 = (texelAddr00[0]) ? texelAddr01[ADDR_WIDTH_DIFF +: ADDR_WIDTH] : texelAddr00[ADDR_WIDTH_DIFF +: ADDR_WIDTH];
    assign memReadAddrOdd0  = (texelAddr00[0]) ? texelAddr00[ADDR_WIDTH_DIFF +: ADDR_WIDTH] : texelAddr01[ADDR_WIDTH_DIFF +: ADDR_WIDTH];
    assign memReadAddrEven1 = (texelAddr10[0]) ? texelAddr11[ADDR_WIDTH_DIFF +: ADDR_WIDTH] : texelAddr10[ADDR_WIDTH_DIFF +: ADDR_WIDTH];
    assign memReadAddrOdd1  = (texelAddr10[0]) ? texelAddr10[ADDR_WIDTH_DIFF +: ADDR_WIDTH] : texelAddr11[ADDR_WIDTH_DIFF +: ADDR_WIDTH];

    always @(posedge aclk)
    begin
        // Save decoding information to select the right word from the memory read vector
        texelAddrForDecoding00 <= texelAddr00;
        texelAddrForDecoding01 <= texelAddr01;
        texelAddrForDecoding10 <= texelAddr10;
        texelAddrForDecoding11 <= texelAddr11;
    end

    //////////////////////////////////////////////
    // Demux RAM adress and expand pixels
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
            assign texelSelect00 = (texelAddrForDecoding00[0])  ? memReadDataOdd0[texelAddrForDecoding00[1 +: ADDR_WIDTH_DIFF - 1] * PIXEL_WIDTH_INT +: PIXEL_WIDTH_INT]
                                                                : memReadDataEven0[texelAddrForDecoding00[1 +: ADDR_WIDTH_DIFF - 1] * PIXEL_WIDTH_INT +: PIXEL_WIDTH_INT];

            assign texelSelect01 = (texelAddrForDecoding01[0])  ? memReadDataOdd0[texelAddrForDecoding01[1 +: ADDR_WIDTH_DIFF - 1] * PIXEL_WIDTH_INT +: PIXEL_WIDTH_INT]
                                                                : memReadDataEven0[texelAddrForDecoding01[1 +: ADDR_WIDTH_DIFF - 1] * PIXEL_WIDTH_INT +: PIXEL_WIDTH_INT];

            assign texelSelect10 = (texelAddrForDecoding10[0])  ? memReadDataOdd1[texelAddrForDecoding10[1 +: ADDR_WIDTH_DIFF - 1] * PIXEL_WIDTH_INT +: PIXEL_WIDTH_INT]
                                                                : memReadDataEven1[texelAddrForDecoding10[1 +: ADDR_WIDTH_DIFF - 1] * PIXEL_WIDTH_INT +: PIXEL_WIDTH_INT];

            assign texelSelect11 = (texelAddrForDecoding11[0])  ? memReadDataOdd1[texelAddrForDecoding11[1 +: ADDR_WIDTH_DIFF - 1] * PIXEL_WIDTH_INT +: PIXEL_WIDTH_INT]
                                                                : memReadDataEven1[texelAddrForDecoding11[1 +: ADDR_WIDTH_DIFF - 1] * PIXEL_WIDTH_INT +: PIXEL_WIDTH_INT];
        end
    endgenerate

    assign texelOutput00 = (confPixelFormat == RENDER_CONFIG_TMU_TEXTURE_PIXEL_FORMAT_RGB565)  ? RGB565TO8888(texelSelect00)
                                                                                    : (confPixelFormat == RENDER_CONFIG_TMU_TEXTURE_PIXEL_FORMAT_RGBA5551) ? RGBA5551TO8888(texelSelect00) 
                                                                                                                                                : Expand(texelSelect00);
    assign texelOutput01 = (confPixelFormat == RENDER_CONFIG_TMU_TEXTURE_PIXEL_FORMAT_RGB565)  ? RGB565TO8888(texelSelect01)
                                                                                    : (confPixelFormat == RENDER_CONFIG_TMU_TEXTURE_PIXEL_FORMAT_RGBA5551) ? RGBA5551TO8888(texelSelect01) 
                                                                                                                                                : Expand(texelSelect01);
    assign texelOutput10 = (confPixelFormat == RENDER_CONFIG_TMU_TEXTURE_PIXEL_FORMAT_RGB565)  ? RGB565TO8888(texelSelect10)
                                                                                    : (confPixelFormat == RENDER_CONFIG_TMU_TEXTURE_PIXEL_FORMAT_RGBA5551) ? RGBA5551TO8888(texelSelect10) 
                                                                                                                                                : Expand(texelSelect10);
    assign texelOutput11 = (confPixelFormat == RENDER_CONFIG_TMU_TEXTURE_PIXEL_FORMAT_RGB565)  ? RGB565TO8888(texelSelect11)
                                                                                    : (confPixelFormat == RENDER_CONFIG_TMU_TEXTURE_PIXEL_FORMAT_RGBA5551) ? RGBA5551TO8888(texelSelect11) 
                                                                                                                                                : Expand(texelSelect11);

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
        for (i = 0; i < STREAM_WIDTH_HALF / PIXEL_WIDTH_INT; i = i + 1)
        begin
            localparam ii = i * (PIXEL_WIDTH_INT * 2);
            localparam jj = i * PIXEL_WIDTH_INT;
            assign tdataEvenS[jj +: PIXEL_WIDTH_INT] = s_axis_tdata[ii +: PIXEL_WIDTH_INT];
        end

        // Stride for the uneven RAM
        for (i = 0; i < STREAM_WIDTH_HALF / PIXEL_WIDTH_INT; i = i + 1)
        begin
            localparam ii = (i * (PIXEL_WIDTH_INT * 2)) + PIXEL_WIDTH_INT;
            localparam jj = i * PIXEL_WIDTH_INT;
            assign tdataOddS[jj +: PIXEL_WIDTH_INT] = s_axis_tdata[ii +: PIXEL_WIDTH_INT];
        end
    end
    endgenerate

endmodule 
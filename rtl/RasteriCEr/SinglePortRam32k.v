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

// RAM Wrapper for the iCE40 Single Port RAM. Note here that the read and write ports cannot be used independenly!
// Only a read or write at a time can be executed. If the write signal is asserted, then it will only use the write address!

module SinglePortRam32k #(
    parameter MEM_SIZE_BYTES = 14, // Dummy. It is just here to be compatible to the DualPortRam module
    parameter MEM_WIDTH = 16, // Dummy. It is just here to be compatible to the DualPortRam module
    parameter WRITE_STROBE_WIDTH = 4, // Dummy. It is just here to be compatible to the DualPortRam module
    localparam MEM_SIZE = MEM_SIZE_BYTES - $clog2(MEM_WIDTH / 8)
) (
    input  wire                         clk,
    input  wire                         reset,

    // Write interface
    input  wire [MEM_WIDTH - 1 : 0]     writeData,
    input  wire                         writeCs,
    input  wire                         write,
    input  wire [MEM_SIZE - 1 : 0]      writeAddr,
    input  wire [ 3 : 0]                writeMask,

    // Read interface
    output wire [MEM_WIDTH - 1 : 0]     readData,
    input  wire                         readCs,
    input  wire [MEM_SIZE - 1 : 0]      readAddr
);
`ifdef UP5K
    SB_SPRAM256KA ram0
    (
        .ADDRESS((write) ? writeAddr[13:0] : readAddr[13:0]),
        .DATAIN(writeData[15:0]),
        .MASKWREN(writeMask),
        .WREN(write),
        .CHIPSELECT(writeCs | readCs),
        .CLOCK(clk),
        .STANDBY(1'b0),
        .SLEEP(1'b0),
        .POWEROFF(1'b1),
        .DATAOUT(readData)
    );
`else
    reg [15:0] mem [16383:0];
    reg [15:0] memOut;
    assign readData = memOut;
    genvar i;
    always @(posedge clk)
    begin
    if (writeCs | readCs)
    begin
        if (write)
        begin
            for (i = 0; i < 4; i = i + 1)
            begin
                if (writeMask[i])
                begin
                    mem[(write) ? writeAddr[13:0] : readAddr[13:0]][i * WRITE_STROBE_WIDTH +: WRITE_STROBE_WIDTH] <= writeData[i * WRITE_STROBE_WIDTH +: WRITE_STROBE_WIDTH];
                end
            end 
        end
        memOut <= mem[(write) ? writeAddr[13:0] : readAddr[13:0]];
    end
    end
`endif
endmodule

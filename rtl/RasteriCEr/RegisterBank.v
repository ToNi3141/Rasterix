// RasteriCEr
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

module RegisterBank
#(
    // Bank size in multiple of 32 bit
    parameter BANK_SIZE = 8,

    // The bit width of the command interface. Allowed values: 32, 64, 128, 256
    parameter CMD_STREAM_WIDTH = 32,

    // Bank register size
    localparam BANK_REG_SIZE = 32
)
(
    input wire                              aclk,
    input wire                              resetn,

    // Register content stream
    input  wire                             s_axis_tvalid,
    output reg                              s_axis_tready,
    input  wire                             s_axis_tlast,
    input  wire [CMD_STREAM_WIDTH - 1 : 0]  s_axis_tdata,

    // Register bank
    output wire [(BANK_SIZE * BANK_REG_SIZE) - 1 : 0] registers
);
    localparam INDEX_WIDTH = $clog2(BANK_SIZE);
    localparam REGISTERS_PER_STREAM_BEAT = CMD_STREAM_WIDTH / BANK_REG_SIZE;

    reg [BANK_REG_SIZE - 1 : 0] registerMem [0 : BANK_SIZE - 1];
    reg [INDEX_WIDTH - 1 : 0] registerIndex;

    always @(posedge aclk)
    begin
        if (!resetn)
        begin
            s_axis_tready <= 1;
        end
        else
        begin : Copy
            integer i;
            if (s_axis_tvalid)
            begin
                for (i = 0; i < REGISTERS_PER_STREAM_BEAT; i = i + 1)
                begin
                    registerMem[registerIndex + i[0 +: INDEX_WIDTH]] <= s_axis_tdata[BANK_REG_SIZE * i +: BANK_REG_SIZE];
                end
                registerIndex <= registerIndex + REGISTERS_PER_STREAM_BEAT[0 +: INDEX_WIDTH];

                if (s_axis_tlast)
                begin
                    registerIndex <= 0;
                end
            end
        end
    end

    generate 
        genvar i;
        for(i = 0; i < BANK_SIZE; i = i + 1)
        begin
            assign registers[i * BANK_REG_SIZE +: BANK_REG_SIZE] = registerMem[i]; 
        end
    endgenerate
endmodule
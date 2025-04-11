// RRX
// https://github.com/ToNi3141/RRX
// Copyright (c) 2024 ToNi3141

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

// Physical interface for the AsyncSramController
module AsyncSramPhy
#(
    parameter ADDR_WIDTH = 19,
    parameter MEM_WIDTH = 8
)
(
    input  wire [ADDR_WIDTH - 1 : 0]    s_mem_addr,
    input  wire [MEM_WIDTH - 1 : 0]     s_mem_data_i,
    output wire [MEM_WIDTH - 1 : 0]     s_mem_data_o,
    input  wire [MEM_WIDTH - 1 : 0]     s_mem_data_t,
    input  wire                         s_mem_noe,
    input  wire                         s_mem_nce,
    input  wire                         s_mem_nwe,

    output wire [ADDR_WIDTH - 1 : 0]    m_mem_addr,
    inout  wire [MEM_WIDTH - 1 : 0]     m_mem_data,
    output wire                         m_mem_noe,
    output wire                         m_mem_nce,
    output wire                         m_mem_nwe
);

    assign m_mem_addr = s_mem_addr;
    assign m_mem_noe = s_mem_noe;
    assign m_mem_nce = s_mem_nce;
    assign m_mem_nwe = s_mem_nwe;

    genvar i;
    generate
        for (i = 0; i < MEM_WIDTH; i = i + 1)
        begin
            IOBUF #(
                .DRIVE(12), // Specify the output drive strength
                .IBUF_LOW_PWR("FALSE"),  // Low Power - "TRUE", High Performance = "FALSE"
                .IOSTANDARD("DEFAULT"), // Specify the I/O standard
                .SLEW("FAST") // Specify the output slew rate
            ) IOBUF_inst (
                .O(s_mem_data_o[i]),     // Buffer output
                .IO(m_mem_data[i]),   // Buffer inout port (connect directly to top-level port)
                .I(s_mem_data_i[i]),     // Buffer input
                .T(s_mem_data_t[i])      // 3-state enable input, high=input, low=output
            );
        end
    endgenerate
endmodule
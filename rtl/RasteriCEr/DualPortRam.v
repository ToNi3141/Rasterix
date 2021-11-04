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

module DualPortRam #(
    parameter MEM_SIZE_BYTES = 14, // The memory size in power of two bytes
    parameter MEM_WIDTH = 16, // Memory width in bits
    parameter WRITE_STROBE_WIDTH = 4, // Write strobe in bits
    parameter MEMORY_PRIMITIVE = "block",
    localparam MEM_SIZE = MEM_SIZE_BYTES - ($clog2(MEM_WIDTH / 8)),
    localparam WRITE_MASK_SIZE = MEM_WIDTH / WRITE_STROBE_WIDTH
)
(
    input  wire                             clk,
    input  wire                             reset,

    // Write interface
    input  wire [MEM_WIDTH - 1 : 0]         writeData,
    input  wire                             writeCs,
    input  wire                             write,
    input  wire [MEM_SIZE - 1 : 0]          writeAddr,
    input  wire [WRITE_MASK_SIZE - 1 : 0]   writeMask, 

    // Read interface
    output wire [MEM_WIDTH - 1 : 0]         readData,
    input  wire                             readCs,
    input  wire [MEM_SIZE - 1 : 0]          readAddr
);
    
`ifndef UNITTEST
    genvar i;
    generate 
        for (i = 0; i < WRITE_MASK_SIZE; i = i + 1)
        begin
            // Xilinx macro to instantiate simple dual port memory
            xpm_memory_sdpram #(
                .ADDR_WIDTH_A(MEM_SIZE),
                .ADDR_WIDTH_B(MEM_SIZE),
                .AUTO_SLEEP_TIME(0),
                .BYTE_WRITE_WIDTH_A(WRITE_STROBE_WIDTH),
                .CASCADE_HEIGHT(0),
                .CLOCKING_MODE("common_clock"),
                .ECC_MODE("no_ecc"),
                .MEMORY_INIT_FILE("none"),
                .MEMORY_INIT_PARAM("0"),
                .MEMORY_OPTIMIZATION("true"),
                .MEMORY_PRIMITIVE(MEMORY_PRIMITIVE),
                .MEMORY_SIZE((2**MEM_SIZE) * WRITE_STROBE_WIDTH),
                .MESSAGE_CONTROL(0),
                .READ_DATA_WIDTH_B(WRITE_STROBE_WIDTH),
                .READ_LATENCY_B(1), 
                .READ_RESET_VALUE_B("0"),
                .RST_MODE_A("SYNC"),
                .RST_MODE_B("SYNC"),
                .SIM_ASSERT_CHK(0), 
                .USE_EMBEDDED_CONSTRAINT(0),  
                .USE_MEM_INIT(0),             
                .WAKEUP_TIME("disable_sleep"),
                .WRITE_DATA_WIDTH_A(WRITE_STROBE_WIDTH),
                .WRITE_MODE_B("read_first")
            )
            xpm_memory_sdpram_inst (
                .dbiterrb(),             // 1-bit output: Status signal to indicate double bit error occurrence
                                                // on the data output of port B.

                .doutb(readData[((i + 1) * WRITE_STROBE_WIDTH) - 1 : i * WRITE_STROBE_WIDTH]), // READ_DATA_WIDTH_B-bit output: Data output for port B read operations.
                .sbiterrb(),             // 1-bit output: Status signal to indicate single bit error occurrence
                                                // on the data output of port B.

                .addra(writeAddr),                   // ADDR_WIDTH_A-bit input: Address for port A write operations.
                .addrb(readAddr),                   // ADDR_WIDTH_B-bit input: Address for port B read operations.
                .clka(clk),                     // 1-bit input: Clock signal for port A. Also clocks port B when
                                                // parameter CLOCKING_MODE is "common_clock".

                .clkb(clk),                     // 1-bit input: Clock signal for port B when parameter CLOCKING_MODE is
                                                // "independent_clock". Unused when parameter CLOCKING_MODE is
                                                // "common_clock".

                .dina(writeData[((i + 1) * WRITE_STROBE_WIDTH) - 1 : i * WRITE_STROBE_WIDTH]), // WRITE_DATA_WIDTH_A-bit input: Data input for port A write operations.
                .ena(1),                       // 1-bit input: Memory enable signal for port A. Must be high on clock
                                                // cycles when write operations are initiated. Pipelined internally.

                .enb(1),                       // 1-bit input: Memory enable signal for port B. Must be high on clock
                                                // cycles when read operations are initiated. Pipelined internally.

                .injectdbiterra(0), // 1-bit input: Controls double bit error injection on input data when
                                                // ECC enabled (Error injection capability is not available in
                                                // "decode_only" mode).

                .injectsbiterra(0), // 1-bit input: Controls single bit error injection on input data when
                                                // ECC enabled (Error injection capability is not available in
                                                // "decode_only" mode).

                .regceb(0),                 // 1-bit input: Clock Enable for the last register stage on the output
                                                // data path.

                .rstb(reset),                     // 1-bit input: Reset signal for the final port B output register stage.
                                                // Synchronously resets output port doutb to the value specified by
                                                // parameter READ_RESET_VALUE_B.

                .sleep(0),                   // 1-bit input: sleep signal to enable the dynamic power saving feature.
                .wea(writeMask[i] & write)                        // WRITE_DATA_WIDTH_A/BYTE_WRITE_WIDTH_A-bit input: Write enable vector
                                                // for port A input data port dina. 1 bit wide when word-wide writes are
                                                // used. In byte-wide write configurations, each bit controls the
                                                // writing one byte of dina to address addra. For example, to
                                                // synchronously write only bits [15-8] of dina when WRITE_DATA_WIDTH_A
                                                // is 32, wea would be 4'b0010.
            );
        end
    endgenerate
`else
    reg [MEM_WIDTH  - 1 : 0]    mem [(1 << MEM_SIZE) - 1 : 0];
    reg [MEM_WIDTH - 1 : 0]     memOut;
    assign readData = memOut;
    integer i;

    always @(posedge clk)
    begin
        if (writeCs)
        begin
            for (i = 0; i < WRITE_MASK_SIZE; i = i + 1)
            begin
                if (write)
                begin
                    if (writeMask[i])
                        mem[writeAddr][(i * WRITE_STROBE_WIDTH) +: WRITE_STROBE_WIDTH] <= writeData[(i * WRITE_STROBE_WIDTH) +: WRITE_STROBE_WIDTH];
                end
            end
        end
        if (readCs)
        begin
            memOut <= mem[readAddr];
        end
    end
`endif
endmodule

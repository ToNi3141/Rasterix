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

module top
(
    // clk
    input  wire clk100,
    // Reset
    input  wire resetn_in,

    // Serial stream
    input  wire serial_mosi,
    // output wire serial_miso, not used right now
    input  wire serial_sck,
    input  wire serial_cs,
    output wire serial_cts,
    input  wire serial_tft_mux,

    // Display stream
    output wire display_mosi,
    output wire display_sck,
    output wire display_cs,
    output wire display_dc,
    output wire display_reset, // Uses resetn_in
    input  wire tft_cs,
    input  wire tft_dc

    // Debug
    // output wire dbgStreamingState0,
    // output wire dbgStreamingState1,
    // output wire dbgStreamingState2,
    // output wire dbgStreamingState3
);
`define X_RESOLUTION 240
`define Y_RESOLUTION 320
`define Y_LINE_RESOLUTION 64
    parameter X_RESOLUTION = `X_RESOLUTION;
    parameter Y_RESOLUTION = `Y_RESOLUTION;
    parameter Y_LINE_RESOLUTION = `Y_LINE_RESOLUTION;
    parameter CMD_STREAM_WIDTH = 32;
    parameter TEXTURE_BUFFER_SIZE = 15;

    wire clk;
    reg resetn;

    wire display_mosi_out;
    wire display_sck_out;
    reg display_mux_reg;

    ///////////////////////////
    // SPI Bypass
    ///////////////////////////
    always @(posedge clk)
    begin
        display_mux_reg <= serial_tft_mux;
    end
    assign display_mosi = (display_mux_reg) ? serial_mosi : display_mosi_out;
    assign display_sck = (display_mux_reg) ? serial_sck : display_sck_out;
    assign display_cs = (display_mux_reg) ? tft_cs : 1'b0;
    assign display_dc = (display_mux_reg) ? tft_dc : 1'b1;
    assign display_reset = resetn;

    ///////////////////////////
    // Clock Instantiation
    ///////////////////////////
    wire pllLocked;
    clk_wiz_0 inst (clk, 0, pllLocked, clk100); // Convert to a 90MHz clock
    // assign clk = clk100; // This can work, but probably this hurts some timing requirements
    // assign pllLocked = 0;

    ///////////////////////////
    // Regs and wires
    ///////////////////////////
    // LCD
    wire        transferRunning;
    wire        m_framebuffer_axis_tvalid;
    wire        m_framebuffer_axis_tready;
    wire        m_framebuffer_axis_tlast;
    wire [15:0] m_framebuffer_axis_tdata;

    // AXI Stream command interface
    wire        s_cmd_axis_tvalid;
    wire        s_cmd_axis_tready;
    wire        s_cmd_axis_tlast;
    wire [CMD_STREAM_WIDTH - 1 : 0] s_cmd_axis_tdata;

    DisplayControllerSpi lcd(
        .reset(!resetn),
        .clk(clk),

        .mosi(display_mosi_out),
        .sck(display_sck_out),

        .s_axis_tvalid(m_framebuffer_axis_tvalid),
        .s_axis_tready(m_framebuffer_axis_tready),
        .s_axis_tlast(m_framebuffer_axis_tlast),
        .s_axis_tdata(m_framebuffer_axis_tdata),

        .startTransfer(!display_mux_reg),
        .transferRunning(transferRunning)
    );
    defparam lcd.PIXEL = X_RESOLUTION * Y_LINE_RESOLUTION;
    defparam lcd.CLOCK_DIV = 2; // Caution, normally the SPI displays work with a maximum frequency of 15.5 MHz. 
                                // A CLOCK_DIV of 2 means that the 90MHz clock is divided by four which results in a frequency of 22.5 MHz.

    RasteriCEr #(.X_RESOLUTION(X_RESOLUTION),
                 .Y_RESOLUTION(Y_RESOLUTION),
                 .Y_LINE_RESOLUTION(Y_LINE_RESOLUTION),
                 .CMD_STREAM_WIDTH(CMD_STREAM_WIDTH),
                 .TEXTURE_BUFFER_SIZE(TEXTURE_BUFFER_SIZE)) rasteriCEr(

        .aclk(clk),
        .resetn(resetn),
        
        // AXI Stream command interface
        .s_cmd_axis_tvalid(s_cmd_axis_tvalid),
        .s_cmd_axis_tready(s_cmd_axis_tready),
        .s_cmd_axis_tlast(s_cmd_axis_tlast),
        .s_cmd_axis_tdata(s_cmd_axis_tdata),

        // AXI Stream framebuffer
        .m_framebuffer_axis_tvalid(m_framebuffer_axis_tvalid),
        .m_framebuffer_axis_tready(m_framebuffer_axis_tready),
        .m_framebuffer_axis_tlast(m_framebuffer_axis_tlast),
        .m_framebuffer_axis_tdata(m_framebuffer_axis_tdata),

        // Debug
        .dbgStreamState(),
        // .dbgStreamState({dbgStreamingState3, dbgStreamingState2, dbgStreamingState1, dbgStreamingState0}),
        .dbgRasterizerRunning()
    );

    Serial2AXIS serial2axis(
        .aclk(clk),
        .resetn(resetn),

        .serial_mosi(serial_mosi),
        // .serial_miso(serial_miso),
        .serial_miso(),
        .serial_sck(serial_sck),
        .serial_cs(serial_cs),
        .serial_cts(serial_cts),
        
        .m_axis_tvalid(s_cmd_axis_tvalid),
        .m_axis_tready(s_cmd_axis_tready),
        .m_axis_tlast(s_cmd_axis_tlast),
        .m_axis_tdata(s_cmd_axis_tdata)
    );

    always @(posedge clk)
    begin
        if (resetn_in & pllLocked)
        begin
            resetn <= 1;
        end
        else
        begin
            resetn <= 0;
        end
    end
endmodule
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

module top #(
    parameter CMD_STREAM_WIDTH = 64,
    parameter FRAMEBUFFER_STREAM_WIDTH = 64
)
(
    input  wire         aclk,
    input  wire         resetn,

    // AXI Stream command interface
    input  wire         s_cmd_axis_tvalid,
    output wire         s_cmd_axis_tready,
    input  wire         s_cmd_axis_tlast,
    input  wire [CMD_STREAM_WIDTH - 1 : 0]  s_cmd_axis_tdata,

    // AXI Stream framebuffer
    output wire         m_framebuffer_axis_tvalid,
    input  wire         m_framebuffer_axis_tready,
    output wire         m_framebuffer_axis_tlast,
    output wire [FRAMEBUFFER_STREAM_WIDTH - 1 : 0]  m_framebuffer_axis_tdata
);
    parameter X_RESOLUTION = `X_RESOLUTION;
    parameter Y_RESOLUTION = `Y_RESOLUTION;
    parameter Y_LINE_RESOLUTION = `Y_LINE_RESOLUTION;
    parameter TEXTURE_BUFFER_SIZE = 15;

    Rasterix #(.X_RESOLUTION(X_RESOLUTION),
                 .Y_RESOLUTION(Y_RESOLUTION),
                 .Y_LINE_RESOLUTION(Y_LINE_RESOLUTION),
                 .CMD_STREAM_WIDTH(CMD_STREAM_WIDTH),
                 .FRAMEBUFFER_STREAM_WIDTH(FRAMEBUFFER_STREAM_WIDTH),
                 .TEXTURE_BUFFER_SIZE(TEXTURE_BUFFER_SIZE)) rasteriCEr(
        .aclk(aclk),
        .resetn(resetn),
        
        .s_cmd_axis_tvalid(s_cmd_axis_tvalid),
        .s_cmd_axis_tready(s_cmd_axis_tready),
        .s_cmd_axis_tlast(s_cmd_axis_tlast),
        .s_cmd_axis_tdata(s_cmd_axis_tdata),

        .m_framebuffer_axis_tvalid(m_framebuffer_axis_tvalid),
        .m_framebuffer_axis_tready(m_framebuffer_axis_tready),
        .m_framebuffer_axis_tlast(m_framebuffer_axis_tlast),
        .m_framebuffer_axis_tdata(m_framebuffer_axis_tdata),

        // Debug
        .dbgStreamState(),
        .dbgRasterizerRunning()
    );
endmodule
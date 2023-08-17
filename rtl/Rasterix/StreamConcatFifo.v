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

// Concatenates four streams into one stream.
// Note that the output stream is not registered. stream_out_tready will have direct impact
// unlike with AXI Streams. Advantage: Skid buffer is not required when handling stream_out_tready.
module StreamConcatFifo
#(
    parameter FIFO_DEPTH_POW2 = 5,
    parameter STREAM0_WIDTH = 8,
    parameter STREAM1_WIDTH = 8,
    parameter STREAM2_WIDTH = 8,
    parameter STREAM3_WIDTH = 8,

    localparam STREAMO_WIDTH = STREAM0_WIDTH + STREAM1_WIDTH + STREAM2_WIDTH + STREAM3_WIDTH,
    localparam NUMBER_OF_STREAMS = 4
)
(
    input  wire                         aclk,
    input  wire                         resetn,

    input  wire                         stream0_tvalid,
    input  wire [STREAM0_WIDTH - 1 : 0] stream0_tdata,
    output wire                         stream0_tready,

    input  wire                         stream1_tvalid,
    input  wire [STREAM1_WIDTH - 1 : 0] stream1_tdata,
    output wire                         stream1_tready,

    input  wire                         stream2_tvalid,
    input  wire [STREAM2_WIDTH - 1 : 0] stream2_tdata,
    output wire                         stream2_tready,

    input  wire                         stream3_tvalid,
    input  wire [STREAM3_WIDTH - 1 : 0] stream3_tdata,
    output wire                         stream3_tready,

    output wire                         stream_out_tvalid,
    output wire [STREAMO_WIDTH - 1 : 0] stream_out_tdata,
    input  wire                         stream_out_tready
);
    wire                            stream_full [NUMBER_OF_STREAMS - 1 : 0];
    wire                            stream_empty [NUMBER_OF_STREAMS - 1 : 0];
    wire [STREAM0_WIDTH - 1 : 0]    stream0_data;
    wire [STREAM1_WIDTH - 1 : 0]    stream1_data;
    wire [STREAM2_WIDTH - 1 : 0]    stream2_data;
    wire [STREAM3_WIDTH - 1 : 0]    stream3_data;
    wire                            complete_transfer_available = !(stream_empty[0] | stream_empty[1] | stream_empty[2] | stream_empty[3]);
    wire                            stream_out_read = stream_out_tready && complete_transfer_available;

    assign stream0_tready = !stream_full[0];
    assign stream1_tready = !stream_full[1];
    assign stream2_tready = !stream_full[2];
    assign stream3_tready = !stream_full[3];

    sfifo #(
        .BW(STREAM0_WIDTH),
        .LGFLEN(FIFO_DEPTH_POW2),
        .OPT_ASYNC_READ(1),
        .OPT_WRITE_ON_FULL(0),
        .OPT_READ_ON_EMPTY(0)
    ) stream0_fifo (
        .i_clk(aclk),
        .i_reset(!resetn),

        .i_wr(stream0_tvalid),
        .i_data(stream0_tdata),
        .o_full(stream_full[0]),
        .o_fill(),

        .i_rd(stream_out_read),
        .o_data(stream0_data),
        .o_empty(stream_empty[0])
    );

    sfifo #(
        .BW(STREAM1_WIDTH),
        .LGFLEN(FIFO_DEPTH_POW2),
        .OPT_ASYNC_READ(1),
        .OPT_WRITE_ON_FULL(0),
        .OPT_READ_ON_EMPTY(0)
    ) stream1_fifo (
        .i_clk(aclk),
        .i_reset(!resetn),

        .i_wr(stream1_tvalid),
        .i_data(stream1_tdata),
        .o_full(stream_full[1]),
        .o_fill(),

        .i_rd(stream_out_read),
        .o_data(stream1_data),
        .o_empty(stream_empty[1])
    );

    sfifo #(
        .BW(STREAM2_WIDTH),
        .LGFLEN(FIFO_DEPTH_POW2),
        .OPT_ASYNC_READ(1),
        .OPT_WRITE_ON_FULL(0),
        .OPT_READ_ON_EMPTY(0)
    ) stream2_fifo (
        .i_clk(aclk),
        .i_reset(!resetn),

        .i_wr(stream2_tvalid),
        .i_data(stream2_tdata),
        .o_full(stream_full[2]),
        .o_fill(),

        .i_rd(stream_out_read),
        .o_data(stream2_data),
        .o_empty(stream_empty[2])
    );

    sfifo #(
        .BW(STREAM3_WIDTH),
        .LGFLEN(FIFO_DEPTH_POW2),
        .OPT_ASYNC_READ(1),
        .OPT_WRITE_ON_FULL(0),
        .OPT_READ_ON_EMPTY(0)
    ) stream3_fifo (
        .i_clk(aclk),
        .i_reset(!resetn),

        .i_wr(stream3_tvalid),
        .i_data(stream3_tdata),
        .o_full(stream_full[3]),
        .o_fill(),

        .i_rd(stream_out_read),
        .o_data(stream3_data),
        .o_empty(stream_empty[3])
    );

    assign stream_out_tvalid = complete_transfer_available;
    assign stream_out_tdata = { stream3_data, stream2_data, stream1_data, stream0_data };
endmodule
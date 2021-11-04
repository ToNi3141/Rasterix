/*
Copyright (c) 2016 Alex Forencich
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

// Language: Verilog 2001

`timescale 1ns / 1ps

/*
 * FT245 to AXI stream bridge
 */
module axis_ft245 #(
    //
    // FT245/FT2232D timings:
    //   WR_SETUP = 20 ns
    //   WR_PULSE = 50 ns
    //   RD_PULSE = 50 ns
    //   RD_WAIT = 25 ns
    //
    // FT2232H timings:
    //   WR_SETUP = 5 ns
    //   WR_PULSE = 30 ns
    //   RD_PULSE = 30 ns
    //   RD_WAIT = 14 ns
    //
    parameter WR_SETUP_CYCLES = 3,
    parameter WR_PULSE_CYCLES = 7,
    parameter RD_PULSE_CYCLES = 8,
    parameter RD_WAIT_CYCLES = 5
)
(
    input wire clk,
    input wire rst,

    /*
     * FT245 FIFO interface
     */
    input  wire [7:0]  ft245_d_in,
    output wire [7:0]  ft245_d_out,
    output wire        ft245_d_oe,
    output wire        ft245_rd_n,
    output wire        ft245_wr_n,
    input  wire        ft245_rxf_n,
    input  wire        ft245_txe_n,
    output wire        ft245_siwu_n,

    /*
     * AXI stream interface
     */
    input  wire [7:0]  input_axis_tdata,
    input  wire        input_axis_tvalid,
    output wire        input_axis_tready,

    output wire [7:0]  output_axis_tdata,
    output wire        output_axis_tvalid,
    input  wire        output_axis_tready
);

// state register
localparam [1:0]
    STATE_IDLE = 2'd0,
    STATE_WRITE = 2'd1,
    STATE_READ = 2'd2;

reg [1:0] state_reg = STATE_IDLE, state_next;

reg [7:0] count_reg = 8'd0, count_next;

reg [7:0] ft245_d_in_reg = 8'd0;
reg [7:0] ft245_d_out_reg = 8'd0, ft245_d_out_next;
reg ft245_d_oe_reg = 1'b0, ft245_d_oe_next;
reg ft245_rd_n_reg = 1'b1, ft245_rd_n_next;
reg ft245_wr_n_reg = 1'b1, ft245_wr_n_next;
reg ft245_rxf_n_reg = 1'b1;
reg ft245_txe_n_reg = 1'b1;
reg ft245_siwu_n_reg = 1'b1, ft245_siwu_n_next;

reg input_axis_tready_reg = 1'b0, input_axis_tready_next;

// internal datapath
reg [7:0] output_axis_tdata_int;
reg       output_axis_tvalid_int;
reg       output_axis_tready_int_reg = 1'b0;
reg       output_axis_tlast_int;
reg       output_axis_tuser_int;
wire      output_axis_tready_int_early;

assign ft245_d_out = ft245_d_out_reg;
assign ft245_d_oe = ft245_d_oe_reg;
assign ft245_rd_n = ft245_rd_n_reg;
assign ft245_wr_n = ft245_wr_n_reg;
assign ft245_siwu_n = ft245_siwu_n_reg;

assign input_axis_tready = input_axis_tready_reg;

always @* begin
    state_next = STATE_IDLE;

    count_next = count_reg;

    ft245_d_out_next = ft245_d_out_reg;
    ft245_d_oe_next = ft245_d_oe_reg;
    ft245_rd_n_next = ft245_rd_n_reg;
    ft245_wr_n_next = ft245_wr_n_reg;
    ft245_siwu_n_next = ft245_siwu_n_reg;

    input_axis_tready_next = 1'b0;

    output_axis_tdata_int = 8'd0;
    output_axis_tvalid_int = 1'b0;
    output_axis_tlast_int = 1'b0;
    output_axis_tuser_int = 1'b0;

    if (count_reg > 0) begin
        count_next = count_reg - 8'd1;
        state_next = state_reg;
    end else begin
        case (state_reg)
            STATE_IDLE: begin
                // idle state
                ft245_d_oe_next = 1'b0;
                ft245_wr_n_next = 1'b1;
                ft245_rd_n_next = 1'b1;

                if (input_axis_tvalid && !ft245_txe_n_reg) begin
                    // data to write
                    input_axis_tready_next = 1'b1;
                    ft245_d_out_next = input_axis_tdata;
                    ft245_d_oe_next = 1'b1;
                    count_next = WR_SETUP_CYCLES-1;
                    state_next = STATE_WRITE;
                end else if (output_axis_tready_int_early && !ft245_rxf_n_reg) begin
                    // data to read
                    ft245_rd_n_next = 1'b0;
                    count_next = RD_PULSE_CYCLES-1;
                    state_next = STATE_READ;
                end else begin
                    state_next = STATE_IDLE;
                end
            end
            STATE_WRITE: begin
                // write
                ft245_wr_n_next = 1'b0;
                count_next = WR_PULSE_CYCLES-1;
                state_next = STATE_IDLE;
            end
            STATE_READ: begin
                // read
                output_axis_tdata_int = ft245_d_in_reg;
                output_axis_tvalid_int = 1'b1;
                ft245_rd_n_next = 1'b1;
                count_next = RD_WAIT_CYCLES-1;
                state_next = STATE_IDLE;
            end
        endcase
    end
end

always @(posedge clk) begin
    if (!rst) begin
        state_reg <= STATE_IDLE;
        count_reg <= 8'd0;
        ft245_d_out_reg <= 8'd0;
        ft245_d_oe_reg <= 1'b0;
        ft245_rd_n_reg <= 1'b1;
        ft245_wr_n_reg <= 1'b1;
        ft245_siwu_n_reg <= 1'b1;
        input_axis_tready_reg <= 1'b0;
    end else begin
        state_reg <= state_next;
        count_reg <= count_next;
        ft245_d_out_reg <= ft245_d_out_next;
        ft245_d_oe_reg <= ft245_d_oe_next;
        ft245_rd_n_reg <= ft245_rd_n_next;
        ft245_wr_n_reg <= ft245_wr_n_next;
        ft245_siwu_n_reg <= ft245_siwu_n_next;
        input_axis_tready_reg <= input_axis_tready_next;
    end

    ft245_d_in_reg <= ft245_d_in;
    ft245_rxf_n_reg <= ft245_rxf_n;
    ft245_txe_n_reg <= ft245_txe_n;
end

// output datapath logic
reg [7:0] output_axis_tdata_reg = 8'd0;
reg       output_axis_tvalid_reg = 1'b0, output_axis_tvalid_next;
reg       output_axis_tlast_reg = 1'b0;
reg       output_axis_tuser_reg = 1'b0;

reg [7:0] temp_axis_tdata_reg = 8'd0;
reg       temp_axis_tvalid_reg = 1'b0, temp_axis_tvalid_next;
reg       temp_axis_tlast_reg = 1'b0;
reg       temp_axis_tuser_reg = 1'b0;

// datapath control
reg store_axis_int_to_output;
reg store_axis_int_to_temp;
reg store_axis_temp_to_output;

assign output_axis_tdata = output_axis_tdata_reg;
assign output_axis_tvalid = output_axis_tvalid_reg;
assign output_axis_tlast = output_axis_tlast_reg;
assign output_axis_tuser = output_axis_tuser_reg;

// enable ready input next cycle if output is ready or the temp reg will not be filled on the next cycle (output reg empty or no input)
assign output_axis_tready_int_early = output_axis_tready | (~temp_axis_tvalid_reg & (~output_axis_tvalid_reg | ~output_axis_tvalid_int));

always @* begin
    // transfer sink ready state to source
    output_axis_tvalid_next = output_axis_tvalid_reg;
    temp_axis_tvalid_next = temp_axis_tvalid_reg;

    store_axis_int_to_output = 1'b0;
    store_axis_int_to_temp = 1'b0;
    store_axis_temp_to_output = 1'b0;
    
    if (output_axis_tready_int_reg) begin
        // input is ready
        if (output_axis_tready | ~output_axis_tvalid_reg) begin
            // output is ready or currently not valid, transfer data to output
            output_axis_tvalid_next = output_axis_tvalid_int;
            store_axis_int_to_output = 1'b1;
        end else begin
            // output is not ready, store input in temp
            temp_axis_tvalid_next = output_axis_tvalid_int;
            store_axis_int_to_temp = 1'b1;
        end
    end else if (output_axis_tready) begin
        // input is not ready, but output is ready
        output_axis_tvalid_next = temp_axis_tvalid_reg;
        temp_axis_tvalid_next = 1'b0;
        store_axis_temp_to_output = 1'b1;
    end
end

always @(posedge clk) begin
    if (!rst) begin
        output_axis_tvalid_reg <= 1'b0;
        output_axis_tready_int_reg <= 1'b0;
        temp_axis_tvalid_reg <= 1'b0;
    end else begin
        output_axis_tvalid_reg <= output_axis_tvalid_next;
        output_axis_tready_int_reg <= output_axis_tready_int_early;
        temp_axis_tvalid_reg <= temp_axis_tvalid_next;
    end

    // datapath
    if (store_axis_int_to_output) begin
        output_axis_tdata_reg <= output_axis_tdata_int;
        output_axis_tlast_reg <= output_axis_tlast_int;
        output_axis_tuser_reg <= output_axis_tuser_int;
    end else if (store_axis_temp_to_output) begin
        output_axis_tdata_reg <= temp_axis_tdata_reg;
        output_axis_tlast_reg <= temp_axis_tlast_reg;
        output_axis_tuser_reg <= temp_axis_tuser_reg;
    end

    if (store_axis_int_to_temp) begin
        temp_axis_tdata_reg <= output_axis_tdata_int;
        temp_axis_tlast_reg <= output_axis_tlast_int;
        temp_axis_tuser_reg <= output_axis_tuser_int;
    end
end

endmodule

//--------------------------------------------------------------------------------------------------------
// Module  : tb_ftdi_245fifo
// Type    : simulation, top
// Standard: SystemVerilog 2005 (IEEE1800-2005)
// Function: testbench for ftdi_245fifo
//--------------------------------------------------------------------------------------------------------

`timescale 1ps/1ps

module tb_ftdi_245fifo();


// -----------------------------------------------------------------------------------------------------------------------------
// simulation control
// -----------------------------------------------------------------------------------------------------------------------------
initial $dumpvars(0, tb_ftdi_245fifo);
initial #100000000 $finish;              // simulation for 100us



// -----------------------------------------------------------------------------------------------------------------------------
// user clock
// -----------------------------------------------------------------------------------------------------------------------------
reg rstn = 1'b0;
reg clk  = 1'b0;
always #5000 clk = ~clk;           // 100MHz.
initial begin repeat(4) @(posedge clk); rstn<=1'b1; end



// -----------------------------------------------------------------------------------------------------------------------------
// user signals(loopback)
// -----------------------------------------------------------------------------------------------------------------------------
localparam USER_DEXP = 1;
wire tvalid, tready;
wire [(8<<USER_DEXP)-1:0] tdata;



// -----------------------------------------------------------------------------------------------------------------------------
// generate a simple FT232H behavior
// -----------------------------------------------------------------------------------------------------------------------------
reg         usb_clk = 1'b0;
wire        usb_rxf;
wire        usb_txe;
wire        usb_oe;
wire        usb_rd;
wire        usb_wr;
tri  [ 7:0] usb_data;

always #8333 usb_clk = ~usb_clk;  // approximately 60MHz.

reg [31:0] usb_clk_cnt = 0;
reg [ 7:0] usb_rdata   = '0;

always @ (posedge usb_clk)
    usb_clk_cnt <= usb_clk_cnt + 1;
        

assign usb_data = ~usb_oe ? usb_rdata : 'z;
assign usb_rxf = (usb_clk_cnt%97) > 19;
assign usb_txe = (usb_clk_cnt%53) > 43;

// data from FTDI-Chip to FPGA (read from FTDI-Chip)
always @ (posedge usb_clk)
    if(~usb_rd & ~usb_rxf) begin
        usb_rdata <= usb_rdata + 8'd1;
        $display("R: %02X", usb_data);
    end

// data from FPGA to FTDI-Chip (write to FTDI-Chip)
always @ (posedge usb_clk)
    if(~usb_wr & ~usb_txe) begin
        $display("W: %02X", usb_data);
    end

        

// -----------------------------------------------------------------------------------------------------------------------------
// ftdi_245fifo module
// -----------------------------------------------------------------------------------------------------------------------------
ftdi_245fifo #(
    .TX_DEXP       ( USER_DEXP     ),
    .TX_AEXP       ( 10            ),
    .RX_DEXP       ( USER_DEXP     ),
    .RX_AEXP       ( 10            ),
    .C_DEXP        ( 0             )
) ftdi_245fifo_i (
    .rstn_async    ( rstn          ),
    // user write interface, loopback connect to user read  interface
    .tx_clk        ( clk           ),
    .tx_valid      ( tvalid        ),
    .tx_ready      ( tready        ),
    .tx_data       ( tdata         ),
    // user read  interface, loopback connect to user write interface
    .rx_clk        ( clk           ),
    .rx_valid      ( tvalid        ), 
    .rx_ready      ( tready        ),
    .rx_data       ( tdata         ),
    // FTDI USB interface, must connect to FT232H pins
    .usb_clk       ( usb_clk       ),
    .usb_rxf       ( usb_rxf       ),
    .usb_txe       ( usb_txe       ),
    .usb_oe        ( usb_oe        ),
    .usb_rd        ( usb_rd        ),
    .usb_wr        ( usb_wr        ),
    .usb_data      ( usb_data      ),
    .usb_be        (               )
);


endmodule

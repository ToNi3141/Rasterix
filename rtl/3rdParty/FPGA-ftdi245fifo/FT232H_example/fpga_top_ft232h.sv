
//--------------------------------------------------------------------------------------------------------
// Module  : fpga_top_ft232h
// Type    : synthesizable, FPGA's top, IP's example design
// Standard: SystemVerilog 2005 (IEEE1800-2005)
// Function: an example of ftdi_245fifo, connect FT232H chip
//           send increase data from FT232H chip,
//           recv data from FT232H chip and check whether it is increasing
//--------------------------------------------------------------------------------------------------------

module fpga_top_ft232h (
    input  wire         clk,   // main clock, connect to on-board crystal oscillator
    output wire         led,   // used to show whether the recv data meets expectations
    
    // USB2.0 HS (FT232H chip)
    output wire         usb_resetn,  // to FT232H's pin34 (RESET#) , Comment out this line if this signal is not connected to FPGA.
    output wire         usb_pwrsav,  // to FT232H's pin31 (PWRSAV#), Comment out this line if this signal is not connected to FPGA.
    output wire         usb_siwu,    // to FT232H's pin28 (SIWU#)  , Comment out this line if this signal is not connected to FPGA.
    input  wire         usb_clk,     // to FT232H's pin29 (CLKOUT)
    input  wire         usb_rxf,     // to FT232H's pin21 (RXF#)
    input  wire         usb_txe,     // to FT232H's pin25 (TXE#)
    output wire         usb_oe,      // to FT232H's pin30 (OE#)
    output wire         usb_rd,      // to FT232H's pin26 (RD#)
    output wire         usb_wr,      // to FT232H's pin27 (WR#)
    inout        [ 7:0] usb_data     // to FT232H's pin20~13 (ADBUS7~ADBUS0)
);

assign usb_resetn = 1'b1;  // 1=normal operation , Comment out this line if this signal is not connected to FPGA.
assign usb_pwrsav = 1'b1;  // 1=normal operation , Comment out this line if this signal is not connected to FPGA.
assign usb_siwu   = 1'b1;  // 1=send immidiently , Comment out this line if this signal is not connected to FPGA.


// for power on reset
reg  [ 3:0] reset_shift = '0;
reg         rstn = '0;

// USB send data stream
wire        usbtx_valid;
wire        usbtx_ready;
reg  [63:0] usbtx_data = '0;

// USB recieved data stream
wire        usbrx_valid;
wire        usbrx_ready;
wire [ 7:0] usbrx_data;

reg  [ 7:0] expect_data = '0;
reg  [31:0] led_cnt = 0;
assign led = led_cnt == 0;


//------------------------------------------------------------------------------------------------------------
// power on reset
//------------------------------------------------------------------------------------------------------------
always @ (posedge clk)
    {rstn, reset_shift} <= {reset_shift, 1'b1};



//------------------------------------------------------------------------------------------------------------
// USB TX behavior
//------------------------------------------------------------------------------------------------------------
assign usbtx_valid = 1'b1;                   // always try to send
always @ (posedge clk or negedge rstn)
    if(~rstn) begin
        usbtx_data <= '0;
    end else begin
        if(usbtx_valid & usbtx_ready)            // if send success,
            usbtx_data <= usbtx_data + 64'd1;    //    send data + 1
    end



//------------------------------------------------------------------------------------------------------------
// USB RX behavior
//------------------------------------------------------------------------------------------------------------
assign usbrx_ready = 1'b1;                   // recv always ready
always @ (posedge clk or negedge rstn)
    if(~rstn) begin
        led_cnt <= 0;
        expect_data <= '0;
    end else begin
        if(led_cnt > 0)
            led_cnt <= led_cnt - 1;
        if(usbrx_valid & usbrx_ready) begin      // if recv success,
            if(expect_data != usbrx_data)        //    if the data does not meet expectations
                led_cnt <= 50000000;
            expect_data <= usbrx_data + 8'd1;
        end
    end


//------------------------------------------------------------------------------------------------------------
// USB TX and RX controller
//------------------------------------------------------------------------------------------------------------
ftdi_245fifo #(
    .TX_DEXP     ( 3           ), // TX data stream width,  0=8bit, 1=16bit, 2=32bit, 3=64bit, 4=128bit ...
    .TX_AEXP     ( 9           ), // TX FIFO depth = 2^TX_AEXP = 2^10 = 1024
    .RX_DEXP     ( 0           ), // RX data stream width,  0=8bit, 1=16bit, 2=32bit, 3=64bit, 4=128bit ...
    .RX_AEXP     ( 10          ), // RX FIFO depth = 2^RX_AEXP = 2^10 = 1024
    .C_DEXP      ( 0           )  // FTDI USB chip data width, 0=8bit, 1=16bit, 2=32bit ... for FT232H is 0, for FT600 is 1, for FT601 is 2.
) ftdi_245fifo_i (
    .rstn_async  ( rstn        ),
    .tx_clk      ( clk         ),
    .tx_valid    ( usbtx_valid ),
    .tx_ready    ( usbtx_ready ),
    .tx_data     ( usbtx_data  ),
    .rx_clk      ( clk         ),
    .rx_valid    ( usbrx_valid ), 
    .rx_ready    ( usbrx_ready ),
    .rx_data     ( usbrx_data  ),
    .usb_clk     ( usb_clk     ),
    .usb_rxf     ( usb_rxf     ),
    .usb_txe     ( usb_txe     ),
    .usb_oe      ( usb_oe      ),
    .usb_rd      ( usb_rd      ),
    .usb_wr      ( usb_wr      ),
    .usb_data    ( usb_data    ),
    .usb_be      (             )  // FT232H do not have BE signals
);


endmodule

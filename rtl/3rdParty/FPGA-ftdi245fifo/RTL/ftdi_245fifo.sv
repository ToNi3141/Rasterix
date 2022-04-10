
//--------------------------------------------------------------------------------------------------------
// Module  : ftdi_245fifo
// Type    : synthesizable, IP's top
// Standard: SystemVerilog 2005 (IEEE1800-2005)
// Function: FTDI USB chip's 245fifo mode controller
//           include: FT232H, FT2232H, FT600Q, FT601Q, ...
//           to realize USB communication
//--------------------------------------------------------------------------------------------------------

module ftdi_245fifo #(
    parameter TX_DEXP = 0,  // TX data stream width,  0=8bit, 1=16bit, 2=32bit, 3=64bit, 4=128bit ...
    parameter TX_AEXP = 10, // TX FIFO depth = 2^TX_AEXP
    parameter RX_DEXP = 0,  // RX data stream width,  0=8bit, 1=16bit, 2=32bit, 3=64bit, 4=128bit ...
    parameter RX_AEXP = 10, // RX FIFO depth = 2^RX_AEXP
    parameter C_DEXP  = 0   // FTDI USB chip data width, 0=8bit, 1=16bit, 2=32bit ... for FT232H is 0, for FT600 is 1, for FT601 is 2.
)(
    // reset, active low
    input  wire                    rstn_async,
    // user send interface (FPGA -> USB -> PC), AXI-stream slave liked.
    input  wire                    tx_clk,    // User-specified clock for send interface
    input  wire                    tx_valid,
    output wire                    tx_ready,
    input  wire [(8<<TX_DEXP)-1:0] tx_data,
    // user recv interface (PC -> USB -> FPGA), AXI-stream master liked.
    input  wire                    rx_clk,    // User-specified clock for recv interface
    output wire                    rx_valid,
    input  wire                    rx_ready,
    output wire [(8<<RX_DEXP)-1:0] rx_data,
    // FTDI 245FIFO interface, connect these signals to FTDI USB chip
    input  wire                    usb_rxf, usb_txe, usb_clk,
    output reg                     usb_oe , usb_rd , usb_wr,
    inout        [(8<<C_DEXP)-1:0] usb_data,
    output wire  [(1<<C_DEXP)-1:0] usb_be // only FT600&FT601 have usb_be signal, ignore it when the chip do NOT have this signal
);

initial {usb_oe, usb_rd, usb_wr} = '1;

localparam TXFIFO_DEXP = TX_DEXP > C_DEXP ? TX_DEXP : C_DEXP;
localparam RXFIFO_DEXP = RX_DEXP > C_DEXP ? RX_DEXP : C_DEXP;

wire                        txfifoi_valid;
wire                        txfifoi_ready;
wire [(8<<TXFIFO_DEXP)-1:0] txfifoi_data;

wire                        txfifoo_valid;
wire                        txfifoo_ready;
wire [(8<<TXFIFO_DEXP)-1:0] txfifoo_data;

wire                        c_tx_valid;
reg                         c_tx_ready;
wire      [(8<<C_DEXP)-1:0] c_tx_data;

reg                         c_rx_valid = '0;
wire                        c_rx_ready;
reg       [(8<<C_DEXP)-1:0] c_rx_data = '0;

wire                        rxfifoi_valid;
wire                        rxfifoi_ready;
wire [(8<<RXFIFO_DEXP)-1:0] rxfifoi_data;

wire                        rxfifoo_valid;
wire                        rxfifoo_ready;
wire [(8<<RXFIFO_DEXP)-1:0] rxfifoo_data;

enum logic [2:0] {RESET1, RESET2, RXIDLE, RXOE, RXD, TXIDLE, TXD} stat = RESET1;

reg                   s_rx_valid = '0;
reg [(8<<C_DEXP)-1:0] s_rx_data = '0;
reg                   s_tx_valid = '0;

reg [(8<<C_DEXP)-1:0] usb_txdata = '0;


// generate reset for tx_clk
reg [1:0] rstn_tx_clk_shift = '0;
reg       rstn_tx_clk = '0;
always @ (posedge tx_clk or negedge rstn_async)
    if(~rstn_async)
        {rstn_tx_clk_shift, rstn_tx_clk} <= '0;
    else
        {rstn_tx_clk_shift, rstn_tx_clk} <= {1'b1, rstn_tx_clk_shift};

// generate reset for rx_clk
reg [1:0] rstn_rx_clk_shift = '0;
reg       rstn_rx_clk = '0;
always @ (posedge rx_clk or negedge rstn_async)
    if(~rstn_async)
        {rstn_rx_clk_shift, rstn_rx_clk} <= '0;
    else
        {rstn_rx_clk_shift, rstn_rx_clk} <= {1'b1, rstn_rx_clk_shift};

// generate reset for usb_clk
reg [1:0] rstn_usb_clk_shift = '0;
reg       rstn_usb_clk = '0;
always @ (posedge usb_clk or negedge rstn_async)
    if(~rstn_async)
        {rstn_usb_clk_shift, rstn_usb_clk} <= '0;
    else
        {rstn_usb_clk_shift, rstn_usb_clk} <= {1'b1, rstn_usb_clk_shift};


stream_wtrans #(
    .I_DEXP    ( TX_DEXP        ),
    .O_DEXP    ( TXFIFO_DEXP    )
) txfifoi_wtrans_i (
    .rstn      ( rstn_tx_clk    ),
    .clk       ( tx_clk         ),
    .itvalid   ( tx_valid       ),
    .itready   ( tx_ready       ),
    .itdata    ( tx_data        ),
    .otvalid   ( txfifoi_valid  ),
    .otready   ( txfifoi_ready  ),
    .otdata    ( txfifoi_data   )
);

stream_async_fifo #(
    .DSIZE     ( 8<<TXFIFO_DEXP ),
    .ASIZE     ( TX_AEXP        )
) txfifo_i (
    .irstn     ( rstn_tx_clk    ),
    .iclk      ( tx_clk         ),
    .itvalid   ( txfifoi_valid  ),
    .itready   ( txfifoi_ready  ),
    .itdata    ( txfifoi_data   ),
    .orstn     ( rstn_usb_clk   ),
    .oclk      ( usb_clk        ),
    .otvalid   ( txfifoo_valid  ),
    .otready   ( txfifoo_ready  ),
    .otdata    ( txfifoo_data   )
);

stream_wtrans #(
    .I_DEXP    ( TXFIFO_DEXP    ),
    .O_DEXP    ( C_DEXP         )
) txfifoo_wtrans_i (
    .rstn      ( rstn_usb_clk   ),
    .clk       ( usb_clk        ),
    .itvalid   ( txfifoo_valid  ),
    .itready   ( txfifoo_ready  ),
    .itdata    ( txfifoo_data   ),
    .otvalid   ( c_tx_valid     ),
    .otready   ( c_tx_ready     ),
    .otdata    ( c_tx_data      )
);

stream_wtrans #(
    .I_DEXP    ( C_DEXP         ),
    .O_DEXP    ( RXFIFO_DEXP    )
) rxfifoi_wtrans_i (
    .rstn      ( rstn_usb_clk   ),
    .clk       ( usb_clk        ),
    .itvalid   ( c_rx_valid     ),
    .itready   ( c_rx_ready     ),
    .itdata    ( c_rx_data      ),
    .otvalid   ( rxfifoi_valid  ),
    .otready   ( rxfifoi_ready  ),
    .otdata    ( rxfifoi_data   )
);

stream_async_fifo #(
    .DSIZE     ( 8<<RXFIFO_DEXP ),
    .ASIZE     ( RX_AEXP        )
) rxfifo_i (
    .irstn     ( rstn_usb_clk   ),
    .iclk      ( usb_clk        ),
    .itvalid   ( rxfifoi_valid  ),
    .itready   ( rxfifoi_ready  ),
    .itdata    ( rxfifoi_data   ),
    .orstn     ( rstn_rx_clk    ),
    .oclk      ( rx_clk         ),
    .otvalid   ( rxfifoo_valid  ),
    .otready   ( rxfifoo_ready  ),
    .otdata    ( rxfifoo_data   )
);

stream_wtrans #(
    .I_DEXP    ( RXFIFO_DEXP    ),
    .O_DEXP    ( RX_DEXP        )
) rxfifoo_wtrans_i (
    .rstn      ( rstn_rx_clk    ),
    .clk       ( rx_clk         ),
    .itvalid   ( rxfifoo_valid  ),
    .itready   ( rxfifoo_ready  ),
    .itdata    ( rxfifoo_data   ),
    .otvalid   ( rx_valid       ),
    .otready   ( rx_ready       ),
    .otdata    ( rx_data        )
);


always @ (posedge usb_clk or negedge rstn_usb_clk)
    if(~rstn_usb_clk) begin
        {usb_oe, usb_rd, usb_wr} <= '1;
        {c_rx_valid, c_rx_data} <= '0;
        {s_rx_valid, s_rx_data} <= '0;
        s_tx_valid <= '0;
        usb_txdata <= '0;
        stat <= RESET1;
    end else begin
        if(c_rx_ready) begin
            {c_rx_valid, c_rx_data} <= {s_rx_valid, s_rx_data};
            {s_rx_valid, s_rx_data} <= '0;
        end
        case(stat)
            RESET1: begin
                {usb_oe, usb_rd, usb_wr} <= '1;
                stat <= RESET2;
            end
            RESET2: begin
                {usb_oe, usb_rd, usb_wr} <= '1;
                stat <= RXIDLE;
            end
            RXIDLE: begin
                if(~usb_rxf & ~s_rx_valid & c_rx_ready) begin
                    usb_oe <= 1'b0;
                    stat <= RXOE;
                end else if(~usb_txe & (s_tx_valid | c_tx_valid)) begin
                    if(~s_tx_valid) begin
                        usb_txdata <= c_tx_data;
                        //assign c_tx_ready = 1'b1;
                    end
                    usb_wr <= 1'b0;
                    stat <= TXD;
                    s_tx_valid <= 1'b0;
                end
            end
            RXOE : begin
                usb_rd <= 1'b0;
                stat <= RXD;
            end
            RXD : begin
                if(~usb_rxf) begin
                    if (~c_rx_valid | c_rx_ready) begin
                        {c_rx_valid, c_rx_data} <= {1'b1, usb_data};
                    end else begin
                        {s_rx_valid, s_rx_data} <= {1'b1, usb_data};
                        usb_oe <= 1'b1;
                        usb_rd <= 1'b1;
                        stat <= TXIDLE;
                    end
                end else begin
                    usb_oe <= 1'b1;
                    usb_rd <= 1'b1;
                    stat <= TXIDLE;
                end
            end
            TXIDLE: begin
                if(~usb_txe & (s_tx_valid | c_tx_valid)) begin
                    if(~s_tx_valid) begin
                        usb_txdata <= c_tx_data;
                        //assign c_tx_ready = 1'b1;
                    end
                    s_tx_valid <= 1'b0;
                    usb_wr <= 1'b0;
                    stat <= TXD;
                end else if(~usb_rxf & ~s_rx_valid & c_rx_ready) begin
                    usb_oe <= 1'b0;
                    stat <= RXOE;
                end
            end
            default : begin   // TXD
                if(~usb_txe) begin
                    if(c_tx_valid) begin
                        usb_txdata <= c_tx_data;
                        //assign c_tx_ready = 1'b1;
                    end else begin
                        usb_wr <= 1'b1;
                        stat <= RXIDLE;
                    end
                end else begin
                    s_tx_valid <= 1'b1;
                    usb_wr <= 1'b1;
                    stat <= RXIDLE;
                end
            end
        endcase
    end

always_comb begin
    c_tx_ready = 1'b0;
    case(stat)
        RXIDLE: if(~usb_rxf & ~s_rx_valid & c_rx_ready) begin
        end else if(~usb_txe & ~s_tx_valid & c_tx_valid)
            c_tx_ready = 1'b1;
        TXIDLE: if(~usb_txe & ~s_tx_valid & c_tx_valid)
            c_tx_ready = 1'b1;
        TXD : if(~usb_txe & c_tx_valid) 
            c_tx_ready = 1'b1;
        default: begin end
    endcase
end

assign usb_be   = usb_wr ? 'z : '1;
assign usb_data = usb_wr ? 'z : usb_txdata;

endmodule


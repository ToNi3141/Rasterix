module FT245X2AXIS (
    input wire          aclk,
    input wire          aresetn,

    output reg [1:0]    set_vadj,
    output reg          vadj_en,
    
    inout  wire         fmc_wkup,
    input  wire         fmc_clk,
    input  wire         fmc_txen,
    input  wire         fmc_rxfn,
    output wire         fmc_rdn,
    output wire         fmc_oen,
    inout  wire [1:0]   fmc_be,
    output wire         fmc_siwun,
    output wire         fmc_wrn,
    inout  wire [15:0]  fmc_data,

    output wire         m_axis_tvalid,
    input  wire         m_axis_tready,
    output wire         m_axis_tlast,
    output wire [15:0]  m_axis_tdata
    );

    // Nexys Video FMC voltage selection
    reg [1:0] adj_state = 0;
    always @(posedge fmc_clk)
    begin
        case (adj_state)
        0: 
        begin
            vadj_en <= 0;
            adj_state <= 1;
        end
        1:
        begin
            set_vadj <= 2'b10; // Set voltage to 2.5V
            adj_state <= 2;
        end
        2:
        begin
            vadj_en <= 1;
        end 
        endcase
    end

    assign m_axis_tlast = 0;

    ftdi_245fifo #(
        .TX_DEXP     ( 1           ), // TX data stream width,  0=8bit, 1=16bit, 2=32bit, 3=64bit, 4=128bit ...
        .TX_AEXP     ( 10          ), // TX FIFO depth = 2^TX_AEXP = 2^10 = 1024
        .RX_DEXP     ( 1           ), // RX data stream width,  0=8bit, 1=16bit, 2=32bit, 3=64bit, 4=128bit ...
        .RX_AEXP     ( 10          ), // RX FIFO depth = 2^RX_AEXP = 2^10 = 1024
        .C_DEXP      ( 1           )  // FTDI USB chip data width, 0=8bit, 1=16bit, 2=32bit ... for FT232H is 0, for FT600 is 1, for FT601 is 2.
    ) ftdi_245fifo_i (
        .rstn_async  ( aresetn      ),
        .tx_clk      ( aclk         ),
        .tx_valid    ( 0            ),
        .tx_ready    (              ),
        .tx_data     (              ),
        .rx_clk      ( aclk         ),
        .rx_valid    ( m_axis_tvalid ), 
        .rx_ready    ( m_axis_tready ),
        .rx_data     ( m_axis_tdata  ),
        .usb_clk     ( fmc_clk      ),
        .usb_rxf     ( fmc_rxfn     ),
        .usb_txe     ( fmc_txen     ),
        .usb_oe      ( fmc_oen      ),
        .usb_rd      ( fmc_rdn      ),
        .usb_wr      ( fmc_wrn      ),
        .usb_data    ( fmc_data     ),
        .usb_be      ( fmc_be       )
    );

endmodule

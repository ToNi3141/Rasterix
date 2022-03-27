//-----------------------------------------------------------------
// TOP
//-----------------------------------------------------------------
module top
(
    input           clk100mhz,
    output [3:0]    tmds_out_p,
    output [3:0]    tmds_out_n    
);

//-----------------------------------------------------------------
// Implementation
//-----------------------------------------------------------------
wire clk50_w;
wire clk250_w;
wire rst_w;

wire dvi_red_w;
wire dvi_green_w;
wire dvi_blue_w;
wire dvi_clock_w;

artix7_pll
u_pll
(
    // Inputs
    .clkref_i(clk100mhz)

    // Outputs
    ,.clkout0_o(clk50_w)
    ,.clkout1_o(clk250_w)
);

reset_gen
u_rst
(
    .clk_i(clk50_w),
    .rst_o(rst_w)
);

fpga_top
u_top
(
     .clk_i(clk50_w)
    ,.rst_i(rst_w)
    ,.clk_x5_i(clk250_w)

    ,.dvi_red_o(dvi_red_w)
    ,.dvi_green_o(dvi_green_w)
    ,.dvi_blue_o(dvi_blue_w)
    ,.dvi_clock_o(dvi_clock_w)
);


OBUFDS u_buf_b
(
    .O(tmds_out_p[0]),
    .OB(tmds_out_n[0]),
    .I(dvi_blue_w)
);

OBUFDS u_buf_g
(
    .O(tmds_out_p[1]),
    .OB(tmds_out_n[1]),
    .I(dvi_green_w)
);

OBUFDS u_buf_r
(
    .O(tmds_out_p[2]),
    .OB(tmds_out_n[2]),
    .I(dvi_red_w)
);

OBUFDS u_buf_c
(
    .O(tmds_out_p[3]),
    .OB(tmds_out_n[3]),
    .I(dvi_clock_w)
);

endmodule

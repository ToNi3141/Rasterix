
module fpga_top
(
    // Inputs
     input           clk_i
    ,input           clk_x5_i
    ,input           rst_i

    // Outputs
    ,output          dvi_red_o
    ,output          dvi_green_o
    ,output          dvi_blue_o
    ,output          dvi_clock_o
);

wire  [  7:0]  video_blue_w;
wire           video_hsync_w;
wire           video_vsync_w;
wire  [  7:0]  video_green_w;
wire  [  7:0]  video_red_w;
wire           video_blank_w;


dvi
u_dvi
(
    // Inputs
     .clk_i(clk_i)
    ,.rst_i(rst_i)
    ,.clk_x5_i(clk_x5_i)
    ,.vga_red_i(video_red_w)
    ,.vga_green_i(video_green_w)
    ,.vga_blue_i(video_blue_w)
    ,.vga_blank_i(video_blank_w)
    ,.vga_hsync_i(video_hsync_w)
    ,.vga_vsync_i(video_vsync_w)

    // Outputs
    ,.dvi_red_o(dvi_red_o)
    ,.dvi_green_o(dvi_green_o)
    ,.dvi_blue_o(dvi_blue_o)
    ,.dvi_clock_o(dvi_clock_o)
);


video_gen
u_gen
(
    // Inputs
     .clk_i(clk_i)
    ,.rst_i(rst_i)

    // Outputs
    ,.vga_red_o(video_red_w)
    ,.vga_green_o(video_green_w)
    ,.vga_blue_o(video_blue_w)
    ,.vga_blank_o(video_blank_w)
    ,.vga_hsync_o(video_hsync_w)
    ,.vga_vsync_o(video_vsync_w)
);



endmodule


module video_gen
//-----------------------------------------------------------------
// Params
//-----------------------------------------------------------------
#(
     parameter WIDTH            = 800
    ,parameter HEIGHT           = 600
    ,parameter REFRESH          = 72
)
//-----------------------------------------------------------------
// Ports
//-----------------------------------------------------------------
(
    // Inputs
     input           clk_i
    ,input           rst_i

    // Outputs
    ,output [  7:0]  vga_red_o
    ,output [  7:0]  vga_green_o
    ,output [  7:0]  vga_blue_o
    ,output          vga_blank_o
    ,output          vga_hsync_o
    ,output          vga_vsync_o
);



//-----------------------------------------------------------------
// Video Timings
//-----------------------------------------------------------------
localparam H_REZ         = WIDTH;
localparam V_REZ         = HEIGHT;
localparam CLK_MHZ       = (WIDTH == 640 && REFRESH == 60)  ? 25.175 :
                           (WIDTH == 800 && REFRESH == 72)  ? 50.00  :
                           (WIDTH == 1280 && REFRESH == 60) ? 74.25  :
                           (WIDTH == 1920 && REFRESH == 60) ? 148.5  :
                                                              0;
localparam H_SYNC_START  = (WIDTH == 640 && REFRESH == 60)  ? 656 :
                           (WIDTH == 800 && REFRESH == 72)  ? 856 :
                           (WIDTH == 1280 && REFRESH == 60) ? 1390:
                           (WIDTH == 1920 && REFRESH == 60) ? 2008:
                                                              0;
localparam H_SYNC_END    = (WIDTH == 640 && REFRESH == 60)  ? 752 :
                           (WIDTH == 800 && REFRESH == 72)  ? 976 :
                           (WIDTH == 1280 && REFRESH == 60) ? 1430:
                           (WIDTH == 1920 && REFRESH == 60) ? 2052:
                                                              0;
localparam H_MAX         = (WIDTH == 640 && REFRESH == 60)  ? 800 :
                           (WIDTH == 800 && REFRESH == 72)  ? 1040:
                           (WIDTH == 1280 && REFRESH == 60) ? 1650:
                           (WIDTH == 1920 && REFRESH == 60) ? 2200:
                                                              0;
localparam V_SYNC_START  = (HEIGHT == 480 && REFRESH == 60) ? 490 :
                           (HEIGHT == 600 && REFRESH == 72) ? 637 :
                           (HEIGHT == 720 && REFRESH == 60) ? 725 :
                           (HEIGHT == 1080 && REFRESH == 60)? 1084 :
                                                              0;
localparam V_SYNC_END    = (HEIGHT == 480 && REFRESH == 60) ? 492 :
                           (HEIGHT == 600 && REFRESH == 72) ? 643 :
                           (HEIGHT == 720 && REFRESH == 60) ? 730 :
                           (HEIGHT == 1080 && REFRESH == 60)? 1089:
                                                              0;
localparam V_MAX         = (HEIGHT == 480 && REFRESH == 60) ? 525 :
                           (HEIGHT == 600 && REFRESH == 72) ? 666 :
                           (HEIGHT == 720 && REFRESH == 60) ? 750 :
                           (HEIGHT == 1080 && REFRESH == 60)? 1125:
                                                              0;

//-----------------------------------------------------------------
// Colour Bars
//-----------------------------------------------------------------
localparam H_STRIDE = WIDTH / 7;
localparam C0_S     = 0;
localparam C0_E     = H_STRIDE-1;
localparam C1_S     = C0_E + 1;
localparam C1_E     = C1_S + H_STRIDE - 1;
localparam C2_S     = C1_E + 1;
localparam C2_E     = C2_S + H_STRIDE - 1;
localparam C3_S     = C2_E + 1;
localparam C3_E     = C3_S + H_STRIDE - 1;
localparam C4_S     = C3_E + 1;
localparam C4_E     = C4_S + H_STRIDE - 1;
localparam C5_S     = C4_E + 1;
localparam C5_E     = C5_S + H_STRIDE - 1;
localparam C6_S     = C5_E + 1;
localparam C6_E     = C6_S + H_STRIDE - 1;

//-----------------------------------------------------------------
// HSYNC, VSYNC
//-----------------------------------------------------------------
reg [11:0] h_pos_q;
reg [11:0] v_pos_q;
reg        h_sync_q;
reg        v_sync_q;
reg        active_q;

always @ (posedge clk_i or posedge rst_i)
if (rst_i)
    h_pos_q  <= 12'b0;
else if (h_pos_q == H_MAX)
    h_pos_q  <= 12'b0;
else
    h_pos_q  <= h_pos_q + 12'd1;

always @ (posedge clk_i or posedge rst_i)
if (rst_i)
    v_pos_q  <= 12'b0;
else if (h_pos_q == H_MAX)
begin
    if (v_pos_q == V_MAX)
        v_pos_q  <= 12'b0;
    else
        v_pos_q  <= v_pos_q + 12'd1;
end

always @ (posedge clk_i or posedge rst_i)
if (rst_i)
    h_sync_q  <= 1'b0;
else if (h_pos_q >= H_SYNC_START && h_pos_q < H_SYNC_END)
    h_sync_q  <= 1'b1;
else
    h_sync_q  <= 1'b0;

always @ (posedge clk_i or posedge rst_i)
if (rst_i)
    v_sync_q  <= 1'b0;
else if (v_pos_q >= V_SYNC_START && v_pos_q < V_SYNC_END)
    v_sync_q  <= 1'b1;
else
    v_sync_q  <= 1'b0;

// 75% white   (192, 192, 192)
// 75% yellow  (192, 192, 0)
// 75% cyan    (0, 192, 192)
// 75% green   (0, 192, 0)
// 75% magenta (192, 0, 192)
// 75% red (192, 0, 0)
// 75% blue    (0, 0, 192)
reg [7:0] red_r;
reg [7:0] green_r;
reg [7:0] blue_r;

always @ *
begin
    red_r   = 8'b0;
    green_r = 8'b0;
    blue_r  = 8'b0;

    if (h_pos_q <= C0_E)
    begin
        red_r   = 8'd192;
        green_r = 8'd192;
        blue_r  = 8'd192;
    end
    else if (h_pos_q >= C1_S && h_pos_q <= C1_E)
    begin
        red_r   = 8'd192;
        green_r = 8'd192;
        blue_r  = 8'd0;
    end
    else if (h_pos_q >= C2_S && h_pos_q <= C2_E)
    begin
        red_r   = 8'd0;
        green_r = 8'd192;
        blue_r  = 8'd192;
    end
    else if (h_pos_q >= C3_S && h_pos_q <= C3_E)
    begin
        red_r   = 8'd0;
        green_r = 8'd192;
        blue_r  = 8'd0;
    end
    else if (h_pos_q >= C4_S && h_pos_q <= C4_E)
    begin
        red_r   = 8'd192;
        green_r = 8'd0;
        blue_r  = 8'd192;
    end
    else if (h_pos_q >= C5_S && h_pos_q <= C5_E)
    begin
        red_r   = 8'd192;
        green_r = 8'd0;
        blue_r  = 8'd0;
    end
    else //if (h_pos_q >= C6_S && h_pos_q <= C6_E)
    begin
        red_r   = 8'd0;
        green_r = 8'd0;
        blue_r  = 8'd192;
    end
end

assign vga_red_o   = red_r;
assign vga_green_o = green_r;
assign vga_blue_o  = blue_r;
assign vga_hsync_o = h_sync_q;
assign vga_vsync_o = v_sync_q;
assign vga_blank_o = (h_pos_q < H_REZ && v_pos_q < V_REZ) ? 1'b0 : 1'b1;



endmodule

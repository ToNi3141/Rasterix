
//--------------------------------------------------------------------------------------------------------
// Module  : stream_async_fifo
// Type    : synthesizable, IP's sub module
// Standard: SystemVerilog 2005 (IEEE1800-2005)
// Function: stream async fifo
//--------------------------------------------------------------------------------------------------------

module stream_async_fifo #(
    parameter   DSIZE = 8,
    parameter   ASIZE = 10
)(
    input  wire             irstn,
    input  wire             iclk,
    input  wire             itvalid,
    output wire             itready,
    input  wire [DSIZE-1:0] itdata,
    input  wire             orstn,
    input  wire             oclk,
    output reg              otvalid,
    input  wire             otready,
    output wire [DSIZE-1:0] otdata
);

initial otvalid = 1'b0;

reg  [DSIZE-1:0] buffer [1<<ASIZE];  // may automatically synthesize to BRAM

reg  [ASIZE:0] wptr='0, wq_wptr_grey='0, rq1_wptr_grey='0, rq2_wptr_grey='0;
reg  [ASIZE:0] rptr='0, rq_rptr_grey='0, wq1_rptr_grey='0, wq2_rptr_grey='0;

wire [ASIZE:0] wptr_grey = (wptr >> 1) ^ wptr;
wire [ASIZE:0] rptr_grey = (rptr >> 1) ^ rptr;

always @ (posedge iclk or negedge irstn)
    if(~irstn)
        wq_wptr_grey <= '0;
    else
        wq_wptr_grey <= wptr_grey;

always @ (posedge oclk or negedge orstn)
    if(~orstn)
        {rq2_wptr_grey, rq1_wptr_grey} <= '0;
    else
        {rq2_wptr_grey, rq1_wptr_grey} <= {rq1_wptr_grey, wq_wptr_grey};

always @ (posedge oclk or negedge orstn)
    if(~orstn)
        rq_rptr_grey <= '0;
    else
        rq_rptr_grey <= rptr_grey;

always @ (posedge iclk or negedge irstn)
    if(~irstn)
        {wq2_rptr_grey, wq1_rptr_grey} <= '0;
    else
        {wq2_rptr_grey, wq1_rptr_grey} <= {wq1_rptr_grey, rq_rptr_grey};

wire w_full  = wq2_rptr_grey == {~wptr_grey[ASIZE:ASIZE-1], wptr_grey[ASIZE-2:0]};
wire r_empty = rq2_wptr_grey == rptr_grey;

assign itready = ~w_full;

always @ (posedge iclk or negedge irstn)
    if(~irstn) begin
        wptr <= '0;
    end else begin
        if(itvalid & ~w_full)
            wptr <= wptr + (1+ASIZE)'(1);
    end

always @ (posedge iclk)
    if(itvalid & ~w_full)
        buffer[wptr[ASIZE-1:0]] <= itdata;

wire            rdready = ~otvalid | otready;
reg             rdack = '0;
reg [DSIZE-1:0] rddata;
reg [DSIZE-1:0] keepdata = '0;
assign otdata = rdack ? rddata : keepdata;

always @ (posedge oclk or negedge orstn)
    if(~orstn) begin
        otvalid <= 1'b0;
        rdack <= 1'b0;
        rptr <= '0;
        keepdata <= '0;
    end else begin
        otvalid <= ~r_empty | ~rdready;
        rdack <= ~r_empty & rdready;
        if(~r_empty & rdready)
            rptr <= rptr + (1+ASIZE)'(1);
        if(rdack)
            keepdata <= rddata;
    end

always @ (posedge oclk)
    rddata <= buffer[rptr[ASIZE-1:0]];

endmodule


//--------------------------------------------------------------------------------------------------------
// Module  : stream_wtrans
// Type    : synthesizable, IP's sub module
// Standard: SystemVerilog 2005 (IEEE1800-2005)
// Function: stream data width transformer
//--------------------------------------------------------------------------------------------------------

module stream_wtrans #(
    parameter I_DEXP = 0,  // input width,  0=1Byte, 1=2Byte, 2=4Byte, 3=8Bytes, 4=16Bytes, ...
    parameter O_DEXP = 0   // output width, 0=1Byte, 1=2Byte, 2=4Byte, 3=8Bytes, 4=16Bytes, ...
)(
    input  wire                   rstn,
    input  wire                   clk,
    input  wire                   itvalid,
    output wire                   itready,
    input  wire [(8<<I_DEXP)-1:0] itdata,
    output wire                   otvalid,
    input  wire                   otready,
    output wire [(8<<O_DEXP)-1:0] otdata
);

localparam DEXP = I_DEXP > O_DEXP ? I_DEXP : O_DEXP;

reg  [2*(8<<DEXP)-1:0] buffer = '0;

reg  [1+DEXP-I_DEXP:0] wptr = '0;
reg  [1+DEXP-O_DEXP:0] rptr = '0;
wire          wmsb;
wire          rmsb;
wire [DEXP:0] wa;
wire [DEXP:0] ra;

generate if(I_DEXP==0) assign {wmsb, wa} = wptr;
else                   assign {wmsb, wa} = {wptr, {I_DEXP{1'b0}}};
endgenerate

generate if(O_DEXP==0) assign {rmsb, ra} = rptr;
else                   assign {rmsb, ra} = {rptr, {O_DEXP{1'b0}}};
endgenerate

wire empty =  {wmsb, wa[DEXP]} == {rmsb, ra[DEXP]};
wire full  = {~wmsb, wa[DEXP]} == {rmsb, ra[DEXP]};

assign itready = ~full;

always @ (posedge clk or negedge rstn)
    if(~rstn) begin
        buffer <= '0;
        wptr <= '0;
    end else begin
        if(itvalid & ~full) begin
            buffer[wa*8+:(8<<I_DEXP)] <= itdata;
            wptr <= wptr + (2+DEXP-I_DEXP)'(1);
        end
    end

assign otvalid = ~empty;
assign otdata = buffer[ra*8+:(8<<O_DEXP)];

always @ (posedge clk or negedge rstn)
    if(~rstn) begin
        rptr <= '0;
    end else begin
        if(otready & ~empty)
            rptr <= rptr + (2+DEXP-O_DEXP)'(1);
    end

endmodule

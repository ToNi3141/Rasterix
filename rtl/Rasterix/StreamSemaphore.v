// Float
// https://github.com/ToNi3141/Float
// Copyright (c) 2021 ToNi3141

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

// This module observes the incoming and outgoing values of a pipeline
// and controls the flow into the pipeline.
module StreamSemaphore #(
    parameter MAX_NUMBER_OF_ELEMENTS = 128,
    parameter STREAM_WIDTH = 32,
    parameter KEEP_WIDTH = 1
)
(
    input  wire                         aclk,
    input  wire                         resetn,

    output wire                         m_axis_tvalid,
    output reg                          m_axis_tlast,
    output reg  [STREAM_WIDTH - 1 : 0]  m_axis_tdata,
    output reg  [KEEP_WIDTH - 1 : 0]    m_axis_tkeep,
    input  wire                         m_axis_tready,

    input  wire                         s_axis_tvalid,
    output reg                          s_axis_tready,
    input  wire                         s_axis_tlast,
    input  wire [STREAM_WIDTH - 1 : 0]  s_axis_tdata,
    input  wire [KEEP_WIDTH - 1 : 0]    s_axis_tkeep,

    input  wire                         sigRelease,
    output reg                          released
);
    localparam SKID_WIDTH = 1 + KEEP_WIDTH + STREAM_WIDTH;
    reg  [7 : 0]                valuesCounter = 0;
    wire [SKID_WIDTH - 1 : 0]   skidOutData;
    wire                        skidInReady;

    wire sigOutgoingValue   = sigRelease;
    wire sigIncommingValue  = s_axis_tvalid && s_axis_tready;
    wire free               = (valuesCounter < MAX_NUMBER_OF_ELEMENTS);

    skidbuffer #(
        .OPT_OUTREG(1),
        .OPT_PASSTHROUGH(0),
        .OPT_INITIAL(1),
        .DW(SKID_WIDTH)
    ) skb (
        .i_clk(aclk),
        .i_reset(!resetn),

        .i_valid(s_axis_tvalid && free),
        .o_ready(skidInReady),
        .i_data({ s_axis_tlast, s_axis_tkeep, s_axis_tdata }),

        .o_valid(m_axis_tvalid),
        .i_ready(m_axis_tready),
        .o_data(skidOutData)
    );

    always @(posedge aclk)
    begin
        if (!resetn)
        begin
            valuesCounter = 0;
        end
        else
        begin
            // Value goes out and no values comes in. 
            // The pipeline loses a value (decrement) but still contains something
            if ((sigOutgoingValue == 1) && (sigIncommingValue == 0) && (valuesCounter != 0))
            begin
                valuesCounter = valuesCounter - 1;
            end
            // No values goes out but one comes in
            // The pipeline receives a new values (increment) therefor it contains values
            if ((sigOutgoingValue == 0) && (sigIncommingValue == 1) && (valuesCounter != MAX_NUMBER_OF_ELEMENTS))
            begin
                valuesCounter = valuesCounter + 1;
            end
            released <= valuesCounter == 0;   
        end
    end

    always @(*)
    begin
        s_axis_tready = skidInReady && free;
        m_axis_tdata = skidOutData[0 +: STREAM_WIDTH];
        m_axis_tkeep = skidOutData[STREAM_WIDTH +: KEEP_WIDTH];
        m_axis_tlast = skidOutData[STREAM_WIDTH + KEEP_WIDTH +: 1];
    end
endmodule

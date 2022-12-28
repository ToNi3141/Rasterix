// Rasterix
// https://github.com/ToNi3141/Rasterix
// Copyright (c) 2022 ToNi3141

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

module CommandParser #(
    parameter CMD_STREAM_WIDTH = 32,
    parameter TEXTURE_STREAM_WIDTH = 32
) (
    input  wire         aclk,
    input  wire         resetn,

    // AXI Stream command interface
    input  wire         s_cmd_axis_tvalid,
    output reg          s_cmd_axis_tready,
    input  wire         s_cmd_axis_tlast,
    input  wire [CMD_STREAM_WIDTH - 1 : 0]  s_cmd_axis_tdata,

    // Fog function LUT stream
    output reg         m_fog_lut_axis_tvalid,
    input  wire        m_fog_lut_axis_tready,
    output reg         m_fog_lut_axis_tlast,
    output reg  [CMD_STREAM_WIDTH - 1 : 0] m_fog_lut_axis_tdata,

    // Control
    input  wire         rasterizerRunning,
    output reg          startRendering,
    input  wire         pixelInPipeline,
    output reg          m_rasterizer_axis_tvalid,
    input  wire         m_rasterizer_axis_tready,
    output reg          m_rasterizer_axis_tlast,
    output reg  [CMD_STREAM_WIDTH - 1 : 0]  m_rasterizer_axis_tdata,

    // Color/Depth buffer control
    output reg          colorBufferApply,
    input  wire         colorBufferApplied,
    output reg          colorBufferCmdCommit,
    output reg          colorBufferCmdMemset,
    output reg          depthBufferApply,
    input  wire         depthBufferApplied,
    output reg          depthBufferCmdCommit,
    output reg          depthBufferCmdMemset,

    // Texture stream interface
    // TMU0
    output reg          m_texture_steam_tmu0_axis_tvalid,
    input  wire         m_texture_steam_tmu0_axis_tready,
    output reg          m_texture_steam_tmu0_axis_tlast,
    output reg  [TEXTURE_STREAM_WIDTH - 1 : 0]  m_texture_steam_tmu0_axis_tdata,
    // TMU1
    output reg          m_texture_steam_tmu1_axis_tvalid,
    input  wire         m_texture_steam_tmu1_axis_tready,
    output reg          m_texture_steam_tmu1_axis_tlast,
    output reg  [TEXTURE_STREAM_WIDTH - 1 : 0]  m_texture_steam_tmu1_axis_tdata,

    // Render Config
    output reg          m_renderer_config_axis_tvalid,
    input  wire         m_renderer_config_axis_tready,
    output reg          m_renderer_config_axis_tlast,
    output reg  [CMD_STREAM_WIDTH - 1 : 0] m_renderer_config_axis_tdata,
    output reg  [ 3 : 0] m_renderer_config_axis_tuser,   

    // Debug
    output wire [ 3 : 0]  dbgStreamState
);
`include "RegisterAndDescriptorDefines.vh"
    localparam DATABUS_SCALE_FACTOR = (CMD_STREAM_WIDTH / 8);
    localparam DATABUS_SCALE_FACTOR_LOG2 = $clog2(DATABUS_SCALE_FACTOR);

    // Command Interface Statemachine
    localparam WAIT_FOR_IDLE = 5'd0;
    localparam COMMAND_IN = 5'd1;
    localparam EXEC_TRIANGLE_STREAM = 5'd2;
    localparam EXEC_TEXTURE0_STREAM = 5'd3;
    localparam EXEC_TEXTURE1_STREAM = 5'd4;
    localparam EXEC_RENDER_CONFIG = 5'd5;
    localparam EXEC_FOG_LUT_STREAM = 5'd6;

    // Wait For Rasterizer Statemachine
    localparam RASTERIZER_CONTROL_WAITFORCOMMAND = 0;
    localparam RASTERIZER_CONTROL_WAITFOREND = 1;

    // Wait For Cache Apply Statemachine
    localparam FB_CONTROL_WAITFORCOMMAND = 0;
    localparam FB_CONTROL_WAITFOREND = 1;

    // Command Unit Variables
    reg             apply;
    wire            applied;
    reg  [18 : 0]   streamCounter;
    reg             parameterComplete;

    // Local Statemachine variables
    reg  [ 4 : 0]   state;
    reg  [ 1 : 0]   rasterizerControlState;
    reg  [ 1 : 0]   fbControlState;
    reg             wlsp = 0;

    assign applied = colorBufferApplied & depthBufferApplied;

    assign dbgStreamState = state[3:0];

    always @(posedge aclk)
    begin
        if (!resetn)
        begin
            state <= WAIT_FOR_IDLE;
            
            rasterizerControlState <= RASTERIZER_CONTROL_WAITFORCOMMAND;

            fbControlState <= FB_CONTROL_WAITFORCOMMAND;
            apply <= 0;
            s_cmd_axis_tready <= 0;

            m_texture_steam_tmu0_axis_tvalid <= 0;
            m_texture_steam_tmu0_axis_tlast <= 0;

            m_texture_steam_tmu1_axis_tvalid <= 0;
            m_texture_steam_tmu1_axis_tlast <= 0;
            
            m_rasterizer_axis_tvalid <= 0;
            m_rasterizer_axis_tlast <= 0;

            m_fog_lut_axis_tvalid <= 0;
            m_fog_lut_axis_tlast <= 0;

            m_renderer_config_axis_tvalid <= 0;
            m_renderer_config_axis_tlast <= 0;

            startRendering <= 0;
        end
        else 
        begin
            case (state)
            WAIT_FOR_IDLE:
            begin
                m_rasterizer_axis_tlast <= 0;
                m_rasterizer_axis_tvalid <= 0;
                m_texture_steam_tmu0_axis_tvalid <= 0;
                m_texture_steam_tmu0_axis_tlast <= 0;
                m_texture_steam_tmu1_axis_tvalid <= 0;
                m_texture_steam_tmu1_axis_tlast <= 0;
                m_fog_lut_axis_tvalid <= 0;
                m_fog_lut_axis_tlast <= 0;
                m_renderer_config_axis_tvalid <= 0;
                m_renderer_config_axis_tlast <= 0;
                if (rasterizerRunning)
                    startRendering <= 0;
                if (m_rasterizer_axis_tready && !m_rasterizer_axis_tlast && !apply && applied && !pixelInPipeline && !rasterizerRunning && !startRendering)
                begin
                    startRendering <= 0;
                    s_cmd_axis_tready <= 1;
                    state <= COMMAND_IN;
                end
            end
            COMMAND_IN:
            begin
                if (s_cmd_axis_tvalid)
                begin
                    // Command decoding
                    case (s_cmd_axis_tdata[OP_POS +: OP_SIZE])
                    OP_TRIANGLE_STREAM:
                    begin
                        /* verilator lint_off WIDTH */
                        streamCounter <= s_cmd_axis_tdata[DATABUS_SCALE_FACTOR_LOG2 +: OP_TRIANGLE_STEEAM_SIZE_SIZE - DATABUS_SCALE_FACTOR_LOG2];
                        /* verilator lint_off WIDTH */
                        state <= EXEC_TRIANGLE_STREAM;
                    end
                    OP_TEXTURE_STREAM:
                    begin
                        streamCounter <= 1 << (s_cmd_axis_tdata[TEXTURE_STREAM_SIZE_POS +: TEXTURE_STREAM_SIZE_SIZE] - DATABUS_SCALE_FACTOR_LOG2);

                        if (|s_cmd_axis_tdata[TEXTURE_STREAM_SIZE_POS +: TEXTURE_STREAM_SIZE_SIZE])
                        begin
                            if (s_cmd_axis_tdata[TEXTURE_STREAM_TMU_NR_POS +: TEXTURE_STREAM_TMU_NR_SIZE] == 0)
                            begin
                                state <= EXEC_TEXTURE0_STREAM;
                            end
                            if (s_cmd_axis_tdata[TEXTURE_STREAM_TMU_NR_POS +: TEXTURE_STREAM_TMU_NR_SIZE] == 1)
                            begin
                                state <= EXEC_TEXTURE1_STREAM;
                            end
                        end
                        else
                        begin
                            s_cmd_axis_tready <= 0;
                            state <= WAIT_FOR_IDLE;    
                        end
                    end
                    OP_FOG_LUT_STREAM:
                    begin
                        streamCounter <= 33;
                        state <= EXEC_FOG_LUT_STREAM;
                    end
                    OP_RENDER_CONFIG:
                    begin
                        streamCounter <= 1;
                        m_renderer_config_axis_tuser <= s_cmd_axis_tdata[0 +: 4];
                        state <= EXEC_RENDER_CONFIG;
                    end
                    OP_FRAMEBUFFER:
                    begin
                        s_cmd_axis_tready <= 0;
                        colorBufferCmdCommit <= s_cmd_axis_tdata[OP_FRAMEBUFFER_COMMIT_POS];
                        colorBufferCmdMemset <= s_cmd_axis_tdata[OP_FRAMEBUFFER_MEMSET_POS];
                        depthBufferCmdCommit <= s_cmd_axis_tdata[OP_FRAMEBUFFER_COMMIT_POS];
                        depthBufferCmdMemset <= s_cmd_axis_tdata[OP_FRAMEBUFFER_MEMSET_POS];
                        colorBufferApply <= s_cmd_axis_tdata[OP_FRAMEBUFFER_COLOR_BUFFER_SELECT_POS];
                        depthBufferApply <= s_cmd_axis_tdata[OP_FRAMEBUFFER_DEPTH_BUFFER_SELECT_POS];
                        apply <= 1;
                        state <= WAIT_FOR_IDLE;
                    end
                    OP_NOP_STREAM:
                    begin
                        s_cmd_axis_tready <= 0;
                        state <= WAIT_FOR_IDLE;
                    end
                    endcase
                    parameterComplete <= 0;
                end
            end
            EXEC_TRIANGLE_STREAM:
            begin
                s_cmd_axis_tready <= m_rasterizer_axis_tready;
                if (m_rasterizer_axis_tready)
                begin
                    m_rasterizer_axis_tvalid <= s_cmd_axis_tvalid;
                    m_rasterizer_axis_tdata <= s_cmd_axis_tdata;
                    if (s_cmd_axis_tvalid)
                    begin
                        streamCounter <= streamCounter - 1;
                        if (streamCounter == 1)
                        begin
                            s_cmd_axis_tready <= 0;
                            m_rasterizer_axis_tlast <= 1;
                            startRendering <= 1;
                            state <= WAIT_FOR_IDLE;
                        end
                    end
                end
            end
            EXEC_TEXTURE0_STREAM:
            begin
                s_cmd_axis_tready <= m_texture_steam_tmu0_axis_tready;
                if (m_texture_steam_tmu0_axis_tready)
                begin
                    m_texture_steam_tmu0_axis_tvalid <= s_cmd_axis_tvalid;
                    m_texture_steam_tmu0_axis_tdata <= s_cmd_axis_tdata[0 +: TEXTURE_STREAM_WIDTH];
                    if (s_cmd_axis_tvalid)
                    begin
                        streamCounter <= streamCounter - 1;
                        if (streamCounter == 1)
                        begin
                            s_cmd_axis_tready <= 0;
                            m_texture_steam_tmu0_axis_tlast <= 1;
                            state <= WAIT_FOR_IDLE;
                        end
                    end
                end
            end
            EXEC_TEXTURE1_STREAM:
            begin
                s_cmd_axis_tready <= m_texture_steam_tmu1_axis_tready;
                if (m_texture_steam_tmu1_axis_tready)
                begin
                    m_texture_steam_tmu1_axis_tvalid <= s_cmd_axis_tvalid;
                    m_texture_steam_tmu1_axis_tdata <= s_cmd_axis_tdata[0 +: TEXTURE_STREAM_WIDTH];
                    if (s_cmd_axis_tvalid)
                    begin
                        streamCounter <= streamCounter - 1;
                        if (streamCounter == 1)
                        begin
                            s_cmd_axis_tready <= 0;
                            m_texture_steam_tmu1_axis_tlast <= 1;
                            state <= WAIT_FOR_IDLE;
                        end
                    end
                end
            end
            EXEC_FOG_LUT_STREAM:
            begin
                s_cmd_axis_tready <= m_fog_lut_axis_tready;
                if (m_fog_lut_axis_tready)
                begin
                    m_fog_lut_axis_tvalid <= s_cmd_axis_tvalid;
                    m_fog_lut_axis_tdata <= s_cmd_axis_tdata;
                    if (s_cmd_axis_tvalid)
                    begin
                        streamCounter <= streamCounter - 1;
                        if (streamCounter == 1)
                        begin
                            s_cmd_axis_tready <= 0;
                            m_fog_lut_axis_tlast <= 1;
                            state <= WAIT_FOR_IDLE;
                        end
                    end
                end
            end
            EXEC_RENDER_CONFIG:
            begin
                s_cmd_axis_tready <= m_renderer_config_axis_tready;
                if (m_renderer_config_axis_tready)
                begin
                    m_renderer_config_axis_tvalid <= s_cmd_axis_tvalid;
                    m_renderer_config_axis_tdata <= s_cmd_axis_tdata;
                    if (s_cmd_axis_tvalid)
                    begin
                        streamCounter <= streamCounter - 1;
                        if (streamCounter == 1)
                        begin
                            s_cmd_axis_tready <= 0;
                            m_renderer_config_axis_tlast <= 1;
                            state <= WAIT_FOR_IDLE;
                        end
                    end
                end
            end
            default:
            begin
            end
            endcase

            case (fbControlState)
            FB_CONTROL_WAITFORCOMMAND:
            begin
                if (apply)
                begin
                    if (applied == 0)
                    begin
                        fbControlState <= FB_CONTROL_WAITFOREND;
                    end
                end
            end
            FB_CONTROL_WAITFOREND:
            begin
                apply <= 0;
                colorBufferApply <= 0;
                depthBufferApply <= 0;
                if (applied)
                begin
                    fbControlState <= FB_CONTROL_WAITFORCOMMAND;
                end
            end
            endcase
        end
    end

endmodule
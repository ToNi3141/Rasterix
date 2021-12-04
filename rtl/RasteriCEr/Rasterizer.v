// RasteriCEr
// https://github.com/ToNi3141/RasteriCEr
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

module Rasterizer
#(
    // This is the fixed screen resolution. In a more advanced implementation, this could
    // be set with a register. This will be necessary when this has to be complaint to OpenGL
    // where it has to render something into a texture
    parameter X_RESOLUTION = 128,
    parameter Y_RESOLUTION = 128,
    parameter Y_LINE_RESOLUTION = Y_RESOLUTION,

    parameter FRAMEBUFFER_INDEX_WIDTH = 14,

    // The bit width of the command interface. Allowed values: 32, 64, 128, 256
    parameter CMD_STREAM_WIDTH = 32
)
(
    input wire                              clk,
    input wire                              reset,
    
    // Rasterizer Control
    output reg                              rasterizerRunning,

    // Triangle Stream
    input  wire                             s_axis_tvalid,
    output reg                              s_axis_tready,
    input  wire                             s_axis_tlast,
    input  wire [CMD_STREAM_WIDTH - 1 : 0]  s_axis_tdata,

    // Fragment Stream
    output reg                              m_axis_tvalid,
    input  wire                             m_axis_tready,
    output reg                              m_axis_tlast,
    output reg  [RASTERIZER_AXIS_PARAMETER_SIZE - 1 : 0] m_axis_tdata
);
`include "RasterizerDefines.vh"
`include "RegisterAndDescriptorDefines.vh"

    localparam PARAMETER_WIDTH = RASTERIZER_AXIS_VERTEX_ATTRIBUTE_SIZE;
    localparam PARAMETERS_PER_STREAM_BEAT = CMD_STREAM_WIDTH / PARAMETER_WIDTH;
    localparam X_BIT_WIDTH = $clog2(X_RESOLUTION) + 1;
    localparam Y_BIT_WIDTH = $clog2(Y_RESOLUTION) + 1;

    // Rasterizer main state machine
    localparam RASTERIZER_WAITFORCOMMAND = 0;
    localparam RASTERIZER_COPY = 1;
    localparam RASTERIZER_INIT = 2;
    localparam RASTERIZER_TEST = 3;
    localparam RASTERIZER_INC_H = 5;

    // Rasterizer edge walker state machine
    localparam RASTERIZER_EDGEWALKER_SEARCH_EDGE = 0;
    localparam RASTERIZER_EDGEWALKER_WALK_OUT = 1;
    localparam RASTERIZER_EDGEWALKER_WALK = 2;
    localparam RASTERIZER_EDGEWALKER_INIT = 3;
    localparam RASTERIZER_EDGEWALKER_CHECK_WALKING_DIR = 4;

    // Triangle and Texture Data
    reg [PARAMETER_WIDTH - 1 : 0] paramMem [0 : `GET_TRIANGLE_SIZE_FOR_BUS_WIDTH(CMD_STREAM_WIDTH) - 1];
    reg [5 : 0] rasterizerCopyCounter;
    reg [4 : 0] paramIndex;
    reg parameterComplete;
    
    // Rasterizer variables
    reg [5 : 0] rasterizerState;
    reg [Y_BIT_WIDTH - 1 : 0] y;
    reg [X_BIT_WIDTH - 1 : 0] x;
    reg [FRAMEBUFFER_INDEX_WIDTH - 1 : 0] fbIndexTmp;
    wire isInTriangle = !(paramMem[INC_W0][31] | paramMem[INC_W1][31] | paramMem[INC_W2][31]);
    wire isInTriangleAndInBounds = isInTriangle & (x < paramMem[BB_END][BB_X_POS +: X_BIT_WIDTH]) & (x >= paramMem[BB_START][BB_X_POS +: X_BIT_WIDTH]);
    /* verilator lint_off WIDTH */
    wire [FRAMEBUFFER_INDEX_WIDTH - 1 : 0] fbIndex = (((Y_LINE_RESOLUTION - 1) - y) * X_RESOLUTION) + x;
    /* verilator lint_on WIDTH */

    // Edge walker variables
    reg [5 : 0] edgeWalkingState;
    reg edgeWalkTryOtherside;
    reg edgeWalkingDirection; 
    localparam EDGE_WALKING_DIRECTION_LEFT = 1'b0;
    localparam EDGE_WALKING_DIRECTION_RIGHT = 1'b1;

    always @(posedge clk)
    begin
        if (reset)
        begin
            s_axis_tready <= 0;
            m_axis_tlast <= 0;
            m_axis_tvalid <= 0;
            rasterizerRunning <= 0;
            rasterizerState <= RASTERIZER_WAITFORCOMMAND;
        end
        else
        begin
            case (rasterizerState)
            RASTERIZER_WAITFORCOMMAND:
            begin
                m_axis_tvalid <= 0;
                paramIndex <= 0;
                parameterComplete <= 0;
                s_axis_tready <= 1;
                rasterizerState <= RASTERIZER_COPY;
            end
            RASTERIZER_COPY:
            begin : RasterizerCopy
                integer i;
                if (s_axis_tvalid)
                begin
                    for (i = 0; i < PARAMETERS_PER_STREAM_BEAT; i = i + 1)
                    begin
                        paramMem[paramIndex + i[0 +: 5]] <= s_axis_tdata[PARAMETER_WIDTH * i +: PARAMETER_WIDTH];
                    end
                    paramIndex <= paramIndex + PARAMETERS_PER_STREAM_BEAT[0 +: 5];

                    if (s_axis_tlast)
                    begin
                        s_axis_tready <= 0;
                        rasterizerRunning <= 1;
                        rasterizerState <= RASTERIZER_INIT;
                    end
                end
            end
            RASTERIZER_INIT:
            begin
                x <= paramMem[BB_START][BB_X_POS +: X_BIT_WIDTH];
                y <= paramMem[BB_START][BB_Y_POS +: Y_BIT_WIDTH];

                // Initialize the edge walker
                edgeWalkingDirection <= EDGE_WALKING_DIRECTION_RIGHT;
                edgeWalkTryOtherside <= 0;

                // Start rasterization
                m_axis_tvalid <= 0;
                edgeWalkingState <= RASTERIZER_EDGEWALKER_INIT;
                rasterizerState <= RASTERIZER_TEST;
            end
            RASTERIZER_TEST:
            begin
                // A rasterization cycle is only executed if the shader is free. Otherwise the rasterizer will stall
                if (m_axis_tready)
                begin
                    // General walking algorithm diagram:
                    //
                    //                      +Bounding Box----------------+
                    //                      |   Triangle to rasterize    |
                    //                      |   +------+                 |
                    //                      |   |      |                 |
                    //   Arrow to show  +-------> +----> +->+            |  Walk right
                    //   walking direction  |   |      |    |            |
                    //                      |   +-----------v-+          |
                    //                      |   |      |    +----v       |  Check if inside of the triangle (in this case yes, so walk out (or till we get to the limit of the bounding box))
                    //                      | + <----+ <----+ <--+       |  Walk left
                    //                      | | |      |      |          |
                    //                      | | +--------------------+   |
                    //                      | | |      |      |      |   |  Check if inside of the triangle (in this case not)
                    //                      | +-> +----> +----> +---->   |  Walk right
                    //                      |   |      |      |      |   |
                    //                      |   +--------------------+   |
                    //                      |                            |
                    //                      +----------------------------+

                    // Triangle increments
                    if (edgeWalkingState == RASTERIZER_EDGEWALKER_CHECK_WALKING_DIR)
                    begin
                        // Do nothing here, just avoid an increment.
                        // It is convinient to do that when we are checking the new direction,
                        // because in 50% of the cases, we are walking in the wrong direction
                        // anyway, so this gives us no advantage, but when we just keep walking
                        // we risk an over or underflow of x.
                    end 
                    else if ((edgeWalkingState == RASTERIZER_EDGEWALKER_WALK) & !isInTriangleAndInBounds)
                    begin
                        // Line Increment
                        y <= y + 1;

                        paramMem[INC_W0] <= paramMem[INC_W0] + $signed(paramMem[INC_W0_Y]);
                        paramMem[INC_W1] <= paramMem[INC_W1] + $signed(paramMem[INC_W1_Y]);
                        paramMem[INC_W2] <= paramMem[INC_W2] + $signed(paramMem[INC_W2_Y]);
                    end
                    else 
                    begin
                        if (edgeWalkingDirection == EDGE_WALKING_DIRECTION_RIGHT)
                        begin
                            // Pixel Increment
                            x <= x + 1;

                            paramMem[INC_W0] <= paramMem[INC_W0] + $signed(paramMem[INC_W0_X]);
                            paramMem[INC_W1] <= paramMem[INC_W1] + $signed(paramMem[INC_W1_X]);
                            paramMem[INC_W2] <= paramMem[INC_W2] + $signed(paramMem[INC_W2_X]);
                        end
                        else
                        begin
                            // Pixel Decrement
                            x <= x - 1;

                            paramMem[INC_W0] <= paramMem[INC_W0] - $signed(paramMem[INC_W0_X]);
                            paramMem[INC_W1] <= paramMem[INC_W1] - $signed(paramMem[INC_W1_X]);
                            paramMem[INC_W2] <= paramMem[INC_W2] - $signed(paramMem[INC_W2_X]);
                        end
                    end

                    if ((y < paramMem[BB_END][BB_Y_POS +: Y_BIT_WIDTH]) && (y < Y_LINE_RESOLUTION))
                    begin
                        case (edgeWalkingState)
                        RASTERIZER_EDGEWALKER_INIT:
                        begin
                            // Check if the first pixel is already in the triangle
                            if (isInTriangle)
                            begin
                                // If yes, then there is nothing to do. We are already at position (0, 0)
                                m_axis_tvalid <= 1;
                                edgeWalkingState <= RASTERIZER_EDGEWALKER_WALK;
                            end
                            else
                            begin
                                // If not, search the edge
                                m_axis_tvalid <= 0;
                                edgeWalkingState <= RASTERIZER_EDGEWALKER_SEARCH_EDGE;
                            end
                        end
                        RASTERIZER_EDGEWALKER_CHECK_WALKING_DIR:
                        begin
                            // Check if after a line increment the pixel is inside the triangle
                            if (isInTriangle)
                            begin
                                // If yes, walk out. It will continue walking in the old direction, this should be closest to the edge
                                // Improvement: Save this position inside in the triangle. Also during walk out it is possible to render this pixel.
                                //      currently we are wasting just clock cycles. Normaly, the pixel are really not far away from the edge.
                                edgeWalkingState <= RASTERIZER_EDGEWALKER_WALK_OUT;
                            end
                            else
                            begin
                                // The current pixel is outside of the triangle. We assume, that the triangle is always on the opposite direction.
                                // This assumption is most of the time true, but there are edge cases, where this is wrong. This edge cases are handled 
                                // in the RASTERIZER_EDGEWALKER_SEARCH_EDGE state.
                                edgeWalkingDirection <= !edgeWalkingDirection; // Change walking direction
                                edgeWalkingState <= RASTERIZER_EDGEWALKER_SEARCH_EDGE;
                            end
                        end
                        RASTERIZER_EDGEWALKER_SEARCH_EDGE:
                        begin
                            if (isInTriangleAndInBounds)
                            begin
                                // The triangle is withing it bounds and everything is fine. So, just shade the pixel
                                edgeWalkTryOtherside <= 0;
                                m_axis_tvalid <= 1; // To prevent, that the first pixel of the triangle is skipped
                                edgeWalkingState <= RASTERIZER_EDGEWALKER_WALK;
                            end
                            else if (x == paramMem[BB_END][BB_X_POS +: X_BIT_WIDTH])
                            begin
                                // The rasterizer reaches the end of the bounding box and has to handle this now. There are now to possible cases:
                                //      Easiest case: Rasterizer was iterating from the left to the right, and there was no triangle on the way.
                                //          In this case, we could just do a line increment.
                                //      Edge Case: Normally we assume, that after a line increment, the current position is near the triangle or in the 
                                //          triangel and  when we change direction, that we hit the triangle. That is for most of the triangles true. But 
                                //          in some cases it is wrong. For instance, walking from left to right. After we run out of the triangle, we 
                                //          make our line increment. We assume now, that after the line increment, the triangle should be on the left side.
                                //          But this is not always true. In some cases when we are on the edge points of the triangle, it can happen, that
                                //          the triangle is even after a line increment on the right side. That is something we have to cover, otherwise
                                //          some triangles are not completely rendered.
                                //          To cover this, the variable edgeWalkTryOtherside is introduced. If we reach the end of the bounding box without
                                //          rendering a triangle, then we switch the walking direction and try again to find the triangle until we reaching
                                //          the beginning of the bounding box. If the triangle would be on the wrong side, we would find it now. Otherwise
                                //          we are sure, that there is no triangle on this line and we can trigger a line increment.
                                //      Imrpovement: Similar to RASTERIZER_EDGEWALKER_WALK_OUT we could save our starting point and could reset to this point
                                //          if we don't find a triangle. This would save cycles because we a don't check pixel twice. Currenlty, (in an extrem
                                //          case) we would travers from left to right and back. That means, we check all pixels in a line twice.
                                if ((edgeWalkingDirection == EDGE_WALKING_DIRECTION_RIGHT) & edgeWalkTryOtherside)
                                begin
                                    // No triangle in the line found, so trigger a line increment
                                    edgeWalkTryOtherside <= 0;
                                    edgeWalkingState <= RASTERIZER_EDGEWALKER_WALK;
                                end
                                else 
                                begin
                                    // No line in the triangle found. But here the rasterizers assumes, that it does not start from the beginning, so 
                                    // it tries to walk also into the other direction
                                    edgeWalkTryOtherside <= 1;
                                    edgeWalkingDirection <= EDGE_WALKING_DIRECTION_LEFT;
                                end
                            end 
                            else if (x == paramMem[BB_START][BB_X_POS +: X_BIT_WIDTH])
                            begin
                                // This case is similar to the case above. It handles just the other direction
                                if ((edgeWalkingDirection == EDGE_WALKING_DIRECTION_LEFT) & edgeWalkTryOtherside)
                                begin
                                    // No triangle in the line found, so trigger a line increment
                                    edgeWalkTryOtherside <= 0;
                                    edgeWalkingState <= RASTERIZER_EDGEWALKER_WALK;
                                end
                                else 
                                begin
                                    // No line in the triangle found. But here the rasterizers assumes, that it does not start from the beginning, so 
                                    // it tries to walk also into the other direction
                                    edgeWalkTryOtherside <= 1;
                                    edgeWalkingDirection <= EDGE_WALKING_DIRECTION_RIGHT;
                                end
                            end
                        end
                        RASTERIZER_EDGEWALKER_WALK_OUT:
                        begin
                            // Walk out of the triangle. To improve the performance: If the rasterizer could save the starting point,
                            // it could also shade pixel while walking out, and if it is out reset to this point, switch direction
                            // and shade the left pixels. But this would again occupy arround 400 luts.
                            if (!isInTriangle | (x == paramMem[BB_START][BB_X_POS +: X_BIT_WIDTH]) | (x >= paramMem[BB_END][BB_X_POS +: X_BIT_WIDTH]))
                            begin                             
                                // Change the walking direction and shade
                                edgeWalkingDirection <= !edgeWalkingDirection;
                                edgeWalkingState <= RASTERIZER_EDGEWALKER_SEARCH_EDGE;
                            end
                        end
                        RASTERIZER_EDGEWALKER_WALK:
                        begin
                            // Render pixels
                                if (isInTriangleAndInBounds)
                                begin
                                    m_axis_tvalid <= 1;
                                end
                                else
                                begin
                                    // Now we are outside on the left side of the triangle.
                                    // The edge walker will now search again the left edge
                                    m_axis_tvalid <= 0;
                                    edgeWalkingState <= RASTERIZER_EDGEWALKER_CHECK_WALKING_DIR;
                                end
                        end
                        endcase
                    end
                    else
                    begin
                        // Now the edge walker is below the triangle. No Triangle hit is expected anymore.
                        // That means, the edge walking is aborted.
                        m_axis_tvalid <= 0;
                        rasterizerRunning <= 0;
                        rasterizerState <= RASTERIZER_WAITFORCOMMAND;
                    end

                    // Arguments for the shader
                    m_axis_tdata <= {{{(RASTERIZER_AXIS_VERTEX_ATTRIBUTE_SIZE - FRAMEBUFFER_INDEX_WIDTH){1'b0}}, fbIndex},
                        paramMem[TRIANGLE_CONFIGURATION],
                        {{{(16 - Y_BIT_WIDTH){1'b0}}, y} - paramMem[BB_START][BB_Y_POS +: 16], {{(16 - X_BIT_WIDTH){1'b0}}, x}} - paramMem[BB_START][BB_X_POS +: 16],
                        paramMem[INC_DEPTH_W_Y],
                        paramMem[INC_DEPTH_W_X],
                        paramMem[INC_DEPTH_W],
                        paramMem[INC_TEX_T_Y],
                        paramMem[INC_TEX_T_X],
                        paramMem[INC_TEX_T],
                        paramMem[INC_TEX_S_Y],
                        paramMem[INC_TEX_S_X],
                        paramMem[INC_TEX_S]
                    };
                end
            end
            endcase 
        end
    end
endmodule
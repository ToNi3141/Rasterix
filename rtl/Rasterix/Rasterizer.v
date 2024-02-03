// Rasterix
// https://github.com/ToNi3141/Rasterix
// Copyright (c) 2023 ToNi3141

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
    // The maximum size of the screen in power of two
    parameter X_BIT_WIDTH = 11,
    parameter Y_BIT_WIDTH = 11,

    parameter INDEX_WIDTH = 14,

    // The bit width of the command interface. Allowed values: 32, 64, 128, 256
    parameter CMD_STREAM_WIDTH = 32,

    parameter ENABLE_INITIAL_Y_INC = 1,

    localparam ATTRIBUTE_SIZE = 32,

    localparam KEEP_WIDTH = 1
)
(
    input wire                              clk,
    input wire                              reset,
    
    // Rasterizer Control
    output reg                              rasterizerRunning,
    input  wire                             startRendering,

    // Rasterizer config
    input  wire [Y_BIT_WIDTH - 1 : 0]       yOffset,
    input  wire [X_BIT_WIDTH - 1 : 0]       xResolution,
    input  wire [Y_BIT_WIDTH - 1 : 0]       yResolution,

    // Triangle Attributes
    input  wire [ATTRIBUTE_SIZE - 1 : 0]    bbStart,
    input  wire [ATTRIBUTE_SIZE - 1 : 0]    bbEnd,
    input  wire [ATTRIBUTE_SIZE - 1 : 0]    w0,
    input  wire [ATTRIBUTE_SIZE - 1 : 0]    w1,
    input  wire [ATTRIBUTE_SIZE - 1 : 0]    w2,
    input  wire [ATTRIBUTE_SIZE - 1 : 0]    w0IncX,
    input  wire [ATTRIBUTE_SIZE - 1 : 0]    w1IncX,
    input  wire [ATTRIBUTE_SIZE - 1 : 0]    w2IncX,
    input  wire [ATTRIBUTE_SIZE - 1 : 0]    w0IncY,
    input  wire [ATTRIBUTE_SIZE - 1 : 0]    w1IncY,
    input  wire [ATTRIBUTE_SIZE - 1 : 0]    w2IncY,

    // Fragment Stream
    output reg                              m_rr_tvalid,
    input  wire                             m_rr_tready,
    output reg                              m_rr_tlast,
    output reg                              m_rr_tpixel,
    output reg  [X_BIT_WIDTH - 1 : 0]       m_rr_tbbx,
    output reg  [Y_BIT_WIDTH - 1 : 0]       m_rr_tbby,
    output reg  [X_BIT_WIDTH - 1 : 0]       m_rr_tspx,
    output reg  [Y_BIT_WIDTH - 1 : 0]       m_rr_tspy,
    output reg  [INDEX_WIDTH - 1 : 0]       m_rr_tindex,
    output reg  [KEEP_WIDTH - 1 : 0]        m_rr_tkeep,
    output reg  [ 1 : 0]                    m_rr_tcmd
);
`include "RasterizerCommands.vh"

    localparam BB_X_POS = 0;
    localparam BB_Y_POS = 16;

    localparam PARAMETERS_PER_STREAM_BEAT = CMD_STREAM_WIDTH / ATTRIBUTE_SIZE;

    // Rasterizer main state machine
    localparam RASTERIZER_WAITFORCOMMAND = 0;
    localparam RASTERIZER_INIT = 1;
    localparam RASTERIZER_TEST = 2;

    // Rasterizer edge walker state machine
    localparam RASTERIZER_EDGEWALKER_SEARCH_EDGE = 0;
    localparam RASTERIZER_EDGEWALKER_WALK_OUT = 1;
    localparam RASTERIZER_EDGEWALKER_WALK = 2;
    localparam RASTERIZER_EDGEWALKER_INIT = 3;
    localparam RASTERIZER_EDGEWALKER_CHECK_WALKING_DIR = 4;

    // Rasterizer variables
    wire [Y_BIT_WIDTH - 1 : 0] yLineResolution = yResolution;
    reg  [5 : 0] rasterizerState;
    reg  [Y_BIT_WIDTH - 1 : 0] y;
    reg  [Y_BIT_WIDTH - 1 : 0] yScreen;
    reg  [Y_BIT_WIDTH - 1 : 0] yScreenEnd;
    reg  [Y_BIT_WIDTH - 1 : 0] lineBBStart;
    reg  [X_BIT_WIDTH - 1 : 0] x;
    wire isInTriangle = !(regW0[31] | regW1[31] | regW2[31]);
    wire isInTriangleAndInBounds = isInTriangle && (x < bbEnd[BB_X_POS +: X_BIT_WIDTH]) && (x >= bbStart[BB_X_POS +: X_BIT_WIDTH]);
    reg  [ATTRIBUTE_SIZE - 1 : 0] regW0;
    reg  [ATTRIBUTE_SIZE - 1 : 0] regW1;
    reg  [ATTRIBUTE_SIZE - 1 : 0] regW2;

    // Edge walker variables
    reg  [5 : 0] edgeWalkingState;
    reg  edgeWalkTryOtherside;
    reg  edgeWalkingDirection; 
    localparam EDGE_WALKING_DIRECTION_LEFT = 1'b0;
    localparam EDGE_WALKING_DIRECTION_RIGHT = 1'b1;

    always @(posedge clk)
    begin
        if (reset)
        begin
            m_rr_tlast <= 0;
            m_rr_tvalid <= 0;
            m_rr_tkeep <= ~0;
            rasterizerRunning <= 0;
            rasterizerState <= RASTERIZER_WAITFORCOMMAND;
        end
        else
        begin
            case (rasterizerState)
            RASTERIZER_WAITFORCOMMAND:
            begin
                m_rr_tvalid <= 0;
                m_rr_tpixel <= 0;
                m_rr_tkeep <= ~0;
                m_rr_tlast <= 0;
                rasterizerRunning <= 0;
                if (startRendering)
                begin
                    lineBBStart <= yOffset - bbStart[BB_Y_POS +: Y_BIT_WIDTH];
                    rasterizerRunning <= 1;
                    rasterizerState <= RASTERIZER_INIT;
                    m_rr_tcmd <= RR_CMD_INIT;
                end
            end
            RASTERIZER_INIT:
            begin
                x <= bbStart[BB_X_POS +: X_BIT_WIDTH];

                if (ENABLE_INITIAL_Y_INC)
                begin
                    // Shift the triangle to the current framebuffer line. Everything can be calculated in software if this implementation
                    // takes too much logic. It can be completely discarded, when the framebuffer is big enough to contain the whole screen. This is only 
                    // required in the line mode, to handle the offsets in y direction when rendering a new line.
                    // Check if the current line offset is above the bounding box. Means, the bounding box starts in this line or in lines after this line.
                    // In any case, set the current yScreen coord to the bounding box start position. If the bounding box start possition is in this
                    // line, then everything is fine. If not, then yScreen will be below yScreenEnd and the rendering of the current triangle is discarded
                    // for this line.
                    if (yOffset <= bbStart[BB_Y_POS +: Y_BIT_WIDTH])
                    begin
                        regW0 <= w0;
                        regW1 <= w1;
                        regW2 <= w2;
                        
                        yScreen <= bbStart[BB_Y_POS +: Y_BIT_WIDTH];
                        y <= bbStart[BB_Y_POS +: Y_BIT_WIDTH] - yOffset;
                    end
                    else
                    begin
                        regW0 <= w0 + ($signed(w0IncY) * lineBBStart);
                        regW1 <= w1 + ($signed(w1IncY) * lineBBStart);
                        regW2 <= w2 + ($signed(w2IncY) * lineBBStart);

                        yScreen <= yOffset;
                        y <= 0;
                    end
                    // Check if the bounding box ends in this line. If not, clamp the bounding box end to the end of the current line.
                    // If the bounding box end in this line, or in a previous line, just set yScreenEnd to the end of the bounding box.
                    // The the condition occures that yScreenEnd is smaller than yScreen which results in discarding the triangle for this line.
                    if ((yOffset + yLineResolution) <= bbEnd[BB_Y_POS +: Y_BIT_WIDTH])
                    begin
                        yScreenEnd <= yOffset + yLineResolution;
                    end
                    else
                    begin
                        yScreenEnd <= bbEnd[BB_Y_POS +: Y_BIT_WIDTH];
                    end
                end
                else
                begin
                    regW0 <= w0;
                    regW1 <= w1;
                    regW2 <= w2;

                    yScreen <= bbStart[BB_Y_POS +: Y_BIT_WIDTH];
                    y <= bbStart[BB_Y_POS +: Y_BIT_WIDTH];

                    if (yLineResolution <= bbEnd[BB_Y_POS +: Y_BIT_WIDTH])
                    begin
                        yScreenEnd <= yLineResolution;
                    end
                    else
                    begin
                        yScreenEnd <= bbEnd[BB_Y_POS +: Y_BIT_WIDTH];
                    end
                end

                // Initialize the edge walker
                edgeWalkingDirection <= EDGE_WALKING_DIRECTION_RIGHT;
                edgeWalkTryOtherside <= 0;

                // Start rasterization
                m_rr_tvalid <= 1;
                edgeWalkingState <= RASTERIZER_EDGEWALKER_INIT;
                rasterizerState <= RASTERIZER_TEST;
            end
            RASTERIZER_TEST:
            begin
                // A rasterization cycle is only executed if the shader is free. Otherwise the rasterizer will stall
                if (m_rr_tready)
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
                        m_rr_tvalid <= 0;
                    end 
                    else if ((edgeWalkingState == RASTERIZER_EDGEWALKER_WALK) & !isInTriangleAndInBounds)
                    begin
                        // Line Increment
                        y <= y + 1;
                        yScreen <= yScreen + 1;

                        regW0 <= regW0 + $signed(w0IncY);
                        regW1 <= regW1 + $signed(w1IncY);
                        regW2 <= regW2 + $signed(w2IncY);

                        m_rr_tcmd <= RR_CMD_Y_INC;
                        m_rr_tvalid <= 1;
                    end
                    else 
                    begin
                        if (edgeWalkingDirection == EDGE_WALKING_DIRECTION_RIGHT)
                        begin
                            // Pixel Increment
                            x <= x + 1;

                            regW0 <= regW0 + $signed(w0IncX);
                            regW1 <= regW1 + $signed(w1IncX);
                            regW2 <= regW2 + $signed(w2IncX);

                            m_rr_tcmd <= RR_CMD_X_INC;
                            m_rr_tvalid <= 1;
                        end
                        else
                        begin
                            // Pixel Decrement
                            x <= x - 1;

                            regW0 <= regW0 - $signed(w0IncX);
                            regW1 <= regW1 - $signed(w1IncX);
                            regW2 <= regW2 - $signed(w2IncX);

                            m_rr_tcmd <= RR_CMD_X_DEC;
                            m_rr_tvalid <= 1;
                        end
                    end

                    if (yScreen < yScreenEnd)
                    begin
                        case (edgeWalkingState)
                        RASTERIZER_EDGEWALKER_INIT:
                        begin
                            // Check if the first pixel is already in the triangle
                            if (isInTriangle)
                            begin
                                // If yes, then there is nothing to do. We are already at position (0, 0)
                                m_rr_tpixel <= 1;
                                edgeWalkingState <= RASTERIZER_EDGEWALKER_WALK;
                            end
                            else
                            begin
                                // If not, search the edge
                                m_rr_tpixel <= 0;
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
                                m_rr_tpixel <= 1; // To prevent, that the first pixel of the triangle is skipped
                                edgeWalkingState <= RASTERIZER_EDGEWALKER_WALK;
                            end
                            else if (x == bbEnd[BB_X_POS +: X_BIT_WIDTH])
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
                            else if (x == bbStart[BB_X_POS +: X_BIT_WIDTH])
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
                            if (!isInTriangle | (x == bbStart[BB_X_POS +: X_BIT_WIDTH]) | (x >= bbEnd[BB_X_POS +: X_BIT_WIDTH]))
                            begin                             
                                // Change the walking direction and shade
                                edgeWalkingDirection <= !edgeWalkingDirection;
                                edgeWalkingState <= RASTERIZER_EDGEWALKER_SEARCH_EDGE;
                            end
                        end
                        RASTERIZER_EDGEWALKER_WALK:
                        begin
                            // Render pixels
                            if (!isInTriangleAndInBounds)
                            begin
                                // Now we are outside on the left side of the triangle.
                                // The edge walker will now search again the left edge
                                edgeWalkingState <= RASTERIZER_EDGEWALKER_CHECK_WALKING_DIR;
                            end
                            m_rr_tpixel <= isInTriangleAndInBounds;
                        end
                        endcase
                    end
                    else
                    begin
                        // Now the edge walker is below the triangle. No Triangle hit is expected anymore.
                        // That means, the edge walking is aborted.
                        m_rr_tpixel <= 1;
                        m_rr_tkeep <= 0;
                        m_rr_tlast <= 1;
                        rasterizerState <= RASTERIZER_WAITFORCOMMAND;
                    end

                    /* verilator lint_off WIDTH */
                    // Check that the index never exceeds the borders of the view port
                    if ((y < yResolution) && (x < xResolution))
                    begin
                        m_rr_tindex <= (((yLineResolution - 1) - y) * xResolution) + x;
                    end
                    /* verilator lint_on WIDTH */
                    
                    // Arguments for the shader
                    m_rr_tbbx <= (x - bbStart[BB_X_POS +: X_BIT_WIDTH]);
                    m_rr_tbby <= (yScreen - bbStart[BB_Y_POS +: Y_BIT_WIDTH]);
                    m_rr_tspx <= x;
                    m_rr_tspy <= yScreen;
                end
            end
            endcase 
        end
    end
endmodule
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

#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "3rdParty/catch.hpp"

// Include common routines
#include <verilated.h>

// Include model header, generated from Verilating "top.v"
#include "VAttributeInterpolator.h"

static constexpr uint32_t RASTERIZER_AXIS_INC_TEXTURE_S_POS = 0;
static constexpr uint32_t RASTERIZER_AXIS_INC_TEXTURE_S_X_POS = 1;
static constexpr uint32_t RASTERIZER_AXIS_INC_TEXTURE_S_Y_POS = 2;
static constexpr uint32_t RASTERIZER_AXIS_INC_TEXTURE_T_POS = 3;
static constexpr uint32_t RASTERIZER_AXIS_INC_TEXTURE_T_X_POS = 4;
static constexpr uint32_t RASTERIZER_AXIS_INC_TEXTURE_T_Y_POS = 5;
static constexpr uint32_t RASTERIZER_AXIS_INC_DEPTH_W_POS = 6;
static constexpr uint32_t RASTERIZER_AXIS_INC_DEPTH_W_X_POS = 7;
static constexpr uint32_t RASTERIZER_AXIS_INC_DEPTH_W_Y_POS = 8;
static constexpr uint32_t RASTERIZER_AXIS_SCREEN_POS_POS = 9;
static constexpr uint32_t RASTERIZER_AXIS_TRIANGLE_COLOR_POS = 10;
static constexpr uint32_t RASTERIZER_AXIS_FRAMEBUFFER_INDEX = 11;

static constexpr uint32_t RASTERIZER_M_AXIS_S = 0;
static constexpr uint32_t RASTERIZER_M_AXIS_T = 1;
static constexpr uint32_t RASTERIZER_M_AXIS_W = 2;
static constexpr uint32_t RASTERIZER_M_AXIS_TRIANGLE_COLOR = 3;
static constexpr uint32_t RASTERIZER_M_AXIS_FRAMEBUFFER_INDEX = 4;

union ScreenPos {
    uint32_t u32;
    struct {
        uint16_t x;
        uint16_t y;
    } val;
};

void clk(VAttributeInterpolator* t)
{
    t->clk = 0;
    t->eval();
    t->clk = 1;
    t->eval();
}

void calculateVertexAttributes(float sb,
                               float sx, 
                               float sy,
                               float tb, 
                               float tx, 
                               float ty,
                               float wb, 
                               float wx, 
                               float wy, 
                               uint16_t x,
                               uint16_t y, 
                               float& s, 
                               float& t, 
                               float& w)
{
    sx *= x;
    sy *= y;
    tx *= x;
    ty *= y;
    wx *= x;
    wy *= y;

    w = wb + wx + wy;
    s = sb + sx + sy;
    t = tb + tx + ty;

    w = 1.0f / w;

    s *= w;
    t *= w;
}

TEST_CASE("Check the interpolation through the pipeline", "[AttributeInterpolator]")
{
    static const uint32_t CLOCK_DELAY = 32;
    VAttributeInterpolator* top = new VAttributeInterpolator();

    // Reset the pipeline
    for (int i = 0; i < CLOCK_DELAY; i++)
    {
        top->s_axis_tdata[RASTERIZER_AXIS_INC_TEXTURE_S_POS] = 0;
        top->s_axis_tdata[RASTERIZER_AXIS_INC_TEXTURE_S_X_POS] = 0;
        top->s_axis_tdata[RASTERIZER_AXIS_INC_TEXTURE_S_Y_POS] = 0;
        top->s_axis_tdata[RASTERIZER_AXIS_INC_TEXTURE_T_POS] = 0;
        top->s_axis_tdata[RASTERIZER_AXIS_INC_TEXTURE_T_X_POS] = 0;
        top->s_axis_tdata[RASTERIZER_AXIS_INC_TEXTURE_T_Y_POS] = 0;
        top->s_axis_tdata[RASTERIZER_AXIS_INC_DEPTH_W_POS] = 0;
        top->s_axis_tdata[RASTERIZER_AXIS_INC_DEPTH_W_X_POS] = 0;
        top->s_axis_tdata[RASTERIZER_AXIS_INC_DEPTH_W_Y_POS] = 0;

        top->s_axis_tdata[RASTERIZER_AXIS_SCREEN_POS_POS] = 0;

        top->s_axis_tdata[RASTERIZER_AXIS_TRIANGLE_COLOR_POS] = 0;

        top->s_axis_tdata[RASTERIZER_AXIS_FRAMEBUFFER_INDEX] = 0;

        top->s_axis_tvalid = 0;
        top->s_axis_tlast = 0;
    }
    REQUIRE(top->s_axis_tdata[RASTERIZER_M_AXIS_S] == 0);
    REQUIRE(top->s_axis_tdata[RASTERIZER_M_AXIS_T] == 0);
    REQUIRE(top->s_axis_tdata[RASTERIZER_M_AXIS_W] == 0);
    REQUIRE(top->s_axis_tdata[RASTERIZER_M_AXIS_TRIANGLE_COLOR] == 0);
    REQUIRE(top->s_axis_tdata[RASTERIZER_M_AXIS_FRAMEBUFFER_INDEX] == 0);
    REQUIRE(top->m_axis_tvalid == 0);
    REQUIRE(top->m_axis_tlast == 0);
    REQUIRE(top->pixelInPipeline == 0);

    ScreenPos sp;
    sp.val.x = 0;
    sp.val.y = 100;

    float sb = 3.0;
    float sx = 0.50;
    float sy = 0.25;
    float tb = 4.0;
    float tx = 0.20;
    float ty = 0.10;
    float wb = 0.06754551082849503;
    float wx = -1.5070709196152166e-05;
    float wy = 3.105480573140085e-05;
    uint16_t triangleColor = 123;

    // Set the vertex attributes
    top->s_axis_tdata[RASTERIZER_AXIS_INC_TEXTURE_S_POS] = *(uint32_t*)&sb;
    top->s_axis_tdata[RASTERIZER_AXIS_INC_TEXTURE_S_X_POS] = *(uint32_t*)&sx;
    top->s_axis_tdata[RASTERIZER_AXIS_INC_TEXTURE_S_Y_POS] = *(uint32_t*)&sy;
    top->s_axis_tdata[RASTERIZER_AXIS_INC_TEXTURE_T_POS] = *(uint32_t*)&tb;
    top->s_axis_tdata[RASTERIZER_AXIS_INC_TEXTURE_T_X_POS] = *(uint32_t*)&tx;
    top->s_axis_tdata[RASTERIZER_AXIS_INC_TEXTURE_T_Y_POS] = *(uint32_t*)&ty;
    top->s_axis_tdata[RASTERIZER_AXIS_INC_DEPTH_W_POS] = *(uint32_t*)&wb;
    top->s_axis_tdata[RASTERIZER_AXIS_INC_DEPTH_W_X_POS] = *(uint32_t*)&wx;
    top->s_axis_tdata[RASTERIZER_AXIS_INC_DEPTH_W_Y_POS] = *(uint32_t*)&wy;
    top->s_axis_tdata[RASTERIZER_AXIS_SCREEN_POS_POS] = sp.u32;
    top->s_axis_tdata[RASTERIZER_AXIS_TRIANGLE_COLOR_POS] = triangleColor;
    top->s_axis_tvalid = 1;
    top->s_axis_tlast = 0;

    // Check the pipeline with 100 pixel
    for (int i = 0; i < 100; i++)
    {
        clk(top);
        if (i > CLOCK_DELAY)
        {
            REQUIRE(top->m_axis_tdata[RASTERIZER_M_AXIS_FRAMEBUFFER_INDEX] == i - CLOCK_DELAY);
            REQUIRE(top->m_axis_tdata[RASTERIZER_M_AXIS_TRIANGLE_COLOR] == 123);
            REQUIRE(top->m_axis_tvalid == 1);
            REQUIRE(top->m_axis_tlast == (i - CLOCK_DELAY) % 5);

            float s = *(float*)&(top->m_axis_tdata[RASTERIZER_M_AXIS_S]);
            float t = *(float*)&(top->m_axis_tdata[RASTERIZER_M_AXIS_T]);
            float w = *(float*)&(top->m_axis_tdata[RASTERIZER_M_AXIS_W]);

            float sr;
            float tr;
            float wr;

            calculateVertexAttributes(sb, sx, sy, tb, tx, ty, wb, wx, wy, sp.val.x - CLOCK_DELAY, sp.val.y + CLOCK_DELAY, sr, tr, wr);
            
            //printf("%d %f %f %f %f %f %f\n", i, s, sr, t, tr, w, wr);
            
            REQUIRE(Approx(s).epsilon(0.01) == sr);
            REQUIRE(Approx(t).epsilon(0.01) == tr);
            REQUIRE(Approx(w).epsilon(0.01) == wr);
        }
        REQUIRE(top->pixelInPipeline == 1);
 
        sp.val.x++;
        sp.val.y--;
        top->s_axis_tdata[RASTERIZER_AXIS_SCREEN_POS_POS] = sp.u32;
        top->s_axis_tvalid = 1;
        top->s_axis_tlast = i % 5;
        top->s_axis_tdata[RASTERIZER_AXIS_FRAMEBUFFER_INDEX] = i;
    }

    // Reset counter for the rest of the pixels in the pipeline
    sp.val.x = 100 - CLOCK_DELAY;
    sp.val.y = CLOCK_DELAY;

    // Check the pending pixels in the pipeline
    for (int i = 100 - CLOCK_DELAY; i < 100; i++)
    {
        clk(top);

        REQUIRE(top->m_axis_tdata[RASTERIZER_M_AXIS_FRAMEBUFFER_INDEX] == i);
        REQUIRE(top->m_axis_tdata[RASTERIZER_M_AXIS_TRIANGLE_COLOR] == 123);
        REQUIRE(top->m_axis_tvalid == 1);
        REQUIRE(top->m_axis_tlast == i % 5);
        REQUIRE(top->pixelInPipeline == 1);

        float s = *(float*)&(top->m_axis_tdata[RASTERIZER_M_AXIS_S]);
        float t = *(float*)&(top->m_axis_tdata[RASTERIZER_M_AXIS_T]);
        float w = *(float*)&(top->m_axis_tdata[RASTERIZER_M_AXIS_W]);

        float sr;
        float tr;
        float wr;

        calculateVertexAttributes(sb, sx, sy, tb, tx, ty, wb, wx, wy, sp.val.x, sp.val.y, sr, tr, wr);

        REQUIRE(Approx(s).epsilon(0.01) == sr);
        REQUIRE(Approx(t).epsilon(0.01) == tr);
        REQUIRE(Approx(w).epsilon(0.01) == wr);

        top->s_axis_tdata[RASTERIZER_AXIS_SCREEN_POS_POS] = 0;
        top->s_axis_tvalid = 0;
        top->s_axis_tlast = 0;
        top->s_axis_tdata[RASTERIZER_AXIS_FRAMEBUFFER_INDEX] = 0;
        top->s_axis_tdata[RASTERIZER_AXIS_TRIANGLE_COLOR_POS] = 0;

        sp.val.x++;
        sp.val.y--;

        // Set init values
        top->s_axis_tdata[RASTERIZER_AXIS_INC_TEXTURE_S_POS] = 0;
        top->s_axis_tdata[RASTERIZER_AXIS_INC_TEXTURE_S_X_POS] = 0;
        top->s_axis_tdata[RASTERIZER_AXIS_INC_TEXTURE_S_Y_POS] = 0;
        top->s_axis_tdata[RASTERIZER_AXIS_INC_TEXTURE_T_POS] = 0;
        top->s_axis_tdata[RASTERIZER_AXIS_INC_TEXTURE_T_X_POS] = 0;
        top->s_axis_tdata[RASTERIZER_AXIS_INC_TEXTURE_T_Y_POS] = 0;
        top->s_axis_tdata[RASTERIZER_AXIS_INC_DEPTH_W_POS] = 0;
        top->s_axis_tdata[RASTERIZER_AXIS_INC_DEPTH_W_X_POS] = 0;
        top->s_axis_tdata[RASTERIZER_AXIS_INC_DEPTH_W_Y_POS] = 0;
        top->s_axis_tvalid = 0;
        top->s_axis_tlast = 0;
    }

    // Check again if the pipeline is now empty
    clk(top);
    REQUIRE(top->s_axis_tdata[RASTERIZER_M_AXIS_S] == 0);
    REQUIRE(top->s_axis_tdata[RASTERIZER_M_AXIS_T] == 0);
    REQUIRE(top->s_axis_tdata[RASTERIZER_M_AXIS_W] == 0);
    REQUIRE(top->s_axis_tdata[RASTERIZER_M_AXIS_TRIANGLE_COLOR] == 0);
    REQUIRE(top->s_axis_tdata[RASTERIZER_M_AXIS_FRAMEBUFFER_INDEX] == 0);
    REQUIRE(top->m_axis_tvalid == 0);
    REQUIRE(top->m_axis_tlast == 0);
    REQUIRE(top->pixelInPipeline == 0);

    // Destroy model
    delete top;
}

// RasteriCEr
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

#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "3rdParty/catch.hpp"

// Include common routines
#include <verilated.h>

// Include model header, generated from Verilating "top.v"
#include "VAttributeInterpolator.h"

static constexpr uint32_t RASTERIZER_AXIS_SCREEN_POS_POS = 0;
static constexpr uint32_t RASTERIZER_AXIS_FRAMEBUFFER_INDEX = 1;

static constexpr uint32_t RASTERIZER_M_AXIS_R = 0;
static constexpr uint32_t RASTERIZER_M_AXIS_G = 1;
static constexpr uint32_t RASTERIZER_M_AXIS_B = 2;
static constexpr uint32_t RASTERIZER_M_AXIS_A = 3;
static constexpr uint32_t RASTERIZER_M_AXIS_Z = 4;
static constexpr uint32_t RASTERIZER_M_AXIS_S = 5;
static constexpr uint32_t RASTERIZER_M_AXIS_T = 6;
static constexpr uint32_t RASTERIZER_M_AXIS_W = 7;
static constexpr uint32_t RASTERIZER_M_AXIS_FRAMEBUFFER_INDEX = 8;

union ScreenPos {
    uint64_t u64;
    struct {
        uint16_t x;
        uint16_t y;
        uint32_t framebufferIndex;
    } val;
};

void clk(VAttributeInterpolator* t)
{
    t->aclk = 0;
    t->eval();
    t->aclk = 1;
    t->eval();
}

struct Attributes 
{
    float sb = 0.0f;
    float sx = 0.0f;
    float sy = 0.0f;
    float tb = 0.0f;
    float tx = 0.0f;
    float ty = 0.0f;
    float wb = 0.0f;
    float wx = 0.0f;
    float wy = 0.0f;
    float zb = 0.0f;
    float zx = 0.0f;
    float zy = 0.0f;
    float rb = 0.0f;
    float rx = 0.0f;
    float ry = 0.0f;
    float gb = 0.0f;
    float gx = 0.0f;
    float gy = 0.0f;
    float bb = 0.0f;
    float bx = 0.0f;
    float by = 0.0f;
    float ab = 0.0f;
    float ax = 0.0f;
    float ay = 0.0f;
};

struct AttributesResult
{
    float s = 0.0f;
    float t = 0.0f;
    float w = 0.0f;
    float z = 0.0f;
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;
    float a = 0.0f;
};

void calculateVertexAttributes(Attributes attr, 
                               uint16_t x, uint16_t y, 
                               AttributesResult& res)
{
    attr.sx *= x;
    attr.sy *= y;
    attr.tx *= x;
    attr.ty *= y;
    attr.wx *= x;
    attr.wy *= y;
    attr.zx *= x;
    attr.zy *= y;
    attr.rx *= x;
    attr.ry *= y;
    attr.gx *= x;
    attr.gy *= y;
    attr.bx *= x;
    attr.by *= y;
    attr.ax *= x;
    attr.ay *= y;

    res.w = attr.wb + (attr.wx + attr.wy);
    res.s = attr.sb + (attr.sx + attr.sy);
    res.t = attr.tb + (attr.tx + attr.ty);
    res.z = attr.zb + (attr.zx + attr.zy);
    res.r = attr.rb + (attr.rx + attr.ry);
    res.g = attr.gb + (attr.gx + attr.gy);
    res.b = attr.bb + (attr.bx + attr.by);
    res.a = attr.ab + (attr.ax + attr.ay);

    res.w = 1.0f / res.w;

    res.s *= res.w;
    res.t *= res.w;
    res.r *= res.w;
    res.g *= res.w;
    res.b *= res.w;
    res.a *= res.w;
}

TEST_CASE("Check the interpolation through the pipeline", "[AttributeInterpolator]")
{
    static const uint32_t CLOCK_DELAY = 32;
    VAttributeInterpolator* top = new VAttributeInterpolator();

    // The output port is always ready. We don't test interrupted transfers right now.
    top->m_axis_tready = 1;

    // Reset cycle
    top->resetn = 0;
    clk(top);
    top->resetn = 1;
    clk(top);

    // Reset the pipeline
    for (int i = 0; i < CLOCK_DELAY; i++)
    {
        top->tex_s = 0;
        top->tex_s_inc_x = 0;
        top->tex_s_inc_y = 0;
        top->tex_t = 0;
        top->tex_t_inc_x = 0;
        top->tex_t_inc_y = 0;
        top->depth_w = 0;
        top->depth_w_inc_x = 0;
        top->depth_w_inc_y = 0;
        top->depth_z = 0;
        top->depth_z_inc_x = 0;
        top->depth_z_inc_y = 0;
        top->color_r = 0;
        top->color_r_inc_x = 0;
        top->color_r_inc_y = 0;
        top->color_g = 0;
        top->color_g_inc_x = 0;
        top->color_g_inc_y = 0;
        top->color_b = 0;
        top->color_b_inc_x = 0;
        top->color_b_inc_y = 0;
        top->color_a = 0;
        top->color_a_inc_x = 0;
        top->color_a_inc_y = 0;

        top->s_axis_tdata = 0;

        top->s_axis_tvalid = 0;
        top->s_axis_tlast = 0;
    }
    REQUIRE(top->m_axis_tdata[RASTERIZER_M_AXIS_S] == 0);
    REQUIRE(top->m_axis_tdata[RASTERIZER_M_AXIS_T] == 0);
    REQUIRE(top->m_axis_tdata[RASTERIZER_M_AXIS_W] == 0);
    REQUIRE(top->m_axis_tdata[RASTERIZER_M_AXIS_Z] == 0);
    REQUIRE(top->m_axis_tdata[RASTERIZER_M_AXIS_R] == 0);
    REQUIRE(top->m_axis_tdata[RASTERIZER_M_AXIS_G] == 0);
    REQUIRE(top->m_axis_tdata[RASTERIZER_M_AXIS_B] == 0);
    REQUIRE(top->m_axis_tdata[RASTERIZER_M_AXIS_A] == 0);
    REQUIRE(top->m_axis_tdata[RASTERIZER_M_AXIS_FRAMEBUFFER_INDEX] == 0);
    REQUIRE(top->m_axis_tvalid == 0);
    REQUIRE(top->m_axis_tlast == 0);
    REQUIRE(top->pixelInPipeline == 0);

    ScreenPos sp;
    sp.val.x = 0;
    sp.val.y = 100;
    sp.val.framebufferIndex = 0;

    Attributes attr;

    attr.sb = 3.0;
    attr.sx = 0.50;
    attr.sy = 0.25;
    attr.tb = 4.0;
    attr.tx = 0.20;
    attr.ty = 0.10;
    attr.wb = 0.06754551082849503;
    attr.wx = -1.5070709196152166e-05;
    attr.wy = 3.105480573140085e-05;
    attr.zb = 1.1;
    attr.zx = 0.1;
    attr.zy = 0.2;
    attr.rb = 1.2;
    attr.rx = 0.3;
    attr.ry = 0.4;
    attr.gb = 1.3;
    attr.gx = 0.5;
    attr.gy = 0.6;
    attr.bb = 1.4;
    attr.bx = 0.7;
    attr.by = 0.8;
    attr.ab = 1.5;
    attr.ax = 0.1;
    attr.ay = 0.2;

    // Set the vertex attributes
    top->tex_s = *(uint32_t*)&attr.sb;
    top->tex_s_inc_x = *(uint32_t*)&attr.sx;
    top->tex_s_inc_y = *(uint32_t*)&attr.sy;
    top->tex_t = *(uint32_t*)&attr.tb;
    top->tex_t_inc_x = *(uint32_t*)&attr.tx;
    top->tex_t_inc_y = *(uint32_t*)&attr.ty;
    top->depth_w = *(uint32_t*)&attr.wb;
    top->depth_w_inc_x = *(uint32_t*)&attr.wx;
    top->depth_w_inc_y = *(uint32_t*)&attr.wy;
    top->depth_z = *(uint32_t*)&attr.zb;
    top->depth_z_inc_x = *(uint32_t*)&attr.zx;
    top->depth_z_inc_y = *(uint32_t*)&attr.zy;
    top->color_r = *(uint32_t*)&attr.rb;
    top->color_r_inc_x = *(uint32_t*)&attr.rx;
    top->color_r_inc_y = *(uint32_t*)&attr.ry;
    top->color_g = *(uint32_t*)&attr.gb;
    top->color_g_inc_x = *(uint32_t*)&attr.gx;
    top->color_g_inc_y = *(uint32_t*)&attr.gy;
    top->color_b = *(uint32_t*)&attr.bb;
    top->color_b_inc_x = *(uint32_t*)&attr.bx;
    top->color_b_inc_y = *(uint32_t*)&attr.by;
    top->color_a = *(uint32_t*)&attr.ab;
    top->color_a_inc_x = *(uint32_t*)&attr.ax;
    top->color_a_inc_y = *(uint32_t*)&attr.ay;

    top->s_axis_tdata = sp.u64;
    top->s_axis_tvalid = 1;
    top->s_axis_tlast = 0;

    // Check the pipeline with 100 pixel
    for (int i = 0; i < 100; i++)
    {
        clk(top);
        if (i > CLOCK_DELAY)
        {
            REQUIRE(top->m_axis_tdata[RASTERIZER_M_AXIS_FRAMEBUFFER_INDEX] == i - CLOCK_DELAY);
            REQUIRE(top->m_axis_tvalid == 1);
            REQUIRE(top->m_axis_tlast == (i - CLOCK_DELAY) % 5);

            float s = *(float*)&(top->m_axis_tdata[RASTERIZER_M_AXIS_S]);
            float t = *(float*)&(top->m_axis_tdata[RASTERIZER_M_AXIS_T]);
            float w = *(float*)&(top->m_axis_tdata[RASTERIZER_M_AXIS_W]);
            float z = *(float*)&(top->m_axis_tdata[RASTERIZER_M_AXIS_Z]);
            float r = *(float*)&(top->m_axis_tdata[RASTERIZER_M_AXIS_R]);
            float g = *(float*)&(top->m_axis_tdata[RASTERIZER_M_AXIS_G]);
            float b = *(float*)&(top->m_axis_tdata[RASTERIZER_M_AXIS_B]);
            float a = *(float*)&(top->m_axis_tdata[RASTERIZER_M_AXIS_A]);

            AttributesResult expectedResult;

            calculateVertexAttributes(attr,
                                      sp.val.x - CLOCK_DELAY, sp.val.y + CLOCK_DELAY, 
                                      expectedResult);
            
            //printf("%d %f %f %f %f %f %f\n", i, s, sr, t, tr, w, wr);
            
            REQUIRE(Approx(s).epsilon(0.01) == expectedResult.s);
            REQUIRE(Approx(t).epsilon(0.01) == expectedResult.t);
            REQUIRE(Approx(w).epsilon(0.01) == expectedResult.w);
            REQUIRE(Approx(z).epsilon(0.01) == expectedResult.z);
            REQUIRE(Approx(r).epsilon(0.01) == expectedResult.r);
            REQUIRE(Approx(g).epsilon(0.01) == expectedResult.g);
            REQUIRE(Approx(b).epsilon(0.01) == expectedResult.b);
            REQUIRE(Approx(a).epsilon(0.01) == expectedResult.a);
        }
        REQUIRE(top->pixelInPipeline == 1);
 
        sp.val.x++;
        sp.val.y--;
        
        top->s_axis_tvalid = 1;
        top->s_axis_tlast = i % 5;
        sp.val.framebufferIndex = i;
        top->s_axis_tdata = sp.u64;
    }

    // Reset counter for the rest of the pixels in the pipeline
    sp.val.x = 100 - CLOCK_DELAY;
    sp.val.y = CLOCK_DELAY;

    // Check the pending pixels in the pipeline
    for (int i = 100 - CLOCK_DELAY; i < 100; i++)
    {
        clk(top);

        REQUIRE(top->m_axis_tdata[RASTERIZER_M_AXIS_FRAMEBUFFER_INDEX] == i);
        REQUIRE(top->m_axis_tvalid == 1);
        REQUIRE(top->m_axis_tlast == i % 5);
        REQUIRE(top->pixelInPipeline == 1);

        float s = *(float*)&(top->m_axis_tdata[RASTERIZER_M_AXIS_S]);
        float t = *(float*)&(top->m_axis_tdata[RASTERIZER_M_AXIS_T]);
        float w = *(float*)&(top->m_axis_tdata[RASTERIZER_M_AXIS_W]);
        float z = *(float*)&(top->m_axis_tdata[RASTERIZER_M_AXIS_Z]);
        float r = *(float*)&(top->m_axis_tdata[RASTERIZER_M_AXIS_R]);
        float g = *(float*)&(top->m_axis_tdata[RASTERIZER_M_AXIS_G]);
        float b = *(float*)&(top->m_axis_tdata[RASTERIZER_M_AXIS_B]);
        float a = *(float*)&(top->m_axis_tdata[RASTERIZER_M_AXIS_A]);

        AttributesResult expectedResult;

        calculateVertexAttributes(attr,
                                  sp.val.x, sp.val.y, 
                                  expectedResult);
        
        REQUIRE(Approx(s).epsilon(0.01) == expectedResult.s);
        REQUIRE(Approx(t).epsilon(0.01) == expectedResult.t);
        REQUIRE(Approx(w).epsilon(0.01) == expectedResult.w);
        REQUIRE(Approx(z).epsilon(0.01) == expectedResult.z);
        REQUIRE(Approx(r).epsilon(0.01) == expectedResult.r);
        REQUIRE(Approx(g).epsilon(0.01) == expectedResult.g);
        REQUIRE(Approx(b).epsilon(0.01) == expectedResult.b);
        REQUIRE(Approx(a).epsilon(0.01) == expectedResult.a);

        top->s_axis_tdata = 0;
        top->s_axis_tvalid = 0;
        top->s_axis_tlast = 0;

        sp.val.x++;
        sp.val.y--;

        // Set init values
        top->s_axis_tvalid = 0;
        top->s_axis_tlast = 0;
        top->s_axis_tdata = 0;
    }

    // Check again if the pipeline is now empty
    clk(top);

    REQUIRE(top->m_axis_tdata[RASTERIZER_M_AXIS_FRAMEBUFFER_INDEX] == 0);
    REQUIRE(top->m_axis_tvalid == 0);
    REQUIRE(top->m_axis_tlast == 0);

    // This signal is one clock delayed
    clk(top);
    REQUIRE(top->pixelInPipeline == 0);

    // Destroy model
    delete top;
}


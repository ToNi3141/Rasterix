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

#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "3rdParty/catch.hpp"
#include <math.h>
#include <array>
#include <algorithm>

// Include common routines
#include <verilated.h>

// Include model header, generated from Verilating "top.v"
#include "VTextureBuffer.h"

void clk(VTextureBuffer* t)
{
    t->aclk = 0;
    t->eval();
    t->aclk = 1;
    t->eval();
}

void reset(VTextureBuffer* t)
{
    t->resetn = 0;
    clk(t);
    t->resetn = 1;
    clk(t);
}

TEST_CASE("Get various values from the texture buffer", "[TextureBuffer]")
{
    VTextureBuffer* top = new VTextureBuffer();
    reset(top);


    // 2x2 texture
    // | 0xf000 | 0x0f00 |
    // | 0x00f0 | 0x000f |
    top->textureSizeX = 0x1;
    top->textureSizeY = 0x1;

    top->s_axis_tvalid = 1;
    top->s_axis_tlast = 0;
    top->s_axis_tdata = 0x0f00'f000;
    clk(top);

    top->s_axis_tvalid = 1;
    top->s_axis_tlast = 1;
    top->s_axis_tdata = 0x000f'00f0;
    clk(top);

    top->s_axis_tvalid = 0;
    top->s_axis_tlast = 0;

    // (0, 0)
    top->texelX = 0;
    top->texelY = 0;
    clk(top);
    REQUIRE(top->texel00 == 0xf000);
    REQUIRE(top->texel01 == 0x0f00);
    REQUIRE(top->texel10 == 0x00f0);
    REQUIRE(top->texel11 == 0x000f);

    // (0.99.., 0.0)
    top->texelX = 0x7fff;
    top->texelY = 0;
    clk(top);
    REQUIRE(top->texel00 == 0x0f00);
    REQUIRE(top->texel01 == 0xf000);
    REQUIRE(top->texel10 == 0x000f);
    REQUIRE(top->texel11 == 0x00f0);

    // (0.0, 0.99..)
    top->texelX = 0;
    top->texelY = 0x7fff;
    clk(top);
    REQUIRE(top->texel00 == 0x00f0);
    REQUIRE(top->texel01 == 0x000f);
    REQUIRE(top->texel10 == 0xf000);
    REQUIRE(top->texel11 == 0x0f00);

    // (0.99.., 0.99..)
    top->texelX = 0x7fff;
    top->texelY = 0x7fff;
    clk(top);
    REQUIRE(top->texel00 == 0x000f);
    REQUIRE(top->texel01 == 0x00f0);
    REQUIRE(top->texel10 == 0x0f00);
    REQUIRE(top->texel11 == 0xf000);


    // (1.0, 0.0)
    top->texelX = 0x8000;
    top->texelY = 0;
    clk(top);
    REQUIRE(top->texel00 == 0xf000);
    REQUIRE(top->texel01 == 0x0f00);
    REQUIRE(top->texel10 == 0x00f0);
    REQUIRE(top->texel11 == 0x000f);

    // (0.0, 1.0)
    top->texelX = 0;
    top->texelY = 0x8000;
    clk(top);
    REQUIRE(top->texel00 == 0xf000);
    REQUIRE(top->texel01 == 0x0f00);
    REQUIRE(top->texel10 == 0x00f0);
    REQUIRE(top->texel11 == 0x000f);

    // (1.0, 1.0)
    top->texelX = 0x8000;
    top->texelY = 0x8000;
    clk(top);
    REQUIRE(top->texel00 == 0xf000);
    REQUIRE(top->texel01 == 0x0f00);
    REQUIRE(top->texel10 == 0x00f0);
    REQUIRE(top->texel11 == 0x000f);


    // Destroy model
    delete top;
}


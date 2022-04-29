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

void uploadTexture(VTextureBuffer* top) 
{
    // 2x2 texture
    // | 0xf000 | 0x0f00 |
    // | 0x00f0 | 0x000f |
    top->textureSizeS = 0x1;
    top->textureSizeT = 0x1;

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

    top->clampS = 0;
    top->clampT = 0;
}

TEST_CASE("Get various values from the texture buffer", "[TextureBuffer]")
{
    VTextureBuffer* top = new VTextureBuffer();
    reset(top);

    uploadTexture(top);

    // (0, 0)
    top->texelS = 0;
    top->texelT = 0;
    clk(top);
    REQUIRE(top->texel00 == 0xf000);
    REQUIRE(top->texel01 == 0x0f00);
    REQUIRE(top->texel10 == 0x00f0);
    REQUIRE(top->texel11 == 0x000f);

    // (0.99.., 0.0)
    top->texelS = 0x7fff;
    top->texelT = 0;
    clk(top);
    REQUIRE(top->texel00 == 0x0f00);
    REQUIRE(top->texel01 == 0xf000);
    REQUIRE(top->texel10 == 0x000f);
    REQUIRE(top->texel11 == 0x00f0);

    // (0.0, 0.99..)
    top->texelS = 0;
    top->texelT = 0x7fff;
    clk(top);
    REQUIRE(top->texel00 == 0x00f0);
    REQUIRE(top->texel01 == 0x000f);
    REQUIRE(top->texel10 == 0xf000);
    REQUIRE(top->texel11 == 0x0f00);

    // (0.99.., 0.99..)
    top->texelS = 0x7fff;
    top->texelT = 0x7fff;
    clk(top);
    REQUIRE(top->texel00 == 0x000f);
    REQUIRE(top->texel01 == 0x00f0);
    REQUIRE(top->texel10 == 0x0f00);
    REQUIRE(top->texel11 == 0xf000);


    // (1.0, 0.0)
    top->texelS = 0x8000;
    top->texelT = 0;
    clk(top);
    REQUIRE(top->texel00 == 0xf000);
    REQUIRE(top->texel01 == 0x0f00);
    REQUIRE(top->texel10 == 0x00f0);
    REQUIRE(top->texel11 == 0x000f);

    // (0.0, 1.0)
    top->texelS = 0;
    top->texelT = 0x8000;
    clk(top);
    REQUIRE(top->texel00 == 0xf000);
    REQUIRE(top->texel01 == 0x0f00);
    REQUIRE(top->texel10 == 0x00f0);
    REQUIRE(top->texel11 == 0x000f);

    // (1.0, 1.0)
    top->texelS = 0x8000;
    top->texelT = 0x8000;
    clk(top);
    REQUIRE(top->texel00 == 0xf000);
    REQUIRE(top->texel01 == 0x0f00);
    REQUIRE(top->texel10 == 0x00f0);
    REQUIRE(top->texel11 == 0x000f);

    // Destroy model
    delete top;
}

TEST_CASE("Check intensity attribute", "[TextureBuffer]")
{
    VTextureBuffer* top = new VTextureBuffer();
    reset(top);

    uploadTexture(top);

    // texel (0.0, 0.0)
    top->texelS = 0x0000;
    top->texelT = 0x0000;
    clk(top);
    REQUIRE(top->texel00 == 0xf000);
    REQUIRE(top->texel01 == 0x0f00);
    REQUIRE(top->texel10 == 0x00f0);
    REQUIRE(top->texel11 == 0x000f);
    // sub texel (0.0, 0.0)
    REQUIRE(top->texelSubCoordS == 0x0); 
    REQUIRE(top->texelSubCoordT == 0x0);


    // To get the sub coordinate of (0.25, 0.0), we have to imagine the following things:
    // In a 2x2 texture, the coordinate (0.0, 0.0) accesses texel (0, 0). The coordinate (0.5, 0.0) accesses (1, 0).
    // Means, the distance between two texels is 0.5. If we want now the sub texel coordinate (0.25, 0.0) we have
    // now to multiply the vector with the texel distance of 0.5 which results in (0.125, 0.0). If we want to convert this
    // to access the texel (1, 0), we would have to add 0.5 to the x value which results in (0.625, 0.0).

    // texel (0.125, 0.125) 
    top->texelS = 0x1000;
    top->texelT = 0x1000;
    clk(top);
    REQUIRE(top->texel00 == 0xf000);
    REQUIRE(top->texel01 == 0x0f00);
    REQUIRE(top->texel10 == 0x00f0);
    REQUIRE(top->texel11 == 0x000f);
    // sub texel (0.25, 0.25)
    REQUIRE(top->texelSubCoordS == 0x4000); 
    REQUIRE(top->texelSubCoordT == 0x4000); 

    // texel (0.125, 0.375) 
    top->texelS = 0x1000;
    top->texelT = 0x3000;
    clk(top);
    REQUIRE(top->texel00 == 0xf000);
    REQUIRE(top->texel01 == 0x0f00);
    REQUIRE(top->texel10 == 0x00f0);
    REQUIRE(top->texel11 == 0x000f);
    // sub texel (0.25, 0.75)
    REQUIRE(top->texelSubCoordS == 0x4000); 
    REQUIRE(top->texelSubCoordT == 0xc000);

    // texel (0.375, 0.125) 
    top->texelS = 0x3000;
    top->texelT = 0x1000;
    clk(top);
    REQUIRE(top->texel00 == 0xf000);
    REQUIRE(top->texel01 == 0x0f00);
    REQUIRE(top->texel10 == 0x00f0);
    REQUIRE(top->texel11 == 0x000f);
    // sub texel (0.75, 0.25)
    REQUIRE(top->texelSubCoordS == 0xc000);
    REQUIRE(top->texelSubCoordT == 0x4000);

    // Destroy model
    delete top;
}

TEST_CASE("clamp to border with x", "[TextureBuffer]")
{
    VTextureBuffer* top = new VTextureBuffer();
    reset(top);

    uploadTexture(top);

    top->clampS = 1;
    top->clampT = 0;

    // texel (0.0, 0.0)
    top->texelS = 0x0000;
    top->texelT = 0x0000;
    clk(top);
    REQUIRE(top->texel00 == 0xf000);
    REQUIRE(top->texel01 == 0x0f00);
    REQUIRE(top->texel10 == 0x00f0);
    REQUIRE(top->texel11 == 0x000f);

    // texel (0.5, 0.0)
    top->texelS = 0x4000;
    top->texelT = 0x0000;
    clk(top);
    REQUIRE(top->texel00 == 0x0f00);
    REQUIRE(top->texel01 == 0x0f00);
    REQUIRE(top->texel10 == 0x000f);
    REQUIRE(top->texel11 == 0x000f);

    // texel (0.5, 0.5)
    top->texelS = 0x4000;
    top->texelT = 0x4000;
    clk(top);
    REQUIRE(top->texel00 == 0x000f);
    REQUIRE(top->texel01 == 0x000f);
    REQUIRE(top->texel10 == 0x0f00);
    REQUIRE(top->texel11 == 0x0f00);

    // Destroy model
    delete top;
}

TEST_CASE("clamp to border with y", "[TextureBuffer]")
{
    VTextureBuffer* top = new VTextureBuffer();
    reset(top);

    uploadTexture(top);

    top->clampS = 0;
    top->clampT = 1;

    // texel (0.0, 0.0)
    top->texelS = 0x0000;
    top->texelT = 0x0000;
    clk(top);
    REQUIRE(top->texel00 == 0xf000);
    REQUIRE(top->texel01 == 0x0f00);
    REQUIRE(top->texel10 == 0x00f0);
    REQUIRE(top->texel11 == 0x000f);

    // texel (0.0, 0.5)
    top->texelS = 0x0000;
    top->texelT = 0x4000;
    clk(top);
    REQUIRE(top->texel00 == 0x00f0);
    REQUIRE(top->texel01 == 0x000f);
    REQUIRE(top->texel10 == 0x00f0);
    REQUIRE(top->texel11 == 0x000f);

    // texel (0.5, 0.5)
    top->texelS = 0x4000;
    top->texelT = 0x4000;
    clk(top);
    REQUIRE(top->texel00 == 0x000f);
    REQUIRE(top->texel01 == 0x00f0);
    REQUIRE(top->texel10 == 0x000f);
    REQUIRE(top->texel11 == 0x00f0);

    // Destroy model
    delete top;
}

TEST_CASE("clamp to border with x and y", "[TextureBuffer]")
{
    VTextureBuffer* top = new VTextureBuffer();
    reset(top);

    uploadTexture(top);

    top->clampS = 1;
    top->clampT = 1;

    // texel (0.0, 0.0)
    top->texelS = 0x0000;
    top->texelT = 0x0000;
    clk(top);
    REQUIRE(top->texel00 == 0xf000);
    REQUIRE(top->texel01 == 0x0f00);
    REQUIRE(top->texel10 == 0x00f0);
    REQUIRE(top->texel11 == 0x000f);

    // texel (0.5, 0.0)
    top->texelS = 0x4000;
    top->texelT = 0x0000;
    clk(top);
    REQUIRE(top->texel00 == 0x0f00);
    REQUIRE(top->texel01 == 0x0f00);
    REQUIRE(top->texel10 == 0x000f);
    REQUIRE(top->texel11 == 0x000f);

    // texel (0.0, 0.5)
    top->texelS = 0x0000;
    top->texelT = 0x4000;
    clk(top);
    REQUIRE(top->texel00 == 0x00f0);
    REQUIRE(top->texel01 == 0x000f);
    REQUIRE(top->texel10 == 0x00f0);
    REQUIRE(top->texel11 == 0x000f);

    // texel (0.5, 0.5)
    top->texelS = 0x4000;
    top->texelT = 0x4000;
    clk(top);
    REQUIRE(top->texel00 == 0x000f);
    REQUIRE(top->texel01 == 0x000f);
    REQUIRE(top->texel10 == 0x000f);
    REQUIRE(top->texel11 == 0x000f);

    // Destroy model
    delete top;
}
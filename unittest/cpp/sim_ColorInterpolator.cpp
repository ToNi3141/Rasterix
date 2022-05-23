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
#include "VColorInterpolator.h"

void clk(VColorInterpolator* t)
{
    t->aclk = 0;
    t->eval();
    t->aclk = 1;
    t->eval();
}

void reset(VColorInterpolator* t)
{
    t->resetn = 0;
    clk(t);
    t->resetn = 1;
    clk(t);
}

TEST_CASE("Check interpolation", "[ColorInterpolator]")
{
    VColorInterpolator* top = new VColorInterpolator();
    reset(top);

    const uint32_t colorA = 0xffffffff;
    const uint32_t colorB = 0x00000000;

    for (uint32_t i = 0; i < 256; i++)
    {
        top->intensity = (255 - i) << 8;
        top->colorA = colorA;
        top->colorB = colorB;
        clk(top);
        clk(top);

        const uint32_t mixedColor = colorA - (0x01010101 * i);

        REQUIRE(top->mixedColor == mixedColor);
    }

    // Destroy model
    delete top;
}

TEST_CASE("Check pipelining", "[ColorInterpolator]")
{
    VColorInterpolator* top = new VColorInterpolator();
    reset(top);

    top->colorA = 0xffffffff;
    top->colorB = 0x00000000;

    top->intensity = 0x0;
    clk(top);

    top->intensity = 0x7fff;
    clk(top);
    REQUIRE(top->mixedColor == 0x0);

    top->intensity = 0xffff;
    clk(top);
    REQUIRE(top->mixedColor == 0x7f7f7f7f);

    clk(top);
    REQUIRE(top->mixedColor == 0xffffffff);
    
    // Destroy model
    delete top;
}

TEST_CASE("Check channels", "[ColorInterpolator]")
{
    VColorInterpolator* top = new VColorInterpolator();
    reset(top);

    top->intensity = 0x7fff;

    top->colorA = 0xff000000;
    top->colorB = 0x7f000000;
    clk(top);
    clk(top);
    REQUIRE(top->mixedColor == 0xbf000000);

    top->colorA = 0x00ff0000;
    top->colorB = 0x007f0000;
    clk(top);
    clk(top);
    REQUIRE(top->mixedColor == 0x00bf0000);

    top->colorA = 0x0000ff00;
    top->colorB = 0x00007f00;
    clk(top);
    clk(top);
    REQUIRE(top->mixedColor == 0x0000bf00);

    top->colorA = 0x000000ff;
    top->colorB = 0x0000007f;
    clk(top);
    clk(top);
    REQUIRE(top->mixedColor == 0x000000bf);
    
    // Destroy model
    delete top;
}

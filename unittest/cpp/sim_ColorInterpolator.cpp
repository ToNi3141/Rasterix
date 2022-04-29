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

    const uint16_t colorA = 0xffff;
    const uint16_t colorB = 0x0000;

    for (uint32_t i = 0; i < 16; i++)
    {
        top->intensity = (15 - i) << 12;
        top->colorA = colorA;
        top->colorB = colorB;
        clk(top);
        clk(top);

        const uint16_t mixedColor = colorA - (0x1111 * i);

        REQUIRE(top->mixedColor == mixedColor);
    }

    // Destroy model
    delete top;
}

TEST_CASE("Check pipelining", "[ColorInterpolator]")
{
    VColorInterpolator* top = new VColorInterpolator();
    reset(top);

    top->colorA = 0xffff;
    top->colorB = 0x0000;

    top->intensity = 0x0;
    clk(top);

    top->intensity = 0x7fff;
    clk(top);
    REQUIRE(top->mixedColor == 0x0);

    top->intensity = 0xffff;
    clk(top);
    REQUIRE(top->mixedColor == 0x7777);

    clk(top);
    REQUIRE(top->mixedColor == 0xffff);
    
    // Destroy model
    delete top;
}

TEST_CASE("Check channels", "[ColorInterpolator]")
{
    VColorInterpolator* top = new VColorInterpolator();
    reset(top);

    top->intensity = 0x7fff;

    top->colorA = 0xf000;
    top->colorB = 0x7000;
    clk(top);
    clk(top);
    REQUIRE(top->mixedColor == 0xb000);

    top->colorA = 0x0f00;
    top->colorB = 0x0700;
    clk(top);
    clk(top);
    REQUIRE(top->mixedColor == 0x0b00);

    top->colorA = 0x00f0;
    top->colorB = 0x0070;
    clk(top);
    clk(top);
    REQUIRE(top->mixedColor == 0x00b0);

    top->colorA = 0x000f;
    top->colorB = 0x0007;
    clk(top);
    clk(top);
    REQUIRE(top->mixedColor == 0x000b);
    
    // Destroy model
    delete top;
}

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
#include "VTexEnv.h"

enum TexEnvParam
{
    DISABLE,
    REPLACE,
    MODULATE,
    DECAL,
    BLEND,
    ADD
};

void clk(VTexEnv* t)
{
    t->aclk = 0;
    t->eval();
    t->aclk = 1;
    t->eval();
}

void reset(VTexEnv* t)
{
    t->resetn = 0;
    clk(t);
    t->resetn = 1;
    clk(t);
}

TEST_CASE("Check interpolation", "[ColorInterpolator]")
{
    VTexEnv* top = new VTexEnv();
    reset(top);

    top->func = REPLACE;
    top->previousColor = 0xff000000;
    top->texSrcColor = 0x00ff0000;
    top->primaryColor = 0x0000ff00;
    top->envColor = 0x000000ff;

    clk(top);
    clk(top);
    REQUIRE(top->color == 0x00ff0000);

    // Destroy model
    delete top;
}
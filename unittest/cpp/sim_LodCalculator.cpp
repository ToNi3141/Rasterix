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

// #define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
// #include "../Unittests/3rdParty/catch.hpp"

#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "3rdParty/catch.hpp"

// Include common routines
#include <verilated.h>

// Include model header, generated from Verilating "top.v"
#include "VLodCalculator.h"

void clk(VLodCalculator* t)
{
    t->aclk = 0;
    t->eval();
    t->aclk = 1;
    t->eval();
}

void reset(VLodCalculator* t)
{
    t->resetn = 0;
    clk(t);
    t->resetn = 1;
    clk(t);
}

TEST_CASE("Check max lod", "[LodCalculator]")
{
    VLodCalculator* t = new VLodCalculator();

    t->textureSizeWidth = 8;
    t->textureSizeHeight = 8;

    t->texelS = 0x1 << (15 - 8);
    t->texelT = 0x1 << (15 - 8);

    t->texelSxy = 0x1 << (15 + 16);
    t->texelTxy = 0x1 << (15 + 16);

    clk(t);

    REQUIRE(t->lod == 8);
}

TEST_CASE("Check lod xy", "[LodCalculator]")
{
    VLodCalculator* t = new VLodCalculator();

    t->textureSizeWidth = 8;
    t->textureSizeHeight = 8;

    t->texelS = 0x1 << (15 - 8);
    t->texelT = 0x1 << (15 - 8);

    for (uint32_t i = 0; i < 8; i++)
    {
        t->texelSxy = 0x1 << (15 - i);
        t->texelTxy = 0x1 << (15 - i);

        clk(t);

        REQUIRE(t->lod == 8 - i);
    }
}

TEST_CASE("Check lod x", "[LodCalculator]")
{
    VLodCalculator* t = new VLodCalculator();

    t->textureSizeWidth = 8;
    t->textureSizeHeight = 8;

    t->texelS = 0x1 << (15 - 8);
    t->texelT = 0x1 << (15 - 8);

    for (uint32_t i = 0; i < 8; i++)
    {
        t->texelSxy = 0x1 << (15 - i);
        t->texelTxy = 0x1 << (15 - 8);

        clk(t);

        REQUIRE(t->lod == 8 - i);
    }
}

TEST_CASE("Check lod y", "[LodCalculator]")
{
    VLodCalculator* t = new VLodCalculator();

    t->textureSizeWidth = 8;
    t->textureSizeHeight = 8;

    t->texelS = 0x1 << (15 - 8);
    t->texelT = 0x1 << (15 - 8);

    for (uint32_t i = 0; i < 8; i++)
    {
        t->texelSxy = 0x1 << (15 - 8);
        t->texelTxy = 0x1 << (15 - i);

        clk(t);

        REQUIRE(t->lod == 8 - i);
    }
}

TEST_CASE("Check lod xy negative", "[LodCalculator]")
{
    VLodCalculator* t = new VLodCalculator();

    t->textureSizeWidth = 8;
    t->textureSizeHeight = 8;

    t->texelS = 0x80000000 | (0x1 << (15 - 8));
    t->texelT = 0x80000000 | (0x1 << (15 - 8));

    for (uint32_t i = 0; i < 8; i++)
    {
        t->texelSxy = 0x80000000 | (0x1 << (15 - i));
        t->texelTxy = 0x80000000 | (0x1 << (15 - i));

        clk(t);

        REQUIRE(t->lod == 8 - i);
    }
}

TEST_CASE("Check lod xy max with varying texture size", "[LodCalculator]")
{
    VLodCalculator* t = new VLodCalculator();

    for (uint32_t i = 0; i < 8; i++)
    {
        t->textureSizeWidth = i;
        t->textureSizeHeight = i;

        t->texelS = 0x1 << (15 - i);
        t->texelT = 0x1 << (15 - i);

        t->texelSxy = 0x1 << (15 + 0);
        t->texelTxy = 0x1 << (15 + 0);

        clk(t);

        REQUIRE(t->lod == i);
    }
}

TEST_CASE("Check lod xy 0 with varying texture size", "[LodCalculator]")
{
    VLodCalculator* t = new VLodCalculator();

    for (uint32_t i = 0; i < 8; i++)
    {
        t->textureSizeWidth = i;
        t->textureSizeHeight = i;

        t->texelS = 0x1 << (15 - i);
        t->texelT = 0x1 << (15 - i);

        t->texelSxy = 0x1 << (15 - i);
        t->texelTxy = 0x1 << (15 - i);

        clk(t);

        REQUIRE(t->lod == 0);
    }
}
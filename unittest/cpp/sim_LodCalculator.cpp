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

#include "general.hpp"

// Include model header, generated from Verilating "top.v"
#include "VLodCalculator.h"


TEST_CASE("Check max lod", "[LodCalculator]")
{
    VLodCalculator* t = new VLodCalculator();
    t->ce = 1;

    t->confEnable = true;

    t->textureSizeWidth = 8;
    t->textureSizeHeight = 8;

    t->texelS = 0x1 << (15 - 8);
    t->texelT = 0x1 << (15 - 8);

    t->texelSxy = 0x1 << (15 + 16);
    t->texelTxy = 0x1 << (15 + 16);

    rr::ut::clk(t);

    REQUIRE(t->lod == 8);
}

TEST_CASE("Check lod xy", "[LodCalculator]")
{
    VLodCalculator* t = new VLodCalculator();
    t->ce = 1;

    t->confEnable = true;

    t->textureSizeWidth = 8;
    t->textureSizeHeight = 8;

    t->texelS = 0x1 << (15 - 8);
    t->texelT = 0x1 << (15 - 8);

    for (uint32_t i = 0; i < 8; i++)
    {
        t->texelSxy = 0x1 << (15 - i);
        t->texelTxy = 0x1 << (15 - i);

        rr::ut::clk(t);

        REQUIRE(t->lod == 8 - i);
    }
}

TEST_CASE("Check lod x", "[LodCalculator]")
{
    VLodCalculator* t = new VLodCalculator();
    t->ce = 1;

    t->confEnable = true;

    t->textureSizeWidth = 8;
    t->textureSizeHeight = 8;

    t->texelS = 0x1 << (15 - 8);
    t->texelT = 0x1 << (15 - 8);

    for (uint32_t i = 0; i < 8; i++)
    {
        t->texelSxy = 0x1 << (15 - i);
        t->texelTxy = 0x1 << (15 - 8);

        rr::ut::clk(t);

        REQUIRE(t->lod == 8 - i);
    }
}

TEST_CASE("Check lod y", "[LodCalculator]")
{
    VLodCalculator* t = new VLodCalculator();
    t->ce = 1;

    t->confEnable = true;

    t->textureSizeWidth = 8;
    t->textureSizeHeight = 8;

    t->texelS = 0x1 << (15 - 8);
    t->texelT = 0x1 << (15 - 8);

    for (uint32_t i = 0; i < 8; i++)
    {
        t->texelSxy = 0x1 << (15 - 8);
        t->texelTxy = 0x1 << (15 - i);

        rr::ut::clk(t);

        REQUIRE(t->lod == 8 - i);
    }
}

TEST_CASE("Check lod xy negative", "[LodCalculator]")
{
    VLodCalculator* t = new VLodCalculator();
    t->ce = 1;

    t->confEnable = true;

    t->textureSizeWidth = 8;
    t->textureSizeHeight = 8;

    t->texelS = 0x80000000 | (0x1 << (15 - 8));
    t->texelT = 0x80000000 | (0x1 << (15 - 8));

    for (uint32_t i = 0; i < 8; i++)
    {
        t->texelSxy = 0x80000000 | (0x1 << (15 - i));
        t->texelTxy = 0x80000000 | (0x1 << (15 - i));

        rr::ut::clk(t);

        REQUIRE(t->lod == 8 - i);
    }
}

TEST_CASE("Check lod xy max with varying texture size", "[LodCalculator]")
{
    VLodCalculator* t = new VLodCalculator();
    t->ce = 1;

    t->confEnable = true;

    for (uint32_t i = 0; i < 8; i++)
    {
        t->textureSizeWidth = i;
        t->textureSizeHeight = i;

        t->texelS = 0x1 << (15 - i);
        t->texelT = 0x1 << (15 - i);

        t->texelSxy = 0x1 << (15 + 0);
        t->texelTxy = 0x1 << (15 + 0);

        rr::ut::clk(t);

        REQUIRE(t->lod == i);
    }
}

TEST_CASE("Check lod xy 0 with varying texture size", "[LodCalculator]")
{
    VLodCalculator* t = new VLodCalculator();
    t->ce = 1;

    t->confEnable = true;

    for (uint32_t i = 0; i < 8; i++)
    {
        t->textureSizeWidth = i;
        t->textureSizeHeight = i;

        t->texelS = 0x1 << (15 - i);
        t->texelT = 0x1 << (15 - i);

        t->texelSxy = 0x1 << (15 - i);
        t->texelTxy = 0x1 << (15 - i);

        rr::ut::clk(t);

        REQUIRE(t->lod == 0);
    }
}

TEST_CASE("Check lod xy 0 with varying values", "[LodCalculator]")
{
    VLodCalculator* t = new VLodCalculator();
    t->ce = 1;

    t->confEnable = true;

    t->textureSizeWidth = 4;
    t->textureSizeHeight = 4;

    t->texelS = 0x1 << (15 - 4);
    t->texelT = 0x1 << (15 - 4);

    for (uint32_t i = 0; i < 16; i++)
    {
        t->texelSxy = (0x1 << (15 - 4)) + ((0x1 << (15 - 8)) * i);
        t->texelTxy = (0x1 << (15 - 4)) + ((0x1 << (15 - 8)) * i);
        rr::ut::clk(t);
        REQUIRE(t->lod == 0);
    }

    // Check that the overflow causes a LOD increment
    t->texelSxy = (0x1 << (15 - 4)) + ((0x1 << (15 - 8)) * 17);
    t->texelTxy = (0x1 << (15 - 4)) + ((0x1 << (15 - 8)) * 17);
    rr::ut::clk(t);
    REQUIRE(t->lod == 1);
}

TEST_CASE("Check lod xy when calculation is disabled", "[LodCalculator]")
{
    VLodCalculator* t = new VLodCalculator();
    t->ce = 1;

    t->confEnable = false;

    for (uint32_t i = 0; i < 8; i++)
    {
        t->textureSizeWidth = i;
        t->textureSizeHeight = i;

        t->texelS = 0x1 << (15 - i);
        t->texelT = 0x1 << (15 - i);

        t->texelSxy = 0x1 << (15 + 0);
        t->texelTxy = 0x1 << (15 + 0);

        rr::ut::clk(t);

        REQUIRE(t->lod == 0);
    }
}

TEST_CASE("Check stall", "[LodCalculator]")
{
    VLodCalculator* t = new VLodCalculator();
    t->ce = 1;

    t->confEnable = true;

    t->textureSizeWidth = 8;
    t->textureSizeHeight = 8;

    t->texelS = 0x1 << (15 - 8);
    t->texelT = 0x1 << (15 - 8);

    t->texelSxy = 0x1 << (15 + 16);
    t->texelTxy = 0x1 << (15 + 16);

    rr::ut::clk(t);
    REQUIRE(t->lod == 8);

    t->texelS = 0;
    t->texelT = 0;
    t->texelSxy = 0;
    t->texelTxy = 0;
    t->ce = 0;
    rr::ut::clk(t);
    REQUIRE(t->lod == 8); // Stays at 8

    t->ce = 1;
    rr::ut::clk(t);
    REQUIRE(t->lod == 0);
}
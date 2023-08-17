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
#include "VStreamConcatFifo.h"

void clk(VStreamConcatFifo* t)
{
    t->aclk = 0;
    t->eval();
    t->aclk = 1;
    t->eval();
}

void reset(VStreamConcatFifo* t)
{
    t->resetn = 0;
    clk(t);
    t->resetn = 1;
    clk(t);
}

TEST_CASE("Check stream concatenation", "[StreamConcatFifo]")
{
    VStreamConcatFifo* t = new VStreamConcatFifo();

    t->stream0_tvalid = 0;
    t->stream1_tvalid = 0;
    t->stream2_tvalid = 0;
    t->stream3_tvalid = 0;

    t->stream_out_tready = 0;

    reset(t);

    CHECK(t->stream_out_tvalid == 0);
    CHECK(t->stream0_tready == 1);
    CHECK(t->stream1_tready == 1);
    CHECK(t->stream2_tready == 1);
    CHECK(t->stream3_tready == 1);

    t->stream_out_tready = 1;

    for (uint32_t i = 0; i < 2; i++)
    {
        // Push one byte into stream0
        t->stream0_tvalid = 1;
        t->stream0_tdata = 0;
        clk(t);
        CHECK(t->stream_out_tvalid == 0);
        CHECK(t->stream0_tready == 1);

        // Push one byte into stream1
        t->stream0_tvalid = 0;
        t->stream1_tvalid = 1;
        t->stream1_tdata = 1;
        clk(t);
        CHECK(t->stream_out_tvalid == 0);
        CHECK(t->stream1_tready == 1);

        // Push one byte into stream2
        t->stream1_tvalid = 0;
        t->stream2_tvalid = 1;
        t->stream2_tdata = 2;
        clk(t);
        CHECK(t->stream_out_tvalid == 0);
        CHECK(t->stream2_tready == 1);

        // Push one byte into stream3
        t->stream2_tvalid = 0;
        t->stream3_tvalid = 1;
        t->stream3_tdata = 3;
        clk(t);
        CHECK(t->stream_out_tvalid == 1);
        CHECK(t->stream_out_tdata == 0x03020100);
        CHECK(t->stream3_tready == 1);

        t->stream3_tvalid = 0;
        clk(t);
        CHECK(t->stream_out_tvalid == 0);
    }

    delete t;
}

TEST_CASE("Check stream interruption", "[StreamConcatFifo]")
{
    VStreamConcatFifo* t = new VStreamConcatFifo();

    t->stream0_tvalid = 0;
    t->stream1_tvalid = 0;
    t->stream2_tvalid = 0;
    t->stream3_tvalid = 0;

    t->stream_out_tready = 0;

    reset(t);

    // Trigger one transfer
    t->stream0_tvalid = 1;
    t->stream0_tdata = 0;
    t->stream1_tvalid = 1;
    t->stream1_tdata = 1;
    t->stream2_tvalid = 1;
    t->stream2_tdata = 2;
    t->stream3_tvalid = 1;
    t->stream3_tdata = 3;
    clk(t);
    CHECK(t->stream_out_tvalid == 1);
    CHECK(t->stream_out_tdata == 0x03020100);
    CHECK(t->stream0_tready == 1);
    CHECK(t->stream1_tready == 1);
    CHECK(t->stream2_tready == 1);
    CHECK(t->stream3_tready == 1);

    // Cycle without read
    t->stream3_tvalid = 0;
    clk(t);
    CHECK(t->stream_out_tvalid == 1);
    CHECK(t->stream_out_tdata == 0x03020100);
    CHECK(t->stream3_tready == 1);

    // Set output (combinatorial). Must have a direct effect on the output.
    t->stream_out_tready = 1;
    clk(t);
    CHECK(t->stream_out_tvalid == 0);

    delete t;
}

TEST_CASE("Check full", "[StreamConcatFifo]")
{
    VStreamConcatFifo* t = new VStreamConcatFifo();

    t->stream0_tvalid = 0;
    t->stream1_tvalid = 0;
    t->stream2_tvalid = 0;
    t->stream3_tvalid = 0;

    t->stream_out_tready = 0;

    reset(t);

    CHECK(t->stream_out_tvalid == 0);
    CHECK(t->stream0_tready == 1);
    CHECK(t->stream1_tready == 1);
    CHECK(t->stream2_tready == 1);
    CHECK(t->stream3_tready == 1);

    t->stream_out_tready = 0;

    // Make the fifo full
    for (uint32_t i = 0; i < 31; i++)
    {
        t->stream0_tvalid = 1;
        t->stream0_tdata = 0;
        t->stream1_tvalid = 1;
        t->stream1_tdata = 1;
        t->stream2_tvalid = 1;
        t->stream2_tdata = 2;
        t->stream3_tvalid = 1;
        t->stream3_tdata = 3;
        clk(t);
        CHECK(t->stream_out_tvalid == 1);
        CHECK(t->stream0_tready == 1);
        CHECK(t->stream1_tready == 1);
        CHECK(t->stream2_tready == 1);
        CHECK(t->stream3_tready == 1);
    }

    // Check that the fifos are full
    clk(t);
    CHECK(t->stream_out_tvalid == 1);
    CHECK(t->stream0_tready == 0);
    CHECK(t->stream1_tready == 0);
    CHECK(t->stream2_tready == 0);
    CHECK(t->stream3_tready == 0);

    delete t;
}
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
#include "VMemoryReadRequestGenerator.h"

void clk(VMemoryReadRequestGenerator* t)
{
    t->aclk = 0;
    t->eval();
    t->aclk = 1;
    t->eval();
}

void reset(VMemoryReadRequestGenerator* t)
{
    t->resetn = 0;
    clk(t);
    t->resetn = 1;
    clk(t);
}

TEST_CASE("Check complete memory request sequence (uninterrupted)", "[MemoryReadRequestGenerator]")
{
    VMemoryReadRequestGenerator* t = new VMemoryReadRequestGenerator();
    t->s_fetch_axis_tlast = 0;
    t->s_fetch_axis_tvalid = 0;
    t->m_mem_axi_arready = 1;
    reset(t);

    t->confAddr = 0x1000'0000;

    // Do this twice, to see, if also a second cycle is possible
    for (uint32_t i = 0; i < 2; i++)
    {
        REQUIRE(t->s_fetch_axis_tready == true);
        REQUIRE(t->m_mem_axi_arvalid == false);

        // Send 1. fetch
        t->s_fetch_axis_tvalid = true;
        t->s_fetch_axis_tdest = 0;
        clk(t);
        REQUIRE(t->s_fetch_axis_tready == true);
        REQUIRE(t->m_mem_axi_arvalid == false);

        // Send 2. fetch
        t->s_fetch_axis_tvalid = true;
        t->s_fetch_axis_tdest = 1;
        clk(t);
        REQUIRE(t->s_fetch_axis_tready == true);
        REQUIRE(t->m_mem_axi_arvalid == false);

        // Send 3. fetch (triggers a mem write). In this clock, the access is prepared, but will be executed in the next clock
        t->s_fetch_axis_tvalid = true;
        t->s_fetch_axis_tdest = 2;
        clk(t);
        REQUIRE(t->s_fetch_axis_tready == true);

        // Send 4. fetch. Triggers also the memory request
        t->s_fetch_axis_tvalid = true;
        t->s_fetch_axis_tdest = 3;
        clk(t);
        REQUIRE(t->s_fetch_axis_tready == true);
        REQUIRE(t->m_mem_axi_arid == 0);
        REQUIRE(t->m_mem_axi_arlen == 0);
        REQUIRE(t->m_mem_axi_arsize == 2);
        REQUIRE(t->m_mem_axi_arburst == 1);
        REQUIRE(t->m_mem_axi_arlock == 0);
        REQUIRE(t->m_mem_axi_arcache == 0);
        REQUIRE(t->m_mem_axi_arprot == 0);
        REQUIRE(t->m_mem_axi_arvalid == true);
        REQUIRE(t->m_mem_axi_araddr == 0x1000'0000);

        // Send 5. fetch. Deasserts the memory request. Also triggers a new one.
        t->s_fetch_axis_tvalid = true;
        t->s_fetch_axis_tdest = 4;
        clk(t);
        REQUIRE(t->s_fetch_axis_tready == true);
        REQUIRE(t->m_mem_axi_arvalid == false);

        // Send 6. fetch. Send the same address as in 5. but with asserted tlast. Memory executes request from 3. and 4.
        t->s_fetch_axis_tvalid = true;
        t->s_fetch_axis_tlast = true;
        t->s_fetch_axis_tdest = 4;
        clk(t);
        REQUIRE(t->s_fetch_axis_tready == false); // It is shortly deasserted when the last fetch is coming
        REQUIRE(t->m_mem_axi_arvalid == true);
        REQUIRE(t->m_mem_axi_araddr == 0x1000'0004);

        // Execute no fetch. Memory request is still pending
        t->s_fetch_axis_tvalid = false;
        t->s_fetch_axis_tlast = false;
        clk(t);
        REQUIRE(t->s_fetch_axis_tready == true);
        REQUIRE(t->m_mem_axi_arvalid == false);

        // Execute no fetch. Memory request is arriving 
        t->s_fetch_axis_tvalid = false;
        clk(t);
        REQUIRE(t->s_fetch_axis_tready == true);
        REQUIRE(t->m_mem_axi_arvalid == true);
        REQUIRE(t->m_mem_axi_araddr == 0x1000'0008);

        // Execute more clock cycles, no additional requests should be executed
        clk(t);
        REQUIRE(t->s_fetch_axis_tready == true);
        REQUIRE(t->m_mem_axi_arvalid == false);
        clk(t);
        REQUIRE(t->s_fetch_axis_tready == true);
        REQUIRE(t->m_mem_axi_arvalid == false);
        clk(t);
        REQUIRE(t->s_fetch_axis_tready == true);
        REQUIRE(t->m_mem_axi_arvalid == false);
        clk(t);
        REQUIRE(t->s_fetch_axis_tready == true);
        REQUIRE(t->m_mem_axi_arvalid == false);
    }

    // Destroy model
    delete t;
}

TEST_CASE("Interrupted fetch stream", "[MemoryReadRequestGenerator]")
{
    VMemoryReadRequestGenerator* t = new VMemoryReadRequestGenerator();
    t->s_fetch_axis_tlast = 0;
    t->s_fetch_axis_tvalid = 0;
    t->m_mem_axi_arready = 1;
    reset(t);

    t->confAddr = 0x1000'0000;

    // Do this twice, to see, if also a second cycle is possible
    for (uint32_t i = 0; i < 2; i++)
    {
        // Send 1. fetch
        t->s_fetch_axis_tvalid = true;
        t->s_fetch_axis_tdest = 0;
        clk(t);
        REQUIRE(t->s_fetch_axis_tready == true);
        REQUIRE(t->m_mem_axi_arvalid == false);

        // Send 2.1 fetch
        t->s_fetch_axis_tvalid = false;
        t->s_fetch_axis_tdest = 1;
        clk(t);
        REQUIRE(t->s_fetch_axis_tready == true);
        REQUIRE(t->m_mem_axi_arvalid == false);

        // Send 2.2 fetch
        t->s_fetch_axis_tvalid = false;
        t->s_fetch_axis_tdest = 1;
        clk(t);
        REQUIRE(t->s_fetch_axis_tready == true);
        REQUIRE(t->m_mem_axi_arvalid == false);

        // Send 2.3 fetch
        t->s_fetch_axis_tvalid = true;
        t->s_fetch_axis_tdest = 1;
        clk(t);
        REQUIRE(t->s_fetch_axis_tready == true);
        REQUIRE(t->m_mem_axi_arvalid == false);

        // Send 3. fetch.
        t->s_fetch_axis_tvalid = true;
        t->s_fetch_axis_tdest = 2;
        clk(t);
        REQUIRE(t->s_fetch_axis_tready == true);
        REQUIRE(t->m_mem_axi_arvalid == false);

        // Send 4.1 fetch.
        t->s_fetch_axis_tvalid = false;
        t->s_fetch_axis_tlast = true;
        t->s_fetch_axis_tdest = 3;
        clk(t);
        REQUIRE(t->s_fetch_axis_tready == true);
        REQUIRE(t->m_mem_axi_arvalid == true);
        REQUIRE(t->m_mem_axi_araddr == 0x1000'0000);

        // Send 4.2 fetch.
        t->s_fetch_axis_tvalid = true;
        t->s_fetch_axis_tlast = true;
        t->s_fetch_axis_tdest = 3;
        clk(t);
        REQUIRE(t->s_fetch_axis_tready == false); // It is shortly deasserted when the last fetch is coming
        REQUIRE(t->m_mem_axi_arvalid == false);

        // Execute no fetch. Memory request is still pending
        t->s_fetch_axis_tvalid = false;
        t->s_fetch_axis_tlast = false;
        clk(t);
        REQUIRE(t->s_fetch_axis_tready == true);
        REQUIRE(t->m_mem_axi_arvalid == false);

        // Execute no fetch. Memory request is arriving 
        t->s_fetch_axis_tvalid = false;
        clk(t);
        REQUIRE(t->s_fetch_axis_tready == true);
        REQUIRE(t->m_mem_axi_arvalid == true);
        REQUIRE(t->m_mem_axi_araddr == 0x1000'0004);
    }

    // Destroy model
    delete t;
}

TEST_CASE("Interrupted memory stream with tlast in skid buffer", "[MemoryReadRequestGenerator]")
{
    VMemoryReadRequestGenerator* t = new VMemoryReadRequestGenerator();
    t->s_fetch_axis_tlast = 0;
    t->s_fetch_axis_tvalid = 0;
    t->m_mem_axi_arready = 0;
    reset(t);

    t->confAddr = 0x1000'0000;

    // Do this twice, to see, if also a second cycle is possible
    for (uint32_t i = 0; i < 2; i++)
    {
        // Send 1. fetch
        t->s_fetch_axis_tvalid = true;
        t->s_fetch_axis_tdest = 0;
        clk(t);
        REQUIRE(t->s_fetch_axis_tready == true);
        REQUIRE(t->m_mem_axi_arvalid == false);

        // Send 2. fetch
        t->s_fetch_axis_tvalid = true;
        t->s_fetch_axis_tdest = 1;
        clk(t);
        REQUIRE(t->s_fetch_axis_tready == true);
        REQUIRE(t->m_mem_axi_arvalid == false);

        // Send 3. fetch
        t->s_fetch_axis_tvalid = true;
        t->s_fetch_axis_tdest = 2;
        clk(t);
        REQUIRE(t->s_fetch_axis_tready == true);

        // Send 4. fetch
        t->s_fetch_axis_tvalid = true;
        t->s_fetch_axis_tdest = 3;
        clk(t);
        REQUIRE(t->m_mem_axi_arvalid == true);
        REQUIRE(t->m_mem_axi_araddr == 0x1000'0000);

        // Send 5. fetch
        t->s_fetch_axis_tvalid = true;
        t->s_fetch_axis_tdest = 4;
        t->s_fetch_axis_tlast = true;
        clk(t);
        REQUIRE(t->s_fetch_axis_tready == false);
        REQUIRE(t->m_mem_axi_arvalid == true);
        REQUIRE(t->m_mem_axi_araddr == 0x1000'0000);

        // Mem requests
        t->s_fetch_axis_tvalid = false;
        clk(t);
        REQUIRE(t->s_fetch_axis_tready == false);
        REQUIRE(t->m_mem_axi_arvalid == true);
        REQUIRE(t->m_mem_axi_araddr == 0x1000'0000);

        // Mem requests
        t->s_fetch_axis_tvalid = false;
        clk(t);
        REQUIRE(t->s_fetch_axis_tready == false);
        REQUIRE(t->m_mem_axi_arvalid == true);
        REQUIRE(t->m_mem_axi_araddr == 0x1000'0000);

        // Mem requests
        t->s_fetch_axis_tvalid = false;
        t->m_mem_axi_arready = true;
        clk(t);
        REQUIRE(t->s_fetch_axis_tready == false);
        REQUIRE(t->m_mem_axi_arvalid == false);

        // Mem requests
        t->s_fetch_axis_tvalid = false;
        t->m_mem_axi_arready = true;
        clk(t);
        REQUIRE(t->s_fetch_axis_tready == false);
        REQUIRE(t->m_mem_axi_arvalid == true);
        REQUIRE(t->m_mem_axi_araddr == 0x1000'0004);

        // Mem requests
        t->s_fetch_axis_tvalid = false;
        t->s_fetch_axis_tlast = false;
        t->m_mem_axi_arready = true;
        clk(t);
        REQUIRE(t->s_fetch_axis_tready == true);
        REQUIRE(t->m_mem_axi_arvalid == false);

        // Last mem request
        t->s_fetch_axis_tvalid = false;
        t->m_mem_axi_arready = false;
        clk(t);
        REQUIRE(t->s_fetch_axis_tready == true);
        REQUIRE(t->m_mem_axi_arvalid == true);
        REQUIRE(t->m_mem_axi_araddr == 0x1000'0008);

        // Last mem request
        t->s_fetch_axis_tvalid = false;
        t->m_mem_axi_arready = false;
        clk(t);
        REQUIRE(t->s_fetch_axis_tready == true);
        REQUIRE(t->m_mem_axi_arvalid == true);
        REQUIRE(t->m_mem_axi_araddr == 0x1000'0008);

        // Last mem request
        t->s_fetch_axis_tvalid = false;
        t->m_mem_axi_arready = true;
        clk(t);
        REQUIRE(t->s_fetch_axis_tready == true);
        REQUIRE(t->m_mem_axi_arvalid == false);

        // Last mem request
        t->s_fetch_axis_tvalid = false;
        t->m_mem_axi_arready = false;
        clk(t);
        REQUIRE(t->s_fetch_axis_tready == true);
        REQUIRE(t->m_mem_axi_arvalid == false);
    }

    // Destroy model
    delete t;
}

TEST_CASE("tlast after one clock", "[MemoryReadRequestGenerator]")
{
    VMemoryReadRequestGenerator* t = new VMemoryReadRequestGenerator();
    t->s_fetch_axis_tlast = 0;
    t->s_fetch_axis_tvalid = 0;
    t->m_mem_axi_arready = 1;
    reset(t);

    t->confAddr = 0x1000'0000;

    // Do this twice, to see, if also a second cycle is possible
    for (uint32_t i = 0; i < 2; i++)
    {
        // Send 1. fetch
        t->s_fetch_axis_tvalid = true;
        t->s_fetch_axis_tdest = 0;
        t->s_fetch_axis_tlast = true;
        clk(t);
        REQUIRE(t->s_fetch_axis_tready == false);
        REQUIRE(t->m_mem_axi_arvalid == false);

        // Wait for memory request
        t->s_fetch_axis_tvalid = false;
        clk(t);
        REQUIRE(t->s_fetch_axis_tready == true);
        REQUIRE(t->m_mem_axi_arvalid == false);

        // Triggers the memory request
        t->s_fetch_axis_tvalid = false;
        clk(t);
        REQUIRE(t->s_fetch_axis_tready == true);
        REQUIRE(t->m_mem_axi_arvalid == true);
        REQUIRE(t->m_mem_axi_araddr == 0x1000'0000);

        // End
        clk(t);
        REQUIRE(t->s_fetch_axis_tready == true);
        REQUIRE(t->m_mem_axi_arvalid == false);
    }

    // Destroy model
    delete t;
}
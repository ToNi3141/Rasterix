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

#include "general.hpp"
#include <math.h>
#include <array>
#include <algorithm>

// Include model header, generated from Verilating "top.v"
#include "VStreamSemaphore.h"

TEST_CASE("Test Forwarding", "[VStreamSemaphore]")
{
    VStreamSemaphore* t = new VStreamSemaphore();
    
    t->sigRelease = 0;
    t->m_axis_tready = 1;
    rr::ut::reset(t);
    CHECK(t->s_axis_tready == 1);
    CHECK(t->m_axis_tvalid == 0);
    CHECK(t->released == 1);

    t->s_axis_tvalid = 1;
    t->sigLock = 1;
    t->s_axis_tlast = 0;
    t->s_axis_tdata = 0x12345678;
    t->s_axis_tkeep = 1;
    rr::ut::clk(t);
    CHECK(t->m_axis_tvalid == 1);
    CHECK(t->m_axis_tlast == 0);
    CHECK(t->m_axis_tdata == 0x12345678);
    CHECK(t->m_axis_tkeep == 1);
    CHECK(t->released == 0);

    t->sigRelease = 1;
    t->s_axis_tvalid = 0;
    t->sigLock = 0;
    rr::ut::clk(t);
    CHECK(t->m_axis_tvalid == 0);
    CHECK(t->released == 1);

    // Destroy model
    delete t;
}

TEST_CASE("Test Stall", "[VStreamSemaphore]")
{
    static constexpr uint32_t MAX_ITERATIONS { 128 };
    VStreamSemaphore* t = new VStreamSemaphore();
    
    t->sigRelease = 0;
    t->m_axis_tready = 1;
    rr::ut::reset(t);
    CHECK(t->s_axis_tready == 1);
    CHECK(t->m_axis_tvalid == 0);
    CHECK(t->released == 1);

    for (uint32_t i = 0; i < MAX_ITERATIONS; i++)
    {
        t->s_axis_tvalid = 1;
        t->sigLock = 1;
        t->s_axis_tlast = 0;
        t->s_axis_tdata = i;
        t->s_axis_tkeep = 0;
        rr::ut::clk(t);
        CHECK(t->m_axis_tvalid == 1);
        CHECK(t->m_axis_tlast == 0);
        CHECK(t->m_axis_tdata == i);
        CHECK(t->m_axis_tkeep == 0);
        CHECK(t->s_axis_tready == (i < (MAX_ITERATIONS - 1)));
        CHECK(t->released == 0);
    }

    t->sigRelease = 1;
    t->s_axis_tvalid = 0;
    t->sigLock = 0;
    for (uint32_t i = 0; i < MAX_ITERATIONS; i++)
    {
        rr::ut::clk(t);
        CHECK(t->s_axis_tready == 1);
        CHECK(t->m_axis_tvalid == 0);
        CHECK(t->released == !(i < (MAX_ITERATIONS - 1)));
    }

    // Destroy model
    delete t;
}

TEST_CASE("Test flow control", "[VStreamSemaphore]")
{
    VStreamSemaphore* t = new VStreamSemaphore();
    
    t->sigRelease = 0;
    t->m_axis_tready = 0;
    rr::ut::reset(t);
    CHECK(t->s_axis_tready == 1);
    CHECK(t->m_axis_tvalid == 0);
    CHECK(t->released == 1);

    t->sigRelease = 0;
    t->m_axis_tready = 0;
    t->s_axis_tvalid = 1;
    t->sigLock = 1;
    t->s_axis_tlast = 0;
    t->s_axis_tdata = 0x12345678;
    t->s_axis_tkeep = 1;
    rr::ut::clk(t);
    CHECK(t->m_axis_tvalid == 1);
    CHECK(t->m_axis_tlast == 0);
    CHECK(t->m_axis_tdata == 0x12345678);
    CHECK(t->m_axis_tkeep == 1);
    CHECK(t->released == 0);

    t->sigRelease = 0;
    t->m_axis_tready = 0;
    t->s_axis_tvalid = 1;
    t->sigLock = 1;
    t->s_axis_tlast = 0;
    t->s_axis_tdata = 0x87654321;
    t->s_axis_tkeep = 1;
    rr::ut::clk(t);
    CHECK(t->m_axis_tvalid == 1);
    CHECK(t->m_axis_tlast == 0);
    CHECK(t->m_axis_tdata == 0x12345678);
    CHECK(t->m_axis_tkeep == 1);
    CHECK(t->released == 0);

    t->sigRelease = 1;
    t->m_axis_tready = 1;
    t->s_axis_tvalid = 1;
    t->sigLock = 1;
    t->s_axis_tlast = 0;
    t->s_axis_tdata = 0x87654321;
    t->s_axis_tkeep = 1;
    rr::ut::clk(t);
    CHECK(t->m_axis_tvalid == 1);
    CHECK(t->m_axis_tlast == 0);
    CHECK(t->m_axis_tdata == 0x87654321);
    CHECK(t->m_axis_tkeep == 1);
    CHECK(t->released == 0);

    t->sigRelease = 1;
    t->s_axis_tvalid = 0;
    t->sigLock = 0;
    rr::ut::clk(t);
    CHECK(t->m_axis_tvalid == 0);
    CHECK(t->released == 1);

    // Destroy model
    delete t;
}

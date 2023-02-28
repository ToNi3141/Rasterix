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
#include "VDisplayController8BitILI9341.h"

void clk(VDisplayController8BitILI9341& t)
{
    t.aclk = 0;
    t.eval();
    t.aclk = 1;
    t.eval();
}

void reset(VDisplayController8BitILI9341& t)
{
    t.resetn = 0;

    t.s_axis_tvalid = 0;
    t.s_axis_tlast = 0;
    t.s_axis_tdata = 0;

    clk(t);
    
    REQUIRE(t.rst == 0);
    t.resetn = 1;

    clk(t);

    REQUIRE(t.rst == 1);
}

void initTest(VDisplayController8BitILI9341& t, const bool dc, const uint8_t data, bool last = false) 
{
    REQUIRE(t.s_axis_tready == 0);
    REQUIRE(t.rd == 1);
    REQUIRE(t.wr == 0);
    REQUIRE(t.cs == 0);
    REQUIRE(t.dc == dc);
    REQUIRE(t.data == data);
    clk(t);
    REQUIRE(t.s_axis_tready == last);
    REQUIRE(t.rd == 1);
    REQUIRE(t.wr == 1);
    REQUIRE(t.cs == 0);
    REQUIRE(t.dc == dc);
    REQUIRE(t.data == data);
    clk(t);
}

TEST_CASE("Stream data", "[Display]")
{
    VDisplayController8BitILI9341* top = new VDisplayController8BitILI9341();
    VDisplayController8BitILI9341& t = *top;

    reset(t);

    t.s_axis_tvalid = 1;
    t.s_axis_tdata = 0xffaa;

    // Initialization sequence for a ILI9341 with 240x320px
    initTest(t, false, 0x01);  
    initTest(t, true, 0); 
    initTest(t, false, 0x28);  
    initTest(t, true, 0); 
    initTest(t, false, 0xC0);  
    initTest(t, true, 0x23); 
    initTest(t, false, 0xC1);  
    initTest(t, true, 0x10); 
    initTest(t, false, 0xC5);  
    initTest(t, true, 0x2B); 
    initTest(t, true, 0x2B); 
    initTest(t, false, 0xC7);  
    initTest(t, true, 0xC0); 
    initTest(t, false, 0x36);  
    initTest(t, true, 0x00 | 0x08 | 0x40);
    initTest(t, false, 0x3A);  
    initTest(t, true, 0x55); 
    initTest(t, false, 0xB1);  
    initTest(t, true, 0x00); 
    initTest(t, true, 0x1B); 
    initTest(t, false, 0xB7); 
    initTest(t, true, 0x07); 
    initTest(t, false, 0x11); 
    initTest(t, true, 0);
    initTest(t, false, 0x29); 
    initTest(t, true, 0);
    initTest(t, false, 0x2A);
    initTest(t, true, 0); 
    initTest(t, true, 0); 
    initTest(t, true, 0x00); 
    initTest(t, true, 0xEF); 
    initTest(t, false, 0x2B);
    initTest(t, true, 0); 
    initTest(t, true, 0); 
    initTest(t, true, 0x01); 
    initTest(t, true, 0x3F); 
    initTest(t, false, 0x2C, true);
    
    // stream clk 0 fetch
    REQUIRE(t.s_axis_tready == 0);
    REQUIRE(t.rd == 1);
    REQUIRE(t.wr == 1);
    REQUIRE(t.cs == 0);
    //REQUIRE(t.dc == 1);
    //REQUIRE(t.data == 0x08);
    t.s_axis_tvalid = 1;
    t.s_axis_tdata = 0x550f;
    clk(t);
    // stream clk 0 stream
    REQUIRE(t.s_axis_tready == 0);
    REQUIRE(t.rd == 1);
    REQUIRE(t.wr == 0);
    REQUIRE(t.cs == 0);
    REQUIRE(t.dc == 1);
    REQUIRE(t.data == 0xff);
    clk(t);
    // stream clk 0 stream
    REQUIRE(t.s_axis_tready == 0);
    REQUIRE(t.rd == 1);
    REQUIRE(t.wr == 1);
    REQUIRE(t.cs == 0);
    REQUIRE(t.dc == 1);
    REQUIRE(t.data == 0xff);
    clk(t);
    // stream clk 0 stream
    REQUIRE(t.s_axis_tready == 0);
    REQUIRE(t.rd == 1);
    REQUIRE(t.wr == 0);
    REQUIRE(t.cs == 0);
    REQUIRE(t.dc == 1);
    REQUIRE(t.data == 0xaa);
    clk(t);
    // stream clk 0 stream
    REQUIRE(t.s_axis_tready == 1);
    REQUIRE(t.rd == 1);
    REQUIRE(t.wr == 1);
    REQUIRE(t.cs == 0);
    REQUIRE(t.dc == 1);
    REQUIRE(t.data == 0xaa);
    clk(t);



    // stream clk 1 fetch
    REQUIRE(t.s_axis_tready == 0);
    REQUIRE(t.rd == 1);
    REQUIRE(t.wr == 1);
    REQUIRE(t.cs == 0);
    //REQUIRE(t.dc == 1);
    //REQUIRE(t.data == 0x08);
    clk(t);
    // stream clk 1 stream
    REQUIRE(t.s_axis_tready == 0);
    REQUIRE(t.rd == 1);
    REQUIRE(t.wr == 0);
    REQUIRE(t.cs == 0);
    REQUIRE(t.dc == 1);
    REQUIRE(t.data == 0x55);
    clk(t);
    // stream clk 1 stream
    REQUIRE(t.s_axis_tready == 0);
    REQUIRE(t.rd == 1);
    REQUIRE(t.wr == 1);
    REQUIRE(t.cs == 0);
    REQUIRE(t.dc == 1);
    REQUIRE(t.data == 0x55);
    clk(t);
    // stream clk 1 stream
    REQUIRE(t.s_axis_tready == 0);
    REQUIRE(t.rd == 1);
    REQUIRE(t.wr == 0);
    REQUIRE(t.cs == 0);
    REQUIRE(t.dc == 1);
    REQUIRE(t.data == 0x0f);
    clk(t);
    // stream clk 1 stream
    REQUIRE(t.s_axis_tready == 1);
    REQUIRE(t.rd == 1);
    REQUIRE(t.wr == 1);
    REQUIRE(t.cs == 0);
    REQUIRE(t.dc == 1);
    REQUIRE(t.data == 0x0f);
    clk(t);

    // Final model cleanup
    top->final();

    // Destroy model
    delete top;
}


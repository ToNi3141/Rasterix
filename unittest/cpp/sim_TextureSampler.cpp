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

#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "3rdParty/catch.hpp"
#include <math.h>
#include <array>
#include <algorithm>

// Include common routines
#include <verilated.h>

// Include model header, generated from Verilating "top.v"
#include "VTextureSamplerTestModule.h"

void clk(VTextureSamplerTestModule* t)
{
    t->aclk = 0;
    t->eval();
    t->aclk = 1;
    t->eval();
}

void reset(VTextureSamplerTestModule* t)
{
    t->resetn = 0;
    clk(t);
    t->resetn = 1;
    clk(t);
}

void uploadTexture(VTextureSamplerTestModule* top) 
{
    // 2x2 texture
    // | 0xf000 | 0x0f00 |
    // | 0x00f0 | 0x000f |
    top->textureSizeWidth = 0x1;
    top->textureSizeHeight = 0x1;

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
    VTextureSamplerTestModule* top = new VTextureSamplerTestModule();
    reset(top);

    uploadTexture(top);

    // (0, 0)
    top->texelS = 0;
    top->texelT = 0;
    clk(top);
    clk(top);
    clk(top);
    clk(top);
    REQUIRE(top->texel00 == 0xff000000);
    REQUIRE(top->texel01 == 0x00ff0000);
    REQUIRE(top->texel10 == 0x0000ff00);
    REQUIRE(top->texel11 == 0x000000ff);

    // (0.99.., 0.0)
    top->texelS = 0x7fff;
    top->texelT = 0;
    clk(top);
    clk(top);
    clk(top);
    clk(top);
    REQUIRE(top->texel00 == 0x00ff0000);
    REQUIRE(top->texel01 == 0xff000000);
    REQUIRE(top->texel10 == 0x000000ff);
    REQUIRE(top->texel11 == 0x0000ff00);

    // (0.0, 0.99..)
    top->texelS = 0;
    top->texelT = 0x7fff;
    clk(top);
    clk(top);
    clk(top);
    clk(top);
    REQUIRE(top->texel00 == 0x0000ff00);
    REQUIRE(top->texel01 == 0x000000ff);
    REQUIRE(top->texel10 == 0xff000000);
    REQUIRE(top->texel11 == 0x00ff0000);

    // (0.99.., 0.99..)
    top->texelS = 0x7fff;
    top->texelT = 0x7fff;
    clk(top);
    clk(top);
    clk(top);
    clk(top);
    REQUIRE(top->texel00 == 0x000000ff);
    REQUIRE(top->texel01 == 0x0000ff00);
    REQUIRE(top->texel10 == 0x00ff0000);
    REQUIRE(top->texel11 == 0xff000000);


    // (1.0, 0.0)
    top->texelS = 0x8000;
    top->texelT = 0;
    clk(top);
    clk(top);
    clk(top);
    clk(top);
    REQUIRE(top->texel00 == 0xff000000);
    REQUIRE(top->texel01 == 0x00ff0000);
    REQUIRE(top->texel10 == 0x0000ff00);
    REQUIRE(top->texel11 == 0x000000ff);

    // (0.0, 1.0)
    top->texelS = 0;
    top->texelT = 0x8000;
    clk(top);
    clk(top);
    clk(top);
    clk(top);
    REQUIRE(top->texel00 == 0xff000000);
    REQUIRE(top->texel01 == 0x00ff0000);
    REQUIRE(top->texel10 == 0x0000ff00);
    REQUIRE(top->texel11 == 0x000000ff);

    // (1.0, 1.0)
    top->texelS = 0x8000;
    top->texelT = 0x8000;
    clk(top);
    clk(top);
    clk(top);
    clk(top);
    REQUIRE(top->texel00 == 0xff000000);
    REQUIRE(top->texel01 == 0x00ff0000);
    REQUIRE(top->texel10 == 0x0000ff00);
    REQUIRE(top->texel11 == 0x000000ff);

    // Destroy model
    delete top;
}

TEST_CASE("Get various values from the texture buffer with pipeline test", "[TextureBuffer]")
{
    VTextureSamplerTestModule* top = new VTextureSamplerTestModule();
    reset(top);

    uploadTexture(top);

    // (0, 0)
    top->texelS = 0;
    top->texelT = 0;
    clk(top);

    // (0.99.., 0.0)
    top->texelS = 0x7fff;
    top->texelT = 0;
    clk(top);

    // (0.0, 0.99..)
    top->texelS = 0;
    top->texelT = 0x7fff;
    clk(top);

    // (0.99.., 0.99..)
    top->texelS = 0x7fff;
    top->texelT = 0x7fff;
    clk(top);

    // Result of (0, 0)
    REQUIRE(top->texel00 == 0xff000000);
    REQUIRE(top->texel01 == 0x00ff0000);
    REQUIRE(top->texel10 == 0x0000ff00);
    REQUIRE(top->texel11 == 0x000000ff);

    clk(top);

    // (0.99.., 0.0)
    REQUIRE(top->texel00 == 0x00ff0000);
    REQUIRE(top->texel01 == 0xff000000);
    REQUIRE(top->texel10 == 0x000000ff);
    REQUIRE(top->texel11 == 0x0000ff00);

    clk(top);

    // Result of (0.0, 0.99..)
    REQUIRE(top->texel00 == 0x0000ff00);
    REQUIRE(top->texel01 == 0x000000ff);
    REQUIRE(top->texel10 == 0xff000000);
    REQUIRE(top->texel11 == 0x00ff0000);

    clk(top);

    // Result of (0.99.., 0.99..)
    REQUIRE(top->texel00 == 0x000000ff);
    REQUIRE(top->texel01 == 0x0000ff00);
    REQUIRE(top->texel10 == 0x00ff0000);
    REQUIRE(top->texel11 == 0xff000000);


    // Destroy model
    delete top;
}

TEST_CASE("Check sub coordinates", "[TextureBuffer]")
{
    VTextureSamplerTestModule* top = new VTextureSamplerTestModule();
    reset(top);

    uploadTexture(top);

    // texel (0.0, 0.0)
    top->texelS = 0x0000;
    top->texelT = 0x0000;
    clk(top);
    clk(top);
    clk(top);
    clk(top);
    REQUIRE(top->texel00 == 0xff000000);
    REQUIRE(top->texel01 == 0x00ff0000);
    REQUIRE(top->texel10 == 0x0000ff00);
    REQUIRE(top->texel11 == 0x000000ff);
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
    clk(top);
    clk(top);
    clk(top);
    REQUIRE(top->texel00 == 0xff000000);
    REQUIRE(top->texel01 == 0x00ff0000);
    REQUIRE(top->texel10 == 0x0000ff00);
    REQUIRE(top->texel11 == 0x000000ff);
    // sub texel (0.25, 0.25)
    REQUIRE(top->texelSubCoordS == 0x4000); 
    REQUIRE(top->texelSubCoordT == 0x4000); 

    // texel (0.125, 0.375) 
    top->texelS = 0x1000;
    top->texelT = 0x3000;
    clk(top);
    clk(top);
    clk(top);
    clk(top);
    REQUIRE(top->texel00 == 0xff000000);
    REQUIRE(top->texel01 == 0x00ff0000);
    REQUIRE(top->texel10 == 0x0000ff00);
    REQUIRE(top->texel11 == 0x000000ff);
    // sub texel (0.25, 0.75)
    REQUIRE(top->texelSubCoordS == 0x4000); 
    REQUIRE(top->texelSubCoordT == 0xc000);

    // texel (0.375, 0.125) 
    top->texelS = 0x3000;
    top->texelT = 0x1000;
    clk(top);
    clk(top);
    clk(top);
    clk(top);
    REQUIRE(top->texel00 == 0xff000000);
    REQUIRE(top->texel01 == 0x00ff0000);
    REQUIRE(top->texel10 == 0x0000ff00);
    REQUIRE(top->texel11 == 0x000000ff);
    // sub texel (0.75, 0.25)
    REQUIRE(top->texelSubCoordS == 0xc000);
    REQUIRE(top->texelSubCoordT == 0x4000);

    // Destroy model
    delete top;
}

TEST_CASE("clamp to border with s", "[TextureBuffer]")
{
    VTextureSamplerTestModule* top = new VTextureSamplerTestModule();
    reset(top);

    uploadTexture(top);

    top->clampS = 1;
    top->clampT = 0;

    // texel (0.0, 0.0)
    top->texelS = 0x0000;
    top->texelT = 0x0000;
    clk(top);
    clk(top);
    clk(top);
    clk(top);
    REQUIRE(top->texel00 == 0xff000000);
    REQUIRE(top->texel01 == 0x00ff0000);
    REQUIRE(top->texel10 == 0x0000ff00);
    REQUIRE(top->texel11 == 0x000000ff);

    // texel (0.5, 0.0)
    top->texelS = 0x4000;
    top->texelT = 0x0000;
    clk(top);
    clk(top);
    clk(top);
    clk(top);
    REQUIRE(top->texel00 == 0x00ff0000);
    REQUIRE(top->texel01 == 0x00ff0000);
    REQUIRE(top->texel10 == 0x000000ff);
    REQUIRE(top->texel11 == 0x000000ff);

    // texel (0.5, 0.5)
    top->texelS = 0x4000;
    top->texelT = 0x4000;
    clk(top);
    clk(top);
    clk(top);
    clk(top);
    REQUIRE(top->texel00 == 0x000000ff);
    REQUIRE(top->texel01 == 0x000000ff);
    REQUIRE(top->texel10 == 0x00ff0000);
    REQUIRE(top->texel11 == 0x00ff0000);

    // Destroy model
    delete top;
}

TEST_CASE("clamp to border with t", "[TextureBuffer]")
{
    VTextureSamplerTestModule* top = new VTextureSamplerTestModule();
    reset(top);

    uploadTexture(top);

    top->clampS = 0;
    top->clampT = 1;

    // texel (0.0, 0.0)
    top->texelS = 0x0000;
    top->texelT = 0x0000;
    clk(top);
    clk(top);
    clk(top);
    clk(top);
    REQUIRE(top->texel00 == 0xff000000);
    REQUIRE(top->texel01 == 0x00ff0000);
    REQUIRE(top->texel10 == 0x0000ff00);
    REQUIRE(top->texel11 == 0x000000ff);

    // texel (0.0, 0.5)
    top->texelS = 0x0000;
    top->texelT = 0x4000;
    clk(top);
    clk(top);
    clk(top);
    clk(top);
    REQUIRE(top->texel00 == 0x0000ff00);
    REQUIRE(top->texel01 == 0x000000ff);
    REQUIRE(top->texel10 == 0x0000ff00);
    REQUIRE(top->texel11 == 0x000000ff);

    // texel (0.5, 0.5)
    top->texelS = 0x4000;
    top->texelT = 0x4000;
    clk(top);
    clk(top);
    clk(top);
    clk(top);
    REQUIRE(top->texel00 == 0x000000ff);
    REQUIRE(top->texel01 == 0x0000ff00);
    REQUIRE(top->texel10 == 0x000000ff);
    REQUIRE(top->texel11 == 0x0000ff00);

    // Destroy model
    delete top;
}

TEST_CASE("clamp to border with s and t", "[TextureBuffer]")
{
    VTextureSamplerTestModule* top = new VTextureSamplerTestModule();
    reset(top);

    uploadTexture(top);

    top->clampS = 1;
    top->clampT = 1;

    // texel (0.0, 0.0)
    top->texelS = 0x0000;
    top->texelT = 0x0000;
    clk(top);
    clk(top);
    clk(top);
    clk(top);
    REQUIRE(top->texel00 == 0xff000000);
    REQUIRE(top->texel01 == 0x00ff0000);
    REQUIRE(top->texel10 == 0x0000ff00);
    REQUIRE(top->texel11 == 0x000000ff);

    // texel (0.5, 0.0)
    top->texelS = 0x4000;
    top->texelT = 0x0000;
    clk(top);
    clk(top);
    clk(top);
    clk(top);
    REQUIRE(top->texel00 == 0x00ff0000);
    REQUIRE(top->texel01 == 0x00ff0000);
    REQUIRE(top->texel10 == 0x000000ff);
    REQUIRE(top->texel11 == 0x000000ff);

    // texel (0.0, 0.5)
    top->texelS = 0x0000;
    top->texelT = 0x4000;
    clk(top);
    clk(top);
    clk(top);
    clk(top);
    REQUIRE(top->texel00 == 0x0000ff00);
    REQUIRE(top->texel01 == 0x000000ff);
    REQUIRE(top->texel10 == 0x0000ff00);
    REQUIRE(top->texel11 == 0x000000ff);

    // texel (0.5, 0.5)
    top->texelS = 0x4000;
    top->texelT = 0x4000;
    clk(top);
    clk(top);
    clk(top);
    clk(top);
    REQUIRE(top->texel00 == 0x000000ff);
    REQUIRE(top->texel01 == 0x000000ff);
    REQUIRE(top->texel10 == 0x000000ff);
    REQUIRE(top->texel11 == 0x000000ff);

    // Destroy model
    delete top;
}

void uploadMipMap4x8Texture(VTextureSamplerTestModule* top) 
{
    // 4x8 texture
    // |   1 |   2 |   3 |   4 |
    // |   5 |   6 |   7 |   8 |
    // |   9 |   A |   B |   C |
    // |   D |   E |   F |  10 |
    // |  11 |  12 |  13 |  14 |
    // |  15 |  16 |  17 |  18 |
    // |  19 |  1A |  1B |  1C |
    // |  1D |  1E |  1F |  20 |
    // 2x4 texture
    // |  31 |  32 |
    // |  33 |  34 |
    // |  35 |  36 |
    // |  37 |  38 |
    // 1x2 texture
    // |  41 |
    // |  42 |
    // 1x1 texture
    // |  51 |
    top->textureSizeWidth = 0x2;
    top->textureSizeHeight = 0x3;
    top->textureLod = 0;

    top->s_axis_tvalid = 1;
    top->s_axis_tlast = 0;
    top->s_axis_tdata = 0x0002'0001;
    clk(top);
    top->s_axis_tdata = 0x0004'0003;
    clk(top);
    top->s_axis_tdata = 0x0006'0005;
    clk(top);
    top->s_axis_tdata = 0x0008'0007;
    clk(top);
    top->s_axis_tdata = 0x000A'0009;
    clk(top);
    top->s_axis_tdata = 0x000C'000B;
    clk(top);
    top->s_axis_tdata = 0x000E'000D;
    clk(top);
    top->s_axis_tdata = 0x0010'000F;
    clk(top);
    top->s_axis_tdata = 0x0012'0011;
    clk(top);
    top->s_axis_tdata = 0x0014'0013;
    clk(top);
    top->s_axis_tdata = 0x0016'0015;
    clk(top);
    top->s_axis_tdata = 0x0018'0017;
    clk(top);
    top->s_axis_tdata = 0x001A'0019;
    clk(top);
    top->s_axis_tdata = 0x001C'001B;
    clk(top);
    top->s_axis_tdata = 0x001E'001D;
    clk(top);
    top->s_axis_tdata = 0x0020'001F;
    clk(top);

    top->s_axis_tdata = 0x0032'0031;
    clk(top);
    top->s_axis_tdata = 0x0034'0033;
    clk(top);
    top->s_axis_tdata = 0x0036'0035;
    clk(top);
    top->s_axis_tdata = 0x0038'0037;
    clk(top);

    top->s_axis_tdata = 0x0042'0041;
    clk(top);

    top->s_axis_tvalid = 1;
    top->s_axis_tlast = 1;
    top->s_axis_tdata = 0x0000'0051;
    clk(top);

    top->s_axis_tvalid = 0;
    top->s_axis_tlast = 0;

    top->clampS = 0;
    top->clampT = 0;
}

TEST_CASE("Get various values from the mipmap 4x8 texture", "[TextureBuffer]")
{
    VTextureSamplerTestModule* top = new VTextureSamplerTestModule();
    reset(top);

    uploadMipMap4x8Texture(top);

    // (0, 0)
    top->textureLod = 0;
    top->texelS = 0;
    top->texelT = 0;
    clk(top);
    clk(top);
    clk(top);
    clk(top);
    REQUIRE(top->texel00 == 0x11);
    REQUIRE(top->texel01 == 0x22);
    REQUIRE(top->texel10 == 0x55);
    REQUIRE(top->texel11 == 0x66);

    // (0, 0)
    top->textureLod = 1;
    top->texelS = 0;
    top->texelT = 0;
    clk(top);
    clk(top);
    clk(top);
    clk(top);
    REQUIRE(top->texel00 == 0x3311);
    REQUIRE(top->texel01 == 0x3322);
    REQUIRE(top->texel10 == 0x3333);
    REQUIRE(top->texel11 == 0x3344);

    // (0, 0)
    top->textureLod = 2;
    top->texelS = 0;
    top->texelT = 0;
    clk(top);
    clk(top);
    clk(top);
    clk(top);
    REQUIRE(top->texel00 == 0x4411);
    REQUIRE(top->texel01 == 0x4411);
    REQUIRE(top->texel10 == 0x4422);
    REQUIRE(top->texel11 == 0x4422);

    // (0, 0)
    top->textureLod = 3;
    top->texelS = 0;
    top->texelT = 0;
    clk(top);
    clk(top);
    clk(top);
    clk(top);
    REQUIRE(top->texel00 == 0x5511);
    REQUIRE(top->texel01 == 0x5511);
    REQUIRE(top->texel10 == 0x5511);
    REQUIRE(top->texel11 == 0x5511);

    // Destroy model
    delete top;
}

void uploadMipMap4x4Texture(VTextureSamplerTestModule* top) 
{
    // 4x4 texture
    // |   1 |   2 |   3 |   4 |
    // |   5 |   6 |   7 |   8 |
    // |   9 |   A |   B |   C |
    // |   D |   E |   F |  10 |
    // 2x2 texture
    // |  31 |  32 |
    // |  33 |  34 |
    // 1x1 texture
    // |  51 |
    top->textureSizeWidth = 0x2;
    top->textureSizeHeight = 0x2;
    top->textureLod = 0;

    top->s_axis_tvalid = 1;
    top->s_axis_tlast = 0;
    top->s_axis_tdata = 0x0002'0001;
    clk(top);
    top->s_axis_tdata = 0x0004'0003;
    clk(top);
    top->s_axis_tdata = 0x0006'0005;
    clk(top);
    top->s_axis_tdata = 0x0008'0007;
    clk(top);
    top->s_axis_tdata = 0x000A'0009;
    clk(top);
    top->s_axis_tdata = 0x000C'000B;
    clk(top);
    top->s_axis_tdata = 0x000E'000D;
    clk(top);
    top->s_axis_tdata = 0x0010'000F;
    clk(top);

    top->s_axis_tdata = 0x0032'0031;
    clk(top);
    top->s_axis_tdata = 0x0034'0033;
    clk(top);

    top->s_axis_tvalid = 1;
    top->s_axis_tlast = 1;
    top->s_axis_tdata = 0x0000'0051;
    clk(top);

    top->s_axis_tvalid = 0;
    top->s_axis_tlast = 0;

    top->clampS = 0;
    top->clampT = 0;
}

TEST_CASE("Get various values from the mipmap 4x4 texture", "[TextureBuffer]")
{
    VTextureSamplerTestModule* top = new VTextureSamplerTestModule();
    reset(top);

    uploadMipMap4x4Texture(top);

    // (0, 0)
    top->textureLod = 0;
    top->texelS = 0;
    top->texelT = 0;
    clk(top);
    clk(top);
    clk(top);
    clk(top);
    REQUIRE(top->texel00 == 0x11);
    REQUIRE(top->texel01 == 0x22);
    REQUIRE(top->texel10 == 0x55);
    REQUIRE(top->texel11 == 0x66);

    // (0, 0)
    top->textureLod = 1;
    top->texelS = 0;
    top->texelT = 0;
    clk(top);
    clk(top);
    clk(top);
    clk(top);
    REQUIRE(top->texel00 == 0x3311);
    REQUIRE(top->texel01 == 0x3322);
    REQUIRE(top->texel10 == 0x3333);
    REQUIRE(top->texel11 == 0x3344);

    // (0, 0)
    top->textureLod = 2;
    top->texelS = 0;
    top->texelT = 0;
    clk(top);
    clk(top);
    clk(top);
    clk(top);
    REQUIRE(top->texel00 == 0x5511);
    REQUIRE(top->texel01 == 0x5511);
    REQUIRE(top->texel10 == 0x5511);
    REQUIRE(top->texel11 == 0x5511);

    // Destroy model
    delete top;
}

void uploadMipMap4x16Texture(VTextureSamplerTestModule* top) 
{
    // 4x16 texture
    // |   1 |   2 |   3 |   4 |
    // |   5 |   6 |   7 |   8 |
    // |   9 |   A |   B |   C |
    // |   D |   E |   F |  10 |
    // |  11 |  12 |  13 |  14 |
    // |  15 |  16 |  17 |  18 |
    // |  19 |  1A |  1B |  1C |
    // |  1D |  1E |  1F |  20 |
    // |  1D |  1E |  1F |  20 |
    // |  1D |  1E |  1F |  20 |
    // |  1D |  1E |  1F |  20 |
    // |  1D |  1E |  1F |  20 |
    // |  1D |  1E |  1F |  20 |
    // |  1D |  1E |  1F |  20 |
    // |  1D |  1E |  1F |  20 |
    // |  1D |  1E |  1F |  20 |
    // 2x8 texture
    // |  31 |  32 |
    // |  33 |  34 |
    // |  35 |  36 |
    // |  37 |  38 |
    // |  3A |  39 |
    // |  3A |  39 |
    // |  3A |  39 |
    // |  3A |  39 |
    // 1x4 texture
    // |  41 |
    // |  42 |
    // |  43 |
    // |  44 |
    // 1x2 texture
    // |  51 |
    // |  52 |
    // 1x1 texture
    // |  61 |
    top->textureSizeWidth = 0x2;
    top->textureSizeHeight = 0x4;
    top->textureLod = 0;

    top->s_axis_tvalid = 1;
    top->s_axis_tlast = 0;
    top->s_axis_tdata = 0x0002'0001;
    clk(top);
    top->s_axis_tdata = 0x0004'0003;
    clk(top);
    top->s_axis_tdata = 0x0006'0005;
    clk(top);
    top->s_axis_tdata = 0x0008'0007;
    clk(top);
    top->s_axis_tdata = 0x000A'0009;
    clk(top);
    top->s_axis_tdata = 0x000C'000B;
    clk(top);
    top->s_axis_tdata = 0x000E'000D;
    clk(top);
    top->s_axis_tdata = 0x0010'000F;
    clk(top);
    top->s_axis_tdata = 0x0012'0011;
    clk(top);
    top->s_axis_tdata = 0x0014'0013;
    clk(top);
    top->s_axis_tdata = 0x0016'0015;
    clk(top);
    top->s_axis_tdata = 0x0018'0017;
    clk(top);
    top->s_axis_tdata = 0x001A'0019;
    clk(top);
    top->s_axis_tdata = 0x001C'001B;
    clk(top);
    top->s_axis_tdata = 0x001E'001D;
    clk(top);
    top->s_axis_tdata = 0x0020'001F;
    clk(top);
    top->s_axis_tdata = 0x0020'001F;
    clk(top);
    top->s_axis_tdata = 0x0020'001F;
    clk(top);
    top->s_axis_tdata = 0x0020'001F;
    clk(top);
    top->s_axis_tdata = 0x0020'001F;
    clk(top);
    top->s_axis_tdata = 0x0020'001F;
    clk(top);
    top->s_axis_tdata = 0x0020'001F;
    clk(top);
    top->s_axis_tdata = 0x0020'001F;
    clk(top);
    top->s_axis_tdata = 0x0020'001F;
    clk(top);
    top->s_axis_tdata = 0x0020'001F;
    clk(top);
    top->s_axis_tdata = 0x0020'001F;
    clk(top);
    top->s_axis_tdata = 0x0020'001F;
    clk(top);
    top->s_axis_tdata = 0x0020'001F;
    clk(top);
    top->s_axis_tdata = 0x0020'001F;
    clk(top);
    top->s_axis_tdata = 0x0020'001F;
    clk(top);
    top->s_axis_tdata = 0x0020'001F;
    clk(top);
    top->s_axis_tdata = 0x0020'001F;
    clk(top);

    top->s_axis_tdata = 0x0032'0031;
    clk(top);
    top->s_axis_tdata = 0x0034'0033;
    clk(top);
    top->s_axis_tdata = 0x0036'0035;
    clk(top);
    top->s_axis_tdata = 0x0038'0037;
    clk(top);
    top->s_axis_tdata = 0x003A'0039;
    clk(top);
    top->s_axis_tdata = 0x003A'0039;
    clk(top);
    top->s_axis_tdata = 0x003A'0039;
    clk(top);
    top->s_axis_tdata = 0x003A'0039;
    clk(top);

    top->s_axis_tdata = 0x0042'0041;
    clk(top);
    top->s_axis_tdata = 0x0044'0043;
    clk(top);

    top->s_axis_tdata = 0x0052'0051;
    clk(top);

    top->s_axis_tvalid = 1;
    top->s_axis_tlast = 1;
    top->s_axis_tdata = 0x0000'0061;
    clk(top);

    top->s_axis_tvalid = 0;
    top->s_axis_tlast = 0;

    top->clampS = 0;
    top->clampT = 0;
}

TEST_CASE("Get various values from the mipmap 4x16 texture", "[TextureBuffer]")
{
    VTextureSamplerTestModule* top = new VTextureSamplerTestModule();
    reset(top);

    uploadMipMap4x16Texture(top);

    // (0, 0)
    top->textureLod = 0;
    top->texelS = 0;
    top->texelT = 0;
    clk(top);
    clk(top);
    clk(top);
    clk(top);
    REQUIRE(top->texel00 == 0x11);
    REQUIRE(top->texel01 == 0x22);
    REQUIRE(top->texel10 == 0x55);
    REQUIRE(top->texel11 == 0x66);

    // (0, 0)
    top->textureLod = 1;
    top->texelS = 0;
    top->texelT = 0;
    clk(top);
    clk(top);
    clk(top);
    clk(top);
    REQUIRE(top->texel00 == 0x3311);
    REQUIRE(top->texel01 == 0x3322);
    REQUIRE(top->texel10 == 0x3333);
    REQUIRE(top->texel11 == 0x3344);

    // (0, 0)
    top->textureLod = 2;
    top->texelS = 0;
    top->texelT = 0;
    clk(top);
    clk(top);
    clk(top);
    clk(top);
    REQUIRE(top->texel00 == 0x4411);
    REQUIRE(top->texel01 == 0x4411);
    REQUIRE(top->texel10 == 0x4422);
    REQUIRE(top->texel11 == 0x4422);

    // (0, 0)
    top->textureLod = 3;
    top->texelS = 0;
    top->texelT = 0;
    clk(top);
    clk(top);
    clk(top);
    clk(top);
    REQUIRE(top->texel00 == 0x5511);
    REQUIRE(top->texel01 == 0x5511);
    REQUIRE(top->texel10 == 0x5522);
    REQUIRE(top->texel11 == 0x5522);

    // (0, 0)
    top->textureLod = 4;
    top->texelS = 0;
    top->texelT = 0;
    clk(top);
    clk(top);
    clk(top);
    clk(top);
    REQUIRE(top->texel00 == 0x6611);
    REQUIRE(top->texel01 == 0x6611);
    REQUIRE(top->texel10 == 0x6611);
    REQUIRE(top->texel11 == 0x6611);

    // Destroy model
    delete top;
}
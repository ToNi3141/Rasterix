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

enum Combine
{
    REPLACE,
    MODULATE,
    ADD,
    ADD_SIGNED,
    INTERPOLATE,
    SUBTRACT,
    DOT3_RGB,
    DOT3_RGBA
};

enum Operand
{
    SRC_ALPHA,
    ONE_MINUS_SRC_ALPHA,
    SRC_COLOR,
    ONE_MINUS_SRC_COLOR
};

enum SrcReg
{
    TEXTURE,
    CONSTANT,
    PRIMARY_COLOR,
    PREVIOUS
};

struct __attribute__ ((__packed__)) ConfReg2
{
    Combine combineRgb : 3;
    Combine combineAlpha : 3;
    SrcReg srcRegRgb0 : 2;
    SrcReg srcRegRgb1 : 2;
    SrcReg srcRegRgb2 : 2;
    SrcReg srcRegAlpha0 : 2;
    SrcReg srcRegAlpha1 : 2;
    SrcReg srcRegAlpha2 : 2;
    Operand operandRgb0 : 2;
    Operand operandRgb1 : 2;
    Operand operandRgb2 : 2;
    Operand operandAlpha0 : 1;
    Operand operandAlpha1 : 1;
    Operand operandAlpha2 : 1;
    uint8_t shiftRgb : 2;
    uint8_t shiftAlpha : 2;
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

union Conf {
    ConfReg2 conf;
    uint32_t value;
} conf;

void initConf(Conf& conf)
{
    conf.conf.combineRgb = REPLACE;
    conf.conf.combineAlpha = REPLACE;
    conf.conf.srcRegRgb0 = TEXTURE;
    conf.conf.srcRegRgb1 = TEXTURE;
    conf.conf.srcRegRgb2 = TEXTURE;
    conf.conf.srcRegAlpha0 = TEXTURE;
    conf.conf.srcRegAlpha1 = TEXTURE;
    conf.conf.srcRegAlpha2 = TEXTURE;
    conf.conf.operandRgb0 = SRC_COLOR;
    conf.conf.operandRgb1 = SRC_COLOR;
    conf.conf.operandRgb2 = SRC_COLOR;
    conf.conf.operandAlpha0 = SRC_ALPHA;
    conf.conf.operandAlpha1 = SRC_ALPHA;
    conf.conf.operandAlpha2 = SRC_ALPHA;
    conf.conf.shiftAlpha = 0;
    conf.conf.shiftRgb = 0;
}



TEST_CASE("Check TexEnv SRC_RGB TEXTURE", "[TexEnv]")
{
    VTexEnv* top = new VTexEnv();
    reset(top);

    Conf conf;
    initConf(conf);
    conf.conf.combineRgb = REPLACE;
    conf.conf.srcRegRgb0 = TEXTURE;
    top->conf = conf.value;

    top->previousColor = 0xff000001;
    top->texSrcColor = 0x00ff0002;
    top->primaryColor = 0x0000ff03;
    top->envColor = 0x000000ff;

    clk(top);
    clk(top);
    clk(top);
    clk(top);
    REQUIRE(top->color == 0x00ff0002);

    // Destroy model
    delete top;
}

TEST_CASE("Check TexEnv SRC_RGB PREVIOUS", "[TexEnv]")
{
    VTexEnv* top = new VTexEnv();
    reset(top);

    Conf conf;
    initConf(conf);
    conf.conf.combineRgb = REPLACE;
    conf.conf.srcRegRgb0 = PREVIOUS;
    top->conf = conf.value;

    top->previousColor = 0xff000001;
    top->texSrcColor = 0x00ff0002;
    top->primaryColor = 0x0000ff03;
    top->envColor = 0x000000ff;

    clk(top);
    clk(top);
    clk(top);
    clk(top);
    REQUIRE(top->color ==  0xff000002);

    // Destroy model
    delete top;
}

TEST_CASE("Check TexEnv SRC_RGB CONSTANT", "[TexEnv]")
{
    VTexEnv* top = new VTexEnv();
    reset(top);

    Conf conf;
    initConf(conf);
    conf.conf.combineRgb = REPLACE;
    conf.conf.srcRegRgb0 = CONSTANT;
    top->conf = conf.value;

    top->previousColor = 0xff000001;
    top->texSrcColor = 0x00ff0002;
    top->primaryColor = 0x0000ff03;
    top->envColor = 0x000000ff;

    clk(top);
    clk(top);
    clk(top);
    clk(top);
    REQUIRE(top->color ==  0x00000002);

    // Destroy model
    delete top;
}

TEST_CASE("Check TexEnv SRC_RGB PRIMARY_COLOR", "[TexEnv]")
{
    VTexEnv* top = new VTexEnv();
    reset(top);

    Conf conf;
    initConf(conf);
    conf.conf.combineRgb = REPLACE;
    conf.conf.srcRegRgb0 = PRIMARY_COLOR;
    top->conf = conf.value;

    top->previousColor = 0xff000001;
    top->texSrcColor = 0x00ff0002;
    top->primaryColor = 0x0000ff03;
    top->envColor = 0x000000ff;

    clk(top);
    clk(top);
    clk(top);
    clk(top);
    REQUIRE(top->color ==  0x0000ff02);

    // Destroy model
    delete top;
}

TEST_CASE("Check TexEnv SRC_ALPHA TEXTURE", "[TexEnv]")
{
    VTexEnv* top = new VTexEnv();
    reset(top);

    Conf conf;
    initConf(conf);
    conf.conf.combineAlpha = REPLACE;
    conf.conf.srcRegAlpha0 = TEXTURE;
    top->conf = conf.value;

    top->previousColor = 0xff000001;
    top->texSrcColor = 0x00ff0002;
    top->primaryColor = 0x0000ff03;
    top->envColor = 0x000000ff;

    clk(top);
    clk(top);
    clk(top);
    clk(top);
    REQUIRE(top->color == 0x00ff0002);

    // Destroy model
    delete top;
}

TEST_CASE("Check TexEnv SRC_ALPHA PREVIOUS", "[TexEnv]")
{
    VTexEnv* top = new VTexEnv();
    reset(top);

    Conf conf;
    initConf(conf);
    conf.conf.combineAlpha = REPLACE;
    conf.conf.srcRegAlpha0 = PREVIOUS;
    top->conf = conf.value;

    top->previousColor = 0xff000001;
    top->texSrcColor = 0x00ff0002;
    top->primaryColor = 0x0000ff03;
    top->envColor = 0x000000ff;

    clk(top);
    clk(top);
    clk(top);
    clk(top);
    REQUIRE(top->color ==  0x00ff0001);

    // Destroy model
    delete top;
}

TEST_CASE("Check TexEnv SRC_ALPHA CONSTANT", "[TexEnv]")
{
    VTexEnv* top = new VTexEnv();
    reset(top);

    Conf conf;
    initConf(conf);
    conf.conf.combineAlpha = REPLACE;
    conf.conf.srcRegAlpha0 = CONSTANT;
    top->conf = conf.value;

    top->previousColor = 0xff000001;
    top->texSrcColor = 0x00ff0002;
    top->primaryColor = 0x0000ff03;
    top->envColor = 0x000000ff;

    clk(top);
    clk(top);
    clk(top);
    clk(top);
    REQUIRE(top->color ==  0x00ff00ff);

    // Destroy model
    delete top;
}

TEST_CASE("Check TexEnv SRC_ALPHA PRIMARY_COLOR", "[TexEnv]")
{
    VTexEnv* top = new VTexEnv();
    reset(top);

    Conf conf;
    initConf(conf);
    conf.conf.combineAlpha = REPLACE;
    conf.conf.srcRegAlpha0 = PRIMARY_COLOR;
    top->conf = conf.value;

    top->previousColor = 0xff000001;
    top->texSrcColor = 0x00ff0002;
    top->primaryColor = 0x0000ff03;
    top->envColor = 0x000000ff;

    clk(top);
    clk(top);
    clk(top);
    clk(top);
    REQUIRE(top->color ==  0x00ff0003);

    // Destroy model
    delete top;
}

TEST_CASE("Check TexEnv OP ONE_MINUS_SRC_COLOR (RGB)", "[TexEnv]")
{
    VTexEnv* top = new VTexEnv();
    reset(top);

    Conf conf;
    initConf(conf);
    conf.conf.operandRgb0 = ONE_MINUS_SRC_COLOR;
    top->conf = conf.value;

    top->texSrcColor = 0x00ff0002;

    clk(top);
    clk(top);
    clk(top);
    clk(top);
    REQUIRE(top->color == 0xff00ff02);

    // Destroy model
    delete top;
}

TEST_CASE("Check TexEnv OP SRC_ALPHA (RGB)", "[TexEnv]")
{
    VTexEnv* top = new VTexEnv();
    reset(top);

    Conf conf;
    initConf(conf);
    conf.conf.operandRgb0 = SRC_ALPHA;
    top->conf = conf.value;

    top->texSrcColor = 0x00ff0002;

    clk(top);
    clk(top);
    clk(top);
    clk(top);
    REQUIRE(top->color == 0x02020202);

    // Destroy model
    delete top;
}

TEST_CASE("Check TexEnv OP ONE_MINUS_SRC_ALPHA (RGB)", "[TexEnv]")
{
    VTexEnv* top = new VTexEnv();
    reset(top);

    Conf conf;
    initConf(conf);
    conf.conf.operandRgb0 = ONE_MINUS_SRC_ALPHA;
    top->conf = conf.value;

    top->texSrcColor = 0x00ff0002;

    clk(top);
    clk(top);
    clk(top);
    clk(top);
    REQUIRE(top->color == 0xfdfdfd02);

    // Destroy model
    delete top;
}

TEST_CASE("Check TexEnv OP ONE_MINUS_SRC_ALPHA (ALPHA)", "[TexEnv]")
{
    VTexEnv* top = new VTexEnv();
    reset(top);

    Conf conf;
    initConf(conf);
    conf.conf.operandAlpha0 = ONE_MINUS_SRC_ALPHA;
    top->conf = conf.value;

    top->texSrcColor = 0x00ff0002;

    clk(top);
    clk(top);
    clk(top);
    clk(top);
    REQUIRE(top->color == 0x00ff00fd);

    // Destroy model
    delete top;
}

TEST_CASE("Check TexEnv REPLACE (RGB)", "[TexEnv]")
{
    VTexEnv* top = new VTexEnv();
    reset(top);

    Conf conf;
    initConf(conf);
    conf.conf.combineRgb = REPLACE;
    top->conf = conf.value;

    top->texSrcColor = 0x00ff0002;

    clk(top);
    clk(top);
    clk(top);
    clk(top);
    REQUIRE(top->color == 0x00ff0002);

    // Destroy model
    delete top;
}

TEST_CASE("Check TexEnv MODULATE (RGB)", "[TexEnv]")
{
    VTexEnv* top = new VTexEnv();
    reset(top);

    Conf conf;
    initConf(conf);
    conf.conf.combineRgb = MODULATE;
    conf.conf.srcRegRgb0 = TEXTURE;
    conf.conf.srcRegRgb1 = PREVIOUS;
    top->conf = conf.value;

    top->previousColor = 0x10203001;
    top->texSrcColor = 0x40506002;

    clk(top);
    clk(top);
    clk(top);
    clk(top);
    REQUIRE(top->color == 0x040A1202);

    // Destroy model
    delete top;
}

TEST_CASE("Check TexEnv ADD (RGB)", "[TexEnv]")
{
    VTexEnv* top = new VTexEnv();
    reset(top);

    Conf conf;
    initConf(conf);
    conf.conf.combineRgb = ADD;
    conf.conf.srcRegRgb0 = TEXTURE;
    conf.conf.srcRegRgb1 = PREVIOUS;
    top->conf = conf.value;

    top->previousColor = 0x10203001;
    top->texSrcColor = 0x40506002;

    clk(top);
    clk(top);
    clk(top);
    clk(top);
    REQUIRE(top->color == 0x50709002);

    // Destroy model
    delete top;
}

TEST_CASE("Check TexEnv ADD_SIGNED (RGB)", "[TexEnv]")
{
    VTexEnv* top = new VTexEnv();
    reset(top);

    Conf conf;
    initConf(conf);
    conf.conf.combineRgb = ADD_SIGNED;
    conf.conf.srcRegRgb0 = TEXTURE;
    conf.conf.srcRegRgb1 = PREVIOUS;
    top->conf = conf.value;

    top->previousColor = 0x10203001;
    top->texSrcColor = 0x6080A002;

    clk(top);
    clk(top);
    clk(top);
    clk(top);
    REQUIRE(top->color == 0x00215102);

    // Destroy model
    delete top;
}

TEST_CASE("Check TexEnv INTERPOLATE (RGB)", "[TexEnv]")
{
    VTexEnv* top = new VTexEnv();
    reset(top);

    Conf conf;
    initConf(conf);
    conf.conf.combineRgb = INTERPOLATE;
    conf.conf.srcRegRgb0 = TEXTURE;
    conf.conf.srcRegRgb1 = PREVIOUS;
    conf.conf.srcRegRgb2 = CONSTANT;
    top->conf = conf.value;

    top->texSrcColor = 0x10203001;
    top->previousColor = 0x6080A002;
    top->envColor = 0x40506003;

    clk(top);
    clk(top);
    clk(top);
    clk(top);
    REQUIRE(top->color == 0x4c627601);
    
    // arg0 * arg1 = 0x040A12
    // 1 - arg2 = 0xbfaf9f
    // arg1 * (1 - arg2) = 0x475763
    // 0x040A12 + 0x475763 = 0x4b6175 (+- 1 tolerance)

    // Destroy model
    delete top;
}

TEST_CASE("Check TexEnv SUBTRACT (RGB)", "[TexEnv]")
{
    VTexEnv* top = new VTexEnv();
    reset(top);

    Conf conf;
    initConf(conf);
    conf.conf.combineRgb = SUBTRACT;
    conf.conf.srcRegRgb0 = TEXTURE;
    conf.conf.srcRegRgb1 = PREVIOUS;
    top->conf = conf.value;

    top->texSrcColor = 0x5080A001;
    top->previousColor = 0x10203002;

    clk(top);
    clk(top);
    clk(top);
    clk(top);
    REQUIRE(top->color == 0x40607001);

    // Destroy model
    delete top;
}

TEST_CASE("Check DOT3_RGB", "[TexEnv]")
{
    VTexEnv* top = new VTexEnv();
    reset(top);

    top->previousColor = 0x11000001;
    top->texSrcColor = 0x00669902; // 0x66 = 0.4
    top->primaryColor = 0x00000003; // 0x99 = 0.6
    top->envColor = 0x000000ff;

    // r = (0 - 0.5) * (0 - 0.5) = 0.25
    // g = (0.4 - 0.5) * (0 - 0.5) = 0.05
    // b = (0.6 - 0.5) * (0 - 0.5) = -0.05
    // rgb = 0.25 + 0.05 + -0.05 = 0.25 (0x40)
    // In reality it is 0x41 because of rounding issues

    Conf conf;
    initConf(conf);
    conf.conf.combineRgb = DOT3_RGB;
    conf.conf.srcRegRgb0 = TEXTURE;
    conf.conf.srcRegRgb1 = PRIMARY_COLOR;
    conf.conf.srcRegRgb2 = PREVIOUS;
    top->conf = conf.value;

    clk(top);
    clk(top);
    clk(top);
    clk(top);
    REQUIRE(top->color == 0x41414102);

    // Destroy model
    delete top;
}

TEST_CASE("Check DOT3_RGBA", "[TexEnv]")
{
    VTexEnv* top = new VTexEnv();
    reset(top);

    top->previousColor = 0x11000001;
    top->texSrcColor = 0x00669902; // 0x66 = 0.4
    top->primaryColor = 0x00000003; // 0x99 = 0.6
    top->envColor = 0x000000ff;

    // r = (0 - 0.5) * (0 - 0.5) = 0.25
    // g = (0.4 - 0.5) * (0 - 0.5) = 0.05
    // b = (0.6 - 0.5) * (0 - 0.5) = -0.05
    // rgb = 0.25 + 0.05 + -0.05 = 0.25 (0x40)
    // In reality it is 0x41 because of rounding issues

    Conf conf;
    initConf(conf);
    conf.conf.combineRgb = DOT3_RGBA;
    conf.conf.srcRegRgb0 = TEXTURE;
    conf.conf.srcRegRgb1 = PRIMARY_COLOR;
    conf.conf.srcRegRgb2 = PREVIOUS;
    top->conf = conf.value;

    clk(top);
    clk(top);
    clk(top);
    clk(top);
    REQUIRE(top->color == 0x41414141);

    // Destroy model
    delete top;
}

TEST_CASE("Check TexEnv MODULATE (ALPHA)", "[TexEnv]")
{
    VTexEnv* top = new VTexEnv();
    reset(top);

    Conf conf;
    initConf(conf);
    conf.conf.combineAlpha = MODULATE;
    conf.conf.srcRegAlpha0 = TEXTURE;
    conf.conf.srcRegAlpha1 = PREVIOUS;
    top->conf = conf.value;

    top->previousColor = 0x00000030;
    top->texSrcColor = 0x00000060;

    clk(top);
    clk(top);
    clk(top);
    clk(top);
    REQUIRE(top->color == 0x00000012);

    // Destroy model
    delete top;
}

TEST_CASE("Check TexEnv ADD (ALPHA)", "[TexEnv]")
{
    VTexEnv* top = new VTexEnv();
    reset(top);

    Conf conf;
    initConf(conf);
    conf.conf.combineAlpha = ADD;
    conf.conf.srcRegAlpha0 = TEXTURE;
    conf.conf.srcRegAlpha1 = PREVIOUS;
    top->conf = conf.value;

    top->previousColor = 0x00000030;
    top->texSrcColor = 0x00000060;

    clk(top);
    clk(top);
    clk(top);
    clk(top);
    REQUIRE(top->color == 0x00000090);

    // Destroy model
    delete top;
}

TEST_CASE("Check TexEnv ADD_SIGNED (ALPHA)", "[TexEnv]")
{
    VTexEnv* top = new VTexEnv();
    reset(top);

    Conf conf;
    initConf(conf);
    conf.conf.combineAlpha = ADD_SIGNED;
    conf.conf.srcRegAlpha0 = TEXTURE;
    conf.conf.srcRegAlpha1 = PREVIOUS;
    top->conf = conf.value;

    top->previousColor = 0x00000030;
    top->texSrcColor = 0x00000060;

    clk(top);
    clk(top);
    clk(top);
    clk(top);
    REQUIRE(top->color == 0x00000011);

    // Destroy model
    delete top;
}

TEST_CASE("Check TexEnv INTERPOLATE (ALPHA)", "[TexEnv]")
{
    VTexEnv* top = new VTexEnv();
    reset(top);

    Conf conf;
    initConf(conf);
    conf.conf.combineAlpha = INTERPOLATE;
    conf.conf.srcRegAlpha0 = TEXTURE;
    conf.conf.srcRegAlpha1 = PREVIOUS;
    conf.conf.srcRegAlpha2 = CONSTANT;
    top->conf = conf.value;

    top->texSrcColor = 0x00000030;
    top->previousColor = 0x00000060;
    top->envColor = 0x00000090;

    clk(top);
    clk(top);
    clk(top);
    clk(top);
    REQUIRE(top->color == 0x00000045);
    // arg0 * arg2 = 0x30 * 0x60 = 0x1B
    // 1 - arg2 = 0x6F
    // arg1 * (1 - arg2) = 0x29
    // 0x1B + 0x29 = 0x44

    // Destroy model
    delete top;
}

TEST_CASE("Check TexEnv SUBTRACT (ALPHA)", "[TexEnv]")
{
    VTexEnv* top = new VTexEnv();
    reset(top);

    Conf conf;
    initConf(conf);
    conf.conf.combineAlpha = SUBTRACT;
    conf.conf.srcRegAlpha0 = TEXTURE;
    conf.conf.srcRegAlpha1 = PREVIOUS;
    top->conf = conf.value;

    top->texSrcColor = 0x00000090;
    top->previousColor = 0x00000060;

    clk(top);
    clk(top);
    clk(top);
    clk(top);
    REQUIRE(top->color == 0x00000030);

    // Destroy model
    delete top;
}

TEST_CASE("Check TexEnv REPLACE Pipeline", "[TexEnv]")
{
    VTexEnv* top = new VTexEnv();
    reset(top);

    Conf conf;
    initConf(conf);
    conf.conf.combineRgb = REPLACE;
    top->conf = conf.value;

    top->texSrcColor = 0xff000001;

    clk(top);
    top->texSrcColor = 0x00ff0002;
    clk(top);
    top->texSrcColor = 0x0000ff03;
    clk(top);
    top->texSrcColor = 0xffff0004;
    clk(top);
    REQUIRE(top->color == 0xff000001);
    clk(top);
    REQUIRE(top->color == 0x00ff0002);
    clk(top);
    REQUIRE(top->color == 0x0000ff03);
    clk(top);
    REQUIRE(top->color == 0xffff0004);

    // Destroy model
    delete top;
}

TEST_CASE("Check TexEnv REPLACE SCALE (RGB)", "[TexEnv]")
{
    VTexEnv* top = new VTexEnv();
    reset(top);

    Conf conf;
    initConf(conf);
    conf.conf.combineRgb = REPLACE;

    top->texSrcColor = 0x01084002;

    // Scale of 1
    conf.conf.shiftRgb = 1;
    top->conf = conf.value;
    clk(top);
    clk(top);
    clk(top);
    clk(top);
    REQUIRE(top->color == 0x02108002);

    // Scale of 2
    conf.conf.shiftRgb = 2;
    top->conf = conf.value;
    clk(top);
    clk(top);
    clk(top);
    clk(top);
    REQUIRE(top->color == 0x0420ff02);

    // Destroy model
    delete top;
}

TEST_CASE("Check TexEnv REPLACE SCALE (ALPHA)", "[TexEnv]")
{
    VTexEnv* top = new VTexEnv();
    reset(top);

    Conf conf;
    initConf(conf);
    conf.conf.combineRgb = REPLACE;

    top->texSrcColor = 0x01084040;

    // Scale of 1
    conf.conf.shiftAlpha = 1;
    top->conf = conf.value;
    clk(top);
    clk(top);
    clk(top);
    clk(top);
    REQUIRE(top->color == 0x01084080);

    // Scale of 2
    conf.conf.shiftAlpha = 2;
    top->conf = conf.value;
    clk(top);
    clk(top);
    clk(top);
    clk(top);
    REQUIRE(top->color == 0x010840ff);

    // Destroy model
    delete top;
}

TEST_CASE("Check DOT3_RGB SCALE", "[TexEnv]")
{
    VTexEnv* top = new VTexEnv();
    reset(top);

    top->previousColor = 0x11000001;
    top->texSrcColor = 0x00669902; // 0x66 = 0.4
    top->primaryColor = 0x00000003; // 0x99 = 0.6
    top->envColor = 0x000000ff;

    // r = (0 - 0.5) * (0 - 0.5) = 0.25
    // g = (0.4 - 0.5) * (0 - 0.5) = 0.05
    // b = (0.6 - 0.5) * (0 - 0.5) = -0.05
    // rgb = 0.25 + 0.05 + -0.05 = 0.25 (0x40)
    // In reality it is 0x41 because of rounding issues

    Conf conf;
    initConf(conf);
    conf.conf.combineRgb = DOT3_RGB;
    conf.conf.srcRegRgb0 = TEXTURE;
    conf.conf.srcRegRgb1 = PRIMARY_COLOR;
    conf.conf.srcRegRgb2 = PREVIOUS;
    

    // Scale of 1
    conf.conf.shiftRgb = 1;
    top->conf = conf.value;
    clk(top);
    clk(top);
    clk(top);
    clk(top);
    REQUIRE(top->color == 0x82828202);

    // Scale of 2
    conf.conf.shiftRgb = 2;
    top->conf = conf.value;
    clk(top);
    clk(top);
    clk(top);
    clk(top);
    REQUIRE(top->color == 0xffffff02);

    // Destroy model
    delete top;
}
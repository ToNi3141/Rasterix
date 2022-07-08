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

TEST_CASE("Check TexEnv", "[TexEnv]")
{
    VTexEnv* top = new VTexEnv();
    reset(top);

    union Conf {
        ConfReg2 conf;
        uint32_t value;
    } conf;

    top->previousColor = 0xff000001;
    top->texSrcColor = 0x00ff0002;
    top->primaryColor = 0x0000ff03;
    top->envColor = 0x000000ff;

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

    top->conf = conf.value;
    

    clk(top);
    clk(top);
    clk(top);
    clk(top);
    REQUIRE(top->color == 0x00ff0002);

    // Destroy model
    delete top;
}

TEST_CASE("Check DOT3_RGB", "[TexEnv]")
{
    VTexEnv* top = new VTexEnv();
    reset(top);

    union Conf {
        ConfReg2 conf;
        uint32_t value;
    } conf;

    top->previousColor = 0x11000001;
    top->texSrcColor = 0x00669902; // 0x66 = 0.4
    top->primaryColor = 0x00000003; // 0x99 = 0.6
    top->envColor = 0x000000ff;

    // r = (0 - 0.5) * (0 - 0.5) = 0.25
    // g = (0.4 - 0.5) * (0 - 0.5) = 0.05
    // b = (0.6 - 0.5) * (0 - 0.5) = -0.05
    // rgb = 0.25 + 0.05 + -0.05 = 0.25 (0x40)
    // In reality it is 0x41 because of rounding issues

    conf.conf.combineRgb = DOT3_RGB;
    conf.conf.combineAlpha = REPLACE;
    conf.conf.srcRegRgb0 = TEXTURE;
    conf.conf.srcRegRgb1 = PRIMARY_COLOR;
    conf.conf.srcRegRgb2 = PREVIOUS;
    conf.conf.srcRegAlpha0 = TEXTURE;
    conf.conf.srcRegAlpha1 = TEXTURE;
    conf.conf.srcRegAlpha2 = TEXTURE;
    conf.conf.operandRgb0 = SRC_COLOR;
    conf.conf.operandRgb1 = SRC_COLOR;
    conf.conf.operandRgb2 = SRC_COLOR;
    conf.conf.operandAlpha0 = SRC_ALPHA;
    conf.conf.operandAlpha1 = SRC_ALPHA;
    conf.conf.operandAlpha2 = SRC_ALPHA;

    top->conf = conf.value;
    

    clk(top);
    clk(top);
    clk(top);
    clk(top);
    REQUIRE(top->color == 0x41414102);

    // Destroy model
    delete top;
}
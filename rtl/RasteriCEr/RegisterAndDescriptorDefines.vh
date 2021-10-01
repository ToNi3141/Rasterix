// RasteriCEr
// https://github.com/ToNi3141/RasteriCEr
// Copyright (c) 2021 ToNi3141

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





// Every Operation and Argument has to be aligned by CommandParser::CMD_STREAM_WIDTH.
// For instance, when CMD_STREAM_WIDTH is configured with 16, then every Operaton 
// and Argument has to be aligned to a by two bytes.
// When CMD_STREAM_WIDTH is 128, then every Operation and Argument has to be aligned
// by eight bytes.
// Every beat of the AXIS contains one Operation, or data for one Argument.

////////////////////////////
// Operations
////////////////////////////
// An operation contains an OP and an immediate value. The prototype of an operation looks like the following
//  +---------------------------------------+
//  | 4 OP | 12 bit immediate value         |
//  +---------------------------------------+
localparam OP_NOP_STREAM = 0;
localparam OP_TEXTURE_STREAM = 1;
localparam OP_RENDER_CONFIG = 2;
localparam OP_FRAMEBUFFER = 3;
localparam OP_TRIANGLE_STREAM = 4;

localparam OP_POS = 12;
localparam OP_SIZE = 4;
localparam OP_IMM_POS = 0;
localparam OP_IMM_SIZE = 12;

////////////////////////////
// Operation Immediate Values
////////////////////////////
// OP_TRIANGLE_STREAM
// Immediate value contains size of triangle in bytes (inclusive the additional bytes which are required for CMD_AXIS bus alignment).
//  +--------------------------------+
//  | 4 bit OP | 12 bit size in bytes|
//  +--------------------------------+

// OP_TEXTURE_STREAM
//  +-----------------------------------------------------+
//  | 4 bit OP | 4 bit reserved | 4 bit mode | 4 bit size |
//  +-----------------------------------------------------+
// The size uses also the OP_TEXTURE_STREAM_MODE defines.
localparam TEXTURE_STREAM_SIZE_POS = 0;
localparam TEXTURE_STREAM_MODE_POS = 4;
localparam TEXTURE_STREAM_IMM_SIZE = 4;
`define OP_TEXTURE_STREAM_MODE_32x32   4'b0001 // When used for size: expects after the command 32x32x2 bytes of texture data
`define OP_TEXTURE_STREAM_MODE_64x64   4'b0010 // When used for size: expects after the command 64x64x2 bytes of texture data
`define OP_TEXTURE_STREAM_MODE_128x128 4'b0100 // When used for size: expects after the command 128x128x2 bytes of texture data
`define OP_TEXTURE_STREAM_MODE_256x256 4'b1000 // When used for size: expects after the command 256x256x2 bytes of texture data

// OP_RENDER_CONFIG
//  +-----------------------------+
//  | 4 bit OP | 12 register addr |
//  +-----------------------------+
localparam OP_RENDER_CONFIG_COLOR_BUFFER_CLEAR_COLOR = 0;
localparam OP_RENDER_CONFIG_DEPTH_BUFFER_CLEAR_DEPTH = 1;
localparam OP_RENDER_CONFIG_REG1 = 2;
localparam OP_RENDER_CONFIG_REG2 = 3;
localparam OP_RENDER_CONFIG_TEX_ENV_COLOR = 4;

// OP_FRAMEBUFFER
//  +----------------------------------------------------------------------------------------------------------------------------------+
//  | 4 bit OP | 6 bit reserved | 1 bit depth buffer select | 1 bit color buffer select | 2 bit reserved | 1 bit memset | 1 bit commit |
//  +----------------------------------------------------------------------------------------------------------------------------------+
// Command to execute on the framebuffer
localparam OP_FRAMEBUFFER_COMMIT_POS = 0; // Streams the frame buffer (color buffer) content via the framebuffer_axis 
localparam OP_FRAMEBUFFER_MEMSET_POS = 1; // Clears the frame buffer with the configured clear color / depth value
// The selected buffer where to execute the command above (multiple selections are possible)
localparam OP_FRAMEBUFFER_COLOR_BUFFER_SELECT_POS = 4;
localparam OP_FRAMEBUFFER_DEPTH_BUFFER_SELECT_POS = 5;


////////////////////////////
// Operation Arguments
////////////////////////////
// OP_TRIANGLE_STREAM 
// Triangle Descriptor, each value containts 4 bytes.
localparam TRIANGLE_CONFIGURATION = 0;
localparam BB_START = 1; // S15.0, S15.0
localparam BB_END = 2; // S15.0, S15.0
localparam INC_W0 = 3; // Sn.m
localparam INC_W1 = 4; // Sn.m
localparam INC_W2 = 5; // Sn.m
localparam INC_W0_X = 6; // Sn.m
localparam INC_W1_X = 7; // Sn.m
localparam INC_W2_X = 8; // Sn.m
localparam INC_W0_Y = 9; // Sn.m
localparam INC_W1_Y = 10; // Sn.m
localparam INC_W2_Y = 11; // Sn.m
localparam INC_TEX_S = 12; // S1.30
localparam INC_TEX_T = 13; // S1.30
localparam INC_TEX_S_X = 14; // S1.30
localparam INC_TEX_T_X = 15; // S1.30
localparam INC_TEX_S_Y = 16; // S1.30
localparam INC_TEX_T_Y = 17; // S1.30
localparam INC_DEPTH_W = 18; // S1.30
localparam INC_DEPTH_W_X = 19; // S1.30
localparam INC_DEPTH_W_Y = 20; // S1.30
localparam PADDING_1 = 21;
localparam PADDING_2 = 22;
localparam PADDING_3 = 23;
localparam TRIANGLE_DATA_SET_SIZE_32 = INC_DEPTH_W_Y + 1; // Max command port width: 32 bit
localparam TRIANGLE_DATA_SET_SIZE_64 = PADDING_1 + 1; // Max command port width: 64 bit
localparam TRIANGLE_DATA_SET_SIZE_256 = PADDING_3 + 1; // Max command port width: 256 bit
`define GET_TRIANGLE_SIZE_FOR_BUS_WIDTH(x) (x <= 32 ? TRIANGLE_DATA_SET_SIZE_32 : \
                                            x <= 64 ? TRIANGLE_DATA_SET_SIZE_64 : \
                                            x <= 256 ? TRIANGLE_DATA_SET_SIZE_256 : \
                                            0)

// BB_START and BB_END defines
localparam BB_X_POS = 0;
localparam BB_Y_POS = 16;
// TRIANGLE_CONFIGURATION defines
localparam TRIANGLE_STATIC_COLOR_POS = 16;
localparam ENABLE_W_INTERPOLATION_POS = 0;
localparam ENABLE_TEXTURE_INTERPOLATION_POS = 1;

// OP_TEXTURE_STREAM
// Texture data, size is dependet on the immediate value, for instance,
// OP_TEXTURE_STREAM_MODE_32x32 = 32px * 32px * 2 = 2048 bytes
// Each texel is build like the following:
//  +---------------------------------------+
//  | 4 bit R | 4 bit G | 4 bit B | 4 bit A |
//  +---------------------------------------+

// OP_RENDER_CONFIG
// OP_RENDER_CONFIG_COLOR_BUFFER_CLEAR_COLOR
//  +---------------------------------------+
//  | 4 bit R | 4 bit G | 4 bit B | 4 bit A |
//  +---------------------------------------+

// OP_RENDER_CONFIG_DEPTH_BUFFER_CLEAR_DEPTH
//  +---------------------------------------+
//  | 16 bit depth val                      |
//  +---------------------------------------+

// OP_RENDER_CONFIG_REG1
//  +--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
//  | 1 bit color mask R | 1 bit color mask G | 1 bit color mask B | 1 bit color mask A | 1 bit depth mask | 4 bit A ref | 3 bit A func | 3 bit depth func | 1 bit enable depth test |
//  +--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
localparam REG1_ENABLE_DEPTH_TEST_POS = 0;
localparam REG1_ENABLE_DEPTH_TEST_SIZE = 1;
localparam REG1_DEPTH_TEST_FUNC_POS = REG1_ENABLE_DEPTH_TEST_POS + REG1_ENABLE_DEPTH_TEST_SIZE;
localparam REG1_DEPTH_TEST_FUNC_SIZE = 3;
localparam REG1_ALPHA_TEST_FUNC_POS = REG1_DEPTH_TEST_FUNC_POS + REG1_DEPTH_TEST_FUNC_SIZE;
localparam REG1_ALPHA_TEST_FUNC_SIZE = 3;
localparam REG1_ALPHA_TEST_REF_VALUE_POS = REG1_ALPHA_TEST_FUNC_POS + REG1_ALPHA_TEST_FUNC_SIZE;
localparam REG1_ALPHA_TEST_REF_VALUE_SIZE = 4;
localparam REG1_DEPTH_MASK_POS = REG1_ALPHA_TEST_REF_VALUE_POS + REG1_ALPHA_TEST_REF_VALUE_SIZE;
localparam REG1_DEPTH_MASK_SIZE = 1;
localparam REG1_COLOR_MASK_A_POS = REG1_DEPTH_MASK_POS + REG1_DEPTH_MASK_SIZE;
localparam REG1_COLOR_MASK_A_SIZE = 1;
localparam REG1_COLOR_MASK_B_POS = REG1_COLOR_MASK_A_POS + REG1_COLOR_MASK_A_SIZE;
localparam REG1_COLOR_MASK_B_SIZE = 1;
localparam REG1_COLOR_MASK_G_POS = REG1_COLOR_MASK_B_POS + REG1_COLOR_MASK_B_SIZE;
localparam REG1_COLOR_MASK_G_SIZE = 1;
localparam REG1_COLOR_MASK_R_POS = REG1_COLOR_MASK_G_POS + REG1_COLOR_MASK_G_SIZE;
localparam REG1_COLOR_MASK_R_SIZE = 1;

// OP_RENDER_CONFIG_REG2
//  +------------------------------------------------------------------------------------------------------------------------------------+
//  | 2 bit reserved | 1 bit tex clamp t | 1 bit tex clamp s | 4 bit blend d | 4 bit blend s | 3 bit tex env | 1 bit en persp correction |
//  +------------------------------------------------------------------------------------------------------------------------------------+
localparam REG2_PERSPECTIVE_CORRECT_TEXTURE_POS = 0;
localparam REG2_PERSPECTIVE_CORRECT_TEXTURE_SIZE = 1;
localparam REG2_TEX_ENV_FUNC_POS = REG2_PERSPECTIVE_CORRECT_TEXTURE_POS + REG2_PERSPECTIVE_CORRECT_TEXTURE_SIZE;
localparam REG2_TEX_ENV_FUNC_SIZE = 3;
localparam REG2_BLEND_FUNC_SFACTOR_POS = REG2_TEX_ENV_FUNC_POS + REG2_TEX_ENV_FUNC_SIZE;
localparam REG2_BLEND_FUNC_SFACTOR_SIZE = 4;
localparam REG2_BLEND_FUNC_DFACTOR_POS = REG2_BLEND_FUNC_SFACTOR_POS + REG2_BLEND_FUNC_SFACTOR_SIZE;
localparam REG2_BLEND_FUNC_DFACTOR_SIZE = 4;
localparam REG2_TEX_CLAMP_S_POS = REG2_BLEND_FUNC_DFACTOR_POS + REG2_BLEND_FUNC_DFACTOR_SIZE;
localparam REG2_TEX_CLAMP_S_SIZE = 1;
localparam REG2_TEX_CLAMP_T_POS = REG2_TEX_CLAMP_S_POS + REG2_TEX_CLAMP_S_SIZE;
localparam REG2_TEX_CLAMP_T_SIZE = 1;
// localparam REG2_LOGIC_OP_POS = REG2_BLEND_FUNC_DFACTOR_POS + REG2_BLEND_FUNC_DFACTOR_SIZE;
// localparam REG2_LOGIC_OP_SIZE = 4;

// Depth and Alpha func defines
localparam ALWAYS = 0;
localparam NEVER = 1;
localparam LESS = 2;
localparam EQUAL = 3;
localparam LEQUAL = 4;
localparam GREATER = 5;
localparam NOTEQUAL = 6;
localparam GEQUAL = 7;

// Tex Env and Blend func defines
localparam DISABLE = 0;
localparam REPLACE = 1;
localparam MODULATE = 2;
localparam DECAL = 3;
localparam BLEND = 4;
localparam ADD = 5;

// D and S factor defines
localparam ZERO = 0;
localparam ONE = 1;
localparam DST_COLOR = 2;
localparam SRC_COLOR = 3;
localparam ONE_MINUS_DST_COLOR = 4;
localparam ONE_MINUS_SRC_COLOR = 5;
localparam SRC_ALPHA = 6;
localparam ONE_MINUS_SRC_ALPHA = 7;
localparam DST_ALPHA = 8;
localparam ONE_MINUS_DST_ALPHA = 9;
localparam SRC_ALPHA_SATURATE = 10;

// Logic op defines (currently not implemented)
// localparam CLEAR = 0;
// localparam SET = 1;
// localparam COPY = 2;
// localparam COPY_INVERTED = 3;
// localparam NOOP = 4;
// localparam INVERTED = 5;
// localparam AND = 6;
// localparam NAND = 7;
// localparam OR = 8;
// localparam NOR = 9;
// localparam XOR = 10;
// localparam EQUIV = 11;
// localparam AND_REVERSE = 12;
// localparam AND_INVERTED = 13;
// localparam OR_REVERSE = 14;
// localparam OR_INVERTE = 15;

// Clamping
localparam REPEAT = 0;
localparam CLAMP_TO_EDGE = 1;

// OP_RENDER_CONFIG_TEX_ENV_COLOR
//  +---------------------------------------+
//  | 4 bit R | 4 bit G | 4 bit B | 4 bit A |
//  +---------------------------------------+

// Color defines
localparam COLOR_R_POS = 12;
localparam COLOR_G_POS = 8;
localparam COLOR_B_POS = 4;
localparam COLOR_A_POS = 0;
localparam COLOR_SUB_PIXEL_WIDTH = 4;

// OP_FRAMEBUFFER
// Does not contain any arguments, just starts the framebuffer commit and clear processes

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
//  | 4 OP | 28 bit immediate value         |
//  +---------------------------------------+
localparam OP_NOP_STREAM = 0;
localparam OP_TEXTURE_STREAM = 1;
localparam OP_RENDER_CONFIG = 2;
localparam OP_FRAMEBUFFER = 3;
localparam OP_TRIANGLE_STREAM = 4;
localparam OP_FOG_LUT_STREAM = 5;

localparam OP_POS = 28;
localparam OP_SIZE = 4;
localparam OP_IMM_POS = 0;
localparam OP_IMM_SIZE = 28;

////////////////////////////
// Operation Immediate Values
////////////////////////////
// OP_TRIANGLE_STREAM
// Immediate value contains size of triangle in bytes (inclusive the additional bytes which are required for CMD_AXIS bus alignment).
//  +--------------------------------+
//  | 4 bit OP | 28 bit size in bytes|
//  +--------------------------------+

// OP_TEXTURE_STREAM
//  +------------------------------------------------------------------------------------------------------------------------+
//  | 4 bit OP | 1 bit reserved | 1 bit mag filter | 1 bit clamp t | 1 bit clamp s | 8 bit size | 8 bit height | 8 bit width |
//  +------------------------------------------------------------------------------------------------------------------------+
// Texture hight and width are in power of two minus one, means: 8'b0 = 1px, 8'b1 = 2px, 8'b100 = 8px ...
// Texture size is in power of two bytes, means 8'h0b = 2kB, 8'h11 = 128kB, ...
localparam TEXTURE_STREAM_WIDTH_POS = 0;
localparam TEXTURE_STREAM_WIDTH_SIZE = 8;
localparam TEXTURE_STREAM_HEIGHT_POS = 8;
localparam TEXTURE_STREAM_HEIGHT_SIZE = 8;
localparam TEXTURE_STREAM_SIZE_POS = 16;
localparam TEXTURE_STREAM_SIZE_SIZE = 8;
localparam TEXTURE_STREAM_CLAMP_S_POS = 24;
localparam TEXTURE_STREAM_CLAMP_S_SIZE = 1;
localparam TEXTURE_STREAM_CLAMP_T_POS = 25;
localparam TEXTURE_STREAM_CLAMP_T_SIZE = 1;
localparam TEXTURE_STREAM_MAG_FILTER_POS = 26;
localparam TEXTURE_STREAM_MAG_FILTER_SIZE = 1;


// OP_RENDER_CONFIG
//  +-----------------------------+
//  | 4 bit OP | 28 register addr |
//  +-----------------------------+
localparam OP_RENDER_CONFIG_COLOR_BUFFER_CLEAR_COLOR = 0;
localparam OP_RENDER_CONFIG_DEPTH_BUFFER_CLEAR_DEPTH = 1;
localparam OP_RENDER_CONFIG_REG1 = 2;
localparam OP_RENDER_CONFIG_REG2 = 3;
localparam OP_RENDER_CONFIG_TEX_ENV_COLOR = 4;
localparam OP_RENDER_CONFIG_FOG_COLOR = 5;

// OP_FRAMEBUFFER
//  +----------------------------------------------------------------------------------------------------------------------------------+
//  | 4 bit OP | 22 bit reserved | 1 bit depth buffer select | 1 bit color buffer select | 2 bit reserved | 1 bit memset | 1 bit commit |
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
localparam PARAM_SIZE = 32;
localparam TRIANGLE_COLOR = 0;
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
localparam INC_TEX_S = 12; // float
localparam INC_TEX_T = 13; // float
localparam INC_TEX_S_X = 14; // float
localparam INC_TEX_T_X = 15; // float
localparam INC_TEX_S_Y = 16; // float
localparam INC_TEX_T_Y = 17; // float
localparam INC_DEPTH_W = 18; // float
localparam INC_DEPTH_W_X = 19; // float
localparam INC_DEPTH_W_Y = 20; // float
localparam INC_DEPTH_Z = 21; // float
localparam INC_DEPTH_Z_X = 22; // float
localparam INC_DEPTH_Z_Y = 23; // float
localparam INC_COLOR_R = 24; // float
localparam INC_COLOR_G = 25; // float
localparam INC_COLOR_B = 26; // float
localparam INC_COLOR_A = 27; // float
localparam INC_COLOR_R_X = 28; // float
localparam INC_COLOR_G_X = 29; // float
localparam INC_COLOR_B_X = 30; // float
localparam INC_COLOR_A_X = 31; // float
localparam INC_COLOR_R_Y = 32; // float
localparam INC_COLOR_G_Y = 33; // float
localparam INC_COLOR_B_Y = 34; // float
localparam INC_COLOR_A_Y = 35; // float
localparam PADDING_1 = INC_COLOR_A_Y + 4;
localparam TRIANGLE_DATA_SET_SIZE_128 = INC_COLOR_A_Y + 1; // Max command port width: 128 bit
localparam TRIANGLE_DATA_SET_SIZE_256 = PADDING_1 + 1; // Max command port width: 256 bit
`define GET_TRIANGLE_SIZE_FOR_BUS_WIDTH(x) (x <= 128 ? TRIANGLE_DATA_SET_SIZE_128 : \
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
// Texture data, size is dependet on the immediate value, see TEXTURE_STREAM_SIZE_POS
// Each texel is build like the following:
//  +---------------------------------------+
//  | 4 bit R | 4 bit G | 4 bit B | 4 bit A |
//  +---------------------------------------+

// OP_RENDER_CONFIG
// OP_RENDER_CONFIG_COLOR_BUFFER_CLEAR_COLOR
//  +---------------------------------------+
//  | 8 bit R | 8 bit G | 8 bit B | 8 bit A |
//  +---------------------------------------+

// OP_RENDER_CONFIG_DEPTH_BUFFER_CLEAR_DEPTH
//  +---------------------------------------+
//  | 16 bit depth val                      |
//  +---------------------------------------+

// OP_RENDER_CONFIG_REG1
//  +---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
//  | 4 bit reserved | 4 bit blend d | 4 bit blend s | 1 bit color mask R | 1 bit color mask G | 1 bit color mask B | 1 bit color mask A | 1 bit depth mask | 8 bit A ref | 3 bit A func | 3 bit depth func | 1 bit enable depth test |
//  +---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
localparam REG1_ENABLE_DEPTH_TEST_POS = 0;
localparam REG1_ENABLE_DEPTH_TEST_SIZE = 1;
localparam REG1_DEPTH_TEST_FUNC_POS = REG1_ENABLE_DEPTH_TEST_POS + REG1_ENABLE_DEPTH_TEST_SIZE;
localparam REG1_DEPTH_TEST_FUNC_SIZE = 3;
localparam REG1_ALPHA_TEST_FUNC_POS = REG1_DEPTH_TEST_FUNC_POS + REG1_DEPTH_TEST_FUNC_SIZE;
localparam REG1_ALPHA_TEST_FUNC_SIZE = 3;
localparam REG1_ALPHA_TEST_REF_VALUE_POS = REG1_ALPHA_TEST_FUNC_POS + REG1_ALPHA_TEST_FUNC_SIZE;
localparam REG1_ALPHA_TEST_REF_VALUE_SIZE = 8;
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
localparam REG1_BLEND_FUNC_SFACTOR_POS = REG1_COLOR_MASK_R_POS + REG1_COLOR_MASK_R_SIZE;
localparam REG1_BLEND_FUNC_SFACTOR_SIZE = 4;
localparam REG1_BLEND_FUNC_DFACTOR_POS = REG1_BLEND_FUNC_SFACTOR_POS + REG1_BLEND_FUNC_SFACTOR_SIZE;
localparam REG1_BLEND_FUNC_DFACTOR_SIZE = 4;

// OP_RENDER_CONFIG_REG2
//  +---------------------------------+
//  | 28 bit reserved | 3 bit tex env |
//  +---------------------------------+
localparam REG2_TEX_ENV_FUNC_POS = 0;
localparam REG2_TEX_ENV_FUNC_SIZE = 3;
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
//  | 8 bit R | 8 bit G | 8 bit B | 8 bit A |
//  +---------------------------------------+

// Color defines
localparam COLOR_SUB_PIXEL_WIDTH = 8;
localparam COLOR_NUMBER_OF_SUB_PIXEL = 4;
localparam COLOR_R_POS = COLOR_SUB_PIXEL_WIDTH * 3;
localparam COLOR_G_POS = COLOR_SUB_PIXEL_WIDTH * 2;
localparam COLOR_B_POS = COLOR_SUB_PIXEL_WIDTH * 1;
localparam COLOR_A_POS = COLOR_SUB_PIXEL_WIDTH * 0;


// OP_FRAMEBUFFER
// Does not contain any arguments, just starts the framebuffer commit and clear processes

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



///////////////////////////////////////////////////////////////////////////////////////////////////////////
// Operations
///////////////////////////////////////////////////////////////////////////////////////////////////////////
// Every Operation and Argument has to be aligned by CommandParser::CMD_STREAM_WIDTH.
// For instance, when CMD_STREAM_WIDTH is configured with 32, then every Operaton 
// and Argument has to be aligned to a by four bytes.
// When CMD_STREAM_WIDTH is 128, then every Operation and Argument has to be aligned
// by eight bytes.
// Every beat of the AXIS contains one Operation, or data for one Argument.
// An operation contains an OP and an immediate value. The prototype of an operation looks like the following
//  +------------------------------+
//  | 4'hx | 28'hx immediate value |
//  +------------------------------+
localparam OP_POS = 28;
localparam OP_SIZE = 4;
localparam OP_IMM_POS = 0;
localparam OP_IMM_SIZE = 28;

//---------------------------------------------------------------------------------------------------------
// No operation
//  +-----------------------+
//  | 4'h0 | 28'hx reserved |
//  +-----------------------+
// Steam size 0 32bit value.
localparam OP_NOP_STREAM = 0;

//---------------------------------------------------------------------------------------------------------
// Texture Stream
//  +-------------------------------------------------------------------------------------------------+
//  | 4'h1 | 12'hx reserved | 8'hx TMU nr (currently unused) | 8'hx texture stream size in power of 2 |
//  +-------------------------------------------------------------------------------------------------+
// Texture size is in power of two bytes, means 8'h0b = 2kB, 8'h11 = 128kB. The stream size is not dependent 
// on the actual texture size. This allows partial texture updates.
// Steam size n 32bit values.
localparam OP_TEXTURE_STREAM_TMU0 = 1;
localparam TEXTURE_STREAM_SIZE_POS = 0;
localparam TEXTURE_STREAM_SIZE_SIZE = 8;
localparam TEXTURE_STREAM_TMU_NR_POS = 8;
localparam TEXTURE_STREAM_TMU_NR_SIZE = 8;

//---------------------------------------------------------------------------------------------------------
// Register writing
//  +----------------------------+
//  | 4'h2 | 28'hx register addr |
//  +----------------------------+
// Steam size 1 32bit value.
localparam OP_RENDER_CONFIG = 2;
localparam OP_RENDER_CONFIG_COLOR_BUFFER_CLEAR_COLOR = 0;
localparam OP_RENDER_CONFIG_DEPTH_BUFFER_CLEAR_DEPTH = OP_RENDER_CONFIG_COLOR_BUFFER_CLEAR_COLOR + 1;
localparam OP_RENDER_CONFIG_FRAGMENT_FOG_COLOR = OP_RENDER_CONFIG_DEPTH_BUFFER_CLEAR_DEPTH + 1;
localparam OP_RENDER_CONFIG_FRAGMENT_PIPELINE = OP_RENDER_CONFIG_FRAGMENT_FOG_COLOR + 1;
localparam OP_RENDER_CONFIG_TMU0_TEX_ENV = OP_RENDER_CONFIG_FRAGMENT_PIPELINE + 1;
localparam OP_RENDER_CONFIG_TMU0_TEXTURE_CONFIG = OP_RENDER_CONFIG_TMU0_TEX_ENV + 1;
localparam OP_RENDER_CONFIG_TMU0_TEX_ENV_COLOR = OP_RENDER_CONFIG_TMU0_TEXTURE_CONFIG + 1;
localparam OP_RENDER_CONFIG_NUMBER_OR_REGS = OP_RENDER_CONFIG_TMU0_TEX_ENV_COLOR;

//---------------------------------------------------------------------------------------------------------
// Framebuffer configuration
//  +-----------------------------------------------------------------------------------------------------------------------------+
//  | 4'h3 | 22'hx bit reserved | 1'hx depth buffer select | 1'hx color buffer select | 2'hx reserved | 1'hx memset | 1'hx commit |
//  +-----------------------------------------------------------------------------------------------------------------------------+
// Command to execute on the framebuffer
// Steam size 1 32bit value.
localparam OP_FRAMEBUFFER = 3;
localparam OP_FRAMEBUFFER_COMMIT_POS = 0; // Streams the frame buffer (color buffer) content via the framebuffer_axis 
localparam OP_FRAMEBUFFER_MEMSET_POS = 1; // Clears the frame buffer with the configured clear color / depth value
// The selected buffer where to execute the command above (multiple selections are possible)
localparam OP_FRAMEBUFFER_COLOR_BUFFER_SELECT_POS = 4;
localparam OP_FRAMEBUFFER_DEPTH_BUFFER_SELECT_POS = 5;

//---------------------------------------------------------------------------------------------------------
// Triangle Stream
//  +---------------------------------------------------+
//  | 4'h4 | 28'hx size of triangle descriptor in bytes |
//  +---------------------------------------------------+
// Immediate value contains size of triangle in bytes (inclusive the additional bytes which are required for CMD_AXIS bus alignment).
// Steam size n 32bit values.
localparam OP_TRIANGLE_STREAM = 4;

//---------------------------------------------------------------------------------------------------------
// Fog LuT configuration
//  +-----------------------+
//  | 4'h5 | 28'hx reserved |
//  +-----------------------+
// Stream of the fog LuT
// Steam size 33 32bit values.
localparam OP_FOG_LUT_STREAM = 5;




///////////////////////////////////////////////////////////////////////////////////////////////////////////
// Operation Arguments
///////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// OP_NOP_STREAM
// Has no arguments. 

//---------------------------------------------------------------------------------------------------------
// OP_TEXTURE_STREAM_TMU0
// Each texel is build like the following:
//  +-----------------------------------+
//  | 4'hx R | 4'hx G | 4'hx B | 4'hx A |
//  +-----------------------------------+

//---------------------------------------------------------------------------------------------------------
// OP_RENDER_CONFIG
// Description of the different registers:

// OP_RENDER_CONFIG_COLOR_BUFFER_CLEAR_COLOR
//  +-----------------------------------+
//  | 8'hx R | 8'hx G | 8'hx B | 8'hx A |
//  +-----------------------------------+

// OP_RENDER_CONFIG_DEPTH_BUFFER_CLEAR_DEPTH
//  +-----------------+
//  | 16'hx depth val |
//  +-----------------+

// OP_RENDER_CONFIG_FRAGMENT_PIPELINE
//  +---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
//  | 4'hx reserved | 4'hx blend d | 4'hx blend s | 1'hx color mask R | 1'hx color mask G | 1'hx color mask B | 1'hx color mask A | 1'hx depth mask | 8'hx A ref | 3'hx A func | 3'hx depth func | 1'hx enable depth test |
//  +---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
localparam RENDER_CONFIG_FRAGMENT_ENABLE_DEPTH_TEST_POS = 0;
localparam RENDER_CONFIG_FRAGMENT_ENABLE_DEPTH_TEST_SIZE = 1;
localparam RENDER_CONFIG_FRAGMENT_DEPTH_TEST_FUNC_POS = RENDER_CONFIG_FRAGMENT_ENABLE_DEPTH_TEST_POS + RENDER_CONFIG_FRAGMENT_ENABLE_DEPTH_TEST_SIZE;
localparam RENDER_CONFIG_FRAGMENT_DEPTH_TEST_FUNC_SIZE = 3;
localparam RENDER_CONFIG_FRAGMENT_ALPHA_TEST_FUNC_POS = RENDER_CONFIG_FRAGMENT_DEPTH_TEST_FUNC_POS + RENDER_CONFIG_FRAGMENT_DEPTH_TEST_FUNC_SIZE;
localparam RENDER_CONFIG_FRAGMENT_ALPHA_TEST_FUNC_SIZE = 3;
localparam RENDER_CONFIG_FRAGMENT_ALPHA_TEST_REF_VALUE_POS = RENDER_CONFIG_FRAGMENT_ALPHA_TEST_FUNC_POS + RENDER_CONFIG_FRAGMENT_ALPHA_TEST_FUNC_SIZE;
localparam RENDER_CONFIG_FRAGMENT_ALPHA_TEST_REF_VALUE_SIZE = 8;
localparam RENDER_CONFIG_FRAGMENT_DEPTH_MASK_POS = RENDER_CONFIG_FRAGMENT_ALPHA_TEST_REF_VALUE_POS + RENDER_CONFIG_FRAGMENT_ALPHA_TEST_REF_VALUE_SIZE;
localparam RENDER_CONFIG_FRAGMENT_DEPTH_MASK_SIZE = 1;
localparam RENDER_CONFIG_FRAGMENT_COLOR_MASK_A_POS = RENDER_CONFIG_FRAGMENT_DEPTH_MASK_POS + RENDER_CONFIG_FRAGMENT_DEPTH_MASK_SIZE;
localparam RENDER_CONFIG_FRAGMENT_COLOR_MASK_A_SIZE = 1;
localparam RENDER_CONFIG_FRAGMENT_COLOR_MASK_B_POS = RENDER_CONFIG_FRAGMENT_COLOR_MASK_A_POS + RENDER_CONFIG_FRAGMENT_COLOR_MASK_A_SIZE;
localparam RENDER_CONFIG_FRAGMENT_COLOR_MASK_B_SIZE = 1;
localparam RENDER_CONFIG_FRAGMENT_COLOR_MASK_G_POS = RENDER_CONFIG_FRAGMENT_COLOR_MASK_B_POS + RENDER_CONFIG_FRAGMENT_COLOR_MASK_B_SIZE;
localparam RENDER_CONFIG_FRAGMENT_COLOR_MASK_G_SIZE = 1;
localparam RENDER_CONFIG_FRAGMENT_COLOR_MASK_R_POS = RENDER_CONFIG_FRAGMENT_COLOR_MASK_G_POS + RENDER_CONFIG_FRAGMENT_COLOR_MASK_G_SIZE;
localparam RENDER_CONFIG_FRAGMENT_COLOR_MASK_R_SIZE = 1;
localparam RENDER_CONFIG_FRAGMENT_BLEND_FUNC_SFACTOR_POS = RENDER_CONFIG_FRAGMENT_COLOR_MASK_R_POS + RENDER_CONFIG_FRAGMENT_COLOR_MASK_R_SIZE;
localparam RENDER_CONFIG_FRAGMENT_BLEND_FUNC_SFACTOR_SIZE = 4;
localparam RENDER_CONFIG_FRAGMENT_BLEND_FUNC_DFACTOR_POS = RENDER_CONFIG_FRAGMENT_BLEND_FUNC_SFACTOR_POS + RENDER_CONFIG_FRAGMENT_BLEND_FUNC_SFACTOR_SIZE;
localparam RENDER_CONFIG_FRAGMENT_BLEND_FUNC_DFACTOR_SIZE = 4;

// OP_RENDER_CONFIG_TMU0_TEX_ENV
//  +- bit 31 ------------------------------------------------------------------ bit 24 -+
//  | 1'hx res | 2'hx a shift | 2'hx RGB shift | 1'hx op a 2 | 1'hx op a 1 | 1'hx op a 0 |
//  +- bit 23 ------------------------------------------------------------------ bit 15 -+
//  |                   2'hx op RGB 2 | 2'hx op RGB 1 | 2'hx op RGB 0 | 2'hx src reg a 2 |
//  +- bit 15 ------------------------------------------------------------------ bit  8 -+
//  |      2'hx src reg a 1 | 2'hx src reg a 0 | 2'hx src reg rgb 2 | 2'hx src reg rgb 1 |
//  +- bit  7 ------------------------------------------------------------------ bit  0 -+
//  |                             2'hx src reg rgb 0 | 3'hx combine a | 3'hx combine RGB |
//  +------------------------------------------------------------------------------------+
// Note: A shift value of 0x3 is undefined.
localparam RENDER_CONFIG_TMU_TEX_ENV_COMBINE_RGB_POS = 0;
localparam RENDER_CONFIG_TMU_TEX_ENV_COMBINE_RGB_SIZE = 3;
localparam RENDER_CONFIG_TMU_TEX_ENV_COMBINE_ALPHA_POS = RENDER_CONFIG_TMU_TEX_ENV_COMBINE_RGB_POS + RENDER_CONFIG_TMU_TEX_ENV_COMBINE_RGB_SIZE;
localparam RENDER_CONFIG_TMU_TEX_ENV_COMBINE_ALPHA_SIZE = 3;
localparam RENDER_CONFIG_TMU_TEX_ENV_SRC_REG_RGB0_POS = RENDER_CONFIG_TMU_TEX_ENV_COMBINE_ALPHA_POS + RENDER_CONFIG_TMU_TEX_ENV_COMBINE_ALPHA_SIZE;
localparam RENDER_CONFIG_TMU_TEX_ENV_SRC_REG_RGB0_SIZE = 2;
localparam RENDER_CONFIG_TMU_TEX_ENV_SRC_REG_RGB1_POS = RENDER_CONFIG_TMU_TEX_ENV_SRC_REG_RGB0_POS + RENDER_CONFIG_TMU_TEX_ENV_SRC_REG_RGB0_SIZE;
localparam RENDER_CONFIG_TMU_TEX_ENV_SRC_REG_RGB1_SIZE = 2;
localparam RENDER_CONFIG_TMU_TEX_ENV_SRC_REG_RGB2_POS = RENDER_CONFIG_TMU_TEX_ENV_SRC_REG_RGB1_POS + RENDER_CONFIG_TMU_TEX_ENV_SRC_REG_RGB1_SIZE;
localparam RENDER_CONFIG_TMU_TEX_ENV_SRC_REG_RGB2_SIZE = 2;
localparam RENDER_CONFIG_TMU_TEX_ENV_SRC_REG_ALPHA0_POS = RENDER_CONFIG_TMU_TEX_ENV_SRC_REG_RGB2_POS + RENDER_CONFIG_TMU_TEX_ENV_SRC_REG_RGB2_SIZE;
localparam RENDER_CONFIG_TMU_TEX_ENV_SRC_REG_ALPHA0_SIZE = 2;
localparam RENDER_CONFIG_TMU_TEX_ENV_SRC_REG_ALPHA1_POS = RENDER_CONFIG_TMU_TEX_ENV_SRC_REG_ALPHA0_POS + RENDER_CONFIG_TMU_TEX_ENV_SRC_REG_ALPHA0_SIZE;
localparam RENDER_CONFIG_TMU_TEX_ENV_SRC_REG_ALPHA1_SIZE = 2;
localparam RENDER_CONFIG_TMU_TEX_ENV_SRC_REG_ALPHA2_POS = RENDER_CONFIG_TMU_TEX_ENV_SRC_REG_ALPHA1_POS + RENDER_CONFIG_TMU_TEX_ENV_SRC_REG_ALPHA1_SIZE;
localparam RENDER_CONFIG_TMU_TEX_ENV_SRC_REG_ALPHA2_SIZE = 2;
localparam RENDER_CONFIG_TMU_TEX_ENV_OPERAND_RGB0_POS = RENDER_CONFIG_TMU_TEX_ENV_SRC_REG_ALPHA2_POS + RENDER_CONFIG_TMU_TEX_ENV_SRC_REG_ALPHA2_SIZE;
localparam RENDER_CONFIG_TMU_TEX_ENV_OPERAND_RGB0_SIZE = 2;
localparam RENDER_CONFIG_TMU_TEX_ENV_OPERAND_RGB1_POS = RENDER_CONFIG_TMU_TEX_ENV_OPERAND_RGB0_POS + RENDER_CONFIG_TMU_TEX_ENV_OPERAND_RGB0_SIZE;
localparam RENDER_CONFIG_TMU_TEX_ENV_OPERAND_RGB1_SIZE = 2;
localparam RENDER_CONFIG_TMU_TEX_ENV_OPERAND_RGB2_POS = RENDER_CONFIG_TMU_TEX_ENV_OPERAND_RGB1_POS + RENDER_CONFIG_TMU_TEX_ENV_OPERAND_RGB1_SIZE;
localparam RENDER_CONFIG_TMU_TEX_ENV_OPERAND_RGB2_SIZE = 2;
localparam RENDER_CONFIG_TMU_TEX_ENV_OPERAND_ALPHA0_POS = RENDER_CONFIG_TMU_TEX_ENV_OPERAND_RGB2_POS + RENDER_CONFIG_TMU_TEX_ENV_OPERAND_RGB2_SIZE;
localparam RENDER_CONFIG_TMU_TEX_ENV_OPERAND_ALPHA0_SIZE = 1;
localparam RENDER_CONFIG_TMU_TEX_ENV_OPERAND_ALPHA1_POS = RENDER_CONFIG_TMU_TEX_ENV_OPERAND_ALPHA0_POS + RENDER_CONFIG_TMU_TEX_ENV_OPERAND_ALPHA0_SIZE;
localparam RENDER_CONFIG_TMU_TEX_ENV_OPERAND_ALPHA1_SIZE = 1;
localparam RENDER_CONFIG_TMU_TEX_ENV_OPERAND_ALPHA2_POS = RENDER_CONFIG_TMU_TEX_ENV_OPERAND_ALPHA1_POS + RENDER_CONFIG_TMU_TEX_ENV_OPERAND_ALPHA1_SIZE;
localparam RENDER_CONFIG_TMU_TEX_ENV_OPERAND_ALPHA2_SIZE = 1;
localparam RENDER_CONFIG_TMU_TEX_ENV_SHIFT_RGB_POS = RENDER_CONFIG_TMU_TEX_ENV_OPERAND_ALPHA2_POS + RENDER_CONFIG_TMU_TEX_ENV_OPERAND_ALPHA2_SIZE;
localparam RENDER_CONFIG_TMU_TEX_ENV_SHIFT_RGB_SIZE = 2;
localparam RENDER_CONFIG_TMU_TEX_ENV_SHIFT_ALPHA_POS = RENDER_CONFIG_TMU_TEX_ENV_SHIFT_RGB_POS + RENDER_CONFIG_TMU_TEX_ENV_SHIFT_RGB_SIZE;
localparam RENDER_CONFIG_TMU_TEX_ENV_SHIFT_ALPHA_SIZE = 2;

// OP_RENDER_CONFIG_TMU0_TEXTURE_CONFIG
//  +-------------------------------------------------------------------------------------------+
//  | 13'hx reserved | 1'hx mag filter | 1'hx clamp t | 1'hx clamp s | 8'hx height | 8'hx width |
//  +-------------------------------------------------------------------------------------------+
// Texture hight and width are in power of two minus one, means: 8'b0 = 1px, 8'b1 = 2px, 8'b100 = 8px ...
localparam RENDER_CONFIG_TMU_TEXTURE_WIDTH_POS = 0;
localparam RENDER_CONFIG_TMU_TEXTURE_WIDTH_SIZE = 8;
localparam RENDER_CONFIG_TMU_TEXTURE_HEIGHT_POS = RENDER_CONFIG_TMU_TEXTURE_WIDTH_POS + RENDER_CONFIG_TMU_TEXTURE_WIDTH_SIZE;
localparam RENDER_CONFIG_TMU_TEXTURE_HEIGHT_SIZE = 8;
localparam RENDER_CONFIG_TMU_TEXTURE_CLAMP_S_POS = RENDER_CONFIG_TMU_TEXTURE_HEIGHT_POS + RENDER_CONFIG_TMU_TEXTURE_HEIGHT_SIZE;
localparam RENDER_CONFIG_TMU_TEXTURE_CLAMP_S_SIZE = 1;
localparam RENDER_CONFIG_TMU_TEXTURE_CLAMP_T_POS = RENDER_CONFIG_TMU_TEXTURE_CLAMP_S_POS + RENDER_CONFIG_TMU_TEXTURE_CLAMP_S_SIZE;
localparam RENDER_CONFIG_TMU_TEXTURE_CLAMP_T_SIZE = 1;
localparam RENDER_CONFIG_TMU_TEXTURE_MAG_FILTER_POS = RENDER_CONFIG_TMU_TEXTURE_CLAMP_T_POS + RENDER_CONFIG_TMU_TEXTURE_CLAMP_T_SIZE;
localparam RENDER_CONFIG_TMU_TEXTURE_MAG_FILTER_SIZE = 1;

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
localparam REPLACE = 0;
localparam MODULATE = 1;
localparam ADD = 2;
localparam ADD_SIGNED = 3;
localparam INTERPOLATE = 4;
localparam SUBTRACT = 5;
localparam DOT3_RGB = 6;
localparam DOT3_RGBA = 7;

localparam SRC_TEXTURE = 0;
localparam SRC_CONSTANT = 1;
localparam SRC_PRIMARY_COLOR = 2;
localparam SRC_PREVIOUS = 3;

localparam OPERAND_RGB_SRC_ALPHA = 0;
localparam OPERAND_RGB_ONE_MINUS_SRC_ALPHA = 1;
localparam OPERAND_RGB_SRC_COLOR = 2;
localparam OPERAND_RGB_ONE_MINUS_SRC_COLOR = 3;

localparam OPERAND_ALPHA_SRC_ALPHA = 0;
localparam OPERAND_ALPHA_ONE_MINUS_SRC_ALPHA = 1;

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

// OP_RENDER_CONFIG_TMU0_TEX_ENV_COLOR
//  +-----------------------------------+
//  | 8'hx R | 8'hx G | 8'hx B | 8'hx A |
//  +-----------------------------------+

// Color defines
localparam COLOR_SUB_PIXEL_WIDTH = 8;
localparam COLOR_NUMBER_OF_SUB_PIXEL = 4;
localparam COLOR_R_POS = COLOR_SUB_PIXEL_WIDTH * 3;
localparam COLOR_G_POS = COLOR_SUB_PIXEL_WIDTH * 2;
localparam COLOR_B_POS = COLOR_SUB_PIXEL_WIDTH * 1;
localparam COLOR_A_POS = COLOR_SUB_PIXEL_WIDTH * 0;

//---------------------------------------------------------------------------------------------------------
// OP_FRAMEBUFFER
// Does not contain any arguments, just starts the framebuffer commit and clear processes

//---------------------------------------------------------------------------------------------------------
// OP_TRIANGLE_STREAM 
// Triangle Descriptor, each value containts 4 bytes.
localparam PARAM_SIZE = 32;
localparam TRIANGLE_COLOR = 0;
localparam BB_START = 1; // S15.0, S15.0 (32bit)
localparam BB_END = 2; // S15.0, S15.0 (32bit)
localparam INC_W0 = 3; // Sn.m (32bit)
localparam INC_W1 = 4; // Sn.m (32bit)
localparam INC_W2 = 5; // Sn.m (32bit)
localparam INC_W0_X = 6; // Sn.m (32bit)
localparam INC_W1_X = 7; // Sn.m (32bit)
localparam INC_W2_X = 8; // Sn.m (32bit)
localparam INC_W0_Y = 9; // Sn.m (32bit)
localparam INC_W1_Y = 10; // Sn.m (32bit)
localparam INC_W2_Y = 11; // Sn.m (32bit)
localparam INC_TEX_S = 12; // float (32bit)
localparam INC_TEX_T = 13; // float (32bit)
localparam INC_TEX_S_X = 14; // float (32bit)
localparam INC_TEX_T_X = 15; // float (32bit)
localparam INC_TEX_S_Y = 16; // float (32bit)
localparam INC_TEX_T_Y = 17; // float (32bit)
localparam INC_DEPTH_W = 18; // float (32bit)
localparam INC_DEPTH_W_X = 19; // float (32bit)
localparam INC_DEPTH_W_Y = 20; // float (32bit)
localparam INC_DEPTH_Z = 21; // float (32bit)
localparam INC_DEPTH_Z_X = 22; // float (32bit)
localparam INC_DEPTH_Z_Y = 23; // float (32bit)
localparam INC_COLOR_R = 24; // float (32bit)
localparam INC_COLOR_G = 25; // float (32bit)
localparam INC_COLOR_B = 26; // float (32bit)
localparam INC_COLOR_A = 27; // float (32bit)
localparam INC_COLOR_R_X = 28; // float (32bit)
localparam INC_COLOR_G_X = 29; // float (32bit)
localparam INC_COLOR_B_X = 30; // float (32bit)
localparam INC_COLOR_A_X = 31; // float (32bit)
localparam INC_COLOR_R_Y = 32; // float (32bit)
localparam INC_COLOR_G_Y = 33; // float (32bit)
localparam INC_COLOR_B_Y = 34; // float (32bit)
localparam INC_COLOR_A_Y = 35; // float (32bit)
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

//---------------------------------------------------------------------------------------------------------
// OP_FOG_LUT_STREAM
// See FunctionInterpolator.v for documentation.

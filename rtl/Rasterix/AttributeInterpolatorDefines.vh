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

localparam ATTR_INTERP_AXIS_VERTEX_ATTRIBUTE_SIZE = 32;

localparam ATTR_INTERP_AXIS_SCREEN_POS_SIZE = 16;
localparam ATTR_INTERP_AXIS_SCREEN_X_POS = 0;
localparam ATTR_INTERP_AXIS_SCREEN_Y_POS = 16;

localparam ATTR_INTERP_AXIS_COLOR_R_POS             = 0;
localparam ATTR_INTERP_AXIS_COLOR_G_POS             = ATTR_INTERP_AXIS_COLOR_R_POS      + ATTR_INTERP_AXIS_VERTEX_ATTRIBUTE_SIZE;
localparam ATTR_INTERP_AXIS_COLOR_B_POS             = ATTR_INTERP_AXIS_COLOR_G_POS      + ATTR_INTERP_AXIS_VERTEX_ATTRIBUTE_SIZE;
localparam ATTR_INTERP_AXIS_COLOR_A_POS             = ATTR_INTERP_AXIS_COLOR_B_POS      + ATTR_INTERP_AXIS_VERTEX_ATTRIBUTE_SIZE;
localparam ATTR_INTERP_AXIS_DEPTH_Z_POS             = ATTR_INTERP_AXIS_COLOR_A_POS      + ATTR_INTERP_AXIS_VERTEX_ATTRIBUTE_SIZE;
localparam ATTR_INTERP_AXIS_TEXTURE1_S_POS          = ATTR_INTERP_AXIS_DEPTH_Z_POS      + ATTR_INTERP_AXIS_VERTEX_ATTRIBUTE_SIZE;
localparam ATTR_INTERP_AXIS_TEXTURE1_T_POS          = ATTR_INTERP_AXIS_TEXTURE1_S_POS   + ATTR_INTERP_AXIS_VERTEX_ATTRIBUTE_SIZE;
localparam ATTR_INTERP_AXIS_TEXTURE0_S_POS          = ATTR_INTERP_AXIS_TEXTURE1_T_POS   + ATTR_INTERP_AXIS_VERTEX_ATTRIBUTE_SIZE;
localparam ATTR_INTERP_AXIS_TEXTURE0_T_POS          = ATTR_INTERP_AXIS_TEXTURE0_S_POS   + ATTR_INTERP_AXIS_VERTEX_ATTRIBUTE_SIZE;
localparam ATTR_INTERP_AXIS_DEPTH_W_POS             = ATTR_INTERP_AXIS_TEXTURE0_T_POS   + ATTR_INTERP_AXIS_VERTEX_ATTRIBUTE_SIZE;
localparam ATTR_INTERP_AXIS_SCREEN_XY_POS           = ATTR_INTERP_AXIS_DEPTH_W_POS      + ATTR_INTERP_AXIS_VERTEX_ATTRIBUTE_SIZE;
localparam ATTR_INTERP_AXIS_FRAMEBUFFER_INDEX_POS   = ATTR_INTERP_AXIS_SCREEN_XY_POS    + ATTR_INTERP_AXIS_VERTEX_ATTRIBUTE_SIZE;
localparam ATTR_INTERP_AXIS_PARAMETER_SIZE          = (12 * ATTR_INTERP_AXIS_VERTEX_ATTRIBUTE_SIZE);

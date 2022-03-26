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

localparam ATTR_INTERP_AXIS_COLOR_R_POS             = 0;
localparam ATTR_INTERP_AXIS_COLOR_G_POS             = ATTR_INTERP_AXIS_COLOR_R_POS      + ATTR_INTERP_AXIS_VERTEX_ATTRIBUTE_SIZE;
localparam ATTR_INTERP_AXIS_COLOR_B_POS             = ATTR_INTERP_AXIS_COLOR_G_POS      + ATTR_INTERP_AXIS_VERTEX_ATTRIBUTE_SIZE;
localparam ATTR_INTERP_AXIS_COLOR_A_POS             = ATTR_INTERP_AXIS_COLOR_B_POS      + ATTR_INTERP_AXIS_VERTEX_ATTRIBUTE_SIZE;
localparam ATTR_INTERP_AXIS_DEPTH_Z_POS             = ATTR_INTERP_AXIS_COLOR_A_POS      + ATTR_INTERP_AXIS_VERTEX_ATTRIBUTE_SIZE;
localparam ATTR_INTERP_AXIS_TEXTURE_S_POS           = ATTR_INTERP_AXIS_DEPTH_Z_POS      + ATTR_INTERP_AXIS_VERTEX_ATTRIBUTE_SIZE;
localparam ATTR_INTERP_AXIS_TEXTURE_T_POS           = ATTR_INTERP_AXIS_TEXTURE_S_POS    + ATTR_INTERP_AXIS_VERTEX_ATTRIBUTE_SIZE;
localparam ATTR_INTERP_AXIS_DEPTH_W_POS             = ATTR_INTERP_AXIS_TEXTURE_T_POS    + ATTR_INTERP_AXIS_VERTEX_ATTRIBUTE_SIZE;
localparam ATTR_INTERP_AXIS_FRAMEBUFFER_INDEX_POS   = ATTR_INTERP_AXIS_DEPTH_W_POS      + ATTR_INTERP_AXIS_VERTEX_ATTRIBUTE_SIZE;
localparam ATTR_INTERP_AXIS_PARAMETER_SIZE          = (9 * ATTR_INTERP_AXIS_VERTEX_ATTRIBUTE_SIZE);

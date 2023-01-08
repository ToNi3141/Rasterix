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


#ifndef GLU_H 
#define GLU_H

#include "gl.h"
#ifdef __cplusplus
extern "C" {
#endif
GLAPI void APIENTRY gluPerspective(GLfloat fovy, GLfloat aspect, GLfloat zNear, GLfloat zFar);
GLAPI void APIENTRY gluLookAt(GLfloat eyex, GLfloat eyey, GLfloat eyez, GLfloat centerx,
                              GLfloat centery, GLfloat centerz, GLfloat upx, GLfloat upy,
                              GLfloat upz);

#ifdef __cplusplus
}
#endif

#endif // GLU_H

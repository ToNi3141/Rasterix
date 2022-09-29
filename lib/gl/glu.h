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

#endif GLU_H

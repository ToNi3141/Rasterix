#include "glu.h"
#include "Mat44.hpp"
#include "Vec.hpp"
#include <cmath>

#define __glPi 3.14159265358979323846f

GLAPI void APIENTRY gluPerspective(GLfloat fovy, GLfloat aspect, GLfloat zNear, GLfloat zFar)
{
    Mat44 m;
    float sine, cotangent, deltaZ;
    float radians = fovy / 2.0f * __glPi / 180.0f;

    deltaZ = zFar - zNear;
    sine = sinf(radians);
    if ((deltaZ == 0.0f) || (sine == 0.0f) || (aspect == 0.0f)) {
        return;
    }
    cotangent = cosf(radians) / sine;

    m.identity();
    m[0][0] = cotangent / aspect;
    m[1][1] = cotangent;
    m[2][2] = -(zFar + zNear) / deltaZ;
    m[2][3] = -1.0f;
    m[3][2] = -2.0f * zNear * zFar / deltaZ;
    m[3][3] = 0.0f;
    glMultMatrixf(&m[0][0]);
}

GLAPI void APIENTRY gluLookAt(GLfloat eyex, GLfloat eyey, GLfloat eyez, GLfloat centerx,
                              GLfloat centery, GLfloat centerz, GLfloat upx, GLfloat upy,
                              GLfloat upz)
{
    Vec3 forward{{centerx - eyex,
                    centery - eyey,
                    centerz - eyez}};
    Vec3 up{{upx, upy, upz}};

    Mat44 m;

    forward.normalize();
    Vec3 side{forward};

    /* Side = forward x up */
    side.cross(up);
    side.normalize();

    /* Recompute up as: up = side x forward */
    up = side;
    up.cross(forward);

    m.identity();
    m[0][0] = side[0];
    m[1][0] = side[1];
    m[2][0] = side[2];

    m[0][1] = up[0];
    m[1][1] = up[1];
    m[2][1] = up[2];

    m[0][2] = -forward[0];
    m[1][2] = -forward[1];
    m[2][2] = -forward[2];

    glMultMatrixf(&m[0][0]);
    glTranslatef(-eyex, -eyey, -eyez);
}

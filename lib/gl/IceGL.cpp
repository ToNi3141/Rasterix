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

#include "IceGL.hpp"
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

IceGL::IceGL(IRenderer &renderer)
    : m_renderer(renderer)
{
    // Preallocate the first texture. This is the default texture and it also can't be deleted.
    m_renderer.createTexture();

    m_m.identity();
    m_p.identity();
    m_t.identity();

    //glLogicOp(GL_COPY);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
    glDisable(GL_ALPHA_TEST);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_FOG);
    glClearDepthf(1.0f);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    float fogColor[] = {0.0f, 0.0f, 0.0f, 0.0f};
    glFogfv(GL_FOG_COLOR, fogColor);
}

void IceGL::commit()
{
    m_renderer.commit();
}

void IceGL::glMatrixMode(GLenum mm)
{
    matrixMode = mm;
}

void IceGL::glMultMatrixf(const GLfloat* m)
{
    const Mat44 *m44 = reinterpret_cast<const Mat44*>(m);
    if (matrixMode == GL_MODELVIEW)
    {
        m_m = *m44 * m_m;
    }
    else
    {
        m_p = *m44 * m_p;
    }

    m_matricesOutdated = true;
}

void IceGL::glMultMatrixd(const GLdouble* m)
{
    GLfloat mf[16];
    for (uint32_t i = 0; i < 4*4; i++)
    {
        mf[i] = m[i];
    }
    glMultMatrixf(mf);
}

void IceGL::glTranslatef(GLfloat x, GLfloat y, GLfloat z)
{
    Mat44 m;
    m.identity();
    m[3][0] = x;
    m[3][1] = y;
    m[3][2] = z;
    glMultMatrixf(&m[0][0]);
    m_matricesOutdated = true;
}

void IceGL::glScalef(GLfloat x, GLfloat y, GLfloat z)
{
    Mat44 m;
    m.identity();
    m[0][0] = x;
    m[1][1] = y;
    m[2][2] = z;
    glMultMatrixf(&m[0][0]);
    m_matricesOutdated = true;
}

void IceGL::glRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
    float angle_rad = angle * (__glPi/180.0f);

    float c = cosf(angle_rad);
    float s = sinf(angle_rad);
    float t = 1.0f - c;

    Mat44 m
    {{{
                {c+x*x*t,   y*x*t+z*s,  z*x*t-y*s,  0.0f},
                {x*y*t-z*s, c+y*y*t,    z*y*t+x*s,  0.0f},
                {x*z*t+y*s, y*z*t-x*s,  z*z*t+c,    0.0f},
                {0.0f,      0.0f,       0.0f,       1.0f}
            }}};

    glMultMatrixf(&m[0][0]);
    m_matricesOutdated = true;
}

void IceGL::glLoadIdentity()
{
    if (matrixMode == GL_MODELVIEW)
    {
        m_m.identity();
    }
    else
    {
        m_p.identity();
    }
    m_matricesOutdated = true;
}

void IceGL::gluPerspective(GLfloat fovy, GLfloat aspect, GLfloat zNear, GLfloat zFar)
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

void IceGL::gluLookAt(GLfloat eyex, GLfloat eyey, GLfloat eyez, GLfloat centerx,
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


void IceGL::glViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
    // TODO: Generate a GL_INVALID_VALUE if width or height is negative
    // TODO: Reversed mapping is not working right now, for instance if zFar < zNear
    // TODO: The precision here affects the texture mapping.
    //      It is not that easy to fix, because the z for the zBuffer is also the same z for textures.
    //      It probably needs to be handled independenly that the z for the texture and for the zbuffer
    //      don't affect each other.
    // Note: The screen resolution is width and height. But during view port transformation we are clamping between
    // 0 and height which means a effective screen resolution of height + 1. For instance, we have a resolution of
    // 480 x 272. The view port transformation would go from 0 to 480 which are then 481px. Thats the reason why we
    // decrement here the resolution by one.
    m_tnl.setViewport(x, y, width, height);
}

void IceGL::glDepthRange(GLclampd zNear, GLclampd zFar)
{
    if (zNear > 1.0f) zNear = 1.0f;
    if (zNear < -1.0f) zNear = -1.0f;
    if (zFar  > 1.0f) zFar  = 1.0f;
    if (zFar  < -1.0f) zFar  = -1.0f;

    m_tnl.setDepthRange(zNear, zFar);
    // TODO: Check in which state the renderer is and throw an GL_INVALID_OPERATION if this is called within an glBegin / glEnd
}

void IceGL::glBegin(GLenum mode)
{
    if ((mode == GL_TRIANGLES)
            || (mode == GL_TRIANGLE_FAN)
            || (mode == GL_TRIANGLE_STRIP)
            || (mode == GL_QUAD_STRIP))
    {
        m_beginMode = mode;
        m_textureVertexBuffer.clear();
        m_vertexBuffer.clear();
        m_normalVertexBuffer.clear();
        m_colorVertexBuffer.clear();
        m_error = GL_NO_ERROR;
    }
    else
    {
        m_error = GL_INVALID_ENUM;
    }
}

void IceGL::glEnd()
{
    recalculateAndSetTnLMatrices();
    if (m_beginMode == GL_TRIANGLES)
    {
        for (uint32_t i = 0; i < m_vertexBuffer.size() - 2; i += 3)
        {
            m_tnl.drawTriangle(m_renderer,
            {m_vertexBuffer[i],
             m_vertexBuffer[i + 1],
             m_vertexBuffer[i + 2],
             m_textureVertexBuffer[i],
             m_textureVertexBuffer[i + 1],
             m_textureVertexBuffer[i + 2],
             m_normalVertexBuffer[i],
             m_normalVertexBuffer[i + 1],
             m_normalVertexBuffer[i + 2],
             m_colorVertexBuffer[i],
             m_colorVertexBuffer[i + 1],
             m_colorVertexBuffer[i + 2]});
        }
    }
    if (m_beginMode == GL_TRIANGLE_FAN)
    {
        for (uint32_t i = 0; i < m_vertexBuffer.size() - 2; i++)
        {
            m_tnl.drawTriangle(m_renderer,
            {m_vertexBuffer[0],
             m_vertexBuffer[i + 1],
             m_vertexBuffer[i + 2],
             m_textureVertexBuffer[0],
             m_textureVertexBuffer[i + 1],
             m_textureVertexBuffer[i + 2],
             m_normalVertexBuffer[0],
             m_normalVertexBuffer[i + 1],
             m_normalVertexBuffer[i + 2],
             m_colorVertexBuffer[0],
             m_colorVertexBuffer[i + 1],
             m_colorVertexBuffer[i + 2]});
        }
    }

    if (m_beginMode == GL_TRIANGLE_STRIP)
    {
        for (uint32_t i = 0; i < m_vertexBuffer.size() - 2; i++)
        {
            if (i & 0x1)
            {
                m_tnl.drawTriangle(m_renderer,
                {m_vertexBuffer[i + 1],
                 m_vertexBuffer[i],
                 m_vertexBuffer[i + 2],
                 m_textureVertexBuffer[i + 1],
                 m_textureVertexBuffer[i],
                 m_textureVertexBuffer[i + 2],
                 m_normalVertexBuffer[i + 1],
                 m_normalVertexBuffer[i],
                 m_normalVertexBuffer[i + 2],
                 m_colorVertexBuffer[i + 1],
                 m_colorVertexBuffer[i],
                 m_colorVertexBuffer[i + 2]});
            }
            else
            {
                m_tnl.drawTriangle(m_renderer,
                {m_vertexBuffer[i],
                 m_vertexBuffer[i + 1],
                 m_vertexBuffer[i + 2],
                 m_textureVertexBuffer[i],
                 m_textureVertexBuffer[i + 1],
                 m_textureVertexBuffer[i + 2],
                 m_normalVertexBuffer[i],
                 m_normalVertexBuffer[i + 1],
                 m_normalVertexBuffer[i + 2],
                 m_colorVertexBuffer[i],
                 m_colorVertexBuffer[i + 1],
                 m_colorVertexBuffer[i + 2]});
            }
        }
    }
    if (m_beginMode == GL_QUAD_STRIP)
    {
        for (uint32_t i = 0; i < m_vertexBuffer.size() - 2; i++)
        {
            if (i & 0x2)
            {
                m_tnl.drawTriangle(m_renderer,
                {m_vertexBuffer[i + 1],
                 m_vertexBuffer[i],
                 m_vertexBuffer[i + 2],
                 m_textureVertexBuffer[i + 1],
                 m_textureVertexBuffer[i],
                 m_textureVertexBuffer[i + 2],
                 m_normalVertexBuffer[i + 1],
                 m_normalVertexBuffer[i],
                 m_normalVertexBuffer[i + 2],
                 m_colorVertexBuffer[i + 1],
                 m_colorVertexBuffer[i],
                 m_colorVertexBuffer[i + 2]});
            }
            else
            {
                m_tnl.drawTriangle(m_renderer,
                {m_vertexBuffer[i],
                 m_vertexBuffer[i + 1],
                 m_vertexBuffer[i + 2],
                 m_textureVertexBuffer[i],
                 m_textureVertexBuffer[i + 1],
                 m_textureVertexBuffer[i + 2],
                 m_normalVertexBuffer[i],
                 m_normalVertexBuffer[i + 1],
                 m_normalVertexBuffer[i + 2],
                 m_colorVertexBuffer[i],
                 m_colorVertexBuffer[i + 1],
                 m_colorVertexBuffer[i + 2]});
            }
        }
    }
}

void IceGL::glTexCoord2f(GLfloat s, GLfloat t)
{
    m_textureCoord = {s, t};
    m_error = GL_NO_ERROR;
}

void IceGL::glVertex3f(GLfloat x, GLfloat y, GLfloat z)
{
    m_vertexBuffer.push_back({{x, y, z, 1.0f}});
    m_normalVertexBuffer.push_back(m_normal);
    m_textureVertexBuffer.push_back(m_textureCoord);
    m_colorVertexBuffer.push_back(m_vertexColor);
}

void IceGL::glNormal3f(GLfloat nx, GLfloat ny, GLfloat nz)
{
    m_normal = {nx, ny, nz};
    m_error = GL_NO_ERROR;
}

void IceGL::glColor4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
    m_vertexColor = {red, green, blue, alpha};
    m_error = GL_NO_ERROR;
}

void IceGL::glColor4ub(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha)
{
    m_vertexColor = {(static_cast<float>(red) / 255.0f), 
                     (static_cast<float>(green) / 255.0f), 
                     (static_cast<float>(blue) / 255.0f), 
                     (static_cast<float>(alpha) / 255.0f)};
    m_error = GL_NO_ERROR;
}

void IceGL::glColor3f(GLfloat red, GLfloat green, GLfloat blue)
{
    m_vertexColor = {red, green, blue, 1.0f};
    m_error = GL_NO_ERROR;
}

void IceGL::glColor3ub(GLubyte red, GLubyte green, GLubyte blue)
{
    glColor4ub(red, green, blue, 255);
}

void IceGL::glColor3dv(const GLdouble *v)
{
    glColor3f(v[0], v[1], v[2]);
}

void IceGL::glColor4dv(const GLdouble *v)
{
    glColor4f(v[0], v[1], v[2], v[3]);
}

void IceGL::glColor4fv(const GLfloat *v)
{
    glColor4f(v[0], v[1], v[2], v[3]);
}

void IceGL::glTexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels)
{
    (void)internalformat; // Currently only RGBA4444 is supported, all unsupported formats are converted to RGBA4444
    m_error = GL_NO_ERROR;
    if (target != GL_TEXTURE_2D
            || !(format == GL_ALPHA
                 || format == GL_RGB
                 || format == GL_RGBA
                 || format == GL_LUMINANCE
                 || format == GL_LUMINANCE_ALPHA)
            || !(type == GL_UNSIGNED_BYTE
                 || type == GL_UNSIGNED_SHORT_4_4_4_4
                 || type == GL_UNSIGNED_SHORT_5_5_5_1
                 || type == GL_UNSIGNED_SHORT_5_6_5))
    {
        m_error = GL_INVALID_ENUM;
        return;
    }

    if (/*level < 0
                    || !(level & GL_TEXTURE_SUPPORTED_SIZES)*/ // Level is rightnow not supported
            !(width & GL_TEXTURE_SUPPORTED_SIZES)
            || !(height & GL_TEXTURE_SUPPORTED_SIZES)
            || border != 0) // In OpenGL ES 1.1 it has to be 0. What does border mean: //https://stackoverflow.com/questions/913801/what-does-border-mean-in-the-glteximage2d-function
    {
        m_error = GL_INVALID_VALUE;
        return;
    }

    if (((type == GL_UNSIGNED_SHORT_5_6_5) && (format != GL_RGB))
            || (((type == GL_UNSIGNED_SHORT_4_4_4_4) || (type == GL_UNSIGNED_SHORT_5_5_5_1)) && (format != GL_RGBA)))
    {
        m_error = GL_INVALID_OPERATION;
        return;
    }

    if (level != 0
            || format == GL_ALPHA
            || format == GL_LUMINANCE
            || format == GL_LUMINANCE_ALPHA
            || height != width) // Only square textures are supported
    {
        m_error = GL_SPEC_DEVIATION;
        return;
    }

    std::shared_ptr<uint16_t> texMemShared(new uint16_t[(width * height * 2)], [] (const uint16_t *p) { delete [] p; });
    if (!texMemShared)
    {
        m_error = GL_OUT_OF_MEMORY;
        return;
    }

    if (pixels != nullptr)
    {
        // Currently only GL_RGB and GL_RGBA is supported
        const int32_t colorComponents = (format == GL_RGB) ? 3 : 4;

        // Convert color to RGBA4444
        if (type == GL_UNSIGNED_BYTE)
        {
            uint16_t tmpColor = 0;
            uint8_t shift = 12;
            for (int32_t i = 0; i < width * height * colorComponents; i++)
            {
                tmpColor |= (reinterpret_cast<const uint8_t*>(pixels)[i] >> 4) << shift;
                shift -= 4;
                if ((i % colorComponents) == (colorComponents - 1))
                {
                    if (colorComponents == 3)
                        tmpColor |= 0xf; // Set alpha to 0xf to make the texture opaque
                    texMemShared.get()[i / colorComponents] = tmpColor;
                    tmpColor = 0;
                    shift = 12;
                }
            }
        }
        // This is the native format, just memcpy it.
        else if (type == GL_UNSIGNED_SHORT_4_4_4_4)
        {
            memcpy(texMemShared.get(), pixels, width * height * 2);
        }
        // Convert to RGBA4444
        else if (type == GL_UNSIGNED_SHORT_5_5_5_1)
        {
            for (int32_t i = 0; i < width * height; i++)
            {
                uint16_t tmp = reinterpret_cast<const uint16_t*>(pixels)[i];
                texMemShared.get()[i] = ((tmp << 1) & 0xf000)
                        | ((tmp << 2) & 0x0f00)
                        | ((tmp << 3) & 0x00f0)
                        | ((tmp & 0x1) ? 0xf : 0x0);
            }
        }
        else if (type == GL_UNSIGNED_SHORT_5_6_5)
        {
            for (int32_t i = 0; i < width * height; i++)
            {
                uint16_t tmp = reinterpret_cast<const uint16_t*>(pixels)[i];
                texMemShared.get()[i] = ((tmp << 1) & 0xf000)
                        | ((tmp << 3) & 0x0f00)
                        | ((tmp << 4) & 0x00f0)
                        | 0x0;
            }
        }

        if (!m_renderer.updateTexture(m_boundTexture, texMemShared, width, height))
        {
            m_error = GL_INVALID_VALUE;
            return;
        }

        // Rebind texture to update the rasterizer with the new texture meta information
        glBindTexture(target, m_boundTexture);
    }
    else
    {
        m_error = GL_INVALID_VALUE;
    }
}

void IceGL::glPixelStorei(GLenum pname, GLint param)
{
    m_error = GL_NO_ERROR;

    if (pname == GL_PACK_ALIGNMENT)
    {
        m_error = GL_SPEC_DEVIATION;
        return;
    }

    if (pname == GL_UNPACK_ALIGNMENT)
    {
        switch (param)
        {
        case 1:
        case 2:
        case 4:
        case 8:
            m_unpackAlignment = param;
            break;
        default:
            m_error = GL_INVALID_VALUE;
            break;
        }
    }
    else
    {
        m_error = GL_INVALID_ENUM;
    }
}

void IceGL::glGenTextures(GLsizei n, GLuint *textures)
{
    m_error = GL_NO_ERROR;
    if (n < 0)
    {
        m_error = GL_INVALID_VALUE;
        return;
    }

    for (GLsizei i = 0; i < n; i++)
    {
        std::pair<bool, uint16_t> ret = m_renderer.createTexture();
        if (ret.first)
        {
            textures[i] = ret.second;
        }
        else
        {
            // TODO: Free allocated textures to avoid leaks
            m_error = GL_OUT_OF_MEMORY;
            return;
        }
    }
}

void IceGL::glDeleteTextures(GLsizei n, const GLuint *textures)
{
    m_error = GL_NO_ERROR;
    if (n < 0)
    {
        m_error = GL_INVALID_VALUE;
        return;
    }

    for (GLsizei i = 0; i < n; i++)
    {
        // From the specification: glDeleteTextures silently ignores 0's and names that do not correspond to existing textures.
        if (textures[i] != 0) 
        {
            m_renderer.deleteTexture(textures[i]);
        }
        
    }
}

void IceGL::glBindTexture(GLenum target, GLuint texture)
{
    m_error = GL_NO_ERROR;
    if (target != GL_TEXTURE_2D)
    {
        m_error = GL_INVALID_ENUM;
        return;
    }

    m_boundTexture = texture;

    if (m_error == GL_NO_ERROR)
    {
        m_renderer.useTexture(m_boundTexture);
    }
    else 
    {
        // If the bound texture is 0 or if using an invalid texture, then use the default texture.
        // Assume the default texture as no texture -> disable texture unit
        glDisable(GL_TEXTURE_2D);
    }
}

void IceGL::glTexParameteri(GLenum target, GLenum pname, GLint param)
{
    m_error = GL_NO_ERROR;
    if (target == GL_TEXTURE_2D)
    {
        auto mode = convertGlTextureWrapMode(static_cast<GLenum>(param));
        if (m_error == GL_NO_ERROR)
        {
            switch (pname) {
            case GL_TEXTURE_WRAP_S:
                m_renderer.setTextureWrapModeS(mode);
                break;
            case GL_TEXTURE_WRAP_T:
                m_renderer.setTextureWrapModeT(mode);
                break;
            default:
                m_error = GL_INVALID_ENUM;
                break;
            }
        }
        else
        {
            m_error = GL_INVALID_ENUM;
        }
    }
    else
    {
        // TODO: Implement other modes like GL_TEXTURE_MAG_FILTER and so on, but they require hardware support (which is right now not implemented).
        m_error = GL_INVALID_ENUM;
    }
}

void IceGL::glClear(GLbitfield mask)
{
    // TODO GL_STENCIL_BUFFER_BIT has to be implemented
    if (m_renderer.clear(mask & GL_COLOR_BUFFER_BIT, mask & GL_DEPTH_BUFFER_BIT))
    {
        m_error = GL_NO_ERROR;
    }
    else
    {
        m_error = GL_OUT_OF_MEMORY;
    }
}

void IceGL::glClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
{
    (void)alpha;
    if (m_renderer.setClearColor({{static_cast<uint8_t>(red * 255.0f),
                                 static_cast<uint8_t>(green * 255.0f),
                                 static_cast<uint8_t>(blue * 255.0f),
                                 static_cast<uint8_t>(alpha * 255.0f)}}))
    {
        m_error = GL_NO_ERROR;
    }
    else
    {
        m_error = GL_OUT_OF_MEMORY;
    }
}

void IceGL::glClearDepthf(GLclampf depth)
{
    if (depth < -1.0f)
    {
        depth = -1.0f;
    }
        
    if (depth > 1.0f)
    {
        depth = 1.0f;
    }

    // Convert from signed float (-1.0 .. 1.0) to unsigned fix (0 .. 65535)
    const uint16_t depthx = (depth + 1.0f) * 32767;
    if (m_renderer.setClearDepth(depthx))
    {
        m_error = GL_NO_ERROR;
    }
    else
    {
        m_error = GL_OUT_OF_MEMORY;
    }
}

void IceGL::glClearStencil(GLint s)
{
    (void)s;
    m_error = GL_SPEC_DEVIATION;
}

void IceGL::glEnable(GLenum cap)
{
    // TODO: Implement enable and disable for the LogicFunc
    switch (cap)
    {
    case GL_TEXTURE_2D:
        m_enableTextureMapping = true;
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, m_texEnvParam);
        break;
    case GL_ALPHA_TEST:
        // Note: The alpha test disabling and enabling is a bit special. If the alpha test is disabled, we are just saveing
        // the current alpha test func and the reference value. When the test is enabled, we just recover this values.
        // Seems to work perfect.
        m_enableAlphaTest = true;
        glAlphaFunc(m_alphaTestFunc, m_alphaTestRefValue);
        break;
    case GL_DEPTH_TEST:
        // For the depth test it is not possible to use a similar algorithem like we do for the alpha test. The main reason
        // behind that is that the depth test uses a depth buffer. Is the depth test is disabled, the depth test always passes
        // and never writes into the depth buffer. We could use glDepthMask to avoid writing, but then we also disabling
        // glClear() what we obviously dont want. The easiest fix is to introduce a special switch. If this switch is disabled
        // the depth buffer always passes and never writes and glClear() can clear the depth buffer.
        m_renderer.enableDepthTest(true);
        break;
    case GL_BLEND:
        m_enableBlending = true;
        glBlendFunc(m_blendSfactor, m_blendDfactor);
        break;
    case GL_LIGHTING:
        m_tnl.enableLighting(true);
        break;
    case GL_LIGHT0:
    case GL_LIGHT1:
    case GL_LIGHT2:
    case GL_LIGHT3:
    case GL_LIGHT4:
    case GL_LIGHT5:
    case GL_LIGHT6:
    case GL_LIGHT7:
        m_tnl.enableLight(cap - GL_LIGHT0, true);
        break;
    case GL_TEXTURE_GEN_S:
        m_tnl.enableTexGenS(true);
        break;
    case GL_TEXTURE_GEN_T:
        m_tnl.enableTexGenT(true);
        break;
    case GL_CULL_FACE:
        m_tnl.enableCulling(true);
        break;
    case GL_COLOR_MATERIAL:
        m_enableColorMaterial = true;
        glColorMaterial(m_colorMaterialFace, m_colorMaterialTracking);
        break;
    case GL_FOG:
        m_enableFog = true;
        m_error = setFogLut(m_fogMode, m_fogStart, m_fogEnd, m_fogDensity);
        break;
    default:
        m_error = GL_SPEC_DEVIATION;
        break;
    }
}

void IceGL::glDisable(GLenum cap)
{
    switch (cap)
    {
    case GL_TEXTURE_2D:
    {
        const GLint tmpParam = m_texEnvParam;
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DISABLE);
        m_texEnvParam = tmpParam;
        m_enableTextureMapping = false;
        break;
    }
    case GL_ALPHA_TEST:
    {
        const GLclampf refValue = m_alphaTestRefValue;
        const GLenum func = m_alphaTestFunc;
        glAlphaFunc(GL_ALWAYS, 0.0f);
        m_alphaTestRefValue = refValue;
        m_alphaTestFunc = func;
        m_enableAlphaTest = false;
        break;
    }
    case GL_DEPTH_TEST:
    {
        m_renderer.enableDepthTest(false);
        break;
    }
    case GL_BLEND:
    {
        const GLenum dfactor = m_blendDfactor;
        const GLenum sfactor = m_blendSfactor;
        glBlendFunc(GL_ONE, GL_ZERO);
        m_blendDfactor = dfactor;
        m_blendSfactor = sfactor;
        m_enableBlending = false;
        break;
    }
    case GL_LIGHTING:
        m_tnl.enableLighting(false);
        break;
    case GL_LIGHT0:
    case GL_LIGHT1:
    case GL_LIGHT2:
    case GL_LIGHT3:
    case GL_LIGHT4:
    case GL_LIGHT5:
    case GL_LIGHT6:
    case GL_LIGHT7:
        m_tnl.enableLight(cap - GL_LIGHT0, false);
        break;
    case GL_TEXTURE_GEN_S:
        m_tnl.enableTexGenS(false);
        break;
    case GL_TEXTURE_GEN_T:
        m_tnl.enableTexGenT(false);
        break;
    case GL_CULL_FACE:
        m_tnl.enableCulling(false);
        break;
    case GL_COLOR_MATERIAL:
        m_enableColorMaterial = false;
        m_tnl.enableColorMaterial(false, false, false, false);
        break;
    case GL_FOG:
        m_enableFog = false;
        m_error = setFogLut(GL_ZERO, 0, std::numeric_limits<float>::max(), m_fogDensity);
        break;
    default:
        m_error = GL_SPEC_DEVIATION;
        break;
    }
}

void IceGL::glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
{
    m_error = GL_NO_ERROR;
    if (m_renderer.setColorMask(red == GL_TRUE, green == GL_TRUE, blue == GL_TRUE, alpha == GL_TRUE))
    {
        m_error = GL_NO_ERROR;
    }
    else
    {
        m_error = GL_OUT_OF_MEMORY;
    }
}

void IceGL::glDepthMask(GLboolean flag)
{
    m_error = GL_NO_ERROR;
    if (m_renderer.setDepthMask(flag == GL_TRUE))
    {
        m_error = GL_NO_ERROR;
    }
    else
    {
        m_error = GL_OUT_OF_MEMORY;
    }
}

void IceGL::glDepthFunc(GLenum func)
{
    IRenderer::TestFunc testFunc{IRenderer::TestFunc::LESS};
    m_error = GL_NO_ERROR;
    switch (func)
    {
    case GL_ALWAYS:
        testFunc = IRenderer::TestFunc::ALWAYS;
        break;
    case GL_NEVER:
        testFunc = IRenderer::TestFunc::NEVER;
        break;
    case GL_LESS:
        testFunc = IRenderer::TestFunc::LESS;
        break;
    case GL_EQUAL:
        testFunc = IRenderer::TestFunc::EQUAL;
        break;
    case GL_LEQUAL:
        testFunc = IRenderer::TestFunc::LEQUAL;
        break;
    case GL_GREATER:
        testFunc = IRenderer::TestFunc::GREATER;
        break;
    case GL_NOTEQUAL:
        testFunc = IRenderer::TestFunc::NOTEQUAL;
        break;
    case GL_GEQUAL:
        testFunc = IRenderer::TestFunc::GEQUAL;
        break;

    default:
        m_error = GL_INVALID_ENUM;
        break;
    }

    if (m_error == GL_NO_ERROR)
    {
        if (m_renderer.setDepthFunc(testFunc))
        {
            m_error = GL_NO_ERROR;
        }
        else
        {
            m_error = GL_OUT_OF_MEMORY;
        }
    }
}

void IceGL::glAlphaFunc(GLenum func, GLclampf ref)
{
    IRenderer::TestFunc testFunc{IRenderer::TestFunc::LESS};
    m_error = GL_NO_ERROR;
    switch (func)
    {
    case GL_ALWAYS:
        testFunc = IRenderer::TestFunc::ALWAYS;
        break;
    case GL_NEVER:
        testFunc = IRenderer::TestFunc::NEVER;
        break;
    case GL_LESS:
        testFunc = IRenderer::TestFunc::LESS;
        break;
    case GL_EQUAL:
        testFunc = IRenderer::TestFunc::EQUAL;
        break;
    case GL_LEQUAL:
        testFunc = IRenderer::TestFunc::LEQUAL;
        break;
    case GL_GREATER:
        testFunc = IRenderer::TestFunc::GREATER;
        break;
    case GL_NOTEQUAL:
        testFunc = IRenderer::TestFunc::NOTEQUAL;
        break;
    case GL_GEQUAL:
        testFunc = IRenderer::TestFunc::GEQUAL;
        break;

    default:
        m_error = GL_INVALID_ENUM;
        break;
    }

    if (m_error == GL_NO_ERROR)
    {
        m_alphaTestFunc = func;
        m_alphaTestRefValue = ref;
        if (m_enableAlphaTest)
        {
            // Convert reference value from float to fix point
            const uint8_t refFix = ref * (1 << 4);
            if (m_renderer.setAlphaFunc(testFunc, refFix))
            {
                m_error = GL_NO_ERROR;
            }
            else
            {
                m_error = GL_OUT_OF_MEMORY;
            }
        }
    }
}

void IceGL::glTexEnvi(GLenum target, GLenum pname, GLint param)
{
    if ((target == GL_TEXTURE_ENV) && (pname == GL_TEXTURE_ENV_MODE))
    {
        const IRenderer::TexEnvTarget targetConv{IRenderer::TexEnvTarget::TEXTURE_ENV};
        const IRenderer::TexEnvParamName pnameConv{IRenderer::TexEnvParamName::TEXTURE_ENV_MODE};
        IRenderer::TexEnvParam paramConv{IRenderer::TexEnvParam::REPLACE};
        m_error = GL_NO_ERROR;
        switch (param) {
        case GL_DISABLE:
            paramConv = IRenderer::TexEnvParam::DISABLE;
            break;
        case GL_REPLACE:
            paramConv = IRenderer::TexEnvParam::REPLACE;
            break;
        case GL_MODULATE:
            paramConv = IRenderer::TexEnvParam::MODULATE;
            break;
        case GL_DECAL:
            paramConv = IRenderer::TexEnvParam::DECAL;
            break;
        case GL_BLEND:
            paramConv = IRenderer::TexEnvParam::BLEND;
            break;
        case GL_ADD:
            paramConv = IRenderer::TexEnvParam::ADD;
            break;
        default:
            m_error = GL_INVALID_ENUM;
            break;
        }
        if (m_error == GL_NO_ERROR)
        {
            m_texEnvParam = param;
            if (m_enableTextureMapping)
            {
                if (m_renderer.setTexEnv(targetConv, pnameConv, paramConv))
                {
                    m_error = GL_NO_ERROR;
                }
                else
                {
                    m_error = GL_OUT_OF_MEMORY;
                }
            }
        }
    }
    else
    {
        m_error = m_error = GL_INVALID_ENUM;
    }
}

void IceGL::glTexEnvf(GLenum target, GLenum pname, GLfloat param)
{
    glTexEnvi(target, pname, static_cast<GLint>(param));
}

void IceGL::glTexEnvfv(GLenum target, GLenum pname, const GLfloat *param)
{
    if ((target == GL_TEXTURE_ENV) && (pname == GL_TEXTURE_ENV_COLOR))
    {
        if (m_renderer.setTexEnvColor({{static_cast<uint8_t>(param[0] * 255.0f),
                                      static_cast<uint8_t>(param[1] * 255.0f),
                                      static_cast<uint8_t>(param[2] * 255.0f),
                                      static_cast<uint8_t>(param[3] * 255.0f)}}))
        {
            m_error = GL_NO_ERROR;
        }
        else
        {
            m_error = GL_OUT_OF_MEMORY;
        }
    }
    else
    {
        m_error = m_error = GL_INVALID_ENUM;
    }
}

IRenderer::BlendFunc IceGL::convertGlBlendFuncToRenderBlendFunc(const GLenum blendFunc)
{
    switch (blendFunc) {
    case GL_ZERO:
        return IRenderer::BlendFunc::ZERO;
    case GL_ONE:
        return IRenderer::BlendFunc::ONE;
    case GL_DST_COLOR:
        return IRenderer::BlendFunc::DST_COLOR;
    case GL_SRC_COLOR:
        return IRenderer::BlendFunc::SRC_COLOR;
    case GL_ONE_MINUS_DST_COLOR:
        return IRenderer::BlendFunc::ONE_MINUS_DST_COLOR;
    case GL_ONE_MINUS_SRC_COLOR:
        return IRenderer::BlendFunc::ONE_MINUS_SRC_COLOR;
    case GL_SRC_ALPHA:
        return IRenderer::BlendFunc::SRC_ALPHA;
    case GL_ONE_MINUS_SRC_ALPHA:
        return IRenderer::BlendFunc::ONE_MINUS_SRC_ALPHA;
    case GL_DST_ALPHA:
        return IRenderer::BlendFunc::DST_ALPHA;
    case GL_ONE_MINUS_DST_ALPHA:
        return IRenderer::BlendFunc::ONE_MINUS_DST_ALPHA;
    case GL_SRC_ALPHA_SATURATE:
        return IRenderer::BlendFunc::SRC_ALPHA_SATURATE;
    default:
        m_error = GL_INVALID_ENUM;
        return IRenderer::BlendFunc::ZERO;
    }
    m_error = GL_INVALID_ENUM;
    return IRenderer::BlendFunc::ZERO;
}

void IceGL::glBlendFunc(GLenum sfactor, GLenum dfactor)
{
    m_error = GL_NO_ERROR;
    if (sfactor == GL_SRC_ALPHA_SATURATE)
    {
        m_error = GL_INVALID_ENUM;
    }
    else
    {
        m_blendSfactor = sfactor;
        m_blendDfactor = dfactor;
        if (m_enableBlending)
        {
            if (m_renderer.setBlendFunc(convertGlBlendFuncToRenderBlendFunc(sfactor), convertGlBlendFuncToRenderBlendFunc(dfactor)))
            {
                m_error = GL_NO_ERROR;
            }
            else
            {
                m_error = GL_OUT_OF_MEMORY;
            }
        }
    }
}

void IceGL::glLogicOp(GLenum opcode)
{
    IRenderer::LogicOp logicOp{IRenderer::LogicOp::COPY};
    switch (opcode) {
    case GL_CLEAR:
        logicOp = IRenderer::LogicOp::CLEAR;
        break;
    case GL_SET:
        logicOp = IRenderer::LogicOp::SET;
        break;
    case GL_COPY:
        logicOp = IRenderer::LogicOp::COPY;
        break;
    case GL_COPY_INVERTED:
        logicOp = IRenderer::LogicOp::COPY_INVERTED;
        break;
    case GL_NOOP:
        logicOp = IRenderer::LogicOp::NOOP;
        break;
    case GL_INVERTED:
        logicOp = IRenderer::LogicOp::INVERTED;
        break;
    case GL_AND:
        logicOp = IRenderer::LogicOp::AND;
        break;
    case GL_NAND:
        logicOp = IRenderer::LogicOp::NAND;
        break;
    case GL_OR:
        logicOp = IRenderer::LogicOp::OR;
        break;
    case GL_NOR:
        logicOp = IRenderer::LogicOp::NOR;
        break;
    case GL_XOR:
        logicOp = IRenderer::LogicOp::XOR;
        break;
    case GL_EQUIV:
        logicOp = IRenderer::LogicOp::EQUIV;
        break;
    case GL_AND_REVERSE:
        logicOp = IRenderer::LogicOp::AND_REVERSE;
        break;
    case GL_AND_INVERTED:
        logicOp = IRenderer::LogicOp::AND_INVERTED;
        break;
    case GL_OR_REVERSE:
        logicOp = IRenderer::LogicOp::OR_REVERSE;
        break;
    case GL_OR_INVERTED:
        logicOp = IRenderer::LogicOp::OR_INVERTED;
        break;
    default:
        logicOp = IRenderer::LogicOp::COPY;
        break;
    }
    if (m_renderer.setLogicOp(logicOp))
    {
        m_error = GL_NO_ERROR;
    }
    else
    {
        m_error = GL_OUT_OF_MEMORY;
    }
}

void IceGL::glFogf(GLenum pname, GLfloat param)
{
    m_error = GL_NO_ERROR;
    switch (pname) {
    case GL_FOG_MODE:
        m_fogMode = static_cast<GLenum>(param);
        break;
    case GL_FOG_DENSITY:
        if (param >= 0.0f)
            m_fogDensity = param;
        else
            m_error = GL_INVALID_VALUE;
        break;
    case GL_FOG_START:
        m_fogStart = param;
        break;
    case GL_FOG_END:
        m_fogEnd = param;
        break;
    default:
        m_error = GL_INVALID_ENUM;
        break;
    }

    if (m_enableFog && (m_error == GL_NO_ERROR))
    {
        m_error = setFogLut(m_fogMode, m_fogStart, m_fogEnd, m_fogDensity);
    }
}

void IceGL::glFogfv(GLenum pname, const GLfloat *params)
{
    m_error = GL_NO_ERROR;
    switch (pname) {
    case GL_FOG_MODE:
        m_fogMode = static_cast<GLenum>(*params);
        break;
    case GL_FOG_DENSITY:
        if (*params >= 0.0f)
            m_fogDensity = *params;
        else
            m_error = GL_INVALID_VALUE;
        break;
    case GL_FOG_START:
        m_fogStart = *params;
        break;
    case GL_FOG_END:
        m_fogEnd = *params;
        break;
    case GL_FOG_COLOR:
    {
        m_fogColor.fromArray(params, 4);
        Vec4i color;
        color.fromVec(m_fogColor.vec);
        color.mul<0>(255);
        m_renderer.setFogColor({{color.vec[0], color.vec[1], color.vec[2], color.vec[3]}});
    }
        break;
    default:
        m_error = GL_INVALID_ENUM;
        break;
    }

    if (m_enableFog && (m_error == GL_NO_ERROR) && (pname != GL_FOG_COLOR))
    {
        m_error = setFogLut(m_fogMode, m_fogStart, m_fogEnd, m_fogDensity);
    }
}

void IceGL::glFogi(GLenum pname, GLint param)
{
    m_error = GL_NO_ERROR;
    switch (pname) {
    case GL_FOG_MODE:
        m_fogMode = static_cast<GLenum>(param);
        break;
    case GL_FOG_DENSITY:
        if (param >= 0)
            m_fogDensity = static_cast<GLfloat>(param);
        else
            m_error = GL_INVALID_VALUE;
        break;
    case GL_FOG_START:
        m_fogStart = static_cast<GLfloat>(param);
        break;
    case GL_FOG_END:
        m_fogEnd = static_cast<GLfloat>(param);
        break;
    default:
        m_error = GL_INVALID_ENUM;
        break;
    }

    if (m_enableFog && (m_error == GL_NO_ERROR))
    {
        m_error = setFogLut(m_fogMode, m_fogStart, m_fogEnd, m_fogDensity);
    }
}

void IceGL::glFogiv(GLenum pname, const GLint *params)
{
    m_error = GL_NO_ERROR;
    switch (pname) {
    case GL_FOG_MODE:
        m_fogMode = static_cast<GLenum>(*params);
        break;
    case GL_FOG_DENSITY:
        if (*params >= 0)
            m_fogDensity = static_cast<GLfloat>(*params);
        else
            m_error = GL_INVALID_VALUE;
        break;
    case GL_FOG_START:
        m_fogStart = static_cast<GLfloat>(*params);
        break;
    case GL_FOG_END:
        m_fogEnd = static_cast<GLfloat>(*params);
        break;
    case GL_FOG_COLOR:
        m_fogColor.fromArray(params, 4);
        m_fogColor.div(255);
        m_renderer.setFogColor({{params[0], params[1], params[2], params[3]}});
        break;
    default:
        m_error = GL_INVALID_ENUM;
        break;
    }

    if (m_enableFog && (m_error == GL_NO_ERROR) && (pname != GL_FOG_COLOR))
    {
        m_error = setFogLut(m_fogMode, m_fogStart, m_fogEnd, m_fogDensity);
    }
}

void IceGL::glMaterialf(GLenum face, GLenum pname, GLfloat param)
{
    m_error = GL_INVALID_ENUM;
    if (face == GL_FRONT_AND_BACK)
    {
        if ((pname == GL_SHININESS) && (param >= 0.0f) && (param <= 128.0f))
        {
            m_tnl.setSpecularExponentMaterial(param);
            m_error = GL_NO_ERROR;
        }
    }
}

void IceGL::glMaterialfv(GLenum face, GLenum pname, const GLfloat *params)
{
    m_error = GL_INVALID_ENUM;
    if (face == GL_FRONT_AND_BACK)
    {
        m_error = GL_NO_ERROR;
        switch (pname) {
        case GL_AMBIENT:
            m_tnl.setAmbientColorMaterial({params});
            break;
        case GL_DIFFUSE:
            m_tnl.setDiffuseColorMaterial({params});
            break;
        case GL_AMBIENT_AND_DIFFUSE:
            m_tnl.setAmbientColorMaterial({params});
            m_tnl.setDiffuseColorMaterial({params});
            break;
        case GL_SPECULAR:
            m_tnl.setSpecularColorMaterial({params});
            break;
        case GL_EMISSION:
            m_tnl.setEmissiveColorMaterial({params});
            break;
        default:
            glMaterialf(face, pname, params[0]);
            break;
        }
    }
}

void IceGL::glColorMaterial(GLenum face, GLenum pname)
{
    m_error = GL_INVALID_ENUM;
    if (face == GL_FRONT_AND_BACK)
    {
        m_error = GL_NO_ERROR;
        m_colorMaterialFace = face;
        m_colorMaterialTracking = pname;
        switch (pname) {
        case GL_AMBIENT:
            if (m_enableColorMaterial)
                m_tnl.enableColorMaterial(false, true, false, false);
            break;
        case GL_DIFFUSE:
            if (m_enableColorMaterial)
                m_tnl.enableColorMaterial(false, false, true, false);
            break;
        case GL_AMBIENT_AND_DIFFUSE:
            if (m_enableColorMaterial)
                m_tnl.enableColorMaterial(false, true, true, false);
            break;
        case GL_SPECULAR:
            if (m_enableColorMaterial)
                m_tnl.enableColorMaterial(false, false, false, true);
            break;
        case GL_EMISSION:
            if (m_enableColorMaterial)
                m_tnl.enableColorMaterial(true, false, false, false);
            break;
        default:
            m_error = GL_INVALID_ENUM;
            m_colorMaterialTracking = GL_AMBIENT_AND_DIFFUSE;
            if (m_enableColorMaterial)
                m_tnl.enableColorMaterial(false, true, true, false);
            break;
        }
    }
}

void IceGL::glLightf(GLenum light, GLenum pname, GLfloat param)
{
    m_error = GL_INVALID_ENUM;
    if (light > GL_LIGHT7)
        return;

    m_error = GL_NO_ERROR;
    switch (pname) {
    case GL_SPOT_EXPONENT:
        m_error = GL_SPEC_DEVIATION;
        break;
    case GL_SPOT_CUTOFF:
        m_error = GL_SPEC_DEVIATION;
        break;
    case GL_CONSTANT_ATTENUATION:
        m_tnl.setConstantAttenuationLight(light - GL_LIGHT0, param);
        break;
    case GL_LINEAR_ATTENUATION:
        m_tnl.setLinearAttenuationLight(light - GL_LIGHT0, param);
        break;
    case GL_QUADRATIC_ATTENUATION:
        m_tnl.setQuadraticAttenuationLight(light - GL_LIGHT0, param);
        break;
    default:
        m_error = GL_INVALID_ENUM;
        break;
    }
}

void IceGL::glLightfv(GLenum light, GLenum pname, const GLfloat *params)
{
    m_error = GL_INVALID_ENUM;
    if (light > GL_LIGHT7)
        return;

    m_error = GL_NO_ERROR;
    switch (pname) {
    case GL_AMBIENT:
        m_tnl.setAmbientColorLight(light - GL_LIGHT0, {params});
        break;
    case GL_DIFFUSE:
        m_tnl.setDiffuseColorLight(light - GL_LIGHT0, {params});
        break;
    case GL_SPECULAR:
        m_tnl.setSpecularColorLight(light - GL_LIGHT0, {params});
        break;
    case GL_POSITION:
        recalculateAndSetTnLMatrices();
        m_tnl.setPosLight(light - GL_LIGHT0, {params});
        break;
    case GL_SPOT_DIRECTION:
        m_error = GL_SPEC_DEVIATION;
        break;
    default:
        glLightf(light, pname, params[0]);
        break;
    }

}

void IceGL::glLightModelf(GLenum pname, GLfloat param)
{
    (void)pname;
    (void)param;
    // Only GL_LIGHT_MODEL_TWO_SIDE has to be supported.
    // Basically this is normal lighting where we enable lighting on the back face of a polygon. But that includes
    // To determine which polygon is on the back side (which can be calculatet with the equation 2.6 of the OpenGL 1.5 spec),
    // and also to converte the normal n to a negative -n. But that adds additional calculations which will probebly be too
    // heavy for a small CPU so that this will be never used.
    m_error = GL_SPEC_DEVIATION;
}

void IceGL::glLightModelfv(GLenum pname, const GLfloat *params)
{
    m_error = GL_INVALID_ENUM;
    if (pname == GL_LIGHT_MODEL_AMBIENT)
    {
        m_tnl.setAmbientColorScene({params});
        m_error = GL_NO_ERROR;
    }
    else
    {
        glLightModelf(pname, params[0]);
    }
}

void IceGL::glVertexPointer(GLint size, GLenum type, GLsizei stride, const void *pointer)
{
    m_renderObj.vertexSize = size;
    m_renderObj.vertexType = convertType(type);
    m_renderObj.vertexStride = stride;
    m_renderObj.vertexPointer = pointer;
}

void IceGL::glTexCoordPointer(GLint size, GLenum type, GLsizei stride, const void *pointer)
{
    m_renderObj.texCoordSize = size;
    m_renderObj.texCoordType = convertType(type);
    m_renderObj.texCoordStride = stride;
    m_renderObj.texCoordPointer = pointer;
}

void IceGL::glNormalPointer(GLenum type, GLsizei stride, const void *pointer)
{
    m_renderObj.normalType = convertType(type);
    m_renderObj.normalStride = stride;
    m_renderObj.normalPointer = pointer;
}

void IceGL::glColorPointer(GLint size, GLenum type, GLsizei stride, const void *pointer)
{
    m_renderObj.colorSize = size;
    m_renderObj.colorType = convertType(type);
    m_renderObj.colorStride = stride;
    m_renderObj.colorPointer = pointer;
}

void IceGL::glDrawArrays(GLenum mode, GLint first, GLsizei count)
{
    recalculateAndSetTnLMatrices();

    m_renderObj.count = count;
    m_renderObj.arrayOffset = first;
    m_renderObj.drawMode = convertDrawMode(mode);
    m_renderObj.indicesEnabled = false;
    m_renderObj.vertexColor = m_vertexColor;

    if (m_error == GL_NO_ERROR)
    {
        m_tnl.drawObj(m_renderer, m_renderObj);
    }
}

void IceGL::glDrawElements(GLenum mode, GLsizei count, GLenum type, const void *indices)
{
    m_error = GL_NO_ERROR;
    switch (type) {
    case GL_UNSIGNED_BYTE:
        m_renderObj.indicesType = TnL::RenderObj::Type::BYTE;
        break;
    case GL_UNSIGNED_SHORT:
        m_renderObj.indicesType = TnL::RenderObj::Type::SHORT;
        break;
    case GL_UNSIGNED_INT:
        m_renderObj.indicesType = TnL::RenderObj::Type::UNSIGNED_INT;
        break;
    default:
        m_error = GL_INVALID_ENUM;
        return;
    }

    recalculateAndSetTnLMatrices();

    m_renderObj.count = count;
    m_renderObj.drawMode = convertDrawMode(mode);
    m_renderObj.indicesPointer = indices;
    m_renderObj.indicesEnabled = true;
    m_renderObj.vertexColor = m_vertexColor;

    if (m_error == GL_NO_ERROR)
    {
        m_tnl.drawObj(m_renderer, m_renderObj);
    }
}

void IceGL::glEnableClientState(GLenum array)
{
    setClientState(array, true);
}

void IceGL::glDisableClientState(GLenum array)
{
    setClientState(array, false);
}

void IceGL::setClientState(const GLenum array, bool enable)
{
    m_error = GL_NO_ERROR;
    switch (array) {
    case GL_COLOR_ARRAY:
        m_renderObj.colorArrayEnabled = enable;
        break;
    case GL_NORMAL_ARRAY:
        m_renderObj.normalArrayEnabled = enable;
        break;
    case GL_TEXTURE_COORD_ARRAY:
        m_renderObj.texCoordArrayEnabled = enable;
        break;
    case GL_VERTEX_ARRAY:
        m_renderObj.vertexArrayEnabled = enable;
        break;
    default:
        m_error = GL_INVALID_ENUM;
        break;
    }
}

TnL::RenderObj::Type IceGL::convertType(GLenum type)
{
    switch (type) {
    case GL_BYTE:
        return TnL::RenderObj::Type::BYTE;
    case GL_UNSIGNED_BYTE:
        return TnL::RenderObj::Type::UNSIGNED_BYTE;
    case GL_SHORT:
        return TnL::RenderObj::Type::SHORT;
    case GL_UNSIGNED_SHORT:
        return TnL::RenderObj::Type::UNSIGNED_SHORT;
    case GL_FLOAT:
        return TnL::RenderObj::Type::FLOAT;
    case GL_UNSIGNED_INT:
        return TnL::RenderObj::Type::UNSIGNED_INT;
    default:
        m_error = GL_INVALID_ENUM;
        return TnL::RenderObj::Type::BYTE;
    }
}

TnL::RenderObj::DrawMode IceGL::convertDrawMode(GLenum drawMode)
{
    switch (drawMode) {
    case GL_TRIANGLES:
        return TnL::RenderObj::DrawMode::TRIANGLES;
    case GL_TRIANGLE_FAN:
        return TnL::RenderObj::DrawMode::TRIANGLE_FAN;
    case GL_TRIANGLE_STRIP:
        return TnL::RenderObj::DrawMode::TRIANGLE_STRIP;
    default:
        m_error = GL_INVALID_ENUM;
        return TnL::RenderObj::DrawMode::TRIANGLES;
    }
}

void IceGL::glPushMatrix()
{
    m_error = GL_NO_ERROR;
    if (matrixMode == GL_MODELVIEW)
    {
        if (m_mStackIndex < MODEL_MATRIX_STACK_DEPTH)
        {
            m_mStack[m_mStackIndex] = m_m;
            m_mStackIndex++;
            m_matricesOutdated = true;
        }
        else
        {
            m_error = GL_STACK_OVERFLOW;
        }
    }
    else if (matrixMode == GL_PROJECTION)
    {
        if (m_pStackIndex < PROJECTION_MATRIX_STACK_DEPTH)
        {
            m_pStack[m_pStackIndex] = m_p;
            m_pStackIndex++;
            m_matricesOutdated = true;
        }
        else
        {
            m_error = GL_STACK_OVERFLOW;
        }
    }
    else
    {
        m_error = GL_INVALID_ENUM;
    }
}

void IceGL::glPopMatrix()
{
    m_error = GL_NO_ERROR;
    if (matrixMode == GL_MODELVIEW)
    {
        if (m_mStackIndex > 0)
        {
            m_mStackIndex--;
            m_m = m_mStack[m_mStackIndex];
            m_matricesOutdated = true;
        }
        else
        {
            m_error = GL_STACK_UNDERFLOW;
        }
    }
    else if (matrixMode == GL_PROJECTION)
    {
        if (m_pStackIndex > 0)
        {
            m_pStackIndex--;
            m_p = m_pStack[m_pStackIndex];
            m_matricesOutdated = true;
        }
        else
        {
            m_error = GL_STACK_UNDERFLOW;
        }
    }
    else
    {
        m_error = GL_INVALID_ENUM;
    }
}

void IceGL::recalculateAndSetTnLMatrices()
{
    if (m_matricesOutdated)
    {
        // Update transformation matrix
        Mat44 t{m_m};
        t *= m_p;
        m_tnl.setModelProjectionMatrix(t);
        m_tnl.setModelMatrix(m_m);

        Mat44 inv{m_m};
        inv.invert();
        inv.transpose();
        // Use the inverse transpose matrix for the normals. This is the standard way how OpenGL transforms normals
        m_tnl.setNormalMatrix(inv);

        m_matricesOutdated = false;
    }
}

void IceGL::glTexGeni(GLenum coord, GLenum pname, GLint param)
{
    m_error = GL_NO_ERROR;
    TnL::TexGenMode mode;
    switch (param) {
    case GL_OBJECT_LINEAR:
        mode = TnL::TexGenMode::OBJECT_LINEAR;
        break;
    case GL_EYE_LINEAR:
        mode = TnL::TexGenMode::EYE_LINEAR;
        break;
    case GL_SPHERE_MAP:
        mode = TnL::TexGenMode::SPHERE_MAP;
        break;
    default:
        m_error = GL_INVALID_ENUM;
        break;
    }

    if (m_error == GL_NO_ERROR && pname == GL_TEXTURE_GEN_MODE)
    {
        switch (coord) {
        case GL_S:
            m_tnl.setTexGenModeS(mode);
            break;
        case GL_T:
            m_tnl.setTexGenModeT(mode);
            break;
        default:
            // Normally GL_R and GL_Q wouldn't rise an invalid enum, but they are right now not implemented
            m_error = GL_INVALID_ENUM;
            break;
        }
    }
    else
    {
        m_error = GL_INVALID_ENUM;
    }
}

void IceGL::glTexGenfv(GLenum coord, GLenum pname, const GLfloat *param)
{
    m_error = GL_NO_ERROR;

    switch (pname) {
    case GL_OBJECT_PLANE:
        if (coord == GL_S)
            m_tnl.setTexGenVecObjS({param});
        else if (coord == GL_T)
            m_tnl.setTexGenVecObjT({param});
        else
            m_error = GL_INVALID_ENUM;
        break;
    case GL_EYE_PLANE:
        if (coord == GL_S)
            m_tnl.setTexGenVecEyeS(calcTexGenEyePlane(m_m, {param}));
        else if (coord == GL_T)
            m_tnl.setTexGenVecEyeT(calcTexGenEyePlane(m_m, {param}));
        else
            m_error = GL_INVALID_ENUM;
        break;
    default:
        glTexGeni(coord, pname, static_cast<GLint>(param[0]));
        break;
    }
}

Vec4 IceGL::calcTexGenEyePlane(const Mat44& mat, const Vec4& plane)
{
    Mat44 inv{mat};
    inv.invert();
    inv.transpose();
    Vec4 newPlane;
    inv.transform(newPlane, plane);
    return newPlane;
}

GLenum IceGL::setFogLut(GLenum mode, float start, float end, float density)
{
    // TODO: Check if the required fog values have changed before calculating a new fog table.
    // For instance, is mode is set to same value, dont calculate fog. If density is set on GL_LINEAR, dont calculate fog
    std::function <float(float)> fogFunction;

    // Set fog function
    switch (mode) {
    case GL_LINEAR:
        fogFunction = [&](float z) {
            float f = (end - z) / (end - start);
            return f;
        };
        break;
    case GL_EXP:
        fogFunction = [&](float z) {
            float f = expf(-(density * z));
            return f;
        };
        break;
    case GL_EXP2:
        fogFunction = [&](float z) {
            float f = expf(powf(-(density * z), 2));
            return f;
        };
        break;
    default:
        fogFunction = [](float) {
            return 1.0f;
        };
        break;
    }

    // Calculate fog LUT
    std::array<float, 33> lut;
    for (std::size_t i = 0; i < lut.size(); i++)
    {
        float f = fogFunction(powf(2, i));
        lut[i] = f;
    }

    // Set fog LUT
    if (m_renderer.setFogLut(lut, start, end))
    {
        return GL_NO_ERROR;
    }
    return GL_OUT_OF_MEMORY;
}

void IceGL::glOrthof(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat near, GLfloat far)
{
    m_error = GL_NO_ERROR;
    if ((near == far) || (left == right) || (top == bottom))
    {
        m_error = GL_INVALID_VALUE;
        return;
    }

    Mat44 o
    {{{
                {2.0f / (right - left),              0.0f,                               0.0f,                           0.0f}, // Col 0
                {0.0f,                               2.0f / (top - bottom),              0.0f,                           0.0f}, // Col 1
                {0.0f,                               0.0f,                               -2.0f / (far - near),           0.0f}, // Col 2
                {-((right + left) / (right - left)), -((top + bottom) / (top - bottom)), -((far + near) / (far - near)), 1.0f}  // Col 3
            }}};
    glMultMatrixf(&o[0][0]);
}

void IceGL::glVertex2f(GLfloat x, GLfloat y)
{
    glVertex3f(x, y, 0);
}

void IceGL::glRect(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2)
{
    glBegin(GL_TRIANGLE_FAN); // The spec says, use GL_POLYGON. GL_POLYGON is not implemented. GL_TRIANGLE_FAN should also do the trick.
    glVertex2f(x1, y1);
    glVertex2f(x2, y1);
    glVertex2f(x2, y2);
    glVertex2f(x1, y2);
    glEnd();
}

const GLubyte *IceGL::glGetString(GLenum name)
{
    switch (name) {
    case GL_VENDOR:
        return reinterpret_cast<const GLubyte*>("ToNi3141");
    case GL_RENDERER:
        return reinterpret_cast<const GLubyte*>("RasteriCEr");
    case GL_VERSION:
        return reinterpret_cast<const GLubyte*>("0.1");
    case GL_EXTENSIONS:
        return reinterpret_cast<const GLubyte*>("");
    default:
        break;
    }
    return reinterpret_cast<const GLubyte*>("");
}

void IceGL::glGetIntegerv(GLenum pname, GLint *params)
{
    switch (pname) {
    case GL_MAX_LIGHTS:
        *params = m_tnl.MAX_LIGHTS;
        break;
    case GL_MAX_MODELVIEW_STACK_DEPTH:
        *params = MODEL_MATRIX_STACK_DEPTH;
        break;
    case GL_MAX_PROJECTION_STACK_DEPTH:
        *params = PROJECTION_MATRIX_STACK_DEPTH;
        break;
    case GL_MAX_TEXTURE_SIZE:
        *params = 128;
        break;
    case GL_DOUBLEBUFFER:
        *params = 1;
        break;
    case GL_RED_BITS:
    case GL_GREEN_BITS:
    case GL_BLUE_BITS:
    case GL_ALPHA_BITS:
        *params = 4;
        break;
    case GL_DEPTH_BITS:
        *params = 16;
        break;
    case GL_STENCIL_BITS:
        *params = 0;
        break;
    default:
        *params = 0;
        break;
    }
}

void IceGL::glGetFloatv(GLenum pname, GLfloat *params)
{
    GLint tmp;
    glGetIntegerv(pname, &tmp);
    *params = tmp;
}

void IceGL::glGetBooleanv(GLenum pname, GLboolean *params)
{
    GLint tmp;
    glGetIntegerv(pname, &tmp);
    *params = tmp;
}

void IceGL::glCullFace(GLenum mode)
{
    m_error = GL_NO_ERROR;
    switch (mode) {
    case GL_BACK:
        m_tnl.setCullMode(TnL::CullMode::BACK);
        break;
    case GL_FRONT:
        m_tnl.setCullMode(TnL::CullMode::FRONT);
        break;
    case GL_FRONT_AND_BACK:
        m_tnl.setCullMode(TnL::CullMode::FRONT_AND_BACK);
        break;
    default:
        m_error = GL_INVALID_ENUM;
        break;
    }
}

IRenderer::TextureWrapMode IceGL::convertGlTextureWrapMode(const GLenum mode)
{
    switch (mode) {
    case GL_CLAMP_TO_EDGE:
        return IRenderer::TextureWrapMode::CLAMP_TO_EDGE;
    case GL_REPEAT:
        return IRenderer::TextureWrapMode::REPEAT;
    default:
        m_error = GL_INVALID_ENUM;
        return IRenderer::TextureWrapMode::REPEAT;
    }
}

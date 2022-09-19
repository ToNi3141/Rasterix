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

#include "IceGL.hpp"
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

IceGL::IceGL(IRenderer &renderer)
    : m_renderer(renderer)
    , m_vertexPipeline(renderer)
{
    // Preallocate the first texture. This is the default texture and it also can't be deleted.
    m_renderer.createTexture();

    //glLogicOp(GL_COPY);
    glDisable(GL_FOG);
    glClearDepthf(1.0f);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    float fogColor[] = {0.0f, 0.0f, 0.0f, 0.0f};
    glFogfv(GL_FOG_COLOR, fogColor);
}

void IceGL::commit()
{
    m_renderer.commit();
}

void IceGL::glMatrixMode(GLenum mm)
{
    m_error = GL_NO_ERROR;
    if (mm == GL_MODELVIEW)
    {
        m_vertexPipeline.setMatrixMode(VertexPipeline::MatrixMode::MODELVIEW);
    }
    else if (mm == GL_PROJECTION)
    {
        m_vertexPipeline.setMatrixMode(VertexPipeline::MatrixMode::PROJECTION);
    }
    else
    {
        m_error = GL_INVALID_ENUM;
    }
}

void IceGL::glMultMatrixf(const GLfloat* m)
{
    const Mat44 *m44 = reinterpret_cast<const Mat44*>(m);
    m_vertexPipeline.multiply(*m44);
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
    m_vertexPipeline.translate(x, y, z);
}

void IceGL::glScalef(GLfloat x, GLfloat y, GLfloat z)
{
    m_vertexPipeline.scale(x, y, z);
}

void IceGL::glRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
    m_vertexPipeline.rotate(angle, x, y, z);
}

void IceGL::glLoadIdentity()
{
    m_vertexPipeline.loadIdentity();
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
    m_vertexPipeline.setViewport(x, y, width, height);
}

void IceGL::glDepthRange(GLclampd zNear, GLclampd zFar)
{
    if (zNear > 1.0f) zNear = 1.0f;
    if (zNear < -1.0f) zNear = -1.0f;
    if (zFar  > 1.0f) zFar  = 1.0f;
    if (zFar  < -1.0f) zFar  = -1.0f;

    m_vertexPipeline.setDepthRange(zNear, zFar);
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
    m_renderObjBeginEnd.enableVertexArray(!m_vertexBuffer.empty());
    m_renderObjBeginEnd.setVertexSize(4);
    m_renderObjBeginEnd.setVertexType(RenderObj::Type::FLOAT);
    m_renderObjBeginEnd.setVertexStride(0);
    m_renderObjBeginEnd.setVertexPointer(m_vertexBuffer.data());

    m_renderObjBeginEnd.enableTexCoordArray(!m_textureVertexBuffer.empty());
    m_renderObjBeginEnd.setTexCoordSize(2);
    m_renderObjBeginEnd.setTexCoordType(RenderObj::Type::FLOAT);
    m_renderObjBeginEnd.setTexCoordStride(0);
    m_renderObjBeginEnd.setTexCoordPointer(m_textureVertexBuffer.data());

    m_renderObjBeginEnd.enableNormalArray(!m_normalVertexBuffer.empty());
    m_renderObjBeginEnd.setNormalType(RenderObj::Type::FLOAT);
    m_renderObjBeginEnd.setNormalStride(0);
    m_renderObjBeginEnd.setNormalPointer(m_normalVertexBuffer.data());

    m_renderObjBeginEnd.enableColorArray(!m_colorVertexBuffer.empty());
    m_renderObjBeginEnd.setColorSize(4);
    m_renderObjBeginEnd.setColorType(RenderObj::Type::FLOAT);
    m_renderObjBeginEnd.setColorStride(0);
    m_renderObjBeginEnd.setColorPointer(m_colorVertexBuffer.data());

    m_renderObjBeginEnd.enableIndices(false);
    m_renderObjBeginEnd.setDrawMode(convertDrawMode(m_beginMode));
    m_renderObjBeginEnd.setCount(m_vertexBuffer.size());


    m_vertexPipeline.drawObj(m_renderObjBeginEnd);
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
                    || (level > MAX_TEX_SIZE)*/ // Level is right now not supported
            (width > MAX_TEX_SIZE)
            || (height > MAX_TEX_SIZE)
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
            || format == GL_LUMINANCE_ALPHA)
    {
        m_error = GL_SPEC_DEVIATION;
        return;
    }

    // It can happen, that a not power of two texture is used. This little hack allows that the texture can sill be used
    // without crashing the software. But it is likely that it will produce graphical errors.
    const uint16_t widthRounded = pow(2, ceil(log(width)/log(2)));
    const uint16_t heightRounded = pow(2, ceil(log(height)/log(2)));
    
    std::shared_ptr<uint16_t> texMemShared(new uint16_t[(widthRounded * heightRounded * 2)], [] (const uint16_t *p) { delete [] p; });
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

        if (!m_renderer.updateTexture(m_boundTexture, 
                                      texMemShared, 
                                      widthRounded,
                                      heightRounded,
                                      m_texWrapModeS,
                                      m_texWrapModeT,
                                      m_texEnableMagFilter))
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
        m_renderer.useTexture(IRenderer::TMU::TMU0, m_boundTexture);
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
        switch (pname) {
        case GL_TEXTURE_WRAP_S:
        {
            auto mode = convertGlTextureWrapMode(static_cast<GLenum>(param));
            if (m_error == GL_NO_ERROR)
            {
                m_texWrapModeS = mode;
            }
            break;
        }
        case GL_TEXTURE_WRAP_T:
        {
            auto mode = convertGlTextureWrapMode(static_cast<GLenum>(param));
            if (m_error == GL_NO_ERROR)
            {
                m_texWrapModeT = mode;
            }
            break;
        }
            break;
        case GL_TEXTURE_MAG_FILTER:
            if ((param == GL_LINEAR) || (param == GL_NEAREST))
            {
                m_texEnableMagFilter = (param == GL_LINEAR);
            }
            else
            {
                m_error = GL_INVALID_ENUM;
            }
            break;
        default:
            m_error = GL_INVALID_ENUM;
            break;
        }
    }
    else
    {
        // TODO: Implement other modes like GL_TEXTURE_MIN_FILTER and so on, but they require hardware support (which is not implemented).
        m_error = GL_INVALID_ENUM;
    }
}

void IceGL::glTexParameterf(GLenum target, GLenum pname, GLfloat param) 
{
    glTexParameteri(target, pname, static_cast<GLint>(param));
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
        m_featureEnableConf.tmu0 = true;
        m_renderer.setFeatureEnableConfig(m_featureEnableConf);
        break;
    case GL_ALPHA_TEST:
        // Note: The alpha test disabling and enabling is a bit special. If the alpha test is disabled, we are just saving
        // the current alpha test func and the reference value. When the test is enabled, we just recover this values.
        // Seems to work perfect.
        m_featureEnableConf.alphaTest = true;
        m_renderer.setFeatureEnableConfig(m_featureEnableConf);
        break;
    case GL_DEPTH_TEST:
        // For the depth test it is not possible to use a similar algorithm like we do for the alpha test. The main reason
        // behind that is that the depth test uses a depth buffer. Is the depth test is disabled, the depth test always passes
        // and never writes into the depth buffer. We could use glDepthMask to avoid writing, but then we also disabling
        // glClear() what we obviously dont want. The easiest fix is to introduce a special switch. If this switch is disabled
        // the depth buffer always passes and never writes and glClear() can clear the depth buffer.
        m_featureEnableConf.depthTest = true;
        m_renderer.setFeatureEnableConfig(m_featureEnableConf);
        break;
    case GL_BLEND:
        m_featureEnableConf.blending = true;
        m_renderer.setFeatureEnableConfig(m_featureEnableConf);
        break;
    case GL_LIGHTING:
        m_vertexPipeline.getLighting().enableLighting(true);
        break;
    case GL_LIGHT0:
    case GL_LIGHT1:
    case GL_LIGHT2:
    case GL_LIGHT3:
    case GL_LIGHT4:
    case GL_LIGHT5:
    case GL_LIGHT6:
    case GL_LIGHT7:
        m_vertexPipeline.getLighting().enableLight(cap - GL_LIGHT0, true);
        break;
    case GL_TEXTURE_GEN_S:
        m_vertexPipeline.getTexGen().enableTexGenS(true);
        break;
    case GL_TEXTURE_GEN_T:
        m_vertexPipeline.getTexGen().enableTexGenT(true);
        break;
    case GL_CULL_FACE:
        m_vertexPipeline.enableCulling(true);
        break;
    case GL_COLOR_MATERIAL:
        m_vertexPipeline.enableColorMaterial(true);
        break;
    case GL_FOG:
        m_featureEnableConf.fog = true;
        m_renderer.setFeatureEnableConfig(m_featureEnableConf);
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
        m_featureEnableConf.tmu0 = false;
        m_renderer.setFeatureEnableConfig(m_featureEnableConf);
        break;
    }
    case GL_ALPHA_TEST:
    {
        m_featureEnableConf.alphaTest = false;
        m_renderer.setFeatureEnableConfig(m_featureEnableConf);
        break;
    }
    case GL_DEPTH_TEST:
    {
        m_featureEnableConf.depthTest = false;
        m_renderer.setFeatureEnableConfig(m_featureEnableConf);
        break;
    }
    case GL_BLEND:
    {
        m_featureEnableConf.blending = false;
        m_renderer.setFeatureEnableConfig(m_featureEnableConf);
        break;
    }
    case GL_LIGHTING:
        m_vertexPipeline.getLighting().enableLighting(false);
        break;
    case GL_LIGHT0:
    case GL_LIGHT1:
    case GL_LIGHT2:
    case GL_LIGHT3:
    case GL_LIGHT4:
    case GL_LIGHT5:
    case GL_LIGHT6:
    case GL_LIGHT7:
        m_vertexPipeline.getLighting().enableLight(cap - GL_LIGHT0, false);
        break;
    case GL_TEXTURE_GEN_S:
        m_vertexPipeline.getTexGen().enableTexGenS(false);
        break;
    case GL_TEXTURE_GEN_T:
        m_vertexPipeline.getTexGen().enableTexGenT(false);
        break;
    case GL_CULL_FACE:
        m_vertexPipeline.enableCulling(false);
        break;
    case GL_COLOR_MATERIAL:
        m_vertexPipeline.enableColorMaterial(false);
        break;
    case GL_FOG:
        m_featureEnableConf.fog = false;
        m_renderer.setFeatureEnableConfig(m_featureEnableConf);
        break;
    default:
        m_error = GL_SPEC_DEVIATION;
        break;
    }
}

void IceGL::glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
{
    m_error = GL_NO_ERROR;
    m_fragmentPipelineConf.colorMaskR = (red == GL_TRUE);
    m_fragmentPipelineConf.colorMaskG = (green == GL_TRUE);
    m_fragmentPipelineConf.colorMaskB = (blue == GL_TRUE);
    m_fragmentPipelineConf.colorMaskA = (alpha == GL_TRUE);
    if (m_renderer.setFragmentPipelineConfig(m_fragmentPipelineConf))
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
    m_fragmentPipelineConf.depthMask = (flag == GL_TRUE);
    if (m_renderer.setFragmentPipelineConfig(m_fragmentPipelineConf))
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
    IRenderer::FragmentPipelineConf::TestFunc testFunc{IRenderer::FragmentPipelineConf::TestFunc::LESS};
    m_error = GL_NO_ERROR;
    switch (func)
    {
    case GL_ALWAYS:
        testFunc = IRenderer::FragmentPipelineConf::FragmentPipelineConf::TestFunc::ALWAYS;
        break;
    case GL_NEVER:
        testFunc = IRenderer::FragmentPipelineConf::FragmentPipelineConf::TestFunc::NEVER;
        break;
    case GL_LESS:
        testFunc = IRenderer::FragmentPipelineConf::TestFunc::LESS;
        break;
    case GL_EQUAL:
        testFunc = IRenderer::FragmentPipelineConf::TestFunc::EQUAL;
        break;
    case GL_LEQUAL:
        testFunc = IRenderer::FragmentPipelineConf::TestFunc::LEQUAL;
        break;
    case GL_GREATER:
        testFunc = IRenderer::FragmentPipelineConf::TestFunc::GREATER;
        break;
    case GL_NOTEQUAL:
        testFunc = IRenderer::FragmentPipelineConf::TestFunc::NOTEQUAL;
        break;
    case GL_GEQUAL:
        testFunc = IRenderer::FragmentPipelineConf::TestFunc::GEQUAL;
        break;

    default:
        m_error = GL_INVALID_ENUM;
        break;
    }

    if (m_error == GL_NO_ERROR)
    {
        m_fragmentPipelineConf.depthFunc = testFunc;
        if (m_renderer.setFragmentPipelineConfig(m_fragmentPipelineConf))
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
    IRenderer::FragmentPipelineConf::TestFunc testFunc{IRenderer::FragmentPipelineConf::TestFunc::LESS};
    m_error = GL_NO_ERROR;
    switch (func)
    {
    case GL_ALWAYS:
        testFunc = IRenderer::FragmentPipelineConf::TestFunc::ALWAYS;
        break;
    case GL_NEVER:
        testFunc = IRenderer::FragmentPipelineConf::TestFunc::NEVER;
        break;
    case GL_LESS:
        testFunc = IRenderer::FragmentPipelineConf::TestFunc::LESS;
        break;
    case GL_EQUAL:
        testFunc = IRenderer::FragmentPipelineConf::TestFunc::EQUAL;
        break;
    case GL_LEQUAL:
        testFunc = IRenderer::FragmentPipelineConf::TestFunc::LEQUAL;
        break;
    case GL_GREATER:
        testFunc = IRenderer::FragmentPipelineConf::TestFunc::GREATER;
        break;
    case GL_NOTEQUAL:
        testFunc = IRenderer::FragmentPipelineConf::TestFunc::NOTEQUAL;
        break;
    case GL_GEQUAL:
        testFunc = IRenderer::FragmentPipelineConf::TestFunc::GEQUAL;
        break;

    default:
        m_error = GL_INVALID_ENUM;
        break;
    }

    if (m_error == GL_NO_ERROR)
    {
        // Convert reference value from float to fix point
        uint8_t refFix = ref * (1 << 8);
        if (ref >= 1.0f)
        {
            refFix = 0xff;
        }
        m_fragmentPipelineConf.alphaFunc = testFunc;
        m_fragmentPipelineConf.referenceAlphaValue = refFix;
        if (m_renderer.setFragmentPipelineConfig(m_fragmentPipelineConf))
        {
            m_error = GL_NO_ERROR;
        }
        else
        {
            m_error = GL_OUT_OF_MEMORY;
        }
    }
}

GLint IceGL::convertTexEnvMode(IRenderer::TexEnvConf& texEnvConf, GLint param) 
{
    GLint ret = GL_NO_ERROR;
    switch (param) {
    case GL_DISABLE:
        texEnvConf.combineRgb = IRenderer::TexEnvConf::Combine::REPLACE;
        texEnvConf.combineAlpha = IRenderer::TexEnvConf::Combine::REPLACE;
        texEnvConf.srcRegRgb0 = IRenderer::TexEnvConf::SrcReg::PRIMARY_COLOR;
        texEnvConf.srcRegAlpha0 = IRenderer::TexEnvConf::SrcReg::PRIMARY_COLOR;
        break;
    case GL_REPLACE:
        texEnvConf.combineRgb = IRenderer::TexEnvConf::Combine::REPLACE;
        texEnvConf.combineAlpha = IRenderer::TexEnvConf::Combine::REPLACE;
        texEnvConf.srcRegRgb0 = IRenderer::TexEnvConf::SrcReg::TEXTURE;
        texEnvConf.srcRegAlpha0 = IRenderer::TexEnvConf::SrcReg::TEXTURE;
        break;
    case GL_MODULATE:
        texEnvConf.combineRgb = IRenderer::TexEnvConf::Combine::MODULATE;
        texEnvConf.combineAlpha = IRenderer::TexEnvConf::Combine::MODULATE;
        texEnvConf.srcRegRgb0 = IRenderer::TexEnvConf::SrcReg::TEXTURE;
        texEnvConf.srcRegRgb1 = IRenderer::TexEnvConf::SrcReg::PRIMARY_COLOR;
        texEnvConf.srcRegAlpha0 = IRenderer::TexEnvConf::SrcReg::TEXTURE;
        texEnvConf.srcRegAlpha1 = IRenderer::TexEnvConf::SrcReg::PRIMARY_COLOR;
        break;
    case GL_DECAL:
        texEnvConf.combineRgb = IRenderer::TexEnvConf::Combine::INTERPOLATE;
        texEnvConf.combineAlpha = IRenderer::TexEnvConf::Combine::REPLACE;
        texEnvConf.srcRegRgb0 = IRenderer::TexEnvConf::SrcReg::TEXTURE;
        texEnvConf.srcRegRgb1 = IRenderer::TexEnvConf::SrcReg::PRIMARY_COLOR;
        texEnvConf.srcRegRgb2 = IRenderer::TexEnvConf::SrcReg::TEXTURE;
        texEnvConf.srcRegAlpha0 = IRenderer::TexEnvConf::SrcReg::PRIMARY_COLOR;
        texEnvConf.operandRgb2 = IRenderer::TexEnvConf::Operand::SRC_ALPHA;
        break;
    case GL_BLEND:
        texEnvConf.combineRgb = IRenderer::TexEnvConf::Combine::INTERPOLATE;
        texEnvConf.combineAlpha = IRenderer::TexEnvConf::Combine::MODULATE;
        texEnvConf.srcRegRgb0 = IRenderer::TexEnvConf::SrcReg::CONSTANT;
        texEnvConf.srcRegRgb1 = IRenderer::TexEnvConf::SrcReg::PRIMARY_COLOR;
        texEnvConf.srcRegRgb2 = IRenderer::TexEnvConf::SrcReg::TEXTURE;
        texEnvConf.srcRegAlpha0 = IRenderer::TexEnvConf::SrcReg::PRIMARY_COLOR;
        texEnvConf.srcRegAlpha1 = IRenderer::TexEnvConf::SrcReg::TEXTURE;
        break;
    case GL_ADD:
        texEnvConf.combineRgb = IRenderer::TexEnvConf::Combine::ADD;
        texEnvConf.combineAlpha = IRenderer::TexEnvConf::Combine::ADD;
        texEnvConf.srcRegRgb0 = IRenderer::TexEnvConf::SrcReg::TEXTURE;
        texEnvConf.srcRegRgb1 = IRenderer::TexEnvConf::SrcReg::PRIMARY_COLOR;
        texEnvConf.srcRegAlpha0 = IRenderer::TexEnvConf::SrcReg::TEXTURE;
        texEnvConf.srcRegAlpha1 = IRenderer::TexEnvConf::SrcReg::PRIMARY_COLOR;
        break;
    case GL_COMBINE:
        // Nothing to do here.
        break;
    default:
        ret = GL_INVALID_ENUM;
        break;
    }
    return ret;
}

GLint IceGL::convertCombine(IRenderer::TexEnvConf::Combine& conv, GLint val, bool alpha) 
{
    GLint ret = GL_NO_ERROR;
    switch (val) 
    {
        case GL_REPLACE:
            conv = IRenderer::TexEnvConf::Combine::REPLACE;
            break;
        case GL_MODULATE:
            conv = IRenderer::TexEnvConf::Combine::MODULATE;
            break;
        case GL_ADD:
            conv = IRenderer::TexEnvConf::Combine::ADD;
            break;
        case GL_ADD_SIGNED:
            conv = IRenderer::TexEnvConf::Combine::ADD_SIGNED;
            break;
        case GL_INTERPOLATE:
            conv = IRenderer::TexEnvConf::Combine::INTERPOLATE;
            break;
        case GL_SUBTRACT:
            conv = IRenderer::TexEnvConf::Combine::SUBTRACT;
            break;
        case GL_DOT3_RGB:
            if (alpha) 
            {
                ret = GL_INVALID_ENUM;
            }
            else 
            {
                conv = IRenderer::TexEnvConf::Combine::DOT3_RGB;
            }
            break;
        case GL_DOT3_RGBA:
            if (alpha) 
            {
                ret = GL_INVALID_ENUM;
            }
            else 
            {
                conv = IRenderer::TexEnvConf::Combine::DOT3_RGBA;
            }
            break;
        default:
            ret = GL_INVALID_ENUM;
            break;
    }
    return ret;
}

GLint IceGL::convertOperand(IRenderer::TexEnvConf::Operand& conf, GLint val, bool alpha)
{
    GLint ret = GL_NO_ERROR;
    switch (val)
    {
        case GL_SRC_ALPHA:
            conf = IRenderer::TexEnvConf::Operand::SRC_ALPHA;
            break;
        case GL_ONE_MINUS_SRC_ALPHA:
            conf = IRenderer::TexEnvConf::Operand::ONE_MINUS_SRC_ALPHA;
            break;
        case GL_SRC_COLOR:
            if (alpha)
            {
                ret = GL_INVALID_ENUM;
            }
            else
            {
                conf = IRenderer::TexEnvConf::Operand::SRC_COLOR;
            }
            break;
        case GL_ONE_MINUS_SRC_COLOR:
            if (alpha)
            {
                ret = GL_INVALID_ENUM;
            }
            else
            {
                conf = IRenderer::TexEnvConf::Operand::ONE_MINUS_SRC_COLOR;
            }
            break;
        default:
            ret = GL_INVALID_ENUM;
    }
    return ret;
}

GLint IceGL::convertSrcReg(IRenderer::TexEnvConf::SrcReg& conf, GLint val)
{
    GLint ret = GL_NO_ERROR;
    switch (val)
    {
        case GL_TEXTURE:
            conf = IRenderer::TexEnvConf::SrcReg::TEXTURE;
            break;
        case GL_CONSTANT:
            conf = IRenderer::TexEnvConf::SrcReg::CONSTANT;
            break;
        case GL_PRIMARY_COLOR:
            conf = IRenderer::TexEnvConf::SrcReg::PRIMARY_COLOR;
            break;
        case GL_PREVIOUS:
            conf = IRenderer::TexEnvConf::SrcReg::PREVIOUS;
            break;
        default:
            ret = GL_INVALID_ENUM;
            break;
    }
    return ret;
}

void IceGL::glTexEnvi(GLenum target, GLenum pname, GLint param)
{
    m_error = GL_INVALID_ENUM;
    if (target == GL_TEXTURE_ENV)
    {
        IRenderer::TexEnvConf texEnvConf = IRenderer::TexEnvConf();
        if (m_texEnvMode == GL_COMBINE) 
        {
            texEnvConf = m_texEnvConf0;
        }
        switch (pname) 
        {
            case GL_TEXTURE_ENV_MODE:
                if (param == GL_COMBINE) 
                {
                    texEnvConf = m_texEnvConf0;
                    m_error = GL_NO_ERROR;
                }
                else
                {
                    m_error = convertTexEnvMode(texEnvConf, param); 
                }
                m_texEnvMode = param;
                break;
            case GL_COMBINE_RGB:
            {
                IRenderer::TexEnvConf::Combine c{ texEnvConf.combineRgb };
                m_error = convertCombine(c, param, false);
                texEnvConf.combineRgb = c;
                break;
            }
            case GL_COMBINE_ALPHA:
            {
                IRenderer::TexEnvConf::Combine c{ texEnvConf.combineAlpha };
                m_error = convertCombine(c, param, true);
                texEnvConf.combineAlpha = c;
                break;
            }
            case GL_SRC0_RGB:
            {
                IRenderer::TexEnvConf::SrcReg c{ texEnvConf.srcRegRgb0 };
                m_error = convertSrcReg(c, param);
                texEnvConf.srcRegRgb0 = c;
                break;
            }
            case GL_SRC1_RGB:
            {
                IRenderer::TexEnvConf::SrcReg c{ texEnvConf.srcRegRgb1 };
                m_error = convertSrcReg(c, param);
                texEnvConf.srcRegRgb1 = c;
                break;            
            }
            case GL_SRC2_RGB:
            {
                IRenderer::TexEnvConf::SrcReg c{ texEnvConf.srcRegRgb2 };
                m_error = convertSrcReg(c, param);
                texEnvConf.srcRegRgb2 = c;
                break;
            }
            case GL_SRC0_ALPHA:
            {
                IRenderer::TexEnvConf::SrcReg c{ texEnvConf.srcRegAlpha0 };
                m_error = convertSrcReg(c, param);
                texEnvConf.srcRegAlpha0 = c;
                break;
            }
            case GL_SRC1_ALPHA:
            {
                IRenderer::TexEnvConf::SrcReg c{ texEnvConf.srcRegAlpha1 };
                m_error = convertSrcReg(c, param);
                texEnvConf.srcRegAlpha1 = c;
                break;            
            }
            case GL_SRC2_ALPHA:
            {
                IRenderer::TexEnvConf::SrcReg c{ texEnvConf.srcRegAlpha2 };
                m_error = convertSrcReg(c, param);
                texEnvConf.srcRegAlpha2 = c;
                break;
            }
            case GL_OPERAND0_RGB:
            {
                IRenderer::TexEnvConf::Operand c{ texEnvConf.operandRgb0 };
                m_error = convertOperand(c, param, false);
                texEnvConf.operandRgb0 = c;
                break;
            }
            case GL_OPERAND1_RGB:
            {
                IRenderer::TexEnvConf::Operand c{ texEnvConf.operandRgb1 };
                m_error = convertOperand(c, param, false);
                texEnvConf.operandRgb1 = c;
                break;
            }
            case GL_OPERAND2_RGB:
            {
                IRenderer::TexEnvConf::Operand c{ texEnvConf.operandRgb2 };
                m_error = convertOperand(c, param, false);
                texEnvConf.operandRgb2 = c;
                break;
            }
            case GL_OPERAND0_ALPHA:
            {
                IRenderer::TexEnvConf::Operand c{ texEnvConf.operandAlpha0 };
                m_error = convertOperand(c, param, true);
                texEnvConf.operandAlpha0 = c;
                break;
            }
            case GL_OPERAND1_ALPHA:
            {
                IRenderer::TexEnvConf::Operand c{ texEnvConf.operandAlpha1 };
                m_error = convertOperand(c, param, true);
                texEnvConf.operandAlpha1 = c;
                break;
            }
            case GL_OPERAND2_ALPHA:
            {
                IRenderer::TexEnvConf::Operand c{ texEnvConf.operandAlpha2 };
                m_error = convertOperand(c, param, true);
                texEnvConf.operandAlpha2 = c;
                break;
            }
            case GL_RGB_SCALE:
                {
                    const uint8_t shift = log2(param);
                    if ((shift >= 0) && (shift <= 2))
                    {
                        texEnvConf.shiftRgb = shift;
                        m_error = GL_NO_ERROR;
                    }
                    else 
                    {
                        m_error = GL_INVALID_VALUE;
                    }
                }
                break;
            case GL_ALPHA_SCALE:
                {
                    const uint8_t shift = log2(param);
                    if ((shift >= 0) && (shift <= 2))
                    {
                        texEnvConf.shiftAlpha = shift;
                        m_error = GL_NO_ERROR;
                    }
                    else 
                    {
                        m_error = GL_INVALID_VALUE;
                    }
                }
                break;
            default:
                m_error = GL_INVALID_ENUM;
                break;
        };
        if (m_error == GL_NO_ERROR)
        {
            // Only set the new texEnvConfig when the current mode is GL_COMBINE, and when it wasn't set yet (otherwise the current config would be overwritten)
            if ((m_texEnvMode == GL_COMBINE) && (param != GL_COMBINE)) 
            {
                m_texEnvConf0 = texEnvConf;
            }

            if (m_renderer.setTexEnv(IRenderer::TMU::TMU0, texEnvConf))
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

IRenderer::FragmentPipelineConf::BlendFunc IceGL::convertGlBlendFuncToRenderBlendFunc(const GLenum blendFunc)
{
    switch (blendFunc) {
    case GL_ZERO:
        return IRenderer::FragmentPipelineConf::BlendFunc::ZERO;
    case GL_ONE:
        return IRenderer::FragmentPipelineConf::BlendFunc::ONE;
    case GL_DST_COLOR:
        return IRenderer::FragmentPipelineConf::BlendFunc::DST_COLOR;
    case GL_SRC_COLOR:
        return IRenderer::FragmentPipelineConf::BlendFunc::SRC_COLOR;
    case GL_ONE_MINUS_DST_COLOR:
        return IRenderer::FragmentPipelineConf::BlendFunc::ONE_MINUS_DST_COLOR;
    case GL_ONE_MINUS_SRC_COLOR:
        return IRenderer::FragmentPipelineConf::BlendFunc::ONE_MINUS_SRC_COLOR;
    case GL_SRC_ALPHA:
        return IRenderer::FragmentPipelineConf::BlendFunc::SRC_ALPHA;
    case GL_ONE_MINUS_SRC_ALPHA:
        return IRenderer::FragmentPipelineConf::BlendFunc::ONE_MINUS_SRC_ALPHA;
    case GL_DST_ALPHA:
        return IRenderer::FragmentPipelineConf::BlendFunc::DST_ALPHA;
    case GL_ONE_MINUS_DST_ALPHA:
        return IRenderer::FragmentPipelineConf::BlendFunc::ONE_MINUS_DST_ALPHA;
    case GL_SRC_ALPHA_SATURATE:
        return IRenderer::FragmentPipelineConf::BlendFunc::SRC_ALPHA_SATURATE;
    default:
        m_error = GL_INVALID_ENUM;
        return IRenderer::FragmentPipelineConf::BlendFunc::ZERO;
    }
    m_error = GL_INVALID_ENUM;
    return IRenderer::FragmentPipelineConf::BlendFunc::ZERO;
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
        m_fragmentPipelineConf.blendFuncSFactor = convertGlBlendFuncToRenderBlendFunc(sfactor);
        m_fragmentPipelineConf.blendFuncDFactor = convertGlBlendFuncToRenderBlendFunc(dfactor);
        if (m_renderer.setFragmentPipelineConfig(m_fragmentPipelineConf))
        {
            m_error = GL_NO_ERROR;
        }
        else
        {
            m_error = GL_OUT_OF_MEMORY;
        }
    }
}

void IceGL::glLogicOp(GLenum opcode)
{
    IRenderer::FragmentPipelineConf::LogicOp logicOp{IRenderer::FragmentPipelineConf::LogicOp::COPY};
    switch (opcode) {
    case GL_CLEAR:
        logicOp = IRenderer::FragmentPipelineConf::LogicOp::CLEAR;
        break;
    case GL_SET:
        logicOp = IRenderer::FragmentPipelineConf::LogicOp::SET;
        break;
    case GL_COPY:
        logicOp = IRenderer::FragmentPipelineConf::LogicOp::COPY;
        break;
    case GL_COPY_INVERTED:
        logicOp = IRenderer::FragmentPipelineConf::LogicOp::COPY_INVERTED;
        break;
    case GL_NOOP:
        logicOp = IRenderer::FragmentPipelineConf::LogicOp::NOOP;
        break;
    case GL_INVERTED:
        logicOp = IRenderer::FragmentPipelineConf::LogicOp::INVERTED;
        break;
    case GL_AND:
        logicOp = IRenderer::FragmentPipelineConf::LogicOp::AND;
        break;
    case GL_NAND:
        logicOp = IRenderer::FragmentPipelineConf::LogicOp::NAND;
        break;
    case GL_OR:
        logicOp = IRenderer::FragmentPipelineConf::LogicOp::OR;
        break;
    case GL_NOR:
        logicOp = IRenderer::FragmentPipelineConf::LogicOp::NOR;
        break;
    case GL_XOR:
        logicOp = IRenderer::FragmentPipelineConf::LogicOp::XOR;
        break;
    case GL_EQUIV:
        logicOp = IRenderer::FragmentPipelineConf::LogicOp::EQUIV;
        break;
    case GL_AND_REVERSE:
        logicOp = IRenderer::FragmentPipelineConf::LogicOp::AND_REVERSE;
        break;
    case GL_AND_INVERTED:
        logicOp = IRenderer::FragmentPipelineConf::LogicOp::AND_INVERTED;
        break;
    case GL_OR_REVERSE:
        logicOp = IRenderer::FragmentPipelineConf::LogicOp::OR_REVERSE;
        break;
    case GL_OR_INVERTED:
        logicOp = IRenderer::FragmentPipelineConf::LogicOp::OR_INVERTED;
        break;
    default:
        logicOp = IRenderer::FragmentPipelineConf::LogicOp::COPY;
        break;
    }
    // m_fragmentPipelineConf.logicOp = setLogicOp; // TODO: Not yet implemented
    if (m_renderer.setFragmentPipelineConfig(m_fragmentPipelineConf))
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
    bool valueChanged = false;
    m_error = GL_NO_ERROR;
    switch (pname) {
    case GL_FOG_MODE:
        valueChanged = m_fogMode != static_cast<GLenum>(param);
        m_fogMode = static_cast<GLenum>(param);
        break;
    case GL_FOG_DENSITY:
        if (param >= 0.0f)
        {
            valueChanged = m_fogDensity != param;
            m_fogDensity = param;
        }
        else
        {
            m_error = GL_INVALID_VALUE;
        }
        break;
    case GL_FOG_START:
        valueChanged = m_fogStart != param;
        m_fogStart = param;
        break;
    case GL_FOG_END:
        valueChanged = m_fogEnd != param;
        m_fogEnd = param;
        break;
    default:
        m_error = GL_INVALID_ENUM;
        break;
    }

    if (valueChanged && (m_error == GL_NO_ERROR))
    {
        m_error = setFogLut(m_fogMode, m_fogStart, m_fogEnd, m_fogDensity);
    }
}

void IceGL::glFogfv(GLenum pname, const GLfloat *params)
{
    m_error = GL_NO_ERROR;
    switch (pname) {
    case GL_FOG_MODE:
    case GL_FOG_DENSITY:
    case GL_FOG_START:
    case GL_FOG_END:
        glFogf(pname, *params);
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
}

void IceGL::glFogi(GLenum pname, GLint param)
{
    glFogf(pname, static_cast<float>(param));
}

void IceGL::glFogiv(GLenum pname, const GLint *params)
{
    m_error = GL_NO_ERROR;
    switch (pname) {
    case GL_FOG_MODE:
    case GL_FOG_DENSITY:
    case GL_FOG_START:
    case GL_FOG_END:
        glFogi(pname, *params);
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
}

void IceGL::glMaterialf(GLenum face, GLenum pname, GLfloat param)
{
    m_error = GL_INVALID_ENUM;
    if (face == GL_FRONT_AND_BACK)
    {
        if ((pname == GL_SHININESS) && (param >= 0.0f) && (param <= 128.0f))
        {
            m_vertexPipeline.getLighting().setSpecularExponentMaterial(param);
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
            m_vertexPipeline.getLighting().setAmbientColorMaterial({params});
            break;
        case GL_DIFFUSE:
            m_vertexPipeline.getLighting().setDiffuseColorMaterial({params});
            break;
        case GL_AMBIENT_AND_DIFFUSE:
            m_vertexPipeline.getLighting().setAmbientColorMaterial({params});
            m_vertexPipeline.getLighting().setDiffuseColorMaterial({params});
            break;
        case GL_SPECULAR:
            m_vertexPipeline.getLighting().setSpecularColorMaterial({params});
            break;
        case GL_EMISSION:
            m_vertexPipeline.getLighting().setEmissiveColorMaterial({params});
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
        VertexPipeline::Face faceConverted;
        switch (face) {
            case GL_FRONT:
                faceConverted = VertexPipeline::Face::FRONT;
                break;
            case GL_BACK:
                faceConverted = VertexPipeline::Face::BACK;
                break;
            case GL_FRONT_AND_BACK:
                faceConverted = VertexPipeline::Face::FRONT_AND_BACK;
                break;
            default:
                faceConverted = VertexPipeline::Face::FRONT_AND_BACK;
                m_error = GL_INVALID_ENUM;
                break;
        }

        if (m_error != GL_INVALID_ENUM)
        {
            switch (pname) {
            case GL_AMBIENT:
                m_vertexPipeline.setColorMaterialTracking(faceConverted, VertexPipeline::ColorMaterialTracking::AMBIENT);
                break;
            case GL_DIFFUSE:
                m_vertexPipeline.setColorMaterialTracking(faceConverted, VertexPipeline::ColorMaterialTracking::DIFFUSE);
                break;
            case GL_AMBIENT_AND_DIFFUSE:
                m_vertexPipeline.setColorMaterialTracking(faceConverted, VertexPipeline::ColorMaterialTracking::AMBIENT_AND_DIFFUSE);
                break;
            case GL_SPECULAR:
                m_vertexPipeline.setColorMaterialTracking(faceConverted, VertexPipeline::ColorMaterialTracking::SPECULAR);
                break;
            case GL_EMISSION:
                m_vertexPipeline.setColorMaterialTracking(faceConverted, VertexPipeline::ColorMaterialTracking::EMISSION);
                break;
            default:
                m_error = GL_INVALID_ENUM;
                m_vertexPipeline.setColorMaterialTracking(faceConverted, VertexPipeline::ColorMaterialTracking::AMBIENT_AND_DIFFUSE);
                break;
            }
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
        m_vertexPipeline.getLighting().setConstantAttenuationLight(light - GL_LIGHT0, param);
        break;
    case GL_LINEAR_ATTENUATION:
        m_vertexPipeline.getLighting().setLinearAttenuationLight(light - GL_LIGHT0, param);
        break;
    case GL_QUADRATIC_ATTENUATION:
        m_vertexPipeline.getLighting().setQuadraticAttenuationLight(light - GL_LIGHT0, param);
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
        m_vertexPipeline.getLighting().setAmbientColorLight(light - GL_LIGHT0, {params});
        break;
    case GL_DIFFUSE:
        m_vertexPipeline.getLighting().setDiffuseColorLight(light - GL_LIGHT0, {params});
        break;
    case GL_SPECULAR:
        m_vertexPipeline.getLighting().setSpecularColorLight(light - GL_LIGHT0, {params});
        break;
    case GL_POSITION:
    {
        Vec4 lightPos{params};
        Vec4 lightPosTransformed{};
        m_vertexPipeline.getModelMatrix().transform(lightPosTransformed, lightPos);
        m_vertexPipeline.getLighting().setPosLight(light - GL_LIGHT0, lightPosTransformed);
        break;
    }
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
        m_vertexPipeline.getLighting().setAmbientColorScene({params});
        m_error = GL_NO_ERROR;
    }
    else
    {
        glLightModelf(pname, params[0]);
    }
}

void IceGL::glVertexPointer(GLint size, GLenum type, GLsizei stride, const void *pointer)
{
    m_renderObj.setVertexSize(size);
    m_renderObj.setVertexType(convertType(type));
    m_renderObj.setVertexStride(stride);
    m_renderObj.setVertexPointer(pointer);
}

void IceGL::glTexCoordPointer(GLint size, GLenum type, GLsizei stride, const void *pointer)
{
    m_renderObj.setTexCoordSize(size);
    m_renderObj.setTexCoordType(convertType(type));
    m_renderObj.setTexCoordStride(stride);
    m_renderObj.setTexCoordPointer(pointer);
}

void IceGL::glNormalPointer(GLenum type, GLsizei stride, const void *pointer)
{
    m_renderObj.setNormalType(convertType(type));
    m_renderObj.setNormalStride(stride);
    m_renderObj.setNormalPointer(pointer);
}

void IceGL::glColorPointer(GLint size, GLenum type, GLsizei stride, const void *pointer)
{
    m_renderObj.setColorSize(size);
    m_renderObj.setColorType(convertType(type));
    m_renderObj.setColorStride(stride);
    m_renderObj.setColorPointer(pointer);
}

void IceGL::glDrawArrays(GLenum mode, GLint first, GLsizei count)
{
    m_renderObj.setCount(count);
    m_renderObj.setArrayOffset(first);
    m_renderObj.setDrawMode(convertDrawMode(mode));
    m_renderObj.enableIndices(false);
    m_renderObj.setVertexColor(m_vertexColor);

    if (m_error == GL_NO_ERROR)
    {
        m_vertexPipeline.drawObj(m_renderObj);
    }
}

void IceGL::glDrawElements(GLenum mode, GLsizei count, GLenum type, const void *indices)
{
    m_error = GL_NO_ERROR;
    switch (type) {
    case GL_UNSIGNED_BYTE:
        m_renderObj.setIndicesType(RenderObj::Type::BYTE);
        break;
    case GL_UNSIGNED_SHORT:
        m_renderObj.setIndicesType(RenderObj::Type::SHORT);
        break;
    case GL_UNSIGNED_INT:
        m_renderObj.setIndicesType(RenderObj::Type::UNSIGNED_INT);
        break;
    default:
        m_error = GL_INVALID_ENUM;
        return;
    }

    m_renderObj.setCount(count);
    m_renderObj.setDrawMode(convertDrawMode(mode));
    m_renderObj.setIndicesPointer(indices);
    m_renderObj.enableIndices(true);
    m_renderObj.setVertexColor(m_vertexColor);

    if (m_error == GL_NO_ERROR)
    {
        m_vertexPipeline.drawObj(m_renderObj);
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
        m_renderObj.enableColorArray(enable);
        break;
    case GL_NORMAL_ARRAY:
        m_renderObj.enableNormalArray(enable);
        break;
    case GL_TEXTURE_COORD_ARRAY:
        m_renderObj.enableTexCoordArray(enable);
        break;
    case GL_VERTEX_ARRAY:
        m_renderObj.enableVertexArray(enable);
        break;
    default:
        m_error = GL_INVALID_ENUM;
        break;
    }
}

RenderObj::Type IceGL::convertType(GLenum type)
{
    switch (type) {
    case GL_BYTE:
        return RenderObj::Type::BYTE;
    case GL_UNSIGNED_BYTE:
        return RenderObj::Type::UNSIGNED_BYTE;
    case GL_SHORT:
        return RenderObj::Type::SHORT;
    case GL_UNSIGNED_SHORT:
        return RenderObj::Type::UNSIGNED_SHORT;
    case GL_FLOAT:
        return RenderObj::Type::FLOAT;
    case GL_UNSIGNED_INT:
        return RenderObj::Type::UNSIGNED_INT;
    default:
        m_error = GL_INVALID_ENUM;
        return RenderObj::Type::BYTE;
    }
}

RenderObj::DrawMode IceGL::convertDrawMode(GLenum drawMode)
{
    switch (drawMode) {
    case GL_TRIANGLES:
        return RenderObj::DrawMode::TRIANGLES;
    case GL_TRIANGLE_FAN:
        return RenderObj::DrawMode::TRIANGLE_FAN;
    case GL_TRIANGLE_STRIP:
        return RenderObj::DrawMode::TRIANGLE_STRIP;
    case GL_QUAD_STRIP:
        return RenderObj::DrawMode::QUAD_STRIP;
    default:
        m_error = GL_INVALID_ENUM;
        return RenderObj::DrawMode::TRIANGLES;
    }
}

void IceGL::glPushMatrix()
{
    if (m_vertexPipeline.pushMatrix())
    {
        m_error = GL_NO_ERROR;
    }
    else
    {
        m_error = GL_STACK_OVERFLOW;
    }
}

void IceGL::glPopMatrix()
{
    if (m_vertexPipeline.popMatrix())
    {
        m_error = GL_NO_ERROR;
    }
    else
    {
        m_error = GL_STACK_OVERFLOW;
    }
}

void IceGL::glTexGeni(GLenum coord, GLenum pname, GLint param)
{
    m_error = GL_NO_ERROR;
    TexGen::TexGenMode mode;
    switch (param) {
    case GL_OBJECT_LINEAR:
        mode = TexGen::TexGenMode::OBJECT_LINEAR;
        break;
    case GL_EYE_LINEAR:
        mode = TexGen::TexGenMode::EYE_LINEAR;
        break;
    case GL_SPHERE_MAP:
        mode = TexGen::TexGenMode::SPHERE_MAP;
        break;
    default:
        m_error = GL_INVALID_ENUM;
        break;
    }

    if (m_error == GL_NO_ERROR && pname == GL_TEXTURE_GEN_MODE)
    {
        switch (coord) {
        case GL_S:
            m_vertexPipeline.getTexGen().setTexGenModeS(mode);
            break;
        case GL_T:
            m_vertexPipeline.getTexGen().setTexGenModeT(mode);
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
            m_vertexPipeline.getTexGen().setTexGenVecObjS({param});
        else if (coord == GL_T)
            m_vertexPipeline.getTexGen().setTexGenVecObjT({param});
        else
            m_error = GL_INVALID_ENUM;
        break;
    case GL_EYE_PLANE:
        if (coord == GL_S)
            m_vertexPipeline.getTexGen().setTexGenVecEyeS(m_vertexPipeline.getModelMatrix(), {param});
        else if (coord == GL_T)
            m_vertexPipeline.getTexGen().setTexGenVecEyeT(m_vertexPipeline.getModelMatrix(), {param});
        else
            m_error = GL_INVALID_ENUM;
        break;
    default:
        glTexGeni(coord, pname, static_cast<GLint>(param[0]));
        break;
    }
}

GLenum IceGL::setFogLut(GLenum mode, float start, float end, float density)
{
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
        return reinterpret_cast<const GLubyte*>("Rasterix");
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
        *params = m_vertexPipeline.getLighting().MAX_LIGHTS;
        break;
    case GL_MAX_MODELVIEW_STACK_DEPTH:
        *params = VertexPipeline::getModelMatrixStackDepth();
        break;
    case GL_MAX_PROJECTION_STACK_DEPTH:
        *params = VertexPipeline::getProjectionMatrixStackDepth();
        break;
    case GL_MAX_TEXTURE_SIZE:
        *params = MAX_TEX_SIZE;
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
        m_vertexPipeline.setCullMode(VertexPipeline::Face::BACK);
        break;
    case GL_FRONT:
        m_vertexPipeline.setCullMode(VertexPipeline::Face::FRONT);
        break;
    case GL_FRONT_AND_BACK:
        m_vertexPipeline.setCullMode(VertexPipeline::Face::FRONT_AND_BACK);
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

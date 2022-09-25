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
    , m_pixelPipeline(renderer)
    , m_vertexPipeline(m_pixelPipeline)
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
        vertexPipeline().setMatrixMode(VertexPipeline::MatrixMode::MODELVIEW);
    }
    else if (mm == GL_PROJECTION)
    {
        vertexPipeline().setMatrixMode(VertexPipeline::MatrixMode::PROJECTION);
    }
    else
    {
        m_error = GL_INVALID_ENUM;
    }
}

void IceGL::glMultMatrixf(const GLfloat* m)
{
    const Mat44 *m44 = reinterpret_cast<const Mat44*>(m);
    vertexPipeline().multiply(*m44);
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
    vertexPipeline().translate(x, y, z);
}

void IceGL::glScalef(GLfloat x, GLfloat y, GLfloat z)
{
    vertexPipeline().scale(x, y, z);
}

void IceGL::glRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
    vertexPipeline().rotate(angle, x, y, z);
}

void IceGL::glLoadIdentity()
{
    vertexPipeline().loadIdentity();
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
    vertexPipeline().setViewport(x, y, width, height);
}

void IceGL::glDepthRange(GLclampd zNear, GLclampd zFar)
{
    if (zNear > 1.0f) zNear = 1.0f;
    if (zNear < -1.0f) zNear = -1.0f;
    if (zFar  > 1.0f) zFar  = 1.0f;
    if (zFar  < -1.0f) zFar  = -1.0f;

    vertexPipeline().setDepthRange(zNear, zFar);
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


    vertexPipeline().drawObj(m_renderObjBeginEnd);
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

    // Initialize the memory with zero for non power of two textures.
    // Its probably the most reasonable init, because if the alpha channel is activated,
    // then the not used area is then just transparent.
    memset(texMemShared.get(), 0, widthRounded * heightRounded * 2);

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

        if (!pixelPipeline().uploadTexture(texMemShared, widthRounded, heightRounded))
        {
            m_error = GL_INVALID_VALUE;
            return;
        }
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
            // m_unpackAlignment = param;
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
        std::pair<bool, uint16_t> ret = pixelPipeline().createTexture();
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
            pixelPipeline().deleteTexture(textures[i]);
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

    pixelPipeline().setBoundTexture(texture);

    if (m_error == GL_NO_ERROR)
    {
        pixelPipeline().useTexture(PixelPipeline::TMU::TMU0);
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
                pixelPipeline().setTexWrapModeS(mode);
            }
            break;
        }
        case GL_TEXTURE_WRAP_T:
        {
            auto mode = convertGlTextureWrapMode(static_cast<GLenum>(param));
            if (m_error == GL_NO_ERROR)
            {
                pixelPipeline().setTexWrapModeT(mode);
            }
            break;
        }
            break;
        case GL_TEXTURE_MAG_FILTER:
            if ((param == GL_LINEAR) || (param == GL_NEAREST))
            {
                pixelPipeline().setEnableMagFilter(param == GL_LINEAR);
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
    if (pixelPipeline().clearFramebuffer(mask & GL_COLOR_BUFFER_BIT, mask & GL_DEPTH_BUFFER_BIT))
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
    if (pixelPipeline().setClearColor({ { red, green, blue, alpha } }))
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

    if (pixelPipeline().setClearDepth(depth))
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
        pixelPipeline().featureEnable().setEnableTmu0(true);
        break;
    case GL_ALPHA_TEST:
        // Note: The alpha test disabling and enabling is a bit special. If the alpha test is disabled, we are just saving
        // the current alpha test func and the reference value. When the test is enabled, we just recover this values.
        // Seems to work perfect.
        pixelPipeline().featureEnable().setEnableAlphaTest(true);
        break;
    case GL_DEPTH_TEST:
        // For the depth test it is not possible to use a similar algorithm like we do for the alpha test. The main reason
        // behind that is that the depth test uses a depth buffer. Is the depth test is disabled, the depth test always passes
        // and never writes into the depth buffer. We could use glDepthMask to avoid writing, but then we also disabling
        // glClear() what we obviously dont want. The easiest fix is to introduce a special switch. If this switch is disabled
        // the depth buffer always passes and never writes and glClear() can clear the depth buffer.
        pixelPipeline().featureEnable().setEnableDepthTest(true);
        break;
    case GL_BLEND:
        pixelPipeline().featureEnable().setEnableBlending(true);
        break;
    case GL_LIGHTING:
        vertexPipeline().getLighting().enableLighting(true);
        break;
    case GL_LIGHT0:
    case GL_LIGHT1:
    case GL_LIGHT2:
    case GL_LIGHT3:
    case GL_LIGHT4:
    case GL_LIGHT5:
    case GL_LIGHT6:
    case GL_LIGHT7:
        vertexPipeline().getLighting().enableLight(cap - GL_LIGHT0, true);
        break;
    case GL_TEXTURE_GEN_S:
        vertexPipeline().getTexGen().enableTexGenS(true);
        break;
    case GL_TEXTURE_GEN_T:
        vertexPipeline().getTexGen().enableTexGenT(true);
        break;
    case GL_CULL_FACE:
        vertexPipeline().enableCulling(true);
        break;
    case GL_COLOR_MATERIAL:
        vertexPipeline().enableColorMaterial(true);
        break;
    case GL_FOG:
        pixelPipeline().featureEnable().setEnableFog(true);
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
        pixelPipeline().featureEnable().setEnableTmu0(false);
        break;
    }
    case GL_ALPHA_TEST:
    {
        pixelPipeline().featureEnable().setEnableAlphaTest(false);
        break;
    }
    case GL_DEPTH_TEST:
    {
        pixelPipeline().featureEnable().setEnableDepthTest(false);
        break;
    }
    case GL_BLEND:
    {
        pixelPipeline().featureEnable().setEnableBlending(false);
        break;
    }
    case GL_LIGHTING:
        vertexPipeline().getLighting().enableLighting(false);
        break;
    case GL_LIGHT0:
    case GL_LIGHT1:
    case GL_LIGHT2:
    case GL_LIGHT3:
    case GL_LIGHT4:
    case GL_LIGHT5:
    case GL_LIGHT6:
    case GL_LIGHT7:
        vertexPipeline().getLighting().enableLight(cap - GL_LIGHT0, false);
        break;
    case GL_TEXTURE_GEN_S:
        vertexPipeline().getTexGen().enableTexGenS(false);
        break;
    case GL_TEXTURE_GEN_T:
        vertexPipeline().getTexGen().enableTexGenT(false);
        break;
    case GL_CULL_FACE:
        vertexPipeline().enableCulling(false);
        break;
    case GL_COLOR_MATERIAL:
        vertexPipeline().enableColorMaterial(false);
        break;
    case GL_FOG:
        pixelPipeline().featureEnable().setEnableFog(false);
        break;
    default:
        m_error = GL_SPEC_DEVIATION;
        break;
    }
}

void IceGL::glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
{
    m_error = GL_NO_ERROR;
    pixelPipeline().fragmentPipeline().setColorMaskR(red == GL_TRUE);
    pixelPipeline().fragmentPipeline().setColorMaskG(green == GL_TRUE);
    pixelPipeline().fragmentPipeline().setColorMaskB(blue == GL_TRUE);
    pixelPipeline().fragmentPipeline().setColorMaskA(alpha == GL_TRUE);
}

void IceGL::glDepthMask(GLboolean flag)
{
    m_error = GL_NO_ERROR;
    pixelPipeline().fragmentPipeline().setDepthMask(flag == GL_TRUE);
}

void IceGL::glDepthFunc(GLenum func)
{
    PixelPipeline::TestFunc testFunc { PixelPipeline::TestFunc::LESS };
    m_error = GL_NO_ERROR;
    switch (func)
    {
    case GL_ALWAYS:
        testFunc = PixelPipeline::TestFunc::ALWAYS;
        break;
    case GL_NEVER:
        testFunc = PixelPipeline::TestFunc::NEVER;
        break;
    case GL_LESS:
        testFunc = PixelPipeline::TestFunc::LESS;
        break;
    case GL_EQUAL:
        testFunc = PixelPipeline::TestFunc::EQUAL;
        break;
    case GL_LEQUAL:
        testFunc = PixelPipeline::TestFunc::LEQUAL;
        break;
    case GL_GREATER:
        testFunc = PixelPipeline::TestFunc::GREATER;
        break;
    case GL_NOTEQUAL:
        testFunc = PixelPipeline::TestFunc::NOTEQUAL;
        break;
    case GL_GEQUAL:
        testFunc = PixelPipeline::TestFunc::GEQUAL;
        break;

    default:
        m_error = GL_INVALID_ENUM;
        break;
    }

    if (m_error == GL_NO_ERROR)
    {
        pixelPipeline().fragmentPipeline().setDepthFunc(testFunc);
    }
}

void IceGL::glAlphaFunc(GLenum func, GLclampf ref)
{
    PixelPipeline::TestFunc testFunc { PixelPipeline::TestFunc::LESS };
    m_error = GL_NO_ERROR;
    switch (func)
    {
    case GL_ALWAYS:
        testFunc = PixelPipeline::TestFunc::ALWAYS;
        break;
    case GL_NEVER:
        testFunc = PixelPipeline::TestFunc::NEVER;
        break;
    case GL_LESS:
        testFunc = PixelPipeline::TestFunc::LESS;
        break;
    case GL_EQUAL:
        testFunc = PixelPipeline::TestFunc::EQUAL;
        break;
    case GL_LEQUAL:
        testFunc = PixelPipeline::TestFunc::LEQUAL;
        break;
    case GL_GREATER:
        testFunc = PixelPipeline::TestFunc::GREATER;
        break;
    case GL_NOTEQUAL:
        testFunc = PixelPipeline::TestFunc::NOTEQUAL;
        break;
    case GL_GEQUAL:
        testFunc = PixelPipeline::TestFunc::GEQUAL;
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
        pixelPipeline().fragmentPipeline().setAlphaFunc(testFunc);
        pixelPipeline().fragmentPipeline().setRefAlphaValue(refFix);
    }
}

GLint IceGL::convertTexEnvMode(PixelPipeline::TexEnvMode& mode, const GLint param) 
{
    GLint ret = GL_NO_ERROR;
    switch (param) {
    case GL_DISABLE:
        mode = PixelPipeline::TexEnvMode::DISABLE;
        break;
    case GL_REPLACE:
        mode = PixelPipeline::TexEnvMode::REPLACE;
        break;
    case GL_MODULATE:
        mode = PixelPipeline::TexEnvMode::MODULATE;
        break;
    case GL_DECAL:
        mode = PixelPipeline::TexEnvMode::DECAL;
        break;
    case GL_BLEND:
        mode = PixelPipeline::TexEnvMode::BLEND;
        break;
    case GL_ADD:
        mode = PixelPipeline::TexEnvMode::ADD;
        break;
    case GL_COMBINE:
        mode = PixelPipeline::TexEnvMode::COMBINE;
        break;
    default:
        ret = GL_INVALID_ENUM;
        mode = PixelPipeline::TexEnvMode::REPLACE;
        break;
    }
    return ret;
}

GLint IceGL::convertCombine(PixelPipeline::TexEnv::Combine& conv, GLint val, bool alpha) 
{
    GLint ret = GL_NO_ERROR;
    switch (val) 
    {
        case GL_REPLACE:
            conv = PixelPipeline::TexEnv::Combine::REPLACE;
            break;
        case GL_MODULATE:
            conv = PixelPipeline::TexEnv::Combine::MODULATE;
            break;
        case GL_ADD:
            conv = PixelPipeline::TexEnv::Combine::ADD;
            break;
        case GL_ADD_SIGNED:
            conv = PixelPipeline::TexEnv::Combine::ADD_SIGNED;
            break;
        case GL_INTERPOLATE:
            conv = PixelPipeline::TexEnv::Combine::INTERPOLATE;
            break;
        case GL_SUBTRACT:
            conv = PixelPipeline::TexEnv::Combine::SUBTRACT;
            break;
        case GL_DOT3_RGB:
            if (alpha) 
            {
                ret = GL_INVALID_ENUM;
            }
            else 
            {
                conv = PixelPipeline::TexEnv::Combine::DOT3_RGB;
            }
            break;
        case GL_DOT3_RGBA:
            if (alpha) 
            {
                ret = GL_INVALID_ENUM;
            }
            else 
            {
                conv = PixelPipeline::TexEnv::Combine::DOT3_RGBA;
            }
            break;
        default:
            ret = GL_INVALID_ENUM;
            break;
    }
    return ret;
}

GLint IceGL::convertOperand(PixelPipeline::TexEnv::Operand& conf, GLint val, bool alpha)
{
    GLint ret = GL_NO_ERROR;
    switch (val)
    {
        case GL_SRC_ALPHA:
            conf = PixelPipeline::TexEnv::Operand::SRC_ALPHA;
            break;
        case GL_ONE_MINUS_SRC_ALPHA:
            conf = PixelPipeline::TexEnv::Operand::ONE_MINUS_SRC_ALPHA;
            break;
        case GL_SRC_COLOR:
            if (alpha)
            {
                ret = GL_INVALID_ENUM;
            }
            else
            {
                conf = PixelPipeline::TexEnv::Operand::SRC_COLOR;
            }
            break;
        case GL_ONE_MINUS_SRC_COLOR:
            if (alpha)
            {
                ret = GL_INVALID_ENUM;
            }
            else
            {
                conf = PixelPipeline::TexEnv::Operand::ONE_MINUS_SRC_COLOR;
            }
            break;
        default:
            ret = GL_INVALID_ENUM;
    }
    return ret;
}

GLint IceGL::convertSrcReg(PixelPipeline::TexEnv::SrcReg& conf, GLint val)
{
    GLint ret = GL_NO_ERROR;
    switch (val)
    {
        case GL_TEXTURE:
            conf = PixelPipeline::TexEnv::SrcReg::TEXTURE;
            break;
        case GL_CONSTANT:
            conf = PixelPipeline::TexEnv::SrcReg::CONSTANT;
            break;
        case GL_PRIMARY_COLOR:
            conf = PixelPipeline::TexEnv::SrcReg::PRIMARY_COLOR;
            break;
        case GL_PREVIOUS:
            conf = PixelPipeline::TexEnv::SrcReg::PREVIOUS;
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
        switch (pname) 
        {
            case GL_TEXTURE_ENV_MODE:
            {
                PixelPipeline::TexEnvMode mode {};
                m_error = convertTexEnvMode(mode, param);
                if (m_error == GL_NO_ERROR)
                    pixelPipeline().setTexEnvMode(mode); 
                break;
            }
            case GL_COMBINE_RGB:
            {
                PixelPipeline::TexEnv::Combine c {};
                m_error = convertCombine(c, param, false);
                if (m_error == GL_NO_ERROR)
                    pixelPipeline().texEnv().setCombineRgb(c);
                break;
            }
            case GL_COMBINE_ALPHA:
            {
                PixelPipeline::TexEnv::Combine c {};
                m_error = convertCombine(c, param, true);
                if (m_error == GL_NO_ERROR)
                    pixelPipeline().texEnv().setCombineAlpha(c);
                break;
            }
            case GL_SRC0_RGB:
            {
                PixelPipeline::TexEnv::SrcReg c {};
                m_error = convertSrcReg(c, param);
                if (m_error == GL_NO_ERROR)
                    pixelPipeline().texEnv().setSrcRegRgb0(c);
                break;
            }
            case GL_SRC1_RGB:
            {
                PixelPipeline::TexEnv::SrcReg c {};
                m_error = convertSrcReg(c, param);
                if (m_error == GL_NO_ERROR)
                    pixelPipeline().texEnv().setSrcRegRgb1(c);
                break;            
            }
            case GL_SRC2_RGB:
            {
                PixelPipeline::TexEnv::SrcReg c {};
                m_error = convertSrcReg(c, param);
                if (m_error == GL_NO_ERROR)
                    pixelPipeline().texEnv().setSrcRegRgb2(c);
                break;
            }
            case GL_SRC0_ALPHA:
            {
                PixelPipeline::TexEnv::SrcReg c {};
                m_error = convertSrcReg(c, param);
                if (m_error == GL_NO_ERROR)
                    pixelPipeline().texEnv().setSrcRegAlpha0(c);
                break;
            }
            case GL_SRC1_ALPHA:
            {
                PixelPipeline::TexEnv::SrcReg c {};
                m_error = convertSrcReg(c, param);
                if (m_error == GL_NO_ERROR)
                    pixelPipeline().texEnv().setSrcRegAlpha1(c);
                break;            
            }
            case GL_SRC2_ALPHA:
            {
                PixelPipeline::TexEnv::SrcReg c {};
                m_error = convertSrcReg(c, param);
                if (m_error == GL_NO_ERROR)
                    pixelPipeline().texEnv().setSrcRegAlpha2(c);
                break;
            }
            case GL_OPERAND0_RGB:
            {
                PixelPipeline::TexEnv::Operand c {};
                m_error = convertOperand(c, param, false);
                if (m_error == GL_NO_ERROR)
                    pixelPipeline().texEnv().setOperandRgb0(c);
                break;
            }
            case GL_OPERAND1_RGB:
            {
                PixelPipeline::TexEnv::Operand c {};
                m_error = convertOperand(c, param, false);
                if (m_error == GL_NO_ERROR)
                    pixelPipeline().texEnv().setOperandRgb1(c);
                break;
            }
            case GL_OPERAND2_RGB:
            {
                PixelPipeline::TexEnv::Operand c {};
                m_error = convertOperand(c, param, false);
                if (m_error == GL_NO_ERROR)
                    pixelPipeline().texEnv().setOperandRgb2(c);
                break;
            }
            case GL_OPERAND0_ALPHA:
            {
                PixelPipeline::TexEnv::Operand c {};
                m_error = convertOperand(c, param, true);
                if (m_error == GL_NO_ERROR)
                    pixelPipeline().texEnv().setOperandAlpha0(c);
                break;
            }
            case GL_OPERAND1_ALPHA:
            {
                PixelPipeline::TexEnv::Operand c {};
                m_error = convertOperand(c, param, true);
                if (m_error == GL_NO_ERROR)
                    pixelPipeline().texEnv().setOperandAlpha1(c);
                break;
            }
            case GL_OPERAND2_ALPHA:
            {
                PixelPipeline::TexEnv::Operand c {};
                m_error = convertOperand(c, param, true);
                if (m_error == GL_NO_ERROR)
                    pixelPipeline().texEnv().setOperandAlpha2(c);
                break;
            }
            case GL_RGB_SCALE:
                {
                    const uint8_t shift = log2(param);
                    if ((shift >= 0) && (shift <= 2))
                    {
                        pixelPipeline().texEnv().setShiftRgb(shift);
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
                        pixelPipeline().texEnv().setShiftAlpha(shift);
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
        if (pixelPipeline().setTexEnvColor({ { param[0], param[1], param[2], param[3] } }))
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

PixelPipeline::BlendFunc IceGL::convertGlBlendFuncToRenderBlendFunc(const GLenum blendFunc)
{
    switch (blendFunc) {
    case GL_ZERO:
        return PixelPipeline::BlendFunc::ZERO;
    case GL_ONE:
        return PixelPipeline::BlendFunc::ONE;
    case GL_DST_COLOR:
        return PixelPipeline::BlendFunc::DST_COLOR;
    case GL_SRC_COLOR:
        return PixelPipeline::BlendFunc::SRC_COLOR;
    case GL_ONE_MINUS_DST_COLOR:
        return PixelPipeline::BlendFunc::ONE_MINUS_DST_COLOR;
    case GL_ONE_MINUS_SRC_COLOR:
        return PixelPipeline::BlendFunc::ONE_MINUS_SRC_COLOR;
    case GL_SRC_ALPHA:
        return PixelPipeline::BlendFunc::SRC_ALPHA;
    case GL_ONE_MINUS_SRC_ALPHA:
        return PixelPipeline::BlendFunc::ONE_MINUS_SRC_ALPHA;
    case GL_DST_ALPHA:
        return PixelPipeline::BlendFunc::DST_ALPHA;
    case GL_ONE_MINUS_DST_ALPHA:
        return PixelPipeline::BlendFunc::ONE_MINUS_DST_ALPHA;
    case GL_SRC_ALPHA_SATURATE:
        return PixelPipeline::BlendFunc::SRC_ALPHA_SATURATE;
    default:
        m_error = GL_INVALID_ENUM;
        return PixelPipeline::BlendFunc::ZERO;
    }
    m_error = GL_INVALID_ENUM;
    return PixelPipeline::BlendFunc::ZERO;
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
        pixelPipeline().fragmentPipeline().setBlendFuncSFactor(convertGlBlendFuncToRenderBlendFunc(sfactor));
        pixelPipeline().fragmentPipeline().setBlendFuncDFactor(convertGlBlendFuncToRenderBlendFunc(dfactor));
    }
}

void IceGL::glLogicOp(GLenum opcode)
{
    PixelPipeline::LogicOp logicOp { PixelPipeline::LogicOp::COPY };
    switch (opcode) {
    case GL_CLEAR:
        logicOp = PixelPipeline::LogicOp::CLEAR;
        break;
    case GL_SET:
        logicOp = PixelPipeline::LogicOp::SET;
        break;
    case GL_COPY:
        logicOp = PixelPipeline::LogicOp::COPY;
        break;
    case GL_COPY_INVERTED:
        logicOp = PixelPipeline::LogicOp::COPY_INVERTED;
        break;
    case GL_NOOP:
        logicOp = PixelPipeline::LogicOp::NOOP;
        break;
    case GL_INVERTED:
        logicOp = PixelPipeline::LogicOp::INVERTED;
        break;
    case GL_AND:
        logicOp = PixelPipeline::LogicOp::AND;
        break;
    case GL_NAND:
        logicOp = PixelPipeline::LogicOp::NAND;
        break;
    case GL_OR:
        logicOp = PixelPipeline::LogicOp::OR;
        break;
    case GL_NOR:
        logicOp = PixelPipeline::LogicOp::NOR;
        break;
    case GL_XOR:
        logicOp = PixelPipeline::LogicOp::XOR;
        break;
    case GL_EQUIV:
        logicOp = PixelPipeline::LogicOp::EQUIV;
        break;
    case GL_AND_REVERSE:
        logicOp = PixelPipeline::LogicOp::AND_REVERSE;
        break;
    case GL_AND_INVERTED:
        logicOp = PixelPipeline::LogicOp::AND_INVERTED;
        break;
    case GL_OR_REVERSE:
        logicOp = PixelPipeline::LogicOp::OR_REVERSE;
        break;
    case GL_OR_INVERTED:
        logicOp = PixelPipeline::LogicOp::OR_INVERTED;
        break;
    default:
        logicOp = PixelPipeline::LogicOp::COPY;
        break;
    }
    // pixelPipeline().setLogicOp(setLogicOp); // TODO: Not yet implemented
}

void IceGL::glFogf(GLenum pname, GLfloat param)
{
    m_error = GL_NO_ERROR;
    switch (pname) {
    case GL_FOG_MODE:
        switch (static_cast<GLenum>(param)) {
            case GL_EXP:
                pixelPipeline().setFogMode(PixelPipeline::FogMode::EXP);
                break;
            case GL_EXP2:
                pixelPipeline().setFogMode(PixelPipeline::FogMode::EXP2);
                break;
            case GL_LINEAR:
                pixelPipeline().setFogMode(PixelPipeline::FogMode::LINEAR);
                break;
            default:
                m_error = GL_INVALID_ENUM;
        }
        break;
    case GL_FOG_DENSITY:
        if (param >= 0.0f)
        {
            pixelPipeline().setFogDensity(param);
        }
        else
        {
            m_error = GL_INVALID_VALUE;
        }
        break;
    case GL_FOG_START:
        pixelPipeline().setFogStart(param);
        break;
    case GL_FOG_END:
        pixelPipeline().setFogEnd(param);
        break;
    default:
        m_error = GL_INVALID_ENUM;
        break;
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
        Vec4 fogColor {};
        fogColor.fromArray(params, 4);
        pixelPipeline().setFogColor({ { fogColor[0], fogColor[1], fogColor[2], fogColor[3] } });
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
    {
        Vec4 fogColor {};
        fogColor.fromArray(params, 4);
        fogColor.div(255);
        pixelPipeline().setFogColor({ { fogColor[0], fogColor[1], fogColor[2], fogColor[3] } });
        break;
    }
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
            vertexPipeline().getLighting().setSpecularExponentMaterial(param);
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
            vertexPipeline().getLighting().setAmbientColorMaterial({params});
            break;
        case GL_DIFFUSE:
            vertexPipeline().getLighting().setDiffuseColorMaterial({params});
            break;
        case GL_AMBIENT_AND_DIFFUSE:
            vertexPipeline().getLighting().setAmbientColorMaterial({params});
            vertexPipeline().getLighting().setDiffuseColorMaterial({params});
            break;
        case GL_SPECULAR:
            vertexPipeline().getLighting().setSpecularColorMaterial({params});
            break;
        case GL_EMISSION:
            vertexPipeline().getLighting().setEmissiveColorMaterial({params});
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
                vertexPipeline().setColorMaterialTracking(faceConverted, VertexPipeline::ColorMaterialTracking::AMBIENT);
                break;
            case GL_DIFFUSE:
                vertexPipeline().setColorMaterialTracking(faceConverted, VertexPipeline::ColorMaterialTracking::DIFFUSE);
                break;
            case GL_AMBIENT_AND_DIFFUSE:
                vertexPipeline().setColorMaterialTracking(faceConverted, VertexPipeline::ColorMaterialTracking::AMBIENT_AND_DIFFUSE);
                break;
            case GL_SPECULAR:
                vertexPipeline().setColorMaterialTracking(faceConverted, VertexPipeline::ColorMaterialTracking::SPECULAR);
                break;
            case GL_EMISSION:
                vertexPipeline().setColorMaterialTracking(faceConverted, VertexPipeline::ColorMaterialTracking::EMISSION);
                break;
            default:
                m_error = GL_INVALID_ENUM;
                vertexPipeline().setColorMaterialTracking(faceConverted, VertexPipeline::ColorMaterialTracking::AMBIENT_AND_DIFFUSE);
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
        vertexPipeline().getLighting().setConstantAttenuationLight(light - GL_LIGHT0, param);
        break;
    case GL_LINEAR_ATTENUATION:
        vertexPipeline().getLighting().setLinearAttenuationLight(light - GL_LIGHT0, param);
        break;
    case GL_QUADRATIC_ATTENUATION:
        vertexPipeline().getLighting().setQuadraticAttenuationLight(light - GL_LIGHT0, param);
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
        vertexPipeline().getLighting().setAmbientColorLight(light - GL_LIGHT0, {params});
        break;
    case GL_DIFFUSE:
        vertexPipeline().getLighting().setDiffuseColorLight(light - GL_LIGHT0, {params});
        break;
    case GL_SPECULAR:
        vertexPipeline().getLighting().setSpecularColorLight(light - GL_LIGHT0, {params});
        break;
    case GL_POSITION:
    {
        Vec4 lightPos{params};
        Vec4 lightPosTransformed{};
        vertexPipeline().getModelMatrix().transform(lightPosTransformed, lightPos);
        vertexPipeline().getLighting().setPosLight(light - GL_LIGHT0, lightPosTransformed);
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
        vertexPipeline().getLighting().setAmbientColorScene({params});
        m_error = GL_NO_ERROR;
    }
    else
    {
        glLightModelf(pname, params[0]);
    }
}

void IceGL::glVertexPointer(GLint size, GLenum type, GLsizei stride, const void *pointer)
{
    renderObj().setVertexSize(size);
    renderObj().setVertexType(convertType(type));
    renderObj().setVertexStride(stride);
    renderObj().setVertexPointer(pointer);
}

void IceGL::glTexCoordPointer(GLint size, GLenum type, GLsizei stride, const void *pointer)
{
    renderObj().setTexCoordSize(size);
    renderObj().setTexCoordType(convertType(type));
    renderObj().setTexCoordStride(stride);
    renderObj().setTexCoordPointer(pointer);
}

void IceGL::glNormalPointer(GLenum type, GLsizei stride, const void *pointer)
{
    renderObj().setNormalType(convertType(type));
    renderObj().setNormalStride(stride);
    renderObj().setNormalPointer(pointer);
}

void IceGL::glColorPointer(GLint size, GLenum type, GLsizei stride, const void *pointer)
{
    renderObj().setColorSize(size);
    renderObj().setColorType(convertType(type));
    renderObj().setColorStride(stride);
    renderObj().setColorPointer(pointer);
}

void IceGL::glDrawArrays(GLenum mode, GLint first, GLsizei count)
{
    renderObj().setCount(count);
    renderObj().setArrayOffset(first);
    renderObj().setDrawMode(convertDrawMode(mode));
    renderObj().enableIndices(false);
    renderObj().setVertexColor(m_vertexColor);

    if (m_error == GL_NO_ERROR)
    {
        vertexPipeline().drawObj(m_renderObj);
    }
}

void IceGL::glDrawElements(GLenum mode, GLsizei count, GLenum type, const void *indices)
{
    m_error = GL_NO_ERROR;
    switch (type) {
    case GL_UNSIGNED_BYTE:
        renderObj().setIndicesType(RenderObj::Type::BYTE);
        break;
    case GL_UNSIGNED_SHORT:
        renderObj().setIndicesType(RenderObj::Type::SHORT);
        break;
    case GL_UNSIGNED_INT:
        renderObj().setIndicesType(RenderObj::Type::UNSIGNED_INT);
        break;
    default:
        m_error = GL_INVALID_ENUM;
        return;
    }

    renderObj().setCount(count);
    renderObj().setDrawMode(convertDrawMode(mode));
    renderObj().setIndicesPointer(indices);
    renderObj().enableIndices(true);
    renderObj().setVertexColor(m_vertexColor);

    if (m_error == GL_NO_ERROR)
    {
        vertexPipeline().drawObj(m_renderObj);
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
        renderObj().enableColorArray(enable);
        break;
    case GL_NORMAL_ARRAY:
        renderObj().enableNormalArray(enable);
        break;
    case GL_TEXTURE_COORD_ARRAY:
        renderObj().enableTexCoordArray(enable);
        break;
    case GL_VERTEX_ARRAY:
        renderObj().enableVertexArray(enable);
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
    if (vertexPipeline().pushMatrix())
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
    if (vertexPipeline().popMatrix())
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
            vertexPipeline().getTexGen().setTexGenModeS(mode);
            break;
        case GL_T:
            vertexPipeline().getTexGen().setTexGenModeT(mode);
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
            vertexPipeline().getTexGen().setTexGenVecObjS({param});
        else if (coord == GL_T)
            vertexPipeline().getTexGen().setTexGenVecObjT({param});
        else
            m_error = GL_INVALID_ENUM;
        break;
    case GL_EYE_PLANE:
        if (coord == GL_S)
            vertexPipeline().getTexGen().setTexGenVecEyeS(vertexPipeline().getModelMatrix(), {param});
        else if (coord == GL_T)
            vertexPipeline().getTexGen().setTexGenVecEyeT(vertexPipeline().getModelMatrix(), {param});
        else
            m_error = GL_INVALID_ENUM;
        break;
    default:
        glTexGeni(coord, pname, static_cast<GLint>(param[0]));
        break;
    }
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
        *params = vertexPipeline().getLighting().MAX_LIGHTS;
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
        vertexPipeline().setCullMode(VertexPipeline::Face::BACK);
        break;
    case GL_FRONT:
        vertexPipeline().setCullMode(VertexPipeline::Face::FRONT);
        break;
    case GL_FRONT_AND_BACK:
        vertexPipeline().setCullMode(VertexPipeline::Face::FRONT_AND_BACK);
        break;
    default:
        m_error = GL_INVALID_ENUM;
        break;
    }
}

PixelPipeline::TextureWrapMode IceGL::convertGlTextureWrapMode(const GLenum mode)
{
    switch (mode) {
    case GL_CLAMP_TO_EDGE:
        return PixelPipeline::TextureWrapMode::CLAMP_TO_EDGE;
    case GL_REPEAT:
        return PixelPipeline::TextureWrapMode::REPEAT;
    default:
        m_error = GL_INVALID_ENUM;
        return PixelPipeline::TextureWrapMode::REPEAT;
    }
}

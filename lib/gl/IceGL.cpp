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

#include "IceGL.hpp"
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <spdlog/spdlog.h>

#define ADDRESS_OF(X) reinterpret_cast<const void *>(&X)
namespace rr
{
IceGL* instance { nullptr };

IceGL& IceGL::getInstance()
{
    return *instance;
}

bool IceGL::createInstance(IRenderer& renderer)
{
    if (instance)
    {
        delete instance;
    }
    instance = new IceGL { renderer };
    return instance != nullptr;
}

IceGL::IceGL(IRenderer &renderer)
    : m_renderer(renderer)
    , m_pixelPipeline(renderer)
    , m_vertexPipeline(m_pixelPipeline)
{
    // Preallocate the first texture. This is the default texture which also can't be deleted.
    m_renderer.createTexture();

    // Set initial values
    m_renderer.setTexEnvColor(0, { { 0, 0, 0, 0 } });
    m_renderer.setClearColor({ {0, 0, 0, 0 } });
    m_renderer.setClearDepth(65535);
    m_renderer.setFogColor({ { 255, 255, 255, 255 } });
    std::array<float, 33> fogLut {};
    std::fill(fogLut.begin(), fogLut.end(), 1.0f);
    m_renderer.setFogLut(fogLut, 0.0f, (std::numeric_limits<float>::max)()); // Windows defines macros with max ... parenthesis are a work around against build errors.

    // Register Open GL 1.2 procedures
    addProcedure("glDrawRangeElements", ADDRESS_OF(impl_glDrawRangeElements));
    addProcedure("glTexImage3D", ADDRESS_OF(impl_glTexImage3D));
    addProcedure("glTexSubImage3D", ADDRESS_OF(impl_glTexSubImage3D));
    addProcedure("glCopyTexSubImage3D", ADDRESS_OF(impl_glCopyTexSubImage3D));

    // Register Open GL 1.3 procedures
    addProcedure("glActiveTexture", ADDRESS_OF(impl_glActiveTexture));
    addProcedure("glSampleCoverage", ADDRESS_OF(impl_glSampleCoverage));
    addProcedure("glCompressedTexImage3D", ADDRESS_OF(impl_glCompressedTexImage3D));
    addProcedure("glCompressedTexImage2D", ADDRESS_OF(impl_glCompressedTexImage2D));
    addProcedure("glCompressedTexImage1D", ADDRESS_OF(impl_glCompressedTexImage1D));
    addProcedure("glCompressedTexSubImage3D", ADDRESS_OF(impl_glCompressedTexSubImage3D));
    addProcedure("glCompressedTexSubImage2D", ADDRESS_OF(impl_glCompressedTexSubImage2D));
    addProcedure("glCompressedTexSubImage1D", ADDRESS_OF(impl_glCompressedTexSubImage1D));
    addProcedure("glGetCompressedTexImage", ADDRESS_OF(impl_glGetCompressedTexImage));
    addProcedure("glClientActiveTexture", ADDRESS_OF(impl_glClientActiveTexture));
    addProcedure("glMultiTexCoord1d", ADDRESS_OF(impl_glMultiTexCoord1d));
    addProcedure("glMultiTexCoord1dv", ADDRESS_OF(impl_glMultiTexCoord1dv));
    addProcedure("glMultiTexCoord1f", ADDRESS_OF(impl_glMultiTexCoord1f));
    addProcedure("glMultiTexCoord1fv", ADDRESS_OF(impl_glMultiTexCoord1fv));
    addProcedure("glMultiTexCoord1i", ADDRESS_OF(impl_glMultiTexCoord1i));
    addProcedure("glMultiTexCoord1iv", ADDRESS_OF(impl_glMultiTexCoord1iv));
    addProcedure("glMultiTexCoord1s", ADDRESS_OF(impl_glMultiTexCoord1s));
    addProcedure("glMultiTexCoord1sv", ADDRESS_OF(impl_glMultiTexCoord1sv));
    addProcedure("glMultiTexCoord2d", ADDRESS_OF(impl_glMultiTexCoord2d));
    addProcedure("glMultiTexCoord2dv", ADDRESS_OF(impl_glMultiTexCoord2dv));
    addProcedure("glMultiTexCoord2f", ADDRESS_OF(impl_glMultiTexCoord2f));
    addProcedure("glMultiTexCoord2fv", ADDRESS_OF(impl_glMultiTexCoord2fv));
    addProcedure("glMultiTexCoord2i", ADDRESS_OF(impl_glMultiTexCoord2i));
    addProcedure("glMultiTexCoord2iv", ADDRESS_OF(impl_glMultiTexCoord2iv));
    addProcedure("glMultiTexCoord2s", ADDRESS_OF(impl_glMultiTexCoord2s));
    addProcedure("glMultiTexCoord2sv", ADDRESS_OF(impl_glMultiTexCoord2sv));
    addProcedure("glMultiTexCoord3d", ADDRESS_OF(impl_glMultiTexCoord3d));
    addProcedure("glMultiTexCoord3dv", ADDRESS_OF(impl_glMultiTexCoord3dv));
    addProcedure("glMultiTexCoord3f", ADDRESS_OF(impl_glMultiTexCoord3f));
    addProcedure("glMultiTexCoord3fv", ADDRESS_OF(impl_glMultiTexCoord3fv));
    addProcedure("glMultiTexCoord3i", ADDRESS_OF(impl_glMultiTexCoord3i));
    addProcedure("glMultiTexCoord3iv", ADDRESS_OF(impl_glMultiTexCoord3iv));
    addProcedure("glMultiTexCoord3s", ADDRESS_OF(impl_glMultiTexCoord3s));
    addProcedure("glMultiTexCoord3sv", ADDRESS_OF(impl_glMultiTexCoord3sv));
    addProcedure("glMultiTexCoord4d", ADDRESS_OF(impl_glMultiTexCoord4d));
    addProcedure("glMultiTexCoord4dv", ADDRESS_OF(impl_glMultiTexCoord4dv));
    addProcedure("glMultiTexCoord4f", ADDRESS_OF(impl_glMultiTexCoord4f));
    addProcedure("glMultiTexCoord4fv", ADDRESS_OF(impl_glMultiTexCoord4fv));
    addProcedure("glMultiTexCoord4i", ADDRESS_OF(impl_glMultiTexCoord4i));
    addProcedure("glMultiTexCoord4iv", ADDRESS_OF(impl_glMultiTexCoord4iv));
    addProcedure("glMultiTexCoord4s", ADDRESS_OF(impl_glMultiTexCoord4s));
    addProcedure("glMultiTexCoord4sv", ADDRESS_OF(impl_glMultiTexCoord4sv));
    addProcedure("glLoadTransposeMatrixf", ADDRESS_OF(impl_glLoadTransposeMatrixf));
    addProcedure("glLoadTransposeMatrixd", ADDRESS_OF(impl_glLoadTransposeMatrixd));
    addProcedure("glMultTransposeMatrixf", ADDRESS_OF(impl_glMultTransposeMatrixf));
    addProcedure("glMultTransposeMatrixd", ADDRESS_OF(impl_glMultTransposeMatrixd));

    // Register the implemented extensions
    addExtension("GL_ARB_texture_env_add");
    addExtension("GL_ARB_texture_env_combine");
    // addExtension("GL_ARB_texture_cube_map");
    addExtension("GL_ARB_texture_env_dot3");
    addExtension("GL_ARB_multitexture");
    {

        addProcedure("glMultiTexCoord1dARB", ADDRESS_OF(impl_glMultiTexCoord1d));
        addProcedure("glMultiTexCoord1dvARB", ADDRESS_OF(impl_glMultiTexCoord1dv));
        addProcedure("glMultiTexCoord1fARB", ADDRESS_OF(impl_glMultiTexCoord1f));
        addProcedure("glMultiTexCoord1fvARB", ADDRESS_OF(impl_glMultiTexCoord1fv));
        addProcedure("glMultiTexCoord1iARB", ADDRESS_OF(impl_glMultiTexCoord1i));
        addProcedure("glMultiTexCoord1ivARB", ADDRESS_OF(impl_glMultiTexCoord1iv));
        addProcedure("glMultiTexCoord1sARB", ADDRESS_OF(impl_glMultiTexCoord1s));
        addProcedure("glMultiTexCoord1svARB", ADDRESS_OF(impl_glMultiTexCoord1sv));
        addProcedure("glMultiTexCoord2dARB", ADDRESS_OF(impl_glMultiTexCoord2d));
        addProcedure("glMultiTexCoord2dvARB", ADDRESS_OF(impl_glMultiTexCoord2dv));
        addProcedure("glMultiTexCoord2fARB", ADDRESS_OF(impl_glMultiTexCoord2f));
        addProcedure("glMultiTexCoord2fvARB", ADDRESS_OF(impl_glMultiTexCoord2fv));
        addProcedure("glMultiTexCoord2iARB", ADDRESS_OF(impl_glMultiTexCoord2i));
        addProcedure("glMultiTexCoord2ivARB", ADDRESS_OF(impl_glMultiTexCoord2iv));
        addProcedure("glMultiTexCoord2sARB", ADDRESS_OF(impl_glMultiTexCoord2s));
        addProcedure("glMultiTexCoord2svARB", ADDRESS_OF(impl_glMultiTexCoord2sv));
        addProcedure("glMultiTexCoord3dARB", ADDRESS_OF(impl_glMultiTexCoord3d));
        addProcedure("glMultiTexCoord3dvARB", ADDRESS_OF(impl_glMultiTexCoord3dv));
        addProcedure("glMultiTexCoord3fARB", ADDRESS_OF(impl_glMultiTexCoord3f));
        addProcedure("glMultiTexCoord3fvARB", ADDRESS_OF(impl_glMultiTexCoord3fv));
        addProcedure("glMultiTexCoord3iARB", ADDRESS_OF(impl_glMultiTexCoord3i));
        addProcedure("glMultiTexCoord3ivARB", ADDRESS_OF(impl_glMultiTexCoord3iv));
        addProcedure("glMultiTexCoord3sARB", ADDRESS_OF(impl_glMultiTexCoord3s));
        addProcedure("glMultiTexCoord3svARB", ADDRESS_OF(impl_glMultiTexCoord3sv));
        addProcedure("glMultiTexCoord4dARB", ADDRESS_OF(impl_glMultiTexCoord4d));
        addProcedure("glMultiTexCoord4dvARB", ADDRESS_OF(impl_glMultiTexCoord4dv));
        addProcedure("glMultiTexCoord4fARB", ADDRESS_OF(impl_glMultiTexCoord4f));
        addProcedure("glMultiTexCoord4fvARB", ADDRESS_OF(impl_glMultiTexCoord4fv));
        addProcedure("glMultiTexCoord4iARB", ADDRESS_OF(impl_glMultiTexCoord4i));
        addProcedure("glMultiTexCoord4ivARB", ADDRESS_OF(impl_glMultiTexCoord4iv));
        addProcedure("glMultiTexCoord4sARB", ADDRESS_OF(impl_glMultiTexCoord4s));
        addProcedure("glMultiTexCoord4svARB", ADDRESS_OF(impl_glMultiTexCoord4sv));
        addProcedure("glActiveTextureARB", ADDRESS_OF(impl_glActiveTexture));
        addProcedure("glClientActiveTextureARB", ADDRESS_OF(impl_glClientActiveTexture));
    }
    // addExtension("GL_EXT_compiled_vertex_array");
    // {

    //     addProcedure("glLockArraysEXT", ADDRESS_OF(impl_glLockArrays));
    //     addProcedure("glUnlockArraysEXT", ADDRESS_OF(impl_glUnlockArrays));
    // }
    // addExtension("WGL_3DFX_gamma_control");
    // {

    //     addProcedure("wglGetDeviceGammaRamp3DFX", ADDRESS_OF(impl_wglGetDeviceGammaRamp));
    //     addProcedure("wglSetDeviceGammaRamp3DFX", ADDRESS_OF(impl_wglSetDeviceGammaRamp));
    // }
    // addExtension("WGL_ARB_extensions_string");
    // {

    //     addProcedure("wglGetExtensionsStringARB", ADDRESS_OF(impl_wglGetExtensionsString));
    // }
}

void IceGL::render()
{
    SPDLOG_INFO("Render called");
    m_renderer.render();
}

const char *IceGL::getLibExtensions() const
{
    return m_glExtensions.c_str();
}

const void *IceGL::getLibProcedure(std::string name) const
{
    auto proc = m_glProcedures.find(name);
    if (proc != m_glProcedures.end())
    {

        return proc->second;
    }

    SPDLOG_WARN("Procedure %s isn't implemented", name.c_str());

    return nullptr;
}

void IceGL::addProcedure(std::string name, const void *address)
{
    m_glProcedures[name] = address;
}

void IceGL::addExtension(std::string extension)
{
    if (!m_glExtensions.empty())
    {

        m_glExtensions.append(" ");
    }
    m_glExtensions.append(extension);
}
} // namespace rr
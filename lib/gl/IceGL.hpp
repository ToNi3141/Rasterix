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

#ifndef ICEGL_HPP
#define ICEGL_HPP

#include <vector>
#include <array>
#include <functional>
#include "IRenderer.hpp"
#include "VertexPipeline.hpp"
#include "Vec.hpp"
#include "Lighting.hpp"
#include "TexGen.hpp"
#include "RenderObj.hpp"
#include "PixelPipeline.hpp"
#include "VertexQueue.hpp"
#include "gl.h"

class IceGL
{
public:
    IceGL(IRenderer& renderer);

    static IceGL& getInstance();
    static bool createInstance(IRenderer& renderer);

    void setError(const uint32_t error) { m_error = error; }
    uint32_t getError() const { return m_error; }

    VertexPipeline& vertexPipeline() { return m_vertexPipeline; }
    PixelPipeline& pixelPipeline() { return m_pixelPipeline; }
    RenderObj& renderObj() { return m_renderObj; }
    VertexQueue& vertexQueue() { return m_vertexQueue; }
    void commit();

    static constexpr uint16_t MAX_TEX_SIZE = 256;
private:


    void setClientState(const GLenum array, bool enable);
    // It would be nice to have std::optional, but it does not work with arduino
    // If we have this, we could make all this functions const and return an empty optional if the conversion failed
    PixelPipeline::FragmentPipeline::BlendFunc convertGlBlendFuncToRenderBlendFunc(const GLenum blendFunc);
    RenderObj::Type convertType(GLenum type);
    RenderObj::DrawMode convertDrawMode(GLenum drawMode);
    PixelPipeline::TextureWrapMode convertGlTextureWrapMode(const GLenum mode);
    GLint convertTexEnvMode(PixelPipeline::TexEnvMode& mode, const GLint param);
    GLint convertCombine(PixelPipeline::TexEnv::Combine& conv, GLint val, bool alpha);
    GLint convertOperand(PixelPipeline::TexEnv::Operand& conf, GLint val, bool alpha);
    GLint convertSrcReg(PixelPipeline::TexEnv::SrcReg& conf, GLint val);

    IRenderer& m_renderer;
    PixelPipeline m_pixelPipeline;
    VertexPipeline m_vertexPipeline;
    RenderObj m_renderObj;
    VertexQueue m_vertexQueue {};

    // Errors
    uint32_t m_error { 0 };
};

#endif // ICEGL_HPP

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
#include <map>

namespace rr
{
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
    VertexQueue& vertexQueue() { return m_vertexQueue; }
    void commit();

    const char *getLibExtensions() const;
    const void *getLibProcedure(std::string name) const;

    static constexpr uint16_t MAX_TEX_SIZE { IRenderer::MAX_TEXTURE_SIZE_PX };
    static constexpr uint16_t MAX_TEXTURE_UNITS { IRenderer::MAX_TMU_COUNT };
private:
    IRenderer& m_renderer;
    PixelPipeline m_pixelPipeline;
    VertexPipeline m_vertexPipeline;
    VertexQueue m_vertexQueue {};

    // Errors
    uint32_t m_error { 0 };

    // OpenGL extensions 
    void addProcedure(std::string name, const void *address);
    void addExtension(std::string extension);
    std::map<std::string, const void *> m_glProcedures;
    std::string m_glExtensions;
};

} // namespace rr
#endif // ICEGL_HPP

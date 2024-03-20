// Rasterix
// https://github.com/ToNi3141/Rasterix
// Copyright (c) 2024 ToNi3141

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

#ifndef VERTEXPIPELINE_HPP
#define VERTEXPIPELINE_HPP

#include "Vec.hpp"
#include "PixelPipeline.hpp"
#include "Mat44.hpp"
#include "Clipper.hpp"
#include "Lighting.hpp"
#include "ViewPort.hpp"
#include "TexGen.hpp"
#include "RenderObj.hpp"
#include "IRenderer.hpp"
#include "Stack.hpp"
#include "MatrixStack.hpp"
#include "Types.hpp"

namespace rr
{
class VertexPipeline
{
public:

    VertexPipeline(PixelPipeline& renderer);

    bool drawObj(const RenderObj &obj);

    void setEnableNormalizing(const bool enable) { m_enableNormalizing = enable; }

    void enableCulling(const bool enable);
    void setCullMode(const Face mode);


    
    void setLineWidth(const float width);

    void activateTmu(const uint8_t tmu) { m_tmu = tmu; m_matrixStack.setTmu(tmu); }

    Lighting& getLighting();
    TexGen& getTexGen();
    ViewPort& getViewPort();
    MatrixStack& getMatrixStack();

private:
    static constexpr std::size_t VERTEX_BUFFER_SIZE { 24 };
    static_assert(VERTEX_BUFFER_SIZE % 4 == 0, "VERTEX_BUFFER_SIZE must be dividable through 4 (used for GL_QUADS");
    static_assert(VERTEX_BUFFER_SIZE % 3 == 0, "VERTEX_BUFFER_SIZE must be dividable through 3 (used for GL_TRIANGLES");
    static constexpr std::size_t VERTEX_OVERLAP { 2 }; // The overlap makes it easier to use the array. The overlap is used to create triangles even if VERTEX_BUFFER_SIZE is exceeded
    using Vec4Array = std::array<Vec4, VERTEX_BUFFER_SIZE + VERTEX_OVERLAP>;
    using TexCoordArray = std::array<Clipper::ClipTexCoords, VERTEX_BUFFER_SIZE + VERTEX_OVERLAP>;
    using Vec3Array = std::array<Vec3, VERTEX_BUFFER_SIZE + VERTEX_OVERLAP>;

    struct Triangle
    {
        const Vec4& v0;
        const Vec4& v1;
        const Vec4& v2;
        const std::array<std::reference_wrapper<const Vec4>, IRenderer::MAX_TMU_COUNT>& tc0;
        const std::array<std::reference_wrapper<const Vec4>, IRenderer::MAX_TMU_COUNT>& tc1;
        const std::array<std::reference_wrapper<const Vec4>, IRenderer::MAX_TMU_COUNT>& tc2;
        const Vec4& color0;
        const Vec4& color1;
        const Vec4& color2;
    };

    struct Line
    {
        const Vec4& v0;
        const Vec4& v1;
        const std::array<std::reference_wrapper<const Vec4>, IRenderer::MAX_TMU_COUNT>& tc0;
        const std::array<std::reference_wrapper<const Vec4>, IRenderer::MAX_TMU_COUNT>& tc1;
        const Vec4& color0;
        const Vec4& color1;
    };

    bool drawTriangle(const Triangle& triangle);
    bool drawLine(const Line& line);

    void getTransformed(Vec4& vertex, Vec4& color, std::array<Vec4, IRenderer::MAX_TMU_COUNT>& tex, const RenderObj& obj, const uint32_t index);

    void loadVertexData(const RenderObj& obj, Vec4Array& vertex, Vec4Array& color, Vec3Array& normal, TexCoordArray& tex, const std::size_t offset, const std::size_t count);
    void transform(
        Vec4Array& transformedVertex, 
        Vec4Array& transformedColor, 
        Vec3Array& transformedNormal, 
        TexCoordArray& transformedTex, 
        const bool enableVertexArray,
        const bool enableColorArray,
        const bool enableNormalArray,
        const std::bitset<IRenderer::MAX_TMU_COUNT> enableTexArray,
        const Vec4Array& vertex, 
        const Vec4Array& color, 
        const Vec3Array& normal, 
        const TexCoordArray& tex, 
        const Vec4& vertexColor,
        const std::size_t count
    );
    bool drawTriangleArray(        
        const Vec4Array& vertex, 
        const Vec4Array& color, 
        const TexCoordArray& tex, 
        const std::size_t count,
        const RenderObj::DrawMode drawMode
    );
    bool drawLineArray(
        const Vec4Array& vertex, 
        const Vec4Array& color, 
        const TexCoordArray& tex, 
        const std::size_t count, 
        const RenderObj::DrawMode drawMode,
        const bool lastRound
    );

    bool m_enableCulling{ false };
    Face m_cullMode{ Face::BACK };

    bool m_enableNormalizing { true };

    // Line width
    float m_lineWidth { 1.0f };

    // Current active TMU
    uint8_t m_tmu {};

    PixelPipeline& m_renderer;
    Lighting m_lighting;
    ViewPort m_viewPort;
    MatrixStack m_matrixStack;
    std::array<TexGen, IRenderer::MAX_TMU_COUNT> m_texGen;
};

} // namespace rr
#endif // VERTEXPIPELINE_HPP

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

#ifndef VERTEXPIPELINE_HPP
#define VERTEXPIPELINE_HPP

#include "Vec.hpp"
#include "PixelPipeline.hpp"
#include "Mat44.hpp"
#include "Clipper.hpp"
#include "Lighting.hpp"
#include "TexGen.hpp"
#include "RenderObj.hpp"
#include "IRenderer.hpp"

class VertexPipeline
{
public:
    enum MatrixMode
    {
        MODELVIEW,
        PROJECTION,
        TEXTURE,
        COLOR
    };

    enum class ColorMaterialTracking
    {
        AMBIENT,
        DIFFUSE,
        AMBIENT_AND_DIFFUSE,
        SPECULAR,
        EMISSION
    };

    enum class Face
    {
        BACK,
        FRONT,
        FRONT_AND_BACK
    };

    VertexPipeline(PixelPipeline& renderer);

    bool drawObj(const RenderObj &obj);

    void setViewport(const int16_t x, const int16_t y, const int16_t width, const int16_t height);
    void setDepthRange(const float zNear, const float zFar);
    void setModelProjectionMatrix(const Mat44& m);
    void setModelMatrix(const Mat44& m);
    void setNormalMatrix(const Mat44& m);
    void setProjectionMatrix(const Mat44& m);
    void setTextureMatrix(const Mat44& m);
    void setColorMatrix(const Mat44& m);
    void setEnableNormalizing(const bool enable) { m_enableNormalizing = enable; }

    void enableCulling(const bool enable);
    void setCullMode(const Face mode);

    void multiply(const Mat44& mat);
    void translate(const float x, const float y, const float z);
    void scale(const float x, const float y, const float z);
    void rotate(const float angle, const float x, const float y, const float z);
    void loadIdentity();

    bool pushMatrix();
    bool popMatrix();

    const Mat44& getModelMatrix() const;
    const Mat44& getProjectionMatrix() const;

    void setMatrixMode(const MatrixMode matrixMode);
    bool loadMatrix(const Mat44& m);

    void setColorMaterialTracking(const Face face, const ColorMaterialTracking material);
    void enableColorMaterial(const bool enable);
    
    void setLineWidth(const float width);

    void activateTmu(const uint8_t tmu) { m_tmu = tmu; }

    Lighting& getLighting();
    TexGen& getTexGen();

    static uint8_t getModelMatrixStackDepth();
    static uint8_t getProjectionMatrixStackDepth();
private:
    static constexpr std::size_t VERTEX_BUFFER_SIZE { 24 };
    static_assert(VERTEX_BUFFER_SIZE % 4 == 0, "VERTEX_BUFFER_SIZE must be dividable through 4 (used for GL_QUADS");
    static_assert(VERTEX_BUFFER_SIZE % 3 == 0, "VERTEX_BUFFER_SIZE must be dividable through 3 (used for GL_TRIANGLES");
    static constexpr std::size_t VERTEX_OVERLAP { 2 }; // The overlap makes it easier to use the array. The overlap is used to create triangles even if VERTEX_BUFFER_SIZE is exceeded
    using Vec4Array = std::array<Vec4, VERTEX_BUFFER_SIZE + VERTEX_OVERLAP>;
    using TexCoordArray = std::array<Clipper::ClipTexCoords, VERTEX_BUFFER_SIZE + VERTEX_OVERLAP>;
    using Vec3Array = std::array<Vec3, VERTEX_BUFFER_SIZE + VERTEX_OVERLAP>;
    static constexpr uint8_t MODEL_MATRIX_STACK_DEPTH { 16 };
    static constexpr uint8_t TEXTURE_MATRIX_STACK_DEPTH { 16 };
    static constexpr uint8_t PROJECTION_MATRIX_STACK_DEPTH { 4 };
    static constexpr uint8_t COLOR_MATRIX_STACK_DEPTH { 16 };

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

    inline void viewportTransform(Vec4 &v0, Vec4 &v1, Vec4 &v2);
    inline void viewportTransform(Vec4 &v);

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

    void recalculateMatrices();

    float m_depthRangeZNear { 0.0f };
    float m_depthRangeZFar { 1.0f };
    int16_t m_viewportX { 0 };
    int16_t m_viewportY { 0 };
    int16_t m_viewportHeight { 0 };
    int16_t m_viewportWidth { 0 };
    float m_viewportXShift { 0.0f };
    float m_viewportYShift { 0.0f };
    float m_viewportHeightHalf { 0.0f };
    float m_viewportWidthHalf { 0.0f };

    bool m_enableCulling{ false };
    Face m_cullMode{ Face::BACK };

    bool m_enableNormalizing { true };

    // Matrix modes
    MatrixMode m_matrixMode { MatrixMode::PROJECTION };
    Mat44 m_mStack[MODEL_MATRIX_STACK_DEPTH] {};
    Mat44 m_pStack[PROJECTION_MATRIX_STACK_DEPTH] {};
    std::array<Mat44, IRenderer::MAX_TMU_COUNT> m_tmStack[TEXTURE_MATRIX_STACK_DEPTH] {};
    Mat44 m_cStack[COLOR_MATRIX_STACK_DEPTH] {};
    uint8_t m_mStackIndex{ 0 };
    uint8_t m_pStackIndex{ 0 };
    std::array<uint8_t, IRenderer::MAX_TMU_COUNT> m_tmStackIndex{ 0 };
    uint8_t m_cStackIndex{ 0 };
    Mat44 m_p {}; // Projection 
    Mat44 m_t {}; // ModelViewProjection
    Mat44 m_m {}; // ModelView
    Mat44 m_n {}; // Normal
    std::array<Mat44, IRenderer::MAX_TMU_COUNT> m_tm; // Texture Matrix
    Mat44 m_c {}; // Color
    bool m_matricesOutdated { true }; // Marks when the model and projection matrices have changed so that the transformation and normal matrices have to be recalculated

    // Color material
    bool m_enableColorMaterial { false };
    ColorMaterialTracking m_colorMaterialTracking { ColorMaterialTracking::AMBIENT_AND_DIFFUSE };
    Face m_colorMaterialFace { Face::FRONT_AND_BACK };

    // Line width
    float m_lineWidth { 1.0f };

    // Current active TMU
    uint8_t m_tmu {};

    PixelPipeline& m_renderer;
    Lighting m_lighting;
    std::array<TexGen, IRenderer::MAX_TMU_COUNT> m_texGen;
};

#endif // VERTEXPIPELINE_HPP

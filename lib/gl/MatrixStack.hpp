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

#ifndef MATRIXSTACK_HPP
#define MATRIXSTACK_HPP

#include "Vec.hpp"
#include "Mat44.hpp"
#include "Stack.hpp"
#include "IRenderer.hpp"
#include "Types.hpp"

namespace rr
{
class MatrixStack
{
public:
    MatrixStack();

    const Mat44& getModelViewProjection() const { return m_t; }
    const Mat44& getModelView() const { return m_m; }
    const Mat44& getProjection() const { return m_p; }
    const Mat44& getTexture(const uint8_t tmu) const { return m_tm[tmu]; }
    const Mat44& getColor() const { return m_c; }
    const Mat44& getNormal() const { return m_n; }

    void setModelProjectionMatrix(const Mat44& m);
    void setModelMatrix(const Mat44& m);
    void setNormalMatrix(const Mat44& m);
    void setProjectionMatrix(const Mat44& m);
    void setTextureMatrix(const Mat44& m);
    void setColorMatrix(const Mat44& m);

    void multiply(const Mat44& mat);
    void translate(const float x, const float y, const float z);
    void scale(const float x, const float y, const float z);
    void rotate(const float angle, const float x, const float y, const float z);
    void loadIdentity();

    bool pushMatrix();
    bool popMatrix();

    void setMatrixMode(const MatrixMode matrixMode);
    void setTmu(const uint8_t tmu);
    bool loadMatrix(const Mat44& m);
    
    void recalculateMatrices();

    static uint8_t getModelMatrixStackDepth();
    static uint8_t getProjectionMatrixStackDepth();
private:
    static constexpr uint8_t MODEL_MATRIX_STACK_DEPTH { 16 };
    static constexpr uint8_t TEXTURE_MATRIX_STACK_DEPTH { 16 };
    static constexpr uint8_t PROJECTION_MATRIX_STACK_DEPTH { 4 };
    static constexpr uint8_t COLOR_MATRIX_STACK_DEPTH { 16 };

    void recalculateModelProjectionMatrix();
    void recalculateNormalMatrix();

    MatrixMode m_matrixMode { MatrixMode::PROJECTION };
    Stack<Mat44, MODEL_MATRIX_STACK_DEPTH> m_mStack {};
    Stack<Mat44, PROJECTION_MATRIX_STACK_DEPTH> m_pStack {};
    std::array<Stack<Mat44, TEXTURE_MATRIX_STACK_DEPTH>, IRenderer::MAX_TMU_COUNT> m_tmStack {};
    Stack<Mat44, COLOR_MATRIX_STACK_DEPTH> m_cStack {};
    Mat44 m_p {}; // Projection 
    Mat44 m_t {}; // ModelViewProjection
    Mat44 m_m {}; // ModelView
    Mat44 m_n {}; // Normal
    std::array<Mat44, IRenderer::MAX_TMU_COUNT> m_tm; // Texture Matrix
    Mat44 m_c {}; // Color
    bool m_modelMatrixChanged { true };
    bool m_projectionMatrixChanged { true };
    uint8_t m_tmu { 0 };
};

} // namespace rr
#endif // MATRIXSTACK_HPP

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

#include "MatrixStack.hpp"
#include <cmath>

namespace rr
{
MatrixStack::MatrixStack()
{
    m_t.identity();
    m_m.identity();
    m_p.identity();
    m_n.identity();
    for (auto& mat : m_tm)
    {
        mat.identity();
    }
    m_c.identity();
}

void MatrixStack::setModelProjectionMatrix(const Mat44 &m)
{
    m_t = m;
}

void MatrixStack::setModelMatrix(const Mat44 &m)
{
    m_m = m;
    m_modelMatrixChanged = true;
}

void MatrixStack::setProjectionMatrix(const Mat44 &m)
{
    m_p = m;
    m_projectionMatrixChanged = true;
}

void MatrixStack::setColorMatrix(const Mat44& m)
{
    m_c = m;
}

void MatrixStack::setTextureMatrix(const Mat44& m)
{
    m_tm[m_tmu] = m;
}

void MatrixStack::setNormalMatrix(const Mat44& m)
{
    m_n = m;
}

void MatrixStack::multiply(const Mat44& mat)
{
    switch (m_matrixMode)
    {
        case MatrixMode::MODELVIEW:
            setModelMatrix(mat * m_m);
            break;
        case MatrixMode::PROJECTION:
            setProjectionMatrix(mat * m_p);
            break;
        case MatrixMode::TEXTURE:
            setTextureMatrix(mat * m_tm[m_tmu]);
            break;
        case MatrixMode::COLOR:
            setColorMatrix(mat * m_c);
            break;
         default:
            break;
    }
}

void MatrixStack::translate(const float x, const float y, const float z)
{
    Mat44 m;
    m.identity();
    m[3][0] = x;
    m[3][1] = y;
    m[3][2] = z;
    multiply(m);
}

void MatrixStack::scale(const float x, const float y, const float z)
{
    Mat44 m;
    m.identity();
    m[0][0] = x;
    m[1][1] = y;
    m[2][2] = z;
    multiply(m);
}

void MatrixStack::rotate(const float angle, const float x, const float y, const float z)
{
    static constexpr float PI { 3.14159265358979323846f };
    float angle_rad = angle * (PI/180.0f);

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

    multiply(m);
}

void MatrixStack::loadIdentity()
{
    switch (m_matrixMode)
    {
        case MatrixMode::MODELVIEW:
            m_m.identity();
            m_modelMatrixChanged = true;
            break;
        case MatrixMode::PROJECTION:
            m_p.identity();
            m_projectionMatrixChanged = true;
            break;
        case MatrixMode::TEXTURE:
            m_tm[m_tmu].identity();
            break;
        case MatrixMode::COLOR:
            m_c.identity();
            break;
        default:
            break;
    }
}

bool MatrixStack::pushMatrix()
{
    switch (m_matrixMode)
    {
        case MatrixMode::MODELVIEW:
            return m_mStack.push(m_m);
        case MatrixMode::PROJECTION:
            return m_pStack.push(m_p);
        case MatrixMode::COLOR:
            return m_cStack.push(m_c);
        case MatrixMode::TEXTURE:
            return m_tmStack[m_tmu].push(m_tm[m_tmu]);
        default:
            return false;
    }
}

bool MatrixStack::popMatrix()
{
    switch (m_matrixMode)
    {
        case MatrixMode::MODELVIEW:
            m_modelMatrixChanged = true;
            return m_mStack.pop(m_m);
        case MatrixMode::PROJECTION:
            m_projectionMatrixChanged = true;
            return m_pStack.pop(m_p);
        case MatrixMode::COLOR:
            return m_cStack.pop(m_c);
        case MatrixMode::TEXTURE:
            return m_tmStack[m_tmu].pop(m_tm[m_tmu]);
        default:
            return false;
    }
}

void MatrixStack::recalculateMatrices()
{
    if (m_modelMatrixChanged)
    {
        recalculateNormalMatrix();
    }
    if (m_modelMatrixChanged || m_projectionMatrixChanged)
    {
        recalculateModelProjectionMatrix();
    }
    m_modelMatrixChanged = false;
    m_projectionMatrixChanged = false;
}

void MatrixStack::recalculateModelProjectionMatrix()
{
    // Update transformation matrix
    setModelProjectionMatrix(m_m * m_p);
}

void MatrixStack::recalculateNormalMatrix()
{
    m_n = m_m;
    m_n.invert();
    m_n.transpose();
}

void MatrixStack::setMatrixMode(const MatrixMode matrixMode)
{
    m_matrixMode = matrixMode;
}

void MatrixStack::setTmu(const std::size_t tmu)
{
    m_tmu = tmu;
}

bool MatrixStack::loadMatrix(const Mat44& m)
{
    switch (m_matrixMode)
    {
    case MatrixMode::MODELVIEW:
        setModelMatrix(m);
        return true;
    case MatrixMode::PROJECTION:
        setProjectionMatrix(m);
        return true;
    case MatrixMode::TEXTURE:
        setTextureMatrix(m);
        return true;
    case MatrixMode::COLOR:
        setColorMatrix(m);
        return true;
    default:
        break;
    }
    return false;
}

std::size_t MatrixStack::getModelMatrixStackDepth()
{
    return MODEL_MATRIX_STACK_DEPTH;
}

std::size_t MatrixStack::getProjectionMatrixStackDepth()
{
    return PROJECTION_MATRIX_STACK_DEPTH;
}

} // namespace rr

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

#include "TexGen.hpp"

namespace rr
{

void TexGen::setMatrixStack(const MatrixStack& matrixStack)
{
    m_matrixStack = &matrixStack;
}

void TexGen::calculateTexGenCoords(Vec4& st0, const Vec4& v0, const Vec3& n0) const
{
    if (m_texGenEnableS || m_texGenEnableT || m_texGenEnableR)
    {
        Vec4 eyeVertex;
        Vec3 reflectionVector;
        reflectionVector.init();
        float m = 0.0f;
        if ((m_texGenModeS == TexGenMode::EYE_LINEAR)
            || (m_texGenModeT == TexGenMode::EYE_LINEAR)
            || (m_texGenModeR == TexGenMode::EYE_LINEAR))
        {
            eyeVertex = m_matrixStack->getModelView().transform(v0);
        }
        if ((m_texGenModeS == TexGenMode::SPHERE_MAP)
            || (m_texGenModeT == TexGenMode::SPHERE_MAP)
            || (m_texGenModeR == TexGenMode::SPHERE_MAP))
        {
            eyeVertex = m_matrixStack->getModelView().transform(v0);
            eyeVertex.normalize();
            Vec3 eyeVertex3 { eyeVertex.data() };
            Vec3 eyeNormal = m_matrixStack->getNormal().transform(n0);
            reflectionVector = eyeVertex3 - eyeNormal * 2.0 * eyeNormal.dot(eyeVertex3);
            reflectionVector[2] += 1.0f;
            m = 1.0f / (2.0f * sqrt(reflectionVector.dot(reflectionVector)));
        }
        if (m_texGenEnableS)
        {
            switch (m_texGenModeS)
            {
            case TexGenMode::OBJECT_LINEAR:
                st0[0] = m_texGenVecObjS.dot(v0);
                break;
            case TexGenMode::EYE_LINEAR:
                st0[0] = m_texGenVecEyeS.dot(eyeVertex);
                break;
            case TexGenMode::SPHERE_MAP:
                st0[0] = reflectionVector[0] * m + 0.5f;
                break;
            default:
                break;
            }
        }
        if (m_texGenEnableT)
        {
            switch (m_texGenModeT)
            {
            case TexGenMode::OBJECT_LINEAR:
                st0[1] = m_texGenVecObjT.dot(v0);
                break;
            case TexGenMode::EYE_LINEAR:
                st0[1] = m_texGenVecEyeT.dot(eyeVertex);
                break;
            case TexGenMode::SPHERE_MAP:
                st0[1] = reflectionVector[1] * m + 0.5f;
                break;
            default:
                break;
            }
        }
        if (m_texGenEnableR)
        {
            switch (m_texGenModeR)
            {
            case TexGenMode::OBJECT_LINEAR:
                st0[2] = m_texGenVecObjR.dot(v0);
                break;
            case TexGenMode::EYE_LINEAR:
                st0[2] = m_texGenVecEyeR.dot(eyeVertex);
                break;
            case TexGenMode::SPHERE_MAP:
                st0[2] = reflectionVector[2] * m + 0.5f;
                break;
            default:
                break;
            }
        }
    }
}

void TexGen::enableTexGenS(bool enable)
{
    m_texGenEnableS = enable;
}

void TexGen::enableTexGenT(bool enable)
{
    m_texGenEnableT = enable;
}

void TexGen::enableTexGenR(bool enable)
{
    m_texGenEnableR = enable;
}

void TexGen::setTexGenModeS(TexGenMode mode)
{
    m_texGenModeS = mode;
}

void TexGen::setTexGenModeT(TexGenMode mode)
{
    m_texGenModeT = mode;
}

void TexGen::setTexGenModeR(TexGenMode mode)
{
    m_texGenModeR = mode;
}

void TexGen::setTexGenVecObjS(const Vec4& val)
{
    m_texGenVecObjS = val;
}

void TexGen::setTexGenVecObjT(const Vec4& val)
{
    m_texGenVecObjT = val;
}

void TexGen::setTexGenVecObjR(const Vec4& val)
{
    m_texGenVecObjR = val;
}

void TexGen::setTexGenVecEyeS(const Vec4& val)
{
    m_texGenVecEyeS = m_matrixStack->getNormal().transform(val);
}

void TexGen::setTexGenVecEyeT(const Vec4& val)
{
    m_texGenVecEyeT = m_matrixStack->getNormal().transform(val);
}

void TexGen::setTexGenVecEyeR(const Vec4& val)
{
    m_texGenVecEyeR = m_matrixStack->getNormal().transform(val);
}

} // namespace rr
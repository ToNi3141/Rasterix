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

void TexGen::setMatrixStore(const MatrixStore& matrixStore)
{
    m_matrixStore = &matrixStore;
}

void TexGen::calculateTexGenCoords(Vec4& st0, const Vec4& v0, const Vec3& n0) const
{
    if (m_texGenEnableS || m_texGenEnableT || m_texGenEnableR)
    {
        if ((m_texGenModeS == TexGenMode::OBJECT_LINEAR)
            || (m_texGenModeT == TexGenMode::OBJECT_LINEAR)
            || (m_texGenModeR == TexGenMode::OBJECT_LINEAR))
        {
            calculateObjectLinear(st0, v0);
        }
        if ((m_texGenModeS == TexGenMode::EYE_LINEAR)
            || (m_texGenModeT == TexGenMode::EYE_LINEAR)
            || (m_texGenModeR == TexGenMode::EYE_LINEAR))
        {
            calculateEyeLinear(st0, v0);
        }
        if ((m_texGenModeS == TexGenMode::SPHERE_MAP)
            || (m_texGenModeT == TexGenMode::SPHERE_MAP)
            || (m_texGenModeR == TexGenMode::SPHERE_MAP))
        {
            calculateSphereMap(st0, v0, n0);
        }
        if ((m_texGenModeS == TexGenMode::REFLECTION_MAP)
            || (m_texGenModeT == TexGenMode::REFLECTION_MAP)
            || (m_texGenModeR == TexGenMode::REFLECTION_MAP))
        {
            calculateReflectionMap(st0, v0, n0);
        }
    }
}

void TexGen::calculateObjectLinear(Vec4& st0, const Vec4& v0) const
{
    if (m_texGenEnableS && (m_texGenModeS == TexGenMode::OBJECT_LINEAR))
    {
        st0[0] = m_texGenVecObjS.dot(v0);
    }
    if (m_texGenEnableT && (m_texGenModeT == TexGenMode::OBJECT_LINEAR))
    {
        st0[1] = m_texGenVecObjT.dot(v0);
    }
    if (m_texGenEnableR && (m_texGenModeR == TexGenMode::OBJECT_LINEAR))
    {
        st0[2] = m_texGenVecObjR.dot(v0);
    }
}

void TexGen::calculateEyeLinear(Vec4& st0, const Vec4& v0) const
{
    const Vec4 eyeVertex = m_matrixStore->getModelView().transform(v0);
    if (m_texGenEnableS && (m_texGenModeS == TexGenMode::EYE_LINEAR))
    {
        st0[0] = m_texGenVecEyeS.dot(eyeVertex);
    }
    if (m_texGenEnableT && (m_texGenModeT == TexGenMode::EYE_LINEAR))
    {
        st0[1] = m_texGenVecEyeT.dot(eyeVertex);
    }
    if (m_texGenEnableR && (m_texGenModeR == TexGenMode::EYE_LINEAR))
    {
        st0[2] = m_texGenVecEyeR.dot(eyeVertex);
    }
}

void TexGen::calculateSphereMap(Vec4& st0, const Vec4& v0, const Vec3& n0) const
{
    const Vec3 sphereVector = calculateSphereVector(v0, n0);
    if (m_texGenEnableS && (m_texGenModeS == TexGenMode::SPHERE_MAP))
    {
        st0[0] = sphereVector[0];
    }
    if (m_texGenEnableT && (m_texGenModeT == TexGenMode::SPHERE_MAP))
    {
        st0[1] = sphereVector[1];
    }
    if (m_texGenEnableR && (m_texGenModeR == TexGenMode::SPHERE_MAP))
    {
        st0[2] = sphereVector[2];
    }
}

void TexGen::calculateReflectionMap(Vec4& st0, const Vec4& v0, const Vec3& n0) const
{
    const Vec3 reflectionVector = calculateReflectionVector(v0, n0);
    if (m_texGenEnableS && (m_texGenModeS == TexGenMode::REFLECTION_MAP))
    {
        st0[0] = reflectionVector[0];
    }
    if (m_texGenEnableT && (m_texGenModeT == TexGenMode::REFLECTION_MAP))
    {
        st0[1] = reflectionVector[1];
    }
    if (m_texGenEnableR && (m_texGenModeR == TexGenMode::REFLECTION_MAP))
    {
        st0[2] = reflectionVector[2];
    }
}

Vec3 TexGen::calculateSphereVector(const Vec4& v0, const Vec3& n0) const
{
    Vec4 eyeVertex = m_matrixStore->getModelView().transform(v0);
    eyeVertex.normalize();
    const Vec3 eyeVertex3 { eyeVertex.data() };
    const Vec3 eyeNormal = m_matrixStore->getNormal().transform(n0);
    Vec3 reflectionVector = eyeVertex3 - eyeNormal * 2.0f * eyeNormal.dot(eyeVertex3);
    reflectionVector[2] += 1.0f;
    const float m = 1.0f / (2.0f * sqrt(reflectionVector.dot(reflectionVector)));
    return (reflectionVector * m) + Vec3 { 0.5f, 0.5f, 0.5f };
}

Vec3 TexGen::calculateReflectionVector(const Vec4& v0, const Vec3& n0) const
{
    Vec4 eyeVertex = m_matrixStore->getModelView().transform(v0);
    eyeVertex.normalize();
    const Vec3 eyeVertex3 { eyeVertex.data() };
    const Vec3 eyeNormal = m_matrixStore->getNormal().transform(n0);
    const float dotResult = 2.0f * eyeVertex3.dot(eyeNormal);
    return eyeVertex3 - (eyeNormal * dotResult);
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
    m_texGenVecEyeS = m_matrixStore->getNormal().transform(val);
}

void TexGen::setTexGenVecEyeT(const Vec4& val)
{
    m_texGenVecEyeT = m_matrixStore->getNormal().transform(val);
}

void TexGen::setTexGenVecEyeR(const Vec4& val)
{
    m_texGenVecEyeR = m_matrixStore->getNormal().transform(val);
}

} // namespace rr
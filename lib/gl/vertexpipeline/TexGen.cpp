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

void TexGen::TexGenCalc::calculateTexGenCoords(
    Vec4& st0,
    const Mat44& modelViewMat,
    const Mat44& normalMat,
    const Vec4& v0,
    const Vec3& n0) const
{
    if (texGenEnableS || texGenEnableT || texGenEnableR)
    {
        if ((texGenModeS == TexGenMode::OBJECT_LINEAR)
            || (texGenModeT == TexGenMode::OBJECT_LINEAR)
            || (texGenModeR == TexGenMode::OBJECT_LINEAR))
        {
            calculateObjectLinear(st0, v0);
        }
        if ((texGenModeS == TexGenMode::EYE_LINEAR)
            || (texGenModeT == TexGenMode::EYE_LINEAR)
            || (texGenModeR == TexGenMode::EYE_LINEAR))
        {
            calculateEyeLinear(st0, modelViewMat.transform(v0));
        }
        if ((texGenModeS == TexGenMode::SPHERE_MAP)
            || (texGenModeT == TexGenMode::SPHERE_MAP)
            || (texGenModeR == TexGenMode::SPHERE_MAP))
        {
            calculateSphereMap(st0, modelViewMat.transform(v0), normalMat.transform(n0));
        }
        if ((texGenModeS == TexGenMode::REFLECTION_MAP)
            || (texGenModeT == TexGenMode::REFLECTION_MAP)
            || (texGenModeR == TexGenMode::REFLECTION_MAP))
        {
            calculateReflectionMap(st0, modelViewMat.transform(v0), normalMat.transform(n0));
        }
    }
}

void TexGen::TexGenCalc::calculateObjectLinear(Vec4& st0, const Vec4& v0) const
{
    if (texGenEnableS && (texGenModeS == TexGenMode::OBJECT_LINEAR))
    {
        st0[0] = texGenVecObjS.dot(v0);
    }
    if (texGenEnableT && (texGenModeT == TexGenMode::OBJECT_LINEAR))
    {
        st0[1] = texGenVecObjT.dot(v0);
    }
    if (texGenEnableR && (texGenModeR == TexGenMode::OBJECT_LINEAR))
    {
        st0[2] = texGenVecObjR.dot(v0);
    }
}

void TexGen::TexGenCalc::calculateEyeLinear(Vec4& st0, const Vec4& eyeVertex) const
{
    if (texGenEnableS && (texGenModeS == TexGenMode::EYE_LINEAR))
    {
        st0[0] = texGenVecEyeS.dot(eyeVertex);
    }
    if (texGenEnableT && (texGenModeT == TexGenMode::EYE_LINEAR))
    {
        st0[1] = texGenVecEyeT.dot(eyeVertex);
    }
    if (texGenEnableR && (texGenModeR == TexGenMode::EYE_LINEAR))
    {
        st0[2] = texGenVecEyeR.dot(eyeVertex);
    }
}

void TexGen::TexGenCalc::calculateSphereMap(Vec4& st0, const Vec4& eyeVertex, const Vec3& eyeNormal) const
{
    const Vec3 sphereVector = calculateSphereVector(eyeVertex, eyeNormal);
    if (texGenEnableS && (texGenModeS == TexGenMode::SPHERE_MAP))
    {
        st0[0] = sphereVector[0];
    }
    if (texGenEnableT && (texGenModeT == TexGenMode::SPHERE_MAP))
    {
        st0[1] = sphereVector[1];
    }
    if (texGenEnableR && (texGenModeR == TexGenMode::SPHERE_MAP))
    {
        st0[2] = sphereVector[2];
    }
}

void TexGen::TexGenCalc::calculateReflectionMap(Vec4& st0, const Vec4& eyeVertex, const Vec3& eyeNormal) const
{
    const Vec3 reflectionVector = calculateReflectionVector(eyeVertex, eyeNormal);
    if (texGenEnableS && (texGenModeS == TexGenMode::REFLECTION_MAP))
    {
        st0[0] = reflectionVector[0];
    }
    if (texGenEnableT && (texGenModeT == TexGenMode::REFLECTION_MAP))
    {
        st0[1] = reflectionVector[1];
    }
    if (texGenEnableR && (texGenModeR == TexGenMode::REFLECTION_MAP))
    {
        st0[2] = reflectionVector[2];
    }
}

Vec3 TexGen::TexGenCalc::calculateSphereVector(Vec4 eyeVertex, const Vec3& eyeNormal) const
{
    eyeVertex.normalize();
    const Vec3 eyeVertex3 { eyeVertex.data() };
    Vec3 reflectionVector = eyeVertex3 - eyeNormal * 2.0f * eyeNormal.dot(eyeVertex3);
    reflectionVector[2] += 1.0f;
    const float m = 1.0f / (2.0f * sqrt(reflectionVector.dot(reflectionVector)));
    return (reflectionVector * m) + Vec3 { 0.5f, 0.5f, 0.5f };
}

Vec3 TexGen::TexGenCalc::calculateReflectionVector(Vec4 eyeVertex, const Vec3& eyeNormal) const
{
    eyeVertex.normalize();
    const Vec3 eyeVertex3 { eyeVertex.data() };
    const float dotResult = 2.0f * eyeVertex3.dot(eyeNormal);
    return eyeVertex3 - (eyeNormal * dotResult);
}

void TexGen::setMatrixStore(const MatrixStore& matrixStore)
{
    m_matrixStore = &matrixStore;
}

void TexGen::enableTexGenS(bool enable)
{
    config.texGenEnableS = enable;
}

void TexGen::enableTexGenT(bool enable)
{
    config.texGenEnableT = enable;
}

void TexGen::enableTexGenR(bool enable)
{
    config.texGenEnableR = enable;
}

void TexGen::setTexGenModeS(TexGenMode mode)
{
    config.texGenModeS = mode;
}

void TexGen::setTexGenModeT(TexGenMode mode)
{
    config.texGenModeT = mode;
}

void TexGen::setTexGenModeR(TexGenMode mode)
{
    config.texGenModeR = mode;
}

void TexGen::setTexGenVecObjS(const Vec4& val)
{
    config.texGenVecObjS = val;
}

void TexGen::setTexGenVecObjT(const Vec4& val)
{
    config.texGenVecObjT = val;
}

void TexGen::setTexGenVecObjR(const Vec4& val)
{
    config.texGenVecObjR = val;
}

void TexGen::setTexGenVecEyeS(const Vec4& val)
{
    config.texGenVecEyeS = m_matrixStore->getNormal().transform(val);
}

void TexGen::setTexGenVecEyeT(const Vec4& val)
{
    config.texGenVecEyeT = m_matrixStore->getNormal().transform(val);
}

void TexGen::setTexGenVecEyeR(const Vec4& val)
{
    config.texGenVecEyeR = m_matrixStore->getNormal().transform(val);
}

} // namespace rr
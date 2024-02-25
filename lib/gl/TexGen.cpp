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
void TexGen::calculateTexGenCoords(const Mat44& modelMatrix, Vec4& st0, const Vec4& v0) const
{
    if (m_texGenEnableS || m_texGenEnableT) [[unlikely]]
    {
        Vec4 v0Transformed;
        if ((m_texGenModeS == TexGenMode::EYE_LINEAR) || (m_texGenModeT == TexGenMode::EYE_LINEAR))
        {
            // TODO: Move this calculation somehow out of this class to increase the performance.
            // Investigate if this functionality is used at all in games.
            modelMatrix.transform(v0Transformed, v0);
        }
        if (m_texGenEnableS)
        {
            switch (m_texGenModeS) {
            case TexGenMode::OBJECT_LINEAR:
                st0[0] = m_texGenVecObjS.dot(v0);
                break;
            case TexGenMode::EYE_LINEAR:
                st0[0] = m_texGenVecEyeS.dot(v0Transformed);
                break;
            case TexGenMode::SPHERE_MAP:
                // TODO: Implement
                break;
            default:
                break;
            }
        }
        if (m_texGenEnableT)
        {
            switch (m_texGenModeT) {
            case TexGenMode::OBJECT_LINEAR:
                st0[1] = m_texGenVecObjT.dot(v0);
                break;
            case TexGenMode::EYE_LINEAR:
                st0[1] = m_texGenVecEyeT.dot(v0Transformed);
                break;
            case TexGenMode::SPHERE_MAP:
                // TODO: Implement
                break;
            default:
                break;
            }
        }
        if (m_texGenEnableR)
        {
            switch (m_texGenModeR) {
            case TexGenMode::OBJECT_LINEAR:
                st0[2] = m_texGenVecObjR.dot(v0);
                break;
            case TexGenMode::EYE_LINEAR:
                st0[2] = m_texGenVecEyeR.dot(v0Transformed);
                break;
            case TexGenMode::SPHERE_MAP:
                // TODO: Implement
                break;
            default:
                break;
            }
        }
    }
}

Vec4 TexGen::calcTexGenEyePlane(const Mat44& mat, const Vec4& plane) const
{
    Mat44 inv{mat};
    inv.invert();
    inv.transpose();
    Vec4 newPlane;
    inv.transform(newPlane, plane);
    return newPlane;
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

void TexGen::setTexGenVecObjS(const Vec4 &val)
{
    m_texGenVecObjS = val;
}

void TexGen::setTexGenVecObjT(const Vec4 &val)
{
    m_texGenVecObjT = val;
}

void TexGen::setTexGenVecObjR(const Vec4 &val)
{
    m_texGenVecObjR = val;
}

void TexGen::setTexGenVecEyeS(const Mat44& modelMatrix, const Vec4 &val)
{
    m_texGenVecEyeS = calcTexGenEyePlane(modelMatrix, val);
}

void TexGen::setTexGenVecEyeT(const Mat44& modelMatrix, const Vec4 &val)
{
    m_texGenVecEyeT = calcTexGenEyePlane(modelMatrix, val);
}

void TexGen::setTexGenVecEyeR(const Mat44& modelMatrix, const Vec4 &val)
{
    m_texGenVecEyeR = calcTexGenEyePlane(modelMatrix, val);
}

} // namespace rr
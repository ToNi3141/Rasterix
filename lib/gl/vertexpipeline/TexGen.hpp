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

#ifndef TEXGEN_HPP
#define TEXGEN_HPP

#include "MatrixStore.hpp"
#include "math/Mat44.hpp"
#include "math/Vec.hpp"

namespace rr
{
class TexGen
{
public:
    enum class TexGenMode
    {
        OBJECT_LINEAR,
        EYE_LINEAR,
        SPHERE_MAP,
        REFLECTION_MAP
    };
    void calculateTexGenCoords(Vec4& st0, const Vec4& v0, const Vec3& n0) const;

    void enableTexGenS(bool enable);
    void enableTexGenT(bool enable);
    void enableTexGenR(bool enable);
    void setTexGenModeS(TexGenMode mode);
    void setTexGenModeT(TexGenMode mode);
    void setTexGenModeR(TexGenMode mode);
    void setTexGenVecObjS(const Vec4& val);
    void setTexGenVecObjT(const Vec4& val);
    void setTexGenVecObjR(const Vec4& val);
    void setTexGenVecEyeS(const Vec4& val);
    void setTexGenVecEyeT(const Vec4& val);
    void setTexGenVecEyeR(const Vec4& val);

    void setMatrixStore(const MatrixStore& matrixStore);

private:
    const MatrixStore* m_matrixStore { nullptr };

    void calculateObjectLinear(Vec4& st0, const Vec4& v0) const;
    void calculateEyeLinear(Vec4& st0, const Vec4& v0) const;
    void calculateSphereMap(Vec4& st0, const Vec4& v0, const Vec3& n0) const;
    void calculateReflectionMap(Vec4& st0, const Vec4& v0, const Vec3& n0) const;

    Vec3 calculateSphereVector(const Vec4& v0, const Vec3& n0) const;
    Vec3 calculateReflectionVector(const Vec4& v0, const Vec3& n0) const;

    bool m_texGenEnableS { false };
    bool m_texGenEnableT { false };
    bool m_texGenEnableR { false };
    TexGenMode m_texGenModeS { TexGenMode::EYE_LINEAR };
    TexGenMode m_texGenModeT { TexGenMode::EYE_LINEAR };
    TexGenMode m_texGenModeR { TexGenMode::EYE_LINEAR };
    Vec4 m_texGenVecObjS { { 1.0f, 0.0f, 0.0f, 0.0f } };
    Vec4 m_texGenVecObjT { { 0.0f, 1.0f, 0.0f, 0.0f } };
    Vec4 m_texGenVecObjR { { 0.0f, 0.0f, 1.0f, 0.0f } };
    Vec4 m_texGenVecEyeS { { 1.0f, 0.0f, 0.0f, 0.0f } };
    Vec4 m_texGenVecEyeT { { 0.0f, 1.0f, 0.0f, 0.0f } };
    Vec4 m_texGenVecEyeR { { 0.0f, 0.0f, 1.0f, 0.0f } };
};

} // namespace rr
#endif // TEXGEN_HPP

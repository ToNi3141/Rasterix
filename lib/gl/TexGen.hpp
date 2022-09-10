#ifndef TEXGEN_HPP
#define TEXGEN_HPP

#include "Vec.hpp"
#include "Mat44.hpp"

class TexGen
{
public:
    enum class TexGenMode
    {
        OBJECT_LINEAR,
        EYE_LINEAR,
        SPHERE_MAP
    };

    void calculateTexGenCoords(const Mat44& modelMatrix, Vec2& st0, const Vec4& v0) const;

    void enableTexGenS(bool enable);
    void enableTexGenT(bool enable);
    void setTexGenModeS(TexGenMode mode);
    void setTexGenModeT(TexGenMode mode);
    void setTexGenVecObjS(const Vec4& val);
    void setTexGenVecObjT(const Vec4& val);
    void setTexGenVecEyeS(const Mat44& modelMatrix, const Vec4& val);
    void setTexGenVecEyeT(const Mat44& modelMatrix, const Vec4& val);
private:
    Vec4 calcTexGenEyePlane(const Mat44& mat, const Vec4& plane) const;

    bool m_texGenEnableS{false};
    bool m_texGenEnableT{false};
    TexGenMode m_texGenModeS{TexGenMode::EYE_LINEAR};
    TexGenMode m_texGenModeT{TexGenMode::EYE_LINEAR};
    Vec4 m_texGenVecObjS{{1.0f, 0.0f, 0.0f, 0.0f}};
    Vec4 m_texGenVecObjT{{0.0f, 1.0f, 0.0f, 0.0f}};
    Vec4 m_texGenVecEyeS{{1.0f, 0.0f, 0.0f, 0.0f}};
    Vec4 m_texGenVecEyeT{{0.0f, 1.0f, 0.0f, 0.0f}};
};

#endif // TEXGEN_HPP

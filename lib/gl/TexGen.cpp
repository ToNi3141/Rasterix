#include "TexGen.hpp"

void TexGen::calculateTexGenCoords(const Mat44& modelMatrix, Vec2& st0, const Vec4& v0) const
{
    if (m_texGenEnableS || m_texGenEnableT)
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

void TexGen::setTexGenModeS(TexGenMode mode)
{
    m_texGenModeS = mode;
}

void TexGen::setTexGenModeT(TexGenMode mode)
{
    m_texGenModeT = mode;
}

void TexGen::setTexGenVecObjS(const Vec4 &val)
{
    m_texGenVecObjS = val;
}

void TexGen::setTexGenVecObjT(const Vec4 &val)
{
    m_texGenVecObjT = val;
}

void TexGen::setTexGenVecEyeS(const Mat44& modelMatrix, const Vec4 &val)
{
    m_texGenVecEyeS = calcTexGenEyePlane(modelMatrix, val);
}

void TexGen::setTexGenVecEyeT(const Mat44& modelMatrix, const Vec4 &val)
{
    m_texGenVecEyeT = calcTexGenEyePlane(modelMatrix, val);
}

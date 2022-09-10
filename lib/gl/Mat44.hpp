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

#ifndef MAT44_HPP
#define MAT44_HPP
#include <array>
#include <math.h>
#include "Vec.hpp"

class Mat44
{
public:
    using ValType = std::array<std::array<float, 4>, 4>;
    Mat44() {}
    Mat44(const ValType& val) { operator=(val); }
    Mat44(const float* val) { operator=(val); }
    ~Mat44() {}
    
    void identity()
    {
        mat[0][0] = 1.0f; mat[1][0] = 0.0f; mat[2][0] = 0.0f; mat[3][0] = 0.0f;
        mat[0][1] = 0.0f; mat[1][1] = 1.0f; mat[2][1] = 0.0f; mat[3][1] = 0.0f;
        mat[0][2] = 0.0f; mat[1][2] = 0.0f; mat[2][2] = 1.0f; mat[3][2] = 0.0f;
        mat[0][3] = 0.0f; mat[1][3] = 0.0f; mat[2][3] = 0.0f; mat[3][3] = 1.0f;
    }

    void transpose()
    {
        const Mat44 m{*this};
        for (int i = 0; i < 4; ++i)
        {
            for (int j = 0; j < 4; ++j)
            {
                mat[i][j] = m[j][i];
            }
        }
    }

    bool invert()
    {
        const float* m = reinterpret_cast<const float*>(&mat[0][0]);
        float inv[16];
        inv[0] = m[5]*m[10]*m[15]-m[5]*m[11]*m[14]-m[9]*m[6]*m[15]+
                m[9]*m[7]*m[14]+m[13]*m[6]*m[11]-m[13]*m[7]*m[10];
        inv[4] = -m[4]*m[10]*m[15]+m[4]*m[11]*m[14]+m[8]*m[6]*m[15]-
                m[8]*m[7]*m[14]-m[12]*m[6]*m[11]+m[12]*m[7]*m[10];
        inv[8] = m[4]*m[9]*m[15]-m[4]*m[11]*m[13]-m[8]*m[5]*m[15]+
                m[8]*m[7]*m[13]+m[12]*m[5]*m[11]-m[12]*m[7]*m[9];
        inv[12]= -m[4]*m[9]*m[14]+m[4]*m[10]*m[13]+m[8]*m[5]*m[14]-
                m[8]*m[6]*m[13]-m[12]*m[5]*m[10]+m[12]*m[6]*m[9];
        inv[1] = -m[1]*m[10]*m[15]+m[1]*m[11]*m[14]+m[9]*m[2]*m[15]-
                m[9]*m[3]*m[14]-m[13]*m[2]*m[11]+m[13]*m[3]*m[10];
        inv[5] = m[0]*m[10]*m[15]-m[0]*m[11]*m[14]-m[8]*m[2]*m[15]+
                m[8]*m[3]*m[14]+m[12]*m[2]*m[11]-m[12]*m[3]*m[10];
        inv[9] = -m[0]*m[9]*m[15]+m[0]*m[11]*m[13]+m[8]*m[1]*m[15]-
                m[8]*m[3]*m[13]-m[12]*m[1]*m[11]+m[12]*m[3]*m[9];
        inv[13]= m[0]*m[9]*m[14]-m[0]*m[10]*m[13]-m[8]*m[1]*m[14]+
                m[8]*m[2]*m[13]+m[12]*m[1]*m[10]-m[12]*m[2]*m[9];
        inv[2] = m[1]*m[6]*m[15]-m[1]*m[7]*m[14]-m[5]*m[2]*m[15]+
                m[5]*m[3]*m[14]+m[13]*m[2]*m[7]-m[13]*m[3]*m[6];
        inv[6] = -m[0]*m[6]*m[15]+m[0]*m[7]*m[14]+m[4]*m[2]*m[15]-
                m[4]*m[3]*m[14]-m[12]*m[2]*m[7]+m[12]*m[3]*m[6];
        inv[10]= m[0]*m[5]*m[15]-m[0]*m[7]*m[13]-m[4]*m[1]*m[15]+
                m[4]*m[3]*m[13]+m[12]*m[1]*m[7]-m[12]*m[3]*m[5];
        inv[14]= -m[0]*m[5]*m[14]+m[0]*m[6]*m[13]+m[4]*m[1]*m[14]-
                m[4]*m[2]*m[13]-m[12]*m[1]*m[6]+m[12]*m[2]*m[5];
        inv[3] = -m[1]*m[6]*m[11]+m[1]*m[7]*m[10]+m[5]*m[2]*m[11]-
                m[5]*m[3]*m[10]-m[9]*m[2]*m[7]+m[9]*m[3]*m[6];
        inv[7] = m[0]*m[6]*m[11]-m[0]*m[7]*m[10]-m[4]*m[2]*m[11]+
                m[4]*m[3]*m[10]+m[8]*m[2]*m[7]-m[8]*m[3]*m[6];
        inv[11]= -m[0]*m[5]*m[11]+m[0]*m[7]*m[9]+m[4]*m[1]*m[11]-
                m[4]*m[3]*m[9]-m[8]*m[1]*m[7]+m[8]*m[3]*m[5];
        inv[15]= m[0]*m[5]*m[10]-m[0]*m[6]*m[9]-m[4]*m[1]*m[10]+
                m[4]*m[2]*m[9]+m[8]*m[1]*m[6]-m[8]*m[2]*m[5];
        float det = m[0]*inv[0]+m[1]*inv[4]+m[2]*inv[8]+m[3]*inv[12];
        if (det == 0.0f) return false;
        det = 1.0f / det;
        for (uint8_t i = 0; i < 4; i++)
        {
            for (uint8_t j = 0; j < 4; j++)
            {
                mat[i][j] = inv[(i * 4) + j] * det;
            }
        }
        return true;
    }

    void transform(Vec4& dst, const Vec4& src) const
    {
        const float src0 = src[0];
        const float src1 = src[1];
        const float src2 = src[2];
        const float src3 = src[3];

        dst[0] = src0 * mat[0][0] + src1 * mat[1][0] + src2 * mat[2][0];
        dst[1] = src0 * mat[0][1] + src1 * mat[1][1] + src2 * mat[2][1];
        dst[2] = src0 * mat[0][2] + src1 * mat[1][2] + src2 * mat[2][2];
        dst[3] = src0 * mat[0][3] + src1 * mat[1][3] + src2 * mat[2][3];

        // Typically the w value of a Vec4 is 1.0. So threat this as a special case.
        // It will probably only speed up when no FPU is available ...
        if (src3 == 1.0f)
        {
            dst[0] += mat[3][0];
            dst[1] += mat[3][1];
            dst[2] += mat[3][2];
            dst[3] += mat[3][3];
        }
        else
        {
            dst[0] += src3 * mat[3][0];
            dst[1] += src3 * mat[3][1];
            dst[2] += src3 * mat[3][2];
            dst[3] += src3 * mat[3][3];
        }
    }

    void transform(Vec3& dst, const Vec3 &src) const
    {
        const float src0 = src[0];
        const float src1 = src[1];
        const float src2 = src[2];

        dst[0] = src0 * mat[0][0] + src1 * mat[1][0] + src2 * mat[2][0];
        dst[1] = src0 * mat[0][1] + src1 * mat[1][1] + src2 * mat[2][1];
        dst[2] = src0 * mat[0][2] + src1 * mat[1][2] + src2 * mat[2][2];
    }

    void transform(Vec4* __restrict dst, const Vec4* const __restrict src, const std::size_t cnt)
    {
        // __restrict and alignedCnt to make it easier for the compiler to vectorize  
        // See https://developer.arm.com/documentation/dht0002/a/Introducing-NEON/Developing-for-NEON/Automatic-vectorization
        const std::size_t alignedCnt = (cnt & ~3);
        for (std::size_t i = 0; i < alignedCnt; i += 4) 
        {
            dst[i][0] = src[i][0] * mat[0][0] + src[i][1] * mat[1][0] + src[i][2] * mat[2][0] + src[i][3] * mat[3][0];
            dst[i][1] = src[i][0] * mat[0][1] + src[i][1] * mat[1][1] + src[i][2] * mat[2][1] + src[i][3] * mat[3][1];
            dst[i][2] = src[i][0] * mat[0][2] + src[i][1] * mat[1][2] + src[i][2] * mat[2][2] + src[i][3] * mat[3][2];
            dst[i][3] = src[i][0] * mat[0][3] + src[i][1] * mat[1][3] + src[i][2] * mat[2][3] + src[i][3] * mat[3][3];
        }
    }

    void transform(Vec3* __restrict dst, const Vec3* const __restrict src, const std::size_t cnt)
    {
        for (std::size_t i = 0; i < cnt; i += 3)
        {
            dst[i][0] = src[i][0] * mat[0][0] + src[i][1] * mat[1][0] + src[i][2] * mat[2][0];
            dst[i][1] = src[i][0] * mat[0][1] + src[i][1] * mat[1][1] + src[i][2] * mat[2][1];
            dst[i][2] = src[i][0] * mat[0][2] + src[i][1] * mat[1][2] + src[i][2] * mat[2][2];
        }
    }

    void operator*= (const Mat44& rhs)
    {
        Mat44 m{*this};
        for (uint8_t i = 0; i < 4; i++)
        {
            for (uint8_t j = 0; j < 4; j++)
            {
                mat[i][j] = m[i][0] * rhs[0][j] + m[i][1] * rhs[1][j] +
                        m[i][2] * rhs[2][j] + m[i][3] * rhs[3][j];
            }
        }
    }

    void operator= (const float* m)
    {
        for (uint8_t i = 0; i < 4; i++)
        {
            for (uint8_t j = 0; j < 4; j++)
            {
                mat[i][j] = m[(i * 4) + j];
            }
        }
    }

    void operator= (const ValType& m)
    {
        operator= (&m[0][0]);
    }

    std::array<float, 4>& operator[] (const uint8_t rhs)
    {
        return mat[rhs];
    }

    const std::array<float, 4>& operator[] (const uint8_t rhs) const
    {
        return mat[rhs];
    }

    ValType mat;
};

inline Mat44 operator* (const Mat44& lhs, const Mat44& rhs)
{
    Mat44 mat;
    for (uint8_t i = 0; i < 4; i++)
    {
        for (uint8_t j = 0; j < 4; j++)
        {
            mat[i][j] = lhs[i][0] * rhs[0][j] + lhs[i][1] * rhs[1][j] +
                    lhs[i][2] * rhs[2][j] + lhs[i][3] * rhs[3][j];
        }
    }
    return mat;
}


#endif // MAT44_HPP

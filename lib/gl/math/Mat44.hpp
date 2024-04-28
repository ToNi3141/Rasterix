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

#ifndef MAT44_HPP
#define MAT44_HPP
#include <array>
#include <math.h>
#include "math/Vec.hpp"

namespace rr
{
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
        if (det == 0.0f) [[unlikely]] return false; 
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

#if defined(__ARM_NEON)
    // ARM NEON optimized version which is much faster than the compiler generated code
    void transform(Vec4& __restrict dst, const Vec4& src) const
    {
        asm volatile (
            "vld1.32    {d0, d1}, [%1]      \n\t" // q0 = src

            "vld1.32    {d18, d19}, [%0]!   \n\t" // q9 = mat[0] (first column)
            "vmul.f32   q13, q9, d0[0]      \n\t" // q13 = mat[0] * src

            "vld1.32    {d18, d19}, [%0]!   \n\t" // q9 = mat[1] (second column)
            "vmla.f32   q13, q9, d0[1]      \n\t" // q13 += mat[1] * src

            "vld1.32    {d18, d19}, [%0]!   \n\t" // q9 = mat[2] (third column)
            "vmla.f32   q13, q9, d1[0]      \n\t" // q13 += mat[2] * src
 
            "vld1.32    {d18, d19}, [%0]!   \n\t" // q9 = mat[3] (fourth column)
            "vmla.f32   q13, q9, d1[1]      \n\t" // q13 += mat[3] * src
            
            "vst1.32    {d26, d27}, [%2]    \n\t"	// dst = q13
            : 
            : "r"(&mat[0][0]), "r"(src.vec.data()), "r"(dst.vec.data()) 
            : "q0", "q9", "q13", "memory"
        );
    }
#else
    void transform(Vec4& __restrict dst, const Vec4& src) const
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
#endif

    inline Vec4 transform(const Vec4& src) const
    {
        Vec4 dst;
        transform(dst, src);
        return dst;
    }

    inline void transform(Vec3& dst, const Vec3 &src) const
    {
        const float src0 = src[0];
        const float src1 = src[1];
        const float src2 = src[2];

        dst[0] = src0 * mat[0][0] + src1 * mat[1][0] + src2 * mat[2][0];
        dst[1] = src0 * mat[0][1] + src1 * mat[1][1] + src2 * mat[2][1];
        dst[2] = src0 * mat[0][2] + src1 * mat[1][2] + src2 * mat[2][2];
    }

    inline Vec3 transform(const Vec3& src) const
    {
        Vec3 dst;
        transform(dst, src);
        return dst;
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
        uint8_t k = 0;
        for (uint8_t i = 0; i < 4; i++)
        {
            for (uint8_t j = 0; j < 4; j++)
            {
                mat[i][j] = m[k++];
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

} // namespace rr

#endif // MAT44_HPP

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

#ifndef TNL_HPP
#define TNL_HPP

#include "Vec.hpp"
#include "IRenderer.hpp"
#include "Mat44.hpp"
#include "Clipper.hpp"
#include "Lighting.hpp"
#include "TexGen.hpp"

class TnL
{
public:
    enum CullMode
    {
        BACK,
        FRONT,
        FRONT_AND_BACK
    };

    struct RenderObj
    {
        enum DrawMode
        {
            TRIANGLES,
            TRIANGLE_FAN,
            TRIANGLE_STRIP
        };

        enum Type
        {
            BYTE,
            UNSIGNED_BYTE,
            SHORT,
            UNSIGNED_SHORT,
            FLOAT,
            UNSIGNED_INT
        };

        DrawMode drawMode;
        uint32_t count;
        Vec4 vertexColor;

        bool vertexArrayEnabled;
        uint8_t vertexSize;
        Type vertexType;
        uint32_t vertexStride;
        const void* vertexPointer;

        bool texCoordArrayEnabled;
        uint8_t texCoordSize;
        Type texCoordType;
        uint32_t texCoordStride;
        const void* texCoordPointer;

        bool normalArrayEnabled;
        Type normalType;
        uint32_t normalStride;
        const void* normalPointer;

        bool colorArrayEnabled;
        uint8_t colorSize;
        Type colorType;
        uint32_t colorStride;
        const void* colorPointer;

        bool indicesEnabled;
        Type indicesType;
        const void* indicesPointer;

        uint32_t arrayOffset;

        template <typename T>
        bool getFromArray(T& vec, const Type type, const void* arr, const uint32_t stride, uint8_t size, const uint32_t index) const
        {
            if (arr)
            {
                const uint32_t indexWithStride = (stride == 0) ? index * size : index * stride;
                const int8_t* a = reinterpret_cast<const int8_t*>(arr);
                if (stride == 0) {
                    switch (type)
                    {
                    case Type::BYTE:
                        vec.fromArray(a + (indexWithStride * 1), size);
                        break;
                    case Type::UNSIGNED_BYTE:
                        vec.fromArray(reinterpret_cast<const uint8_t*>(a + (indexWithStride * 1)), size);
                        break;
                    case Type::SHORT:
                        vec.fromArray(reinterpret_cast<const int16_t*>(a + (indexWithStride * 2)), size);
                        break;
                    case Type::UNSIGNED_SHORT:
                        vec.fromArray(reinterpret_cast<const uint16_t*>(a + (indexWithStride * 2)), size);
                        break;
                    case Type::UNSIGNED_INT:
                        vec.fromArray(reinterpret_cast<const int32_t*>(a + (indexWithStride * 4)), size);
                        break;
                    default:
                        vec.fromArray(reinterpret_cast<const float*>(a + (indexWithStride * 4)), size);
                        break;
                    }
                }
                else
                {
                    switch (type)
                    {
                    case Type::BYTE:
                        vec.fromArray(a + indexWithStride, size);
                        break;
                    case Type::UNSIGNED_BYTE:
                        vec.fromArray(reinterpret_cast<const uint8_t*>(a + indexWithStride), size);
                        break;
                    case Type::SHORT:
                        vec.fromArray(reinterpret_cast<const int16_t*>(a + indexWithStride), size);
                        break;
                    case Type::UNSIGNED_SHORT:
                        vec.fromArray(reinterpret_cast<const uint16_t*>(a + indexWithStride), size);
                        break;
                    case Type::UNSIGNED_INT:
                        vec.fromArray(reinterpret_cast<const int32_t*>(a + indexWithStride), size);
                        break;
                    default:
                        vec.fromArray(reinterpret_cast<const float*>(a + indexWithStride), size);
                        break;
                    }
                }

                return true;
            }
            return false;
        }

        bool getVertex(Vec4& vec, const uint32_t index) const
        {
            vec.initHomogeneous();
            return getFromArray(vec, vertexType, vertexPointer, vertexStride, vertexSize, index);
        }

        bool getTexCoord(Vec2& vec, const uint32_t index) const
        {
            return getFromArray(vec, texCoordType, texCoordPointer, texCoordStride, texCoordSize, index);
        }

        bool getColor(Vec4& vec, const uint32_t index) const
        {
            bool retVal = getFromArray(vec, colorType, colorPointer, colorStride, colorSize, index);
            if (retVal)
            {
                switch (colorType) {
                case Type::UNSIGNED_BYTE:
                case Type::UNSIGNED_SHORT:
                case Type::UNSIGNED_INT:
                    // Map unsigned values to 0.0 .. 1.0
                    vec *= 1.0f / 255.0f;
                    break;
                case Type::BYTE:
                case Type::SHORT:
                    // Map signed values to -1.0 .. 1.0
                    vec *= 1.0f / 127.0f;
                    break;
                default:
                    // Other types like floats can be used as they are
                    break;
                }
            }
            return retVal;
        }

        bool getNormal(Vec3& vec, const uint32_t index) const
        {
            return getFromArray(vec, normalType, normalPointer, normalStride, 3, index);
        }

        uint32_t getIndex(const uint32_t index) const
        {
            if (indicesEnabled)
            {
                switch (indicesType)
                {
                case Type::BYTE:
                case Type::UNSIGNED_BYTE:
                    return static_cast<const uint8_t*>(indicesPointer)[index];
                case Type::SHORT:
                case Type::UNSIGNED_SHORT:
                    return static_cast<const uint16_t*>(indicesPointer)[index];
                case Type::UNSIGNED_INT:
                    return static_cast<const uint32_t*>(indicesPointer)[index];
                default:
                    return index;
                }
            }
            return index;
        }
    };

    struct Triangle
    {
        Vec4 v0;
        Vec4 v1;
        Vec4 v2;
        Vec2 st0;
        Vec2 st1;
        Vec2 st2;
        Vec3 n0;
        Vec3 n1;
        Vec3 n2;
        Vec4 color0;
        Vec4 color1;
        Vec4 color2;
    };

    TnL(Lighting& lighting, TexGen& texGen);

    bool drawObj(IRenderer& renderer, const RenderObj& obj);
    bool drawTriangle(IRenderer &renderer, const Triangle& triangle);
    void setViewport(const int16_t x, const int16_t y, const int16_t width, const int16_t height);
    void setDepthRange(const float zNear, const float zFar);
    void setModelProjectionMatrix(const Mat44& m);
    void setModelMatrix(const Mat44& m);
    void setNormalMatrix(const Mat44& m);

    void enableCulling(bool enable);
    void setCullMode(CullMode mode);
private:
    inline void viewportTransform(Vec4 &v0, Vec4 &v1, Vec4 &v2);
    inline void viewportTransform(Vec4 &v);
    inline void perspectiveDivide(Vec4& v);

    Mat44 m_t; // ModelViewProjection
    Mat44 m_m; // ModelView
    Mat44 m_n; // Normal

    float m_depthRangeZNear = 0.0f;
    float m_depthRangeZFar = 1.0f;
    int16_t m_viewportX = 0;
    int16_t m_viewportY = 0;
    int16_t m_viewportHeight = 0;
    int16_t m_viewportWidth = 0;
    float m_viewportXShift = 0.0f;
    float m_viewportYShift = 0.0f;
    float m_viewportHeightHalf = 0.0f;
    float m_viewportWidthHalf = 0.0f;

    bool m_enableCulling{false};
    CullMode m_cullMode{CullMode::BACK};

    Lighting& m_lighting;
    TexGen& m_texGen;
};

#endif // TNL_HPP

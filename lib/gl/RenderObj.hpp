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


#ifndef RENDEROBJ_HPP
#define RENDEROBJ_HPP

#include "Vec.hpp"
#include "IRenderer.hpp"
#include <array>
#include <bitset>

class RenderObj
{
public:
    static constexpr uint8_t MAX_TMU_COUNT { IRenderer::MAX_TMU_COUNT };

    enum DrawMode
    {
        TRIANGLES,
        TRIANGLE_FAN,
        TRIANGLE_STRIP,
        POLYGON,
        QUADS,
        QUAD_STRIP,
        LINES,
        LINE_STRIP,
        LINE_LOOP
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

    void logCurrentConfig() const;

    bool vertexArrayEnabled() const { return m_vertexArrayEnabled; }
    bool getVertex(Vec4& vec, const uint32_t index) const;
    std::bitset<MAX_TMU_COUNT> texCoordArrayEnabled() const { return m_texCoordArrayEnabled; }
    bool getTexCoord(const uint8_t tmu, Vec4& vec, const uint32_t index) const;
    bool colorArrayEnabled() const { return m_colorArrayEnabled; }
    bool getColor(Vec4& vec, const uint32_t index) const;
    const Vec4& getVertexColor() const { return m_vertexColor; }
    bool normalArrayEnabled() const { return m_normalArrayEnabled; }
    bool getNormal(Vec3& vec, const uint32_t index) const;
    bool isLine() const;

    uint32_t getIndex(const uint32_t index) const;
    DrawMode getDrawMode() const { return m_drawMode; }
    std::size_t getCount() const { return m_count; }
    

    void enableVertexArray(bool enable) { m_vertexArrayEnabled = enable; }
    void setVertexSize(uint8_t size) { m_vertexSize = size; }
    void setVertexType(Type type) { m_vertexType = type; }
    void setVertexStride(uint32_t stride) { m_vertexStride = stride; }
    void setVertexPointer(const void* ptr) { m_vertexPointer = ptr; }

    void enableTexCoordArray(const uint8_t tmu, bool enable) { m_texCoordArrayEnabled[tmu] = enable; }
    void setTexCoordSize(const uint8_t tmu, uint8_t size) { m_texCoordSize[tmu] = size; }
    void setTexCoordType(const uint8_t tmu, Type type) { m_texCoordType[tmu] = type; }
    void setTexCoordStride(const uint8_t tmu, uint32_t stride) { m_texCoordStride[tmu] = stride; }
    void setTexCoordPointer(const uint8_t tmu, const void* ptr) { m_texCoordPointer[tmu] = ptr; }
    void setTexCoord(const uint8_t tmu, const Vec4& texCoord){ m_texCoord[tmu] = texCoord; }

    void enableNormalArray(bool enable) { m_normalArrayEnabled = enable; }
    void setNormalType(Type type) { m_normalType = type; }
    void setNormalStride(uint32_t stride) { m_normalStride = stride; }
    void setNormalPointer(const void* ptr) { m_normalPointer = ptr; }
    void setNormal(const Vec3& normal) { m_normal = normal; }

    void enableColorArray(bool enable) { m_colorArrayEnabled = enable; }
    void setColorSize(uint8_t size) { m_colorSize = size; }
    void setColorType(Type type) { m_colorType = type; }
    void setColorStride(uint32_t stride) { m_colorStride = stride; }
    void setColorPointer(const void* ptr) { m_colorPointer = ptr; }
    void setVertexColor(const Vec4& color) { m_vertexColor = color; }

    void setDrawMode(DrawMode mode) { m_drawMode = mode; }
    
    void enableIndices(bool enable) { m_indicesEnabled = enable; }
    void setCount(std::size_t count) { m_count = count; }
    void setIndicesType(Type type) { m_indicesType = type; }
    void setIndicesPointer(const void* ptr) { m_indicesPointer = ptr; }
    void setArrayOffset(uint32_t offset) { m_arrayOffset = offset; }

private:
    template <typename T>
    bool getFromArray(T& vec, const Type type, const void* arr, const uint32_t stride, uint8_t size, const uint32_t index) const
    {
        if (arr)
        {
            const int8_t* a = reinterpret_cast<const int8_t*>(arr);
            if (stride == 0) {
                const uint32_t indexWithStride = index * size;
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
                const uint32_t indexWithStride = index * stride;
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

    const char* drawModeToString(const DrawMode drawMode) const;
    const char* typeToString(const Type type) const;

    DrawMode m_drawMode;
    std::size_t m_count;

    bool m_vertexArrayEnabled;
    uint8_t m_vertexSize;
    Type m_vertexType;
    uint32_t m_vertexStride;
    const void* m_vertexPointer;

    std::bitset<MAX_TMU_COUNT> m_texCoordArrayEnabled;
    std::array<uint8_t, MAX_TMU_COUNT> m_texCoordSize;
    std::array<Type, MAX_TMU_COUNT> m_texCoordType;
    std::array<uint32_t, MAX_TMU_COUNT> m_texCoordStride;
    std::array<const void*, MAX_TMU_COUNT> m_texCoordPointer;
    std::array<Vec4, MAX_TMU_COUNT> m_texCoord {{{{ 0.0f, 0.0f, 0.0f, 1.0f }}, {{ 0.0f, 0.0f, 0.0f, 1.0f }}}}; // TODO: Make it variable.

    bool m_normalArrayEnabled;
    Type m_normalType;
    uint32_t m_normalStride;
    const void* m_normalPointer;
    Vec3 m_normal {{ 0.0f, 0.0f, 1.0f }};

    bool m_colorArrayEnabled;
    uint8_t m_colorSize;
    Type m_colorType;
    uint32_t m_colorStride;
    const void* m_colorPointer;
    Vec4 m_vertexColor {{ 1.0f, 1.0f, 1.0f, 1.0f }};

    bool m_indicesEnabled;
    Type m_indicesType;
    const void* m_indicesPointer;

    uint32_t m_arrayOffset;
};
#endif // RENDEROBJ_HPP

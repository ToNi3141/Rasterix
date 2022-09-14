#ifndef RENDEROBJ_HPP
#define RENDEROBJ_HPP

#include "Vec.hpp"

class RenderObj
{
public:
    enum DrawMode
    {
        TRIANGLES,
        TRIANGLE_FAN,
        TRIANGLE_STRIP,
        QUAD_STRIP
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

    bool vertexArrayEnabled() const { return m_vertexArrayEnabled; }
    bool getVertex(Vec4& vec, const uint32_t index) const;
    bool texCoordArrayEnabled() const { return m_texCoordArrayEnabled; }
    bool getTexCoord(Vec2& vec, const uint32_t index) const;
    bool colorArrayEnabled() const { return m_colorArrayEnabled; }
    bool getColor(Vec4& vec, const uint32_t index) const;
    bool normalArrayEnabled() const { return m_normalArrayEnabled; }
    bool getNormal(Vec3& vec, const uint32_t index) const;

    uint32_t getIndex(const uint32_t index) const;
    DrawMode getDrawMode() const { return m_drawMode; }
    std::size_t getCount() const { return m_count; }
    const Vec4& getVertexColor() const { return m_vertexColor; }

    void enableVertexArray(bool enable) { m_vertexArrayEnabled = enable; }
    void setVertexSize(uint8_t size) { m_vertexSize = size; }
    void setVertexType(Type type) { m_vertexType = type; }
    void setVertexStride(uint32_t stride) { m_vertexStride = stride; }
    void setVertexPointer(const void* ptr) { m_vertexPointer = ptr; }

    void enableTexCoordArray(bool enable) { m_texCoordArrayEnabled = enable; }
    void setTexCoordSize(uint8_t size) { m_texCoordSize = size; }
    void setTexCoordType(Type type) { m_texCoordType = type; }
    void setTexCoordStride(uint32_t stride) { m_texCoordStride = stride; }
    void setTexCoordPointer(const void* ptr) { m_texCoordPointer = ptr; }

    void enableNormalArray(bool enable) { m_normalArrayEnabled = enable; }
    void setNormalType(Type type) { m_normalType = type; }
    void setNormalStride(uint32_t stride) { m_normalStride = stride; }
    void setNormalPointer(const void* ptr) { m_normalPointer = ptr; }

    void enableColorArray(bool enable) { m_colorArrayEnabled = enable; }
    void setColorSize(uint8_t size) { m_colorSize = size; }
    void setColorType(Type type) { m_colorType = type; }
    void setColorStride(uint32_t stride) { m_colorStride = stride; }
    void setColorPointer(const void* ptr) { m_colorPointer = ptr; }

    void setDrawMode(DrawMode mode) { m_drawMode = mode; }
    void setVertexColor(const Vec4& color) { m_vertexColor = color; }

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

    DrawMode m_drawMode;
    std::size_t m_count;
    Vec4 m_vertexColor;

    bool m_vertexArrayEnabled;
    uint8_t m_vertexSize;
    Type m_vertexType;
    uint32_t m_vertexStride;
    const void* m_vertexPointer;

    bool m_texCoordArrayEnabled;
    uint8_t m_texCoordSize;
    Type m_texCoordType;
    uint32_t m_texCoordStride;
    const void* m_texCoordPointer;

    bool m_normalArrayEnabled;
    Type m_normalType;
    uint32_t m_normalStride;
    const void* m_normalPointer;

    bool m_colorArrayEnabled;
    uint8_t m_colorSize;
    Type m_colorType;
    uint32_t m_colorStride;
    const void* m_colorPointer;

    bool m_indicesEnabled;
    Type m_indicesType;
    const void* m_indicesPointer;

    uint32_t m_arrayOffset;
};
#endif // RENDEROBJ_HPP

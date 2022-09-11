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

    bool getVertex(Vec4& vec, const uint32_t index) const;
    bool getTexCoord(Vec2& vec, const uint32_t index) const;
    bool getColor(Vec4& vec, const uint32_t index) const;
    bool getNormal(Vec3& vec, const uint32_t index) const;
    uint32_t getIndex(const uint32_t index) const;

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

    static constexpr std::size_t MAX_VERTEX_CNT { 252 }; // TODO: Static assert that it must be dividable with 3 and 4
    static_assert(MAX_VERTEX_CNT % 4 == 0, "MAX_VERTEX_CNT must be dividable through 4 (used for GL_QUADS");
    static_assert(MAX_VERTEX_CNT % 3 == 0, "MAX_VERTEX_CNT must be dividable through 3 (used for GL_TRIANGLES");
    static constexpr std::size_t OVERLAP { 2 };
    std::array<Vec4, MAX_VERTEX_CNT + OVERLAP> transformedVertex;
    std::array<Vec4, MAX_VERTEX_CNT + OVERLAP> transformedColor;
    std::array<Vec2, MAX_VERTEX_CNT + OVERLAP> transformedTexCoord;

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
};
#endif // RENDEROBJ_HPP

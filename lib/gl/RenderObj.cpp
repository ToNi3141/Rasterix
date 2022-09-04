#include "RenderObj.hpp"

bool RenderObj::getVertex(Vec4& vec, const uint32_t index) const
{
    vec.initHomogeneous();
    return getFromArray(vec, vertexType, vertexPointer, vertexStride, vertexSize, index);
}

bool RenderObj::getTexCoord(Vec2& vec, const uint32_t index) const
{
    return getFromArray(vec, texCoordType, texCoordPointer, texCoordStride, texCoordSize, index);
}

bool RenderObj::getColor(Vec4& vec, const uint32_t index) const
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

bool RenderObj::getNormal(Vec3& vec, const uint32_t index) const
{
    return getFromArray(vec, normalType, normalPointer, normalStride, 3, index);
}

uint32_t RenderObj::getIndex(const uint32_t index) const
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
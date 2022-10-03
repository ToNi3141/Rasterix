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


#include "RenderObj.hpp"

bool RenderObj::getVertex(Vec4& vec, const uint32_t index) const
{
    vec.initHomogeneous();
    return getFromArray(vec, m_vertexType, m_vertexPointer, m_vertexStride, m_vertexSize, index);
}

bool RenderObj::getTexCoord(Vec4& vec, const uint32_t index) const
{
    vec.initHomogeneous();
    return getFromArray(vec, m_texCoordType, m_texCoordPointer, m_texCoordStride, m_texCoordSize, index);
}

bool RenderObj::getColor(Vec4& vec, const uint32_t index) const
{
    bool retVal = getFromArray(vec, m_colorType, m_colorPointer, m_colorStride, m_colorSize, index);
    if (retVal)
    {
        switch (m_colorType) {
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
    return getFromArray(vec, m_normalType, m_normalPointer, m_normalStride, 3, index);
}

uint32_t RenderObj::getIndex(const uint32_t index) const
{
    if (m_indicesEnabled)
    {
        switch (m_indicesType)
        {
        case Type::BYTE:
        case Type::UNSIGNED_BYTE:
            return static_cast<const uint8_t*>(m_indicesPointer)[index];
        case Type::SHORT:
        case Type::UNSIGNED_SHORT:
            return static_cast<const uint16_t*>(m_indicesPointer)[index];
        case Type::UNSIGNED_INT:
            return static_cast<const uint32_t*>(m_indicesPointer)[index];
        default:
            return index;
        }
    }
    return index + m_arrayOffset;
}

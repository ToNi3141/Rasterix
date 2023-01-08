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
#include <spdlog/spdlog.h>

namespace rr
{
bool RenderObj::isLine() const
{
    return (getDrawMode() == DrawMode::LINES) || (getDrawMode() == DrawMode::LINE_LOOP) || (getDrawMode() == DrawMode::LINE_STRIP);
}

bool RenderObj::getVertex(Vec4& vec, const uint32_t index) const
{
    vec.initHomogeneous();
    return getFromArray(vec, m_vertexType, m_vertexPointer, m_vertexStride, m_vertexSize, index);
}

bool RenderObj::getTexCoord(const uint8_t tmu, Vec4& vec, const uint32_t index) const
{
    if (texCoordArrayEnabled()[tmu])
    {
        vec.initHomogeneous();
        return getFromArray(vec, m_texCoordType[tmu], m_texCoordPointer[tmu], m_texCoordStride[tmu], m_texCoordSize[tmu], index);
    }
    else
    {
        vec = m_texCoord[tmu];
    }
    return true;
}

bool RenderObj::getColor(Vec4& vec, const uint32_t index) const
{
    if (colorArrayEnabled())
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
    else
    {
        vec = m_vertexColor;
    }
    return true;
}

bool RenderObj::getNormal(Vec3& vec, const uint32_t index) const
{
    if (normalArrayEnabled())
    {
        return getFromArray(vec, m_normalType, m_normalPointer, m_normalStride, 3, index);
    }
    else
    {
        vec = m_normal;
    }
    return true;
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

const char* RenderObj::drawModeToString(const DrawMode drawMode) const 
{
    switch (drawMode)
    {
    case TRIANGLES:
        return "TRIANGLES";
    
    case TRIANGLE_FAN:
        return "TRIANGLE_FAN";
    
    case TRIANGLE_STRIP:
        return "TRIANGLE_STRIP";
    
    case POLYGON:
        return "POLYGON";
    
    case QUADS:
        return "QUADS";
    
    case QUAD_STRIP:
        return "QUAD_STRIP";
    
    case LINES:
        return "LINES";
    
    case LINE_STRIP:
        return "LINE_STRIP";
    
    case LINE_LOOP:
        return "LINE_LOOP";
    
    default:
        return "UNKNWON";
    }
}

const char* RenderObj::typeToString(const Type type) const
{
    switch (type)
    {
    case BYTE:
        return "BYTE";
    
    case UNSIGNED_BYTE:
        return "UNSIGNED_BYTE";
    
    case SHORT:
        return "SHORT";
    
    case UNSIGNED_SHORT:
        return "UNSIGNED_SHORT";
    
    case FLOAT:
        return "FLOAT";
    
    case UNSIGNED_INT:
        return "UNSIGNED_INT";
    
    default:
        return "UNKNWON";
    }
}

void RenderObj::logCurrentConfig() const 
{
    SPDLOG_DEBUG("RenderObj:");
    SPDLOG_DEBUG("  m_indicesEnabled {}", m_indicesEnabled);
    SPDLOG_DEBUG("  m_indicesType {}", m_indicesType);
    SPDLOG_DEBUG("  m_arrayOffset {}", m_arrayOffset);
    SPDLOG_DEBUG("  getDrawMode {}", drawModeToString(getDrawMode()));
    SPDLOG_DEBUG("  getCount {}", getCount());
    SPDLOG_DEBUG("  VertexArray:");
    SPDLOG_DEBUG("      vertexArrayEnabled {}", vertexArrayEnabled());
    SPDLOG_DEBUG("      m_vertexSize {}", m_vertexSize);
    SPDLOG_DEBUG("      m_vertexType {}", typeToString(m_vertexType));
    SPDLOG_DEBUG("      m_vertexStride {}", m_vertexStride);
    for (uint32_t i = 0; i < MAX_TMU_COUNT; i++)
    {
        SPDLOG_DEBUG("  TexCoordArray {}:", i);
        SPDLOG_DEBUG("      m_texCoordArrayEnabled {}", texCoordArrayEnabled()[i]);
        if (texCoordArrayEnabled()[i])
        {
            SPDLOG_DEBUG("      m_texCoordSize {}", m_texCoordSize[i]);
            SPDLOG_DEBUG("      m_texCoordType {}", typeToString(m_texCoordType[i]));
            SPDLOG_DEBUG("      m_texCoordStride {}", m_texCoordStride[i]);
            SPDLOG_DEBUG("      m_texCoord ({}, {}, {}, {})", m_texCoord[i][0], m_texCoord[i][1], m_texCoord[i][2], m_texCoord[i][3]);
        }
    }
    SPDLOG_DEBUG("  colorArrayEnabled {}", colorArrayEnabled());
    SPDLOG_DEBUG("      m_colorSize {}", m_colorSize);
    SPDLOG_DEBUG("      m_colorType {}", typeToString(m_colorType));
    SPDLOG_DEBUG("      m_colorStride {}", m_colorStride);
    SPDLOG_DEBUG("      m_vertexColor ({}, {}, {}, {})", m_vertexColor[0], m_vertexColor[1], m_vertexColor[2], m_vertexColor[3]);

    SPDLOG_DEBUG("  normalArrayEnabled {}", normalArrayEnabled());
    SPDLOG_DEBUG("      m_normalType {}", typeToString(m_normalType));
    SPDLOG_DEBUG("      m_normalStride {}", m_normalStride);
    SPDLOG_DEBUG("      m_normal ({}, {}, {})", m_normal[0], m_normal[1], m_normal[2]);
}

} // namespace rr
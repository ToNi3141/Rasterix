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


#ifndef VERTEXQUEUE_HPP_
#define VERTEXQUEUE_HPP_

#include "math/Vec.hpp"
#include "RenderObj.hpp"
#include <vector>

namespace rr
{
class VertexQueue 
{
public:
    using DrawMode = RenderObj::DrawMode;
    using Type = RenderObj::Type;

    void setActiveTexture(const uint8_t tmu) { m_tmu = tmu; }

    void begin(const DrawMode drawMode)
    {
        m_beginMode = drawMode;
        m_textureVertexBuffer.clear();
        m_vertexBuffer.clear();
        m_normalVertexBuffer.clear();
        m_colorVertexBuffer.clear();
    }
    void addVertex(const Vec4& vertex)
    {
        m_vertexBuffer.push_back(vertex);
        m_normalVertexBuffer.push_back(m_normal);
        m_textureVertexBuffer.push_back(m_textureCoord);
        m_colorVertexBuffer.push_back(m_vertexColor);
    }
    void setColor(const Vec4& color) { m_vertexColor = color; m_objPtr.setVertexColor(color); }
    void setNormal(const Vec3& normal) { m_normal = normal; m_objPtr.setNormal(normal); }
    void setTexCoord(const Vec4& texCoord) { m_textureCoord[0] = texCoord; m_objPtr.setTexCoord(m_tmu, texCoord); }
    void setMultiTexCoord(const uint8_t tmu, const Vec4& texCoord) { m_textureCoord[tmu] = texCoord; m_objPtr.setTexCoord(tmu, texCoord); }
    const RenderObj& end()
    {
        m_objBeginEnd.reset();

        m_objBeginEnd.enableVertexArray(!m_vertexBuffer.empty());
        m_objBeginEnd.setVertexSize(4);
        m_objBeginEnd.setVertexType(RenderObj::Type::FLOAT);
        m_objBeginEnd.setVertexStride(0);
        m_objBeginEnd.setVertexPointer(m_vertexBuffer.data());

        for (uint8_t i = 0; i < RenderObj::MAX_TMU_COUNT; i++)
        {
            m_objBeginEnd.enableTexCoordArray(i, !m_textureVertexBuffer.empty());
            m_objBeginEnd.setTexCoordSize(i, 4);
            m_objBeginEnd.setTexCoordType(i, RenderObj::Type::FLOAT);
            m_objBeginEnd.setTexCoordStride(i, RenderObj::MAX_TMU_COUNT * sizeof(Vec4));
            m_objBeginEnd.setTexCoordPointer(i, m_textureVertexBuffer.data() + i);
        }

        m_objBeginEnd.enableNormalArray(!m_normalVertexBuffer.empty());
        m_objBeginEnd.setNormalType(RenderObj::Type::FLOAT);
        m_objBeginEnd.setNormalStride(0);
        m_objBeginEnd.setNormalPointer(m_normalVertexBuffer.data());

        m_objBeginEnd.enableColorArray(!m_colorVertexBuffer.empty());
        m_objBeginEnd.setColorSize(4);
        m_objBeginEnd.setColorType(RenderObj::Type::FLOAT);
        m_objBeginEnd.setColorStride(0);
        m_objBeginEnd.setColorPointer(m_colorVertexBuffer.data());

        m_objBeginEnd.enableIndices(false);
        m_objBeginEnd.setDrawMode(m_beginMode);
        m_objBeginEnd.setCount(m_vertexBuffer.size());

        return m_objBeginEnd;
    }

    const Vec4 color() const { return m_vertexColor; }

    const RenderObj& renderObj() const { return m_objPtr; }

    void reset() { m_objPtr.reset(); }

    void enableVertexArray(bool enable) { m_objPtr.enableVertexArray(enable); }
    void setVertexSize(uint8_t size) { m_objPtr.setVertexSize(size); }
    void setVertexType(Type type) { m_objPtr.setVertexType(type); }
    void setVertexStride(uint32_t stride) { m_objPtr.setVertexStride(stride); }
    void setVertexPointer(const void* ptr) { m_objPtr.setVertexPointer(ptr); }

    void enableTexCoordArray(bool enable) { m_objPtr.enableTexCoordArray(m_tmu, enable); }
    void setTexCoordSize(uint8_t size) { m_objPtr.setTexCoordSize(m_tmu, size); }
    void setTexCoordType(Type type) { m_objPtr.setTexCoordType(m_tmu, type); }
    void setTexCoordStride(uint32_t stride) { m_objPtr.setTexCoordStride(m_tmu, stride); }
    void setTexCoordPointer(const void* ptr) { m_objPtr.setTexCoordPointer(m_tmu, ptr); }

    void enableNormalArray(bool enable) { m_objPtr.enableNormalArray(enable); }
    void setNormalType(Type type) { m_objPtr.setNormalType(type); }
    void setNormalStride(uint32_t stride) { m_objPtr.setNormalStride(stride); }
    void setNormalPointer(const void* ptr) { m_objPtr.setNormalPointer(ptr); }

    void enableColorArray(bool enable) { m_objPtr.enableColorArray(enable); }
    void setColorSize(uint8_t size) { m_objPtr.setColorSize(size); }
    void setColorType(Type type) { m_objPtr.setColorType(type); }
    void setColorStride(uint32_t stride) { m_objPtr.setColorStride(stride); }
    void setColorPointer(const void* ptr) { m_objPtr.setColorPointer(ptr); }

    void setDrawMode(DrawMode mode) { m_objPtr.setDrawMode(mode); }

    void enableIndices(bool enable) { m_objPtr.enableIndices(enable); }
    void setCount(std::size_t count) { m_objPtr.setCount(count); }
    void setIndicesType(Type type) { m_objPtr.setIndicesType(type); }
    void setIndicesPointer(const void* ptr) { m_objPtr.setIndicesPointer(ptr); }
    void setArrayOffset(uint32_t offset) { m_objPtr.setArrayOffset(offset); }

private:
    // Buffer
    std::vector<Vec4> m_vertexBuffer;
    std::vector<std::array<Vec4, RenderObj::MAX_TMU_COUNT>> m_textureVertexBuffer;
    std::vector<Vec3> m_normalVertexBuffer;
    std::vector<Vec4> m_colorVertexBuffer;

    // State values
    Vec4 m_vertexColor {};
    std::array<Vec4, RenderObj::MAX_TMU_COUNT> m_textureCoord {};
    Vec3 m_normal {};
    DrawMode m_beginMode { DrawMode::TRIANGLES };

    // Render Object
    RenderObj m_objBeginEnd {};
    RenderObj m_objPtr {};

    uint8_t m_tmu { 0 };
};

} // namespace rr
#endif // VERTEXQUEUE_HPP_

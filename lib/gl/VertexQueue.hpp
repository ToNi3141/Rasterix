#ifndef VERTEXQUEUE_HPP_
#define VERTEXQUEUE_HPP_

#include "Vec.hpp"
#include "RenderObj.hpp"
#include <vector>

class VertexQueue 
{
public:
    using DrawMode = RenderObj::DrawMode;

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
    void setColor(const Vec4& color) { m_vertexColor = color; }
    void setNormal(const Vec3& normal) { m_normal = normal; }
    void setTexCoord(const Vec2& texCoord) { m_textureCoord = texCoord; }
    void end()
    {
        m_obj.enableVertexArray(!m_vertexBuffer.empty());
        m_obj.setVertexSize(4);
        m_obj.setVertexType(RenderObj::Type::FLOAT);
        m_obj.setVertexStride(0);
        m_obj.setVertexPointer(m_vertexBuffer.data());

        m_obj.enableTexCoordArray(!m_textureVertexBuffer.empty());
        m_obj.setTexCoordSize(2);
        m_obj.setTexCoordType(RenderObj::Type::FLOAT);
        m_obj.setTexCoordStride(0);
        m_obj.setTexCoordPointer(m_textureVertexBuffer.data());

        m_obj.enableNormalArray(!m_normalVertexBuffer.empty());
        m_obj.setNormalType(RenderObj::Type::FLOAT);
        m_obj.setNormalStride(0);
        m_obj.setNormalPointer(m_normalVertexBuffer.data());

        m_obj.enableColorArray(!m_colorVertexBuffer.empty());
        m_obj.setColorSize(4);
        m_obj.setColorType(RenderObj::Type::FLOAT);
        m_obj.setColorStride(0);
        m_obj.setColorPointer(m_colorVertexBuffer.data());

        m_obj.enableIndices(false);
        m_obj.setDrawMode(m_beginMode);
        m_obj.setCount(m_vertexBuffer.size());
    }

    const Vec4 color() const { return m_vertexColor; }

    const RenderObj& renderObj() const { return m_obj; }

private:
    // Buffer
    std::vector<Vec4> m_vertexBuffer;
    std::vector<Vec2> m_textureVertexBuffer;
    std::vector<Vec3> m_normalVertexBuffer;
    std::vector<Vec4> m_colorVertexBuffer;

    // State values
    Vec4 m_vertexColor {};
    Vec2 m_textureCoord {};
    Vec3 m_normal {};
    DrawMode m_beginMode { DrawMode::TRIANGLES };

    // Render Object
    RenderObj m_obj {};
};

#endif // VERTEXQUEUE_HPP_

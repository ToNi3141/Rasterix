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

#ifndef VERTEXPIPELINE_HPP
#define VERTEXPIPELINE_HPP

#include "Vec.hpp"
#include "IRenderer.hpp"
#include "Mat44.hpp"
#include "Clipper.hpp"
#include "Lighting.hpp"
#include "TexGen.hpp"
#include "RenderObj.hpp"

class VertexPipeline
{
public:
    enum CullMode
    {
        BACK,
        FRONT,
        FRONT_AND_BACK
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

    VertexPipeline(Lighting& lighting, TexGen& texGen);

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

#endif // VERTEXPIPELINE_HPP

// Rasterix
// https://github.com/ToNi3141/Rasterix
// Copyright (c) 2024 ToNi3141

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

#ifndef VIEWPORT_HPP
#define VIEWPORT_HPP

#include "Vec.hpp"

namespace rr
{
class ViewPort
{
public:
    void transform(Vec4 &v);
    void setViewport(const int16_t x, const int16_t y, const int16_t width, const int16_t height);
    void setDepthRange(const float zNear, const float zFar);

    int16_t getViewPortWidth() const { return m_viewportWidth; }
    int16_t getViewPortHeight() const { return m_viewportHeight; }

private:
    float m_depthRangeZNear { 0.0f };
    float m_depthRangeZFar { 1.0f };
    float m_depthRangeOffset { 0.0f };
    float m_depthRangeScale { 1.0f };
    int16_t m_viewportX { 0 };
    int16_t m_viewportY { 0 };
    int16_t m_viewportHeight { 0 };
    int16_t m_viewportWidth { 0 };
    float m_viewportHeightHalf { 0.0f };
    float m_viewportWidthHalf { 0.0f };
};

} // namespace rr
#endif // VIEWPORT_HPP

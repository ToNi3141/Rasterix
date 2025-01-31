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

#include "ViewPort.hpp"

namespace rr
{
void ViewPort::transform(Vec4& v)
{
    // v has the range from -1 to 1. When we multiply it, it has a range from -viewPortWidth/2 to viewPortWidth/2
    // With the addition we shift it from -viewPortWidth/2 to 0 + viewPortX
    v[0] = ((v[0] + 1.0f) * m_viewportWidthHalf) + m_viewportX;
    v[1] = ((v[1] + 1.0f) * m_viewportHeightHalf) + m_viewportY;
    v[2] = (m_depthRangeScale * v[2]) + m_depthRangeOffset;
    v[2] *= 65534.0f / 65536.0f; // Scales down the z value a bit, because the hardware expects a range of [0 .. 65535], which is basically [0.0 .. 0.999..]
}

void ViewPort::setViewport(const float x, const float y, const float width, const float height)
{
    m_viewportHeight = height;
    m_viewportWidth = width;
    m_viewportX = x;
    m_viewportY = y;

    m_viewportHeightHalf = m_viewportHeight / 2.0f;
    m_viewportWidthHalf = m_viewportWidth / 2.0f;
}

void ViewPort::setDepthRange(const float zNear, const float zFar)
{
    m_depthRangeZFar = zFar;
    m_depthRangeZNear = zNear;

    m_depthRangeScale = ((m_depthRangeZFar - m_depthRangeZNear) / 2.0f);
    m_depthRangeOffset = ((m_depthRangeZNear + m_depthRangeZFar) / 2.0f);
}
} // namespace rr
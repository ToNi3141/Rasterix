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

#ifndef FOG_HPP_
#define FOG_HPP_

#include "IRenderer.hpp"
#include "Vec.hpp"
#include <optional>

namespace rr
{
class Fog
{
public:
    enum class FogMode
    {
        ONE,
        LINEAR,
        EXP,
        EXP2
    };

    Fog(IRenderer& renderer);

    void setFogMode(const FogMode val);
    void setFogStart(const float val);
    void setFogEnd(const float val);
    void setFogDensity(const float val);
    bool setFogColor(const Vec4& val);

    bool updateFogLut();

private:
    IRenderer& m_renderer;
    bool m_fogDirty { false };
    FogMode m_fogMode { FogMode::EXP };
    float m_fogStart { 0.0f };
    float m_fogEnd { 1.0f };
    float m_fogDensity { 1.0f };
};

} // namespace rr
#endif // FOG_HPP_

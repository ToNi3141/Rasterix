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

#ifndef CULLING_HPP
#define CULLING_HPP

#include "Types.hpp"
#include "math/Vec.hpp"

namespace rr
{
class Culling
{
public:
    Culling();

    bool cull(const Vec4& v0, const Vec4& v1, const Vec4& v2) const;

    void enableCulling(const bool enable);
    void setCullMode(const Face mode);

private:
    bool m_enableCulling { false };
    Face m_cullMode { Face::BACK };
};

} // namespace rr
#endif // CULLING_HPP
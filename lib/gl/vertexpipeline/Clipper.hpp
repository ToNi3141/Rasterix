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

#ifndef CLIPPER_HPP
#define CLIPPER_HPP

#include "math/Vec.hpp"
#include <span>
#include <array>
#include "renderer/IRenderer.hpp"
#include "PrimitiveAssembler.hpp"

namespace rr
{

class Clipper 
{
public:
    // Each clipping plane can potentially introduce one more vertex. A triangle contains 3 vertexes, plus 6 possible planes, results in 9 vertexes.
    using ClipList = std::array<VertexParameter, 9>;

    static std::span<VertexParameter> clip(ClipList& list, ClipList& listBuffer);
    
    static bool isOutside(const Vec4& v0, const Vec4& v1, const Vec4& v2);
    static bool isInside(const Vec4& v0, const Vec4& v1, const Vec4& v2);

private:
    enum OutCode
    {
        OC_NONE    = 0x00,
        OC_NEAR    = 0x01,
        OC_FAR     = 0x02,
        OC_TOP     = 0x04,
        OC_BOTTOM  = 0x08,
        OC_LEFT    = 0x10,
        OC_RIGHT   = 0x20
    };

    static float lerpAmt(OutCode plane, const Vec4 &v0, const Vec4 &v1);
    static Vec4 lerpVert(const Vec4& v0, const Vec4& v1, const float amt);
    static std::array<Vec4, IRenderer::MAX_TMU_COUNT> lerpTexCoord(const std::array<Vec4, IRenderer::MAX_TMU_COUNT>& v0, const std::array<Vec4, IRenderer::MAX_TMU_COUNT>& v1, const float amt);
    static OutCode outCode(const Vec4 &v);
    static bool hasOutCode(const Vec4& v, const OutCode oc);
    
    static std::size_t clipAgainstPlane(ClipList& listOut, const OutCode clipPlane, const ClipList& listIn, const std::size_t listSize);

    static VertexParameter lerp(const OutCode clipPlane, const VertexParameter& curr, const VertexParameter& next);

    friend Clipper::OutCode operator|=(Clipper::OutCode& lhs, Clipper::OutCode rhs);
};

Clipper::OutCode operator|=(Clipper::OutCode& lhs, Clipper::OutCode rhs);

} // namespace rr

#endif // CLIPPER_HPP

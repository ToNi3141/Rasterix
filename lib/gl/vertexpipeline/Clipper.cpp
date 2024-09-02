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

#include "Clipper.hpp"

namespace rr
{

Vec4 Clipper::lerpVert(const Vec4& v0, const Vec4& v1, const float amt)
{
    Vec4 vOut;
    vOut[3] = ((v0[3] - v1[3]) * (1-amt)) + v1[3];
    vOut[2] = ((v0[2] - v1[2]) * (1-amt)) + v1[2];
    vOut[1] = ((v0[1] - v1[1]) * (1-amt)) + v1[1];
    vOut[0] = ((v0[0] - v1[0]) * (1-amt)) + v1[0];
    return vOut;
}

std::array<Vec4, TransformedTriangle::MAX_TMU_COUNT> Clipper::lerpTexCoord(const std::array<Vec4, TransformedTriangle::MAX_TMU_COUNT>& v0, const std::array<Vec4, TransformedTriangle::MAX_TMU_COUNT>& v1, const float amt)
{
    std::array<Vec4, TransformedTriangle::MAX_TMU_COUNT> vOut;
    for (std::size_t i = 0; i < vOut.size(); i++)
    {
        vOut[i] = lerpVert(v0[i], v1[i], amt);
    }
    return vOut;
}

bool Clipper::hasOutCode(const Vec4& v, const OutCode oc)
{
    switch (oc)
    {
    case OutCode::OC_LEFT:
        return v[0] < -v[3];
    case OutCode::OC_RIGHT:
        return v[0] > v[3];
    case OutCode::OC_BOTTOM:
        return v[1] < -v[3];
    case OutCode::OC_TOP:
        return v[1] > v[3];
    case OutCode::OC_NEAR:
        return v[2] < -v[3];
    case OutCode::OC_FAR:
        return v[2] > v[3];
    default:
        return false;
    }
}

float Clipper::lerpAmt(OutCode plane, const Vec4& v0, const Vec4& v1)
{
#ifdef CLIP_UNITCUBE
    // The clipping with a unit cube is easier to imagine, because it uses normal coordinates instead of homogeneous.
    // In the unit cube, we just have to check, if the axis against we want to clip is greater than one. If this is the case
    // than we have to check how much is the point over the one to get our factor by how much we have the lerp.

    // Clipping against near clipping plane
    float clip = -1; // This is the clipping plane. Values over -1 are clipped
    float a = v0[2] / v0[3]; // Convert from homogeneous coords to normal ones (Perspective division)
    float b = v1[2] / v1[3];
    return (clip-b) / (a - b); // Calculate the difference between two points. Then calculate how much b is over the clipping range
    // and then divide that with the difference of the two points and we will get our amount, how much
    // the values from the clipped point have to be lerped to the not clipped point
#else
    // For a better explanation see https://chaosinmotion.com/2016/05/22/3d-clipping-in-homogeneous-coordinates/
    // and https://github.com/w3woody/arduboy/blob/master/Demo3D/pipeline.cpp
    float zDot0 = 0.0f;
    float zDot1 = 0.0f;

    switch (plane)
    {
    case OutCode::OC_RIGHT: // v.dot(1,0,0,-1)
        zDot0 = v0[0] - v0[3];
        zDot1 = v1[0] - v1[3];
        break;
    case OutCode::OC_LEFT: // v.dot(1,0,0,1)
        zDot0 = v0[0] + v0[3];
        zDot1 = v1[0] + v1[3];
        break;
    case OutCode::OC_TOP: // v.dot(0,1,0,-1)
        zDot0 = v0[1] - v0[3];
        zDot1 = v1[1] - v1[3];
        break;
    case OutCode::OC_BOTTOM: // v.dot(0,1,0,1)
        zDot0 = v0[1] + v0[3];
        zDot1 = v1[1] + v1[3];
        break;
    case OutCode::OC_NEAR: // v.dot(0,0,1,1)
        zDot0 = v0[2] + v0[3];
        zDot1 = v1[2] + v1[3];
        break;
    case OutCode::OC_FAR: // v.dot(0,0,1,-1)
    default:
        zDot0 = v0[2] - v0[3];
        zDot1 = v1[2] - v1[3];
        break;
    }
    return zDot0 / (zDot0 - zDot1);
#endif
}

tcb::span<VertexParameter> Clipper::clip(ClipList& list, ClipList& listBuffer)
{
    ClipList* listIn = &list;
    ClipList* listOut = &listBuffer;

    std::size_t numberOfVerts = 3; // Initial the list contains 3 vertices
    std::size_t numberOfVertsCurrentPlane = 0;

    for (auto oc : {OutCode::OC_NEAR, OutCode::OC_FAR, OutCode::OC_LEFT, OutCode::OC_RIGHT, OutCode::OC_TOP, OutCode::OC_BOTTOM})
    {
        // Optimization hint: If no vertex has an outcode given from oc, then it can just be
        // ignored. We just have to avoid that the swapping of the buffers is executed.
        // Then we can skip the unneeded copying of data.
        numberOfVertsCurrentPlane = clipAgainstPlane(*listOut, oc, *listIn, numberOfVerts);
        if (numberOfVertsCurrentPlane > 0)
        {
            // Swap buffers
            std::swap(listIn, listOut);

            // Safe the new number of planes
            numberOfVerts = numberOfVertsCurrentPlane;
        }
    }

    // Assume in this trivial case, that we have clipped a triangle, which was already
    // complete outside. So this triangle shouldn't result in a bigger triangle
    if (outCode((*listIn)[0].vertex) & outCode((*listIn)[1].vertex) & outCode((*listIn)[2].vertex))
        return {};

    return { listIn->data(), numberOfVerts };
}

VertexParameter Clipper::lerp(const OutCode clipPlane, const VertexParameter& curr, const VertexParameter& next)
{
    VertexParameter out;
    const float lerpw = lerpAmt(clipPlane, curr.vertex, next.vertex);
    out.vertex = lerpVert(curr.vertex, next.vertex, lerpw);
    out.color = lerpVert(curr.color, next.color, lerpw);
    out.tex = lerpTexCoord(curr.tex, next.tex, lerpw);
    return out;
}

std::size_t Clipper::clipAgainstPlane(ClipList& listOut, const OutCode clipPlane, const ClipList& listIn, const std::size_t listSize)
{
    // Start Clipping
    std::size_t i = 0;

    for (int32_t vert = 0; vert < listSize; vert++)
    {
        if (hasOutCode(listIn[vert].vertex, clipPlane))
        {
            // std::size_t vertMod = (vert - 1) % listSize;
            const std::size_t vertPrev = (vert - 1) < 0 ? listSize - 1 : vert - 1;
            if (!hasOutCode(listIn[vertPrev].vertex, clipPlane))
            {
                listOut[i] = lerp(clipPlane, listIn[vert], listIn[vertPrev]);
                i++;
            }

            // vertMod = (vert + 1) % listSize;
            const std::size_t vertNext = (vert + 1) >= listSize ? 0 : vert + 1;
            if (!hasOutCode(listIn[vertNext].vertex, clipPlane))
            {
                listOut[i] = lerp(clipPlane, listIn[vert], listIn[vertNext]);
                i++;
            }
        }
        else
        {
            listOut[i] = listIn[vert];
            i++;
        }
    }

    return i;
}

Clipper::OutCode operator|=(Clipper::OutCode& lhs, Clipper::OutCode rhs)
{
    return lhs = static_cast<Clipper::OutCode>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
}

} // namespace rr
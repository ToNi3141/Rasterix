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

Clipper::ClipTexCoords Clipper::lerpTexCoord(const ClipTexCoords& v0, const ClipTexCoords& v1, const float amt)
{
    ClipTexCoords vOut;
    for (std::size_t i = 0; i < vOut.size(); i++)
    {
        vOut[i] = lerpVert(v0[i], v1[i], amt);
    }
    return vOut;
}

Clipper::OutCode Clipper::outCode(const Vec4& v)
{
    OutCode c = OutCode::OC_NONE;
    const float w = v[3];

    if (v[0] < -w)
        c |= OutCode::OC_LEFT;
    if (v[0] > w)
        c |= OutCode::OC_RIGHT;
    if (v[1] < -w)
        c |= OutCode::OC_BOTTOM;
    if (v[1] > w)
        c |= OutCode::OC_TOP;
    if (v[2] < -w)
        c |= OutCode::OC_NEAR;
    if (v[2] > w)
        c |= OutCode::OC_FAR;

    return c;
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

std::tuple<const uint32_t, 
    Clipper::ClipVertList&, 
    Clipper::ClipTexCoordList&, 
    Clipper::ClipVertList&> Clipper::clip(ClipVertList& vertList,
                                          ClipVertList& vertListBuffer,
                                          ClipTexCoordList& texCoordList,
                                          ClipTexCoordList& texCoordListBuffer,
                                          ClipVertList& colorList,
                                          ClipVertList& colorListBuffer)
{
    // Check if the triangle is completely outside by checking if all vertices have the same outcode
    const OutCode oc0 = outCode(vertList[0]);
    const OutCode oc1 = outCode(vertList[1]);
    const OutCode oc2 = outCode(vertList[2]);

    // Checking if the triangle is completely inside by checking, if no vertex has an outcode
    if ((oc0 | oc1 | oc2) == OutCode::OC_NONE) [[likely]]
    {
        return {3u, vertList, texCoordList, colorList};
    }

    if (oc0 & oc1 & oc2) [[likely]]
    {
        return {0u, vertList, texCoordList, colorList};
    }

    ClipVertList* currentVertListBufferIn = &vertList;
    ClipVertList* currentVertListBufferOut = &vertListBuffer;
    ClipTexCoordList* currentTexCoordListBufferIn = &texCoordList;
    ClipTexCoordList* currentTexCoordListBufferOut = &texCoordListBuffer;
    ClipVertList* currentColorListBufferIn = &colorList;
    ClipVertList* currentColorListBufferOut = &colorListBuffer;

    int8_t numberOfVerts = 3; // Initial the list contains 3 vertices
    int8_t numberOfVertsCurrentPlane = 0;

    for (auto oc : {OutCode::OC_NEAR, OutCode::OC_FAR, OutCode::OC_LEFT, OutCode::OC_RIGHT, OutCode::OC_TOP, OutCode::OC_BOTTOM})
    {
        // Optimization hint: If no vertex has an outcode given from oc, then it can just be
        // ignored. We just have to avoid that the swapping of the buffers is executed.
        // Then we can skip the unneeded copying of data.
        numberOfVertsCurrentPlane = clipAgainstPlane(*currentVertListBufferOut,
                                                     *currentTexCoordListBufferOut,
                                                     *currentColorListBufferOut,
                                                     oc,
                                                     *currentVertListBufferIn,
                                                     *currentTexCoordListBufferIn,
                                                     *currentColorListBufferIn,
                                                     numberOfVerts);
        if (numberOfVertsCurrentPlane > 0)
        {
            // Swap buffers
            ClipVertList* tmpVertIn = currentVertListBufferIn;
            currentVertListBufferIn = currentVertListBufferOut;
            currentVertListBufferOut = tmpVertIn;

            ClipTexCoordList* tmpTexIn = currentTexCoordListBufferIn;
            currentTexCoordListBufferIn = currentTexCoordListBufferOut;
            currentTexCoordListBufferOut = tmpTexIn;

            ClipVertList* tmpColorIn = currentColorListBufferIn;
            currentColorListBufferIn = currentColorListBufferOut;
            currentColorListBufferOut = tmpColorIn;

            // Safe the new number of planes
            numberOfVerts = numberOfVertsCurrentPlane;
        }
    }

    // Assume in this trivial case, that we have clipped a triangle, which was already
    // complete outside. So this triangle shouldn't result in a bigger triangle
    if (outCode((*currentVertListBufferIn)[0]) & outCode((*currentVertListBufferIn)[1]) & outCode((*currentVertListBufferIn)[2]))
        return {0u, vertList, texCoordList, colorList};

    return {numberOfVerts, *currentVertListBufferIn, *currentTexCoordListBufferIn, *currentColorListBufferIn};
}

uint32_t Clipper::clipAgainstPlane(ClipVertList& vertListOut,
                               ClipTexCoordList& texCoordListOut,
                               ClipVertList& colorListOut,
                               const OutCode clipPlane,
                               const ClipVertList& vertListIn,
                               const ClipTexCoordList& texCoordListIn,
                               const ClipVertList& colorListIn,
                               const uint32_t listInSize)
{
    // Start Clipping
    int i = 0;

    for (int8_t vert = 0; vert < static_cast<int8_t>(listInSize); vert++)
    {
        if (hasOutCode(vertListIn[vert], clipPlane))
        {
            //            uint8_t vertMod = (vert-1)%vertSize;
            uint8_t vertMod = (vert - 1) < 0 ? listInSize - 1 : vert - 1;
            if (!hasOutCode(vertListIn[vertMod], clipPlane))
            {
                const float lerpw = lerpAmt(clipPlane, vertListIn[vert], vertListIn[vertMod]);
                vertListOut[i] = lerpVert(vertListIn[vert], vertListIn[vertMod], lerpw);
                colorListOut[i] = lerpVert(colorListIn[vert], colorListIn[vertMod], lerpw);
                texCoordListOut[i] = lerpTexCoord(texCoordListIn[vert], texCoordListIn[vertMod], lerpw);
                i++;
            }

            // vertMod = (vert + 1) % listInSize;
            vertMod = (vert + 1) >= listInSize ? 0 : vert + 1;
            if (!hasOutCode(vertListIn[vertMod], clipPlane))
            {
                const float lerpw = lerpAmt(clipPlane, vertListIn[vert], vertListIn[vertMod]);
                vertListOut[i] = lerpVert(vertListIn[vert], vertListIn[vertMod], lerpw);
                colorListOut[i] = lerpVert(colorListIn[vert], colorListIn[vertMod], lerpw);
                texCoordListOut[i] = lerpTexCoord(texCoordListIn[vert], texCoordListIn[vertMod], lerpw);
                i++;
            }
        }
        else
        {
            vertListOut[i] = vertListIn[vert];
            colorListOut[i] = colorListIn[vert];
            texCoordListOut[i] = texCoordListIn[vert];
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
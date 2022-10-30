#include "Clipper.hpp"


void Clipper::lerpVert(Vec4& vOut, const Vec4& v0, const Vec4& v1, const float amt)
{
#ifdef CLIP_UNITCUBE
    vOut[3] = ((v0[3] - v1[3]) * amt) + v1[3];
    vOut[2] = ((v0[2] - v1[2]) * amt) + v1[2];
    vOut[1] = ((v0[1] - v1[1]) * amt) + v1[1];
    vOut[0] = ((v0[0] - v1[0]) * amt) + v1[0];
#else
    vOut[3] = ((v0[3] - v1[3]) * (1-amt)) + v1[3];
    vOut[2] = ((v0[2] - v1[2]) * (1-amt)) + v1[2];
    vOut[1] = ((v0[1] - v1[1]) * (1-amt)) + v1[1];
    vOut[0] = ((v0[0] - v1[0]) * (1-amt)) + v1[0];
    // float a1 = 1.0 - amt;
    // vOut[3] = (a1 * v0[3]) + (amt * v1[3]);
    // vOut[2] = (a1 * v0[2]) + (amt * v1[2]);
    // vOut[1] = (a1 * v0[1]) + (amt * v1[1]);
    // vOut[0] = (a1 * v0[0]) + (amt * v1[0]);
#endif
}

void Clipper::lerpTexCoord(ClipTexCoords& vOut, const ClipTexCoords& v0, const ClipTexCoords& v1, const float amt)
{
    for (std::size_t i = 0; i < vOut.size(); i++)
    {
        lerpVert(vOut[i], v0[i], v1[i], amt);
    }
}

Clipper::OutCode Clipper::outCode(const Vec4& v)
{
    OutCode c = OutCode::NONE;
    const float w = v[3];

    if (v[0] < -w)
        c |= OutCode::LEFT;
    if (v[0] > w)
        c |= OutCode::RIGHT;
    if (v[1] < -w)
        c |= OutCode::BOTTOM;
    if (v[1] > w)
        c |= OutCode::TOP;
    if (v[2] < -w)
        c |= OutCode::NEAR;
    if (v[2] > w)
        c |= OutCode::FAR;

    return c;
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
    case OutCode::RIGHT: // v.dot(1,0,0,-1)
        zDot0 = v0[0] - v0[3];
        zDot1 = v1[0] - v1[3];
        break;
    case OutCode::LEFT: // v.dot(1,0,0,1)
        zDot0 = v0[0] + v0[3];
        zDot1 = v1[0] + v1[3];
        break;
    case OutCode::TOP: // v.dot(0,1,0,-1)
        zDot0 = v0[1] - v0[3];
        zDot1 = v1[1] - v1[3];
        break;
    case OutCode::BOTTOM: // v.dot(0,1,0,1)
        zDot0 = v0[1] + v0[3];
        zDot1 = v1[1] + v1[3];
        break;
    case OutCode::NEAR: // v.dot(0,0,1,1)
        zDot0 = v0[2] + v0[3];
        zDot1 = v1[2] + v1[3];
        break;
    case OutCode::FAR: // v.dot(0,0,1,-1)
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
    OutCode oc0 = outCode(vertList[0]);
    OutCode oc1 = outCode(vertList[1]);
    OutCode oc2 = outCode(vertList[2]);
    if (oc0 & oc1 & oc2)
    {
        return {0u, vertList, texCoordList, colorList};
    }

    // Checking if the triangle is completely inside by checking, if no vertex has an outcode
    if ((oc0 | oc1 | oc2) == OutCode::NONE)
    {
        return {3u, vertList, texCoordList, colorList};
    }

    ClipVertList* currentVertListBufferIn = &vertList;
    ClipVertList* currentVertListBufferOut = &vertListBuffer;
    ClipTexCoordList* currentTexCoordListBufferIn = &texCoordList;
    ClipTexCoordList* currentTexCoordListBufferOut = &texCoordListBuffer;
    ClipVertList* currentColorListBufferIn = &colorList;
    ClipVertList* currentColorListBufferOut = &colorListBuffer;

    int8_t numberOfVerts = 3; // Initial the list contains 3 vertecies
    int8_t numberOfVertsCurrentPlane = 0;

    for (auto oc : {OutCode::NEAR, OutCode::FAR, OutCode::LEFT, OutCode::RIGHT, OutCode::TOP, OutCode::BOTTOM})
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
        if (outCode(vertListIn[vert]) & clipPlane)
        {
            //            uint8_t vertMod = (vert-1)%vertSize;
            uint8_t vertMod = (vert - 1) < 0 ? listInSize-1 : vert - 1;
            if (!(outCode(vertListIn[vertMod]) & clipPlane))
            {
                float lerpw = lerpAmt(clipPlane, vertListIn[vert], vertListIn[vertMod]);
                lerpVert(vertListOut[i], vertListIn[vert], vertListIn[vertMod], lerpw);
                lerpVert(colorListOut[i], colorListIn[vert], colorListIn[vertMod], lerpw);
                lerpTexCoord(texCoordListOut[i], texCoordListIn[vert], texCoordListIn[vertMod], lerpw);
                i++;
            }

            vertMod = (vert + 1) % listInSize;
            if (!(outCode(vertListIn[vertMod]) & clipPlane))
            {
                float lerpw = lerpAmt(clipPlane, vertListIn[vert], vertListIn[vertMod]);
                lerpVert(vertListOut[i], vertListIn[vert], vertListIn[vertMod], lerpw);
                lerpVert(colorListOut[i], colorListIn[vert], colorListIn[vertMod], lerpw);
                lerpTexCoord(texCoordListOut[i], texCoordListIn[vert], texCoordListIn[vertMod], lerpw);
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

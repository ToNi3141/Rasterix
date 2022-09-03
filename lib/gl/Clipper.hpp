#ifndef CLIPPER_HPP
#define CLIPPER_HPP

#include "Vec.hpp"
#include <tuple>
#include <array>

class Clipper 
{
public:
    // Each clipping plane can potentially introduce one more vertex. A triangle contains 3 vertexes, plus 6 possible planes, results in 9 vertexes
    using ClipVertList = std::array<Vec4, 9>;
    using ClipStList = std::array<Vec2, 9>;
    using ClipColorList = std::array<Vec4, 9>;

    static std::tuple<const uint32_t,
        ClipVertList &, 
        ClipStList &, 
        ClipColorList &> clip(ClipVertList& vertList,
                              ClipVertList& vertListBuffer,
                              ClipStList& stList,
                              ClipStList& stListBuffer,
                              ClipColorList& colorList,
                              ClipColorList& colorListBuffer);

private:
    enum OutCode
    {
        NONE    = 0x00,
        NEAR    = 0x01,
        FAR     = 0x02,
        TOP     = 0x04,
        BOTTOM  = 0x08,
        LEFT    = 0x10,
        RIGHT   = 0x20
    };

    static float lerpAmt(OutCode plane, const Vec4 &v0, const Vec4 &v1);
    static void lerpVert(Vec4& vOut, const Vec4& v0, const Vec4& v1, const float amt);
    static void lerpSt(Vec2& vOut, const Vec2& v0, const Vec2& v1, const float amt);
    static OutCode outCode(const Vec4 &v);
    
    static uint32_t clipAgainstPlane(ClipVertList& vertListOut,
                                     ClipStList& stListOut,
                                     ClipColorList &colorListOut,
                                     const OutCode clipPlane,
                                     const ClipVertList& vertListIn,
                                     const ClipStList& stListIn,
                                     const ClipColorList &colorListIn,
                                     const uint32_t listInSize);

    friend Clipper::OutCode operator|=(Clipper::OutCode& lhs, Clipper::OutCode rhs);
};

Clipper::OutCode operator|=(Clipper::OutCode& lhs, Clipper::OutCode rhs);
#endif // CLIPPER_HPP
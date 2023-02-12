#ifndef _TRIANGLE_STREAM_DESC_
#define _TRIANGLE_STREAM_DESC_

#include <cstdint>
#include <array>
#include "Vec.hpp"
#include "tcb/span.hpp"
#include "IRenderer.hpp"

namespace rr
{

struct TriangleStreamDesc
{
#pragma pack(push, 4)
    struct Texture
    {
        Vec3 texStq;
        Vec3 texStqXInc;
        Vec3 texStqYInc;
    };

    uint32_t reserved;
    uint16_t bbStartX;
    uint16_t bbStartY;
    uint16_t bbEndX;
    uint16_t bbEndY;
    Vec3i wInit;
    Vec3i wXInc;
    Vec3i wYInc;
    Vec4 color;
    Vec4 colorXInc;
    Vec4 colorYInc;
    float depthW;
    float depthWXInc;
    float depthWYInc;
    float depthZ;
    float depthZXInc;
    float depthZYInc;
    std::array<Texture, IRenderer::MAX_TMU_COUNT> texture;
#pragma pack(pop)
};

} // namespace rr

#endif // _TRIANGLE_STREAM_DESC_
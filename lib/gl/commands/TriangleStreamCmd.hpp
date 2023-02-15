#ifndef _TRIANGLE_STREAM_CMD_HPP_
#define _TRIANGLE_STREAM_CMD_HPP_

#include <cstdint>
#include <array>
#include "Vec.hpp"
#include "tcb/span.hpp"
#include "IRenderer.hpp"

namespace rr
{

class TriangleStreamCmd
{
    static constexpr uint32_t TRIANGLE_STREAM { 0x3000'0000 };
public:
    struct TriangleDesc
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

    TriangleDesc& getDesc() { return m_desc; }

    using Desc = std::array<tcb::span<TriangleDesc>, 1>;
    void serialize(Desc& desc) const 
    { 
        std::memcpy(desc[0].data(), &m_desc, sizeof(TriangleDesc));
    }
    static constexpr uint32_t command() { return TRIANGLE_STREAM | sizeof(TriangleDesc); }

private:
    TriangleDesc m_desc;
};

} // namespace rr

#endif // _TRIANGLE_STREAM_CMD_HPP_

#ifndef _FRAMEBUFFER_CMD_HPP_
#define _FRAMEBUFFER_CMD_HPP_

#include <cstdint>
#include <array>
#include <tcb/span.hpp>
#include "DmaStreamEngineCommands.hpp"

namespace rr
{

class FramebufferCmd
{
    static constexpr uint32_t OP_FRAMEBUFFER { 0x2000'0000 };
    static constexpr uint32_t OP_FRAMEBUFFER_COMMIT { OP_FRAMEBUFFER | 0x0000'0001 };
    static constexpr uint32_t OP_FRAMEBUFFER_MEMSET { OP_FRAMEBUFFER | 0x0000'0002 };
    static constexpr uint32_t OP_FRAMEBUFFER_COLOR_BUFFER_SELECT { OP_FRAMEBUFFER | 0x0000'0010 };
    static constexpr uint32_t OP_FRAMEBUFFER_DEPTH_BUFFER_SELECT { OP_FRAMEBUFFER | 0x0000'0020 };
public:
    FramebufferCmd(const bool selColorBuffer, const bool selDepthBuffer)
    {
        if (selColorBuffer)
        {
            selectColorBuffer();
        }
        if (selDepthBuffer)
        {
            selectDepthBuffer();
        }
    }

    void enableCommit(const uint32_t size, const uint32_t addr, const bool commitToStream) 
    { 
        m_op |= OP_FRAMEBUFFER_COMMIT; 
        if (commitToStream)
        {
            m_dseOp = DSEC::COMMIT_TO_STREAM;
        }
        else
        {
            m_dseOp = DSEC::COMMIT_TO_MEMORY;
        }
        m_dseSize = size;
        m_dseAddr = addr;
    }
    void enableMemset() 
    { 
        m_op |= OP_FRAMEBUFFER_MEMSET;
        m_dseOp = DSEC::NOP;
    }
    void selectColorBuffer() { m_op |= OP_FRAMEBUFFER_COLOR_BUFFER_SELECT; }
    void selectDepthBuffer() { m_op |= OP_FRAMEBUFFER_DEPTH_BUFFER_SELECT; }

    using ValType = uint8_t;
    using Desc = std::array<tcb::span<ValType>, 0>;
    void serialize(Desc&) const {}
    static constexpr std::size_t size() { return 0; }
    uint32_t command() const { return m_op; }

    uint32_t dseCommand() const { return m_dseOp; }
    std::array<DSEC::Transfer, 1> dseTransfer() const { return { { m_dseAddr, m_dseSize } }; }

private:
    uint32_t m_op {};
    uint32_t m_dseOp { DSEC::NOP };
    uint32_t m_dseSize {};
    uint32_t m_dseAddr {};
};

} // namespace rr

#endif // _FRAMEBUFFER_CMD_HPP_

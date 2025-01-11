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


#ifndef _FRAMEBUFFER_CMD_HPP_
#define _FRAMEBUFFER_CMD_HPP_

#include <cstdint>
#include <array>
#include <tcb/span.hpp>
#include "renderer/DmaStreamEngineCommands.hpp"

namespace rr
{

class FramebufferCmd
{
    static constexpr uint32_t OP_FRAMEBUFFER { 0x2000'0000 };
    static constexpr uint32_t OP_FRAMEBUFFER_COMMIT { OP_FRAMEBUFFER | 0x0000'0001 };
    static constexpr uint32_t OP_FRAMEBUFFER_MEMSET { OP_FRAMEBUFFER | 0x0000'0002 };
    static constexpr uint32_t OP_FRAMEBUFFER_SWAP { OP_FRAMEBUFFER | 0x0000'0004 };
    static constexpr uint32_t OP_FRAMEBUFFER_COLOR_BUFFER_SELECT { OP_FRAMEBUFFER | 0x0000'0010 };
    static constexpr uint32_t OP_FRAMEBUFFER_DEPTH_BUFFER_SELECT { OP_FRAMEBUFFER | 0x0000'0020 };
    static constexpr uint32_t OP_FRAMEBUFFER_STENCIL_BUFFER_SELECT { OP_FRAMEBUFFER | 0x0000'0040 };
public:
    FramebufferCmd(const bool selColorBuffer, const bool selDepthBuffer, const bool selStencilBuffer)
    {
        if (selColorBuffer)
        {
            selectColorBuffer();
        }
        if (selDepthBuffer)
        {
            selectDepthBuffer();
        }
        if (selStencilBuffer)
        {
            selectStencilBuffer();
        }
    }

    void swapFramebuffer()
    {
        m_op |= OP_FRAMEBUFFER_SWAP;
        m_dseCommand.op = DSEC::OP_NOP;
    }
    void streamFromFramebuffer(const std::size_t size, const uint32_t addr)
    {
        m_op = 0;
        m_dseCommand.op = DSEC::OP_STREAM_FROM_MEMORY | static_cast<uint32_t>(size);
        m_dseCommand.addr = addr;
    }
    void commitFramebuffer(const std::size_t size, const uint32_t addr, const bool commitToStream) 
    { 
        m_op |= OP_FRAMEBUFFER_COMMIT; 
        if (commitToStream)
        {
            m_dseCommand.op = DSEC::OP_COMMIT_TO_STREAM | static_cast<uint32_t>(size);
        }
        else
        {
            m_dseCommand.op = DSEC::OP_COMMIT_TO_MEMORY | static_cast<uint32_t>(size);
        }
        m_dseCommand.addr = addr;
    }
    void enableMemset() 
    { 
        m_op |= OP_FRAMEBUFFER_MEMSET;
        m_dseCommand.op = DSEC::OP_NOP;
    }
    void selectColorBuffer() { m_op |= OP_FRAMEBUFFER_COLOR_BUFFER_SELECT; }
    void selectDepthBuffer() { m_op |= OP_FRAMEBUFFER_DEPTH_BUFFER_SELECT; }
    void selectStencilBuffer() { m_op |= OP_FRAMEBUFFER_STENCIL_BUFFER_SELECT; }

    using PayloadType = tcb::span<const uint8_t>;
    const PayloadType payload() const { return {}; }
    using CommandType = uint32_t;
    CommandType command() const { return m_op; }

    DSEC::Command dseCommand() const { return m_dseCommand; }
    const tcb::span<const uint8_t>& dsePayload() const { return m_dsePayload; }

private:
    CommandType m_op {};
    DSEC::Command m_dseCommand { DSEC::OP_NOP, 0 };
    tcb::span<const uint8_t> m_dsePayload {};
};

} // namespace rr

#endif // _FRAMEBUFFER_CMD_HPP_

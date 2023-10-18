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
#include <span>
#include "DmaStreamEngineCommands.hpp"

namespace rr
{

template <typename RenderConfig>
class FramebufferCmd
{
    static constexpr uint32_t OP_FRAMEBUFFER { 0x2000'0000 };
    static constexpr uint32_t OP_FRAMEBUFFER_COMMIT { OP_FRAMEBUFFER | 0x0000'0001 };
    static constexpr uint32_t OP_FRAMEBUFFER_MEMSET { OP_FRAMEBUFFER | 0x0000'0002 };
    static constexpr uint32_t OP_FRAMEBUFFER_COLOR_BUFFER_SELECT { OP_FRAMEBUFFER | 0x0000'0010 };
    static constexpr uint32_t OP_FRAMEBUFFER_DEPTH_BUFFER_SELECT { OP_FRAMEBUFFER | 0x0000'0020 };
    static constexpr uint32_t OP_FRAMEBUFFER_STENCIL_BUFFER_SELECT { OP_FRAMEBUFFER | 0x0000'0040 };
    using DseTransferType = std::array<DSEC::Transfer, 1>;
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

    void enableExternalFramebufferSwap()
    {
        m_op |= OP_FRAMEBUFFER_COMMIT;
        m_dseOp = DSEC::OP_NOP;
    }
    void enableExternalCommit(const uint32_t size, const uint32_t addr)
    {
        m_op = 0;
        m_dseOp = DSEC::OP_STREAM_FROM_MEMORY;
        m_dseData = { { RenderConfig::GRAM_MEMORY_LOC + addr, size } };
    }
    void enableInternalCommit(const uint32_t size, const uint32_t addr, const bool commitToStream) 
    { 
        m_op |= OP_FRAMEBUFFER_COMMIT; 
        if (commitToStream)
        {
            m_dseOp = DSEC::OP_COMMIT_TO_STREAM;
        }
        else
        {
            m_dseOp = DSEC::OP_COMMIT_TO_MEMORY;
        }
        m_dseData = { { RenderConfig::GRAM_MEMORY_LOC + addr, size } };
    }
    void enableMemset() 
    { 
        m_op |= OP_FRAMEBUFFER_MEMSET;
        m_dseOp = DSEC::OP_NOP;
    }
    void selectColorBuffer() { m_op |= OP_FRAMEBUFFER_COLOR_BUFFER_SELECT; }
    void selectDepthBuffer() { m_op |= OP_FRAMEBUFFER_DEPTH_BUFFER_SELECT; }
    void selectStencilBuffer() { m_op |= OP_FRAMEBUFFER_STENCIL_BUFFER_SELECT; }

    using Desc = std::array<std::span<uint8_t>, 0>;
    void serialize(Desc&) const {}
    uint32_t command() const { return m_op; }

    DSEC::SCT dseOp() const { return m_dseOp; }
    const DseTransferType& dseTransfer() const { return m_dseData; }

private:
    uint32_t m_op {};
    DSEC::SCT m_dseOp { DSEC::OP_NOP };
    DseTransferType m_dseData {};
};

} // namespace rr

#endif // _FRAMEBUFFER_CMD_HPP_

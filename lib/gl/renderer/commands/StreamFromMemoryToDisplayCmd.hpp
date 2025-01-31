// Rasterix
// https://github.com/ToNi3141/Rasterix
// Copyright (c) 2025 ToNi3141

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

#ifndef _STREAM_FROM_MEMORY_TO_DISPLAY_CMD_HPP_
#define _STREAM_FROM_MEMORY_TO_DISPLAY_CMD_HPP_

#include "RenderConfigs.hpp"
#include "renderer/DmaStreamEngineCommands.hpp"
#include <array>
#include <cstdint>
#include <tcb/span.hpp>

namespace rr
{

class StreamFromMemoryToDisplayCmd
{
public:
    StreamFromMemoryToDisplayCmd(const uint32_t addr, const std::size_t size)
    {
        m_dseTransfer = { DSEC::OP_STREAM_FROM_MEMORY, size, addr + RenderConfig::GRAM_MEMORY_LOC, {} };
    }

    const DSEC::Transfer& dseTransfer() const { return m_dseTransfer; }

private:
    DSEC::Transfer m_dseTransfer {};
};

} // namespace rr

#endif // _STREAM_FROM_MEMORY_TO_DISPLAY_CMD_HPP_

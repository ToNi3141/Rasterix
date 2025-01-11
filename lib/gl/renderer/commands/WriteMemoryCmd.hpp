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


#ifndef _WRITE_MEMORY_CMD_HPP_
#define _WRITE_MEMORY_CMD_HPP_

#include <cstdint>
#include <array>
#include <tcb/span.hpp>
#include "renderer/DmaStreamEngineCommands.hpp"

namespace rr
{

class WriteMemoryCmd
{
public:
    WriteMemoryCmd(const uint32_t addr, const tcb::span<const uint8_t> data)
    {
        // TODO: Maybe also check if the texture is a multiple of DEVICE_MIN_TRANSFER_SIZE
        std::size_t sizeOnDevice { (std::max)(data.size(), DSEC::DEVICE_MIN_TRANSFER_SIZE) };
        m_dseCommand = { DSEC::OP_STORE | sizeOnDevice, addr };
        m_dsePayload = data;
    }

    DSEC::Command dseCommand() const { return m_dseCommand; }
    const tcb::span<const uint8_t>& dsePayload() const { return m_dsePayload; }

private:
    DSEC::Command m_dseCommand {};
    tcb::span<const uint8_t> m_dsePayload;
};

} // namespace rr

#endif // _WRITE_MEMORY_CMD_HPP_

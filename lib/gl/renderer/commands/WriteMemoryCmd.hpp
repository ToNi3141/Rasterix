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
#include "RenderConfigs.hpp"

namespace rr
{

class WriteMemoryCmd
{
public:
    WriteMemoryCmd(const uint32_t addr, const tcb::span<const uint8_t> data)
    {
        // TODO: Maybe also check if the texture is a multiple of DEVICE_MIN_TRANSFER_SIZE
        std::size_t sizeOnDevice { (std::max)(data.size(), DSEC::DEVICE_MIN_TRANSFER_SIZE) };
        m_dseTransfer = { DSEC::OP_STORE, sizeOnDevice, addr + RenderConfig::GRAM_MEMORY_LOC, data };
    }

    const DSEC::Transfer& dseTransfer() const { return m_dseTransfer; }

private:
    DSEC::Transfer m_dseTransfer {};
};

} // namespace rr

#endif // _WRITE_MEMORY_CMD_HPP_

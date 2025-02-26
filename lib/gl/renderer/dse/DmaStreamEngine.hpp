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

#ifndef _DSE_BUSCONNECTOR_HPP_
#define _DSE_BUSCONNECTOR_HPP_

#include "DmaStreamEngineCommands.hpp"
#include "IBusConnector.hpp"
#include "RenderConfigs.hpp"

namespace rr::DSEC
{

class DmaStreamEngine
{
public:
    DmaStreamEngine(IBusConnector& busConnector)
        : m_busConnector { busConnector }
    {
    }

    void streamDisplayList(const uint8_t index, const uint32_t size)
    {
        const std::size_t commandSize = addDseStreamCommand(index, size);
        m_busConnector.writeData(index, size + commandSize);
    }

    void writeToDeviceMemory(tcb::span<const uint8_t> data, const uint32_t addr)
    {
        const std::size_t commandSize = addDseStoreCommand(data.size(), addr + RenderConfig::GRAM_MEMORY_LOC);
        addDseStorePayload(commandSize, data);
        m_busConnector.writeData(getStoreBufferIndex(), commandSize + data.size());
    }

    tcb::span<uint8_t> requestDisplayListBuffer(const uint8_t index)
    {
        tcb::span<uint8_t> s = m_busConnector.requestBuffer(index);
        return s.last(s.size() - sizeof(Command));
    }

    bool clearToSend()
    {
        return m_busConnector.clearToSend();
    }

    uint8_t getDisplayListBufferCount() const
    {
        return m_busConnector.getBufferCount() - 1;
    }

private:
    std::size_t getStoreBufferIndex() const
    {
        return getDisplayListBufferCount();
    }

    std::size_t addDseStreamCommand(const uint8_t index, const uint32_t size)
    {
        return addDseCommand(index, OP_STREAM, size, 0);
    }

    std::size_t addDseStoreCommand(const uint32_t size, const uint32_t addr)
    {
        return addDseCommand(getStoreBufferIndex(), OP_STORE, size, addr);
    }

    std::size_t addDseStorePayload(const std::size_t offset, const tcb::span<const uint8_t> payload)
    {
        tcb::span<uint8_t> s = m_busConnector.requestBuffer(getStoreBufferIndex());
        std::copy(payload.begin(), payload.end(), s.last(s.size() - offset).begin());
        return std::max(payload.size(), DEVICE_MIN_TRANSFER_SIZE);
    }

    std::size_t addDseCommand(
        const uint8_t index,
        const uint32_t op,
        const uint32_t size,
        const uint32_t addr)
    {
        tcb::span<uint8_t> s = m_busConnector.requestBuffer(index);
        Command* c = reinterpret_cast<Command*>(s.first<sizeof(Command)>().data());
        c->op = op | (IMM_MASK & size);
        c->addr = addr;
        return sizeof(Command);
    }

    IBusConnector& m_busConnector;
};

} // namespace rr::DSEC

#endif // _DSE_BUSCONNECTOR_HPP_

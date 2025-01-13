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

#ifndef DSEDISPLAYLISTASSEMBLER_HPP
#define DSEDISPLAYLISTASSEMBLER_HPP

#include <stdint.h>
#include <array>
#include <bitset>
#include <algorithm>
#include <tcb/span.hpp>
#include "renderer/DmaStreamEngineCommands.hpp"

namespace rr::displaylist
{

template <typename TDisplayList>
class DSEDisplayListAssembler {
public:
    struct DisplayListEntry
    {
        DSEC::Command& command;
        tcb::span<uint8_t> payload;
    };

    DSEDisplayListAssembler(TDisplayList& displayList)
        : m_displayList { displayList }
    {
    }

    template <typename TCommand>
    static std::size_t getCommandSize(const TCommand& cmd)
    {
        return TDisplayList::template sizeOf<DSEC::Command>() + cmd.dseTransfer().len;
    }

    template <typename TCommand>
    bool addCommand(const TCommand& cmd)
    {
        if (cmd.dseTransfer().op == DSEC::OP_NOP)
        {
            return true;
        }
        std::optional<DisplayListEntry> entry = allocateCommand(cmd);
        if (!entry) 
        {
            return false;
        }
        writeCommandToEntry(*entry, cmd);
        return true;
    }

    template <typename TCommand>
    std::optional<DisplayListEntry> allocateCommand(const TCommand& cmd)
    {
        if (getCommandSize<TCommand>(cmd) >= m_displayList.getFreeSpace()) 
        {
            return std::nullopt;
        }

        DSEC::Command* c = m_displayList.template create<DSEC::Command>();
        tcb::span<uint8_t> p {};
        if (!cmd.dseTransfer().payload.empty())
        {
            const std::size_t dataSize = cmd.dseTransfer().len;
            void* dest = m_displayList.alloc(dataSize);
            p = { reinterpret_cast<uint8_t*>(dest), dataSize };
        }
        return DisplayListEntry { *c, p };
    }

    template <typename TCommand>
    void writeCommandToEntry(DisplayListEntry& entry, const TCommand& cmd)
    {
        const DSEC::Transfer& transfer = cmd.dseTransfer();
        entry.command.op = transfer.op | transfer.len;
        entry.command.addr = transfer.addr;
        std::copy(transfer.payload.begin(), transfer.payload.end(), entry.payload.begin());
    }
private:
    TDisplayList& m_displayList;
};

} // namespace rr::displaylists

#endif // DSEDISPLAYLISTASSEMBLER_HPP

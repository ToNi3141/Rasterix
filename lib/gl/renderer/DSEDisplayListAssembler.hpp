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
#include "DmaStreamEngineCommands.hpp"

namespace rr
{

template <typename TDisplayList>
class DSEDisplayListAssembler {
public:
    DSEDisplayListAssembler(TDisplayList& displayList)
        : m_displayList { displayList }
    {
    }

    template <typename TCommand>
    static std::size_t getCommandSize(const TCommand& cmd)
    {
        return TDisplayList::template sizeOf<DSEC::Command>() + cmd.dsePayload().size();;
    }

    template <typename TCommand>
    bool addCommand(const TCommand& cmd)
    {
        if (getCommandSize<TCommand>(cmd) >= m_displayList.getFreeSpace()) 
        {
            return false;
        }
        writeCommand(cmd);
        return true;
    }
private:
    template <typename TCommand>
    void writeCommand(const TCommand& cmd)
    {
        if ((cmd.dseCommand().op & DSEC::OP_MASK) != DSEC::OP_NOP)
        {
            writeCommand(cmd.dseCommand(), cmd.dsePayload());
        }
    }

    void writeCommand(const DSEC::Command command, const tcb::span<const uint8_t>& payload)
    {
        DSEC::Command *cmd = m_displayList.template create<DSEC::Command>();
        *cmd = command;

        if (!payload.empty())
        {
            void *dest = m_displayList.alloc(command.op & DSEC::IMM_MASK );
            memcpy(dest, payload.data(), payload.size());
        }
    }

    TDisplayList& m_displayList;
};

} // namespace rr

#endif // DSEDISPLAYLISTASSEMBLER_HPP

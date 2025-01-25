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

#ifndef RRXDISPLAYLISTASSEMBLER_HPP
#define RRXDISPLAYLISTASSEMBLER_HPP

#include <stdint.h>
#include <array>
#include <algorithm>
#include <tcb/span.hpp>

namespace rr::displaylist
{

template <typename TDisplayList>
class RRXDisplayListAssembler 
{
public:
    RRXDisplayListAssembler(TDisplayList& displayList)
        : m_displayList { displayList }
    {
    }

    template <typename TCommand>
    static std::size_t getCommandSize(const TCommand& cmd)
    {
        std::size_t expectedSize = 0;
        expectedSize += TDisplayList::template sizeOf<typename TCommand::CommandType>();
        expectedSize += TDisplayList::template sizeOf<typename TCommand::PayloadType::element_type>() * cmd.payload().size();
        return expectedSize;
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
        *(m_displayList.template create<typename TCommand::CommandType>()) = cmd.command();
        for (auto& a : cmd.payload())
        {
            using PayloadType = typename std::remove_const<typename TCommand::PayloadType::element_type>::type;
            *(m_displayList.template create<PayloadType>()) = a;
        }
    }

    TDisplayList& m_displayList;
};

} // namespace rr::displaylist

#endif // RRXDISPLAYLISTASSEMBLER_HPP

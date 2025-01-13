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

#ifndef STREAMSECTIONMANAGER_HPP
#define STREAMSECTIONMANAGER_HPP

#include <stdint.h>
#include <array>
#include <bitset>
#include <algorithm>
#include <tcb/span.hpp>
#include "renderer/DmaStreamEngineCommands.hpp"

namespace rr::displaylist
{

template <typename TRRXDisplayList, typename TDSEDisplayListAssembler, typename TCommand>
class StreamSectionManager 
{
public:
    StreamSectionManager(TRRXDisplayList& rrxDisplayList,
                                    TDSEDisplayListAssembler& dseDisplayListAssembler)
        : m_rrxDisplayList { rrxDisplayList }
        , m_dseDisplayListAssembler { dseDisplayListAssembler }
    {
    }

    void reset()
    {
        m_streamCommand = std::nullopt;
    }

    bool openNewStreamSection()
    {
        if (!m_streamCommand) 
        {
            auto streamCommand = m_dseDisplayListAssembler.template allocateCommand(TCommand { 0 });
            if (streamCommand)
            {
                m_streamCommand.emplace(*streamCommand);
                m_currentDisplayListSize = m_rrxDisplayList.getSize();
            }
        }
        return m_streamCommand.has_value();
    }

    void closeStreamSection()
    {
        if (m_streamCommand)
        {
            std::size_t len = m_rrxDisplayList.getSize() - m_currentDisplayListSize;
            m_dseDisplayListAssembler.writeCommandToEntry(*m_streamCommand, TCommand { len });
            m_streamCommand = std::nullopt;
        }
    }

private:
    TRRXDisplayList& m_rrxDisplayList;
    TDSEDisplayListAssembler& m_dseDisplayListAssembler;
    std::optional<typename TDSEDisplayListAssembler::DisplayListEntry> m_streamCommand {};
    std::size_t m_currentDisplayListSize {};
};

} // namespace rr::displaylist

#endif // STREAMSECTIONMANAGER_HPP

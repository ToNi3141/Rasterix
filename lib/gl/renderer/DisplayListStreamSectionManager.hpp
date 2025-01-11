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

#ifndef DISPLAYLISTSTREAMSECTIONMANAGER_HPP
#define DISPLAYLISTSTREAMSECTIONMANAGER_HPP

#include <stdint.h>
#include <array>
#include <bitset>
#include <algorithm>
#include <tcb/span.hpp>
#include "DmaStreamEngineCommands.hpp"

namespace rr
{

template <typename TDisplayList>
class DisplayListStreamSectionManager 
{
public:
    DisplayListStreamSectionManager(TDisplayList& displayList)
        : m_displayList { displayList }
    {
    }

    void reset()
    {
        m_streamCommand = nullptr;
    }

    bool openNewStreamSection()
    {
        if (m_streamCommand == nullptr) 
        {
            m_streamCommand = m_displayList.template create<DSEC::SCT>();
            m_displayList.template create<DSEC::SCT>(); // Address field, but unused in a stream to stream command
            if (m_streamCommand)
            {
                *m_streamCommand = m_displayList.getSize();
            }
        }
        return m_streamCommand != nullptr;
    }

    void closeStreamSection()
    {
        // Note during open, we write the current size of the display list into this command.
        // Now we just have to subtract from the current display list size the last display list size
        // to know how big our stream section is.
        if (m_streamCommand)
        {
            *m_streamCommand = DSEC::OP_STREAM | (m_displayList.getSize() - *m_streamCommand);
            m_streamCommand = nullptr;
        }
    }

private:
    TDisplayList& m_displayList;
    DSEC::SCT *m_streamCommand { nullptr };
};

} // namespace rr

#endif // DISPLAYLISTSTREAMSECTIONMANAGER_HPP

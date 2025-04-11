// RRX
// https://github.com/ToNi3141/RRX
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

#ifndef DISPLAYLISTDOUBLEBUFFER_HPP
#define DISPLAYLISTDOUBLEBUFFER_HPP

#include "DisplayListDispatcher.hpp"
#include <stdint.h>
#include <tcb/span.hpp>

namespace rr::displaylist
{

template <typename TDisplayList>
class DisplayListDoubleBuffer
{
public:
    DisplayListDoubleBuffer(TDisplayList& dl1, TDisplayList& dl2)
        : m_displayList { dl1, dl2 }
    {
    }

    void swap()
    {
        if (m_selectedDisplayList == 1)
        {
            m_selectedDisplayList = 0;
        }
        else
        {
            m_selectedDisplayList = 1;
        }
    }

    TDisplayList& getFront()
    {
        return m_displayList[1 - m_selectedDisplayList];
    }

    TDisplayList& getBack()
    {
        return m_displayList[m_selectedDisplayList];
    }

private:
    std::array<std::reference_wrapper<TDisplayList>, 2> m_displayList {};
    std::size_t m_selectedDisplayList { 0 };
};

} // namespace rr::displaylist
#endif // DISPLAYLISTDOUBLEBUFFER_HPP

// Rasterix
// https://github.com/ToNi3141/Rasterix
// Copyright (c) 2023 ToNi3141

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

#ifndef DISPLAYLISTASSEMBLER_HPP
#define DISPLAYLISTASSEMBLER_HPP

#include <stdint.h>
#include <array>
#include <bitset>
#include <algorithm>
#include <tcb/span.hpp>
#include "DisplayList.hpp"
#include "RRXDisplayListAssembler.hpp"
#include "DSEDisplayListAssembler.hpp"
#include "TextureLoadOptimizer.hpp"
#include "StreamSectionManager.hpp"
#include "renderer/commands/StreamFromStreamCmd.hpp"

namespace rr::displaylist
{

template <class RenderConfig>
class DisplayListAssembler 
{
public:
    static constexpr uint8_t ALIGNMENT { 4 }; // 4 bytes alignment (for the 32 bit AXIS)
    using List = DisplayList<ALIGNMENT>;

    void setBuffer(tcb::span<uint8_t> buffer)
    {
        m_displayList.setBuffer(buffer);
    }

    const List* getDisplayList() const
    {
        return &m_displayList;
    }

    void clearAssembler()
    {
        m_displayList.clear();
        m_streamSectionManager.reset();
        m_textureLoadOptimizer.reset();
    }

    bool begin()
    {
        return m_streamSectionManager.openNewStreamSection();
    }

    void end()
    {
        m_streamSectionManager.closeStreamSection();
    }

    template <typename TCommand>
    std::size_t getCommandSize(const TCommand& cmd)
    {
        std::size_t expectedSize = 0;
        
        if constexpr (HasCommand<decltype(cmd)>::value)
        {
            expectedSize += m_rrxDisplayListAssembler.getCommandSize<TCommand>(cmd);
        }

        if constexpr (HasDseTransfer<decltype(cmd)>::value)
        {
            expectedSize += m_dseDisplayListAssembler.getCommandSize<TCommand>(cmd);
        }
        return expectedSize;
    }

    void saveSectionStart()
    {
        m_displayList.saveSectionStart();
    }

    void removeSection()
    {
        m_displayList.removeSection();
    }

    template <typename TCommand>
    bool addCommand(const TCommand& cmd)
    {
        m_textureLoadOptimizer.optimize(cmd);

        if constexpr (HasCommand<decltype(cmd)>::value)
        {
            // Only add a new RRX command when the stream section is open
            if (!m_streamSectionManager.sectionOpen())
            {
                return false;
            }
            return m_rrxDisplayListAssembler.addCommand(cmd);
        }

        if constexpr (HasDseTransfer<decltype(cmd)>::value)
        {
            // Only add a new DSE command when the section is open
            if (m_streamSectionManager.sectionOpen())
            {
                return false;
            }
            return m_dseDisplayListAssembler.addCommand(cmd);
        }

        return true;
    }
private:
    template<typename T> 
    class HasDseTransfer 
    {
        template<typename> 
        static std::false_type test(...);
        template<typename U> 
        static auto test(int) -> decltype(std::declval<U>().dseTransfer(), std::true_type());
    public:
        static constexpr bool value = std::is_same<decltype(test<T>(0)), std::true_type>::value;
    };

    template<typename T> 
    class HasCommand 
    {
        template<typename> 
        static std::false_type test(...);
        template<typename U> 
        static auto test(int) -> decltype(std::declval<U>().command(), std::true_type());
    public:
        static constexpr bool value = std::is_same<decltype(test<T>(0)), std::true_type>::value;
    };

    template <typename TCommand>
    bool hasDisplayListEnoughSpace(const TCommand& cmd)
    {
        if (getCommandSize(cmd) >= m_displayList.getFreeSpace()) 
        {
            return false;
        }
        return true;
    }

    List m_displayList {};
    RRXDisplayListAssembler<List> m_rrxDisplayListAssembler { m_displayList };
    DSEDisplayListAssembler<List> m_dseDisplayListAssembler { m_displayList };
    TextureLoadOptimizer<RenderConfig::TMU_COUNT, List> m_textureLoadOptimizer { m_displayList };
    StreamSectionManager<List,
                         DSEDisplayListAssembler<List>,
                         StreamFromStreamCmd> m_streamSectionManager { m_displayList, m_dseDisplayListAssembler };
};

} // namespace rr::displaylist

#endif // DISPLAYLISTASSEMBLER_HPP

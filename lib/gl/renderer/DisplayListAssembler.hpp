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
#include "Rasterizer.hpp"
#include "DmaStreamEngineCommands.hpp"
#include "commands/TriangleStreamCmd.hpp"
#include "commands/TextureStreamCmd.hpp"
#include "RRXDisplayListAssembler.hpp"
#include "DSEDisplayListAssembler.hpp"
#include "DisplayListTextureLoadOptimizer.hpp"
#include "DisplayListStreamSectionManager.hpp"

namespace rr
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

    void finish()
    {
        m_streamSectionManager.closeStreamSection();
    }

    template <typename TCommand>
    bool addCommand(const TCommand& cmd)
    {
        if (!hasDisplayListEnoughSpace(cmd)) 
        {
            return false;
        }

        m_textureLoadOptimizer.optimize(cmd);

        if constexpr (HasCommand<decltype(cmd)>::value)
        {
            if (!m_streamSectionManager.openNewStreamSection())
            {
                return false;
            }
            m_rrxDisplayListAssembler.addCommand(cmd);
        }

        if constexpr (HasDseCommand<decltype(cmd)>::value)
        {
            m_streamSectionManager.closeStreamSection();
            m_dseDisplayListAssembler.addCommand(cmd);
        }

        return true;
    }
private:
    template<typename T> 
    class HasDseCommand 
    {
        template<typename> 
        static std::false_type test(...);
        template<typename U> 
        static auto test(int) -> decltype(std::declval<U>().dseCommand(), std::true_type());
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
        std::size_t expectedSize = 0;
        
        if constexpr (HasCommand<decltype(cmd)>::value)
        {
            expectedSize += m_rrxDisplayListAssembler.getCommandSize<TCommand>(cmd);
        }

        if constexpr (HasDseCommand<decltype(cmd)>::value)
        {
            expectedSize += m_dseDisplayListAssembler.getCommandSize<TCommand>(cmd);
        }

        if (expectedSize >= m_displayList.getFreeSpace()) 
        {
            // Not enough memory to finish the operation
            return false;
        }
        return true;
    }

    List m_displayList {};
    RRXDisplayListAssembler<List> m_rrxDisplayListAssembler { m_displayList };
    DSEDisplayListAssembler<List> m_dseDisplayListAssembler { m_displayList };
    DisplayListTextureLoadOptimizer<RenderConfig, List> m_textureLoadOptimizer { m_displayList };
    DisplayListStreamSectionManager<List> m_streamSectionManager { m_displayList };

    DSEC::SCT *m_streamCommand { nullptr };
};

} // namespace rr

#endif // DISPLAYLISTASSEMBLER_HPP

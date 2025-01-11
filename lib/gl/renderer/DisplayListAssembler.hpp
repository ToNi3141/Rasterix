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
#include "CommandDisplayListAssembler.hpp"
#include "DSEDisplayListAssembler.hpp"

namespace rr
{

template <class RenderConfig>
class DisplayListAssembler {
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
        m_streamCommand = nullptr;
        m_wasLastCommandATextureCommand.reset();
    }

    void finish()
    {
        closeStreamSection();
    }

    template <typename TCommand>
    bool addCommand(const TCommand& cmd)
    {
        if (!hasDisplayListEnoughSpace(cmd)) 
        {
            return false;
        }

        // Optimization for texture loading: To avoid unecessary texture loads, track if a texture was used by a triangle.
        // If the texture wasn't used, then it is not necessary to send to he renderer a load command.
        // Unfortunately this optimization breaks the code separation. It can be removed, the functionality of the command
        // shouldn't be affected.
        if constexpr (std::is_same<TCommand, TriangleStreamCmd<List>>::value)
        {
            // Mark that a triangle was rendered
            m_wasLastCommandATextureCommand.reset();
        }
        if constexpr (std::is_same<TCommand, TextureStreamCmd>::value)
        {
            const std::size_t tmu = cmd.getTmu();
            if (tmu >= m_wasLastCommandATextureCommand.size()) 
            {
                return false;
            }
            // Check if the last command was a texture command. If so, remove the commands from the display list
            if (m_wasLastCommandATextureCommand[tmu]) 
            {
                m_displayList.initArea(m_texPosInDisplayList[tmu], m_texSizeInDisplayList[tmu]);
            }
            // Remember the current position in the display list
            m_texPosInDisplayList[tmu] = m_displayList.getCurrentWritePos();
            // Mark that a texture was loaded
            m_wasLastCommandATextureCommand.set(tmu);
        }

        if constexpr (HasCommand<decltype(cmd)>::value)
        {
            if (!openNewStreamSection())
            {
                return false;
            }
            m_rrxDisplayListAssembler.addCommand(cmd);
        }

        if constexpr (HasDseCommand<decltype(cmd)>::value)
        {
            closeStreamSection();
            m_dseDisplayListAssembler.addCommand(cmd);
        }

        if constexpr (std::is_same<TCommand, TextureStreamCmd>::value)
        {
            const std::size_t tmu = cmd.getTmu();
            // Store the end position of the display list.
            m_texSizeInDisplayList[tmu] = m_displayList.getCurrentWritePos() - m_texPosInDisplayList[tmu];
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

    List m_displayList {};
    CommandDisplayListAssembler<List> m_rrxDisplayListAssembler { m_displayList };
    DSEDisplayListAssembler<List> m_dseDisplayListAssembler { m_displayList };

    DSEC::SCT *m_streamCommand { nullptr };

    // Helper variables to optimize the texture loading
    std::bitset<RenderConfig::TMU_COUNT> m_wasLastCommandATextureCommand {};
    std::array<uint32_t, RenderConfig::TMU_COUNT> m_texPosInDisplayList {};
    std::array<uint32_t, RenderConfig::TMU_COUNT> m_texSizeInDisplayList {};
};

} // namespace rr

#endif // DISPLAYLISTASSEMBLER_HPP

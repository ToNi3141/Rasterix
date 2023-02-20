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
#include <span>
#include "DisplayList.hpp"
#include "Rasterizer.hpp"
#include "IRenderer.hpp"
#include "DmaStreamEngineCommands.hpp"
#include "commands/TriangleStreamCmd.hpp"
#include "commands/TextureStreamCmd.hpp"

namespace rr
{

template <class RenderConfig, uint32_t DISPLAY_LIST_SIZE = RenderConfig::DISPLAYLIST_SIZE>
class DisplayListAssembler {
public:
    static constexpr uint8_t ALIGNMENT { RenderConfig::CMD_STREAM_WIDTH / 8 };
    using List = DisplayList<DISPLAY_LIST_SIZE, ALIGNMENT>;
public:
    void clearAssembler()
    {
        m_displayList.clear();
        m_streamCommand = nullptr;
        m_wasLastCommandATextureCommand.reset();
    }

    bool uploadToDeviceMemory(const uint32_t addr, const std::span<const uint8_t> data)
    {
        const std::size_t sizeOnDevice { (std::max)(data.size(), DSEC::DEVICE_MIN_TRANSFER_SIZE) }; // TODO: Maybe also check if the texture is a multiple of DEVICE_MIN_TRANSFER_SIZE
        const std::size_t expectedSize = List::template sizeOf<DSEC::SCT>() + sizeOnDevice;
        if (expectedSize > m_displayList.getFreeSpace())
        {
            return false;
        }
        closeStreamSection();
        
        appendStreamCommand<DSEC::SCT>(DSEC::OP_STORE | sizeOnDevice, addr);
        void *dest = m_displayList.alloc(sizeOnDevice);
        memcpy(dest, data.data(), data.size());
        return true;
    }

    void setCheckpoint()
    {
        closeStreamSection();
        m_displayList.setCheckpoint();
    }

    void resetToCheckpoint()
    {
        m_streamCommand = nullptr;
        m_displayList.resetToCheckpoint();
    }

    void closeStream()
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
        if constexpr (std::is_same<TCommand, TriangleStreamCmd<IRenderer::MAX_TMU_COUNT>>::value)
        {
            // Mark that a triangle was rendered
            m_wasLastCommandATextureCommand.reset();
        }
        if constexpr (std::is_same<TCommand, TextureStreamCmd<RenderConfig>>::value)
        {
            const uint8_t tmu = cmd.getTmu();
            if (tmu >= m_wasLastCommandATextureCommand.size())
            {
                return false;
            }
            // Close the current stream to avoid and undefined behaviour 
            closeStreamSection();
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

        writeCommand(cmd);

        if constexpr (HasDseOp<decltype(cmd)>::value)
        {
            writeDseCommand(cmd);
        }

        if constexpr (std::is_same<TCommand, TextureStreamCmd<RenderConfig>>::value)
        {
            const uint8_t tmu = cmd.getTmu();
            // Store the end position of the display list.
            m_texSizeInDisplayList[tmu] = m_displayList.getCurrentWritePos() - m_texPosInDisplayList[tmu];
        }

        return true;
    }

    const List* getDisplayList() const
    {
        return &m_displayList;
    }

    static constexpr uint32_t uploadCommandSize()
    {
        return List::template sizeOf<DSEC::SCT>() + List::template sizeOf<uint32_t>();
    }

private:
    template<typename T> 
    class HasDseOp 
    {
        template<typename> 
        static std::false_type test(...);
        template<typename U> 
        static auto test(int) -> decltype(std::declval<U>().dseOp(), std::true_type());
    public:
        static constexpr bool value = std::is_same<decltype(test<T>(0)), std::true_type>::value;
    };

    template <typename TArg>
    void appendStreamCommand(const DSEC::SCT op, const TArg& arg)
    {
        DSEC::SCT *opDl = m_displayList.template create<DSEC::SCT>();
        TArg *argDl = m_displayList.template create<TArg>();
        *opDl = op;
        *argDl = arg;
    }

    template <typename TArg>
    bool hasDisplayListEnoughSpace()
    {
        static constexpr std::size_t expectedSize = List::template sizeOf<uint32_t>() + List::template sizeOf<TArg>() ;

        if (expectedSize >= m_displayList.getFreeSpace())
        {
            // Not enough memory to finish the operation
            return false;
        }
        return true;
    }

    template <typename TCommand>
    bool hasDisplayListEnoughSpace(const TCommand& cmd)
    {
        using DescArray = typename TCommand::Desc;
        using DescValueType = typename DescArray::value_type::element_type;

        // Check if the display list contains enough space
        std::size_t expectedSize = List::template sizeOf<uint32_t>() + (List::template sizeOf<DescValueType>() * std::tuple_size<DescArray>());
        if constexpr (HasDseOp<decltype(cmd)>::value)
        {
            expectedSize += List::template sizeOf<DSEC::SCT>() * 2 * cmd.dseTransfer().size();
        }

        if (expectedSize >= m_displayList.getFreeSpace())
        {
            // Not enough memory to finish the operation
            return false;
        }
        return true;
    }

    template <typename TCommand>
    void writeCommand(const TCommand& cmd)
    {
        using DescArray = typename TCommand::Desc;
        using DescValueType = typename DescArray::value_type::element_type;
        if (openNewStreamSection())
        {
            // Write command
            uint32_t *opDl = m_displayList.template create<uint32_t>();
            *opDl = cmd.command();

            // Create elements
            DescArray arr;
            for (auto& a : arr)
            {
                DescValueType *argDl = m_displayList.template create<DescValueType>();
                a = { argDl, sizeof(DescValueType) };
            }
            cmd.serialize(arr);
        }
    }

    template <typename TCommand>
    void writeDseCommand(const TCommand& cmd)
    {
        if (cmd.dseOp() != DSEC::OP_NOP)
        {
            closeStreamSection();
            for (const DSEC::Transfer& t : cmd.dseTransfer())
            {
                appendStreamCommand<DSEC::SCT>(cmd.dseOp() | t.size, t.addr);
            }
        }
    }

    bool openNewStreamSection()
    {
        if (m_streamCommand == nullptr)
        {
            m_streamCommand = m_displayList.template create<DSEC::SCT>();
            m_displayList.template create<DSEC::SCT>(); // Dummy
            if (m_streamCommand)
            {
                *m_streamCommand = m_displayList.getSize();
            }
        }
        return m_streamCommand != nullptr;
    }

    bool closeStreamSection()
    {
        // Note during open, we write the current size of the display list into this command.
        // Now we just have to subtract from the current display list size the last display list size
        // to know how big our stream section is.
        if (m_streamCommand)
        {
            *m_streamCommand = DSEC::OP_STREAM | (m_displayList.getSize() - *m_streamCommand);
            m_streamCommand = nullptr;
            return true;
        }
        return false;
    }

    #pragma pack(push, 8)
    List m_displayList;
    #pragma pack(pop)

    DSEC::SCT *m_streamCommand { nullptr };

    // Helper variables to optimize the texture loading
    std::bitset<RenderConfig::TMU_COUNT> m_wasLastCommandATextureCommand {};
    std::array<uint32_t, RenderConfig::TMU_COUNT> m_texPosInDisplayList {};
    std::array<uint32_t, RenderConfig::TMU_COUNT> m_texSizeInDisplayList {};
};

} // namespace rr

#endif // DISPLAYLISTASSEMBLER_HPP

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
#include "IRenderer.hpp"
#include "DmaStreamEngineCommands.hpp"
#include "commands/TriangleStreamCmd.hpp"
#include "commands/FogLutStreamCmd.hpp"
#include "commands/TextureStreamCmd.hpp"

namespace rr
{

template <class RenderConfig, uint32_t DISPLAY_LIST_SIZE = RenderConfig::DISPLAYLIST_SIZE>
class DisplayListAssembler {
public:
    static constexpr uint8_t ALIGNMENT { RenderConfig::CMD_STREAM_WIDTH / 8 };
    using List = DisplayList<DISPLAY_LIST_SIZE, ALIGNMENT>;
private:
    static constexpr uint32_t DEVICE_MIN_TRANSFER_SIZE { 512 }; // The DSE only supports transfers as a multiple of this size. The alignment is not important.
    struct StreamCommand
    {
        // Anathomy of a command:
        // | 4 bit OP | 28 bit IMM |

        using StreamCommandType = uint32_t;

        // This mask will set the command
        static constexpr StreamCommandType STREAM_COMMAND_OP_MASK = 0xf000'0000;

        // This mask will set the immediate value
        static constexpr StreamCommandType STREAM_COMMAND_IMM_MASK = 0x0fff'ffff;

        // OPs for the DMA Stream Engine
        static constexpr StreamCommandType DSE_NOP              = 0x0000'0000;
        static constexpr StreamCommandType DSE_STORE            = 0xD000'0000;
        static constexpr StreamCommandType DSE_LOAD             = 0xB000'0000;
        static constexpr StreamCommandType DSE_STREAM           = 0x9000'0000;
        static constexpr StreamCommandType DSE_COMMIT_TO_STREAM = 0x6000'0000;
        static constexpr StreamCommandType DSE_COMMIT_TO_MEMORY = 0xE000'0000;

        // OPs for the rasterizer
        static constexpr StreamCommandType RR_OP_NOP                = 0x0000'0000;
        static constexpr StreamCommandType RR_OP_RENDER_CONFIG      = 0x1000'0000;
        static constexpr StreamCommandType RR_OP_FRAMEBUFFER        = 0x2000'0000;
        static constexpr StreamCommandType RR_OP_TRIANGLE_STREAM    = 0x3000'0000;
        static constexpr StreamCommandType RR_OP_FOG_LUT_STREAM     = 0x4000'0000;
        static constexpr StreamCommandType RR_OP_TEXTURE_STREAM     = 0x5000'0000;

        // Immediate values
        static constexpr StreamCommandType RR_TEXTURE_STREAM_SIZE_POS           = 0; // size: 8 bit
        static constexpr StreamCommandType RR_TEXTURE_STREAM_TMU_NR_POS         = 8; // size: 8 bit

        static constexpr StreamCommandType RR_OP_FRAMEBUFFER_COMMIT                 = RR_OP_FRAMEBUFFER | 0x0000'0001;
        static constexpr StreamCommandType RR_OP_FRAMEBUFFER_MEMSET                 = RR_OP_FRAMEBUFFER | 0x0000'0002;
        static constexpr StreamCommandType RR_OP_FRAMEBUFFER_COLOR_BUFFER_SELECT    = RR_OP_FRAMEBUFFER | 0x0000'0010;
        static constexpr StreamCommandType RR_OP_FRAMEBUFFER_DEPTH_BUFFER_SELECT    = RR_OP_FRAMEBUFFER | 0x0000'0020;
    };
    using SCT = typename StreamCommand::StreamCommandType;

public:
    void clearAssembler()
    {
        m_displayList.clear();
        m_streamCommand = nullptr;
        m_wasLastCommandATextureCommand.reset();
    }

    bool updateTexture(const uint32_t addr, const uint16_t* pixels, const uint32_t texSize)
    {
        closeStreamSection();
        const std::size_t texSizeOnDevice { (std::max)(texSize, DEVICE_MIN_TRANSFER_SIZE) }; // TODO: Maybe also check if the texture is a multiple of DEVICE_MIN_TRANSFER_SIZE
        bool ret = appendStreamCommand<SCT>(StreamCommand::DSE_STORE | texSizeOnDevice, addr);
        void *dest = m_displayList.alloc(texSizeOnDevice);
        if (ret && dest)
        {
            memcpy(dest, pixels, texSize);
            return true;
        }
        return false;
    }

    template <typename TCommand>
    bool addCommand(const TCommand cmd)
    {
        // Optimization for texture loading: To avoid unecessary texture loads, track if a texture was used by a triangle.
        // If the texture wasn't used, then it is not necessary to send to he renderer a load command.
        // Unfortunately this optimization breaks the code separation. It can be removed, the functionality of the command
        // shouldn't be affected.
        if constexpr (std::is_same<TCommand, TriangleStreamCmd>::value)
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

        // Add commands for the render core
        using DescArray = typename TCommand::Desc;
        using DescValueType = typename DescArray::value_type::element_type;
        if (openNewStreamSection())
        {
            // Check if the display list contains enough space
            static constexpr std::size_t expectedSize = List::template sizeOf<SCT>() + (List::template sizeOf<DescValueType>() * std::tuple_size<DescArray>());
            if (expectedSize >= m_displayList.getFreeSpace())
            {
                return false;
            }

            // Write command
            SCT *opDl = m_displayList.template create<SCT>();
            if (!opDl)
            {
                if (opDl)
                {
                    m_displayList.template remove<SCT>();
                }
                // Out of memory error
                return false;
            }
            *opDl = cmd.command();


            // Create elements
            DescArray arr;
            for (auto& a : arr)
            {
                DescValueType *argDl = m_displayList.template create<DescValueType>();
                if (argDl)
                {
                    a = { argDl, sizeof(DescValueType) };
                }
                else
                {
                    // TODO: Current elements have to be removed
                    return false;
                }
            }
            cmd.serialize(arr);
        }

        // Add commands for the DmaStreamEngine
        bool ret = true;
        if constexpr (HasDseCommand<decltype(cmd)>::value)
        {
            if (cmd.dseCommand() != DSEC::NOP) // rename dseCommand into dseOp
            {
                closeStreamSection();
                for (const DSEC::Transfer& t : cmd.dseTransfer())
                {
                    ret = ret && appendStreamCommand<SCT>(cmd.dseCommand() | t.size, t.addr);
                }
            }
        }

        if constexpr (std::is_same<TCommand, TextureStreamCmd<RenderConfig>>::value)
        {
            const uint8_t tmu = cmd.getTmu();
            // Store the end position of the display list.
            m_texSizeInDisplayList[tmu] = m_displayList.getCurrentWritePos() - m_texPosInDisplayList[tmu];
        }

        return ret;
    }

    const List* getDisplayList() const
    {
        return &m_displayList;
    }

    static constexpr uint32_t uploadCommandSize()
    {
        return List::template sizeOf<SCT>() + List::template sizeOf<uint32_t>();
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

    template <typename TArg> 
    TArg* createStreamCommand(const SCT op)
    {
        SCT *opDl = m_displayList.template create<SCT>();
        TArg *argDl = m_displayList.template create<TArg>();

        if (!(opDl && argDl))
        {
            if (opDl)
            {
                m_displayList.template remove<SCT>();
            }

            if (argDl)
            {
                m_displayList.template remove<TArg>();
            }
            // Out of memory error
            return nullptr;
        }
        *opDl = op;
        return argDl;
    }

    template <typename TArg, bool CallConstructor = false>
    bool appendStreamCommand(const SCT op, const TArg& arg)
    {
        TArg *argDl = createStreamCommand<TArg>(op);
        if (argDl)
        {
            // This is an optimization. Most of the time, a constructor call is not necessary and will just take a
            // significant amount of CPU time. So, if it is not required, we omit it.
            if constexpr (CallConstructor)
            {
                new (argDl) TArg();
            }

            *argDl = arg;
            return true;
        }
        return false;
    }

    bool openNewStreamSection()
    {
        if (m_streamCommand == nullptr)
        {
            m_streamCommand = m_displayList.template create<SCT>();
            m_displayList.template create<SCT>(); // Dummy
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
            *m_streamCommand = StreamCommand::DSE_STREAM | (m_displayList.getSize() - *m_streamCommand);
            m_streamCommand = nullptr;
            return true;
        }
        return false;
    }

    #pragma pack(push, 8)
    List m_displayList;
    #pragma pack(pop)

    SCT *m_streamCommand { nullptr };

    // Helper variables to optimize the texture loading
    std::bitset<RenderConfig::TMU_COUNT> m_wasLastCommandATextureCommand {};
    std::array<uint32_t, RenderConfig::TMU_COUNT> m_texPosInDisplayList {};
    std::array<uint32_t, RenderConfig::TMU_COUNT> m_texSizeInDisplayList {};
};

} // namespace rr

#endif // DISPLAYLISTASSEMBLER_HPP

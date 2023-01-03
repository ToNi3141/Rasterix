// Rasterix
// https://github.com/ToNi3141/Rasterix
// Copyright (c) 2022 ToNi3141

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

template <uint32_t DISPLAY_LIST_SIZE, uint8_t ALIGNMENT>
class DisplayListAssembler {
public:
        using List = DisplayList<DISPLAY_LIST_SIZE, ALIGNMENT>;
private:
    static constexpr std::size_t TMU_COUNT { IRenderer::MAX_TMU_COUNT };
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

        // Calculate the triangle size with align overhead.
        static constexpr StreamCommandType TRIANGLE_SIZE_ALIGNED = List::template sizeOf<Rasterizer::RasterizedTriangle>();

        // OPs for the DMA Stream Engine
        static constexpr StreamCommandType DSE_NOP       = 0x0000'0000;
        static constexpr StreamCommandType DSE_STORE     = 0x1000'0000;
        static constexpr StreamCommandType DSE_LOAD      = 0x2000'0000;
        static constexpr StreamCommandType DSE_MEMSET    = 0x3000'0000;
        static constexpr StreamCommandType DSE_STREAM    = 0x4000'0000;

        // OPs for the rasterizer
        static constexpr StreamCommandType RR_OP_NOP                = 0x0000'0000;
        static constexpr StreamCommandType RR_OP_RENDER_CONFIG      = 0x1000'0000;
        static constexpr StreamCommandType RR_OP_FRAMEBUFFER        = 0x2000'0000;
        static constexpr StreamCommandType RR_OP_TRIANGLE_STREAM    = 0x3000'0000;
        static constexpr StreamCommandType RR_OP_FOG_LUT_STREAM     = 0x4000'0000;
        static constexpr StreamCommandType RR_OP_TEXTURE_STREAM     = 0x5000'0000;

        // Immediate values
        static constexpr StreamCommandType RR_TEXTURE_STREAM_SIZE_POS           = 0; // size: 8 bit
        static constexpr StreamCommandType RR_TEXTURE_STREAM_OFFSET_POS         = 8; // size: 8 bit
        static constexpr StreamCommandType RR_TEXTURE_STREAM_TMU_NR_POS         = 16; // size: 8 bit

        static constexpr StreamCommandType RR_RENDER_CONFIG_FEATURE_ENABLE              = 0x0000'0000;
        static constexpr StreamCommandType RR_RENDER_CONFIG_COLOR_BUFFER_CLEAR_COLOR    = 0x0000'0001;
        static constexpr StreamCommandType RR_RENDER_CONFIG_DEPTH_BUFFER_CLEAR_DEPTH    = 0x0000'0002;
        static constexpr StreamCommandType RR_RENDER_CONFIG_FRAGMENT_PIPELINE           = 0x0000'0003;
        static constexpr StreamCommandType RR_RENDER_CONFIG_FRAGMENT_FOG_COLOR          = 0x0000'0004;
        static constexpr StreamCommandType RR_RENDER_CONFIG_SCISSOR_START_XY_CONFIG     = 0x0000'0005;
        static constexpr StreamCommandType RR_RENDER_CONFIG_SCISSOR_END_XY_CONFIG       = 0x0000'0006;
        static constexpr StreamCommandType RR_RENDER_CONFIG_Y_OFFSET                    = 0x0000'0007;
        static constexpr StreamCommandType RR_RENDER_CONFIG_TMU_OFFSET_TEX_ENV          = 0x0000'0008;
        static constexpr StreamCommandType RR_RENDER_CONFIG_TMU_OFFSET_TEX_ENV_COLOR    = 0x0000'0009;
        static constexpr StreamCommandType RR_RENDER_CONFIG_TMU_OFFSET_TEXTURE_CONFIG   = 0x0000'000A;
        static constexpr StreamCommandType RR_RENDER_CONFIG_TMU_STRIDE                  = 0x0000'0003;

        static constexpr StreamCommandType RR_OP_FRAMEBUFFER_COMMIT                 = RR_OP_FRAMEBUFFER | 0x0000'0001;
        static constexpr StreamCommandType RR_OP_FRAMEBUFFER_MEMSET                 = RR_OP_FRAMEBUFFER | 0x0000'0002;
        static constexpr StreamCommandType RR_OP_FRAMEBUFFER_COLOR_BUFFER_SELECT    = RR_OP_FRAMEBUFFER | 0x0000'0010;
        static constexpr StreamCommandType RR_OP_FRAMEBUFFER_DEPTH_BUFFER_SELECT    = RR_OP_FRAMEBUFFER | 0x0000'0020;

        static constexpr StreamCommandType RR_TRIANGLE_STREAM_FULL  = RR_OP_TRIANGLE_STREAM | TRIANGLE_SIZE_ALIGNED;
    };
    using SCT = typename StreamCommand::StreamCommandType;

public:
    static constexpr uint32_t SET_FEATURE_ENABLE            = StreamCommand::RR_RENDER_CONFIG_FEATURE_ENABLE; 
    static constexpr uint32_t SET_COLOR_BUFFER_CLEAR_COLOR  = StreamCommand::RR_RENDER_CONFIG_COLOR_BUFFER_CLEAR_COLOR;
    static constexpr uint32_t SET_DEPTH_BUFFER_CLEAR_DEPTH  = StreamCommand::RR_RENDER_CONFIG_DEPTH_BUFFER_CLEAR_DEPTH;
    static constexpr uint32_t SET_FRAGMENT_PIPELINE_CONFIG  = StreamCommand::RR_RENDER_CONFIG_FRAGMENT_PIPELINE;
    static constexpr uint32_t SET_SCISSOR_START_XY          = StreamCommand::RR_RENDER_CONFIG_SCISSOR_START_XY_CONFIG;
    static constexpr uint32_t SET_SCISSOR_END_XY            = StreamCommand::RR_RENDER_CONFIG_SCISSOR_END_XY_CONFIG;
    static constexpr uint32_t SET_FOG_COLOR                 = StreamCommand::RR_RENDER_CONFIG_FRAGMENT_FOG_COLOR;
    static constexpr uint32_t SET_TMU_TEX_ENV(const uint8_t tmu) { return StreamCommand::RR_RENDER_CONFIG_TMU_OFFSET_TEX_ENV + (StreamCommand::RR_RENDER_CONFIG_TMU_STRIDE * tmu); }
    static constexpr uint32_t SET_TMU_TEXTURE_CONFIG(const uint8_t tmu) { return StreamCommand::RR_RENDER_CONFIG_TMU_OFFSET_TEXTURE_CONFIG + (StreamCommand::RR_RENDER_CONFIG_TMU_STRIDE * tmu); }
    static constexpr uint32_t SET_TMU_TEX_ENV_COLOR(const uint8_t tmu) { return StreamCommand::RR_RENDER_CONFIG_TMU_OFFSET_TEX_ENV_COLOR + (StreamCommand::RR_RENDER_CONFIG_TMU_STRIDE * tmu); }

    static constexpr uint32_t SET_FOG_LUT                   = StreamCommand::RR_OP_FOG_LUT_STREAM;

    void clearAssembler()
    {
        m_displayList.clear();
        m_streamCommand = nullptr;
        m_wasLastCommandATextureCommand.reset();
    }

    bool commit()
    {
        if (openNewStreamSection())
        {
            // Add frame buffer flush command
            SCT *op = m_displayList.template create<SCT>();
            if (op)
            {
                *op = StreamCommand::RR_OP_FRAMEBUFFER_COMMIT | StreamCommand::RR_OP_FRAMEBUFFER_COLOR_BUFFER_SELECT;
            }

            closeStreamSection();
            return op != nullptr;
        }
        return false;
    }

    Rasterizer::RasterizedTriangle* drawTriangle()
    {
        if (openNewStreamSection())
        {
            m_wasLastCommandATextureCommand.reset();
            return createStreamCommand<Rasterizer::RasterizedTriangle>(StreamCommand::RR_TRIANGLE_STREAM_FULL);
        }
        return nullptr;
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

    bool useTexture(const uint8_t tmu,
                    const tcb::span<const uint16_t> pages,
                    const uint32_t pageSize,
                    const uint32_t texSize)
    {
        bool ret = false;
        const std::size_t texSizeOnDevice { (std::max)(texSize, DEVICE_MIN_TRANSFER_SIZE) }; // TODO: Maybe also check if the texture is a multiple of DEVICE_MIN_TRANSFER_SIZE
        const std::size_t ps { (texSizeOnDevice > pageSize) ? pageSize : texSizeOnDevice };
        closeStreamSection();
        if (m_wasLastCommandATextureCommand[tmu])
        {
            m_displayList.initArea(m_texPosInDisplayList[tmu], m_texSizeInDisplayList[tmu]);
        }

        m_texPosInDisplayList[tmu] = m_displayList.getCurrentWritePos();
        if (openNewStreamSection())
        {
            SCT *texStreamOp { nullptr };
            SCT *texLoad { nullptr };
            uint32_t *texLoadAddr { nullptr };
            texStreamOp = m_displayList.template create<SCT>();
            if (texStreamOp)
            {
                const uint32_t texSizeLog2 = static_cast<uint32_t>(std::log2(static_cast<float>(texSizeOnDevice))) << StreamCommand::RR_TEXTURE_STREAM_SIZE_POS;
                const uint32_t tmuShifted = static_cast<uint32_t>(tmu) << StreamCommand::RR_TEXTURE_STREAM_TMU_NR_POS;

                *texStreamOp = StreamCommand::RR_OP_TEXTURE_STREAM | texSizeLog2 | tmuShifted;
                closeStreamSection();
            }
            else
            {
                return false;
            }
            m_wasLastCommandATextureCommand.set(tmu);
            for (const uint16_t p : pages)
            {
                texLoad = m_displayList.template create<SCT>();
                texLoadAddr = m_displayList.template create<uint32_t>();
                if (texLoad && texLoadAddr)
                {
                    *texLoad = StreamCommand::DSE_LOAD | ps;
                    *texLoadAddr = p * pageSize;
                    ret = true;
                }
                else
                {
                    m_displayList.initArea(m_texPosInDisplayList[tmu], m_displayList.getCurrentWritePos() - m_texPosInDisplayList[tmu]);
                }
            }
            m_texSizeInDisplayList[tmu] = m_displayList.getCurrentWritePos() - m_texPosInDisplayList[tmu];
        }

        return ret;
    }

    bool clear(bool colorBuffer, bool depthBuffer)
    {
        if (openNewStreamSection())
        {
            const SCT opColorBuffer = StreamCommand::RR_OP_FRAMEBUFFER_MEMSET | StreamCommand::RR_OP_FRAMEBUFFER_COLOR_BUFFER_SELECT;
            const SCT opDepthBuffer = StreamCommand::RR_OP_FRAMEBUFFER_MEMSET | StreamCommand::RR_OP_FRAMEBUFFER_DEPTH_BUFFER_SELECT;

            SCT *op = m_displayList.template create<SCT>();
            if (op)
            {
                if (colorBuffer && depthBuffer)
                {
                    *op = opColorBuffer | opDepthBuffer;
                }
                else if (colorBuffer)
                {
                    *op = opColorBuffer;
                }
                else if (depthBuffer)
                {
                    *op = opDepthBuffer;
                }
                else
                {
                    *op = StreamCommand::RR_OP_NOP;
                }
            }
            return op != nullptr;
        }
        return false;
    }

    template <typename TArg>
    bool writeRegister(uint32_t regIndex, const TArg& regVal)
    {
        if (openNewStreamSection())
        {
            return appendStreamCommand<TArg>(StreamCommand::RR_OP_RENDER_CONFIG | regIndex, regVal);
        }
        return false;
    }

    template <typename TArg, std::size_t TSize>
    bool writeArray(uint32_t command, const std::array<TArg, TSize>& arr)
    {
        if (openNewStreamSection())
        {
            // Check if the display list contains enough space
            std::size_t expectedSize = List::template sizeOf<SCT>() + (List::template sizeOf<TArg>() * TSize);
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
            *opDl = command;

            // Copy array elements
            for (auto a : arr)
            {
                TArg *argDl = m_displayList.template create<TArg>();
                if (argDl)
                {
                    *argDl = a;
                }
                else
                {
                    // TODO: Current elements have to be removed
                    return false;
                }
            }
            return true;
        }
        return false;
    }

    bool setYOffset(const uint32_t offset)
    {
        return writeRegister(StreamCommand::RR_RENDER_CONFIG_Y_OFFSET, offset);
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
    std::bitset<TMU_COUNT> m_wasLastCommandATextureCommand {};
    std::array<uint32_t, TMU_COUNT> m_texPosInDisplayList {};
    std::array<uint32_t, TMU_COUNT> m_texSizeInDisplayList {};
};


#endif // DISPLAYLISTASSEMBLER_HPP

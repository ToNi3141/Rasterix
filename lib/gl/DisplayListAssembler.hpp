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
#include "DisplayList.hpp"
#include "Rasterizer.hpp"

template <uint32_t DISPLAY_LIST_SIZE, uint8_t ALIGNMENT>
class DisplayListAssembler {
public:
        using List = DisplayList<DISPLAY_LIST_SIZE, ALIGNMENT>;
private:
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
        static constexpr StreamCommandType RR_NOP              = 0x0000'0000;
        static constexpr StreamCommandType RR_TEXTURE_STREAM   = 0x1000'0000;
        static constexpr StreamCommandType RR_SET_REG          = 0x2000'0000;
        static constexpr StreamCommandType RR_FRAMEBUFFER_OP   = 0x3000'0000;
        static constexpr StreamCommandType RR_TRIANGLE_STREAM  = 0x4000'0000;
        static constexpr StreamCommandType RR_FOG_LUT_STREAM   = 0x5000'0000;

        // Immediate values
        static constexpr StreamCommandType RR_TEXTURE_STREAM_SIZE_POS   = 0; // size: 8 bit
        static constexpr StreamCommandType RR_TEXTURE_STREAM_TMU_NR_POS = 8; // size: 8 bit
       

        static constexpr StreamCommandType RR_COLOR_BUFFER_CLEAR_COLOR_REG_ADDR = 0x0000;
        static constexpr StreamCommandType RR_DEPTH_BUFFER_CLEAR_DEPTH_REG_ADDR = RR_COLOR_BUFFER_CLEAR_COLOR_REG_ADDR + 1;
        static constexpr StreamCommandType RR_FOG_COLOR_ADDR                    = RR_DEPTH_BUFFER_CLEAR_DEPTH_REG_ADDR + 1;
        static constexpr StreamCommandType RR_CONF_REG1_ADDR                    = RR_FOG_COLOR_ADDR + 1;
        static constexpr StreamCommandType RR_CONF_REG2_ADDR                    = RR_CONF_REG1_ADDR + 1;
        static constexpr StreamCommandType RR_CONF_REG3_ADDR                    = RR_CONF_REG2_ADDR + 1;
        static constexpr StreamCommandType RR_TEX_ENV_COLOR_REG_ADDR            = RR_CONF_REG3_ADDR + 1;
        

        static constexpr StreamCommandType RR_FRAMEBUFFER_COMMIT   = RR_FRAMEBUFFER_OP | 0x0001;
        static constexpr StreamCommandType RR_FRAMEBUFFER_MEMSET   = RR_FRAMEBUFFER_OP | 0x0002;
        static constexpr StreamCommandType RR_FRAMEBUFFER_COLOR    = RR_FRAMEBUFFER_OP | 0x0010;
        static constexpr StreamCommandType RR_FRAMEBUFFER_DEPTH    = RR_FRAMEBUFFER_OP | 0x0020;

        static constexpr StreamCommandType RR_TEXTURE_STREAM_FULL  = RR_TRIANGLE_STREAM | TRIANGLE_SIZE_ALIGNED;
    };
    using SCT = typename StreamCommand::StreamCommandType;

public:
    static constexpr uint32_t SET_COLOR_BUFFER_CLEAR_COLOR = StreamCommand::RR_COLOR_BUFFER_CLEAR_COLOR_REG_ADDR;
    static constexpr uint32_t SET_DEPTH_BUFFER_CLEAR_DEPTH = StreamCommand::RR_DEPTH_BUFFER_CLEAR_DEPTH_REG_ADDR;
    static constexpr uint32_t SET_CONF_REG1                = StreamCommand::RR_CONF_REG1_ADDR;
    static constexpr uint32_t SET_CONF_REG2                = StreamCommand::RR_CONF_REG2_ADDR;
    static constexpr uint32_t SET_CONF_REG3                = StreamCommand::RR_CONF_REG3_ADDR;
    static constexpr uint32_t SET_TEX_ENV_COLOR            = StreamCommand::RR_TEX_ENV_COLOR_REG_ADDR;
    static constexpr uint32_t SET_FOG_COLOR                = StreamCommand::RR_FOG_COLOR_ADDR;
    static constexpr uint32_t SET_FOG_LUT                  = StreamCommand::RR_FOG_LUT_STREAM;

    void clearAssembler()
    {
        m_displayList.clear();
        m_streamCommand = nullptr;
        m_wasLastCommandATextureCommand = false;
    }

    bool commit()
    {
        if (openNewStreamSection())
        {
            // Add frame buffer flush command
            SCT *op = m_displayList.template create<SCT>();
            if (op)
            {
                *op = StreamCommand::RR_FRAMEBUFFER_COMMIT | StreamCommand::RR_FRAMEBUFFER_COLOR;
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
            m_wasLastCommandATextureCommand = false;
            return createStreamCommand<Rasterizer::RasterizedTriangle>(StreamCommand::RR_TEXTURE_STREAM_FULL);
        }
        return nullptr;
    }

    bool updateTexture(const uint32_t addr, std::shared_ptr<const uint16_t> pixels, const uint32_t texSize)
    {
        closeStreamSection();
        bool ret = appendStreamCommand<SCT>(StreamCommand::DSE_STORE | texSize, addr);
        void *dest = m_displayList.alloc(texSize);
        if (ret && dest)
        {
            memcpy(dest, pixels.get(), texSize);
            return true;
        }
        return false;
    }

    bool useTexture(const uint32_t texAddr, 
                    const uint32_t texSize)
    {
        bool ret = false;
        if (openNewStreamSection())
        {
            // Check if the last command was a texture command and not a triangle. If no triangle has to be drawn
            // with the recent texture, then we can just overwrite this texture with the current one and avoiding
            // with that mechanism unnecessary texture loads.
            if (!m_wasLastCommandATextureCommand)
            {
                m_texStreamOp = m_displayList.template create<SCT>();
                if (m_texStreamOp)
                {
                    closeStreamSection();
                    m_texLoad = m_displayList.template create<SCT>();
                    m_texLoadAddr = m_displayList.template create<uint32_t>();
                }
            }
            if (m_texStreamOp && m_texLoad && m_texLoadAddr)
            {
                const uint32_t texSizeLog2 = static_cast<uint32_t>(std::log2(static_cast<float>(texSize))) << StreamCommand::RR_TEXTURE_STREAM_SIZE_POS;

                *m_texStreamOp = StreamCommand::RR_TEXTURE_STREAM 
                    | texSizeLog2;

                *m_texLoad = StreamCommand::DSE_LOAD | texSize;
                *m_texLoadAddr = texAddr;
                m_wasLastCommandATextureCommand = true;
                ret = true;
            }
            else
            {
                if (!m_wasLastCommandATextureCommand)
                {
                    if (m_texStreamOp)
                    {
                        *m_texStreamOp = StreamCommand::DSE_NOP;
                    }
                    if (m_texLoad)
                    {
                        m_displayList.template remove<SCT>();
                    }
                    if (m_texLoadAddr)
                    {
                        m_displayList.template remove<uint32_t>();
                    }
                }
            }
        }

        return ret;
    }

    bool clear(bool colorBuffer, bool depthBuffer)
    {
        if (openNewStreamSection())
        {
            const SCT opColorBuffer = StreamCommand::RR_FRAMEBUFFER_MEMSET | StreamCommand::RR_FRAMEBUFFER_COLOR;
            const SCT opDepthBuffer = StreamCommand::RR_FRAMEBUFFER_MEMSET | StreamCommand::RR_FRAMEBUFFER_DEPTH;

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
                    *op = StreamCommand::RR_NOP;
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
            return appendStreamCommand<TArg>(StreamCommand::RR_SET_REG | regIndex, regVal);
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
        // Note during open, we write the current size of the displaylist into this command.
        // Now we just have to substract from the current display list size the last display list size
        // to know how big our stream section is.
        if (m_streamCommand)
        {
            *m_streamCommand = StreamCommand::DSE_STREAM | (m_displayList.getSize() - *m_streamCommand);
            m_streamCommand = nullptr;
            return true;
        }
        return false;
    }

    List m_displayList __attribute__ ((aligned (8)));

    SCT *m_streamCommand{nullptr};

    // Helper variables to optimize the texture loading
    bool m_wasLastCommandATextureCommand{false};
    SCT *m_texStreamOp{nullptr};
    SCT *m_texLoad{nullptr};
    uint32_t *m_texLoadAddr{nullptr};
};


#endif // DISPLAYLISTASSEMBLER_HPP

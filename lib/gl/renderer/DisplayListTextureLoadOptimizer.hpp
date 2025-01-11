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

#ifndef TEXTURELOADOPTIMIZER_HPP
#define TEXTURELOADOPTIMIZER_HPP

#include <stdint.h>
#include <array>
#include <bitset>
#include <algorithm>
#include <tcb/span.hpp>
#include "commands/TriangleStreamCmd.hpp"
#include "commands/TextureStreamCmd.hpp"
#include "CommandDisplayListAssembler.hpp"

namespace rr
{
// Optimization for texture loading: To avoid unecessary texture loads, track if a texture was used by a triangle.
// If the texture wasn't used, then it is not necessary to send to the renderer a load command.
template <typename RenderConfig, typename TDisplayList>
class DisplayListTextureLoadOptimizer 
{
public:
    DisplayListTextureLoadOptimizer(TDisplayList& displayList)
        : m_displayList { displayList }
    {
        reset();
    }

    void reset()
    {
        m_textureCommandFlag.reset();
    }

    template <typename TCommand>
    void optimize(const TCommand& cmd)
    {
        markTriangleCommand<TCommand>();
        optimizeTextureLoad(cmd);
    }
private:
    template <typename TCommand>
    void markTriangleCommand()
    {
        if constexpr (std::is_same<TCommand, TriangleStreamCmd<TDisplayList>>::value)
        {
            m_textureCommandFlag.reset();
        }
    }

    template <typename TCommand>
    void optimizeTextureLoad(const TCommand& cmd)
    {
        if constexpr (std::is_same<TCommand, TextureStreamCmd>::value)
        {
            const std::size_t tmu = cmd.getTmu();
            if (tmu >= m_textureCommandFlag.size()) 
            {
                return;
            }
            if (m_textureCommandFlag[tmu]) 
            {
                m_displayList.initArea(m_texPosInDisplayList[tmu], m_texSizeInDisplayList[tmu]);
            }
            m_textureCommandFlag.set(tmu);
            m_texPosInDisplayList[tmu] = m_displayList.getCurrentWritePos();
            m_texSizeInDisplayList[tmu] = CommandDisplayListAssembler<TDisplayList>::template getCommandSize(cmd);
        }
    }

    TDisplayList& m_displayList;

    std::bitset<RenderConfig::TMU_COUNT> m_textureCommandFlag {};
    std::array<uint32_t, RenderConfig::TMU_COUNT> m_texPosInDisplayList {};
    std::array<uint32_t, RenderConfig::TMU_COUNT> m_texSizeInDisplayList {};
};

} // namespace rr

#endif // TEXTURELOADOPTIMIZER_HPP

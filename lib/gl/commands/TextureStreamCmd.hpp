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


#ifndef _TEXTURE_STREAM_CMD_HPP_
#define _TEXTURE_STREAM_CMD_HPP_

#include <cstdint>
#include <array>
#include <span>
#include "DmaStreamEngineCommands.hpp"

namespace rr
{

template <class RenderConfig>
class TextureStreamCmd
{
    static constexpr uint32_t MAX_PAGES { static_cast<uint32_t>((static_cast<float>(RenderConfig::MAX_TEXTURE_SIZE * RenderConfig::MAX_TEXTURE_SIZE * 2.0f * 1.33f) / static_cast<float>(RenderConfig::TEXTURE_PAGE_SIZE)) + 1.0f) };
    static constexpr uint32_t OP_TEXTURE_STREAM { 0x5000'0000 };
    static constexpr uint32_t TEXTURE_STREAM_SIZE_POS { 0 }; // size: 18 bit
    static constexpr uint32_t TEXTURE_STREAM_TMU_NR_POS { 19 }; // size: 2 bit
    using DseTransferType = std::span<const DSEC::Transfer>;
public:
    TextureStreamCmd(const uint8_t tmu,
                    const std::span<const uint16_t>& pages)
        : m_tmu { tmu }
        , m_texSize { pages.size() * RenderConfig::TEXTURE_PAGE_SIZE }
    {
        for (uint32_t i = 0; i < pages.size(); i++)
        {
            m_pages[i] = { RenderConfig::GRAM_MEMORY_LOC + (pages[i] * RenderConfig::TEXTURE_PAGE_SIZE), RenderConfig::TEXTURE_PAGE_SIZE };
        }
        m_dseData = { m_pages.data(), pages.size() };  
    }

    uint8_t getTmu() const { return m_tmu; }

    using Desc = std::array<std::span<uint8_t>, 0>;
    void serialize(Desc&) const {}
    uint32_t command() const 
    { 
        const uint32_t texSize = static_cast<uint32_t>(m_texSize) << TEXTURE_STREAM_SIZE_POS;
        const uint32_t tmuShifted = static_cast<uint32_t>(m_tmu) << TEXTURE_STREAM_TMU_NR_POS;
        return OP_TEXTURE_STREAM | texSize | tmuShifted;
    }

    DSEC::SCT dseOp() const { return DSEC::OP_LOAD; }
    const DseTransferType& dseTransfer() const { return m_dseData; }
private:
    uint8_t m_tmu {};
    std::array<DSEC::Transfer, MAX_PAGES> m_pages;
    std::size_t m_texSize {};
    DseTransferType m_dseData {};
};

} // namespace rr

#endif // _TEXTURE_STREAM_CMD_HPP_

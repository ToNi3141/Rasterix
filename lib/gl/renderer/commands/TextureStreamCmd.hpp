// RasterIX
// https://github.com/ToNi3141/RasterIX
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

#include "RenderConfigs.hpp"
#include "renderer/displaylist/DisplayList.hpp"
#include <array>
#include <cstdint>
#include <tcb/span.hpp>

namespace rr
{

class TextureStreamCmd
{
    static constexpr uint32_t MAX_PAGES { static_cast<uint32_t>((static_cast<float>(RenderConfig::MAX_TEXTURE_SIZE * RenderConfig::MAX_TEXTURE_SIZE * 2.0f * 1.33f) / static_cast<float>(RenderConfig::TEXTURE_PAGE_SIZE)) + 1.0f) };
    static constexpr uint32_t OP_TEXTURE_STREAM { 0x5000'0000 };
    static constexpr uint32_t TEXTURE_STREAM_SIZE_POS { 0 }; // size: 18 bit
    static constexpr uint32_t TEXTURE_STREAM_SIZE_MASK { 0x3FFFF }; // size: 18 bit
    static constexpr uint32_t TEXTURE_STREAM_TMU_NR_POS { 19 }; // size: 2 bit
    static constexpr uint32_t OP_MASK { 0xF000'0000 };

public:
    TextureStreamCmd() = default;
    TextureStreamCmd(const std::size_t tmu,
        const tcb::span<const std::size_t>& pages)
        : m_tmu { tmu }
        , m_texSize { pages.size() }
    {
        for (std::size_t i = 0; i < pages.size(); i++)
        {
            m_pages[i] = RenderConfig::GRAM_MEMORY_LOC + static_cast<uint32_t>(pages[i] * RenderConfig::TEXTURE_PAGE_SIZE);
        }
        m_payload = { m_pages.data(), pages.size() };
    }

    std::size_t getTmu() const { return m_tmu; }

    using PayloadType = tcb::span<const uint32_t>;
    const PayloadType& payload() const { return m_payload; }
    using CommandType = uint32_t;
    CommandType command() const
    {
        const uint32_t texSize = static_cast<uint32_t>(m_texSize) << TEXTURE_STREAM_SIZE_POS;
        const uint32_t tmuShifted = static_cast<uint32_t>(m_tmu) << TEXTURE_STREAM_TMU_NR_POS;
        return OP_TEXTURE_STREAM | texSize | tmuShifted;
    }

    static std::size_t getNumberOfElementsInPayloadByCommand(const uint32_t cmd) { return cmd & TEXTURE_STREAM_SIZE_MASK; }
    static bool isThis(const CommandType cmd) { return (cmd & OP_MASK) == OP_TEXTURE_STREAM; }

private:
    std::size_t m_tmu {};
    std::array<uint32_t, MAX_PAGES> m_pages;
    std::size_t m_texSize {};
    PayloadType m_payload {};
};

} // namespace rr

#endif // _TEXTURE_STREAM_CMD_HPP_

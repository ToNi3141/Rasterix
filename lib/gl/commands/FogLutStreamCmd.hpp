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


#ifndef _FOG_LUT_STREAM_CMD_HPP_
#define _FOG_LUT_STREAM_CMD_HPP_

#include <cstdint>
#include <array>
#include <tcb/span.hpp>

namespace rr
{

class FogLutStreamCmd
{
    static constexpr std::size_t LUT_SIZE { 33 };
    static constexpr uint32_t FOG_LUT_STREAM { 0x4000'0000 };
public:
    void setBounds(const float lower, const float upper)
    {
        m_lut[0].floats.lower = lower;
        m_lut[0].floats.upper = upper;
    }
    
    void setLutValue(const uint8_t index, const float m, const float b)
    {
        m_lut[index + 1].numbers.m = m;
        m_lut[index + 1].numbers.b = b;
    }

    using Desc = std::array<tcb::span<uint64_t>, LUT_SIZE>;
    void serialize(Desc& desc) const 
    { 
        for (uint8_t i = 0; i < desc.size(); i++)
        {
            *(desc[i].data()) = m_lut[i].val;
        }
    }
    static constexpr uint32_t command() { return FOG_LUT_STREAM; }
private:
    union Value {
#pragma pack(push, 4)
        uint64_t val;
        struct {
            int32_t m;
            int32_t b;
        } numbers;
        struct {
            float lower;
            float upper;
        } floats;
#pragma pack(pop)
    };

    std::array<Value, LUT_SIZE> m_lut;
};

} // namespace rr

#endif // _FOG_LUT_STREAM_CMD_HPP_

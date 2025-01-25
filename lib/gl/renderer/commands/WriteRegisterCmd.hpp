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


#ifndef _WRITE_REGISTER_CMD_HPP_
#define _WRITE_REGISTER_CMD_HPP_

#include <cstdint>
#include <array>
#include <tcb/span.hpp>

namespace rr
{

template <class TRegister>
class WriteRegisterCmd
{
    static constexpr uint32_t OP_RENDER_CONFIG { 0x1000'0000 };
public:
    WriteRegisterCmd(const TRegister& reg)
    {
        m_val[0] = reg.serialize();
        m_op = OP_RENDER_CONFIG | reg.getAddr();
        m_payload = { m_val };
    }

    using PayloadType = tcb::span<const uint32_t>;
    const PayloadType& payload() const { return m_payload; }
    using CommandType = uint32_t;
    CommandType command() const { return m_op; }

private:
    CommandType m_op {};
    std::array<uint32_t, 1> m_val;
    PayloadType m_payload;
};

} // namespace rr

#endif // _WRITE_REGISTER_CMD_HPP_

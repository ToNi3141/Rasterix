// RRX
// https://github.com/ToNi3141/RRX
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

#ifndef _BASE_XY_REG_
#define _BASE_XY_REG_

#include "math/Veci.hpp"
#include <cstdint>

namespace rr
{
template <uint16_t MASK_X, uint16_t MASK_Y>
class BaseXYReg
{
public:
    BaseXYReg() = default;
    BaseXYReg(const uint16_t x, const uint16_t y)
    {
        setX(x);
        setY(y);
    }

    void setX(const uint16_t val) { m_regValX = val & MASK_X; }
    void setY(const uint16_t val) { m_regValY = val & MASK_Y; }

    uint16_t getX() const { return m_regValX; }
    uint16_t getY() const { return m_regValY; }

    uint32_t serialize() const { return (static_cast<uint32_t>(m_regValY) << 16) | (static_cast<uint32_t>(m_regValX)); }
    void deserialize(uint32_t data)
    {
        static constexpr uint32_t mask = 0xffff;
        m_regValX = data & mask;
        m_regValY = (data >> 16) & mask;
    }

private:
    uint16_t m_regValX { 0 };
    uint16_t m_regValY { 0 };
};
} // namespace rr

#endif // _BASE_XY_REG_

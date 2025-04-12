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

#ifndef _TEX_ENV_COLOR_REG_
#define _TEX_ENV_COLOR_REG_

#include "renderer/registers/BaseColorReg.hpp"

namespace rr
{
class TexEnvColorReg : public BaseColorReg
{
public:
    TexEnvColorReg(const std::size_t tmu, const Vec4i& color)
        : BaseColorReg { color }
    {
        setTmu(tmu);
    }

    void setTmu(const std::size_t tmu) { m_offset = tmu * TMU_OFFSET; }
    uint32_t getAddr() const { return 0xB + m_offset; }

private:
    static constexpr std::size_t TMU_OFFSET { 3 };
    std::size_t m_offset { 0 };
};
} // namespace rr

#endif // _TEX_ENV_COLOR_REG_

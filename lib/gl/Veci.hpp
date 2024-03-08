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

#ifndef VECI_HPP
#define VECI_HPP
#include <array>
#include <cstdint>
#include <algorithm>

namespace rr
{
// This is a configurable fixed point vector class
template <typename T, uint8_t VecSize>
class Veci
{
public:
    Veci() {}
    Veci(const Veci<T, VecSize>& val) { operator=(val.vec); }
    Veci(const std::array<T, VecSize>& val) { operator=(val); }
    ~Veci() {}

    Veci<T, VecSize>& operator*= (T val)
    {
        for (uint32_t i = 0; i < VecSize; i++)
            vec[i] = vec[i] * val;
        return *this;
    }

    Veci<T, VecSize>& operator*= (const Veci<T, VecSize>& val)
    {
        for (uint32_t i = 0; i < VecSize; i++)
            vec[i] = vec[i] * val[i];
        return *this;
    }

    template <uint8_t shift>
    void div(T val)
    {
        for (uint32_t i = 0; i < VecSize; i++)
            vec[i] = (((static_cast<int64_t>(vec[i]) << shift) / static_cast<int64_t>(val)));
    }

    template <uint8_t shift>
    void mul(T val)
    {
        for (uint32_t i = 0; i < VecSize; i++)
            vec[i] = (static_cast<int64_t>(vec[i]) * val) >> shift;
    }

    template <uint8_t shift>
    void mul(const Veci<T, VecSize>& val)
    {
        for (uint32_t i = 0; i < VecSize; i++)
            vec[i] = (static_cast<int64_t>(vec[i]) * val[i]) >> shift;
    }

    Veci<T, VecSize>& operator+= (const Veci<T, VecSize>& val)
    {
        for (uint32_t i = 0; i < VecSize; i++)
            vec[i] += val[i];
        return *this;
    }

    Veci<T, VecSize>& operator-= (const Veci<T, VecSize>& val)
    {
        for (uint32_t i = 0; i < VecSize; i++)
            vec[i] -= val[i];
        return *this;
    }

    Veci<T, VecSize> operator<<= (uint8_t val)
    {
        for (uint32_t i = 0; i < VecSize; i++)
            vec[i] <<= val;
        return *this;
    }

    Veci<T, VecSize> operator>>= (uint8_t val)
    {
        for (uint32_t i = 0; i < VecSize; i++)
            vec[i] >>= val;
        return *this;
    }

    template <uint8_t shift = 0>
    void fromVec(const std::array<float, VecSize> val)
    {
        for (uint32_t i = 0; i < VecSize; i++)
            vec[i] = (val[i] * (1ul << shift)) + 0.5f;
    }

    T& operator[] (int index) { return vec[index]; }
    T operator[] (int index) const { return vec[index]; }
    void operator= (const std::array<T, VecSize>& val) { vec = val; }

    template <uint8_t shift = 0>
    int64_t dot(const Veci<T, VecSize>& val) const
    {
        int64_t retVal = 0;
        for (uint32_t i = 0; i < VecSize; i++)
            retVal += (static_cast<int64_t>(vec[i]) * val[i]);
        return retVal >> shift;
    }

    void clamp(const T low, const T high)
    {
        for (uint32_t i = 0; i < VecSize; i++)
        {
            vec[i] = std::clamp(vec[i], low, high);
        }
    }

    std::array<T, VecSize> vec;
};

using VecInt = int32_t;
using Vec2i = Veci<VecInt, 2>;
using Vec3i = Veci<VecInt, 3>;
using Vec4i = Veci<VecInt, 4>;
} // namespace rr
#endif // VECI_HPP

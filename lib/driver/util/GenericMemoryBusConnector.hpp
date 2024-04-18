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

#ifndef GENERICMEMORYBUSCONNECTOR_HPP
#define GENERICMEMORYBUSCONNECTOR_HPP

#include "renderer/IBusConnector.hpp"
#include <array>
#include <span>
#include <cstdint>

namespace rr
{
template <uint32_t NUMBER_OF_DISPLAY_LISTS, uint32_t DISPLAY_LIST_SIZE>
class GenericMemoryBusConnector : public IBusConnector
{
public:
    virtual ~GenericMemoryBusConnector() = default;

    virtual std::span<uint8_t> requestBuffer(const uint8_t index) override { return { m_dlMem[index] }; }
    virtual uint8_t getBufferCount() const override { return m_dlMem.size(); }
protected:
    std::array<std::array<uint8_t, DISPLAY_LIST_SIZE>, NUMBER_OF_DISPLAY_LISTS> m_dlMem;
};

} // namespace rr
#endif // #ifndef GENERICMEMORYBUSCONNECTOR_HPP

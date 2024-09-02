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

#ifndef DISPLAYLIST_HPP
#define DISPLAYLIST_HPP

#include <stdint.h>
#include <cstring>
#include <tcb/span.hpp>

namespace rr
{

template <uint8_t ALIGNMENT>
class DisplayList {
public:
    void setBuffer(tcb::span<uint8_t> buffer)
    {
        mem = buffer;
    }

    // Interface for writing the display list

    void* __restrict alloc(const uint32_t size)
    {
        if ((size + writePos) <= mem.size()) [[likely]]
        {
            void* memPlace = &mem[writePos];
            writePos += size;
            return memPlace;
        }
        return nullptr;
    }

    template <typename GET_TYPE>
    GET_TYPE* __restrict create()
    {
        static constexpr uint32_t size = sizeOf<GET_TYPE>();
        return reinterpret_cast<GET_TYPE* __restrict>(alloc(size));
    }

    template <typename GET_TYPE>
    void remove()
    {
        static constexpr uint32_t size = sizeOf<GET_TYPE>();
        if (size <= writePos) [[likely]]
        {
            writePos -= size;
        }
    }

    template <typename GET_TYPE>
    static constexpr uint32_t sizeOf()
    {
        constexpr uint32_t mask = ALIGNMENT - 1;
        return (sizeof(GET_TYPE) + mask) & ~mask;
    }

    void clear()
    {
        writePos = 0;
    }

    void setCheckpoint()
    {
        checkpoint = writePos;
    }

    void resetToCheckpoint()
    {
        writePos = checkpoint;
    }

    tcb::span<const uint8_t> getMemPtr() const
    {
        return { mem.data(), getSize() };
    }

    uint32_t getSize() const
    {
        return writePos;
    }

    uint32_t getFreeSpace() const
    {
        return mem.size() - writePos;
    }

    void initArea(const uint32_t start, const uint32_t size)
    {
        memset(&mem[start], 0, size);
    }

    uint32_t getCurrentWritePos() const
    {
        return writePos;
    }

    // Interface for reading the display list

    template <typename GET_TYPE>
    GET_TYPE* __restrict lookAhead()
    {
        static constexpr uint32_t size = sizeOf<GET_TYPE>();
        if ((size + readPos) <= writePos) [[likely]]
        {
            return reinterpret_cast<GET_TYPE* __restrict>(&mem[readPos]);
        }
        return nullptr;
    }

    template <typename GET_TYPE>
    GET_TYPE* __restrict getNext()
    {
        static constexpr uint32_t size = sizeOf<GET_TYPE>();
        if ((size + readPos) <= writePos) [[likely]]
        {
            void* memPlace = &mem[readPos];
            readPos += size;
            return reinterpret_cast<GET_TYPE* __restrict>(memPlace);
        }
        return nullptr;
    }

    bool atEnd() const
    {
        return writePos <= readPos;
    }

    void resetGet()
    {
        readPos = 0;
    }

private:
    tcb::span<uint8_t> mem;
    uint32_t writePos { 0 };
    uint32_t readPos { 0 };
    uint32_t checkpoint { 0 };
};

} // namespace rr
#endif // DISPLAYLIST_HPP

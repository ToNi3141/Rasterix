// RasteriCEr
// https://github.com/ToNi3141/RasteriCEr
// Copyright (c) 2021 ToNi3141

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

template <uint32_t DISPLAY_LIST_SIZE, uint8_t ALIGNMENT>
class DisplayList {
public:
    enum State
    {
        IDLE,
        QUEUED,
        TRANSFERRING
    };

    // Interface for writing the display list

    template <typename GET_TYPE>
    GET_TYPE* create()
    {
        static constexpr uint32_t size = sizeOf<GET_TYPE>();
        if ((size + consumedMemory) <= DISPLAY_LIST_SIZE)
        {
            void* memPlace = &mem[consumedMemory];
            consumedMemory += size;
            return static_cast<GET_TYPE*>(memPlace);
        }
        return nullptr;
    }

    template <typename GET_TYPE>
    void remove()
    {
        static constexpr uint32_t size = sizeOf<GET_TYPE>();
        if (size <= consumedMemory)
        {
            consumedMemory -= size;
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
        listState = State::IDLE;
        consumedMemory = 0;
    }

    const uint8_t* getMemPtr() const
    {
        return &mem[0];
    }

    uint32_t getSize() const
    {
        return consumedMemory;
    }

    uint32_t getFreeSpace() const
    {
        return DISPLAY_LIST_SIZE - consumedMemory;
    }

    State state() const
    {
        return listState;
    }

    void enqueue()
    {
        listState = State::QUEUED;
    }

    void transfer()
    {
        listState = State::TRANSFERRING;
    }

    // Interface for reading the display list

    template <typename GET_TYPE>
    GET_TYPE* lookAhead()
    {
        static constexpr uint32_t size = sizeOf<GET_TYPE>();
        if ((size + getPos) <= consumedMemory)
        {
            return reinterpret_cast<GET_TYPE*>(&mem[getPos]);
        }
        return nullptr;
    }

    template <typename GET_TYPE>
    GET_TYPE* getNext()
    {
        static constexpr uint32_t size = sizeOf<GET_TYPE>();
        if ((size + getPos) <= consumedMemory)
        {
            void* memPlace = &mem[getPos];
            getPos += size;
            return reinterpret_cast<GET_TYPE*>(memPlace);
        }
        return nullptr;
    }

    bool atEnd() const
    {
        return consumedMemory <= getPos;
    }

    void resetGet()
    {
        getPos = 0;
    }

private:
    uint8_t mem[DISPLAY_LIST_SIZE];
    State listState = State::IDLE;
    uint32_t consumedMemory = 0;
    uint32_t getPos = 0;
};

#endif // DISPLAYLIST_HPP

// RasteriCEr
// https://github.com/ToNi3141/Rasterix
// Copyright (c) 2022 ToNi3141

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

#ifndef GRAMALLOC_HPP
#define GRAMALLOC_HPP
#include <stdint.h>

/// @brief Allocator class for the GRAM.
/// @param BlockSize The block size of one memory block in bytes
/// @param Blocks The number of memory blocks available
template <uint32_t BlockSize = 32*32, uint32_t Blocks = 16>
class GRamAlloc
{
public: 
    static constexpr uint32_t GRAM_ALLOC_INVALID = 0xffffffff;
    GRamAlloc()
    {
        for (uint32_t i = 0; i < Blocks; i++)
        {
            m_memBlocks[i] = GRAM_ALLOC_INVALID;
        }
    }

    /// @brief Allocator function to allocate memory
    /// @param size Size in bytes to allocate. Should be multiple of
    /// BlockSize for efficient memory usage
    /// @return Address in the GRAM or GRAM_ALLOC_INVALID if allocation failed
    uint32_t alloc(uint32_t size)
    {
        // Calculate the required blocks for the given size
        uint32_t requiredBlocks = size / BlockSize;
        if ((requiredBlocks * BlockSize) < size) requiredBlocks++;

        // Search free block
        for (uint32_t i = 0; i < Blocks; i++)
        {
            if (m_memBlocks[i] == GRAM_ALLOC_INVALID)
            {
                // Check if enough blocks free to allocate the required blocks
                bool currentBlockIsGood = true;
                for (uint32_t j = 0; j < requiredBlocks; j++)
                {
                    if ((i+j) < Blocks)
                    {
                        currentBlockIsGood &= m_memBlocks[i+j] == GRAM_ALLOC_INVALID;
                    }
                    else
                    {
                        currentBlockIsGood = false;
                    }

                    // Check if the allocator found a used block or the end of the memory.
                    // If so, the allocator can break this loop and increment i to this position,
                    // because it's pointles to check this blocks again if there are free
                    if (currentBlockIsGood == false)
                    {
                        i += j;
                        break;
                    }
                }

                // Check if the following blocks are all free to be allocated for the requested memory
                if (currentBlockIsGood)
                {
                    // Take now the blocks
                    const uint32_t memStartAddress = i * BlockSize;
                    for (uint32_t j = 0; j < requiredBlocks; j++)
                    {
                        // All used blocks will contain the start address of the requested memory.
                        // When the memory is freed, it has then just to search through all blocks
                        // the address which has to be freed
                        m_memBlocks[i+j] = memStartAddress;
                    }
                    return memStartAddress;
                }
            }
        }
        return GRAM_ALLOC_INVALID;
    }

    /// @brief Will free a given address from the GRAM
    /// @param mem The memory section which has to be freed
    void free(uint32_t mem)
    {
        bool usedMemBlockFound = false;
        for (uint32_t i = 0; i < Blocks; i++)
        {
            if (m_memBlocks[i] == mem)
            {
                m_memBlocks[i] = GRAM_ALLOC_INVALID;
                usedMemBlockFound = true;
            }
            else
            {
                // A allocated memory is never fragmented. That means, if the allocator found the given
                // address, it will set this variable to true. If the further blocks don't contain
                // the given address anymore, then the whole memory is erased. It is pointless then
                // to iterate through the rest of the memory, because we will never find a block which
                // contains the given address.
                // To, if this is true, just quit!
                if (usedMemBlockFound)
                {
                    return;
                }
            }
        }
    }
private:
    uint32_t m_memBlocks[Blocks];
};

#endif // GRAMALLOC_HPP

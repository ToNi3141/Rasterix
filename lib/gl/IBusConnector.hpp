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

#ifndef IBUSCONNECTOR_HPP
#define IBUSCONNECTOR_HPP
#include <stdint.h>

namespace rr
{
class IBusConnector
{
public:
    virtual ~IBusConnector() = default;

    /// @brief Uploads a chunk of data
    /// @param data The data
    /// @param bytes how many bytes to transfer
    virtual void writeData(const uint8_t* data, const uint32_t bytes) = 0;

    /// @brief Signals if the FIFO of the target is empty and is able to receive the next chunk of data
    /// @return true if the FIFO is empty
    virtual bool clearToSend() = 0;

    /// @brief Will start a dma transfer from the internal buffer to an external memory
    /// @param index The index of the frame buffer line which has to be transferred
    virtual void startColorBufferTransfer(const uint8_t index) = 0;
};

} // namespace rr
#endif // IBUSCONNECTOR_HPP

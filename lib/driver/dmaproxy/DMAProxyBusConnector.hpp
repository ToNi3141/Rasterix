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

#ifndef DMAPROXYBUSCONNECTOR_HPP
#define DMAPROXYBUSCONNECTOR_HPP

#include "IBusConnector.hpp"
struct channel_buffer;
namespace rr
{
class DMAProxyBusConnector : public IBusConnector
{
public:
    virtual ~DMAProxyBusConnector() = default;

    DMAProxyBusConnector();

    virtual void writeData(const uint8_t index, const uint32_t size) override;
    virtual bool clearToSend() override;
    virtual tcb::span<uint8_t> requestBuffer(const uint8_t index) override;
    virtual uint8_t getBufferCount() const override;
private:
    struct Channel {
        struct channel_buffer *buf_ptr;
        int fd;
    };
    Channel m_txChannel;
    tcb::span<uint8_t> m_tmpBuffer{};
    bool m_transferOngoing { false };
};

} // namespace rr
#endif // #ifndef DMAPROXYBUSCONNECTOR_HPP
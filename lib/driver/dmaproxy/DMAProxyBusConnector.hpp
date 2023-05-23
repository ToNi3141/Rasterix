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

namespace rr
{
class DMAProxyBusConnector : public IBusConnector
{
public:
    virtual ~DMAProxyBusConnector() = default;

    DMAProxyBusConnector();

    virtual void writeData(const std::span<const uint8_t>& data) override;
    virtual bool clearToSend() override;
private:
    std::span<uint8_t> m_tmpBuffer{};
};

} // namespace rr
#endif // #ifndef DMAPROXYBUSCONNECTOR_HPP
#ifndef FT60XBUSCONNECTOR_H
#define FT60XBUSCONNECTOR_H

#include <chrono>
#include <thread>

#include "IBusConnector.hpp"
#include <ftd3xx.h>

#include <stdio.h>

namespace rr
{
// Bus connector to use an FT600 chip configured in the FT245 FIFO mode.
// Important: Before you use this class, configure the FT600 in the FT245 FIFO mode with FT60X Chip Configuration Programmer. Otherwise, it will not work.
// This class uses GPIO0 to reset the FPGA
class FT60XBusConnector : public IBusConnector
{
public:
    virtual ~FT60XBusConnector();

    FT60XBusConnector();

    virtual void writeData(const std::span<const uint8_t>& data) override;
    virtual bool clearToSend() override;
private:
    FT_HANDLE fthandle;
};

} // namespace rr
#endif // FT60XBUSCONNECTOR_H

#ifndef FT60XBUSCONNECTOR_H
#define FT60XBUSCONNECTOR_H

#include <chrono>
#include <thread>

#include "IBusConnector.hpp"
#include <ftd3xx.h>
#include <stdio.h>

// Bus connector to use an FT600 chip configured in the FT245 FIFO mode.
// Important: Before you use this class, configure the FT600 in the FT245 FIFO mode with FT60X Chip Configuration Programmer. Otherwise, it will not work.
// This class uses GPIO0 to reset the FPGA
class FT60XBusConnector : public IBusConnector
{
public:
    virtual ~FT60XBusConnector()
    {
        FT_Close(fthandle);
    }

    FT60XBusConnector()
    {
        FT_STATUS ftStatus = FT_OK;
        ftStatus = FT_Create(0, FT_OPEN_BY_INDEX, &fthandle);
        if (!fthandle) {
            printf("Failed to create device\r\n");
            return;
        }

        ftStatus = FT_EnableGPIO(fthandle, 0x3, 0x3); //bit 0 and 1 both set.
        //ftStatus = FT_SetGPIOPull(fthandle, 0x3, GPIO_PULL_50K_PU); // Windows does not have GPIO_PULL_50K_PU constant?
        ftStatus = FT_SetGPIOPull(fthandle, 0x3, 0x2);

        ftStatus = FT_WriteGPIO(fthandle, 0x3, 0x3);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        ftStatus = FT_WriteGPIO(fthandle, 0x3, 0x0);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        ftStatus = FT_WriteGPIO(fthandle, 0x3, 0x3);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    virtual void writeData(const uint8_t* data, const uint32_t bytes) override
    {
        ULONG transfered;
        FT_WritePipe(fthandle, 0x2, (PUCHAR)data, bytes, &transfered, NULL);
    }

    virtual bool clearToSend() override
    {
        return true;
    }

    virtual void startColorBufferTransfer(const uint8_t) override
    {
    }
private:
    FT_HANDLE fthandle;
};

#endif // FT60XBUSCONNECTOR_H

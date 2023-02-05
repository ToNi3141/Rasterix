#include "FT60XBusConnector.hpp"

namespace rr
{
FT60XBusConnector::~FT60XBusConnector()
{
    FT_Close(fthandle);
}

FT60XBusConnector::FT60XBusConnector()
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

void FT60XBusConnector::writeData(const tcb::span<const uint8_t>& data)
{
    ULONG transfered;
    FT_WritePipe(fthandle, 0x2, (PUCHAR)(data.data()), data.size(), &transfered, NULL);
}

bool FT60XBusConnector::clearToSend()
{
    return true;
}

} // namespace rr
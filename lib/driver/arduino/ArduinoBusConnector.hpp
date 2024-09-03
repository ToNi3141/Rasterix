#ifndef ARDUINOBUSCONNECTOR_H
#define ARDUINOBUSCONNECTOR_H

#include <Arduino.h>
#include <SPI.h>
#include "IBusConnector.hpp"
#include <tcb/span.hpp>

namespace rr
{
template <uint32_t DISPLAYLIST_SIZE = 32 * 1024>
class ArduinoBusConnector : public IBusConnector
{
public:
    static constexpr uint32_t RESET { 21 };
    static constexpr uint32_t CTS { 20 };
    static constexpr uint32_t MAX_CHUNK_SIZE { 32768 - 2048 };
    static constexpr uint32_t CSN { 17 };
    static constexpr uint32_t MOSI { 19 };
    static constexpr uint32_t MISO { 16 };
    static constexpr uint32_t SCK { 18 };

    virtual ~ArduinoBusConnector() = default;

    ArduinoBusConnector()
    {
        pinMode(RESET, OUTPUT);
        pinMode(CTS, INPUT);
        pinMode(CSN, OUTPUT);
        digitalWrite(CSN, HIGH);
        digitalWrite(RESET, HIGH);
        //Arduino_DataBus *bus = new Arduino_RPiPicoSPI(12 /* DC */, 13 /* CS */, 10 /* SCK */, 11 /* MOSI */, UINT8_MAX /* MISO */, spi1 /* spi */);
        SPI.begin();
        SPI.beginTransaction(SPISettings(50000000, MSBFIRST, SPI_MODE0));
        init();
    }

    virtual void writeData(const uint8_t index, const uint32_t size) override
    {
        tcb::span<uint8_t> data = requestBuffer(index);
        uint32_t dataToSend = size;
        uint32_t counter = 0;
        while (dataToSend != 0)
        {
            while (!clearToSend())
                ;
            // SPI has no flow control. Therefore the flow control must be implemented in software.
            // Divide the data into smaller chunks. Check after each chunk, if the fifo has enough space
            // before sending the next chunk.
            const uint32_t chunkSize = (dataToSend < MAX_CHUNK_SIZE) ? dataToSend : MAX_CHUNK_SIZE;
            SPI.transfer(data.data() + counter, chunkSize);

            // dma_channel_config c = dma_channel_get_default_config(dma_tx);
            // channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
            // channel_config_set_dreq(&c, spi_get_dreq(spi_default, true));
            // dma_channel_configure(dma_tx, &c,
            //                 &spi_get_hw(spi_default)->dr, // write address
            //                 data.data() + counter, // read address
            //                 chunkSize, // element count (each element is of size transfer_data_size)
            //                 true); // Start immediately
            counter += chunkSize;
            dataToSend -= chunkSize;
        }
    }

    virtual bool clearToSend() override
    {
        return digitalRead(CTS);
    }
    virtual tcb::span<uint8_t> requestBuffer(const uint8_t index) override 
    { 
        switch (index) 
        {
            case 2:
                return { m_displayListTmp }; 
            default:
                return { m_dlMem[index] }; 
        }
        return {}; 
    }

    virtual uint8_t getBufferCount() const override 
    { 
        return m_dlMem.size() + 1; 
    }

    void init()
    {
        // spi_init(spi_default, 50 * 1000 * 1000);
        // gpio_set_function(PICO_DEFAULT_SPI_RX_PIN, GPIO_FUNC_SPI);
        // gpio_init(PICO_DEFAULT_SPI_CSN_PIN);
        // gpio_set_dir(PICO_DEFAULT_SPI_CSN_PIN, true);
        // gpio_set_function(PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI);
        // gpio_set_function(PICO_DEFAULT_SPI_TX_PIN, GPIO_FUNC_SPI);

        // // Make the SPI pins available to picotool
        // bi_decl(bi_3pins_with_func(PICO_DEFAULT_SPI_RX_PIN, PICO_DEFAULT_SPI_TX_PIN, PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI));
        // // Make the CS pin available to picotool
        // bi_decl(bi_1pin_with_name(PICO_DEFAULT_SPI_CSN_PIN, "SPI CS"));

        // // Grab some unused dma channels
        // dma_tx = dma_claim_unused_channel(true);

        // gpio_put(PICO_DEFAULT_SPI_CSN_PIN, 1);
        digitalWrite(RESET, LOW);
        delay(50);
        digitalWrite(RESET, HIGH);
        delay(500); // Wait a moment till the FPGA has load its configuration
        digitalWrite(RESET, LOW);
        digitalWrite(CSN, LOW);
        delay(50);
    }
private:
    std::array<std::array<uint8_t, DISPLAYLIST_SIZE>, 2> m_dlMem;
    std::array<uint8_t, 4096 + 64> m_displayListTmp;
    // SPIClass s(MISO, MOSI, SCK);
};

} // namespace rr
#endif // ARDUINOBUSCONNECTOR_H

#include "IceGL.hpp"
#include "RendererMemoryOptimized.hpp"
#include "RenderConfigs.hpp"
#include "gl.h"
#include "glu.h"

#include <pico/stdlib.h>
#include <pico/binary_info.h>
#include <hardware/spi.h>
#include <hardware/dma.h>

#include <algorithm>

// Simple BusConnector which wraps the SPI
class BusConnector : public rr::IBusConnector
{
public:
    static constexpr uint32_t RESET { 21 };
    static constexpr uint32_t CTS { 20 };
    static constexpr uint32_t MAX_CHUNK_SIZE { 16384 };
    
    BusConnector() { }

    virtual ~BusConnector() = default;

    virtual void writeData(const std::span<const uint8_t>& data) override 
    {
        uint32_t dataToSend = data.size();
        uint32_t counter = 0;
        while (dataToSend != 0)
        {
            while (!clearToSend())
                ;
            // SPI has no flow control. Therefore the flow control must be implemented in software.
            // Divide the data into smaller chunks. Check after each chunk, if the fifo has enough space
            // before sending the next chunk.
            const uint32_t chunkSize = (dataToSend < MAX_CHUNK_SIZE) ? dataToSend : MAX_CHUNK_SIZE;
            dma_channel_config c = dma_channel_get_default_config(dma_tx);
            channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
            channel_config_set_dreq(&c, spi_get_dreq(spi_default, true));
            dma_channel_configure(dma_tx, &c,
                            &spi_get_hw(spi_default)->dr, // write address
                            data.data() + counter, // read address
                            chunkSize, // element count (each element is of size transfer_data_size)
                            true); // Start immediately
            counter += chunkSize;
            dataToSend -= chunkSize;
        }
    }

    virtual bool clearToSend() override 
    {
        return !dma_channel_is_busy(dma_tx) && gpio_get(CTS);
    }

    void init()
    {
        gpio_init(RESET);
        gpio_set_dir(RESET, true);
        gpio_set_dir(CTS, false);
        spi_init(spi_default, 50 * 1000 * 1000);
        gpio_set_function(PICO_DEFAULT_SPI_RX_PIN, GPIO_FUNC_SPI);
        gpio_init(PICO_DEFAULT_SPI_CSN_PIN);
        gpio_set_dir(PICO_DEFAULT_SPI_CSN_PIN, true);
        gpio_set_function(PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI);
        gpio_set_function(PICO_DEFAULT_SPI_TX_PIN, GPIO_FUNC_SPI);

        // Make the SPI pins available to picotool
        bi_decl(bi_3pins_with_func(PICO_DEFAULT_SPI_RX_PIN, PICO_DEFAULT_SPI_TX_PIN, PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI));
        // Make the CS pin available to picotool
        bi_decl(bi_1pin_with_name(PICO_DEFAULT_SPI_CSN_PIN, "SPI CS"));

        // Grab some unused dma channels
        dma_tx = dma_claim_unused_channel(true);

        gpio_put(PICO_DEFAULT_SPI_CSN_PIN, 1);
        gpio_put(RESET, 0);
        sleep_ms(50);
        gpio_put(RESET, 1);
        sleep_ms(500); // Wait a moment till the FPGA has load its configuration
        gpio_put(RESET, 0);
        gpio_put(PICO_DEFAULT_SPI_CSN_PIN, 0);
        sleep_ms(50);
    }

    uint dma_tx;
};

template <typename Scene>
class Runner
{
public:
    Runner() 
    {
        stdio_init_all();
        gpio_init(LED_PIN);
        gpio_set_dir(LED_PIN, GPIO_OUT);
        m_busConnector.init();
        rr::IceGL::createInstance(m_renderer);
        m_renderer.setRenderResolution(RESOLUTION_W, RESOLUTION_H);
        rr::IceGL::getInstance().enableColorBufferStream();
    }

    void execute()
    {
        m_scene.init(RESOLUTION_W, RESOLUTION_H);
        while (1)
        {
            gpio_put(LED_PIN, led);
            led = !led;
            m_scene.draw();
            rr::IceGL::getInstance().render();
        }
    }
private:
    static constexpr uint32_t RESOLUTION_H = 240;
    static constexpr uint32_t RESOLUTION_W = 320;
    static constexpr uint LED_PIN = 25;
    BusConnector m_busConnector;
    rr::RendererMemoryOptimized<rr::RenderConfigPico> m_renderer{m_busConnector};
    bool led = false;
    Scene m_scene {};
};

#include <Arduino.h>
#include <SPI.h>
#include <IBusConnector.hpp>
#include <RRXGL.hpp>
#include <Minimal.hpp>

template <uint32_t DISPLAYLIST_SIZE = 32 * 1024>
class ArduinoBusConnector : public rr::IBusConnector
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
        // Initialize hardware
        pinMode(RESET, OUTPUT);
        pinMode(CTS, INPUT);
        pinMode(CSN, OUTPUT);
        digitalWrite(CSN, HIGH);
        digitalWrite(RESET, HIGH);
        SPI.begin();
        SPI.beginTransaction(SPISettings(50000000, MSBFIRST, SPI_MODE0));
    
        // Reset FPGA
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
    std::array<uint8_t, 4096 + 64> m_displayListTmp; // The last buffer is used to upload texture chuncks and can be smaller
};

static ArduinoBusConnector<> m_busConnector {};
static constexpr uint32_t RESOLUTION_H = 240;
static constexpr uint32_t RESOLUTION_W = 320;
static constexpr uint LED_PIN = 25;
bool state { false };

static Minimal m_scene {};

void setup() 
{
    pinMode(LED_PIN, OUTPUT);
    m_busConnector.init();
    rr::RRXGL::createInstance(m_busConnector);
    rr::RRXGL::getInstance().setRenderResolution(RESOLUTION_W, RESOLUTION_H);
    m_scene.init(RESOLUTION_W, RESOLUTION_H);
}

void loop() 
{
    m_scene.draw();
    rr::RRXGL::getInstance().swapDisplayList();
    rr::RRXGL::getInstance().uploadDisplayList();
    digitalWrite(LED_PIN, state);
    state = !state;
}
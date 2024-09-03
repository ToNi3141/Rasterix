#include "RRXGL.hpp"
#include "ArduinoBusConnector.hpp"
#include <algorithm>
#include <Arduino.h>

template <typename Scene>
class Runner
{
public:
    static constexpr uint LED_PIN = 25;
    Runner() 
    {
        rr::RRXGL::createInstance(m_busConnector);
        rr::RRXGL::getInstance().setRenderResolution(RESOLUTION_W, RESOLUTION_H);
        m_scene.init(RESOLUTION_W, RESOLUTION_H);
        pinMode(LED_PIN, OUTPUT);
    }

    void execute()
    {
        m_scene.draw();
        rr::RRXGL::getInstance().swapDisplayList();
        rr::RRXGL::getInstance().uploadDisplayList();
        digitalWrite(LED_PIN, state);
        state = !state;
    }
private:
    static constexpr uint32_t RESOLUTION_H = 240;
    static constexpr uint32_t RESOLUTION_W = 320;
    rr::ArduinoBusConnector<32 * 1024> m_busConnector;
    Scene m_scene {};
    bool state { false };
};

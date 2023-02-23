/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pico/stdlib.h"
#include "IceGL.hpp"
#include "RendererArduino.hpp"
#include "RenderConfigs.hpp"
#include "gl.h"
#include "glu.h"

// Simple BusConnector which wraps the SPI
class BusConnector : public rr::IBusConnector
{
public:
    BusConnector() { }

    virtual ~BusConnector() = default;

    virtual void writeData(const std::span<const uint8_t>& data) override 
    {
    }

    virtual bool clearToSend() override 
    {
        return true;
    }
};

static const uint32_t RESOLUTION_H = 600;
static const uint32_t RESOLUTION_W = 1024;
BusConnector m_busConnector;
rr::RendererArduino<rr::RenderConfigPico> m_renderer{m_busConnector};

int main() {
    rr::IceGL::createInstance(m_renderer);
    m_renderer.setRenderResolution(RESOLUTION_W, RESOLUTION_H);

    const uint LED_PIN = 25;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    while (true) {
        gpio_put(LED_PIN, 1);
        sleep_ms(250);
        gpio_put(LED_PIN, 0);
        sleep_ms(1000);
    }
}

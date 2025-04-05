#include "FT60XBusConnector.hpp"
#include "MultiThreadRunner.hpp"
#include "RRXGL.hpp"
#include "gl.h"
#include "glu.h"
#include <spdlog/spdlog.h>
#include <stdio.h>

template <typename Scene>
class Runner
{
public:
    Runner()
    {
        spdlog::set_level(spdlog::level::trace);
        rr::RRXGL::createInstance(m_busConnector, m_runner);
        rr::RRXGL::getInstance().setRenderResolution(RESOLUTION_W, RESOLUTION_H);
    }

    void execute()
    {
        m_scene.init(RESOLUTION_W, RESOLUTION_H);
        while (1)
        {
            m_scene.draw();
            rr::RRXGL::getInstance().swapDisplayList();
            rr::RRXGL::getInstance().uploadDisplayList();
        }
    }

private:
    static constexpr uint32_t RESOLUTION_H = 600;
    static constexpr uint32_t RESOLUTION_W = 1024;
    rr::FT60XBusConnector m_busConnector {};
    rr::MultiThreadRunner m_runner {};
    Scene m_scene {};
};

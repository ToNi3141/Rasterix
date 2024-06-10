#include <stdio.h>
#include <spdlog/spdlog.h>
#include "FT60XBusConnector.hpp"
#include "RRXGL.hpp"
#include "gl.h"
#include "glu.h"
#include "ThreadedRenderer.hpp"

template <typename Scene>
class Runner
{
public:
    Runner() 
    {
        spdlog::set_level(spdlog::level::trace);
        rr::RRXGL::createInstance(m_busConnector);
        rr::RRXGL::getInstance().setRenderResolution(RESOLUTION_W, RESOLUTION_H);
        m_threadedRenderer.setRenderer(&(rr::RRXGL::getInstance()));
    }

    void execute()
    {
        m_scene.init(RESOLUTION_W, RESOLUTION_H);
        while (1)
        {
            m_scene.draw();
            m_threadedRenderer.waitForThread();
            m_threadedRenderer.render();
        }
    }
private:
    static constexpr uint32_t RESOLUTION_H = 600;
    static constexpr uint32_t RESOLUTION_W = 1024;
    rr::FT60XBusConnector m_busConnector;
    rr::ThreadedRenderer<rr::RRXGL> m_threadedRenderer {};
    Scene m_scene {};
};

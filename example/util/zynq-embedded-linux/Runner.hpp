#include <stdio.h>
#include <spdlog/spdlog.h>
#include "IceGL.hpp"
#include "gl.h"
#include "glu.h"
#include "DMAProxyBusConnector.hpp"
#include "ThreadedRenderer.hpp"


template <typename Scene>
class Runner
{
public:
    Runner() 
    {
        spdlog::set_level(spdlog::level::trace);
        rr::IceGL::createInstance(m_busConnector);
        rr::IceGL::getInstance().setRenderResolution(RESOLUTION_W, RESOLUTION_H);
        m_threadedRenderer.setRenderer(&rr::IceGL::getInstance());
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
    rr::DMAProxyBusConnector m_busConnector;
    rr::ThreadedRenderer<rr::IceGL> m_threadedRenderer {};
    Scene m_scene {};
};

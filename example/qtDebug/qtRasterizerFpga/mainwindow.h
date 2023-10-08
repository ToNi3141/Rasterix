#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QImage>
#include <QTimer>
#include "IceGL.hpp"
#include "Renderer.hpp"
#include "RendererMemoryOptimized.hpp"
#include "gl.h"
#include "RenderConfigs.hpp"
#if USE_SIMULATION
#include "VerilatorBusConnector.hpp"
#endif
#if USE_HARDWARE
#undef VOID // Undef void because it is defined in the tcl.h and there is a typedef in WinTypes.h (which is used for the FT2232 library)
#include "FT60XBusConnector.hpp"
#endif

#include "../../stencilShadow/StencilShadow.hpp"
#include "../../minimal/Minimal.hpp"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void newFrame();

private:

#if USE_SIMULATION
public:
    static const uint32_t PREVIEW_WINDOW_SCALING = 1;
    static const uint32_t RESOLUTION_W = 640;
    static const uint32_t RESOLUTION_H = 480;
private:
    uint16_t m_framebuffer[RESOLUTION_W * RESOLUTION_H];

    rr::VerilatorBusConnector<uint64_t> m_busConnector{reinterpret_cast<uint64_t*>(m_framebuffer), RESOLUTION_W, RESOLUTION_H};
    rr::Renderer<rr::RenderConfigSimulationMemory> m_renderer{m_busConnector};
#endif

#if USE_HARDWARE
public:
    static const uint32_t RESOLUTION_H = 600;
    static const uint32_t RESOLUTION_W = 1024;
private:
    rr::FT60XBusConnector m_busConnector;
    rr::Renderer<rr::RenderConfigRasterixNexys> m_renderer{m_busConnector};
#endif

    Ui::MainWindow *ui;

    QTimer m_timer;
    QImage m_image;
    Minimal m_testScene;
};

#endif // MAINWINDOW_H

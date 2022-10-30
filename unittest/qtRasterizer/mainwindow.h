#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QImage>
#include <QTimer>
#include "IceGL.hpp"
#include "Renderer.hpp"
#include "gl.h"
#if USE_SIMULATION
#include "VerilatorBusConnector.hpp"
#endif
#if USE_HARDWARE
#undef VOID // Undef void because it is defined in the tcl.h and there is a typedef in WinTypes.h (which is used for the FT2232 library)
#include "FT60XBusConnector.hpp"
#endif

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
    GLuint loadTexture(const char *tex);

#if USE_SIMULATION
public:
    static const uint32_t PREVIEW_WINDOW_SCALING = 1;
    static const uint32_t RESOLUTION_W = 640;
    static const uint32_t RESOLUTION_H = 480;
private:
    uint16_t m_framebuffer[RESOLUTION_W * RESOLUTION_H];

    VerilatorBusConnector<uint64_t> m_busConnector{reinterpret_cast<uint64_t*>(m_framebuffer), RESOLUTION_W, RESOLUTION_H};
    Renderer<1024*1024, 10, RESOLUTION_H / 10, 64> m_renderer{m_busConnector};
#endif

#if USE_HARDWARE
public:
    static const uint32_t RESOLUTION_H = 600;
    static const uint32_t RESOLUTION_W = 1024;
private:
    FT60XBusConnector m_busConnector;
    Renderer<1024*1024, 5, RESOLUTION_H / 5, 128, 256> m_renderer{m_busConnector};
#endif

    GLuint m_textureId = 0;
    GLuint m_textureId2 = 0;
    int32_t m_fbSel = 0;
    float m_clipPlane = 1;

    Ui::MainWindow *ui;

    QTimer m_timer;
    QImage m_image;
};

#endif // MAINWINDOW_H

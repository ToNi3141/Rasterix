#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QImage>
#include <QTimer>
#include "IceGL.hpp"
#include "Renderer.hpp"
#include "VerilatorBusConnector.hpp"
#ifdef SOFTWARE_RENDERER
#include "softwarerenderer.h"
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
    static const uint32_t RESOLUTION_W = 640;
    static const uint32_t RESOLUTION_H = 480;
    static const uint32_t PREVIEW_WINDOW_SCALING = 1;
    GLuint loadTexture(const char *tex);

#ifdef SOFTWARE_RENDERER
    uint16_t m_zbuffer[RESOLUTION_W*RESOLUTION_H];
    SoftwareRenderer m_renderer{m_framebuffer, m_zbuffer, RESOLUTION_W, RESOLUTION_H};
#else
    VerilatorBusConnector<uint64_t> m_busConnector{reinterpret_cast<uint64_t*>(m_framebuffer), RESOLUTION_W, RESOLUTION_H};
    Renderer<16384, 4, RESOLUTION_H / 4, 32> m_renderer{m_busConnector};
#endif
    IceGL m_ogl{m_renderer};

    GLuint m_textureId = 0;
    int32_t m_fbSel = 0;
    float m_clipPlane = 1;

    Ui::MainWindow *ui;

    QTimer m_timer;
    QImage m_image;

    uint16_t m_framebuffer[RESOLUTION_W*RESOLUTION_H];
};

#endif // MAINWINDOW_H

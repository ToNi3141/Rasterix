#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QImage>
#include <QPixmap>
#include <math.h>
#include <QDebug>
#include "QTime"
#include <QThread>
#include <QFile>
#include "IceGLWrapper.h"

static constexpr uint16_t cubeIndex[] = {
    0, 1, 2, 0, 2, 3, // Face three
    4, 5, 6, 4, 6, 7, // Face five
    8, 9, 10, 8, 10, 11, // Face two
    12, 13, 14, 12, 14, 15, // Face six
    16, 17, 18, 16, 18, 19, // Face one
    20, 21, 22, 20, 22, 23 // Face four
};

static constexpr float cubeVerts[] = {
    -1.0f, 1.0f, 1.0f,
    -1.0f,-1.0f, 1.0f,
    1.0f,-1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,

    -1.0f, 1.0f,-1.0f,
    -1.0f,-1.0f,-1.0f,
    -1.0f,-1.0f, 1.0f,
    -1.0f, 1.0f, 1.0f,

    1.0f, 1.0f, 1.0f,
    1.0f,-1.0f, 1.0f,
    1.0f,-1.0f,-1.0f,
    1.0f, 1.0f,-1.0f,

    -1.0f, 1.0f,-1.0f,
    -1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f,-1.0f,

    -1.0f,-1.0f, 1.0f,
    -1.0f,-1.0f,-1.0f,
    1.0f,-1.0f,-1.0f,
    1.0f,-1.0f, 1.0f,

    1.0f, 1.0f,-1.0f,
    1.0f,-1.0f,-1.0f,
    -1.0f,-1.0f,-1.0f,
    -1.0f, 1.0f,-1.0f,
};

static constexpr float cubeTexCoords[] = {
    0.0f, 1.0f,
    0.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,

    1.0f, 0.0f,
    0.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f,

    1.0f, 1.0f,
    0.0f, 1.0f,
    0.0f, 0.0f,
    1.0f, 0.0f,

    0.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,

    0.0f, 1.0f,
    0.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,

    1.0f, 1.0f,
    1.0f, 0.0f,
    0.0f, 0.0f,
    0.0f, 1.0f,
};

static constexpr float cubeNormals[] = {
    0.0f,  0.0f,  1.0f,
    0.0f,  0.0f,  1.0f,
    0.0f,  0.0f,  1.0f,
    0.0f,  0.0f,  1.0f,

    -1.0f,  0.0f,  0.0f,
    -1.0f,  0.0f,  0.0f,
    -1.0f,  0.0f,  0.0f,
    -1.0f,  0.0f,  0.0f,

    1.0f,  0.0f,  0.0f,
    1.0f,  0.0f,  0.0f,
    1.0f,  0.0f,  0.0f,
    1.0f,  0.0f,  0.0f,

    0.0f,  1.0f,  0.0f,
    0.0f,  1.0f,  0.0f,
    0.0f,  1.0f,  0.0f,
    0.0f,  1.0f,  0.0f,

    0.0f, -1.0f,  0.0f,
    0.0f, -1.0f,  0.0f,
    0.0f, -1.0f,  0.0f,
    0.0f, -1.0f,  0.0f,

    0.0f,  0.0f, -1.0f,
    0.0f,  0.0f, -1.0f,
    0.0f,  0.0f, -1.0f,
    0.0f,  0.0f, -1.0f,
};

// Image source: https://opengameart.org/content/basic-map-32x32-by-ivan-voirol
// Picture exported from GIMP
#define HEADER_PIXEL(data,pixel) {\
pixel[0] = (((data[0] - 33) << 2) | ((data[1] - 33) >> 4)); \
pixel[1] = ((((data[1] - 33) & 0xF) << 4) | ((data[2] - 33) >> 2)); \
pixel[2] = ((((data[2] - 33) & 0x3) << 6) | ((data[3] - 33))); \
data += 4; \
}
static char *cubeTexture =
	"<6!DX<_4X<_4X<_4X<_4X<_4X<_4X<_4X<_4X<_4X<_4X<_4X<_4X<_4X<_4X<_4"
	"X<_4X<_4X<_4X<_4X<_4X<_4X<_4X<_4X<_4X<_4X<_4X<_4X<_4X<_4X<_4<6!D"
	"RJ.'RJ.'RJ.'RJ.'RJ.'RJ.'RJ.'RJ.'RJ.'RJ.'RJ.'RJ.'T[6FT[6FT[6FT[6F"
	"T[6FT[6FRJ.'RJ.'RJ.'T[6FT[6FT[6FT[6FT[6FT[6FRJ.'RJ.'RJ.'RJ.'X<_4"
	"RJ.'RJ.'RJ.'T[6FT[6FT[6FT[6FT[6FT[6FT[6FRJ.'RJ.'RJ.'T[6FT[6FT[6F"
	"T[6FT[6FT[6FT[6FT[6FT[6FT[6FT[6FT[6FT[6FT[6FT[6FT[6FRJ.'RJ.'X<_4"
	"H8)RRJ.'T[6FT[6FT[6FRJ.'T[6FT[6FT[6FT[6FT[6FRJ.'H8)RH8)RRJ.'RJ.'"
	"RJ.'T[6FT[6FT[6FT[6FT[6FT[6FT[6FT[6FT[6FT[6FT[6FT[6FT[6FT[6FX<_4"
	"H8)RH8)RH8)RRJ.'RJ.'RJ.'RJ.'RJ.'RJ.'RJ.'H8)RH8)RRJ.'T[6FT[6FT[6F"
	"T[6FT[6FT[6FT[6FT[6FRJ.'RJ.'RJ.'RJ.'RJ.'RJ.'T[6FT[6FT[6FT[6FX<_4"
	"H8)RH8)RH8)RH8)RRJ.'RJ.'RJ.'RJ.'T[6FRJ.'T[6FT[6FT[6FT[6FT[6FT[6F"
	"T[6FT[6FT[6FT[6FT[6FT[6FT[6FT[6FT[6FT[6FT[6FRJ.'RJ.'RJ.'RJ.'X<_4"
	"X<_4X<_4X<_4X<_4X<_4X<_4X<_4X<_4X<_4X<_4X<_4X<_4X<_4X<_4X<_4X<_4"
	"X<_4X<_4X<_4X<_4X<_4X<_4X<_4X<_4X<_4X<_4X<_4X<_4X<_4X<_4X<_4X<_4"
	"RJ.'T[6FT[6FT[6FT[6FX<_4RJ.'RJ.'RJ.'RJ.'RJ.'RJ.'RJ.'RJ.'RJ.'T[6F"
	"T[6FT[6FT[6FT[6FT[6FT[6FT[6FT[6FT[6FX<_4T[6FT[6FT[6FT[6FT[6FX<_4"
	"RJ.'T[6FT[6FT[6FT[6FX<_4RJ.'RJ.'RJ.'RJ.'T[6FT[6FT[6FT[6FT[6FT[6F"
	"T[6FT[6FRJ.'T[6FT[6FT[6FT[6FT[6FT[6FX<_4T[6FT[6FT[6FT[6FT[6FX<_4"
	"RJ.'T[6FT[6FT[6FT[6FX<_4T[6FT[6FT[6FT[6FT[6FT[6FT[6FT[6FT[6FT[6F"
	"T[6FT[6FT[6FT[6FT[6FT[6FT[6FT[6FT[6FX<_4T[6FRJ.'T[6FT[6FT[6FX<_4"
	"RJ.'T[6FT[6FT[6FT[6FX<_4T[6FT[6FT[6FT[6FT[6FT[6FT[6FT[6FRJ.'RJ.'"
	"RJ.'T[6FT[6FT[6FT[6FT[6FT[6FT[6FT[6FX<_4T[6FRJ.'T[6FT[6FT[6FX<_4"
	"T[6FT[6FT[6FT[6FT[6FX<_4RJ.'RJ.'RJ.'RJ.'RJ.'T[6FT[6FT[6FT[6FT[6F"
	"T[6FT[6FT[6FT[6FT[6FRJ.'RJ.'RJ.'RJ.'X<_4T[6FT[6FT[6FT[6FT[6FX<_4"
	"T[6FT[6FRJ.'T[6FT[6FX<_4RJ.'T[6FT[6FT[6FT[6FT[6FT[6FT[6FT[6FT[6F"
	"T[6FT[6FT[6FT[6FT[6FT[6FT[6FT[6FT[6FX<_4T[6FT[6FT[6FT[6FRJ.'X<_4"
	"T[6FT[6FRJ.'T[6FT[6FX<_4X<_4X<_4X<_4X<_4X<_4X<_4X<_4X<_4X<_4X<_4"
	"X<_4X<_4X<_4X<_4X<_4X<_4X<_4X<_4X<_4X<_4T[6FT[6FT[6FT[6FH8)RX<_4"
	"T[6FT[6FRJ.'T[6FT[6FX<_4RJ.'RJ.'RJ.'RJ.'RJ.'RJ.'RJ.'RJ.'RJ.'T[6F"
	"T[6FT[6FT[6FT[6FT[6FT[6FT[6FT[6FT[6FX<_4T[6FT[6FT[6FT[6FRJ.'X<_4"
	"T[6FT[6FH8)RT[6FT[6FX<_4RJ.'RJ.'RJ.'RJ.'T[6FT[6FT[6FT[6FT[6FT[6F"
	"T[6FT[6FRJ.'T[6FT[6FT[6FT[6FT[6FT[6FX<_4RJ.'T[6FRJ.'T[6FRJ.'X<_4"
	"T[6FRJ.'H8)RRJ.'T[6FX<_4T[6FRJ.'T[6FT[6FT[6FT[6FT[6FT[6FT[6FT[6F"
	"T[6FT[6FT[6FT[6FT[6FT[6FT[6FT[6FT[6FX<_4RJ.'T[6FT[6FT[6FT[6FX<_4"
	"RJ.'RJ.'RJ.'H8)RT[6FX<_4T[6FT[6FT[6FT[6FT[6FT[6FT[6FT[6FT[6FT[6F"
	"T[6FT[6FT[6FT[6FT[6FRJ.'RJ.'RJ.'RJ.'X<_4H8)RT[6FT[6FT[6FT[6FX<_4"
	"RJ.'RJ.'T[6FH8)RT[6FX<_4RJ.'RJ.'RJ.'RJ.'RJ.'T[6FT[6FT[6FT[6FT[6F"
	"T[6FT[6FT[6FT[6FT[6FT[6FT[6FT[6FT[6FX<_4RJ.'T[6FT[6FT[6FT[6FX<_4"
	"RJ.'T[6FT[6FRJ.'RJ.'X<_4X<_4X<_4X<_4X<_4X<_4X<_4X<_4X<_4X<_4X<_4"
	"X<_4X<_4X<_4X<_4X<_4X<_4X<_4X<_4X<_4X<_4RJ.'T[6FT[6FT[6FT[6FX<_4"
	"RJ.'T[6FT[6FRJ.'T[6FX<_4RJ.'RJ.'RJ.'RJ.'RJ.'H8)RRJ.'RJ.'T[6FT[6F"
	"T[6FT[6FT[6FT[6FT[6FT[6FT[6FT[6FT[6FX<_4RJ.'RJ.'T[6FT[6FT[6FX<_4"
	"RJ.'T[6FT[6FRJ.'RJ.'X<_4H8)RRJ.'RJ.'T[6FT[6FT[6FT[6FT[6FT[6FT[6F"
	"T[6FT[6FRJ.'T[6FT[6FT[6FT[6FT[6FT[6FX<_4RJ.'RJ.'T[6FT[6FT[6FX<_4"
	"RJ.'T[6FT[6FRJ.'RJ.'X<_4T[6FT[6FT[6FT[6FT[6FT[6FT[6FRJ.'T[6FT[6F"
	"T[6FT[6FT[6FT[6FT[6FT[6FT[6FT[6FT[6FX<_4RJ.'H8)RT[6FT[6FT[6FX<_4"
	"RJ.'T[6FRJ.'RJ.'RJ.'X<_4T[6FT[6FT[6FT[6FT[6FT[6FT[6FT[6FT[6FT[6F"
	"T[6FT[6FT[6FT[6FT[6FT[6FT[6FT[6FT[6FX<_4H8)RRJ.'T[6FT[6FT[6FX<_4"
	"RJ.'T[6FT[6FRJ.'RJ.'X<_4T[6FT[6FT[6FT[6FT[6FT[6FT[6FT[6FT[6FT[6F"
	"T[6FT[6FT[6FT[6FT[6FRJ.'RJ.'RJ.'RJ.'X<_4H8)RH8)RT[6FT[6FT[6FX<_4"
	"RJ.'T[6FT[6FRJ.'H8)RX<_4T[6FT[6FT[6FT[6FT[6FT[6FT[6FRJ.'RJ.'H8)R"
	"RJ.'T[6FT[6FT[6FT[6FT[6FT[6FT[6FT[6FX<_4H8)RH8)RT[6FT[6FT[6FX<_4"
	"X<_4X<_4X<_4X<_4X<_4X<_4X<_4X<_4X<_4X<_4X<_4X<_4X<_4X<_4X<_4X<_4"
	"X<_4X<_4X<_4X<_4X<_4X<_4X<_4X<_4X<_4X<_4X<_4X<_4X<_4X<_4X<_4X<_4"
	"H8)RH8)RH8)RH8)RH8)RH8)RRJ.'RJ.'RJ.'RJ.'RJ.'H8)RRJ.'RJ.'T[6FT[6F"
	"T[6FT[6FT[6FT[6FT[6FT[6FT[6FT[6FT[6FT[6FT[6FRJ.'RJ.'RJ.'RJ.'X<_4"
	"H8)RH8)RH8)RH8)RH8)RRJ.'H8)RRJ.'RJ.'T[6FT[6FT[6FT[6FT[6FT[6FT[6F"
	"T[6FT[6FRJ.'T[6FT[6FT[6FT[6FT[6FT[6FT[6FT[6FT[6FT[6FT[6FT[6FX<_4"
	"RJ.'RJ.'T[6FT[6FT[6FT[6FT[6FT[6FT[6FT[6FT[6FT[6FT[6FRJ.'T[6FT[6F"
	"T[6FT[6FT[6FT[6FT[6FT[6FT[6FT[6FT[6FT[6FT[6FT[6FT[6FRJ.'RJ.'X<_4"
	"H8)RH8)RT[6FT[6FT[6FT[6FT[6FT[6FT[6FT[6FT[6FT[6FT[6FT[6FT[6FT[6F"
	"T[6FT[6FT[6FT[6FT[6FRJ.'RJ.'RJ.'RJ.'RJ.'RJ.'T[6FT[6FT[6FT[6FX<_4"
	"<6!DT[6FT[6FT[6FT[6FT[6FT[6FT[6FT[6FT[6FT[6FT[6FT[6FRJ.'RJ.'H8)R"
	"RJ.'T[6FT[6FT[6FT[6FT[6FT[6FT[6FT[6FT[6FT[6FRJ.'RJ.'RJ.'RJ.'<6!D"
	"";

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_image(RESOLUTION_W, RESOLUTION_H, QImage::Format_RGB888)
{
    ui->setupUi(this);
    initIceGlCWrapper(&m_ogl);

    connect(&m_timer, &QTimer::timeout, this, &MainWindow::newFrame);
    ui->label->setPixmap(QPixmap::fromImage(m_image));
    m_timer.setInterval(50);
    m_timer.setSingleShot(false);
    m_timer.start();

    m_textureId = loadTexture(cubeTexture);

    // Setup viewport, depth range, and projection matrix
    glViewport(0, 0, RESOLUTION_W, RESOLUTION_H);
    glDepthRange(0.0, 1.0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(30.0, (float)RESOLUTION_W/(float)RESOLUTION_H, 1.0, 111.0);

    // Enable depth test and depth mask
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    // Setup one light which will lighten the cube
    GLfloat light_ambient[] = { 0.5, 0.5, 0.5, 0.0 };
    GLfloat light_diffuse[] = { 0.5, 0.5, 0.5, 1.0 };
    GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat light_position[] = { 0.0, 3.0, 2.0, 0.0 };

    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 8.0f);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    static constexpr bool ENABLE_LIGHT = true;
    if constexpr (ENABLE_LIGHT)
    {
        glEnable(GL_LIGHT0);
        glEnable(GL_LIGHTING);

        static constexpr float colors[4] = {0.0, 0.0, 0.0, 0.0};
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, colors);
    }
}

// returns a valid textureID on success, otherwise 0
GLuint MainWindow::loadTexture(const char* tex)
{
    static constexpr uint32_t WIDTH = 32;
    static constexpr uint32_t HEIGHT = 32;
    int level = 0;
    int border = 0;

    // data is aligned in byte order
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    //request textureID
    GLuint textureID;
    glGenTextures(1, &textureID);

    // bind texture
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Convert from the GIMP format to RGB888
    char pixelBuffer[WIDTH * HEIGHT * 3];
    char *pixelBufferPtr = pixelBuffer;
    for (uint32_t i = 0; i < WIDTH * HEIGHT; i++)
    {
        HEADER_PIXEL(tex, pixelBufferPtr);
        pixelBufferPtr += 3;
    }

    // specify the 2D texture map
    glTexImage2D(GL_TEXTURE_2D, level, GL_RGBA, WIDTH, HEIGHT, border, GL_RGB, GL_UNSIGNED_BYTE, pixelBuffer);

    // return unique texture identifier
    return textureID;
}

void MainWindow::newFrame()
{
    // Setup clear color and clear the framebuffer
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Setup the model view matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // set camera
    gluLookAt(12.0f, -2.0f, 5.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);

    // Rotate cube
    static float t2 = 0;
    t2 += 1.0f;
    glRotatef(t2, 0.0f, 0.0f, 1.0f);

    // Scale the cube a bit to make it a bit bigger
    glScalef(1.5f, 1.5f, 1.5f);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, m_textureId);

    // Draw the cube
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, cubeVerts);
    glTexCoordPointer(2, GL_FLOAT, 0, cubeTexCoords);
    glNormalPointer(GL_FLOAT, 0, cubeNormals);
    glDrawElements(GL_TRIANGLES, sizeof(cubeIndex) / sizeof(cubeIndex[0]), GL_UNSIGNED_SHORT, cubeIndex);

    m_ogl.commit();

    for (uint32_t i = 0; i < RESOLUTION_H; i++)
    {
        for (uint32_t j = 0; j < RESOLUTION_W; j++)
        {
            uint8_t r = ((m_framebuffer[(i*RESOLUTION_W)+j] >> 12) & 0xf) << 4;
            uint8_t g = ((m_framebuffer[(i*RESOLUTION_W)+j] >> 8) & 0xf) << 4;
            uint8_t b = ((m_framebuffer[(i*RESOLUTION_W)+j] >> 4) & 0xf) << 4;
            m_image.setPixelColor(QPoint(j, i), QColor(r, g, b));
        }
    }
    ui->label->setPixmap(QPixmap::fromImage(m_image.scaled(m_image.width()*PREVIEW_WINDOW_SCALING, m_image.height()*PREVIEW_WINDOW_SCALING)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

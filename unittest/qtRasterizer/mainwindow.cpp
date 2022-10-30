#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QImage>
#include <QPixmap>
#include <math.h>
#include <QDebug>
#include "QTime"
#include <QThread>
#include <QFile>
#include <spdlog/spdlog.h>
#include "glu.h"
#include <QMessageBox>

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


bool loadPPMImage(std::string &filename, unsigned &width, unsigned &height, unsigned char *&imgData) {

  FILE *file = fopen(filename.c_str(), "rb");
  if (file == NULL ) {
    QMessageBox msgBox;
    msgBox.setText(QString("Can not find texture data file ")+QString(filename.c_str()));
    msgBox.exec();
    return false;
  }

  char line[256];
  fgets(line, 256, file);
  if(strncmp(line, "P6", 2)) {
    QMessageBox msgBox;
    msgBox.setText("File is not PPM P6 raw format");
    msgBox.exec();
    fclose(file);
    return false;
  }

  width = 0;
  height = 0;
  unsigned depth = 0;
  unsigned readItems =0;
  while (!feof(file) && readItems < 3) {
    fscanf(file, "%s", line);
    if (line[0] != '#' ) {
      if ( readItems == 0 )
        readItems += sscanf(line, "%d", &width);
      else if ( readItems == 1 )
        readItems += sscanf(line, "%d", &height);
      else if ( readItems == 2 ) {
        readItems += sscanf(line, "%d", &depth);
        while (!feof(file) && fgetc(file) != '\n') ;
      }
    }else{ // skip comments
      while (!feof(file) && fgetc(file) != '\n') ;
    }
  }
  if(depth >= 256) {
    QMessageBox msgBox;
    msgBox.setText("Only 8-bit PPM format is supported");
    msgBox.exec();
    fclose(file);
    return false;
  }

  unsigned byteCount = width * height * 3;
  imgData = (unsigned char *)malloc( width * height * 3 * sizeof(unsigned char));
  fread(imgData, byteCount, sizeof(unsigned char), file);
  fclose(file);

  return true;
}

      // returns a valid textureID on success, otherwise 0
GLuint loadTextureFile(std::string &filename) {
    unsigned width;
    unsigned height;
    int level = 0;
    int border = 0;
    unsigned char *imgData = NULL;

    // load image data
    if(!loadPPMImage(filename, width, height, imgData)) return 0;

    // data is aligned in byte order
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    //request textureID
    GLuint textureID;
    glGenTextures( 1, &textureID);

    // bind texture
    glBindTexture( GL_TEXTURE_2D, textureID);

    //define how to filter the texture (important but ignore for now)
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    // specify the 2D texture map
    glTexImage2D(GL_TEXTURE_2D, level, GL_RGB, width, height, border, GL_RGB, GL_UNSIGNED_BYTE, imgData);

    //the texture data is now handled by opengl, we can free the local copy
    free(imgData);

    // return unique texture identifier
    return textureID;
  }



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_image(RESOLUTION_W, RESOLUTION_H, QImage::Format_RGB888)
{
    ui->setupUi(this);
    IceGL::createInstance(m_renderer);

    connect(&m_timer, &QTimer::timeout, this, &MainWindow::newFrame);
    ui->label->setPixmap(QPixmap::fromImage(m_image));
    m_timer.setInterval(17);
    m_timer.setSingleShot(false);
    m_timer.start();

    std::string fileName("/Users/tobias/oglbsp/textures/dice_texture_flip.ppm");
    m_textureId = loadTextureFile(fileName);

    m_textureId2 = loadTexture(cubeTexture);

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
    GLfloat light_diffuse[] = { 1.5, 1.5, 1.5, 1.0 };
    GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat light_position[] = { 1.0, 3.0, 6.0, 0.0 };

    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 8.0f);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    static constexpr bool ENABLE_LIGHT = true;
    if constexpr (ENABLE_LIGHT)
    {
       glActiveTexture(GL_TEXTURE0);
       glEnable(GL_LIGHT0);
       glEnable(GL_LIGHTING);

       static constexpr float colors[4] = {0.0, 0.0, 0.0, 0.0};
       glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
       glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, colors);
    }
    else
    {
        glActiveTexture(GL_TEXTURE0);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    }

    static constexpr bool ENABLE_BLACK_WHITE = true;
    if constexpr (ENABLE_BLACK_WHITE)
    {


//        static constexpr float colors[4] = {.7, .7, .7, 0.0};
//        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
//        glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, colors);
//        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE);
//        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_CONSTANT);
//        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_DOT3_RGB);
//        glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
//        glTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE, 1);

        glActiveTexture(GL_TEXTURE1);
        static constexpr float colors[4] = {.7, .7, .7, 0.0};
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
        glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, colors);
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE);
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_CONSTANT);
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
        glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
        glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
        glTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE, 1);
    }

    // glLineWidth(5.0f);
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
    glDisable(GL_SCISSOR_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_SCISSOR_TEST);
    glScissor(0, 50, 200, 46);
    glClearColor(1.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glScissor(200, 50, 200, 48);
    glClearColor(0.0f, 1.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glScissor(400, 50, 200, 50);
    glClearColor(0.0f, 0.0f, 1.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_SCISSOR_TEST);
    glScissor(200, 200, 200, 100);

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

    glActiveTexture(GL_TEXTURE0);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, m_textureId);
    glClientActiveTexture(GL_TEXTURE0);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, 0, cubeTexCoords);

    glActiveTexture(GL_TEXTURE1);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, m_textureId2);
    glClientActiveTexture(GL_TEXTURE1);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, 0, cubeTexCoords);

    // Draw the cube
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, cubeVerts);
    glNormalPointer(GL_FLOAT, 0, cubeNormals);
    glDrawElements(GL_TRIANGLES, sizeof(cubeIndex) / sizeof(cubeIndex[0]), GL_UNSIGNED_SHORT, cubeIndex);

    IceGL::getInstance().commit();

#if USE_SIMULATION
    for (uint32_t i = 0; i < RESOLUTION_H; i++)
    {
        for (uint32_t j = 0; j < RESOLUTION_W; j++)
        {
            uint8_t r = ((m_framebuffer[(i*RESOLUTION_W)+j] >> 11) & 0x1f) << 3;
            uint8_t g = ((m_framebuffer[(i*RESOLUTION_W)+j] >> 5) & 0x3f) << 2;
            uint8_t b = ((m_framebuffer[(i*RESOLUTION_W)+j] >> 0) & 0x1f) << 3;
            // uint8_t r = ((m_framebuffer[(i*RESOLUTION_W)+j] >> 24) & 0xff) << 0;
            // uint8_t g = ((m_framebuffer[(i*RESOLUTION_W)+j] >> 16) & 0xff) << 0;
            // uint8_t b = ((m_framebuffer[(i*RESOLUTION_W)+j] >> 8) & 0xff) << 0;
            m_image.setPixelColor(QPoint(j, i), QColor(r, g, b));
        }
    }
    ui->label->setPixmap(QPixmap::fromImage(m_image.scaled(m_image.width()*PREVIEW_WINDOW_SCALING, m_image.height()*PREVIEW_WINDOW_SCALING)));
#endif
}

MainWindow::~MainWindow()
{
    delete ui;
}

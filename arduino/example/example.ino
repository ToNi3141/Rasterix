// Rasterix
// Copyright (c) 2023 ToNi3141

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include <Adafruit_GFX.h>    // Core graphics library

#include "IceGL.hpp"
#include "IceGLWrapper.h"
#include "IBusConnector.hpp"
#include "Renderer.hpp"

// TFT control pins
static constexpr int8_t TFT_CS = 9;
static constexpr int8_t TFT_RST = -1;
static constexpr int8_t TFT_DC = 8;

// Use this pins with a Raspberry Pi Pico (RP2040)
// Map the pins to other ports because the pins from a normal arduino clashing with the SPI of the RP2040
static constexpr uint8_t CMD_CTS = 13;
static constexpr uint8_t CMD_RESETN = 12;
static constexpr uint8_t CMD_CS = 11;
static constexpr uint8_t CMD_TFT_MUX = 10;

// Use this pins with an normal arduino board 
// static constexpr uint8_t CMD_CTS = 7;
// static constexpr uint8_t CMD_RESETN = 6;
// static constexpr uint8_t CMD_CS = 5;
// static constexpr uint8_t CMD_TFT_MUX = 4;

// Setup the screen
#define SCREEN240x320 // ILI9341
//#define SCREEN240x240 // ST7789
//#define SCREEN128x128 // ST7735 
//#define SCREEN128x160 // ST7735

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

#ifdef SCREEN128x128
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
static constexpr uint16_t RESOLUTION_W = 128;
static constexpr uint16_t RESOLUTION_H = 128;
static constexpr uint16_t RENDER_LINES = 1;
#endif

#ifdef SCREEN128x160
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
static constexpr uint16_t RESOLUTION_W = 128;
static constexpr uint16_t RESOLUTION_H = 160;
static constexpr uint16_t RENDER_LINES = 2;
#endif

#ifdef SCREEN240x240
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
static constexpr uint16_t RESOLUTION_W = 240;
static constexpr uint16_t RESOLUTION_H = 240;
static constexpr uint16_t RENDER_LINES = 4;
#endif

#ifdef SCREEN240x320
#include "Adafruit_ILI9341.h"
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
static constexpr uint16_t RESOLUTION_W = 240;
static constexpr uint16_t RESOLUTION_H = 320;
static constexpr uint16_t RENDER_LINES = 5;
#endif

// Simple BusConnector which wraps the SPI
class BusConnector : public IBusConnector
{
public:
    BusConnector() { }

    virtual ~BusConnector() = default;

    virtual void writeData(const uint8_t* data, uint32_t bytes) override 
    {
        // TODO: Use SPI driver which accepts const uint8_t pointer.
        // The Arduino transfer() method saves the received data in the given buffer, which then
        // destoryes texture memory and so on (when using a const_cast). To avoid that, copy
        // the data temporary into a local buffer. Because this is a hack, we can be more hacky 
        // and use by default a 2048 bytes buffer. The iCE40 FiFo can't handle more more bytes anyway
        uint8_t bla[2048];
        memcpy(bla, data, bytes);
        SPI.transfer((void*)bla, bytes);
    }

    virtual bool clearToSend() override 
    {
        return digitalRead(CMD_CTS);
    }

    virtual void startColorBufferTransfer(const uint8_t) override {
        // Nothing to do here, data is automatically streamed to the display
    }

    void init() 
    {
        pinMode(CMD_CTS, INPUT); 
        pinMode(CMD_RESETN, OUTPUT);
        pinMode(CMD_CS, OUTPUT);
        pinMode(CMD_TFT_MUX, OUTPUT);
        // Disable display and gfx
        csGfx(false);
        tftMux(false);
        // Run Reset cycle
        digitalWrite(CMD_RESETN, 0);
        delay(1);
        digitalWrite(CMD_RESETN, 1);

        // Enable tft pass through for configuration data
        tftMux(true);
        #ifdef SCREEN128x128
        tft.initR(INITR_144GREENTAB);
        #endif
        #ifdef SCREEN128x160
        tft.initR(INITR_BLACKTAB); 
        #endif
        #ifdef SCREEN240x240
        tft.init(240, 240); 
        #endif
        #ifdef SCREEN240x320
        tft.begin();
        #endif
        tft.setRotation(2);
        // Create an area for pixel data which is as big as the display is
        tft.startWrite();
        tft.setAddrWindow(0, 0, RESOLUTION_W, RESOLUTION_H);
        // Disable tft pass through
        tftMux(false);
        // Enable gfx (which will now feed the display)
        csGfx(true);
    }

    void csGfx(bool select)
    {
        digitalWrite(CMD_CS, select);
    }

    void tftMux(bool select)
    {
        digitalWrite(CMD_TFT_MUX, select);
    }
};

// Instantiate the BusConnector
BusConnector busConnector;
// Instantiate the renderer, this is basically the driver for the Rasterix.
// Set display list size (one trinagle requires around 84 bytes in the display list), screen resolution and bus width (used internally for alignments depending on the FPGA configuration)
Renderer<32768, RENDER_LINES, RESOLUTION_H / RENDER_LINES, 32> renderer{busConnector};
// Instantiate the IceGL library
IceGL m_ogl{renderer};

GLuint m_textureId;

// returns a valid textureID on success, otherwise 0
GLuint loadTexture(const char* tex)
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

void setup() {
    Serial.begin(115200);

    // Initialize the IceGL wrapper so that we can use standard c functions
    initIceGlCWrapper(&m_ogl);

    // Initialize the bus connector
    busConnector.init();

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

    
void loop() {
    const uint32_t mstart = millis();

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

    // Tell IceGL that the image is finished and it can start rendering the scene 
    m_ogl.commit();

    // Calculate the FPS
    const uint32_t mend = millis();
    Serial.print("FPS: ");
    Serial.print(1000 / (mend - mstart));
    Serial.println();
}
#include <stdio.h>
#include "gl.h"
#include "glu.h"

class Mipmap
{
    static constexpr bool ENABLE_LIGHT = true;
    static constexpr bool ENABLE_BLACK_WHITE = false;
public:
    void init(const uint32_t resolutionW, const uint32_t resolutionH)
    {
        glGetIntegerv(GL_MAX_TEXTURE_UNITS, &m_tmuCount);

        m_textureId = generateLodTexture();

        // Setup viewport, depth range, and projection matrix
        glViewport(0, 0, resolutionW, resolutionH);
        glDepthRange(0.0, 1.0);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(30.0, static_cast<float>(resolutionW) / static_cast<float>(resolutionH), 0.1, 111.0);

        // Enable depth test and depth mask
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);

        // Setup one light which will lighten the plane
        GLfloat light_ambient[] = { 0.5, 0.5, 0.5, 0.0 };
        GLfloat light_diffuse[] = { 1.5, 1.5, 1.5, 1.0 };
        GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
        GLfloat light_position[] = { 1.0, 3.0, 6.0, 0.0 };

        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 8.0f);
        glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
        glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
        glLightfv(GL_LIGHT0, GL_POSITION, light_position);

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

        if constexpr (ENABLE_BLACK_WHITE)
        {
            static constexpr float colors[4] = {.7, .7, .7, 0.0};
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
            glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, colors);
            glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE);
            glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_CONSTANT);
            glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_DOT3_RGB);
            glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
            glTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE, 1);
        }
    }

    void draw()
    {
        // Setup clear color and clear the framebuffer
        glDisable(GL_SCISSOR_TEST);
        glClearColor(135.0f / 255.0f, 206.0f / 255.0f, 235.0f / 255.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Setup the model view matrix
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        // set camera
        gluLookAt(2.0f, 0.0f, 0.2f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);

        // Rotate cube
        static float t2 = 0;
        t2 += 1.0f;
        glRotatef(t2, 0.0f, 0.0f, 1.0f);

        // Tex Coords for texture 0
        glActiveTexture(GL_TEXTURE0);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, m_textureId);
        glClientActiveTexture(GL_TEXTURE0);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glTexCoordPointer(2, GL_FLOAT, 0, planeTexCoords);

        // Enable normals
        glEnableClientState(GL_NORMAL_ARRAY);
        glNormalPointer(GL_FLOAT, 0, planeNormals);
        
        // Enable vertex arrays
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(3, GL_FLOAT, 0, planeVerts);

        // Draw the cube
        glDrawElements(GL_TRIANGLES, sizeof(planeIndex) / sizeof(planeIndex[0]), GL_UNSIGNED_SHORT, planeIndex);
    }

private:
    static constexpr uint16_t planeIndex[] = {
        0, 1, 2, 0, 2, 3,
    };

    static constexpr float planeVerts[] = {
        -1.0f, 1.0f, 0.0f,
        -1.0f,-1.0f, 0.0f,
        1.0f,-1.0f, 0.0f,
        1.0f, 1.0f, 0.0f,
    };

    static constexpr float planeTexCoords[] = {
        0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
    };

    static constexpr float planeNormals[] = {
        0.0f,  0.0f,  1.0f,
        0.0f,  0.0f,  1.0f,
        0.0f,  0.0f,  1.0f,
        0.0f,  0.0f,  1.0f,
    };

    void generateTexture(char* buffer, uint32_t w, uint32_t h, uint8_t r, uint8_t g, uint8_t b)
    {
        for (uint32_t i = 0; i < w * h * 3; i+= 3)
        {
            buffer[i + 0] = r;
            buffer[i + 1] = g;
            buffer[i + 2] = b;
        }
    }

    // returns a valid textureID on success, otherwise 0
    GLuint generateLodTexture()
    {
        static constexpr uint32_t WIDTH = 256;
        static constexpr uint32_t HEIGHT = 256;
        int level = 0;
        int border = 0;

        // data is aligned in byte order
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        // request textureID
        GLuint textureID;
        glGenTextures(1, &textureID);

        // bind texture
        glBindTexture(GL_TEXTURE_2D, textureID);

        // Temporary buffer
        static char pixelBuffer[WIDTH * HEIGHT * 3];

        // LOD 0
        generateTexture(pixelBuffer, 256, 256, 255, 0, 0);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, 256, border, GL_RGB, GL_UNSIGNED_BYTE, pixelBuffer);
        // LOD 1
        generateTexture(pixelBuffer, 128, 128, 0, 255, 0);
        glTexImage2D(GL_TEXTURE_2D, 1, GL_RGB, 128, 128, border, GL_RGB, GL_UNSIGNED_BYTE, pixelBuffer);
        // LOD 2
        generateTexture(pixelBuffer, 64, 64, 0, 0, 255);
        glTexImage2D(GL_TEXTURE_2D, 2, GL_RGB, 64, 64, border, GL_RGB, GL_UNSIGNED_BYTE, pixelBuffer);
        // LOD 3
        generateTexture(pixelBuffer, 32, 32, 255, 0, 255);
        glTexImage2D(GL_TEXTURE_2D, 3, GL_RGB, 32, 32, border, GL_RGB, GL_UNSIGNED_BYTE, pixelBuffer);
        // LOD 4
        generateTexture(pixelBuffer, 16, 16, 0, 255, 255);
        glTexImage2D(GL_TEXTURE_2D, 4, GL_RGB, 16, 16, border, GL_RGB, GL_UNSIGNED_BYTE, pixelBuffer);
        // LOD 5
        generateTexture(pixelBuffer, 8, 8, 255, 255, 0);
        glTexImage2D(GL_TEXTURE_2D, 5, GL_RGB, 8, 8, border, GL_RGB, GL_UNSIGNED_BYTE, pixelBuffer);
        // LOD 6
        generateTexture(pixelBuffer, 4, 4, 255, 255, 255);
        glTexImage2D(GL_TEXTURE_2D, 6, GL_RGB, 4, 4, border, GL_RGB, GL_UNSIGNED_BYTE, pixelBuffer);
        // LOD 7
        generateTexture(pixelBuffer, 2, 2, 127, 127, 127);
        glTexImage2D(GL_TEXTURE_2D, 7, GL_RGB, 2, 2, border, GL_RGB, GL_UNSIGNED_BYTE, pixelBuffer);
        // LOD 8
        generateTexture(pixelBuffer, 1, 1, 0, 0, 0);
        glTexImage2D(GL_TEXTURE_2D, 8, GL_RGB, 1, 1, border, GL_RGB, GL_UNSIGNED_BYTE, pixelBuffer);
        
        // define how to filter the texture
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);

        // define clamping mode
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        // return unique texture identifier
        return textureID;
    }

    GLint  m_tmuCount = 0;
    GLuint m_textureId = 0;
    GLuint m_multiTextureId = 0;
};

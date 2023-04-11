#include <GLUT/glut.h>
#include "../../stencilShadow/StencilShadow.hpp"
#include "../../minimal/Minimal.hpp"

#include <chrono>
#include <thread>


static const uint32_t RESOLUTION_H = 600;
static const uint32_t RESOLUTION_W = 1024;

StencilShadow m_testScene;
//Minimal m_testScene;
static int window;
void scene_render()
{
    m_testScene.draw();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    glutSwapBuffers();
}

 int main (int argc, char* argv[]) {
     glutInit(&argc, argv);
     glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_STENCIL);
     glutInitWindowSize(RESOLUTION_W, RESOLUTION_H);

     window = glutCreateWindow(argv[0]);
 #ifdef FULLSCREEN
     glutFullScreen();
 #endif

     glutDisplayFunc(scene_render);
     glutIdleFunc(scene_render);

     m_testScene.init(RESOLUTION_W, RESOLUTION_H);

     glutMainLoop();
 }

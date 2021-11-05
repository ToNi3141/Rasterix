# Set here the path to your local verilator installation
VERILATOR_PATH = /usr/local/Cellar/verilator/4.200/share/verilator

ICEGL_PATH = ../../lib/gl
VERILATOR_BUS_CONNECTOR_PATH = ../../unittest/cpp/include
VERILATOR_CODE_GEN_PATH = ../../rtl/top/Verilator/obj_dir

QT       += core gui
CONFIG += c++17
QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.15

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

#DEFINES += NO_PERSP_CORRECT
#DEFINES += NO_ZBUFFER
DEFINES += HARDWARE_RENDERER
#DEFINES += SOFTWARE_RENDERER

TARGET = qtRasterizer
TEMPLATE = app
QT += serialport

SOURCES += main.cpp\
    $${ICEGL_PATH}/IceGLWrapper.cpp \
    $${ICEGL_PATH}/TnL.cpp \
        mainwindow.cpp \
    $${ICEGL_PATH}/IceGL.cpp \
    $${ICEGL_PATH}/Rasterizer.cpp\
    $${VERILATOR_PATH}/include/verilated.cpp

HEADERS  += mainwindow.h\
    $${ICEGL_PATH}/DisplayList.hpp \
    $${ICEGL_PATH}/IBusConnector.hpp \
    $${ICEGL_PATH}/IRenderer.hpp \
    $${ICEGL_PATH}/IceGLTypes.h \
    $${ICEGL_PATH}/IceGLWrapper.h \
    $${ICEGL_PATH}/Renderer.hpp \
    $${ICEGL_PATH}/TnL.hpp \
    $${ICEGL_PATH}/Vec.hpp \
    $${ICEGL_PATH}/Veci.hpp \
    $${ICEGL_PATH}/GRamAlloc.hpp \
    $${ICEGL_PATH}/IceGL.hpp \
    $${ICEGL_PATH}/Rasterizer.hpp \
    $${ICEGL_PATH}/Mat44.hpp \
    $${VERILATOR_BUS_CONNECTOR_PATH}/VerilatorBusConnector.hpp \
    ../../lib/gl/DisplayListAssembler.hpp

FORMS    += mainwindow.ui


QMAKE_CXXFLAGS += -I$${VERILATOR_CODE_GEN_PATH}/ \
    -I$${VERILATOR_BUS_CONNECTOR_PATH}/ \
    -I$${VERILATOR_PATH}/include/ \
    -I$${ICEGL_PATH}/


LIBS += $${VERILATOR_CODE_GEN_PATH}/Vtop__ALL.a


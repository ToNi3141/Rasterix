TARGET_BUILD = simulation
#TARGET_BUILD = hardware

# Set here the path to your local verilator installation

ICEGL_PATH = ../../lib/gl


QT       += core gui
CONFIG += c++17
QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.15

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

DEFINES += HARDWARE_RENDERER

TEMPLATE = app
QT += serialport
TARGET = qtRasterizer

SOURCES += main.cpp\
    $${ICEGL_PATH}/IceGLWrapper.cpp \
    $${ICEGL_PATH}/VertexPipeline.cpp \
        mainwindow.cpp \
    $${ICEGL_PATH}/IceGL.cpp \
    $${ICEGL_PATH}/Clipper.cpp \
    $${ICEGL_PATH}/Lighting.cpp \
    $${ICEGL_PATH}/TexGen.cpp \
    $${ICEGL_PATH}/RenderObj.cpp \
    $${ICEGL_PATH}/Rasterizer.cpp

HEADERS  += mainwindow.h\
    $${ICEGL_PATH}/DisplayList.hpp \
    $${ICEGL_PATH}/IBusConnector.hpp \
    $${ICEGL_PATH}/IRenderer.hpp \
    $${ICEGL_PATH}/IceGLTypes.h \
    $${ICEGL_PATH}/IceGLWrapper.h \
    $${ICEGL_PATH}/Renderer.hpp \
    $${ICEGL_PATH}/VertexPipeline.hpp \
    $${ICEGL_PATH}/Vec.hpp \
    $${ICEGL_PATH}/Veci.hpp \
    $${ICEGL_PATH}/IceGL.hpp \
    $${ICEGL_PATH}/Rasterizer.hpp \
    $${ICEGL_PATH}/Mat44.hpp \
    $${ICEGL_PATH}/Clipper.hpp \
    $${ICEGL_PATH}/Lighting.hpp \
    $${ICEGL_PATH}/TexGen.hpp \
    $${ICEGL_PATH}/RenderObj.hpp \
    $${ICEGL_PATH}/TextureMemoryManager.hpp \
    $${ICEGL_PATH}/DisplayListAssembler.hpp

equals(TARGET_BUILD, "hardware") {
    DEFINES += USE_HARDWARE

    FT60X_BUS_CONNECTOR_PATH = ../../lib/driver
    FT60X_LIB_PATH = ../../lib/ftd3xx/osx

    LIBS += /usr/local/homebrew/Cellar/libusb/1.0.26/lib/libusb-1.0.dylib
    LIBS += $${FT60X_LIB_PATH}/libftd3xx-static.a

    QMAKE_CXXFLAGS += -I$${FT60X_BUS_CONNECTOR_PATH}/ \
        -I$${FT60X_LIB_PATH}/

    QMAKE_CFLAGS += -I$${FT60X_BUS_CONNECTOR_PATH}/\
        -I$${FT60X_LIB_PATH}/

    HEADERS += $${FT60X_BUS_CONNECTOR_PATH}/FT60XBusConnector.hpp
}
equals(TARGET_BUILD, "simulation") {
    VERILATOR_PATH = /opt/homebrew/Cellar/verilator/4.220/share/verilator
    VERILATOR_BUS_CONNECTOR_PATH = ../../lib/driver
    VERILATOR_CODE_GEN_PATH = ../../rtl/top/Verilator/obj_dir

    DEFINES += USE_SIMULATION

    HEADERS += $${VERILATOR_BUS_CONNECTOR_PATH}/VerilatorBusConnector.hpp

    SOURCES += $${VERILATOR_PATH}/include/verilated.cpp

    LIBS += $${VERILATOR_CODE_GEN_PATH}/Vtop__ALL.a
}

FORMS    += mainwindow.ui


QMAKE_CXXFLAGS += -I$${VERILATOR_CODE_GEN_PATH}/ \
    -I$${VERILATOR_BUS_CONNECTOR_PATH}/ \
    -I$${VERILATOR_PATH}/include/ \
    -I$${ICEGL_PATH}/





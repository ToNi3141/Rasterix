TARGET_BUILD = simulation
#TARGET_BUILD = hardware
VARIANT = RRXIF
#VARIANT = RRXEF

PATH_PREFIX = ../../..

RRXGL_PATH = $${PATH_PREFIX}/lib/gl

QT       += core gui
CONFIG += c++17
QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.15

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

DEFINES += HARDWARE_RENDERER

TEMPLATE = app
TARGET = qtRasterizerFpga

SOURCES += main.cpp\
    $${RRXGL_PATH}/vertexpipeline/VertexPipeline.cpp \
        mainwindow.cpp \
    $${RRXGL_PATH}/RRXGL.cpp \
    $${RRXGL_PATH}/vertexpipeline/Clipper.cpp \
    $${RRXGL_PATH}/vertexpipeline/Lighting.cpp \
    $${RRXGL_PATH}/vertexpipeline/TexGen.cpp \
    $${RRXGL_PATH}/vertexpipeline/RenderObj.cpp \
    $${RRXGL_PATH}/pixelpipeline/PixelPipeline.cpp \
    $${RRXGL_PATH}/glImpl.cpp \
    $${RRXGL_PATH}/renderer/Rasterizer.cpp \
    $${RRXGL_PATH}/renderer/Renderer.cpp \
    $${RRXGL_PATH}/vertexpipeline/ViewPort.cpp \
    $${RRXGL_PATH}/vertexpipeline/MatrixStack.cpp \
    $${RRXGL_PATH}/vertexpipeline/Culling.cpp \
    $${RRXGL_PATH}/vertexpipeline/PrimitiveAssembler.cpp \
    $${RRXGL_PATH}/pixelpipeline/Fogging.cpp \
    $${RRXGL_PATH}/pixelpipeline/Texture.cpp \
    $${RRXGL_PATH}/pixelpipeline/Stencil.cpp \
    $${RRXGL_PATH}/gl.cpp \
    $${RRXGL_PATH}/glu.cpp

HEADERS  += mainwindow.h \
    ../testscene.hpp \
    $${RRXGL_PATH}/*.hpp \
    $${RRXGL_PATH}/vertexpipeline/*.hpp \
    $${RRXGL_PATH}/pixelpipeline/*.hpp \
    $${RRXGL_PATH}/math/*.hpp \
    $${RRXGL_PATH}/renderer/*.hpp \
    $${RRXGL_PATH}/*.h \
    $${RRXGL_PATH}/renderer/registers/* \
    $${RRXGL_PATH}/renderer/commands/*


# spdlog
DEFINES += SPDLOG_ACTIVE_LEVEL=3
DEFINES += GL_SILENCE_DEPRICATION
QMAKE_CXXFLAGS += -I$${PATH_PREFIX}/lib/3rdParty/spdlog-1.10.0/include/
QMAKE_CFLAGS += -I$${PATH_PREFIX}/lib/3rdParty/spdlog-1.10.0/include/

# tcb span
QMAKE_CXXFLAGS += -I$${PATH_PREFIX}/lib/3rdParty/span/include/
QMAKE_CFLAGS += -I$${PATH_PREFIX}/lib/3rdParty/span/include/

# TMU settings
DEFINES += RRX_CORE_TMU_COUNT=2
DEFINES += RRX_CORE_MAX_TEXTURE_SIZE=256
DEFINES += RRX_CORE_ENABLE_MIPMAPPING=true
# Display Settings
DEFINES += RRX_CORE_MAX_DISPLAY_WIDTH=640
DEFINES += RRX_CORE_MAX_DISPLAY_HEIGHT=480
# Rasterizer settings
DEFINES += RRX_CORE_USE_FLOAT_INTERPOLATION=false
# Bus Settings
DEFINES += RRX_CORE_CMD_STREAM_WIDTH=64
# Texture Memory Settings
DEFINES += RRX_CORE_NUMBER_OF_TEXTURE_PAGES=6912
DEFINES += RRX_CORE_NUMBER_OF_TEXTURES=6912
DEFINES += RRX_CORE_TEXTURE_PAGE_SIZE=4096
# Memory RAM location. This is used as memory offset for all device memory  
# address calculations. Mostly useful for architectures with shared memory
DEFINES += RRX_CORE_GRAM_MEMORY_LOC=0x0 # No shared memory
# Framebuffer Memory Location
DEFINES += RRX_CORE_COLOR_BUFFER_LOC_1=0x01E00000
DEFINES += RRX_CORE_COLOR_BUFFER_LOC_2=0x01C00000
DEFINES += RRX_CORE_DEPTH_BUFFER_LOC=0x01A00000
DEFINES += RRX_CORE_STENCIL_BUFFER_LOC=0x01900000
equals(VARIANT, "RRXIF") {
    DEFINES += RRX_CORE_FRAMEBUFFER_TYPE=FramebufferType::INTERNAL_TO_STREAM
    DEFINES += RRX_CORE_FRAMEBUFFER_SIZE_IN_WORDS=32768
}
equals(VARIANT, "RRXEF") {
    DEFINES += RRX_CORE_FRAMEBUFFER_TYPE=FramebufferType::EXTERNAL_MEMORY_TO_STREAM
    DEFINES += RRX_CORE_FRAMEBUFFER_SIZE_IN_WORDS=614400
}

equals(TARGET_BUILD, "hardware") {
    DEFINES += USE_HARDWARE

    FT60X_BUS_CONNECTOR_PATH = $${PATH_PREFIX}/lib/driver/ft60x
    FT60X_LIB_PATH = $${PATH_PREFIX}/lib/driver/ft60x/ftd3xx/osx

    LIBS += /usr/local/homebrew/Cellar/libusb/1.0.26/lib/libusb-1.0.dylib
    LIBS += $${FT60X_LIB_PATH}/libftd3xx-static.a

    QMAKE_CXXFLAGS += -I$${FT60X_BUS_CONNECTOR_PATH}/ \
        -I$${FT60X_LIB_PATH}/

    QMAKE_CFLAGS += -I$${FT60X_BUS_CONNECTOR_PATH}/\
        -I$${FT60X_LIB_PATH}/

    HEADERS += $${FT60X_BUS_CONNECTOR_PATH}/FT60XBusConnector.hpp
    SOURCES += $${FT60X_BUS_CONNECTOR_PATH}/FT60XBusConnector.cpp
}
equals(TARGET_BUILD, "simulation") {
    # Set here the path to your local verilator installation
    unix:!macx {
        VERILATOR_PATH = /usr/local/share/verilator
        SOURCES += $${VERILATOR_PATH}/include/verilated_threads.cpp
    }
    macx: {
        VERILATOR_PATH = /opt/homebrew/Cellar/verilator/4.220/share/verilator
    }

    VERILATOR_BUS_CONNECTOR_PATH = $${PATH_PREFIX}/lib/driver/verilator
    VERILATOR_CODE_GEN_PATH = $${PATH_PREFIX}/rtl/top/Verilator/obj_dir

    DEFINES += USE_SIMULATION

    HEADERS += $${VERILATOR_BUS_CONNECTOR_PATH}/VerilatorBusConnector.hpp
    HEADERS += $${PATH_PREFIX}/lib/utils/GenericMemoryBusConnector.hpp

    SOURCES += $${VERILATOR_PATH}/include/verilated.cpp
    SOURCES += $${VERILATOR_PATH}/include/verilated_vcd_c.cpp

    LIBS += $${VERILATOR_CODE_GEN_PATH}/Vtop__ALL.a
}

FORMS    += mainwindow.ui

QMAKE_CXXFLAGS += -I$${VERILATOR_CODE_GEN_PATH}/ \
    -I$${VERILATOR_BUS_CONNECTOR_PATH}/ \
    -I$${VERILATOR_BUS_CONNECTOR_PATH}/../ \
    -I$${VERILATOR_PATH}/include/ \
    -I$${RRXGL_PATH}/ \
    -I$${PATH_PREFIX}/lib/utils

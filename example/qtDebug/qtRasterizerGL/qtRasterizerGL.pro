
ICEGL_PATH = ../../../lib/gl

LIBS+=-framework GLUT
QT       += core gui opengl
CONFIG += c++2a
QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.15

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TEMPLATE = app
TARGET = qtRasterizerGL

SOURCES += main.cpp

HEADERS  += ../testscene.hpp \
    $${ICEGL_PATH}/Vec.hpp \
    $${ICEGL_PATH}/Mat44.hpp

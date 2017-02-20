QT       -= gui
QT       += network

TARGET = MEGAShellExtDolphin
TEMPLATE = lib

SOURCES += megasync-plugin.cpp

HEADERS += megasync-plugin.h

#greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

#CONFIG += link_pkgconfig
#PKGCONFIG+=dolphin

## library
#target.path = $$system(pkg-config --variable=extensionsdir thunarx-2)
INSTALLS += target

QMAKE_CLEAN += $(TARGET) lib$${TARGET}.so lib$${TARGET}.so.1 lib$${TARGET}.so.1.0

QMAKE_CXXFLAGS += -std=c++11

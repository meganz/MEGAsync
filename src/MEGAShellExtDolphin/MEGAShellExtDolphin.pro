# Compilation with this .pro is broken. Use cmake instead. This is kept for convenience only.

QT       -= gui
QT       += network

TARGET = megasyncplugin
TEMPLATE = lib

CONFIG += WITH_KF5
DEFINES += WITH_KF5

SOURCES += megasync-plugin.cpp \
    megasync-plugin-overlay.cpp \
    megasync-plugin-overlay.json


HEADERS += megasync-plugin.h

# library
target.path = $$system(kde4-config --path module | cut -d ":" -f2)
INSTALLS += target

QMAKE_CLEAN += $(TARGET) lib$${TARGET}.so lib$${TARGET}.so.1 lib$${TARGET}.so.1.0

QMAKE_CXXFLAGS += -std=c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += KIOCore KIOFileWidgets KIOWidgets KNTLM

DISTFILES += \
    megasync-plugin-overlay.json


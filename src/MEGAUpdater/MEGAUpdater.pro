CONFIG -= qt

CONFIG(debug, debug|release) {
    CONFIG -= debug release
    CONFIG += debug
}
CONFIG(release, debug|release) {
    CONFIG -= debug release
    CONFIG += release
}

TARGET = MEGAUpdater
TEMPLATE = app
CONFIG += console
CONFIG += USE_MEGAAPI

HEADERS += UpdateTask.h \
    Preferences.h \

SOURCES += MEGAUpdater.cpp \
    UpdateTask.cpp

include(../MEGASync/mega/bindings/qt/sdk.pri)
DEFINES -= MEGA_QT_LOGGING
DEFINES -= USE_QT

LIBS += -lpthread

SOURCES += src/thread/posixthread.cpp
SOURCES -= src/gfx/qt.cpp
SOURCES -= src/thread/qtthread.cpp
SOURCES -= bindings/qt/QTMegaRequestListener.cpp
SOURCES -= bindings/qt/QTMegaTransferListener.cpp
SOURCES -= bindings/qt/QTMegaGlobalListener.cpp
SOURCES -= bindings/qt/QTMegaSyncListener.cpp
SOURCES -= bindings/qt/QTMegaListener.cpp
SOURCES -= bindings/qt/QTMegaEvent.cpp

macx {

    DEFINES += USE_PTHREAD
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.6
    QMAKE_CXXFLAGS -= -stdlib=libc++
    QMAKE_LFLAGS -= -stdlib=libc++
    CONFIG -= c++11
    LIBS += -framework Cocoa -framework SystemConfiguration -framework CoreFoundation -framework Foundation -framework Security
    QMAKE_CXXFLAGS += -g
}


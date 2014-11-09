#-------------------------------------------------
#
# Project created by QtCreator 2014-07-09T10:41:26
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MEGAlogger
TEMPLATE = app


SOURCES += main.cpp \
    MegaDebugServer.cpp

HEADERS  += \
    MegaDebugServer.h

FORMS    += \
    MegaDebugServer.ui

win32 {
    RC_FILE = icon.rc
}

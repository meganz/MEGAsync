#-------------------------------------------------
#
# Project created by QtCreator 2013-10-17T12:41:38
#
#-------------------------------------------------

debug_and_release {
    CONFIG -= debug_and_release
    CONFIG += debug_and_release
}
CONFIG(debug, debug|release) {
    CONFIG -= debug release
    CONFIG += debug
}
CONFIG(release, debug|release) {
    CONFIG -= debug release
    CONFIG += release
}

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MEGAsync
TEMPLATE = app

debug {
    CONFIG += console
}

include(gui/gui.pri)
include(sdk/sdk.pri)
include(control/control.pri)
include(platform/platform.pri)
include(google_breakpad/google_breakpad.pri)

DEPENDPATH += $$PWD
INCLUDEPATH += $$PWD

DEFINES += QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII

SOURCES += MegaApplication.cpp
HEADERS += MegaApplication.h

TRANSLATIONS    = gui/translations/MEGASyncStrings_es.ts
CODECFORTR = UTF8

win32 {
    RC_FILE = icon.rc
}


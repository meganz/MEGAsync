#-------------------------------------------------
#
# Project created by QtCreator 2013-10-17T12:41:38
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MEGAsync
TEMPLATE = app

include(gui/gui.pri)
include(sdk/sdk.pri)
include(utils/utils.pri)

DEPENDPATH += $$PWD
INCLUDEPATH += $$PWD

DEFINES += QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII

SOURCES += MegaApplication.cpp
HEADERS += MegaApplication.h

win32 {
    RC_FILE = icon.rc
}

FORMS +=


#-------------------------------------------------
#
# Project created by QtCreator 2013-10-22T17:12:03
#
#-------------------------------------------------

CONFIG += dll qaxserver qt
QT += network

TARGET = MEGAShellExt
TEMPLATE = lib

DEFINES += MEGASHELLEXT_LIBRARY

SOURCES += MEGAShellExt.cpp dllmain.cpp
HEADERS += MEGAShellExt.h

LIBS += -luser32 -lole32 -loleaut32 -lgdi32 -luuid

DEF_FILE = qaxserver.def
RC_FILE = qaxserver.rc

OTHER_FILES += \
    qaxserver.rc \
    qaxserver.def

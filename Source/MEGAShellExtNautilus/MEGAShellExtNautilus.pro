QT       -= core gui

TARGET = MEGAShellExtNautilus
TEMPLATE = lib

SOURCES += MEGAShellExt.cpp \
    ContextMenuExt.cpp

HEADERS += MEGAShellExt.h \
    ContextMenuExt.h

CONFIG += link_pkgconfig
PKGCONFIG += libnautilus-extension

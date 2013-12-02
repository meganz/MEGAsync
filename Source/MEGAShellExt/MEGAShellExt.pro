#-------------------------------------------------
#
# Project created by QtCreator 2013-10-22T17:12:03
#
#-------------------------------------------------

CONFIG -= qt

TARGET = MEGAShellExt
TEMPLATE = lib

LIBS += -luser32 -lole32 -loleaut32 -lgdi32 -luuid -lAdvapi32 -lShell32

DEF_FILE = GlobalExportFunctions.def
RC_FILE = MEGAShellExt.rc

OTHER_FILES += \
    MEGAShellExt.rc \
    GlobalExportFunctions.def

HEADERS += \
    ShellExtSyncing.h \
    ShellExtSynced.h \
    ShellExt.h \
    resource.h \
    RegUtils.h \
    ContextMenuExt.h \
    ClassFactoryShellExtSyncing.h \
    ClassFactoryShellExtSynced.h \
    ClassFactoryShellExtPending.h \
    ClassFactoryContextMenuExt.h \
    ClassFactory.h

SOURCES += \
    ShellExtSyncing.cpp \
    ShellExtSynced.cpp \
    ShellExt.cpp \
    RegUtils.cpp \
    dllmain.cpp \
    ContextMenuExt.cpp \
    ClassFactoryShellExtSyncing.cpp \
    ClassFactoryShellExtSynced.cpp \
    ClassFactoryShellExtPending.cpp \
    ClassFactoryContextMenuExt.cpp \
    ClassFactory.cpp

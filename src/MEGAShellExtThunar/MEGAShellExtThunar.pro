QT       -= core gui

TARGET = MEGAShellExtThunar
TEMPLATE = lib

SOURCES += MEGAShellExt.c \
    mega_ext_client.c

HEADERS += MEGAShellExt.h \
    mega_ext_client.h

CONFIG += link_pkgconfig
PKGCONFIG+=thunarx-3 glib-2.0

# library
target.path = $$system(pkg-config --variable=extensionsdir thunarx-3)
INSTALLS += target

QMAKE_CLEAN += $(TARGET) lib$${TARGET}.so lib$${TARGET}.so.1 lib$${TARGET}.so.1.0

QT       -= core gui

TARGET = MEGAShellExtThunar
TEMPLATE = lib

SOURCES += MEGAShellExt.c \
    mega_ext_client.c

HEADERS += MEGAShellExt.h \
    mega_ext_client.h

CONFIG += link_pkgconfig
PKGCONFIG += thunarx-2

# library
target.path = $$system(pkg-config --variable=extensionsdir thunarx-2)
INSTALLS += target

QMAKE_CLEAN += $(TARGET) lib$${TARGET}.so lib$${TARGET}.so.1 lib$${TARGET}.so.1.0

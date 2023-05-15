QT       -= core gui

TARGET = MEGAShellExtThunar
TEMPLATE = lib

SOURCES += MEGAShellExt.c \
    mega_ext_client.c

HEADERS += MEGAShellExt.h \
    mega_ext_client.h

CONFIG += link_pkgconfig

system(pkg-config --exists thunarx-2) {
PKGCONFIG+=thunarx-2
DEFINES+=USING_THUNAR2
} else {
PKGCONFIG+=thunarx-3
DEFINES+=USING_THUNAR3
}

PKGCONFIG+=glib-2.0

# get env variable
DESKTOP_DESTDIR = $$(DESKTOP_DESTDIR)
isEmpty(DESKTOP_DESTDIR) {
    DESKTOP_DESTDIR = /usr
}

# library
EXTENSIONS_PATH = $$system(pkg-config --variable=extensionsdir thunarx-2 || pkg-config --variable=extensionsdir thunarx-3 | sed \"s@/usr@@\")
target.path = $${DESKTOP_DESTDIR}$${EXTENSIONS_PATH}
INSTALLS += target

QMAKE_CLEAN += $(TARGET) lib$${TARGET}.so lib$${TARGET}.so.1 lib$${TARGET}.so.1.0

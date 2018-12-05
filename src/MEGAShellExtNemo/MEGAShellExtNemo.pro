QT       -= core gui

TARGET = MEGAShellExtNemo
TEMPLATE = lib

SOURCES += mega_ext_module.c \
    mega_ext_client.c \
    mega_notify_client.c \
    MEGAShellExt.c

HEADERS += MEGAShellExt.h \
    mega_ext_client.h \
    mega_notify_client.h

CONFIG += link_pkgconfig
PKGCONFIG += libnemo-extension

# library
target.path = $$system(pkg-config libnemo-extension --variable=extensiondir)
INSTALLS += target

QMAKE_CLEAN += $(TARGET) lib$${TARGET}.so lib$${TARGET}.so.1 lib$${TARGET}.so.1.0

# get env variable
DESKTOP_DESTDIR = $$(DESKTOP_DESTDIR)
isEmpty(DESKTOP_DESTDIR) {
    DESKTOP_DESTDIR = /usr
}

HICOLOR = $$DESKTOP_DESTDIR/share/icons/hicolor

# emblems
EMBLEMS_LOC = $$PWD/data/emblems
emblems32.path = $${HICOLOR}/32x32/emblems
emblems32.files = $${EMBLEMS_LOC}/32x32/*
emblems64.path = $${HICOLOR}/64x64/emblems
emblems64.files = $${EMBLEMS_LOC}/64x64/*
INSTALLS += emblems32  emblems64

# update icons cache
!contains(DEFINES, no_desktop) {
    update_cache.commands = gtk-update-icon-cache -f -t $${HICOLOR} || true
    update_cache.path = $$PWD
    INSTALLS += update_cache
}

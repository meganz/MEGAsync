QT       -= core gui

TARGET = MEGAShellExtNautilus
TEMPLATE = lib

SOURCES += mega_ext_module.c \
    mega_ext_client.c \
    MEGAShellExt.c

HEADERS += MEGAShellExt.h \
    mega_ext_client.h

CONFIG += link_pkgconfig
PKGCONFIG += libnautilus-extension

# library
target.path = $$system(pkg-config libnautilus-extension --variable=extensiondir)
INSTALLS += target

QMAKE_CLEAN += $(TARGET) lib$${TARGET}.so lib$${TARGET}.so.1 lib$${TARGET}.so.1.0

HICOLOR = /usr/share/icons/hicolor

# icons
ICONS_LOC = $$PWD/data/icons/hicolor
icons32.path = $${HICOLOR}/32x32/apps
icons32.files = $${ICONS_LOC}/32x32/apps/mega.png
icons48.path = $${HICOLOR}/48x48/apps
icons48.files = $${ICONS_LOC}/48x48/apps/mega.png
icons128.path = $${HICOLOR}/128x128/apps
icons128.files = $${ICONS_LOC}/128x128/apps/mega.png
icons256.path = $${HICOLOR}/256x256/apps
icons256.files = $${ICONS_LOC}/256x256/apps/mega.png
icons512.path = $${HICOLOR}/512x512/apps
icons512.files = $${ICONS_LOC}/512x512/apps/mega.png
INSTALLS += icons32 icons48 icons128 icons256 icons512

# emblems
EMBLEMS_LOC = $$PWD/data/emblems
emblems32.path = $${HICOLOR}/32x32/emblems
emblems32.files = $${EMBLEMS_LOC}/32x32/*
emblems64.path = $${HICOLOR}/64x64/emblems
emblems64.files = $${EMBLEMS_LOC}/64x64/*
INSTALLS += emblems32  emblems64

# update icons cache
update_cache.commands = gtk-update-icon-cache -f -t $${HICOLOR} || true
update_cache.path = $$PWD
INSTALLS += update_cache

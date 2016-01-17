#-------------------------------------------------
#
# Project created by QtCreator 2013-10-17T12:41:38
#
#-------------------------------------------------

DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x000000

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

unix:!macx {
TARGET = megasync
}
else {
TARGET = MEGAsync
}

TEMPLATE = app

#DEFINES += LOG_TO_LOGGER
#DEFINES += LOG_TO_FILE
#DEFINES += LOG_TO_STDOUT

debug {
    CONFIG += console
    DEFINES += CREATE_COMPATIBLE_MINIDUMPS
    DEFINES += LOG_TO_STDOUT
#   DEFINES += LOG_TO_FILE
    DEFINES += LOG_TO_LOGGER
}

CONFIG += USE_LIBUV

include(gui/gui.pri)
include(mega/bindings/qt/sdk.pri)
include(control/control.pri)
include(platform/platform.pri)
include(google_breakpad/google_breakpad.pri)
include(qtlockedfile/qtlockedfile.pri)

DEPENDPATH += $$PWD
INCLUDEPATH += $$PWD

DEFINES += QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII

SOURCES += MegaApplication.cpp
HEADERS += MegaApplication.h

TRANSLATIONS = \
    gui/translations/MEGASyncStrings_ar.ts \
    gui/translations/MEGASyncStrings_bg.ts \
    gui/translations/MEGASyncStrings_cs.ts \
    gui/translations/MEGASyncStrings_de.ts \
    gui/translations/MEGASyncStrings_ee.ts \
    gui/translations/MEGASyncStrings_en.ts \
    gui/translations/MEGASyncStrings_es.ts \
    gui/translations/MEGASyncStrings_fa.ts \
    gui/translations/MEGASyncStrings_fi.ts \
    gui/translations/MEGASyncStrings_fr.ts \
    gui/translations/MEGASyncStrings_he.ts \
    gui/translations/MEGASyncStrings_hr.ts \
    gui/translations/MEGASyncStrings_hu.ts \
    gui/translations/MEGASyncStrings_id.ts \
    gui/translations/MEGASyncStrings_it.ts \
    gui/translations/MEGASyncStrings_ja.ts \
    gui/translations/MEGASyncStrings_ka.ts \
    gui/translations/MEGASyncStrings_ko.ts \
    gui/translations/MEGASyncStrings_nl.ts \
    gui/translations/MEGASyncStrings_pl.ts \
    gui/translations/MEGASyncStrings_pt_BR.ts \
    gui/translations/MEGASyncStrings_pt.ts \
    gui/translations/MEGASyncStrings_ro.ts \
    gui/translations/MEGASyncStrings_ru.ts \
    gui/translations/MEGASyncStrings_sk.ts \
    gui/translations/MEGASyncStrings_sl.ts \
    gui/translations/MEGASyncStrings_sr.ts \
    gui/translations/MEGASyncStrings_sv.ts \
    gui/translations/MEGASyncStrings_tl.ts \
    gui/translations/MEGASyncStrings_tr.ts \
    gui/translations/MEGASyncStrings_uk.ts \
    gui/translations/MEGASyncStrings_vi.ts \
    gui/translations/MEGASyncStrings_zh_CN.ts \
    gui/translations/MEGASyncStrings_zh_TW.ts

CODECFORTR = UTF8

win32 {
    RC_FILE = icon.rc
    QMAKE_LFLAGS += /LARGEADDRESSAWARE
}


macx {
    QMAKE_CXXFLAGS += -DCRYPTOPP_DISABLE_ASM -D_DARWIN_C_SOURCE
    MAC_ICONS_RESOURCES.files += folder.icns
    MAC_ICONS_RESOURCES.files += folder_yosemite.icns
    MAC_ICONS_RESOURCES.files += appicon32.tiff
    MAC_ICONS_RESOURCES.path = Contents/Resources
    QMAKE_BUNDLE_DATA += MAC_ICONS_RESOURCES
    ICON = app.icns

    QMAKE_INFO_PLIST = Info_MEGA.plist

    QMAKE_CXXFLAGS += -fvisibility=hidden -fvisibility-inlines-hidden
    QMAKE_LFLAGS += -F /System/Library/Frameworks/Security.framework/
}

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

debug {
    CONFIG += console
    DEFINES += CREATE_COMPATIBLE_MINIDUMPS
}

include(gui/gui.pri)
include(sdk/sdk.pri)
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
    gui/translations/MEGASyncStrings_ar.ts	gui/translations/MEGASyncStrings_ca.ts \
    gui/translations/MEGASyncStrings_cs.ts	gui/translations/MEGASyncStrings_da.ts \
    gui/translations/MEGASyncStrings_de.ts	gui/translations/MEGASyncStrings_ee.ts \
    gui/translations/MEGASyncStrings_es.ts	gui/translations/MEGASyncStrings_fi.ts \
    gui/translations/MEGASyncStrings_fr.ts	gui/translations/MEGASyncStrings_he.ts \
    gui/translations/MEGASyncStrings_hu.ts	gui/translations/MEGASyncStrings_id.ts \
    gui/translations/MEGASyncStrings_it.ts	gui/translations/MEGASyncStrings_ja.ts \
    gui/translations/MEGASyncStrings_ko.ts	gui/translations/MEGASyncStrings_ms.ts \
    gui/translations/MEGASyncStrings_nl.ts	gui/translations/MEGASyncStrings_no.ts \
    gui/translations/MEGASyncStrings_pl.ts	gui/translations/MEGASyncStrings_pt.ts \
    gui/translations/MEGASyncStrings_pt_BR.ts	gui/translations/MEGASyncStrings_ro.ts \
    gui/translations/MEGASyncStrings_ru.ts	gui/translations/MEGASyncStrings_sk.ts \
    gui/translations/MEGASyncStrings_sv.ts	gui/translations/MEGASyncStrings_th.ts \
    gui/translations/MEGASyncStrings_tr.ts	gui/translations/MEGASyncStrings_uk.ts \
    gui/translations/MEGASyncStrings_zh_CN.ts	gui/translations/MEGASyncStrings_zh_TW.ts \
    gui/translations/MEGASyncStrings_af.ts	gui/translations/MEGASyncStrings_bg.ts \
    gui/translations/MEGASyncStrings_bs.ts	gui/translations/MEGASyncStrings_cy.ts \
    gui/translations/MEGASyncStrings_el.ts	gui/translations/MEGASyncStrings_eu.ts \
    gui/translations/MEGASyncStrings_hi.ts	gui/translations/MEGASyncStrings_hr.ts \
    gui/translations/MEGASyncStrings_ka.ts	gui/translations/MEGASyncStrings_lv.ts \
    gui/translations/MEGASyncStrings_lt.ts	gui/translations/MEGASyncStrings_mi.ts \
    gui/translations/MEGASyncStrings_mk.ts	gui/translations/MEGASyncStrings_sl.ts \
    gui/translations/MEGASyncStrings_tl.ts	gui/translations/MEGASyncStrings_vn.ts

CODECFORTR = UTF8

win32 {
    RC_FILE = icon.rc
    QMAKE_LFLAGS += /LARGEADDRESSAWARE
}


macx {
    QMAKE_CXXFLAGS += -DCRYPTOPP_DISABLE_ASM
    MAC_ICONS_RESOURCES.files = folder.icns appicon32.png
    MAC_ICONS_RESOURCES.path = Contents/Resources
    QMAKE_BUNDLE_DATA += MAC_ICONS_RESOURCES
    ICON = app.icns

    QMAKE_CXXFLAGS += -fvisibility=hidden -fvisibility-inlines-hidden
    QMAKE_LFLAGS += -F /System/Library/Frameworks/Security.framework/
}

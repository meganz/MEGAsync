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

QT += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MEGAupdater
TEMPLATE = app

SOURCES += ../MEGAsync/sdk/src/crypto/cryptopp.cpp ../MEGAsync/sdk/src/base64.cpp ../MEGAsync/sdk/src/utils.cpp
LIBS += -lws2_32 -lcryptopp
release {
    LIBS += -L"$$_PRO_FILE_PWD_/../MEGAsync/sdk/3rdparty/libs/static"
}
else {
    LIBS += -L"$$_PRO_FILE_PWD_/../MEGAsync/sdk/3rdparty/libs/staticd"
}

DEFINES += USE_CRYPTOPP
DEPENDPATH += $$PWD
INCLUDEPATH += $$PWD ../MEGASync ../MEGAsync/sdk/3rdparty/include ../MEGAsync/sdk/include/ ../MEGAsync/sdk/include/mega/win32

DEFINES += QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII

HEADERS += \
    MegaUpdater.h \
    UpdaterGUI.h

SOURCES += \
    MegaUpdater.cpp \
    UpdaterGUI.cpp

FORMS += \
    UpdaterGUI.ui


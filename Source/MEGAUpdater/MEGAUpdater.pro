CONFIG(debug, debug|release) {
    CONFIG -= debug release
    CONFIG += debug
}
CONFIG(release, debug|release) {
    CONFIG -= debug release
    CONFIG += release
}

CONFIG -= qt

TARGET = MEGAupdater
TEMPLATE = app
CONFIG += console

SOURCES += ../MEGASync/sdk/sdk/src/crypto/cryptopp.cpp ../MEGASync/sdk/sdk/src/base64.cpp ../MEGASync/sdk/sdk/src/utils.cpp
LIBS += -lcryptopp

win32 {
    release {
        LIBS += -L"$$_PRO_FILE_PWD_/../MEGAsync/sdk/3rdparty/libs/static"
    }
    else {
        LIBS += -L"$$_PRO_FILE_PWD_/../MEGAsync/sdk/3rdparty/libs/staticd"
    }

    LIBS += -lws2_32
}

DEFINES += USE_CRYPTOPP
DEPENDPATH += $$PWD
INCLUDEPATH += $$PWD ../MEGASync/sdk/3rdparty/include ../MEGASync/sdk/3rdparty/include/cryptopp ../MEGASync/sdk/include/ ../MEGASync/sdk/sdk/include/

win32 {
    INCLUDEPATH += ../MEGASync/sdk/sdk/include/mega/win32
}

unix:!macx {
    INCLUDEPATH += ../MEGASync/sdk/sdk/include/mega/posix
}

SOURCES += MegaUpdater.cpp

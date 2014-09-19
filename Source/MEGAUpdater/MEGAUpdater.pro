CONFIG(debug, debug|release) {
    CONFIG -= debug release
    CONFIG += debug
}
CONFIG(release, debug|release) {
    CONFIG -= debug release
    CONFIG += release
}

TARGET = MEGAupdater
TEMPLATE = app
CONFIG += console

SOURCES += ../MEGASync/mega/src/crypto/cryptopp.cpp ../MEGASync/mega/src/base64.cpp ../MEGASync/mega/src/utils.cpp
LIBS += -lcryptopp

win32 {
    release {
        LIBS += -L"$$_PRO_FILE_PWD_/../MEGAsync/mega/bindings/qt/3rdparty/libs/static"
    }
    else {
        LIBS += -L"$$_PRO_FILE_PWD_/../MEGAsync/mega/bindings/qt/3rdparty/libs/staticd"
    }

    INCLUDEPATH += $$[QT_INSTALL_PREFIX]/src/3rdparty/zlib
    LIBS += -lws2_32
}

macx {
   LIBS += -L$$_PRO_FILE_PWD_/../MEGAsync/mega/bindings/qt/3rdparty/libs
}

DEFINES += USE_CRYPTOPP
DEPENDPATH += $$PWD
INCLUDEPATH += $$PWD ../MEGASync/mega/bindings/qt/3rdparty/include ../MEGASync/mega/bindings/qt/3rdparty/include/cryptopp ../MEGASync/mega/include/

win32 {
    INCLUDEPATH += ../MEGASync/mega/include/mega/win32
}

unix {
    INCLUDEPATH += ../MEGASync/mega/include/mega/posix
}

SOURCES += MegaUpdater.cpp

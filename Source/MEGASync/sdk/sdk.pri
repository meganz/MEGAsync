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

SOURCES += sdk/megaapi.cpp \
    sdk/crypto/cryptopp.cpp \
    sdk/megaclient.cpp \
    sdk/win32/fs.cpp \
    sdk/win32/net.cpp \
    sdk/win32/megaapiwait.cpp \
    sdk/db/sqlite.cpp \
    sdk/sqlite3.c

HEADERS  += sdk/megaapi.h \
    sdk/crypto/cryptopp.h \
    sdk/megaclient.h \
    sdk/win32/fs.h \
    sdk/win32/net.h \
    sdk/win32/megaapiwait.h

LIBS += -L"$$_PRO_FILE_PWD_/sdk/3rdparty/libs"

debug {
    LIBS += -L"$$_PRO_FILE_PWD_/sdk/3rdparty/libs/staticd"
}
else {
    LIBS += -L"$$_PRO_FILE_PWD_/sdk/3rdparty/libs/static"
}

DEFINES += USE_SQLITE USE_CRYPTOPP
LIBS += -lcryptopp -lpthread -lFreeImage

win32 {
    INCLUDEPATH += $$PWD
    INCLUDEPATH += $$PWD/3rdparty/include
    INCLUDEPATH += $$PWD/3rdparty/include/cryptopp
    INCLUDEPATH += $$PWD/3rdparty/include/db
    INCLUDEPATH += $$PWD/3rdparty/include/pthread

    LIBS += winhttp.lib ws2_32.lib
}

unix {
   INCLUDEPATH += /usr/include/cryptopp

   LIBS += -lssl -lcrypto -lcurl
}


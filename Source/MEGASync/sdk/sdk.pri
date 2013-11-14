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
    sdk/megacrypto.cpp \
    sdk/megaclient.cpp \
    sdk/megawin32.cpp \
    sdk/megasqlitedb.cpp

HEADERS  += sdk/megaapi.h \
    sdk/megacrypto.h \
    sdk/megaclient.h \
    sdk/megabdb.h \
    sdk/mega.h \
    sdk/megawin32.h \
    sdk/megasqlitedb.h

LIBS += -L"$$_PRO_FILE_PWD_/sdk/3rdparty/libs"

debug {
    LIBS += -L"$$_PRO_FILE_PWD_/sdk/3rdparty/libs/staticd"
}
else {
    LIBS += -L"$$_PRO_FILE_PWD_/sdk/3rdparty/libs/static"
}


LIBS += -lcryptopp -ldb_cxx -lpthread

win32 {
    INCLUDEPATH += $$PWD/3rdparty/include/cryptopp
    INCLUDEPATH += $$PWD/3rdparty/include/db
    INCLUDEPATH += $$PWD/3rdparty/include/pthread

    LIBS += winhttp.lib ws2_32.lib
}

unix {
   INCLUDEPATH += /usr/include/cryptopp

   LIBS += -lssl -lcrypto -lcurl
}


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

SOURCES += $$PWD/src/attrmap.cpp \
    $$PWD/src/backofftimer.cpp \
    $$PWD/src/base64.cpp \
    $$PWD/src/command.cpp \
    $$PWD/src/commands.cpp \
    $$PWD/src/db.cpp \
    $$PWD/src/file.cpp \
    $$PWD/src/fileattributefetch.cpp \
    $$PWD/src/filefingerprint.cpp \
    $$PWD/src/filesystem.cpp \
    $$PWD/src/http.cpp \
    $$PWD/src/json.cpp \
    $$PWD/src/megaclient.cpp \
    $$PWD/src/node.cpp \
    $$PWD/src/pubkeyaction.cpp \
    $$PWD/src/request.cpp \
    $$PWD/src/serialize64.cpp \
    $$PWD/src/share.cpp \
    $$PWD/src/sharenodekeys.cpp \
    $$PWD/src/sync.cpp \
    $$PWD/src/transfer.cpp \
    $$PWD/src/transferslot.cpp \
    $$PWD/src/treeproc.cpp \
    $$PWD/src/user.cpp \
    $$PWD/src/utils.cpp \
    $$PWD/src/waiter.cpp  \
    $$PWD/src/synclocalops.cpp  \
    $$PWD/src/crypto/cryptopp.cpp  \
    $$PWD/src/db/sqlite.cpp  \
    $$PWD/src/win32/net.cpp  \
    $$PWD/src/win32/fs.cpp  \
    $$PWD/win32/megaapiwait.cpp  \
    $$PWD/megaapi.cpp \
    $$PWD/sqlite3.c \
    $$PWD/qt/QTMegaRequestListener.cpp \
    sdk/qt/QTMegaTransferListener.cpp \
    sdk/qt/QTMegaListener.cpp

HEADERS  += $$PWD/include/mega.h \
	    $$PWD/include/mega/account.h \
	    $$PWD/include/mega/attrmap.h \
	    $$PWD/include/mega/backofftimer.h \
	    $$PWD/include/mega/base64.h \
	    $$PWD/include/mega/command.h \
	    $$PWD/include/mega/commands.h \
	    $$PWD/include/mega/console.h \
	    $$PWD/include/mega/db.h \
	    $$PWD/include/mega/file.h \
	    $$PWD/include/mega/fileattributefetch.h \
	    $$PWD/include/mega/filefingerprint.h \
	    $$PWD/include/mega/filesystem.h \
	    $$PWD/include/mega/http.h \
	    $$PWD/include/mega/json.h \
	    $$PWD/include/mega/megaapp.h \
	    $$PWD/include/mega/megaclient.h \
	    $$PWD/include/mega/node.h \
	    $$PWD/include/mega/pubkeyaction.h \
	    $$PWD/include/mega/request.h \
	    $$PWD/include/mega/serialize64.h \
	    $$PWD/include/mega/share.h \
	    $$PWD/include/mega/sharenodekeys.h \
	    $$PWD/include/mega/sync.h \
	    $$PWD/include/mega/transfer.h \
	    $$PWD/include/mega/transferslot.h \
	    $$PWD/include/mega/treeproc.h \
	    $$PWD/include/mega/types.h \
	    $$PWD/include/mega/user.h \
	    $$PWD/include/mega/utils.h \
	    $$PWD/include/mega/waiter.h \
	    $$PWD/include/mega/synclocalops.cpp  \
	    $$PWD/include/mega/crypto/cryptopp.h  \
	    $$PWD/include/mega/db/sqlite.h  \
	    $$PWD/include/mega/win32/meganet.h  \
	    $$PWD/include/mega/win32/megasys.h  \
	    $$PWD/include/mega/win32/megafs.h  \
	    $$PWD/win32/megaapiwait.h  \
	    $$PWD/megaapi.h \
	    $$PWD/qt/QTMegaRequestListener.h \
    sdk/qt/QTMegaTransferListener.h \
    sdk/qt/QTMegaListener.h

LIBS += -L"$$_PRO_FILE_PWD_/sdk/3rdparty/libs"

debug {
    LIBS += -L"$$_PRO_FILE_PWD_/sdk/3rdparty/libs/staticd"
    DEFINES += SQLITE_DEBUG
}
else {
    LIBS += -L"$$_PRO_FILE_PWD_/sdk/3rdparty/libs/static"
}

DEFINES += USE_SQLITE USE_CRYPTOPP USE_QT
LIBS += -lcryptopp -lpthread
INCLUDEPATH += $$PWD/include

win32 {
    INCLUDEPATH += $$PWD
    INCLUDEPATH += $$PWD/include/mega/win32
    INCLUDEPATH += $$PWD/3rdparty/include
    INCLUDEPATH += $$PWD/3rdparty/include/cryptopp
    INCLUDEPATH += $$PWD/3rdparty/include/db
    INCLUDEPATH += $$PWD/3rdparty/include/pthread

    LIBS += -lwinhttp -lws2_32
}

unix {
   INCLUDEPATH += /usr/include/cryptopp

   LIBS += -lssl -lcrypto -lcurl
}


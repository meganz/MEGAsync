CONFIG -= qt
MEGASDK_BASE_PATH = $$PWD/../MEGASync/mega

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

HEADERS += UpdateTask.h \
    Preferences.h \
    MacUtils.h

SOURCES += MEGAUpdater.cpp \
    UpdateTask.cpp

INCLUDEPATH += $$MEGASDK_BASE_PATH/bindings/qt/3rdparty/include

macx {    
    OBJECTIVE_SOURCES +=  MacUtils.mm
    DEFINES += _DARWIN_FEATURE_64_BIT_INODE USE_OPENSSL CRYPTOPP_DISABLE_ASM
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.6
    QMAKE_CXXFLAGS -= -stdlib=libc++
    QMAKE_LFLAGS -= -stdlib=libc++
    CONFIG -= c++11
    LIBS += -framework Cocoa -framework SystemConfiguration -framework CoreFoundation -framework Foundation -framework Security
    QMAKE_CXXFLAGS += -g
    LIBS += -lcryptopp
}

win32 {
    contains(CONFIG, BUILDX64) {
       release {
            LIBS += -L"$$MEGASDK_BASE_PATH/bindings/qt/3rdparty/libs/x64"
        }
        else {
            LIBS += -L"$$MEGASDK_BASE_PATH/bindings/qt/3rdparty/libs/x64d"
        }
    }

    !contains(CONFIG, BUILDX64) {
        release {
            LIBS += -L"$$MEGASDK_BASE_PATH/bindings/qt/3rdparty/libs/x32"
        }
        else {
            LIBS += -L"$$MEGASDK_BASE_PATH/bindings/qt/3rdparty/libs/x32d"
        }
    }

    DEFINES += UNICODE _UNICODE NTDDI_VERSION=0x05010000 _WIN32_WINNT=0x0501
    LIBS += -lurlmon -lShlwapi -lShell32 -lcryptoppmt

    QMAKE_CXXFLAGS_RELEASE += -MT
    QMAKE_CXXFLAGS_DEBUG += -MTd

    QMAKE_CXXFLAGS_RELEASE -= -MD
    QMAKE_CXXFLAGS_DEBUG -= -MDd
}

DEPENDPATH += $$PWD
INCLUDEPATH += $$PWD

QT       += network

SOURCES += utils/HTTPServer.cpp utils/Preferences.cpp \
    utils/FileDownloader.cpp

win32 {
    SOURCES += utils/WindowsUtils.cpp utils/win32/PipeDispatcher.cpp
}

HEADERS  +=  utils/HTTPServer.h utils/Preferences.h \
    utils/FileDownloader.h

win32 {
    HEADERS  += utils/WindowsUtils.h utils/win32/PipeDispatcher.h
}

win32 {
    LIBS += -lole32 -lShell32
}

DEPENDPATH += $$PWD
INCLUDEPATH += $$PWD

QT       += network

SOURCES += utils/HTTPServer.cpp utils/Preferences.cpp \
    utils/FileDownloader.cpp

win32 {
    SOURCES += utils/WindowsUtils.cpp
}

HEADERS  +=  utils/HTTPServer.h utils/Preferences.h \
    utils/FileDownloader.h

win32 {
    HEADERS  += utils/WindowsUtils.h
}

win32 {
    LIBS += -lole32 -lShell32
}

DEPENDPATH += $$PWD
INCLUDEPATH += $$PWD

QT       += network

SOURCES += utils/HTTPServer.cpp utils/Preferences.cpp \
    utils/FileDownloader.cpp \
    utils/LinkProcessor.cpp \
    utils/MegaUploader.cpp \
    utils/CommonUtils.cpp \
    utils/win32/TrayNotificationReceiver.cpp

win32 {
    SOURCES += utils/win32/WindowsUtils.cpp utils/win32/WindowsShellDispatcher.cpp
}

HEADERS  +=  utils/HTTPServer.h utils/Preferences.h \
    utils/FileDownloader.h \
    utils/LinkProcessor.h \
    utils/MegaUploader.h \
    utils/CommonUtils.h \
    utils/Utils.h \
    utils/win32/TrayNotificationReceiver.h

win32 {
    HEADERS  += utils/win32/WindowsUtils.h utils/win32/WindowsShellDispatcher.h
}

win32 {
    LIBS += -lole32 -lShell32
}

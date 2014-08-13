DEPENDPATH += $$PWD
INCLUDEPATH += $$PWD

QT       += network

SOURCES += $$PWD/HTTPServer.cpp \
    $$PWD/Preferences.cpp \
    $$PWD/LinkProcessor.cpp \
    $$PWD/MegaUploader.cpp \
    $$PWD/UpdateTask.cpp \
    $$PWD/EncryptedSettings.cpp \
    $$PWD/CrashHandler.cpp \
    $$PWD/ExportProcessor.cpp \
    $$PWD/Utilities.cpp \
    $$PWD/QTMegaApi.cpp

HEADERS  +=  $$PWD/HTTPServer.h \
    $$PWD/Preferences.h \
    $$PWD/LinkProcessor.h \
    $$PWD/MegaUploader.h \
    $$PWD/UpdateTask.h \
    $$PWD/EncryptedSettings.h \
    $$PWD/CrashHandler.h \
    $$PWD/ExportProcessor.h \
    $$PWD/Utilities.h \
    $$PWD/QTMegaApi.h

debug {
    DEFINES += SHOW_LOGS
#    DEFINES += LOG_TO_FILE
}

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
    $$PWD/AsyncFileCopy.cpp \
    $$PWD/Utilities.cpp

HEADERS  +=  $$PWD/HTTPServer.h \
    $$PWD/Preferences.h \
    $$PWD/LinkProcessor.h \
    $$PWD/MegaUploader.h \
    $$PWD/UpdateTask.h \
    $$PWD/EncryptedSettings.h \
    $$PWD/CrashHandler.h \
    $$PWD/ExportProcessor.h \
    $$PWD/AsyncFileCopy.h \
    $$PWD/Utilities.h


    DEFINES += SHOW_LOGS


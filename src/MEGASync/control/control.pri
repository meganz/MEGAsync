DEPENDPATH += $$PWD
INCLUDEPATH += $$PWD

QT       += network

SOURCES += $$PWD/HTTPServer.cpp \
    $$PWD/Preferences.cpp \
    $$PWD/LinkProcessor.cpp \
    $$PWD/MegaUploader.cpp \
    $$PWD/TransferRemainingTime.cpp \
    $$PWD/UpdateTask.cpp \
    $$PWD/EncryptedSettings.cpp \
    $$PWD/CrashHandler.cpp \
    $$PWD/ExportProcessor.cpp \
    $$PWD/UserAttributesManager.cpp \
    $$PWD/Utilities.cpp \
    $$PWD/ThreadPool.cpp \
    $$PWD/MegaDownloader.cpp \
    $$PWD/MegaController.cpp \
    $$PWD/MegaSyncLogger.cpp \
    $$PWD/ConnectivityChecker.cpp \
    $$PWD/qrcodegen.c

HEADERS  +=  $$PWD/HTTPServer.h \
    $$PWD/AppStatsEvents.h \
    $$PWD/Preferences.h \
    $$PWD/LinkProcessor.h \
    $$PWD/MegaUploader.h \
    $$PWD/TransferRemainingTime.h \
    $$PWD/UpdateTask.h \
    $$PWD/EncryptedSettings.h \
    $$PWD/CrashHandler.h \
    $$PWD/ExportProcessor.h \
    $$PWD/UserAttributesManager.h \
    $$PWD/Utilities.h \
    $$PWD/ThreadPool.h \
    $$PWD/MegaDownloader.h \
    $$PWD/MegaController.h \
    $$PWD/MegaSyncLogger.h \
    $$PWD/ConnectivityChecker.h \
    $$PWD/qrcodegen.h \
    $$PWD/gzjoin.h

macx {
    OBJECTIVE_SOURCES += $$PWD/macx/OSUtilities.mm
}

unix:!macx {
    SOURCES += $$PWD/linux/OSUtilities.cpp
}

win32 {
    SOURCES += $$PWD/win/OSUtilities.cpp
}


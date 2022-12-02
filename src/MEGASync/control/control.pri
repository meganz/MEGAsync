DEPENDPATH += $$PWD
INCLUDEPATH += $$PWD

QT       += network

SOURCES += $$PWD/HTTPServer.cpp \
    $$PWD/DialogOpener.cpp \
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
    $$PWD/MegaSyncLogger.cpp \
    $$PWD/ConnectivityChecker.cpp \
    $$PWD/TransferBatch.cpp \
    $$PWD/TextDecorator.cpp \
    $$PWD/qrcodegen.c \

HEADERS  +=  $$PWD/HTTPServer.h \
    $$PWD/AppStatsEvents.h \
    $$PWD/DialogOpener.h \
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
    $$PWD/MegaSyncLogger.h \
    $$PWD/ConnectivityChecker.h \
    $$PWD/TransferBatch.h \
    $$PWD/TextDecorator.h \
    $$PWD/qrcodegen.h \
    $$PWD/gzjoin.h

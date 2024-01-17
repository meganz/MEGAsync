DEPENDPATH += $$PWD
INCLUDEPATH += $$PWD

QT       += network

SOURCES += $$PWD/HTTPServer.cpp \
    $$PWD/AccountStatusController.cpp \
    $$PWD/DialogOpener.cpp \
    $$PWD/DownloadQueueController.cpp \
    $$PWD/FileFolderAttributes.cpp \
    $$PWD/LoginController.cpp \
    $$PWD/Preferences/Preferences.cpp \
    $$PWD/Preferences/EphemeralCredentials.cpp \
    $$PWD/Preferences/EncryptedSettings.cpp \
    $$PWD/LinkProcessor.cpp \
    $$PWD/MegaUploader.cpp \
    $$PWD/TransferRemainingTime.cpp \
    $$PWD/UpdateTask.cpp \
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
    $$PWD/qrcodegen.c

HEADERS  +=  $$PWD/HTTPServer.h \
    $$PWD/AccountStatusController.h \
    $$PWD/AppStatsEvents.h \
    $$PWD/DialogOpener.h \
    $$PWD/FileFolderAttributes.h \
    $$PWD/DownloadQueueController.h \
    $$PWD/LoginController.h \
    $$PWD/Preferences/Preferences.h \
    $$PWD/Preferences/EphemeralCredentials.h \
    $$PWD/Preferences/EncryptedSettings.h \
    $$PWD/FileFolderAttributes.h \
    $$PWD/LinkProcessor.h \
    $$PWD/MegaUploader.h \
    $$PWD/TransferRemainingTime.h \
    $$PWD/UpdateTask.h \
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
    $$PWD/Version.h \
    $$PWD/qrcodegen.h \
    $$PWD/gzjoin.h

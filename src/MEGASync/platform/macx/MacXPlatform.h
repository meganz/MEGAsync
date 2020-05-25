#ifndef MACXPLATFORM_H
#define MACXPLATFORM_H

#include "MacXFunctions.h"
#include "MacXSystemServiceTask.h"
#include "MacXExtServer.h"

#include <QApplication>
#include <QString>
#include <QFile>
#include <QHash>
#include <QPixmap>
#include <QThread>
#include <QDir>
#include <QProcess>
#include <QThread>

class MacXPlatform
{

private:
    MacXPlatform() {}
    static bool enableSetuidBit();
    static MacXSystemServiceTask *systemServiceTask;
    static MacXExtServer *extServer;
    static std::unique_ptr<QThread> threadExtServer;

public:
    static void initialize(int argc, char *argv[]);
    static void prepareForSync();
    static QStringList multipleUpload(QString uploadTitle);
    static bool enableTrayIcon(QString executable);
    static void notifyItemChange(std::string *localPath, int newState);
    static bool startOnStartup(bool value);
    static bool isStartOnStartupActive();
    static void addFinderExtensionToSystem();
    static bool isFinderExtensionEnabled();
    static void reinstallFinderExtension();
    static void reloadFinderExtension();
    static void enableFinderExtension(bool value);
    static void showInFolder(QString pathIn);
    static void startShellDispatcher(MegaApplication *receiver);
    static void stopShellDispatcher();
    static void syncFolderAdded(QString syncPath, QString syncName, QString syncID);
    static void syncFolderRemoved(QString syncPath, QString syncName, QString syncID);
    static void notifyRestartSyncFolders();
    static void notifyAllSyncFoldersAdded();
    static void notifyAllSyncFoldersRemoved();
    static QByteArray encrypt(QByteArray data, QByteArray key);
    static QByteArray decrypt(QByteArray data, QByteArray key);
    static QByteArray getLocalStorageKey();
    static QString getDefaultOpenApp(QString extension);
    static void enableDialogBlur(QDialog *dialog);
    static void activateBackgroundWindow(QDialog *window);
    static bool registerUpdateJob();
    static void execBackgroundWindow(QDialog *window);
    static void uninstall();
    static bool shouldRunHttpServer();
    static bool shouldRunHttpsServer();
    static bool isUserActive();
    static double getUpTime();

    static int fd;

private:
    static void disableSignalHandler();
};

#endif // MACXPLATFORM_H

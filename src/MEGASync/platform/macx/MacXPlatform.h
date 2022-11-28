#ifndef MACXPLATFORM_H
#define MACXPLATFORM_H

#include "MacXFunctions.h"
#include "MacXSystemServiceTask.h"
#include "MacXExtServerService.h"
#include "ShellNotifier.h"

#include <QApplication>
#include <QString>
#include <QFile>
#include <QHash>
#include <QPixmap>
#include <QThread>
#include <QDir>
#include <QProcess>

class MacXPlatform
{

private:
    MacXPlatform() {}
    static MacXSystemServiceTask *systemServiceTask;
    static QPointer<MacXExtServerService> extService; //Set to NULL upon deletion

public:
    static void initialize(int argc, char *argv[]);
    static void prepareForSync();
    static QStringList multipleUpload(QString uploadTitle);
    static bool enableTrayIcon(QString);
    void notifyItemChange(const QString& path, int newState);
    static void notifyItemChange(std::string *localPath, int newState);
    static bool startOnStartup(bool value);
    static bool isStartOnStartupActive();
    static void addFinderExtensionToSystem();
    static bool isFinderExtensionEnabled();
    static void reinstallFinderExtension();
    static void reloadFinderExtension();
    static void enableFinderExtension(bool value);
    static bool showInFolder(QString pathIn);
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
    static bool registerUpdateJob();
    static void execBackgroundWindow(QDialog *window);
    static void showBackgroundWindow(QDialog *window);
    static void uninstall();
    static bool shouldRunHttpServer();
    static bool shouldRunHttpsServer();
    static bool isUserActive();
    static double getUpTime();
    static QString getDeviceName();
    static void initMenu(QMenu* m);
    static std::shared_ptr<AbstractShellNotifier> getShellNotifier();

    static const char* settingsString;
    static const char* exitString;
    static const char* fileExplorerString;
    static std::shared_ptr<AbstractShellNotifier> mShellNotifier;

private:
    static void disableSignalHandler();

    static void notifySyncFileChange(std::string *localPath, int newState);

};

#endif // MACXPLATFORM_H

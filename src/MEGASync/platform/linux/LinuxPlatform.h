#ifndef LINUXPLATFORM_H
#define LINUXPLATFORM_H

#include <QApplication>
#include <QString>
#include <QFile>
#include <QHash>
#include <QPixmap>
#include <QThread>
#include <QDir>
#include <QProcess>
#include <QMenu>

#include "MegaApplication.h"
#include "ExtServer.h"
#include "NotifyServer.h"

class LinuxPlatform
{

private:
    static ExtServer *ext_server;
    static NotifyServer *notify_server;
    static QString set_icon;
    static QString custom_icon;
    static QString remove_icon;

    LinuxPlatform() {}
    static QStringList getListRunningProcesses();

public:
    static void initialize(int argc, char *argv[]);
    static void prepareForSync();
    static QString desktop_file;
    static bool enableTrayIcon(QString executable);
    static void notifyItemChange(std::string *localPath, int newState);
    static bool startOnStartup(bool value);
    static bool isStartOnStartupActive();
    static bool isTilingWindowManager();
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
    static QString getDefaultFileBrowserApp();
    static QString getDefaultOpenApp(QString extension);
    static QString getDefaultOpenAppByMimeType(QString mimeType);
    static bool getValue(const char * const name, const bool default_value);
    static std::string getValue(const char * const name, const std::string &default_value);
    static QString getWindowManagerName();
    static void enableDialogBlur(QDialog *dialog);
    static void execBackgroundWindow(QDialog *window);
    static void showBackgroundWindow(QDialog *window);
    static bool registerUpdateJob();
    static void uninstall();
    static bool shouldRunHttpServer();
    static bool shouldRunHttpsServer();
    static bool isUserActive();
    static QString getDeviceName();
    static void initMenu(QMenu* m);

    static const char* settingsString;
    static const char* exitString;
    static const char* fileExplorerString;
};

#endif // LINUXPLATFORM_H

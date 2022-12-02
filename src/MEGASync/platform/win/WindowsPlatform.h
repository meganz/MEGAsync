#ifndef WINDOWSPLATFORM_H
#define WINDOWSPLATFORM_H

#include "platform/ShellNotifier.h"
#include "platform/win/WinShellDispatcherTask.h"
#include "platform/win/WinTrayReceiver.h"

#include <QApplication>
#include <QString>
#include <QFile>
#include <QHash>
#include <QPixmap>
#include <QThread>
#include <QDir>
#include <QProcess>
#include <QMenu>

#include <queue>

class WindowsPlatform
{

private:
    WindowsPlatform() {}

    static HRESULT CreateLink(LPCWSTR lpszPathObj, LPCWSTR lpszPathLink, LPCWSTR lpszDesc, LPCWSTR pszIconfile=NULL, int iIconindex=0);
    static LPTSTR getCurrentSid();
    static WinShellDispatcherTask *shellDispatcherTask;

public:
    static void addSyncToLeftPane(QString syncPath, QString syncName, QString uuid);
    static void removeSyncFromLeftPane(QString syncPath, QString syncName, QString uuid);
    static void removeAllSyncsFromLeftPane();
    static bool makePubliclyReadable(LPTSTR fileName);

    static void initialize(int argc, char *argv[]);
    static void prepareForSync();
    static bool enableTrayIcon(QString executable);
    static void notifyItemChange(const QString& path, int newState);
    static void notifySyncFileChange(std::string *localPath, int newState);
    static bool startOnStartup(bool value);
    static bool isStartOnStartupActive();
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
    static void uninstall();
    static bool shouldRunHttpServer();
    static bool shouldRunHttpsServer();
    static bool isUserActive();
    static QString getDeviceName();
    static void initMenu(QMenu* m);
    static std::shared_ptr<AbstractShellNotifier> getShellNotifier();

    static const char* settingsString;
    static const char* exitString;
    static const char* fileExplorerString;

private:
    static void notifyItemChange(const QString &localPath, AbstractShellNotifier* notifier);
    static QString getPreparedPath(const QString& localPath);

    static std::shared_ptr<AbstractShellNotifier> mSyncFileNotifier;
    static std::shared_ptr<AbstractShellNotifier> mGeneralNotifier;
};

#endif // WINDOWSPLATFORM_H

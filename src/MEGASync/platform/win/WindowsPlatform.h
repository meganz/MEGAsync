#ifndef WINDOWSPLATFORM_H
#define WINDOWSPLATFORM_H

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
    static void notifyItemChange(std::string *localPath, int newState, std::shared_ptr<ShellNotifier> notifier = nullptr);
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
    static void execBackgroundWindow(QDialog *window);
    static void showBackgroundWindow(QDialog* window);
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


class ShellNotifier
{
public:
    ~ShellNotifier();

    void enqueueItemChange(std::string&& localPath);

private:
    void doInThread();

    void notify(const std::string& path) const; // called from secondary thread context

    void checkReportQueueSize();

    std::thread mThread;
    std::queue<std::string> mPendingNotifications;

    size_t lastReportedQueueSize = 0;

    std::mutex mQueueAccessMutex;
    std::condition_variable mWaitCondition;
    bool mExit = false;
};

#endif // WINDOWSPLATFORM_H

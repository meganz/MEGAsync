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

class WindowsPlatform
{

private:
    WindowsPlatform() {}

    static HRESULT CreateLink(LPCWSTR lpszPathObj, LPCWSTR lpszPathLink, LPCWSTR lpszDesc, LPCWSTR pszIconfile=NULL, int iIconindex=0);
    static void countFilesAndFolders(QString path, long *numFiles, long *numFolders, long fileLimit, long folderLimit);
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
    static void notifyItemChange(std::string *localPath, int newState);
    static bool startOnStartup(bool value);
    static bool isStartOnStartupActive();
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
};

#endif // WINDOWSPLATFORM_H

#ifndef WINDOWSUTILS_H
#define WINDOWSUTILS_H

#include "utils/win32/ShellDispatcherTask.h"
#include "utils/win32/TrayNotificationReceiver.h"

#include <QApplication>
#include <QString>
#include <QFile>
#include <QHash>
#include <QPixmap>
#include <QThread>
#include <QDir>
#include <QProcess>

class ShellDispatcherTask;
class WindowsUtils
{

private:
    WindowsUtils() {}

    static HRESULT CreateLink(LPCWSTR lpszPathObj, LPCWSTR lpszPathLink, LPCWSTR lpszDesc, LPCWSTR pszIconfile=NULL, int iIconindex=0);
    static void countFilesAndFolders(QString path, long *numFiles, long *numFolders, long fileLimit, long folderLimit);
    static ShellDispatcherTask *shellDispatcherTask;
    static QThread shellDispatcherThread;

public:

    static boolean enableTrayIcon(QString executable);
    static void notifyItemChange(QString path);
    static bool startOnStartup(bool value);
    static bool isStartOnStartupActive();
    static void showInFolder(QString pathIn);
    static void startShellDispatcher(MegaApplication *receiver);
    static void stopShellDispatcher();
    static void syncFolderAdded(QString syncPath);
    static void syncFolderRemoved(QString syncPath);
    static QByteArray encrypt(QByteArray data, QByteArray key);
    static QByteArray decrypt(QByteArray data, QByteArray key);
    static QByteArray getLocalStorageKey();
};

#endif // WINDOWSUTILS_H

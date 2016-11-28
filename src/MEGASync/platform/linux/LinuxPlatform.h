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
public:
    static void initialize(int argc, char *argv[]);
    static QString desktop_file;
    static bool enableTrayIcon(QString executable);
    static void notifyItemChange(QString path);
    static bool startOnStartup(bool value);
    static bool isStartOnStartupActive();
    static void showInFolder(QString pathIn);
    static void startShellDispatcher(MegaApplication *receiver);
    static void stopShellDispatcher();
    static void syncFolderAdded(QString syncPath, QString syncName);
    static void syncFolderRemoved(QString syncPath, QString syncName);
    static QByteArray encrypt(QByteArray data, QByteArray key);
    static QByteArray decrypt(QByteArray data, QByteArray key);
    static QByteArray getLocalStorageKey();
    static QString getDefaultOpenApp(QString extension);
    static void enableDialogBlur(QDialog *dialog);
};

#endif // LINUXPLATFORM_H

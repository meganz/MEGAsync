#ifndef MACXPLATFORM_H
#define MACXPLATFORM_H

#include "MacXFunctions.h"
#include "MacXSystemServiceTask.h"

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
    static bool enableSetuidBit();
    static MacXSystemServiceTask *systemServiceTask;

public:
    static void initialize(int argc, char *argv[]);
    static QStringList multipleUpload(QString uploadTitle);
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
    static QString getDefaultOpenApp(QString file);

    static int fd;
};

#endif // MACXPLATFORM_H

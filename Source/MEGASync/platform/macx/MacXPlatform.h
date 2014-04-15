#ifndef MACXPLATFORM_H
#define MACXPLATFORM_H

#include <QApplication>
#include <QString>
#include <QFile>
#include <QHash>
#include <QPixmap>
#include <QThread>
#include <QDir>
#include <QProcess>

#include "MegaApplication.h"
#include "MacXFunctions.h"

class MacXPlatform
{

private:
    MacXPlatform() {}

public:
    static void initialize(int argc, char *argv[]);
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
};

#endif // MACXPLATFORM_H

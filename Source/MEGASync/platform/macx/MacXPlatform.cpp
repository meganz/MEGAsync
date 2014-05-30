#include "MacXPlatform.h"

int MacXPlatform::fd = -1;

void MacXPlatform::initialize(int argc, char *argv[])
{
    setMacXActivationPolicy();

    SetProcessName(QString::fromUtf8("MEGAsync"));

    fd = -1;
    if(argc)
    {
        long int value = strtol(argv[argc-1], NULL, 10);
        if(value > 0 && value < INT_MAX)
            fd = value;
    }

    if(fd < 0)
    {
        if(!enableSetuidBit())
            ::exit(0);

        //Reboot
        QString app = MegaApplication::applicationDirPath();
        QString launchCommand = QString::fromUtf8("open");
        QStringList args = QStringList();
        QDir appPath(app);
        appPath.cdUp();
        appPath.cdUp();
        args.append(QString::fromAscii("-n"));
        args.append(appPath.absolutePath());
        QProcess::startDetached(launchCommand, args);
        sleep(2);
        ::exit(0);
    }
}

bool MacXPlatform::enableTrayIcon(QString executable)
{
    return false;
}

void MacXPlatform::notifyItemChange(QString path)
{

}

bool MacXPlatform::startOnStartup(bool value)
{
   return startAtLogin(value);
}

bool MacXPlatform::isStartOnStartupActive()
{
    return isStartAtLoginActive();
}

void MacXPlatform::showInFolder(QString pathIn)
{
    QStringList scriptArgs;
    scriptArgs << QLatin1String("-e")
               << QString::fromLatin1("tell application \"Finder\" to reveal POSIX file \"%1\"").arg(pathIn);
    QProcess::startDetached(QLatin1String("osascript"), scriptArgs);
    scriptArgs.clear();
    scriptArgs << QLatin1String("-e")
               << QLatin1String("tell application \"Finder\" to activate");
    QProcess::startDetached(QString::fromAscii("osascript"), scriptArgs);
}

void MacXPlatform::startShellDispatcher(MegaApplication *receiver)
{

}

void MacXPlatform::stopShellDispatcher()
{

}

void MacXPlatform::syncFolderAdded(QString syncPath, QString syncName)
{
    addPathToPlaces(syncPath,syncName);
    setFolderIcon(syncPath);
}

void MacXPlatform::syncFolderRemoved(QString syncPath, QString syncName)
{
    removePathFromPlaces(syncPath);
    unSetFolderIcon(syncPath);
}

QByteArray MacXPlatform::encrypt(QByteArray data, QByteArray key)
{
    return data;
}

QByteArray MacXPlatform::decrypt(QByteArray data, QByteArray key)
{
    return data;
}

QByteArray MacXPlatform::getLocalStorageKey()
{
    return QByteArray(128, 0);
}

bool MacXPlatform::enableSetuidBit()
{
    QString command = QString::fromUtf8("chmod 4755 /Applications/MEGAsync.app/Contents/MacOS/MEGAsync && chown root /Applications/MEGAsync.app/Contents/MacOS/MEGAsync && echo true");
    char *response = runWithRootPrivileges((char *)command.toStdString().c_str());
    if(!response) return NULL;

    bool result = strlen(response) >= 4 && !strncmp(response, "true", 4);
    delete response;
    return result;
}

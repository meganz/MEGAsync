#include "MacXPlatform.h"

void MacXPlatform::initialize(int argc, char *argv[])
{
    setMacXActivationPolicy();
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
                       << QString::fromLatin1("tell application \"Finder\" to reveal POSIX file \"%1\"")
                                             .arg(pathIn);
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

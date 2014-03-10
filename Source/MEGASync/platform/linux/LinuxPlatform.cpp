#include "LinuxPlatform.h"

bool LinuxPlatform::enableTrayIcon(QString executable)
{
    return false;
}

void LinuxPlatform::notifyItemChange(QString path)
{

}

bool LinuxPlatform::startOnStartup(bool value)
{
    return false;
}

bool LinuxPlatform::isStartOnStartupActive()
{
    return false;
}

void LinuxPlatform::showInFolder(QString pathIn)
{

}

void LinuxPlatform::startShellDispatcher(MegaApplication *receiver)
{

}

void LinuxPlatform::stopShellDispatcher()
{

}

void LinuxPlatform::syncFolderAdded(QString syncPath, QString syncName)
{

}

void LinuxPlatform::syncFolderRemoved(QString syncPath, QString syncName)
{

}

QByteArray LinuxPlatform::encrypt(QByteArray data, QByteArray key)
{
    return data;
}

QByteArray LinuxPlatform::decrypt(QByteArray data, QByteArray key)
{
    return data;
}

QByteArray LinuxPlatform::getLocalStorageKey()
{
    return QByteArray(128, 0);
}

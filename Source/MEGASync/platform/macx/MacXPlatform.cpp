#include "MacXPlatform.h"

bool MacXPlatform::enableTrayIcon(QString executable)
{
    return false;
}

void MacXPlatform::notifyItemChange(QString path)
{

}

bool MacXPlatform::startOnStartup(bool value)
{
    return false;
}

bool MacXPlatform::isStartOnStartupActive()
{
    return false;
}

void MacXPlatform::showInFolder(QString pathIn)
{

}

void MacXPlatform::startShellDispatcher(MegaApplication *receiver)
{

}

void MacXPlatform::stopShellDispatcher()
{

}

void MacXPlatform::syncFolderAdded(QString syncPath, QString syncName)
{

}

void MacXPlatform::syncFolderRemoved(QString syncPath, QString syncName)
{

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

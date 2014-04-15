#include "LinuxPlatform.h"

ExtServer *LinuxPlatform::ext_server = NULL;

QString LinuxPlatform::desktop_file = QDir::homePath() + QString::fromAscii("/.config/autostart/megasync.desktop");


void LinuxPlatform::initialize(int argc, char *argv[])
{


}

bool LinuxPlatform::enableTrayIcon(QString executable)
{
    return false;
}

void LinuxPlatform::notifyItemChange(QString path)
{
//cout << "notifyItemChange " << path.toStdString().c_str() << endl;
}

bool LinuxPlatform::startOnStartup(bool value)
{
    if (value) {
        if (QFile(desktop_file).exists())
            return true;
        else {
            QString app_desktop = QString::fromAscii("/usr/share/applications/megasync.desktop");
            if (QFile(app_desktop).exists()) {
                QFile::copy(app_desktop, desktop_file);
            } else
                return false;
        }
    } else {
        if (QFile(desktop_file).exists())
            QFile::remove(desktop_file);
    }
    return true;
}

bool LinuxPlatform::isStartOnStartupActive()
{
    return QFile(desktop_file).exists();
}

void LinuxPlatform::showInFolder(QString pathIn)
{
    QProcess::startDetached(QString::fromAscii("nautilus ") + pathIn);
}

void LinuxPlatform::startShellDispatcher(MegaApplication *receiver)
{
    if(ext_server)
        return;
    ext_server = new ExtServer(receiver);
}

void LinuxPlatform::stopShellDispatcher()
{
    if(ext_server)
    {
        delete ext_server;
        ext_server = NULL;
    }
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

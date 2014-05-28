#include "LinuxPlatform.h"

using namespace mega;

ExtServer *LinuxPlatform::ext_server = NULL;
NotifyServer *LinuxPlatform::notify_server = NULL;

static QString autostart_dir = QDir::homePath() + QString::fromAscii("/.config/autostart/");
QString LinuxPlatform::desktop_file = autostart_dir + QString::fromAscii("megasync.desktop");
QString LinuxPlatform::set_icon = QString::fromUtf8("gvfs-set-attribute -t string %1 metadata::custom-icon file://%2");
QString LinuxPlatform::remove_icon = QString::fromUtf8("gvfs-set-attribute -t unset %1 metadata::custom-icon");
QString LinuxPlatform::custom_icon = QString::fromUtf8("/usr/share/icons/hicolor/256x256/apps/mega.png");

void LinuxPlatform::initialize(int argc, char *argv[])
{
}

bool LinuxPlatform::enableTrayIcon(QString executable)
{
    return false;
}

void LinuxPlatform::notifyItemChange(QString path)
{
    if (notify_server && !Preferences::instance()->overlayIconsDisabled())
        notify_server->notifyItemChange(path);
}

// enable or disable MEGASync launching at startup
// return true if operation succeeded
bool LinuxPlatform::startOnStartup(bool value)
{
    // copy desktop file into autostart directory
    if (value) {
        if (QFile(desktop_file).exists())
            return true;
        else {
            // make sure directory exist
            if (!QDir(autostart_dir).exists()) {
                if (!QDir().mkdir(autostart_dir)) {
                    LOG_debug << "Failed to create autostart dir: " << autostart_dir;
                    return false;
                }
            }
            QString app_desktop = QString::fromAscii("/usr/share/applications/megasync.desktop");
            if (QFile(app_desktop).exists()) {
                return QFile::copy(app_desktop, desktop_file);
            } else {
                LOG_debug << "Desktop file does not exist: " << app_desktop;
                return false;
            }
        }
    // remove desktop file if it exists
    } else {
        if (QFile(desktop_file).exists())
            return QFile::remove(desktop_file);
    }
    return true;
}

bool LinuxPlatform::isStartOnStartupActive()
{
    return QFile(desktop_file).exists();
}

void LinuxPlatform::showInFolder(QString pathIn)
{
    QProcess::startDetached(QString::fromAscii("nautilus \"") + pathIn + QString::fromUtf8("\""));
}

void LinuxPlatform::startShellDispatcher(MegaApplication *receiver)
{
    if (!ext_server)
        ext_server = new ExtServer(receiver);
    if (!notify_server)
        notify_server = new NotifyServer();
}

void LinuxPlatform::stopShellDispatcher()
{
    if(ext_server) {
        delete ext_server;
        ext_server = NULL;
    }
    if(notify_server) {
        delete notify_server;
        notify_server = NULL;
    }
}

void LinuxPlatform::syncFolderAdded(QString syncPath, QString syncName)
{
    if(QFile(custom_icon).exists())
        QProcess::startDetached(set_icon.arg(syncPath).arg(custom_icon));
    if (notify_server)
        notify_server->notifySyncAdd(syncPath);
}

void LinuxPlatform::syncFolderRemoved(QString syncPath, QString syncName)
{
    QProcess::startDetached(remove_icon.arg(syncPath));
    if (notify_server)
        notify_server->notifySyncDel(syncPath);
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

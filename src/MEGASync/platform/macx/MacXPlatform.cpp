#include "MacXPlatform.h"
#include <unistd.h>

using namespace std;

MacXSystemServiceTask* MacXPlatform::systemServiceTask = NULL;
QPointer<MacXExtServerService> MacXPlatform::extService;

static const QString kFinderSyncBundleId = QString::fromUtf8("mega.mac.MEGAShellExtFinder");
static const QString kFinderSyncPath = QString::fromUtf8("/Applications/MEGAsync.app/Contents/PlugIns/MEGAShellExtFinder.appex/");

void MacXPlatform::initialize(int /*argc*/, char *[] /*argv*/)
{
    setMacXActivationPolicy();
}

void MacXPlatform::prepareForSync()
{

}

QStringList MacXPlatform::multipleUpload(QString uploadTitle)
{
    return uploadMultipleFiles(uploadTitle);
}

bool MacXPlatform::enableTrayIcon(QString /*executable*/)
{
    return false;
}

bool MacXPlatform::startOnStartup(bool value)
{
   return startAtLogin(value);
}

bool MacXPlatform::isStartOnStartupActive()
{
    return isStartAtLoginActive();
}

void MacXPlatform::addFinderExtensionToSystem()
{
    QStringList scriptArgs;
    scriptArgs << QString::fromUtf8("-a")
               << kFinderSyncPath;

    QProcess::startDetached(QString::fromUtf8("pluginkit"), scriptArgs);
}

bool MacXPlatform::isFinderExtensionEnabled()
{
    QStringList scriptArgs;
    scriptArgs << QString::fromUtf8("-m")
               << QString::fromUtf8("-i")
               << kFinderSyncBundleId;

    QProcess p;
    p.start(QString::fromAscii("pluginkit"), scriptArgs);
    if (!p.waitForFinished(2000))
    {
        return false;
    }

    QString out = QString::fromUtf8(p.readAllStandardOutput().trimmed());
    if (out.isEmpty())
    {
        return false;
    }

    if (out.at(0) != QChar::fromAscii('?') && out.at(0) != QChar::fromAscii('+'))
    {
        return false;
    }

    return true;
}

void MacXPlatform::reinstallFinderExtension()
{
    QStringList scriptArgs;
    scriptArgs << QString::fromUtf8("-r")
               << kFinderSyncPath;

    QProcess::startDetached(QString::fromUtf8("pluginkit"), scriptArgs);
}

void MacXPlatform::reloadFinderExtension()
{
    bool finderExtEnabled = isFinderExtensionEnabled();
    if (!finderExtEnabled) // No need to reload, extension is currenctly disabled and next time user enable it, it will launch updated version
    {
        return;
    }

    QStringList scriptArgs;
    scriptArgs << QString::fromUtf8("-e")
               << QString::fromUtf8("tell application \"MEGAShellExtFinder\" to quit");

    QProcess p;
    p.start(QString::fromAscii("osascript"), scriptArgs);
    if (!p.waitForFinished(2000))
    {
        return;
    }

    scriptArgs.clear();
    scriptArgs << QString::fromUtf8("-c")
               << QString::fromUtf8("pluginkit -e ignore -i mega.mac.MEGAShellExtFinder && sleep 1 && pluginkit -e use -i mega.mac.MEGAShellExtFinder");
    QProcess::startDetached(QString::fromUtf8("bash"), scriptArgs);
}

void MacXPlatform::enableFinderExtension(bool value)
{
    QStringList scriptArgs;
    scriptArgs << QString::fromUtf8("-e")
               << (value ? QString::fromUtf8("use") : QString::fromUtf8("ignore")) //Enable or disable extension plugin
               << QString::fromUtf8("-i")
               << kFinderSyncBundleId;

    QProcess::startDetached(QString::fromUtf8("pluginkit"), scriptArgs);
}

void MacXPlatform::showInFolder(QString pathIn)
{

    //Escape possible double quotes from osascript command to avoid syntax errors and stop parsing arguments
    pathIn.replace(QString::fromLatin1("\""), QString::fromLatin1("\\\""));

    QStringList scriptArgs;
    scriptArgs << QString::fromUtf8("-e")
               << QString::fromUtf8("tell application \"Finder\" to reveal POSIX file \"%1\"").arg(pathIn);
    QProcess::startDetached(QString::fromUtf8("osascript"), scriptArgs);
    scriptArgs.clear();
    scriptArgs << QString::fromUtf8("-e")
               << QString::fromUtf8("tell application \"Finder\" to activate");
    QProcess::startDetached(QString::fromAscii("osascript"), scriptArgs);
}

void MacXPlatform::startShellDispatcher(MegaApplication *receiver)
{
    if (!systemServiceTask)
    {
        systemServiceTask = new MacXSystemServiceTask(receiver);
    }

    if (!extService)
    {
        extService = new MacXExtServerService(receiver);
    }
}

void MacXPlatform::stopShellDispatcher()
{
    if (systemServiceTask)
    {
        delete systemServiceTask;
        systemServiceTask = NULL;
    }

    if (extService)
    {
        delete extService;
    }
}

void MacXPlatform::notifyItemChange(string *localPath, int newState)
{
    if (extService && localPath && localPath->size())
    {
        emit extService->itemChange(QString::fromStdString(*localPath), newState);
    }
}

void MacXPlatform::syncFolderAdded(QString syncPath, QString syncName, QString syncID)
{
    addPathToPlaces(syncPath,syncName);
    setFolderIcon(syncPath);

    if (extService)
    {
        emit extService->syncAdd(syncPath, syncName);
    }
}

void MacXPlatform::syncFolderRemoved(QString syncPath, QString syncName, QString syncID)
{
    removePathFromPlaces(syncPath);
    unSetFolderIcon(syncPath);

    if (extService)
    {
        emit extService->syncDel(syncPath, syncName);
    }
}

void MacXPlatform::notifyRestartSyncFolders()
{
    notifyAllSyncFoldersRemoved();
    notifyAllSyncFoldersAdded();
}

void MacXPlatform::notifyAllSyncFoldersAdded()
{
    if (extService)
    {
        emit extService->allClients(MacXExtServer::NOTIFY_ADD_SYNCS);
    }
}

void MacXPlatform::notifyAllSyncFoldersRemoved()
{
    if (extService)
    {
        emit extService->allClients(MacXExtServer::NOTIFY_DEL_SYNCS);
    }
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

QString MacXPlatform::getDefaultOpenApp(QString extension)
{
    return defaultOpenApp(extension);
}

void MacXPlatform::enableDialogBlur(QDialog *dialog)
{

}

bool MacXPlatform::registerUpdateJob()
{
    return registerUpdateDaemon();
}

void MacXPlatform::uninstall()
{

}

bool MacXPlatform::shouldRunHttpServer()
{
    return runHttpServer();
}

bool MacXPlatform::shouldRunHttpsServer()
{
    return runHttpsServer();
}

bool MacXPlatform::isUserActive()
{
    return userActive();
}

double MacXPlatform::getUpTime()
{
    return uptime();
}

void MacXPlatform::disableSignalHandler()
{
    signal(SIGSEGV, SIG_DFL);
    signal(SIGBUS, SIG_DFL);
    signal(SIGILL, SIG_DFL);
    signal(SIGFPE, SIG_DFL);
    signal(SIGABRT, SIG_DFL);
}

QString MacXPlatform::getDeviceName()
{
    // First, try to read maker and model
    QString vendor;
    QString model;

    QString deviceName;
    // If failure or empty strings, give hostname
    if (vendor.isEmpty() && model.isEmpty())
    {
        deviceName = QSysInfo::machineHostName();
        deviceName.remove(QLatin1Literal(".local"));
    }
    else
    {
        deviceName = vendor + QLatin1Literal(" ") + model;
    }

    return deviceName;
}

void MacXPlatform::initMenu(QMenu* m)
{
    if (m)
    {
        m->setStyleSheet(QLatin1String("QMenu {"
                                           "background: #ffffff;"
                                           "padding-top: 5px;"
                                           "padding-bottom: 5px;"
                                           "border: 1px solid #B8B8B8;"
                                           "border-radius: 5px;"
                                       "}"
                                       "QMenu::separator {"
                                           "height: 1px;"
                                           "margin: 0px 8px 0px 8px;"
                                           "background-color: rgba(0, 0, 0, 0.1);"
                                       "}"
                                       // For vanilla QMenus (only in TransferManager and MegaItemTreeView (NodeSelector))
                                       "QMenu::item {"
                                           "font-size: 14px;"
                                           "margin: 6px 16px 6px 16px;"
                                           "color: #777777;"
                                           "padding-right: 16px;"
                                       "}"
                                       "QMenu::item:selected {"
                                           "color: #000000;"
                                       "}"
                                       // For menus with MenuItemActions
                                       "QLabel {"
                                           "font-family: Lato;"
                                           "font-size: 14px;"
                                           "padding: 0px;"
                                       "}"
                                       ));
        m->setAttribute(Qt::WA_TranslucentBackground);
        m->setWindowFlags(m->windowFlags() | Qt::FramelessWindowHint);
        m->ensurePolished();
    }
}

// Platform-specific strings
const char* MacXPlatform::settingsString {QT_TRANSLATE_NOOP("Platform", "Preferences")};
const char* MacXPlatform::exitString {QT_TRANSLATE_NOOP("Platform", "Quit")};
const char* MacXPlatform::fileExplorerString {QT_TRANSLATE_NOOP("Platform","Show in Finder")};

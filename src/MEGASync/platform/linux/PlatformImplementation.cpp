#include "PlatformImplementation.h"

#include "DolphinFileManager.h"
#include "NautilusFileManager.h"
#include "QMegaMessageBox.h"

#include <QHostInfo>
#include <QProgressBar>
#include <QScreen>
#include <QSet>
#include <QX11Info>
#include <sys/statvfs.h>

#include <cstdlib>
#include <cstring>

using namespace std;
using namespace mega;

static const QString NotAllowedDefaultFactoryBiosName = QString::fromUtf8("To be filled by O.E.M.");

PlatformImplementation::PlatformImplementation()
{
    autostart_dir = QDir::homePath() + QString::fromLatin1("/.config/autostart/");
    desktop_file = autostart_dir + QString::fromLatin1("megasync.desktop");
    custom_icon = QString::fromUtf8("/usr/share/icons/hicolor/256x256/apps/mega.png");
}

void PlatformImplementation::initialize(int /*argc*/, char** /*argv*/)
{
    mShellNotifier = std::make_shared<SignalShellNotifier>();
}

void PlatformImplementation::notifyItemChange(const QString& path, int)
{
    if (!path.isEmpty())
    {
        if (notify_server && !Preferences::instance()->overlayIconsDisabled())
        {
            std::string stdPath = path.toUtf8().constData();
            notify_server->notifyItemChange(&stdPath);
        }
        mShellNotifier->notify(path);
    }
}

void PlatformImplementation::notifySyncFileChange(std::string *localPath, int newState)
{
    if(localPath && localPath->size())
    {
        notifyItemChange(QString::fromStdString(*localPath), newState);
    }
}

// enable or disable MEGASync launching at startup
// return true if operation succeeded
bool PlatformImplementation::startOnStartup(bool value)
{
    // copy desktop file into autostart directory
    if (value)
    {
        if (QFile(desktop_file).exists())
        {
            return true;
        }
        else
        {
            // make sure directory exist
            if (!QDir(autostart_dir).exists())
            {
                if (!QDir().mkdir(autostart_dir))
                {
                    //LOG_debug << "Failed to create autostart dir: " << autostart_dir;
                    return false;
                }
            }
            QString app_desktop = QString::fromLatin1("/usr/share/applications/megasync.desktop");
            if (QFile(app_desktop).exists())
            {
                return QFile::copy(app_desktop, desktop_file);
            }
            else
            {
                //LOG_debug << "Desktop file does not exist: " << app_desktop;
                return false;
            }
        }
    }
    else
    {
        // remove desktop file if it exists
        if (QFile(desktop_file).exists())
        {
            return QFile::remove(desktop_file);
        }
    }
    return true;
}

bool PlatformImplementation::isStartOnStartupActive()
{
    return QFile(desktop_file).exists();
}

bool PlatformImplementation::isTilingWindowManager()
{
    static const QSet<QString> tiling_wms = {
        QLatin1String("i3"),
        QLatin1String("Hyprland"),
        QLatin1String("sway")
    };

    return getValue("MEGASYNC_ASSUME_TILING_WM", false)
           || tiling_wms.contains(getWindowManagerName());
}

bool PlatformImplementation::showInFolder(QString pathIn)
{
    QString fileBrowser = getDefaultFileBrowserApp();

    static const QMap<QString, QStringList> showInFolderCallMap
    {
        {QLatin1String("dolphin"), DolphinFileManager::getShowInFolderParams()},
        {QLatin1String("nautilus"), NautilusFileManager::getShowInFolderParams()}
    };

    QStringList params;
    auto itFoundAppParams = showInFolderCallMap.constFind(fileBrowser);
    if (itFoundAppParams != showInFolderCallMap.constEnd())
    {
        params << *itFoundAppParams;
        return QProcess::startDetached(fileBrowser, params << QUrl::fromLocalFile(pathIn).toString());
    }
    else
    {
        QString folderToOpen;
        QFileInfo file(pathIn);
        if(file.isFile())
        {
            //xdg-open open folders, so we choose the file parent folder
            folderToOpen = file.absolutePath();
        }
        else
        {
            folderToOpen = pathIn;
        }

        return QProcess::startDetached(QLatin1String("xdg-open"), params << QUrl::fromLocalFile(folderToOpen).toString());
    }

}

void PlatformImplementation::startShellDispatcher(MegaApplication *receiver)
{
    if (!ext_server)
    {
        ext_server = new ExtServer(receiver);
    }

    if (!notify_server)
    {
        notify_server = new NotifyServer();
    }
}

void PlatformImplementation::stopShellDispatcher()
{
    if (ext_server)
    {
        delete ext_server;
        ext_server = NULL;
    }

    if (notify_server)
    {
        delete notify_server;
        notify_server = NULL;
    }
}

void PlatformImplementation::syncFolderAdded(QString syncPath, QString /*syncName*/, QString /*syncID*/)
{
    if (QFile(custom_icon).exists())
    {
        QFile *folder = new QFile(syncPath);
        if (folder->exists())
        {
            NautilusFileManager::changeFolderIcon(syncPath, custom_icon);
            DolphinFileManager::changeFolderIcon(syncPath, custom_icon);
        }
        delete folder;

    }

    if (notify_server)
    {
        notify_server->notifySyncAdd(syncPath);
    }
}

void PlatformImplementation::syncFolderRemoved(QString syncPath, QString /*syncName*/, QString /*syncID*/)
{
    QFile *folder = new QFile(syncPath);
    if (folder->exists())
    {
        NautilusFileManager::changeFolderIcon(syncPath);
        DolphinFileManager::changeFolderIcon(syncPath);
    }
    delete folder;

    if (notify_server)
    {
        notify_server->notifySyncDel(syncPath);
    }
}

void PlatformImplementation::notifyRestartSyncFolders()
{

}

void PlatformImplementation::notifyAllSyncFoldersAdded()
{

}

void PlatformImplementation::notifyAllSyncFoldersRemoved()
{

}

QString PlatformImplementation::preparePathForSync(const QString& path)
{
    return QDir::toNativeSeparators(QDir::cleanPath(path));
}

void PlatformImplementation::processSymLinks()
{

}

QString PlatformImplementation::getDefaultFileBrowserApp()
{
    return getDefaultOpenAppByMimeType(QString::fromUtf8("inode/directory"));
}

QString PlatformImplementation::getDefaultOpenApp(QString extension)
{
    char *mimeType = MegaApi::getMimeType(extension.toUtf8().constData());
    if (!mimeType)
    {
        return QString();
    }
    QString qsMimeType(QString::fromUtf8(mimeType));
    delete mimeType;
    return getDefaultOpenAppByMimeType(qsMimeType);
}

QString PlatformImplementation::getDefaultOpenAppByMimeType(QString mimeType)
{
    QString getDefaultAppDesktopFileName = QString::fromUtf8("xdg-mime query default ") + mimeType;

    QProcess process;
    process.start(getDefaultAppDesktopFileName,
                  QIODevice::ReadWrite | QIODevice::Text);
    if(!process.waitForFinished(5000))
    {
        return QString();
    }

    QString desktopFileName = QString::fromUtf8(process.readAllStandardOutput());
    desktopFileName = desktopFileName.trimmed();
    desktopFileName.replace(QString::fromUtf8(";"), QString::fromUtf8(""));
    if (!desktopFileName.size())
    {
        return QString();
    }

    QFileInfo desktopFile(QString::fromUtf8("/usr/share/applications/") + desktopFileName);
    if (!desktopFile.exists())
    {
        return QString();
    }

    QFile f(desktopFile.absoluteFilePath());
    if (!f.open(QFile::ReadOnly | QFile::Text))
    {
        return QString();
    }

    QTextStream in(&f);
    QStringList contents = in.readAll().split(QString::fromUtf8("\n"));
    contents = contents.filter(QRegExp(QString::fromUtf8("^Exec=")));
    if (!contents.size())
    {
        return QString();
    }

    QString line = contents.first();
    QRegExp captureRegexCommand(QString::fromUtf8("^Exec=([^' ']*)"));
    if (captureRegexCommand.indexIn(line) != -1)
    {
        return captureRegexCommand.cap(1); // return first group from regular expression.
    }

    return QString();
}

bool PlatformImplementation::getValue(const char * const name, const bool default_value)
{
    QString value = qEnvironmentVariable(name);

    if (value.isEmpty())
    {
        return default_value;
    }

    return value != QString::fromUtf8("0");
}

std::string PlatformImplementation::getValue(const char * const name, const std::string &default_value)
{
    QString value = qEnvironmentVariable(name);

    if (value.isEmpty())
    {
        return default_value;
    }

    return value.toUtf8().constData();
}

QString PlatformImplementation::getWindowManagerName()
{
    static QString wmName;
    static bool cached = false;

    if (!cached)
    {
        if (QX11Info::isPlatformX11())
        {
            const int maxLen = 1024;
            const auto connection = QX11Info::connection();
            const auto appRootWindow = static_cast<xcb_window_t>(QX11Info::appRootWindow());

            if (connection != nullptr)
            {
                auto wmCheckAtom = getAtom(connection, "_NET_SUPPORTING_WM_CHECK");
                // Get window manager
                auto reply = xcb_get_property_reply(connection,
                                                    xcb_get_property(connection,
                                                                     false,
                                                                     appRootWindow,
                                                                     wmCheckAtom,
                                                                     XCB_ATOM_WINDOW,
                                                                     0,
                                                                     maxLen),
                                                    nullptr);

                if (reply && reply->format == 32 && reply->type == XCB_ATOM_WINDOW)
                {
                    // Get window manager name
                    const xcb_window_t windowManager = *(static_cast<xcb_window_t*>(xcb_get_property_value(reply)));

                    if (windowManager != XCB_WINDOW_NONE)
                    {
                        const auto utf8StringAtom = getAtom(connection, "UTF8_STRING");
                        const auto wmNameAtom = getAtom(connection, "_NET_WM_NAME");

                        auto wmReply = xcb_get_property_reply(connection,
                                                              xcb_get_property(connection,
                                                                               false,
                                                                               windowManager,
                                                                               wmNameAtom,
                                                                               utf8StringAtom,
                                                                               0,
                                                                               maxLen),
                                                              nullptr);
                        if (wmReply && wmReply->format == 8 && wmReply->type == utf8StringAtom)
                        {
                            wmName = QString::fromUtf8(static_cast<const char*>(xcb_get_property_value(wmReply)),
                                                                                xcb_get_property_value_length(wmReply));
                        }
                        free(wmReply);
                    }
                }
                free(reply);
                cached = true;
            }
        }

        if (!cached)
        {
            // The previous method failed. We are most probably on Wayland.
            // Try to get info from environment.
            wmName = qEnvironmentVariable("XDG_CURRENT_DESKTOP");
            cached = !wmName.isEmpty();
        }
    }
    return wmName;
}

bool PlatformImplementation::registerUpdateJob()
{
    return true;
}

// Check if it's needed to start the local HTTP server
// for communications with the webclient
bool PlatformImplementation::shouldRunHttpServer()
{
    QStringList data = getListRunningProcesses();
    if (data.size() > 1)
    {
        for (int i = 1; i < data.size(); i++)
        {
            // The MEGA webclient sends request to MEGAsync to improve the
            // user experience. We check if web browsers are running because
            // otherwise it isn't needed to run the local web server for this purpose.
            // Here is the list or web browsers that allow HTTP communications
            // with 127.0.0.1 inside HTTPS webs.
            QString command = data.at(i).trimmed();
            if (command.contains(QString::fromUtf8("firefox"), Qt::CaseInsensitive)
                    || command.contains(QString::fromUtf8("chrome"), Qt::CaseInsensitive)
                    || command.contains(QString::fromUtf8("chromium"), Qt::CaseInsensitive)
                    )
            {
                return true;
            }
        }
    }
    return false;
}

bool PlatformImplementation::isUserActive()
{
    return true;
}

QString PlatformImplementation::getDeviceName()
{
    // First, try to read maker and model
    QString vendor;
    QFile vendorFile(QLatin1String("/sys/devices/virtual/dmi/id/board_vendor"));
    if (vendorFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        vendor = QString::fromUtf8(vendorFile.readLine()).trimmed();
    }
    vendorFile.close();

    QString model;
    QFile modelFile(QLatin1String("/sys/devices/virtual/dmi/id/product_name"));
    if (modelFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        model = QString::fromUtf8(modelFile.readLine()).trimmed();
    }
    modelFile.close();

    QString deviceName = vendor + QLatin1String(" ") + model;
    // If failure, empty strings or defaultFactoryBiosName, give hostname.
    if ((vendor.isEmpty() && model.isEmpty()) || deviceName.contains(NotAllowedDefaultFactoryBiosName))
    {
        deviceName = QHostInfo::localHostName();
    }

    return deviceName;
}

void PlatformImplementation::fileSelector(const SelectorInfo& info)
{
    if (info.defaultDir.isEmpty())
    {
        auto updateInfo = info;
        updateInfo.defaultDir = QLatin1String("/");
        AbstractPlatform::fileSelector(updateInfo);
    }
    else
    {
        AbstractPlatform::fileSelector(info);
    }
}

void PlatformImplementation::folderSelector(const SelectorInfo& info)
{
    if (info.defaultDir.isEmpty())
    {
        auto updateInfo = info;
        updateInfo.defaultDir = QLatin1String("/");
        AbstractPlatform::folderSelector(updateInfo);
    }
    else
    {
        AbstractPlatform::folderSelector(info);
    }
}

void PlatformImplementation::fileAndFolderSelector(const SelectorInfo& info)
{
    if (info.defaultDir.isEmpty())
    {
        auto updateInfo = info;
        updateInfo.defaultDir = QLatin1String("/");
        AbstractPlatform::fileAndFolderSelector(updateInfo);
    }
    else
    {
        AbstractPlatform::fileAndFolderSelector(info);
    }
}

void PlatformImplementation::streamWithApp(const QString &app, const QString &url)
{
    QString command = QString::fromUtf8("%1 \"%2\"").arg(QDir::toNativeSeparators(app)).arg(url);
    QProcess::startDetached(command);
}

DriveSpaceData PlatformImplementation::getDriveData(const QString& path)
{
    DriveSpaceData data;

    struct statvfs statData;
    const int result = statvfs(path.toUtf8().constData(), &statData);
    data.mIsReady = (result == 0);
    data.mTotalSpace = static_cast<long long>(statData.f_blocks * statData.f_bsize);
    data.mAvailableSpace = static_cast<long long>(statData.f_bfree * statData.f_bsize);
    return data;
}

#if defined(ENABLE_SDK_ISOLATED_GFX)
QString PlatformImplementation::getGfxProviderPath()
{
    return QCoreApplication::applicationDirPath() + QLatin1String("/mega-desktop-app-gfxworker");
}
#endif

QStringList PlatformImplementation::getListRunningProcesses()
{
    QProcess p;
    p.start(QString::fromUtf8("ps ax -o comm"));
    p.waitForFinished(2000);
    QString output = QString::fromUtf8(p.readAllStandardOutput().constData());
    QString e = QString::fromUtf8(p.readAllStandardError().constData());
    if (e.size())
    {
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR, "Error for \"ps ax -o comm\" command:");
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR, e.toUtf8().constData());
    }

    p.start(QString::fromUtf8("/bin/bash -c \"readlink /proc/*/exe\""));
    p.waitForFinished(2000);
    QString output2 = QString::fromUtf8(p.readAllStandardOutput().constData());
    e = QString::fromUtf8(p.readAllStandardError().constData());
    if (e.size())
    {
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR, "Error for \"readlink /proc/*/exe\" command:");
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR, e.toUtf8().constData());
    }
    QStringList data = output.split(QString::fromUtf8("\n"));

    data.append(output2.split(QString::fromUtf8("\n")));

    return data;
}

xcb_atom_t PlatformImplementation::getAtom(xcb_connection_t * const connection, const char *name)
{
    xcb_intern_atom_cookie_t cookie =
      xcb_intern_atom(connection, 0, static_cast<uint16_t>(strlen(name)), name);
    xcb_intern_atom_reply_t *reply =
      xcb_intern_atom_reply(connection, cookie, nullptr);

    if (!reply)
        return XCB_ATOM_NONE;

    xcb_atom_t result = reply->atom;
    free(reply);

    return result;
}

bool PlatformImplementation::validateSystemTrayIntegration()
{
    if (QSystemTrayIcon::isSystemTrayAvailable() || verifyAndEnableAppIndicatorExtension())
    {
        return true;
    }

    QByteArray appIndicatorInstallFlag = qgetenv("MEGA_ALLOW_DNF_APPINDICATOR_INSTALL");

    bool isAppIndicatorInstallEnabled = !appIndicatorInstallFlag.isEmpty() &&
                                        (appIndicatorInstallFlag == "1" ||
                                         appIndicatorInstallFlag.toLower() == "true" ||
                                         appIndicatorInstallFlag.toLower() == "yes");

    if (isAppIndicatorInstallEnabled || isFedoraWithGnome())
    {
        constexpr qint64 SECONDS_IN_A_WEEK = 604800;

        qint64 currentTimestamp = QDateTime::currentSecsSinceEpoch();
        qint64 lastPromptTimestamp = Preferences::instance()->getSystemTrayLastPromptTimestamp();

        bool aWeekHasPassed = (currentTimestamp - lastPromptTimestamp >= SECONDS_IN_A_WEEK);
        bool oneTimeSystrayCheck = !Preferences::instance()->isOneTimeActionDone(Preferences::ONE_TIME_ACTION_NO_SYSTRAY_AVAILABLE);
        bool promptSuppressed = Preferences::instance()->isSystemTrayPromptSuppressed();

        // Check if it's time to prompt the user
        if ((oneTimeSystrayCheck || aWeekHasPassed) && !promptSuppressed)
        {
            promptFedoraGnomeUser();
        }
        return true; // Always return true for Fedora GNOME environment as the system tray integration is already handled
    }

    // Return false for non-Fedora GNOME environments
    return false;
}

bool PlatformImplementation::isFedoraWithGnome()
{
    // Check for GNOME Desktop Environment
    QByteArray desktopEnvironment = qgetenv("XDG_CURRENT_DESKTOP");
    if (!desktopEnvironment.toLower().contains(QStringLiteral("gnome").toUtf8()))
    {
        return false;
    }

    // Check for Fedora OS
    QString osReleasePath = QStringLiteral("/etc/os-release");
    QFile osReleaseFile(osReleasePath);
    if (osReleaseFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QString content = QString::fromUtf8(osReleaseFile.readAll());
        if (content.contains(QStringLiteral("ID=fedora"), Qt::CaseInsensitive))
        {
            return true;
        }
    }

    return false;
}

void PlatformImplementation::promptFedoraGnomeUser()
{
    QMegaMessageBox::MessageBoxInfo msgInfo;
    msgInfo.title = QCoreApplication::translate("LinuxPlatformNotificationAreaIcon", "Install notification area icon");
    msgInfo.text = QCoreApplication::translate("LinuxPlatformNotificationAreaIcon", "For a better experience on Fedora with GNOME, we recommend you enable the notification area icon.\n"
                                                                                    "Would you like to install the necessary components now?");
    msgInfo.buttons = QMessageBox::Yes | QMessageBox::No;
    msgInfo.defaultButton = QMessageBox::Yes;
    msgInfo.checkboxText = QCoreApplication::translate("LinuxPlatformNotificationAreaIcon", "Do not show again");
    msgInfo.finishFunc = [this](QPointer<QMessageBox> msg)
    {
        if (!msg) return;

        bool isInstallationAttempted = (msg->result() == QMessageBox::Yes);
        bool isInstallationSuccessful = false;

        if (isInstallationAttempted)
        {
            // Execute the bash script to install appindicator.
            isInstallationSuccessful = installAppIndicatorForFedoraGnome();
        }

        if (msg->checkBox() && msg->checkBox()->isChecked())
        {
            Preferences::instance()->setSystemTrayPromptSuppressed(true);
        }
        else
        {
            Preferences::instance()->setSystemTrayLastPromptTimestamp(QDateTime::currentSecsSinceEpoch());
        }

        // Set the one-time action done if the user clicked "No" or if the installation was successful.
        if (!isInstallationAttempted || isInstallationSuccessful)
        {
            Preferences::instance()->setOneTimeActionDone(Preferences::ONE_TIME_ACTION_NO_SYSTRAY_AVAILABLE, true);
        }
    };

    QMegaMessageBox::question(msgInfo);
}

bool PlatformImplementation::installAppIndicatorForFedoraGnome()
{
    constexpr char GNOME_EXTENSIONS_CMD[] = "gnome-extensions";
    constexpr char APP_INDICATOR_EXTENSION_ID[] = "appindicatorsupport@rgcjonas.gmail.com";
    const int PROCESS_TIMEOUT_MS = 120000; // 2 minutes

    QProcess checkProcess;
    checkProcess.start(QString::fromUtf8(GNOME_EXTENSIONS_CMD), { QStringLiteral("version") });
    bool gnomeExtensionsAvailable = checkProcess.waitForFinished(PROCESS_TIMEOUT_MS) && checkProcess.exitCode() == 0;

    QStringList installArgs = { QStringLiteral("dnf"), QStringLiteral("install"), QStringLiteral("-y") };
    if (!gnomeExtensionsAvailable)
    {
        installArgs << QStringLiteral("gnome-shell-extensions");
    }
    installArgs << QStringLiteral("gnome-shell-extension-appindicator");

    QProgressDialog progressDialog(QCoreApplication::translate("LinuxPlatformNotificationAreaIcon", "Installing notification area icon..."), QCoreApplication::translate("LinuxPlatformNotificationAreaIcon","Cancel"), 0, 100);
    progressDialog.setWindowTitle(QCoreApplication::translate("LinuxPlatformNotificationAreaIcon","Installing"));
    progressDialog.setWindowModality(Qt::WindowModal);
    progressDialog.show();

    QProcess installProcess;
    QEventLoop loop;
    QByteArray byteArray;
    bool cancellationInitiated = false;

    auto updateProgress =  [this, &cancellationInitiated, &installProcess, &progressDialog, &byteArray, &loop]()
    {
        QString output = QString::fromUtf8(byteArray.data(), byteArray.size());
        int parsedValue = this->parseDnfOutput(output);
        progressDialog.setValue(parsedValue);

        if (progressDialog.wasCanceled() && !cancellationInitiated)
        {
            cancellationInitiated = true;
            installProcess.kill();
            installProcess.waitForFinished();
            progressDialog.close();

            QMegaMessageBox::MessageBoxInfo msgWarnInfo;
            msgWarnInfo.title = QCoreApplication::translate("LinuxPlatformNotificationAreaIcon", "Installation Cancelled");
            msgWarnInfo.text = QCoreApplication::translate("LinuxPlatformNotificationAreaIcon", "The notification area icon installation was cancelled.");
            QMegaMessageBox::warning(msgWarnInfo);

            loop.exit(1);
        }
    };

    QObject::connect(&installProcess, &QProcess::readyReadStandardOutput, [&]() {
        byteArray += installProcess.readAllStandardOutput();
        updateProgress();
    });

    QObject::connect(&installProcess, &QProcess::readyReadStandardError,  [&]() {
        byteArray += installProcess.readAllStandardError();
        updateProgress();
    });

    QObject::connect(&installProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [&](int exitCode) {
        if (exitCode == 0)
        {
            progressDialog.close();
            loop.exit(0);
        } else
        {
            QMegaMessageBox::MessageBoxInfo errorInfo;
            errorInfo.title = QCoreApplication::translate("LinuxPlatformNotificationAreaIcon", "Error installing components");
            errorInfo.text = QCoreApplication::translate("LinuxPlatformNotificationAreaIcon", "Failed to install the necessary components.");
            errorInfo.informativeText = QCoreApplication::translate("LinuxPlatformNotificationAreaIcon", "To install manually, please run the following commands:\n\n"
                                                       "sudo dnf install gnome-shell-extensions\n"
                                                       "sudo dnf install gnome-shell-extension-appindicator\n"
                                                       "gnome-extensions enable appindicatorsupport@rgcjonas.gmail.com");
            QMegaMessageBox::critical(errorInfo);
            loop.exit(1);
        }
    });

    installProcess.start(QStringLiteral("pkexec"), installArgs);
    int loopResult = loop.exec();

    if (loopResult != 0)
    {
        return false;
    }

    // After successful installation, enable the extension
    QProcess enableProcess;
    enableProcess.start(QString::fromUtf8(GNOME_EXTENSIONS_CMD), { QStringLiteral("enable"), QString::fromUtf8(APP_INDICATOR_EXTENSION_ID) });
    enableProcess.waitForFinished(PROCESS_TIMEOUT_MS);

    QMegaMessageBox::MessageBoxInfo successInfo;
    successInfo.title = QCoreApplication::translate("LinuxPlatformNotificationAreaIcon", "Install complete");
    successInfo.text = QCoreApplication::translate("LinuxPlatformNotificationAreaIcon", "The notification area icon was installed successfully.\n"
                                                                                        "Please log out of your computer to complete the installation.");
    QMegaMessageBox::information(successInfo);

    return true;
}

int PlatformImplementation::parseDnfOutput(const QString& dnfOutput)
{
    QRegularExpression progressRegExp(QString::fromUtf8(R"(\((\d+)/(\d+)\):)"));
    int totalPackages = 0;
    int packagesProcessed = 0;

    QStringList lines = dnfOutput.split(QLatin1Char('\n'));
    for (const QString& line : lines)
    {
        QRegularExpressionMatch match = progressRegExp.match(line);
        if (match.hasMatch())
        {
            int currentPackageIndex = match.captured(1).toInt();
            int currentTotalPackages = match.captured(2).toInt();
            totalPackages = qMax(totalPackages, currentTotalPackages);
            packagesProcessed = qMax(packagesProcessed, currentPackageIndex);
        }
    }

    return totalPackages > 0 ? static_cast<int>(100.0 * packagesProcessed / totalPackages) : 0;
}

bool PlatformImplementation::verifyAndEnableAppIndicatorExtension()
{
    constexpr char GNOME_EXTENSIONS_CMD[] = "gnome-extensions";
    constexpr char APP_INDICATOR_EXTENSION_ID[] = "appindicatorsupport@rgcjonas.gmail.com";
    constexpr char GNOME_EXTENSION_PATH[] = "/usr/share/gnome-shell/extensions/";
    const int PROCESS_TIMEOUT_MS = 120000; // 2 minutes

    QProcess process;

    // Check if the extension directory exists in the filesystem
    const QString extensionDirPath = QString::fromUtf8(GNOME_EXTENSION_PATH) + QString::fromUtf8(APP_INDICATOR_EXTENSION_ID);
    if (!QDir(extensionDirPath).exists())
    {
        return false;
    }

    // Check if GNOME extensions command is available
    process.start(QString::fromUtf8(GNOME_EXTENSIONS_CMD), { QStringLiteral("version") });
    bool gnomeExtensionsAvailable = process.waitForFinished(PROCESS_TIMEOUT_MS) && process.exitCode() == 0;
    if (!gnomeExtensionsAvailable)
    {
        return false;
    }

    // List all the available extensions to ensure the extension is recognized by GNOME
    process.start(QString::fromUtf8(GNOME_EXTENSIONS_CMD), {QLatin1String("list")});
    if (!process.waitForFinished(PROCESS_TIMEOUT_MS))
    {
        return false;
    }

    // Check if the app indicator extension is recognized by GNOME
    QString output = QString::fromUtf8(process.readAllStandardOutput());
    if (!output.contains(QString::fromUtf8(APP_INDICATOR_EXTENSION_ID))) {
        return false;
    }

    // Enable the app indicator extension using GNOME extensions command
    process.start(QString::fromUtf8(GNOME_EXTENSIONS_CMD), { QStringLiteral("enable"), QString::fromUtf8(APP_INDICATOR_EXTENSION_ID) });
    if (!process.waitForFinished(PROCESS_TIMEOUT_MS) || process.exitCode() != 0)
    {
        return false;
    }

    return true;
}

void PlatformImplementation::calculateInfoDialogCoordinates(const QRect& rect, int* posx, int* posy)
{
    int xSign = 1;
    int ySign = 1;
    QPoint position;
    QRect screenGeometry;

    position = QCursor::pos();
    QScreen* currentScreen = QGuiApplication::screenAt(position);
    if (currentScreen)
    {
        screenGeometry = currentScreen->availableGeometry();

        QString otherInfo = QString::fromUtf8("pos = [%1,%2], name = %3").arg(position.x()).arg(position.y()).arg(currentScreen->name());
        logInfoDialogCoordinates("availableGeometry", screenGeometry, otherInfo);

        if (!screenGeometry.isValid())
        {
            screenGeometry = currentScreen->geometry();
            otherInfo = QString::fromUtf8("dialog rect = %1").arg(rectToString(rect));
            logInfoDialogCoordinates("screenGeometry", screenGeometry, otherInfo);

            if (screenGeometry.isValid())
            {
                screenGeometry.setTop(28);
            }
            else
            {
                screenGeometry = rect;
                screenGeometry.setBottom(screenGeometry.bottom() + 4);
                screenGeometry.setRight(screenGeometry.right() + 4);
            }

            logInfoDialogCoordinates("screenGeometry 2", screenGeometry, otherInfo);
        }
        else
        {
            if (screenGeometry.y() < 0)
            {
                ySign = -1;
            }

            if (screenGeometry.x() < 0)
            {
                xSign = -1;
            }
        }

        if (position.x() * xSign > (screenGeometry.right() / 2) * xSign)
        {
            *posx = screenGeometry.right() - rect.width() - 2;
        }
        else
        {
            *posx = screenGeometry.left() + 2;
        }

        if (position.y() * ySign > (screenGeometry.bottom() / 2) * ySign)
        {
            *posy = screenGeometry.bottom() - rect.height() - 2;
        }
        else
        {
            *posy = screenGeometry.top() + 2;
        }
    }

    QString otherInfo = QString::fromUtf8("dialog rect = %1, posx = %2, posy = %3").arg(rectToString(rect)).arg(*posx).arg(*posy);
    logInfoDialogCoordinates("Final", screenGeometry, otherInfo);
}

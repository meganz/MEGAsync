#include "MegaApplication.h"
#include "gui/MegaProxyStyle.h"
#include "platform/Platform.h"
#include "qtlockedfile/qtlockedfile.h"
#include "control/AppStatsEvents.h"
#include "control/CrashHandler.h"
#include "ScaleFactorManager.h"

#include <QFontDatabase>
#include <assert.h>

#ifdef Q_OS_LINUX
    #include <signal.h>
    #include <condition_variable>
#endif

#ifndef WIN32
//sleep
#include <unistd.h>
#else
#include <Windows.h>
#include <Psapi.h>
#include <Strsafe.h>
#include <Shellapi.h>
#endif

#if defined(WIN32) || defined(Q_OS_LINUX)
#include <QScreen>
#endif

using namespace mega;
using namespace std;

struct LogMessage
{
    int logLevel;
    QString message;

    LogMessage(int logLevel, QString message):logLevel{logLevel}, message{message}{};
};
std::vector<LogMessage> logMessages;

void msgHandler(QtMsgType type, const char *msg)
{
    switch (type)
    {
        case QtInfoMsg:
            MegaApi::log(MegaApi::LOG_LEVEL_INFO, QString::fromUtf8("Qt Info: %1").arg(QString::fromUtf8(msg)).toUtf8().constData());
            break;
        case QtDebugMsg:
            MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, QString::fromUtf8("Qt Debug: %1").arg(QString::fromUtf8(msg)).toUtf8().constData());
            break;
        case QtWarningMsg:
            MegaApi::log(MegaApi::LOG_LEVEL_WARNING, QString::fromUtf8("Qt Warning: %1").arg(QString::fromUtf8(msg)).toUtf8().constData());
            break;
        case QtCriticalMsg:
            MegaApi::log(MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("Qt Critical: %1").arg(QString::fromUtf8(msg)).toUtf8().constData());
            break;
        case QtFatalMsg:
            MegaApi::log(MegaApi::LOG_LEVEL_FATAL, QString::fromUtf8("Qt FATAL: %1").arg(QString::fromUtf8(msg)).toUtf8().constData());
            break;
        default:
            MegaApi::log(MegaApi::LOG_LEVEL_MAX, QString::fromUtf8("Qt MSG: %1").arg(QString::fromUtf8(msg)).toUtf8().constData());
            break;
    }
}

#ifdef Q_OS_LINUX
MegaApplication *theapp = NULL;
bool waitForRestartSignal = false;
std::mutex mtxcondvar;
std::condition_variable condVarRestart;
QString appToWaitForSignal;

void LinuxSignalHandler(int signum)
{
    if (signum == SIGUSR2)
    {
        std::unique_lock<std::mutex> lock(mtxcondvar);
        condVarRestart.notify_one();
    }
    else if (signum == SIGUSR1)
    {
        waitForRestartSignal = true;
        if (waitForRestartSignal)
        {
            appToWaitForSignal.append(QString::fromUtf8(" --waitforsignal"));
            bool success = QProcess::startDetached(appToWaitForSignal);
            cout << "Started detached MEGAsync to wait for restart signal: " << appToWaitForSignal.toUtf8().constData() << " " << (success?"OK":"FAILED!") << endl;
        }

        if (theapp)
        {
            theapp->tryExitApplication(true);
        }
    }
}

#endif

    void messageHandler(QtMsgType type,const QMessageLogContext &context, const QString &msg)
    {       
        switch (type)
        {
            case QtInfoMsg:
                MegaApi::log(MegaApi::LOG_LEVEL_INFO, QString::fromUtf8("Qt Info: %1").arg(msg).toUtf8().constData());
                // deliberately not showing context for Info level
                break;
            case QtDebugMsg:
                MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, QString::fromUtf8("Qt Debug: %1").arg(msg).toUtf8().constData());
                MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, QString::fromUtf8("Qt Context: %1 %2 %3 %4 %5")
                             .arg(QString::fromUtf8(context.category))
                             .arg(QString::fromUtf8(context.file))
                             .arg(QString::fromUtf8(context.function))
                             .arg(QString::fromUtf8(context.file))
                             .arg(context.version).toUtf8().constData());
                break;
            case QtWarningMsg:
                MegaApi::log(MegaApi::LOG_LEVEL_WARNING, QString::fromUtf8("Qt Warning: %1").arg(msg).toUtf8().constData());
                MegaApi::log(MegaApi::LOG_LEVEL_WARNING, QString::fromUtf8("Qt Context: %1 %2 %3 %4 %5")
                             .arg(QString::fromUtf8(context.category))
                             .arg(QString::fromUtf8(context.file))
                             .arg(QString::fromUtf8(context.function))
                             .arg(QString::fromUtf8(context.file))
                             .arg(context.version).toUtf8().constData());
                break;
            case QtCriticalMsg:
                MegaApi::log(MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("Qt Critical: %1").arg(msg).toUtf8().constData());
                MegaApi::log(MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("Qt Context: %1 %2 %3 %4 %5")
                             .arg(QString::fromUtf8(context.category))
                             .arg(QString::fromUtf8(context.file))
                             .arg(QString::fromUtf8(context.function))
                             .arg(QString::fromUtf8(context.file))
                             .arg(context.version).toUtf8().constData());
                break;
            case QtFatalMsg:
                MegaApi::log(MegaApi::LOG_LEVEL_FATAL, QString::fromUtf8("Qt FATAL: %1").arg(msg).toUtf8().constData());
                MegaApi::log(MegaApi::LOG_LEVEL_FATAL, QString::fromUtf8("Qt Context: %1 %2 %3 %4 %5")
                             .arg(QString::fromUtf8(context.category))
                             .arg(QString::fromUtf8(context.file))
                             .arg(QString::fromUtf8(context.function))
                             .arg(QString::fromUtf8(context.file))
                             .arg(context.version).toUtf8().constData());
                break;
           default:
                MegaApi::log(MegaApi::LOG_LEVEL_MAX, QString::fromUtf8("Qt MSG: %1").arg(msg).toUtf8().constData());
                MegaApi::log(MegaApi::LOG_LEVEL_MAX, QString::fromUtf8("Qt Context: %1 %2 %3 %4 %5")
                             .arg(QString::fromUtf8(context.category))
                             .arg(QString::fromUtf8(context.file))
                             .arg(QString::fromUtf8(context.function))
                             .arg(QString::fromUtf8(context.file))
                             .arg(context.version).toUtf8().constData());
                break;
        }
    }

void removeSyncData(const QString &localFolder, const QString & name, const QString &syncID)
{
    Platform::getInstance()->syncFolderRemoved(localFolder, name, syncID);

    #ifdef WIN32
    // unhide debris folder
    QString debrisPath = QDir::toNativeSeparators(localFolder +
            QDir::separator() + QString::fromUtf8(MEGA_DEBRIS_FOLDER));

    WIN32_FILE_ATTRIBUTE_DATA fad;
    if (GetFileAttributesExW((LPCWSTR)debrisPath.utf16(), GetFileExInfoStandard, &fad))
    {
        SetFileAttributesW((LPCWSTR)debrisPath.utf16(), fad.dwFileAttributes & ~FILE_ATTRIBUTE_HIDDEN);
    }

    QDir dir(debrisPath);
    QFileInfoList fList = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden);
    for (int j = 0; j < fList.size(); j++)
    {
        QString folderPath = QDir::toNativeSeparators(fList[j].absoluteFilePath());
        WIN32_FILE_ATTRIBUTE_DATA fa;
        if (GetFileAttributesExW((LPCWSTR)folderPath.utf16(), GetFileExInfoStandard, &fa))
        {
            SetFileAttributesW((LPCWSTR)folderPath.utf16(), fa.dwFileAttributes & ~FILE_ATTRIBUTE_HIDDEN);
        }
    }
    #endif
}

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName(QString::fromUtf8("Mega Limited"));
    QCoreApplication::setOrganizationDomain(QString::fromUtf8("mega.co.nz"));
    QCoreApplication::setApplicationName(QString::fromUtf8("MEGAsync")); //Do not change app name, keep MEGAsync because Linux rely on that for app paths.
    QCoreApplication::setApplicationVersion(QString::number(Preferences::VERSION_CODE));

    if ((argc == 2) && !strcmp("/uninstall", argv[1]))
    {
        auto preferences = Preferences::instance();
        preferences->initialize(MegaApplication::applicationDataPath());
        if (!preferences->error())
        {
            if (preferences->logged())
            {
                preferences->unlink();
            }

            for (int i = 0; i < preferences->getNumUsers(); i++)
            {
                preferences->enterUser(i);

                // we do first for old sync configuration (in case there were remaining for some user)
                QList<SyncData> syncData = preferences->readOldCachedSyncs();
                foreach(SyncData osd, syncData)
                {
                    removeSyncData(osd.mLocalFolder, osd.mName.remove(QLatin1Char(':')), osd.mSyncID);
                }

                // now for the new syncs cached configurations
                auto loadedSyncs = preferences->getLoadedSyncsMap();
                for (auto it = loadedSyncs.begin(); it != loadedSyncs.end(); it++)
                {
                    removeSyncData(it.value()->getLocalFolder(), it.value()->name(true), it.value()->getSyncID());
                }

                preferences->leaveUser();
            }
        }

        Utilities::removeRecursively(MegaApplication::applicationDataPath());
        Platform::getInstance()->uninstall();

#ifdef WIN32
        if (preferences->installationTime() != -1)
        {
            MegaApi *megaApi = new MegaApi(Preferences::CLIENT_KEY, (char *)NULL, Preferences::USER_AGENT);
            QString stats = QString::fromUtf8("{\"it\":%1,\"act\":%2,\"lt\":%3}")
                    .arg(preferences->installationTime())
                    .arg(preferences->accountCreationTime())
                    .arg(preferences->hasLoggedIn());

            QByteArray base64stats = stats.toUtf8().toBase64();
            base64stats.replace('+', '-');
            base64stats.replace('/', '_');
            while (base64stats.size() && base64stats[base64stats.size() - 1] == '=')
            {
                base64stats.resize(base64stats.size() - 1);
            }

            megaApi->sendEvent(AppStatsEvents::EVENT_INSTALL_STATS, base64stats.constData());
            Sleep(5000);
        }
#endif
        return 0;
    }

#ifdef Q_OS_LINUX

    // Ensure interesting signals are unblocked.
    sigset_t signalstounblock;
    sigemptyset (&signalstounblock);
    sigaddset(&signalstounblock, SIGUSR1);
    sigaddset(&signalstounblock, SIGUSR2);
    sigprocmask(SIG_UNBLOCK, &signalstounblock, NULL);

    if (signal(SIGUSR1, LinuxSignalHandler))
    {
        cerr << " Failed to register signal SIGUSR1 " << endl;
    }

    for (int i = 1; i < argc ; i++)
    {
        if (!strcmp(argv[i],"--waitforsignal"))
        {
            std::unique_lock<std::mutex> lock(mtxcondvar);
            if (signal(SIGUSR2, LinuxSignalHandler))
            {
                cerr << " Failed to register signal SIGUSR2 " << endl;
            }

            cout << "Waiting for signal to restart MEGAsync ... "<< endl;
            if (condVarRestart.wait_for(lock, std::chrono::minutes(30)) == std::cv_status::no_timeout )
            {
                QString app;

                for (int j = 0; j < argc; j++)
                {
                    if (strcmp(argv[j],"--waitforsignal"))
                    {
                        app.append(QString::fromUtf8(" \""));
                        app.append(QString::fromUtf8(argv[j]));
                        app.append(QString::fromUtf8("\""));
                    }
                }

                bool success = QProcess::startDetached(app);
                cout << "Restarting MEGAsync: " << app.toUtf8().constData() << " " << (success?"OK":"FAILED!") << endl;
                exit(!success);
            }
            cout << "Timed out waiting for restart signal" << endl;
            exit(2);
        }
    }

    // Block SIGUSR2 for normal execution: we don't want it to kill the process, in case there's a rogue update going on.
    sigset_t signalstoblock;
    sigemptyset (&signalstoblock);
    sigaddset(&signalstoblock, SIGUSR2);
    sigprocmask(SIG_BLOCK, &signalstoblock, NULL);
#endif

    // adds thread-safety to OpenSSL
    QSslSocket::supportsSsl();

#ifndef Q_OS_MACX
   const auto autoScreenScaleFactor = getenv("QT_AUTO_SCREEN_SCALE_FACTOR");
   const bool autoScreenScaleFactorDisabled{autoScreenScaleFactor && autoScreenScaleFactor == std::string("0")};
   if(autoScreenScaleFactorDisabled)
   {
       logMessages.emplace_back(MegaApi::LOG_LEVEL_DEBUG, QStringLiteral("auto screen scale factor disabled because of QT_AUTO_SCREEN_SCALE_FACTOR set to 0"));
   }
   else
   {
       QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
       QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
   }
#endif

#ifdef Q_OS_MACX

    bool harfbuzzEnabled = qputenv("QT_HARFBUZZ","old");

    // From QT (5.9) documentation:
    // Secure Transport SSL backend on macOS may update the default keychain (the default is probably your login keychain) by importing your local certificates and keys.
    // This can also result in system dialogs showing up and asking for permission when your application is using these private keys.
    // If such behavior is undesired, set the QT_SSL_USE_TEMPORARY_KEYCHAIN environment variable to a non-zero value this will prompt QSslSocket to use its own temporary keychain.
    bool useSSLtemporaryKeychain = qputenv("QT_SSL_USE_TEMPORARY_KEYCHAIN","1");

    qputenv("QT_BEARER_POLL_TIMEOUT", QByteArray::number(-1));

#endif

#if defined(Q_OS_LINUX)
    if (!(getenv("DO_NOT_SET_QT_PLUGIN_PATH")))
    {
        if (QDir(QString::fromUtf8("/opt/mega/plugins")).exists())
        {
            qputenv("QT_PLUGIN_PATH","/opt/mega/plugins");
        }
    }
#endif

#if defined(Q_OS_LINUX) && QT_VERSION >= 0x050C00
    // Linux && Qt >= 5.12.0
    if (!(getenv("DO_NOT_UNSET_XDG_SESSION_TYPE")))
    {
        if ( getenv("XDG_SESSION_TYPE") && !strcmp(getenv("XDG_SESSION_TYPE"),"wayland") )
        {
            std::cerr << "Avoiding wayland" << std::endl;
            unsetenv("XDG_SESSION_TYPE");
        }
    }
#endif

#ifndef Q_OS_MACX
#if defined(WIN32)
    ScaleFactorManager scaleFactorManager(OsType::WIN);
#endif

#if defined(Q_OS_LINUX)
    ScaleFactorManager scaleFactorManager(OsType::LINUX);
#endif

    try {
        scaleFactorManager.setScaleFactorEnvironmentVariable();
    } catch (const std::exception& exception)
    {
        const QString errorMessage{QString::fromStdString("Error while setting scale factor environment variable: "+
                    std::string(exception.what()))};
        logMessages.emplace_back(MegaApi::LOG_LEVEL_DEBUG, errorMessage);
    }
#endif

#if defined(Q_OS_LINUX)
#if QT_VERSION >= 0x050000
    if (!(getenv("DO_NOT_UNSET_QT_QPA_PLATFORMTHEME")) && getenv("QT_QPA_PLATFORMTHEME"))
    {
        if (!unsetenv("QT_QPA_PLATFORMTHEME")) //open folder dialog & similar crashes is fixed with this
        {
            std::cerr <<  "Error unsetting QT_QPA_PLATFORMTHEME vble" << std::endl;
        }
    }
    if (!(getenv("DO_NOT_UNSET_SHLVL")) && getenv("SHLVL"))
    {
        if (!unsetenv("SHLVL")) // reported failure in mint
        {
            //std::cerr <<  "Error unsetting SHLVL vble" << std::endl; //Fedora fails to unset this env var ... too verbose error
        }
    }
#endif
    if (!(getenv("DO_NOT_SET_DESKTOP_SETTINGS_UNAWARE")))
    {
        QApplication::setDesktopSettingsAware(false);
    }
#endif


    Platform::create();
    MegaApplication app(argc, argv);
#if defined(Q_OS_LINUX)
    theapp = &app;
    appToWaitForSignal = QString::fromUtf8("\"%1\"").arg(MegaApplication::applicationFilePath());
    for (int i = 1; i < argc; i++)
    {
        appToWaitForSignal.append(QString::fromUtf8(" \""));
        appToWaitForSignal.append(QString::fromUtf8(argv[i]));
        appToWaitForSignal.append(QString::fromUtf8("\""));
    }
#endif

    for(const auto &message : logMessages)
    {
        MegaApi::log(message.logLevel, message.message.toStdString().c_str());
    }

#ifndef Q_OS_MACX
    const auto scaleFactorLogMessages = scaleFactorManager.getLogMessages();
    for(const auto& message : scaleFactorLogMessages)
    {
        MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, message.c_str());
    }
#endif

#if defined(Q_OS_LINUX) && QT_VERSION >= 0x050600
    for (const auto& screen : app.screens())
    {
        MegaApi::log(MegaApi::LOG_LEVEL_INFO, ("Device pixel ratio on '" +
                                               screen->name().toStdString() + "': " +
                                               std::to_string(screen->devicePixelRatio())).c_str());
    }
#endif

    qInstallMsgHandler(msgHandler);
    qInstallMessageHandler(messageHandler);

    app.setStyle(new MegaProxyStyle());

#ifdef Q_OS_MACX

    auto current = QOperatingSystemVersion::current();
    MegaApi::log(MegaApi::LOG_LEVEL_INFO, QString::fromUtf8("Running on macOS version: %1.%2.%3")
                 .arg(current.majorVersion()).arg(current.minorVersion()).arg(current.microVersion())
                 .toUtf8().constData());

    if (!harfbuzzEnabled)
    {
       MegaApi::log(MegaApi::LOG_LEVEL_WARNING, "Error setting QT_HARFBUZZ vble");
    }

    if (!useSSLtemporaryKeychain)
    {
        MegaApi::log(MegaApi::LOG_LEVEL_WARNING, "Error setting QT_SSL_USE_TEMPORARY_KEYCHAIN vble");
    }

    app.setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

    QDir dataDir(app.applicationDataPath());
    QString crashPath = dataDir.filePath(QString::fromUtf8("crashDumps"));
    QString avatarPath = dataDir.filePath(QString::fromUtf8("avatars"));
    QString appLockPath = dataDir.filePath(QString::fromUtf8("megasync.lock"));
    QString appShowPath = dataDir.filePath(QString::fromUtf8("megasync.show"));
    QDir crashDir(crashPath);
    if (!crashDir.exists())
    {
        crashDir.mkpath(QString::fromUtf8("."));
    }

    QDir avatarsDir(avatarPath);
    if (!avatarsDir.exists())
    {
        avatarsDir.mkpath(QString::fromUtf8("."));
    }

#ifndef DEBUG
    CrashHandler::instance()->Init(QDir::toNativeSeparators(crashPath));
#endif

    QtLockedFile singleInstanceChecker(appLockPath);
    bool alreadyStarted = true;
    for (int i = 0; i < 10; i++)
    {
        if (i > 0)
        {
            if (dataDir.exists(appShowPath))
            {
                QFile appShowFile(appShowPath);
                if (appShowFile.open(QIODevice::ReadOnly))
                {
                    if (appShowFile.size() == 0)
                    {
                        // the file has been emptied; so the infoDialog was shown in the primary MEGAsync instance.  We can exit.
                        alreadyStarted = true;
                        break;
                    }
                }
            }
        }
        singleInstanceChecker.open(QtLockedFile::ReadWrite);
        if (singleInstanceChecker.lock(QtLockedFile::WriteLock, false))
        {
            alreadyStarted = false;
            break;
        }
        else if (i == 0)
        {
             QFile appShowFile(appShowPath);
             if (appShowFile.open(QIODevice::WriteOnly))
             {
                 appShowFile.write("open");
                 appShowFile.close();
             }
        }
#ifdef __APPLE__
        else if (i == 5)
        {
            QString appVersionPath = dataDir.filePath(QString::fromUtf8("megasync.version"));
            QFile fappVersionPath(appVersionPath);
            if (!fappVersionPath.exists())
            {
                QProcess::startDetached(QString::fromUtf8("/bin/bash -c \"lsof ~/Library/Application\\ Support/Mega\\ Limited/MEGAsync/megasync.lock 2>/dev/null | grep MEGAclien | cut -d' ' -f2 | xargs kill\""));
            }
        }
#endif

        #ifdef WIN32
            Sleep(1000);
        #else
            sleep(1);
        #endif
    }

    QString appVersionPath = dataDir.filePath(QString::fromUtf8("megasync.version"));
    QFile fappVersionPath(appVersionPath);
    if (fappVersionPath.open(QIODevice::WriteOnly))
    {
        fappVersionPath.write(QString::number(Preferences::VERSION_CODE).toUtf8());
        fappVersionPath.close();
    }

    if (alreadyStarted)
    {
        MegaApi::log(MegaApi::LOG_LEVEL_WARNING, "MEGAsync is already started");
        return 0;
    }
    Platform::getInstance()->initialize(argc, argv);

#if !defined(__APPLE__) && !defined (_WIN32)
    QFontDatabase::addApplicationFont(QString::fromUtf8("://fonts/OpenSans-Regular.ttf"));
    QFontDatabase::addApplicationFont(QString::fromUtf8("://fonts/OpenSans-Semibold.ttf"));

    QFont font(QString::fromUtf8("Open Sans"), 8);
    app.setFont(font);
#endif
    QFontDatabase::addApplicationFont(QString::fromUtf8("://fonts/SourceSansPro-Light.ttf"));
    QFontDatabase::addApplicationFont(QString::fromUtf8("://fonts/SourceSansPro-Bold.ttf"));
    QFontDatabase::addApplicationFont(QString::fromUtf8("://fonts/SourceSansPro-Regular.ttf"));
    QFontDatabase::addApplicationFont(QString::fromUtf8("://fonts/SourceSansPro-Semibold.ttf"));

    QFontDatabase::addApplicationFont(QString::fromUtf8("://fonts/Lato-Light.ttf"));
    QFontDatabase::addApplicationFont(QString::fromUtf8("://fonts/Lato-Bold.ttf"));
    QFontDatabase::addApplicationFont(QString::fromUtf8("://fonts/Lato-Regular.ttf"));
    QFontDatabase::addApplicationFont(QString::fromUtf8("://fonts/Lato-Semibold.ttf"));

    app.initialize();
    app.start();

    int toret = app.exec();

    Platform::destroy();
#ifdef WIN32
    extern bool WindowsPlatform_exiting;
    WindowsPlatform_exiting = true;
#endif

#ifdef Q_OS_LINUX
    theapp = nullptr;
#endif
    return toret;

#if 0 //Strings for the translation system. These lines don't need to be built
    QT_TRANSLATE_NOOP("QDialogButtonBox", "&Yes");
    QT_TRANSLATE_NOOP("QDialogButtonBox", "&No");
    QT_TRANSLATE_NOOP("QDialogButtonBox", "&OK");
    QT_TRANSLATE_NOOP("QDialogButtonBox", "&Cancel");
    QT_TRANSLATE_NOOP("QPlatformTheme", "&Yes");
    QT_TRANSLATE_NOOP("QPlatformTheme", "&No");
    QT_TRANSLATE_NOOP("QPlatformTheme", "OK");
    QT_TRANSLATE_NOOP("QPlatformTheme", "Cancel");

    QT_TRANSLATE_NOOP("QFileDialog", "Look in:");
    QT_TRANSLATE_NOOP("QFileDialog", "Back");
    QT_TRANSLATE_NOOP("QFileDialog", "Go back");
    QT_TRANSLATE_NOOP("QFileDialog", "Alt+Left");
    QT_TRANSLATE_NOOP("QFileDialog", "Forward");
    QT_TRANSLATE_NOOP("QFileDialog", "Go forward");
    QT_TRANSLATE_NOOP("QFileDialog", "Alt+Right");
    QT_TRANSLATE_NOOP("QFileDialog", "Parent Directory");
    QT_TRANSLATE_NOOP("QFileDialog", "Go to the parent directory");
    QT_TRANSLATE_NOOP("QFileDialog", "Alt+Up");
    QT_TRANSLATE_NOOP("QFileDialog", "Create New Folder");
    QT_TRANSLATE_NOOP("QFileDialog", "Create a New Folder");
    QT_TRANSLATE_NOOP("QFileDialog", "List View");
    QT_TRANSLATE_NOOP("QFileDialog", "Change to list view mode");
    QT_TRANSLATE_NOOP("QFileDialog", "Detail View");
    QT_TRANSLATE_NOOP("QFileDialog", "Change to detail view mode");
    QT_TRANSLATE_NOOP("QFileDialog", "Sidebar");
    QT_TRANSLATE_NOOP("QFileDialog", "List of places and bookmarks");
    QT_TRANSLATE_NOOP("QFileDialog", "Files");
    QT_TRANSLATE_NOOP("QFileDialog", "Files of type:");
    QT_TRANSLATE_NOOP("QFileDialog", "Find Directory");
    QT_TRANSLATE_NOOP("QFileDialog", "Open");
    QT_TRANSLATE_NOOP("QFileDialog", "Save As");
    QT_TRANSLATE_NOOP("QFileDialog", "Directory:");
    QT_TRANSLATE_NOOP("QFileDialog", "File &name:");
    QT_TRANSLATE_NOOP("QFileDialog", "&Open");
    QT_TRANSLATE_NOOP("QFileDialog", "&Choose");
    QT_TRANSLATE_NOOP("QFileDialog", "&Save");
    QT_TRANSLATE_NOOP("QFileDialog", "All Files (*)");
    QT_TRANSLATE_NOOP("QFileDialog", "Show ");
    QT_TRANSLATE_NOOP("QFileDialog", "&Rename");
    QT_TRANSLATE_NOOP("QFileDialog", "&Delete");
    QT_TRANSLATE_NOOP("QFileDialog", "Show &hidden files");
    QT_TRANSLATE_NOOP("QFileDialog", "&New Folder");
    QT_TRANSLATE_NOOP("QFileDialog", "All files (*)");
    QT_TRANSLATE_NOOP("QFileDialog", "Directories");
    QT_TRANSLATE_NOOP("QFileDialog", "%1\nDirectory not found.\nPlease verify the correct directory name was given.");
    QT_TRANSLATE_NOOP("QFileDialog", "%1 already exists.\nDo you want to replace it?");
    QT_TRANSLATE_NOOP("QFileDialog", "%1\nFile not found.\nPlease verify the correct file name was given.");
    QT_TRANSLATE_NOOP("QFileDialog", "New Folder");
    QT_TRANSLATE_NOOP("QFileDialog", "Delete");
    QT_TRANSLATE_NOOP("QFileDialog", "'%1' is write protected.\nDo you want to delete it anyway?");
    QT_TRANSLATE_NOOP("QFileDialog", "Are you sure you want to delete '%1'?");
    QT_TRANSLATE_NOOP("QFileDialog", "Could not delete directory.");
    QT_TRANSLATE_NOOP("QFileDialog", "Recent Places");
    QT_TRANSLATE_NOOP("QFileDialog", "Remove");
    QT_TRANSLATE_NOOP("QFileDialog", "My Computer");
    QT_TRANSLATE_NOOP("QFileDialog", "Drive");
    QT_TRANSLATE_NOOP("QFileDialog", "%1 File");
    QT_TRANSLATE_NOOP("QFileDialog", "File");
    QT_TRANSLATE_NOOP("QFileDialog", "File Folder");
    QT_TRANSLATE_NOOP("QFileDialog", "Folder");
    QT_TRANSLATE_NOOP("QFileDialog", "Alias");
    QT_TRANSLATE_NOOP("QFileDialog", "Shortcut");
    QT_TRANSLATE_NOOP("QFileDialog", "Unknown");

    QT_TRANSLATE_NOOP("QFileSystemModel", "%1 TB");
    QT_TRANSLATE_NOOP("QFileSystemModel", "%1 GB");
    QT_TRANSLATE_NOOP("QFileSystemModel", "%1 MB");
    QT_TRANSLATE_NOOP("QFileSystemModel", "%1 KB");
    QT_TRANSLATE_NOOP("QFileSystemModel", "%1 bytes");
    QT_TRANSLATE_NOOP("QFileSystemModel", "Invalid filename");
    QT_TRANSLATE_NOOP("QFileSystemModel", "<b>The name \"%1\" cannot be used.</b><p>Try using another name, with fewer characters or no punctuation marks.");
    QT_TRANSLATE_NOOP("QFileSystemModel", "Name");
    QT_TRANSLATE_NOOP("QFileSystemModel", "Size");
    QT_TRANSLATE_NOOP("QFileSystemModel", "Kind");
    QT_TRANSLATE_NOOP("QFileSystemModel", "Type");
    QT_TRANSLATE_NOOP("QFileSystemModel", "Date Modified");
    QT_TRANSLATE_NOOP("QFileSystemModel", "My Computer");
    QT_TRANSLATE_NOOP("QFileSystemModel", "Computer");

    QT_TRANSLATE_NOOP("Installer", "Choose Users");
    QT_TRANSLATE_NOOP("Installer", "Choose for which users you want to install $(^NameDA).");
    QT_TRANSLATE_NOOP("Installer", "Select whether you want to install $(^NameDA) for yourself only or for all users of this computer. $(^ClickNext)");
    QT_TRANSLATE_NOOP("Installer", "Install for anyone using this computer");
    QT_TRANSLATE_NOOP("Installer", "Install just for me");

    QT_TRANSLATE_NOOP("MegaError", "No error");
    QT_TRANSLATE_NOOP("MegaError", "Internal error");
    QT_TRANSLATE_NOOP("MegaError", "Invalid argument");
    QT_TRANSLATE_NOOP("MegaError", "Request failed, retrying");
    QT_TRANSLATE_NOOP("MegaError", "Rate limit exceeded");
    QT_TRANSLATE_NOOP("MegaError", "Failed permanently");
    QT_TRANSLATE_NOOP("MegaError", "Too many concurrent connections or transfers");
    QT_TRANSLATE_NOOP("MegaError", "Terms of Service breached");
    QT_TRANSLATE_NOOP("MegaError", "Not accessible due to ToS/AUP violation");
    QT_TRANSLATE_NOOP("MegaError", "Out of range");
    QT_TRANSLATE_NOOP("MegaError", "Expired");
    QT_TRANSLATE_NOOP("MegaError", "Not found");
    QT_TRANSLATE_NOOP("MegaError", "Circular linkage detected");
    QT_TRANSLATE_NOOP("MegaError", "Upload produces recursivity");
    QT_TRANSLATE_NOOP("MegaError", "Access denied");
    QT_TRANSLATE_NOOP("MegaError", "Already exists");
    QT_TRANSLATE_NOOP("MegaError", "Incomplete");
    QT_TRANSLATE_NOOP("MegaError", "Invalid key/Decryption error");
    QT_TRANSLATE_NOOP("MegaError", "Bad session ID");
    QT_TRANSLATE_NOOP("MegaError", "Blocked");
    QT_TRANSLATE_NOOP("MegaError", "Over quota");
    QT_TRANSLATE_NOOP("MegaError", "Temporarily not available");
    QT_TRANSLATE_NOOP("MegaError", "Connection overflow");
    QT_TRANSLATE_NOOP("MegaError", "Write error");
    QT_TRANSLATE_NOOP("MegaError", "Read error");
    QT_TRANSLATE_NOOP("MegaError", "Invalid application key");
    QT_TRANSLATE_NOOP("MegaError", "SSL verification failed");
    QT_TRANSLATE_NOOP("MegaError", "Not enough quota");
    QT_TRANSLATE_NOOP("MegaError", "Unknown error");
    QT_TRANSLATE_NOOP("MegaError", "Your account has been suspended due to copyright violations. Please check your email inbox.");
    QT_TRANSLATE_NOOP("MegaError", "Your account was terminated due to a breach of MEGA's Terms of Service, such as abuse of rights of others; sharing and/or importing illegal data; or system abuse.");
    QT_TRANSLATE_NOOP("MegaError", "Storage Quota Exceeded. Upgrade now");
    QT_TRANSLATE_NOOP("MegaError", "Decryption error");
    QT_TRANSLATE_NOOP("FinderExtensionApp", "Get MEGA link");
    QT_TRANSLATE_NOOP("FinderExtensionApp", "View on MEGA");
    QT_TRANSLATE_NOOP("FinderExtensionApp", "No options available");
    QT_TRANSLATE_NOOP("FinderExtensionApp", "Click the toolbar item for a menu.");
    QT_TRANSLATE_NOOP("FinderExtensionApp", "%n file", "", n);
    QT_TRANSLATE_NOOP("FinderExtensionApp", "%n folder", "", n);
    QT_TRANSLATE_NOOP("FinderExtensionApp", "View previous versions");
    QT_TRANSLATE_NOOP("MegaSyncError", "No error");
    QT_TRANSLATE_NOOP("MegaSyncError", "Unknown error");
    QT_TRANSLATE_NOOP("MegaSyncError", "File system not supported");
    QT_TRANSLATE_NOOP("MegaSyncError", "Remote node is not valid");
    QT_TRANSLATE_NOOP("MegaSyncError", "Local path is not valid");
    QT_TRANSLATE_NOOP("MegaSyncError", "Initial scan failed");
    QT_TRANSLATE_NOOP("MegaSyncError", "Local path temporarily unavailable");
    QT_TRANSLATE_NOOP("MegaSyncError", "Local path not available");
    QT_TRANSLATE_NOOP("MegaSyncError", "Remote node not found");
    QT_TRANSLATE_NOOP("MegaSyncError", "Reached storage quota limit");
    QT_TRANSLATE_NOOP("MegaSyncError", "Account expired (business or Pro Flexi)");
    QT_TRANSLATE_NOOP("MegaSyncError", "Foreign target storage quota reached");
    QT_TRANSLATE_NOOP("MegaSyncError", "Remote path has changed");
    QT_TRANSLATE_NOOP("MegaSyncError", "Remote node moved to Rubbish Bin");
    QT_TRANSLATE_NOOP("MegaSyncError", "Share without full access");
    QT_TRANSLATE_NOOP("MegaSyncError", "Local fingerprint mismatch");
    QT_TRANSLATE_NOOP("MegaSyncError", "Put nodes error");
    QT_TRANSLATE_NOOP("MegaSyncError", "Active sync below path");
    QT_TRANSLATE_NOOP("MegaSyncError", "Active sync above path");
    QT_TRANSLATE_NOOP("MegaSyncError", "Remote node has been deleted");
    QT_TRANSLATE_NOOP("MegaSyncError", "Remote node is inside Rubbish Bin");
    QT_TRANSLATE_NOOP("MegaSyncError", "Unsupported VBoxSharedFolderFS filesystem");
    QT_TRANSLATE_NOOP("MegaSyncError", "Local path collides with an existing sync");
    QT_TRANSLATE_NOOP("MegaSyncError", "Local filesystem is FAT");
    QT_TRANSLATE_NOOP("MegaSyncError", "Local filesystem is HGFS");
    QT_TRANSLATE_NOOP("MegaSyncError", "Your account is blocked");
    QT_TRANSLATE_NOOP("MegaSyncError", "Unknown temporary error");
    QT_TRANSLATE_NOOP("MegaSyncError", "Too many changes in account, local state invalid");
    QT_TRANSLATE_NOOP("MegaSyncError", "Undefined error");
#endif
}

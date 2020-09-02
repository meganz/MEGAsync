#include "mega/types.h"
#include "MegaApplication.h"
#include "gui/CrashReportDialog.h"
#include "gui/MegaProxyStyle.h"
#include "gui/ConfirmSSLexception.h"
#include "gui/QMegaMessageBox.h"
#include "control/Utilities.h"
#include "control/CrashHandler.h"
#include "control/ExportProcessor.h"
#include "platform/Platform.h"
#include "qtlockedfile/qtlockedfile.h"
#include "OverQuotaDialog.h"

#include <QTranslator>
#include <QClipboard>
#include <QDesktopWidget>
#include <QFontDatabase>
#include <QNetworkProxy>
#include <QSettings>
#include <QToolTip>

#include <assert.h>

#ifdef Q_OS_LINUX
    #include <signal.h>
    #include <condition_variable>
    #include <QSvgRenderer>
#endif

#if QT_VERSION >= 0x050000
#include <QtConcurrent/QtConcurrent>
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

#if ( defined(WIN32) && QT_VERSION >= 0x050000 ) || (defined(Q_OS_LINUX) && QT_VERSION >= 0x050600)
#include <QScreen>
#endif

using namespace mega;
using namespace std;

QString MegaApplication::appPath = QString();
QString MegaApplication::appDirPath = QString();
QString MegaApplication::dataPath = QString();
QString MegaApplication::lastNotificationError = QString();

void msgHandler(QtMsgType type, const char *msg)
{
    switch (type) {
    case QtDebugMsg:
        MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, QString::fromUtf8("QT Debug: %1").arg(QString::fromUtf8(msg)).toUtf8().constData());
        break;
    case QtWarningMsg:
        MegaApi::log(MegaApi::LOG_LEVEL_WARNING, QString::fromUtf8("QT Warning: %1").arg(QString::fromUtf8(msg)).toUtf8().constData());
        break;
    case QtCriticalMsg:
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("QT Critical: %1").arg(QString::fromUtf8(msg)).toUtf8().constData());
        break;
    case QtFatalMsg:
        MegaApi::log(MegaApi::LOG_LEVEL_FATAL, QString::fromUtf8("QT FATAL ERROR: %1").arg(QString::fromUtf8(msg)).toUtf8().constData());
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
            theapp->exitApplication(true);
        }
    }
}

#endif

#if QT_VERSION >= 0x050000
    void messageHandler(QtMsgType type,const QMessageLogContext &context, const QString &msg)
    {
        switch (type) {
        case QtDebugMsg:
            MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, QString::fromUtf8("QT Debug: %1").arg(msg).toUtf8().constData());
            MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, QString::fromUtf8("QT Context: %1 %2 %3 %4 %5")
                         .arg(QString::fromUtf8(context.category))
                         .arg(QString::fromUtf8(context.file))
                         .arg(QString::fromUtf8(context.function))
                         .arg(QString::fromUtf8(context.file))
                         .arg(context.version).toUtf8().constData());
            break;
        case QtWarningMsg:
            MegaApi::log(MegaApi::LOG_LEVEL_WARNING, QString::fromUtf8("QT Warning: %1").arg(msg).toUtf8().constData());
            MegaApi::log(MegaApi::LOG_LEVEL_WARNING, QString::fromUtf8("QT Context: %1 %2 %3 %4 %5")
                         .arg(QString::fromUtf8(context.category))
                         .arg(QString::fromUtf8(context.file))
                         .arg(QString::fromUtf8(context.function))
                         .arg(QString::fromUtf8(context.file))
                         .arg(context.version).toUtf8().constData());
            break;
        case QtCriticalMsg:
            MegaApi::log(MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("QT Critical: %1").arg(msg).toUtf8().constData());
            MegaApi::log(MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("QT Context: %1 %2 %3 %4 %5")
                         .arg(QString::fromUtf8(context.category))
                         .arg(QString::fromUtf8(context.file))
                         .arg(QString::fromUtf8(context.function))
                         .arg(QString::fromUtf8(context.file))
                         .arg(context.version).toUtf8().constData());
            break;
        case QtFatalMsg:
            MegaApi::log(MegaApi::LOG_LEVEL_FATAL, QString::fromUtf8("QT FATAL ERROR: %1").arg(msg).toUtf8().constData());
            MegaApi::log(MegaApi::LOG_LEVEL_FATAL, QString::fromUtf8("QT Context: %1 %2 %3 %4 %5")
                         .arg(QString::fromUtf8(context.category))
                         .arg(QString::fromUtf8(context.file))
                         .arg(QString::fromUtf8(context.function))
                         .arg(QString::fromUtf8(context.file))
                         .arg(context.version).toUtf8().constData());
            break;
        }
    }
#endif

#if ( defined(WIN32) && QT_VERSION >= 0x050000 ) || (defined(Q_OS_LINUX) && QT_VERSION >= 0x050600)
namespace {

constexpr auto dpiScreensSuitableIncrement = 1. / 6.; // this seems to work fine with 24x24 images at least
#ifdef Q_OS_LINUX

double getXrdbdpi( bool enforce = false)
{
    static int calculated = 0;
    if (calculated && !enforce) //avoid multiple calls
    {
        return calculated;
    }

    QProcess p;
    p.start(QString::fromUtf8("bash -c \"xrdb -query | grep dpi | awk '{print $2}'\""));
    p.waitForFinished(2000);
    QString output = QString::fromUtf8(p.readAllStandardOutput().constData()).trimmed();
    QString e = QString::fromUtf8(p.readAllStandardError().constData());
    if (e.size())
    {
        qDebug() << "Error for \"xrdb -query\" command:" << e;
    }

    calculated = qRound(output.toDouble());
    return calculated;
}

double computeScale(const QScreen& screen)
{
    constexpr auto base_dpi = 96.;
    auto scale = 1.;
    auto screendpi = getXrdbdpi(); //the best cross platform solution found (caveat: screen agnostic)
    if (screendpi <= 0) //failsafe: in case xrdb fails to retrieve a valid value
    {
        screendpi = screen.logicalDotsPerInch(); //Use Qt to get dpi value (faulty in certain environments)
    }

    if (screendpi > base_dpi) // high dpi screen | zoom configured ...
    {
        scale = screendpi / base_dpi;
        scale = min(3., scale);
    }
    else // low dpi screen
    {
        const auto geom = screen.availableGeometry();
        scale = min(geom.width() / 1920., geom.height() / 1080.) * 0.75;
        scale = max(1., scale);
    }

    scale = qRound(scale / dpiScreensSuitableIncrement) * dpiScreensSuitableIncrement;

    return scale;
}
#endif

void setScreenScaleFactorsEnvVar(const QMap<QString, double> &screenscales)
{
    QString scale_factors;
    for (auto ss = screenscales.begin(); ss != screenscales.end(); ++ss)
    {
        if (scale_factors.size())
        {
            scale_factors += QString::fromAscii(";");
        }
        scale_factors += ss.key() + QString::fromAscii("=") + QString::number(ss.value());
    }

    if (scale_factors.size())
    {
        qDebug() << "Setting QT_SCREEN_SCALE_FACTORS=" << scale_factors;
        qputenv("QT_SCREEN_SCALE_FACTORS", scale_factors.toAscii());
    }
    else
    {
        assert(false && "No screen found");
    }

    return;
}

bool adjustScreenScaleFactors(QMap<QString, double> &screenscales)
{
    constexpr auto minTitleBarHeight = 20; // give some pixels to the tittle bar
    constexpr auto biggestDialogHeight = minTitleBarHeight + 600; //This is the height of the biggest dialog in megassync (Settings)

    bool adjusted = false;

    for (auto ssname : screenscales.keys())
    {
        if (screenscales[ssname] > 1)
        {
            auto &ssvalue = screenscales[ssname];
            auto sprevious = ssvalue;

            do
            {
                sprevious = ssvalue;

                int argc = 0;
                QGuiApplication app{argc, nullptr};
                const auto screens = app.screens();
                for (const auto& s : screens)
                {
                    if (s->name() == ssname)
                    {
                        auto height = s->availableGeometry().height();

                        if (biggestDialogHeight > height)
                        {
                            ssvalue = max(1., ssvalue - dpiScreensSuitableIncrement); //Qt don't like scale factors below 1
                            qDebug() << "Screen \"" << ssname << "\" too small for calculated scaling, reducing from " << sprevious << " to " << ssvalue;
                            setScreenScaleFactorsEnvVar(screenscales);
#if !defined(Q_OS_LINUX)
                            QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling, false);
#endif
                            adjusted = true;
                        }
                        break;
                    }
                }
            } while(screenscales[ssname] > 1 && ssvalue != sprevious);
        }
    }

    return adjusted;
}

void setScaleFactors()
{
    if (getenv("QT_SCALE_FACTOR"))
    {
        qDebug() << "Not setting scale factors. Using predefined QT_SCALE_FACTOR=" << getenv("QT_SCALE_FACTOR");
#if !defined(Q_OS_LINUX)
        QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling, false);
#endif
        return;
    }

    if (getenv("QT_SCREEN_SCALE_FACTORS"))
    {
        const QString predefScreenScaleFactors = QString::fromUtf8(getenv("QT_SCREEN_SCALE_FACTORS"));
        int argc = 0;
        QGuiApplication app{argc, nullptr};
        const auto screens = app.screens();
        bool screen_scale_factors_valid = predefScreenScaleFactors.size();
        for (const auto& screen : screens)
        {
            if (!predefScreenScaleFactors.contains(screen->name()))
            {
                screen_scale_factors_valid = false;
                break;
            }
#if !defined(Q_OS_LINUX)
            else
            {
                QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling, false);
            }
#endif
        }

        if (screen_scale_factors_valid)
        {
            qDebug() << "Not setting scale factors. Using predefined QT_SCREEN_SCALE_FACTORS=" << getenv("QT_SCREEN_SCALE_FACTORS");
            return;
        }
    }

    QMap<QString, double> screenscales;

    {
        int argc = 0;
        QGuiApplication app{argc, nullptr};
        const auto screens = app.screens();
        for (const auto& screen : screens)
        {
#ifdef Q_OS_LINUX
            const double computed_scale = computeScale(*screen);
#else
            // In windows, devicePixelRatio is calculated according to zoom level when AA_EnableHighDpiScaling
            const double computed_scale = screen->devicePixelRatio();
#endif
            screenscales.insert(screen->name(), computed_scale);
        }
    }

#ifdef Q_OS_LINUX
    setScreenScaleFactorsEnvVar(screenscales);
#endif
    if (adjustScreenScaleFactors(screenscales))
    {
        qDebug() << "Some screen is too small to apply automatic DPI scaling, enforced QT_SCREEN_SCALE_FACTORS=" << QString::fromUtf8(getenv("QT_SCREEN_SCALE_FACTORS"));;
    }
}

}
#endif


void removeSyncData(const QString &localFolder, const QString & name, const QString &syncID)
{
    Platform::syncFolderRemoved(localFolder, name, syncID);

    #ifdef WIN32
    // unhide debris folder
    QString debrisPath = QDir::toNativeSeparators(localFolder +
            QDir::separator() + QString::fromAscii(MEGA_DEBRIS_FOLDER));

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
    QCoreApplication::setOrganizationName(QString::fromAscii("Mega Limited"));
    QCoreApplication::setOrganizationDomain(QString::fromAscii("mega.co.nz"));
    QCoreApplication::setApplicationName(QString::fromAscii("MEGAsync"));

    if ((argc == 2) && !strcmp("/uninstall", argv[1]))
    {
        Preferences *preferences = Preferences::instance();
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
                    removeSyncData(osd.mLocalFolder, osd.mName, osd.mSyncID);
                }

                // now for the new syncs cached configurations
                auto loadedSyncs = preferences->getLoadedSyncsMap();
                for (auto it = loadedSyncs.begin(); it != loadedSyncs.end(); it++)
                {
                    removeSyncData(it.value()->getLocalFolder(), it.value()->name(), it.value()->getSyncID());
                }

                preferences->leaveUser();
            }
        }

        Utilities::removeRecursively(MegaApplication::applicationDataPath());
        Platform::uninstall();

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

            megaApi->sendEvent(99504, base64stats.constData());
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
#if QT_VERSION >= 0x050600
#if !defined(Q_OS_LINUX)
   QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
   QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
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

#if defined(Q_OS_LINUX) && QT_VERSION >= 0x050600
    if (!(getenv("DO_NOT_SET_QT_PLUGIN_PATH")))
    {
        if (QDir(QString::fromUtf8("/opt/mega/plugins")).exists())
        {
            qputenv("QT_PLUGIN_PATH","/opt/mega/plugins");
        }
    }

    if (!(getenv("DO_NOT_OVERRIDE_XDG_CURRENT_DESKTOP")))
    {
        if (getenv("XDG_CURRENT_DESKTOP") && !strcmp(getenv("XDG_CURRENT_DESKTOP"),"KDE") && (!getenv("XDG_SESSION_TYPE") || strcmp(getenv("XDG_SESSION_TYPE"),"wayland") ) )
        {
            qputenv("XDG_CURRENT_DESKTOP","GNOME");
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

#if ( defined(WIN32) && QT_VERSION >= 0x050000 ) || (defined(Q_OS_LINUX) && QT_VERSION >= 0x050600)
    setScaleFactors();
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

#if defined(Q_OS_LINUX) && QT_VERSION >= 0x050600
    for (const auto& screen : app.screens())
    {
        MegaApi::log(MegaApi::LOG_LEVEL_INFO, ("Device pixel ratio on '" +
                                               screen->name().toStdString() + "': " +
                                               std::to_string(screen->devicePixelRatio())).c_str());
    }
#endif

    qInstallMsgHandler(msgHandler);
#if QT_VERSION >= 0x050000
    qInstallMessageHandler(messageHandler);
#endif

    app.setStyle(new MegaProxyStyle());

#ifdef Q_OS_MACX

    MegaApi::log(MegaApi::LOG_LEVEL_INFO, QString::fromUtf8("Running on macOS version: %1").arg(QString::number(QSysInfo::MacintoshVersion)).toUtf8().constData());

    if (!harfbuzzEnabled)
    {
       MegaApi::log(MegaApi::LOG_LEVEL_WARNING, "Error setting QT_HARFBUZZ vble");
    }

    if (!useSSLtemporaryKeychain)
    {
        MegaApi::log(MegaApi::LOG_LEVEL_WARNING, "Error setting QT_SSL_USE_TEMPORARY_KEYCHAIN vble");
    }

    if (QSysInfo::MacintoshVersion > QSysInfo::MV_10_8)
    {
        // fix Mac OS X 10.9 (mavericks) font issue
        // https://bugreports.qt-project.org/browse/QTBUG-32789
        QFont::insertSubstitution(QString::fromUtf8(".Lucida Grande UI"), QString::fromUtf8("Lucida Grande"));
    }

    app.setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

    QDir dataDir(app.applicationDataPath());
    QString crashPath = dataDir.filePath(QString::fromAscii("crashDumps"));
    QString avatarPath = dataDir.filePath(QString::fromAscii("avatars"));
    QString appLockPath = dataDir.filePath(QString::fromAscii("megasync.lock"));
    QString appShowPath = dataDir.filePath(QString::fromAscii("megasync.show"));
    QDir crashDir(crashPath);
    if (!crashDir.exists())
    {
        crashDir.mkpath(QString::fromAscii("."));
    }

    QDir avatarsDir(avatarPath);
    if (!avatarsDir.exists())
    {
        avatarsDir.mkpath(QString::fromAscii("."));
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
            QString appVersionPath = dataDir.filePath(QString::fromAscii("megasync.version"));
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

    QString appVersionPath = dataDir.filePath(QString::fromAscii("megasync.version"));
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
    Platform::initialize(argc, argv);

#if !defined(__APPLE__) && !defined (_WIN32)
    QFontDatabase::addApplicationFont(QString::fromAscii("://fonts/OpenSans-Regular.ttf"));
    QFontDatabase::addApplicationFont(QString::fromAscii("://fonts/OpenSans-Semibold.ttf"));

    QFont font(QString::fromAscii("Open Sans"), 8);
    app.setFont(font);
#endif
    QFontDatabase::addApplicationFont(QString::fromAscii("://fonts/SourceSansPro-Light.ttf"));
    QFontDatabase::addApplicationFont(QString::fromAscii("://fonts/SourceSansPro-Bold.ttf"));
    QFontDatabase::addApplicationFont(QString::fromAscii("://fonts/SourceSansPro-Regular.ttf"));
    QFontDatabase::addApplicationFont(QString::fromAscii("://fonts/SourceSansPro-Semibold.ttf"));

    QFontDatabase::addApplicationFont(QString::fromAscii("://fonts/Lato-Light.ttf"));
    QFontDatabase::addApplicationFont(QString::fromAscii("://fonts/Lato-Bold.ttf"));
    QFontDatabase::addApplicationFont(QString::fromAscii("://fonts/Lato-Regular.ttf"));
    QFontDatabase::addApplicationFont(QString::fromAscii("://fonts/Lato-Semibold.ttf"));

    app.initialize();
    app.start();

    int toret = app.exec();


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
    QT_TRANSLATE_NOOP("QFileDialog", "File &amp;name:");
    QT_TRANSLATE_NOOP("QFileDialog", "&amp;Open");
    QT_TRANSLATE_NOOP("QFileDialog", "&amp;Choose");
    QT_TRANSLATE_NOOP("QFileDialog", "&amp;Save");
    QT_TRANSLATE_NOOP("QFileDialog", "All Files (*)");
    QT_TRANSLATE_NOOP("QFileDialog", "Show ");
    QT_TRANSLATE_NOOP("QFileDialog", "&amp;Rename");
    QT_TRANSLATE_NOOP("QFileDialog", "&amp;Delete");
    QT_TRANSLATE_NOOP("QFileDialog", "Show &amp;hidden files");
    QT_TRANSLATE_NOOP("QFileDialog", "&amp;New Folder");
    QT_TRANSLATE_NOOP("QFileDialog", "All files (*)");
    QT_TRANSLATE_NOOP("QFileDialog", "Directories");
    QT_TRANSLATE_NOOP("QFileDialog", "%1 Please verify the correct directory name was given.");
    QT_TRANSLATE_NOOP("QFileDialog", "%1 already exists.Do you want to replace it?");
    QT_TRANSLATE_NOOP("QFileDialog", "%1 Please verify the correct file name was given.");
    QT_TRANSLATE_NOOP("QFileDialog", "New Folder");
    QT_TRANSLATE_NOOP("QFileDialog", "Delete");
    QT_TRANSLATE_NOOP("QFileDialog", "&apos;%1&apos; is write protected. Do you want to delete it anyway?");
    QT_TRANSLATE_NOOP("QFileDialog", "Are you sure you want to delete &apos;%1&apos;?");
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
    QT_TRANSLATE_NOOP("QFileSystemModel", "&lt;b&gt;The name &quot;%1&quot; can not be used.&lt;/b&gt;&lt;p&gt;Try using another name, with fewer characters or no punctuations marks.");
    QT_TRANSLATE_NOOP("QFileSystemModel", "Name");
    QT_TRANSLATE_NOOP("QFileSystemModel", "Size");
    QT_TRANSLATE_NOOP("QFileSystemModel", "Kind");
    QT_TRANSLATE_NOOP("QFileSystemModel", "Type");
    QT_TRANSLATE_NOOP("QFileSystemModel", "Date Modified");
    QT_TRANSLATE_NOOP("QFileSystemModel", "My Computer");
    QT_TRANSLATE_NOOP("QFileSystemModel", "Computer");
    QT_TRANSLATE_NOOP("QFileSystemModel", "%1 byte(s)");

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
    QT_TRANSLATE_NOOP("MegaError", "Your account has been suspended due to multiple breaches of MEGA’s Terms of Service. Please check your email inbox.");
    QT_TRANSLATE_NOOP("MegaError", "Your account was terminated due to breach of Mega’s Terms of Service, such as abuse of rights of others; sharing and/or importing illegal data; or system abuse.");
    QT_TRANSLATE_NOOP("FinderExtensionApp", "Get MEGA link");
    QT_TRANSLATE_NOOP("FinderExtensionApp", "View on MEGA");
    QT_TRANSLATE_NOOP("FinderExtensionApp", "No options available");
    QT_TRANSLATE_NOOP("FinderExtensionApp", "Click the toolbar item for a menu.");
    QT_TRANSLATE_NOOP("FinderExtensionApp", "1 file");
    QT_TRANSLATE_NOOP("FinderExtensionApp", "%i files");
    QT_TRANSLATE_NOOP("FinderExtensionApp", "1 folder");
    QT_TRANSLATE_NOOP("FinderExtensionApp", "%i folders");
    QT_TRANSLATE_NOOP("FinderExtensionApp", "View previous versions");
    QT_TRANSLATE_NOOP("MegaNodeNames", "Cloud Drive");
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
    QT_TRANSLATE_NOOP("MegaSyncError", "Business account expired");
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

void MegaApplication::loadDataPath()
{
#if QT_VERSION < 0x050000
    dataPath = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
#else
#ifdef Q_OS_LINUX
    dataPath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QString::fromUtf8("/data/Mega Limited/MEGAsync");
#else
    QStringList dataPaths = QStandardPaths::standardLocations(QStandardPaths::DataLocation);
    if (dataPaths.size())
    {
        dataPath = dataPaths.at(0);
    }
#endif
#endif

    if (dataPath.isEmpty())
    {
        dataPath = QDir::currentPath();
    }

    dataPath = QDir::toNativeSeparators(dataPath);
    QDir currentDir(dataPath);
    if (!currentDir.exists())
    {
        currentDir.mkpath(QString::fromAscii("."));
    }
}

MegaApplication::MegaApplication(int &argc, char **argv) :
    QApplication(argc, argv)
{
#ifdef _WIN32
    for (QScreen *s: this->screens() )
    {
        lastCheckedScreens.insert(s->name(), s->devicePixelRatio());
    }
#endif
    appfinished = false;

    bool logToStdout = false;

#if defined(LOG_TO_STDOUT)
    logToStdout = true;
#endif

#ifdef Q_OS_LINUX
    if (argc == 2)
    {
         if (!strcmp("--debug", argv[1]))
         {
             logToStdout = true;
         }
         else if (!strcmp("--version", argv[1]))
         {
            QTextStream(stdout) << "MEGAsync" << " v" << Preferences::VERSION_STRING << " (" << Preferences::SDK_ID << ")" << endl;
            ::exit(0);
         }
    }
#endif


    connect(this, SIGNAL(blocked()), this, SLOT(onBlocked()));
    connect(this, SIGNAL(unblocked()), this, SLOT(onUnblocked()));

#ifdef _WIN32
    connect(this, SIGNAL(screenAdded(QScreen *)), this, SLOT(changeDisplay(QScreen *)));
    connect(this, SIGNAL(screenRemoved(QScreen *)), this, SLOT(changeDisplay(QScreen *)));
#endif

    //Set QApplication fields
    setOrganizationName(QString::fromAscii("Mega Limited"));
    setOrganizationDomain(QString::fromAscii("mega.co.nz"));
    setApplicationName(QString::fromAscii("MEGAsync"));
    setApplicationVersion(QString::number(Preferences::VERSION_CODE));

#ifdef _WIN32
    setStyleSheet(QString::fromUtf8("QMessageBox QLabel {font-size: 13px;}"
                                    "QMessageBox QPushButton "
                                    "{font-size: 13px; padding-right: 12px;"
                                    "padding-left: 12px;}"
                                    "QMenu {font-size: 13px;}"
                                    "QToolTip {font-size: 13px;}"
                                    "QFileDialog QPushButton "
                                    "{font-size: 13px; padding-right: 12px;"
                                    "padding-left: 12px;}"
                                    "QFileDialog QWidget"
                                    "{font-size: 13px;}"));
#endif

    QPalette palette = QToolTip::palette();
    palette.setColor(QPalette::ToolTipBase,QColor("#333333"));
    palette.setColor(QPalette::ToolTipText,QColor("#FAFAFA"));
    QToolTip::setPalette(palette);

    appPath = QDir::toNativeSeparators(QCoreApplication::applicationFilePath());
    appDirPath = QDir::toNativeSeparators(QCoreApplication::applicationDirPath());

    //Set the working directory
    QDir::setCurrent(MegaApplication::applicationDataPath());

    QString desktopPath;
#if QT_VERSION < 0x050000
    desktopPath = QDesktopServices::storageLocation(QDesktopServices::DesktopLocation);
#else
    QStringList desktopPaths = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation);
    if (desktopPaths.size())
    {
        desktopPath = desktopPaths.at(0);
    }
    else
    {
        desktopPath = Utilities::getDefaultBasePath();
    }
#endif

    logger.reset(new MegaSyncLogger(this, dataPath, desktopPath, logToStdout));
#if defined(LOG_TO_FILE)
    logger->setDebug(true);
#endif

    updateAvailable = false;
    networkConnectivity = true;
    activeTransferPriority[MegaTransfer::TYPE_DOWNLOAD] = 0xFFFFFFFFFFFFFFFFULL;
    activeTransferPriority[MegaTransfer::TYPE_UPLOAD] = 0xFFFFFFFFFFFFFFFFULL;
    activeTransferState[MegaTransfer::TYPE_DOWNLOAD] = MegaTransfer::STATE_NONE;
    activeTransferState[MegaTransfer::TYPE_UPLOAD] = MegaTransfer::STATE_NONE;
    activeTransferTag[MegaTransfer::TYPE_DOWNLOAD] = 0;
    activeTransferTag[MegaTransfer::TYPE_UPLOAD] = 0;
    trayIcon = NULL;
    verifyEmail = nullptr;
    infoDialogMenu = NULL;
    guestMenu = NULL;
    syncsMenu = NULL;
    menuSignalMapper = NULL;
    megaApi = NULL;
    megaApiFolders = NULL;
    delegateListener = NULL;
    httpServer = NULL;
    httpsServer = NULL;
    numTransfers[MegaTransfer::TYPE_DOWNLOAD] = 0;
    numTransfers[MegaTransfer::TYPE_UPLOAD] = 0;
    exportOps = 0;
    infoDialog = NULL;
    blockState = MegaApi::ACCOUNT_NOT_BLOCKED;
    blockStateSet = false;
    setupWizard = NULL;
    settingsDialog = NULL;
    streamSelector = NULL;
    reboot = false;
    exitAction = NULL;
    exitActionGuest = NULL;
    settingsAction = NULL;
    settingsActionGuest = NULL;
    importLinksAction = NULL;
    initialMenu = NULL;
    lastHovered = NULL;
    isPublic = false;
    prevVersion = 0;
    updatingSSLcert = false;
    lastSSLcertUpdate = 0;

    notificationsModel = NULL;
    notificationsProxyModel = NULL;
    notificationsDelegate = NULL;

#ifdef _WIN32
    windowsMenu = NULL;
    windowsExitAction = NULL;
    windowsUpdateAction = NULL;
    windowsImportLinksAction = NULL;
    windowsUploadAction = NULL;
    windowsDownloadAction = NULL;
    windowsStreamAction = NULL;
    windowsTransferManagerAction = NULL;
    windowsSettingsAction = NULL;

    WCHAR commonPath[MAX_PATH + 1];
    if (SHGetSpecialFolderPathW(NULL, commonPath, CSIDL_COMMON_APPDATA, FALSE))
    {
        int len = lstrlen(commonPath);
        if (!memcmp(commonPath, (WCHAR *)appDirPath.utf16(), len * sizeof(WCHAR))
                && appDirPath.size() > len && appDirPath[len] == QChar::fromAscii('\\'))
        {
            isPublic = true;

            int intVersion = 0;
            QDir dataDir(dataPath);
            QString appVersionPath = dataDir.filePath(QString::fromAscii("megasync.version"));
            QFile f(appVersionPath);
            if (f.open(QFile::ReadOnly | QFile::Text))
            {
                QTextStream in(&f);
                QString version = in.readAll();
                intVersion = version.toInt();
            }

            prevVersion = intVersion;
        }
    }

#endif
    changeProxyAction = NULL;
    initialExitAction = NULL;
    uploadAction = NULL;
    downloadAction = NULL;
    streamAction = NULL;
    myCloudAction = NULL;
    addSyncAction = NULL;
    waiting = false;
    updated = false;
    syncing = false;
    transferring = false;
    checkupdate = false;
    updateAction = NULL;
    updateActionGuest = NULL;
    showStatusAction = NULL;
    pasteMegaLinksDialog = NULL;
    changeLogDialog = NULL;
    importDialog = NULL;
    uploadFolderSelector = NULL;
    downloadFolderSelector = NULL;
    fileUploadSelector = NULL;
    folderUploadSelector = NULL;
    updateBlocked = false;
    updateThread = NULL;
    updateTask = NULL;
    multiUploadFileDialog = NULL;
    exitDialog = NULL;
    sslKeyPinningError = NULL;
    downloadNodeSelector = NULL;
    notificator = NULL;
    pricing = NULL;
    storageOverquotaDialog = NULL;
    infoWizard = NULL;
    noKeyDetected = 0;
    isFirstFileSynced = false;
    transferManager = NULL;
    cleaningSchedulerExecution = 0;
    lastUserActivityExecution = 0;
    lastTsBusinessWarning = 0;
    lastTsErrorMessageShown = 0;
    maxMemoryUsage = 0;
    nUnviewedTransfers = 0;
    completedTabActive = false;
    nodescurrent = false;
    mFetchingNodes = false;
    mQueringWhyAmIBlocked = false;
    whyamiblockedPeriodicPetition = false;
    getUserDataRequestReady = false;
    storageState = MegaApi::STORAGE_STATE_UNKNOWN;
    appliedStorageState = MegaApi::STORAGE_STATE_UNKNOWN;
    transferOverQuotaWaitTimeExpiredReceived = false;

    for (unsigned i = 3; i--; )
    {
        inflightUserStats[i] = false;
        userStatsLastRequest[i] = 0;
        queuedUserStats[i] = false;
    }
    queuedStorageUserStatsReason = 0;

#ifdef __APPLE__
    scanningTimer = NULL;
#endif
}

MegaApplication::~MegaApplication()
{
    logger.reset();

    if (!translator.isEmpty())
    {
        removeTranslator(&translator);
    }
    delete pricing;
}

void MegaApplication::showInterface(QString)
{
    if (appfinished)
    {
        return;
    }

    bool show = true;

    QDir dataDir(dataPath);
    if (dataDir.exists(QString::fromAscii("megasync.show")))
    {
        QFile showFile(dataDir.filePath(QString::fromAscii("megasync.show"))); 
        if (showFile.open(QIODevice::ReadOnly))
        {
            show = showFile.size() > 0;
            if (show)
            {
                // clearing the file content will cause the instance that asked us to show the dialog to exit
                showFile.close();
                showFile.open(QIODevice::WriteOnly | QIODevice::Truncate);
                showFile.close();
            }
        }
    }

    if (show)
    {
        // we saw the file had bytes in it, or if anything went wrong when trying to check that
        showInfoDialog();
        //If the dialog is active and visible -> show it
        if (settingsDialog && settingsDialog->isVisible())
        {
            settingsDialog->activateWindow();
            settingsDialog->raise();
            return;
        }
    }
}

void MegaApplication::initialize()
{
    if (megaApi)
    {
        return;
    }

    paused = false;
    indexing = false;
    setQuitOnLastWindowClosed(false);

#ifdef Q_OS_LINUX
    isLinux = true;
#else
    isLinux = false;
#endif

    //Register own url schemes
    QDesktopServices::setUrlHandler(QString::fromUtf8("mega"), this, "handleMEGAurl");
    QDesktopServices::setUrlHandler(QString::fromUtf8("local"), this, "handleLocalPath");

    //Register metatypes to use them in signals/slots
    qRegisterMetaType<QQueue<QString> >("QQueueQString");
    qRegisterMetaTypeStreamOperators<QQueue<QString> >("QQueueQString");

    preferences = Preferences::instance();
    connect(preferences, SIGNAL(stateChanged()), this, SLOT(changeState()));
    connect(preferences, SIGNAL(updated(int)), this, SLOT(showUpdatedMessage(int)));
    preferences->initialize(dataPath);

    model = Model::instance();

    connect(model, SIGNAL(syncStateChanged(std::shared_ptr<SyncSetting>)),
            this, SLOT(onSyncStateChanged(std::shared_ptr<SyncSetting>)));
    connect(model, SIGNAL(syncRemoved(std::shared_ptr<SyncSetting>)),
            this, SLOT(onSyncDeleted(std::shared_ptr<SyncSetting>)));

    if (preferences->error())
    {
        QMegaMessageBox::critical(nullptr, QString::fromAscii("MEGAsync"), tr("Your config is corrupt, please start over"));
    }

    preferences->setLastStatsRequest(0);
    lastExit = preferences->getLastExit();

    installTranslator(&translator);
    QString language = preferences->language();
    changeLanguage(language);

#ifdef __APPLE__
    notificator = new Notificator(applicationName(), NULL, this);
#else
    notificator = new Notificator(applicationName(), trayIcon, this);
#endif

    Qt::KeyboardModifiers modifiers = queryKeyboardModifiers();
    if (modifiers.testFlag(Qt::ControlModifier)
            && modifiers.testFlag(Qt::ShiftModifier))
    {
        toggleLogging();
    }

    QString basePath = QDir::toNativeSeparators(dataPath + QString::fromAscii("/"));
#ifndef __APPLE__
    megaApi = new MegaApi(Preferences::CLIENT_KEY, basePath.toUtf8().constData(), Preferences::USER_AGENT);
#else
    megaApi = new MegaApi(Preferences::CLIENT_KEY, basePath.toUtf8().constData(), Preferences::USER_AGENT, MacXPlatform::fd);
#endif

    megaApiFolders = new MegaApi(Preferences::CLIENT_KEY, basePath.toUtf8().constData(), Preferences::USER_AGENT);

    //Set payload max size to be logged: a 10th of system's memory
    long long availMemory = Utilities::getSystemsAvailableMemory();
    auto newPayLoadLogSize = availMemory / 10 ;
    if (newPayLoadLogSize < 10240)
    {
        newPayLoadLogSize = 10240;
    }
    megaApi->log(MegaApi::LOG_LEVEL_INFO, QString::fromUtf8("Establishing max payload log size: %1").arg(newPayLoadLogSize).toUtf8().constData());
    megaApi->setMaxPayloadLogSize(newPayLoadLogSize);
    megaApiFolders->setMaxPayloadLogSize(newPayLoadLogSize);

    megaApi->setKeepSyncsAfterLogout(true);


    controller = Controller::instance();
    controller->setApi(this->megaApi);


    QString stagingPath = QDir(dataPath).filePath(QString::fromAscii("megasync.staging"));
    QFile fstagingPath(stagingPath);
    if (fstagingPath.exists())
    {
        QSettings settings(stagingPath, QSettings::IniFormat);
        QString apiURL = settings.value(QString::fromUtf8("apiurl"), QString::fromUtf8("https://staging.api.mega.co.nz/")).toString();
        megaApi->changeApiUrl(apiURL.toUtf8());
        megaApiFolders->changeApiUrl(apiURL.toUtf8());
        QMegaMessageBox::warning(nullptr, QString::fromUtf8("MEGAsync"), QString::fromUtf8("API URL changed to ")+ apiURL);

        QString baseURL = settings.value(QString::fromUtf8("baseurl"), Preferences::BASE_URL).toString();
        Preferences::setBaseUrl(baseURL);
        if (baseURL.compare(QString::fromUtf8("https://mega.nz")))
        {
            QMegaMessageBox::warning(nullptr, QString::fromUtf8("MEGAsync"), QString::fromUtf8("base URL changed to ") + Preferences::BASE_URL);
        }

        Preferences::overridePreferences(settings);
        Preferences::SDK_ID.append(QString::fromUtf8(" - STAGING"));
    }
    trayIcon->show();

    megaApi->log(MegaApi::LOG_LEVEL_INFO, QString::fromUtf8("MEGAsync is starting. Version string: %1   Version code: %2.%3   User-Agent: %4").arg(Preferences::VERSION_STRING)
             .arg(Preferences::VERSION_CODE).arg(Preferences::BUILD_ID).arg(QString::fromUtf8(megaApi->getUserAgent())).toUtf8().constData());

    megaApi->setLanguage(currentLanguageCode.toUtf8().constData());
    megaApiFolders->setLanguage(currentLanguageCode.toUtf8().constData());
    megaApi->setDownloadMethod(preferences->transferDownloadMethod());
    megaApi->setUploadMethod(preferences->transferUploadMethod());
    setMaxConnections(MegaTransfer::TYPE_UPLOAD,   preferences->parallelUploadConnections());
    setMaxConnections(MegaTransfer::TYPE_DOWNLOAD, preferences->parallelDownloadConnections());
    setUseHttpsOnly(preferences->usingHttpsOnly());

    megaApi->setDefaultFilePermissions(preferences->filePermissionsValue());
    megaApi->setDefaultFolderPermissions(preferences->folderPermissionsValue());
    megaApi->retrySSLerrors(true);
    megaApi->setPublicKeyPinning(!preferences->SSLcertificateException());

    delegateListener = new MEGASyncDelegateListener(megaApi, this, this);
    megaApi->addListener(delegateListener);
    uploader = new MegaUploader(megaApi);
    downloader = new MegaDownloader(megaApi);
    connect(downloader, SIGNAL(finishedTransfers(unsigned long long)), this, SLOT(showNotificationFinishedTransfers(unsigned long long)), Qt::QueuedConnection);


    connectivityTimer = new QTimer(this);
    connectivityTimer->setSingleShot(true);
    connectivityTimer->setInterval(Preferences::MAX_LOGIN_TIME_MS);
    connect(connectivityTimer, SIGNAL(timeout()), this, SLOT(runConnectivityCheck()));

    proExpirityTimer.setSingleShot(true);
    connect(&proExpirityTimer, SIGNAL(timeout()), this, SLOT(proExpirityTimedOut()));

#ifdef _WIN32
    if (isPublic && prevVersion <= 3104 && preferences->canUpdate(appPath))
    {
        megaApi->log(MegaApi::LOG_LEVEL_INFO, QString::fromUtf8("Fixing permissions for other users in the computer").toUtf8().constData());
        QDirIterator it (appDirPath, QDir::AllEntries | QDir::Hidden | QDir::System | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
        while (it.hasNext())
        {
            Platform::makePubliclyReadable((LPTSTR)QDir::toNativeSeparators(it.next()).utf16());
        }
    }
#endif

#ifdef __APPLE__
    MEGA_SET_PERMISSIONS;
#endif

    if (!preferences->isOneTimeActionDone(Preferences::ONE_TIME_ACTION_REGISTER_UPDATE_TASK))
    {
        bool success = Platform::registerUpdateJob();
        if (success)
        {
            preferences->setOneTimeActionDone(Preferences::ONE_TIME_ACTION_REGISTER_UPDATE_TASK, true);
        }
    }

    if (preferences->isCrashed())
    {
        preferences->setCrashed(false);
        QDirIterator di(dataPath, QDir::Files | QDir::NoDotAndDotDot);
        while (di.hasNext())
        {
            di.next();
            const QFileInfo& fi = di.fileInfo();
            if (!fi.fileName().contains(QString::fromUtf8("transfers_"))
                && !fi.fileName().contains(QString::fromUtf8("syncconfigsv2_"))
                && (fi.fileName().endsWith(QString::fromAscii(".db"))
                    || fi.fileName().endsWith(QString::fromAscii(".db-wal"))
                    || fi.fileName().endsWith(QString::fromAscii(".db-shm"))))
            {
                QFile::remove(di.filePath());
            }
        }

        QStringList reports = CrashHandler::instance()->getPendingCrashReports();
        if (reports.size())
        {
            CrashReportDialog crashDialog(reports.join(QString::fromAscii("------------------------------\n")));
            if (crashDialog.exec() == QDialog::Accepted)
            {
                applyProxySettings();
                CrashHandler::instance()->sendPendingCrashReports(crashDialog.getUserMessage());
                if (crashDialog.sendLogs())
                {
                    auto timestampString = reports[0].mid(reports[0].indexOf(QString::fromUtf8("Timestamp: "))+11,20);
                    timestampString = timestampString.left(timestampString.indexOf(QString::fromUtf8("\n")));
                    QDateTime crashTimestamp = QDateTime::fromMSecsSinceEpoch(timestampString.toLongLong());

                    if (crashTimestamp != QDateTime::fromMSecsSinceEpoch(0))
                    {
                        crashTimestamp = crashTimestamp.addSecs(-300); //to gather some logging before the crash
                    }

                    connect(logger.get(), &MegaSyncLogger::logReadyForReporting, context.get(), [this, crashTimestamp]()
                    {
                        crashReportFilePath = Utilities::joinLogZipFiles(megaApi, &crashTimestamp, CrashHandler::instance()->getLastCrashHash());
                        if (!crashReportFilePath.isNull()
                                && megaApi && megaApi->isLoggedIn())
                        {
                            megaApi->startUploadForSupport(QDir::toNativeSeparators(crashReportFilePath).toUtf8().constData(), false);
                            crashReportFilePath.clear();
                        }
                        context.get()->deleteLater();
                    });

                    logger->prepareForReporting();
                }

#ifndef __APPLE__
                QMegaMessageBox::information(nullptr, QString::fromAscii("MEGAsync"), tr("Thank you for your collaboration!"));
#endif
            }
        }
    }

    transferQuota = ::mega::make_unique<TransferQuota>(megaApi, preferences, notificator);
    connect(transferQuota.get(), &TransferQuota::waitTimeIsOver, this, &MegaApplication::updateStatesAfterTransferOverQuotaTimeHasExpired);

    periodicTasksTimer = new QTimer(this);
    periodicTasksTimer->start(Preferences::STATE_REFRESH_INTERVAL_MS);
    connect(periodicTasksTimer, SIGNAL(timeout()), this, SLOT(periodicTasks()));

    infoDialogTimer = new QTimer(this);
    infoDialogTimer->setSingleShot(true);
    connect(infoDialogTimer, SIGNAL(timeout()), this, SLOT(showInfoDialog()));

    firstTransferTimer = new QTimer(this);
    firstTransferTimer->setSingleShot(true);
    firstTransferTimer->setInterval(200);
    connect(firstTransferTimer, SIGNAL(timeout()), this, SLOT(checkFirstTransfer()));

    connect(this, SIGNAL(aboutToQuit()), this, SLOT(cleanAll()));

    if (preferences->logged() && preferences->getGlobalPaused())
    {
        pauseTransfers(true);
    }

    QDir dataDir(dataPath);
    if (dataDir.exists())
    {
        QString appShowInterfacePath = dataDir.filePath(QString::fromAscii("megasync.show"));
        QFileSystemWatcher *watcher = new QFileSystemWatcher(this);
        QFile fappShowInterfacePath(appShowInterfacePath);
        if (fappShowInterfacePath.open(QIODevice::WriteOnly | QIODevice::Truncate))
        {
            // any text added to this file will cause the infoDialog to show
            fappShowInterfacePath.close();
        }
        watcher->addPath(appShowInterfacePath);
        connect(watcher, SIGNAL(fileChanged(QString)), this, SLOT(showInterface(QString)));
    }
}

QString MegaApplication::applicationFilePath()
{
    return appPath;
}

QString MegaApplication::applicationDirPath()
{
    return appDirPath;
}

QString MegaApplication::applicationDataPath()
{
    if (dataPath.isEmpty())
    {
        loadDataPath();
    }
    return dataPath;
}

QString MegaApplication::getCurrentLanguageCode()
{
    return currentLanguageCode;
}

void MegaApplication::changeLanguage(QString languageCode)
{
    if (appfinished)
    {
        return;
    }

    if (!translator.load(Preferences::TRANSLATION_FOLDER
                            + Preferences::TRANSLATION_PREFIX
                            + languageCode))
    {
        translator.load(Preferences::TRANSLATION_FOLDER
                                   + Preferences::TRANSLATION_PREFIX
                                   + QString::fromUtf8("en"));
        currentLanguageCode = QString::fromUtf8("en");
    }
    else
    {
        currentLanguageCode = languageCode;
    }

    createTrayIcon();
}

#ifdef Q_OS_LINUX
void MegaApplication::setTrayIconFromTheme(QString icon)
{
    QString name = QString(icon).replace(QString::fromAscii("://images/"), QString::fromAscii("mega")).replace(QString::fromAscii(".svg"),QString::fromAscii(""));
    trayIcon->setIcon(QIcon::fromTheme(name, QIcon(icon)));
}
#endif

void MegaApplication::updateTrayIcon()
{
    if (appfinished || !trayIcon)
    {
        return;
    }

    QString tooltip;
    QString icon;

    const auto isOverQuotaOrPaywall{appliedStorageState == MegaApi::STORAGE_STATE_RED ||
                transferQuota->isOverQuota() ||
                appliedStorageState == MegaApi::STORAGE_STATE_PAYWALL};
    if (isOverQuotaOrPaywall)
    {
        tooltip = QCoreApplication::applicationName()
                + QString::fromAscii(" ")
                + Preferences::VERSION_STRING
                + QString::fromAscii("\n")
                + tr("Over quota");

#ifndef __APPLE__
    #ifdef _WIN32
        icon = QString::fromUtf8("://images/warning_ico.ico");
    #else
        icon = QString::fromUtf8("://images/warning.svg");
    #endif
#else
        icon = QString::fromUtf8("://images/icon_overquota_mac.png");

        if (scanningTimer->isActive())
        {
            scanningTimer->stop();
        }
#endif
    }
    else if (blockState)
    {
        tooltip = QCoreApplication::applicationName()
                + QString::fromAscii(" ")
                + Preferences::VERSION_STRING
                + QString::fromAscii("\n")
                + tr("Locked account");

#ifndef __APPLE__
    #ifdef _WIN32
        icon = QString::fromUtf8("://images/alert_ico.ico");
    #else
        icon = QString::fromUtf8("://images/alert.svg");
    #endif
#else
        icon = QString::fromUtf8("://images/icon_alert_mac.png");

        if (scanningTimer->isActive())
        {
            scanningTimer->stop();
        }
#endif

    }
    else if (!megaApi->isLoggedIn())
    {
        if (!infoDialog)
        {
            tooltip = QCoreApplication::applicationName()
                    + QString::fromAscii(" ")
                    + Preferences::VERSION_STRING
                    + QString::fromAscii("\n")
                    + tr("Logging in");

    #ifndef __APPLE__
        #ifdef _WIN32
            icon = QString::fromUtf8("://images/tray_sync.ico");
        #else
            icon = QString::fromUtf8("://images/synching.svg");
        #endif
    #else
            icon = QString::fromUtf8("://images/icon_syncing_mac.png");

            if (!scanningTimer->isActive())
            {
                scanningAnimationIndex = 1;
                scanningTimer->start();
            }
    #endif
        }
        else
        {
            tooltip = QCoreApplication::applicationName()
                    + QString::fromAscii(" ")
                    + Preferences::VERSION_STRING
                    + QString::fromAscii("\n")
                    + tr("You are not logged in");

    #ifndef __APPLE__
        #ifdef _WIN32
            icon = QString::fromUtf8("://images/app_ico.ico");
        #else
            icon = QString::fromUtf8("://images/uptodate.svg");
        #endif
    #else
            icon = QString::fromUtf8("://images/icon_synced_mac.png");

            if (scanningTimer->isActive())
            {
                scanningTimer->stop();
            }
    #endif
        }
    }
    else if (!megaApi->isFilesystemAvailable())
    {
        tooltip = QCoreApplication::applicationName()
                + QString::fromAscii(" ")
                + Preferences::VERSION_STRING
                + QString::fromAscii("\n")
                + tr("Fetching file list...");

#ifndef __APPLE__
    #ifdef _WIN32
        icon = QString::fromUtf8("://images/tray_sync.ico");
    #else
        icon = QString::fromUtf8("://images/synching.svg");
    #endif
#else
        icon = QString::fromUtf8("://images/icon_syncing_mac.png");

        if (!scanningTimer->isActive())
        {
            scanningAnimationIndex = 1;
            scanningTimer->start();
        }
#endif
    }
    else if (paused)
    {
        tooltip = QCoreApplication::applicationName()
                + QString::fromAscii(" ")
                + Preferences::VERSION_STRING
                + QString::fromAscii("\n")
                + tr("Paused");

#ifndef __APPLE__
    #ifdef _WIN32
        icon = QString::fromUtf8("://images/tray_pause.ico");
    #else
        icon = QString::fromUtf8("://images/paused.svg");
    #endif
#else
        icon = QString::fromUtf8("://images/icon_paused_mac.png");

        if (scanningTimer->isActive())
        {
            scanningTimer->stop();
        }
#endif
    }
    else if (indexing || waiting || syncing || transferring)
    {
        if (indexing)
        {
            tooltip = QCoreApplication::applicationName()
                    + QString::fromAscii(" ")
                    + Preferences::VERSION_STRING
                    + QString::fromAscii("\n")
                    + tr("Scanning");
        }
        else if (syncing)
        {
            tooltip = QCoreApplication::applicationName()
                    + QString::fromAscii(" ")
                    + Preferences::VERSION_STRING
                    + QString::fromAscii("\n")
                    + tr("Syncing");
        }
        else if (waiting)
        {
            tooltip = QCoreApplication::applicationName()
                    + QString::fromAscii(" ")
                    + Preferences::VERSION_STRING
                    + QString::fromAscii("\n")
                    + tr("Waiting");
        }
        else
        {
            tooltip = QCoreApplication::applicationName()
                    + QString::fromAscii(" ")
                    + Preferences::VERSION_STRING
                    + QString::fromAscii("\n")
                    + tr("Transferring");
        }

#ifndef __APPLE__
    #ifdef _WIN32
        icon = QString::fromUtf8("://images/tray_sync.ico");
    #else
        icon = QString::fromUtf8("://images/synching.svg");
    #endif
#else
        icon = QString::fromUtf8("://images/icon_syncing_mac.png");

        if (!scanningTimer->isActive())
        {
            scanningAnimationIndex = 1;
            scanningTimer->start();
        }
#endif
    }
    else
    {
        tooltip = QCoreApplication::applicationName()
                + QString::fromAscii(" ")
                + Preferences::VERSION_STRING
                + QString::fromAscii("\n")
                + tr("Up to date");

#ifndef __APPLE__
    #ifdef _WIN32
        icon = QString::fromUtf8("://images/app_ico.ico");
    #else
        icon = QString::fromUtf8("://images/uptodate.svg");
    #endif
#else
        icon = QString::fromUtf8("://images/icon_synced_mac.png");

        if (scanningTimer->isActive())
        {
            scanningTimer->stop();
        }
#endif
        if (reboot)
        {
            rebootApplication();
        }
    }

    if (!networkConnectivity)
    {
        //Override the current state
        tooltip = QCoreApplication::applicationName()
                + QString::fromAscii(" ")
                + Preferences::VERSION_STRING
                + QString::fromAscii("\n")
                + tr("No Internet connection");

#ifndef __APPLE__
    #ifdef _WIN32
        icon = QString::fromUtf8("://images/login_ico.ico");
    #else
        icon = QString::fromUtf8("://images/logging.svg");
    #endif
#else
        icon = QString::fromUtf8("://images/icon_logging_mac.png");
#endif
    }

    if (updateAvailable)
    {
        tooltip += QString::fromAscii("\n")
                + tr("Update available!");
    }

    if (!icon.isEmpty())
    {
#ifndef __APPLE__
    #ifdef _WIN32
        trayIcon->setIcon(QIcon(icon));
    #else
        setTrayIconFromTheme(icon);
    #endif
#else
    QIcon ic = QIcon(icon);
    ic.setIsMask(true);
    trayIcon->setIcon(ic);
#endif
    }

    if (!tooltip.isEmpty())
    {
        trayIcon->setToolTip(tooltip);
    }
}

void MegaApplication::start()
{
#ifdef Q_OS_LINUX
    QSvgRenderer qsr; //to have svg library linked
#endif

    if (appfinished)
    {
        return;
    }

    blockState = MegaApi::ACCOUNT_NOT_BLOCKED;
    blockStateSet = false;

    indexing = false;
    paused = false;
    nodescurrent = false;
    appliedStorageState = MegaApi::STORAGE_STATE_UNKNOWN;
    getUserDataRequestReady = false;
    mFetchingNodes = false;
    mQueringWhyAmIBlocked = false;
    whyamiblockedPeriodicPetition = false;
    storageState = MegaApi::STORAGE_STATE_UNKNOWN;
    appliedStorageState = MegaApi::STORAGE_STATE_UNKNOWN;
    eventsPendingLoggedIn.clear();
    receivedStorageSum = 0;
    finishedBlockedTransfers.clear();

    for (unsigned i = 3; i--; )
    {
        inflightUserStats[i] = false;
        userStatsLastRequest[i] = 0;
        queuedUserStats[i] = false;
    }
    queuedStorageUserStatsReason = 0;

    if (infoDialog)
    {
        infoDialog->reset();
    }
    transferQuota->reset();
    transferOverQuotaWaitTimeExpiredReceived = false;
    updateTrayIconMenu();

    if(notificationsModel) notificationsModel->deleteLater();
    notificationsModel = NULL;
    if (notificationsProxyModel) notificationsProxyModel->deleteLater();
    notificationsProxyModel = NULL;
    if (notificationsDelegate) notificationsDelegate->deleteLater();
    notificationsDelegate = NULL;

#ifndef __APPLE__
    #ifdef _WIN32
        trayIcon->setIcon(QIcon(QString::fromAscii("://images/tray_sync.ico")));
    #else
        setTrayIconFromTheme(QString::fromAscii("://images/synching.svg"));
    #endif
#else
    QIcon ic = QIcon(QString::fromAscii("://images/icon_syncing_mac.png"));
    ic.setIsMask(true);
    trayIcon->setIcon(ic);

    if (!scanningTimer->isActive())
    {
        scanningAnimationIndex = 1;
        scanningTimer->start();
    }
#endif
    trayIcon->setToolTip(QCoreApplication::applicationName() + QString::fromAscii(" ") + Preferences::VERSION_STRING + QString::fromAscii("\n") + tr("Logging in"));
    trayIcon->show();

    if (!preferences->lastExecutionTime())
    {
        Platform::enableTrayIcon(QFileInfo(MegaApplication::applicationFilePath()).fileName());
    }

    if (updated)
    {
        showInfoMessage(tr("MEGAsync has been updated"));
        preferences->setFirstSyncDone();
        preferences->setFirstFileSynced();
        preferences->setFirstWebDownloadDone();

        if (!preferences->installationTime())
        {
            preferences->setInstallationTime(-1);
        }
#ifdef Q_OS_MACX
        Platform::reloadFinderExtension();
#endif
    }

    applyProxySettings();
    Platform::startShellDispatcher(this);
#ifdef Q_OS_MACX
    if (QSysInfo::MacintoshVersion > QSysInfo::MV_10_9) //FinderSync API support from 10.10+
    {
        if (!preferences->isOneTimeActionDone(Preferences::ONE_TIME_ACTION_ACTIVE_FINDER_EXT))
        {
            MegaApi::log(MegaApi::LOG_LEVEL_INFO, "MEGA Finder Sync added to system database and enabled");
            Platform::addFinderExtensionToSystem();
            QTimer::singleShot(5000, this, SLOT(enableFinderExt()));
        }
    }
#endif

    //Start the initial setup wizard if needed
    if (!preferences->logged() && preferences->getSession().isEmpty())
    {
        if (!preferences->installationTime())
        {
            preferences->setInstallationTime(QDateTime::currentDateTime().toMSecsSinceEpoch() / 1000);
        }

        startUpdateTask();
        QString language = preferences->language();
        changeLanguage(language);

        initLocalServer();
        if (updated)
        {
            megaApi->sendEvent(99510, "MEGAsync update");
            checkupdate = true;
        }
        updated = false;

        checkOperatingSystem();

        if (!infoDialog)
        {
            createInfoDialog();

            if (!QSystemTrayIcon::isSystemTrayAvailable())
            {
                if (!preferences->isOneTimeActionDone(Preferences::ONE_TIME_ACTION_NO_SYSTRAY_AVAILABLE))
                {
                    QMegaMessageBox::warning(nullptr, tr("MEGAsync"),
                                         tr("Could not find a system tray to place MEGAsync tray icon. "
                                            "MEGAsync is intended to be used with a system tray icon but it can work fine without it. "
                                            "If you want to open the interface, just try to open MEGAsync again."));
                    preferences->setOneTimeActionDone(Preferences::ONE_TIME_ACTION_NO_SYSTRAY_AVAILABLE, true);
                }
            }
            createTrayIcon();
        }


        if (!preferences->isFirstStartDone())
        {
            megaApi->sendEvent(99500, "MEGAsync first start");
            openInfoWizard();
        }
        else if (!QSystemTrayIcon::isSystemTrayAvailable() && !getenv("START_MEGASYNC_IN_BACKGROUND"))
        {
            showInfoDialog();
        }

        onGlobalSyncStateChanged(megaApi);
        return;
    }
    else //Otherwise, login in the account
    {
        if (preferences->logged()) //we have per account settings to restore
        {
            QStringList exclusions = preferences->getExcludedSyncNames();
            vector<string> vExclusions;
            for (int i = 0; i < exclusions.size(); i++)
            {
                vExclusions.push_back(exclusions[i].toUtf8().constData());
            }
            megaApi->setExcludedNames(&vExclusions);

            QStringList exclusionPaths = preferences->getExcludedSyncPaths();
            vector<string> vExclusionPaths;
            for (int i = 0; i < exclusionPaths.size(); i++)
            {
                vExclusionPaths.push_back(exclusionPaths[i].toUtf8().constData());
            }
            megaApi->setExcludedPaths(&vExclusionPaths);

            if (preferences->lowerSizeLimit())
            {
                megaApi->setExclusionLowerSizeLimit(preferences->lowerSizeLimitValue() * pow((float)1024, preferences->lowerSizeLimitUnit()));
            }
            else
            {
                megaApi->setExclusionLowerSizeLimit(0);
            }

            if (preferences->upperSizeLimit())
            {
                megaApi->setExclusionUpperSizeLimit(preferences->upperSizeLimitValue() * pow((float)1024, preferences->upperSizeLimitUnit()));
            }
            else
            {
                megaApi->setExclusionUpperSizeLimit(0);
            }
        }

        QString theSession;
        theSession = preferences->getSession();

        if (theSession.size())
        {
            megaApi->fastLogin(theSession.toUtf8().constData());
        }
        else //In case preferences are corrupt with empty session, just unlink and remove associated data.
        {
            MegaApi::log(MegaApi::LOG_LEVEL_ERROR, "MEGAsync preferences logged but empty session. Unlink account and fresh start.");
            unlink();
        }

        if (updated)
        {
            megaApi->sendEvent(99510, "MEGAsync update");
            checkupdate = true;
        }
    }
}

void MegaApplication::requestUserData()
{
    if (!megaApi)
    {
        return;
    }

    megaApi->getPricing();
    megaApi->getUserAttribute(MegaApi::USER_ATTR_FIRSTNAME);
    megaApi->getUserAttribute(MegaApi::USER_ATTR_LASTNAME);
    megaApi->getFileVersionsOption();
    megaApi->getPSA();

    const char *email = megaApi->getMyEmail();
    if (email)
    {
        megaApi->getUserAvatar(Utilities::getAvatarPath(QString::fromUtf8(email)).toUtf8().constData());
        delete [] email;
    }
}

void MegaApplication::loggedIn(bool fromWizard)
{
    if (appfinished)
    {
        return;
    }

    if (infoWizard)
    {
        infoWizard->deleteLater();
        infoWizard = NULL;
    }

    //Send pending crash report log if neccessary
    if (!crashReportFilePath.isNull() && megaApi)
    {
        QFileInfo crashReportFile{crashReportFilePath};
        megaApi->startUploadForSupport(QDir::toNativeSeparators(crashReportFilePath).toUtf8().constData(),
                                       false);
        crashReportFilePath.clear();
    }

    registerUserActivity();
    pauseTransfers(paused);

    int cachedStorageState = preferences->getStorageState();

    // ask for storage on first login (fromWizard), or when cached value is invalid
    updateUserStats(fromWizard || cachedStorageState == MegaApi::STORAGE_STATE_UNKNOWN, true, true, true, fromWizard ? USERSTATS_LOGGEDIN : USERSTATS_STORAGECACHEUNKNOWN);

    requestUserData();

    if (settingsDialog)
    {
        settingsDialog->setProxyOnly(false);
    }

    // Apply the "Start on startup" configuration, make sure configuration has the actual value
    // get the requested value
    bool startOnStartup = preferences->startOnStartup();
    // try to enable / disable startup (e.g. copy or delete desktop file)
    if (!Platform::startOnStartup(startOnStartup)) {
        // in case of failure - make sure configuration keeps the right value
        //LOG_debug << "Failed to " << (startOnStartup ? "enable" : "disable") << " MEGASync on startup.";
        preferences->setStartOnStartup(!startOnStartup);
    }

if (!preferences->lastExecutionTime())
{
    #ifdef __APPLE__
        showInfoMessage(tr("MEGAsync is now running. Click the menu bar icon to open the status window."));
    #else
        showInfoMessage(tr("MEGAsync is now running. Click the system tray icon to open the status window."));
    #endif
}

    preferences->setLastExecutionTime(QDateTime::currentDateTime().toMSecsSinceEpoch());
    QDateTime now = QDateTime::currentDateTime();
    preferences->setDsDiffTimeWithSDK(now.toMSecsSinceEpoch() / 100 - megaApi->getSDKtime());

    startUpdateTask();
    QString language = preferences->language();
    changeLanguage(language);
    updated = false;

    checkOperatingSystem();

    if (!infoDialog)
    {
        createInfoDialog();

        if (!QSystemTrayIcon::isSystemTrayAvailable())
        {
            if (!preferences->isOneTimeActionDone(Preferences::ONE_TIME_ACTION_NO_SYSTRAY_AVAILABLE))
            {
                QMegaMessageBox::warning(nullptr, tr("MEGAsync"),
                                     tr("Could not find a system tray to place MEGAsync tray icon. "
                                        "MEGAsync is intended to be used with a system tray icon but it can work fine without it. "
                                        "If you want to open the interface, just try to open MEGAsync again."));
                preferences->setOneTimeActionDone(Preferences::ONE_TIME_ACTION_NO_SYSTRAY_AVAILABLE, true);
            }
            if (!getenv("START_MEGASYNC_IN_BACKGROUND"))
            {
                showInfoDialog();
            }
        }
    }
    infoDialog->setUsage();
    infoDialog->setAccountType(preferences->accountType());

    createAppMenus();

    //Set the upload limit
    if (preferences->uploadLimitKB() > 0)
    {
        setUploadLimit(0);
    }
    else
    {
        setUploadLimit(preferences->uploadLimitKB());
    }
    setMaxUploadSpeed(preferences->uploadLimitKB());
    setMaxDownloadSpeed(preferences->downloadLimitKB());
    setMaxConnections(MegaTransfer::TYPE_UPLOAD,   preferences->parallelUploadConnections());
    setMaxConnections(MegaTransfer::TYPE_DOWNLOAD, preferences->parallelDownloadConnections());
    setUseHttpsOnly(preferences->usingHttpsOnly());

    megaApi->setDefaultFilePermissions(preferences->filePermissionsValue());
    megaApi->setDefaultFolderPermissions(preferences->folderPermissionsValue());

    // Process any pending download/upload queued during GuestMode
    processDownloads();
    processUploads();
    for (QMap<QString, QString>::iterator it = pendingLinks.begin(); it != pendingLinks.end(); it++)
    {
        QString link = it.key();
        megaApi->getPublicNode(link.toUtf8().constData());
    }

    // Apply all pending events that arrived before its time
    for (auto & event: eventsPendingLoggedIn)
    {
        onEvent(megaApi, event.get());
    }
    eventsPendingLoggedIn.clear();


    if (storageState == MegaApi::STORAGE_STATE_RED && receivedStorageSum < preferences->totalStorage())
    {
        preferences->setUsedStorage(preferences->totalStorage());
    }
    else
    {
        preferences->setUsedStorage(receivedStorageSum);
    }
    preferences->sync();
    refreshStorageUIs();

    onGlobalSyncStateChanged(megaApi);

    if (cachedStorageState != MegaApi::STORAGE_STATE_UNKNOWN)
    {
        applyStorageState(cachedStorageState, true);
    }
}

void MegaApplication::startSyncs(QList<PreConfiguredSync> syncs)
{
    if (appfinished)
    {
        return;
    }

    // add syncs from setupWizard
    for (auto & ps : syncs)
    {
        MegaApi::log(MegaApi::LOG_LEVEL_INFO, QString::fromAscii("Adding sync %1 from SetupWizard: ").arg(ps.localFolder()).toUtf8().constData());
        QString localFolderPath = ps.localFolder();

        ActionProgress *addSyncStep = new ActionProgress(true, QString::fromUtf8("Adding sync: %1")
                                                         .arg(localFolderPath));

        //Connect failing signals
        connect(addSyncStep, &ActionProgress::failed, this, [this, localFolderPath](int errorCode)
        {
            static_cast<MegaApplication *>(qApp)->showAddSyncError(errorCode, localFolderPath);
        }, Qt::QueuedConnection);
        connect(addSyncStep, &ActionProgress::failedRequest, this, [this, localFolderPath](MegaRequest *request, MegaError *error)
        {
            if (error->getErrorCode())
            {
                auto reqCopy = request->copy();
                auto errCopy = error->copy();

                QObject temporary;
                QObject::connect(&temporary, &QObject::destroyed, this, [reqCopy, errCopy, localFolderPath](){

                    // we might want to handle this separately (i.e: indicate errors in SyncSettings engine)
                    static_cast<MegaApplication *>(qApp)->showAddSyncError(reqCopy, errCopy, localFolderPath);

                    delete reqCopy;
                    delete errCopy;
                    //(syncSettings might have some old values), that's why we don't use syncSetting->getError.
                }, Qt::QueuedConnection);
            }
        }, Qt::DirectConnection); //Note, we need direct connection to use request & error


        controller->addSync(ps.localFolder(), ps.megaFolderHandle(), ps.syncName(), addSyncStep);
    }
}

void MegaApplication::applyStorageState(int state, bool doNotAskForUserStats)
{
    if (state == MegaApi::STORAGE_STATE_CHANGE)
    {
        // this one is requested with force=false so it can't possibly occur to often.
        // It will in turn result in another call of this function with the actual new state (if it changed), which is taken care of below with force=true (so that one does not have to wait further)
        // Also request pro state (low cost) in case the storage status is due to expiration of paid period etc.
        updateUserStats(true, false, true, true, USERSTATS_STORAGESTATECHANGE);
        return;
    }

    storageState = state;
    int previousCachedStoragestate = preferences->getStorageState();
    preferences->setStorageState(storageState);
    if (preferences->logged())
    {
        if (storageState != appliedStorageState)
        {
            if (!doNotAskForUserStats && previousCachedStoragestate!= MegaApi::STORAGE_STATE_UNKNOWN)
            {
                updateUserStats(true, false, true, true, USERSTATS_TRAFFICLIGHT);
            }
            if (storageState == MegaApi::STORAGE_STATE_RED)
            {
                if (appliedStorageState != MegaApi::STORAGE_STATE_RED)
                {
                    if (infoDialogMenu && infoDialogMenu->isVisible())
                    {
                        infoDialogMenu->close();
                    }
                    if (infoDialog && infoDialog->isVisible())
                    {
                        infoDialog->hide();
                    }
                }

                if (settingsDialog)
                {
                    delete settingsDialog;
                    settingsDialog = NULL;
                }
            }
            else if (storageState == MegaApi::STORAGE_STATE_PAYWALL)
            {
                if (megaApi)
                {
                    getUserDataRequestReady = false;
                    megaApi->getUserData();
                }
            }
            else
            {
                if (appliedStorageState == MegaApi::STORAGE_STATE_RED
                        || appliedStorageState == MegaApi::STORAGE_STATE_PAYWALL)
                {
                    if (infoDialogMenu && infoDialogMenu->isVisible())
                    {
                        infoDialogMenu->close();
                    }

                    MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, QString::fromUtf8("no longer on storage OQ").toUtf8().constData());
                }
            }

            appliedStorageState = storageState;
            emit storageStateChanged(appliedStorageState);
            checkOverStorageStates();
        }
    }
}

//This function is called to upload all files in the uploadQueue field
//to the Mega node that is passed as parameter
void MegaApplication::processUploadQueue(MegaHandle nodeHandle)
{
    if (appfinished)
    {
        return;
    }

    MegaNode *node = megaApi->getNodeByHandle(nodeHandle);

    //If the destination node doesn't exist in the current filesystem, clear the queue and show an error message
    if (!node || node->isFile())
    {
        uploadQueue.clear();
        showErrorMessage(tr("Error: Invalid destination folder. The upload has been cancelled"));
        delete node;
        return;
    }

    unsigned long long transferId = preferences->transferIdentifier();
    TransferMetaData* data = new TransferMetaData(MegaTransfer::TYPE_UPLOAD, uploadQueue.size(), uploadQueue.size());
    transferAppData.insert(transferId, data);
    preferences->setOverStorageDismissExecution(0);

    //Process the upload queue using the MegaUploader object
    while (!uploadQueue.isEmpty())
    {
        QString filePath = uploadQueue.dequeue();

        // Load parent folder to provide "Show in Folder" option
        if (data->localPath.isEmpty())
        {
            QDir uploadPath(filePath);
            if (data->totalTransfers > 1)
            {
                uploadPath.cdUp();
            }
            data->localPath = uploadPath.path();
        }

        if (QFileInfo (filePath).isDir())
        {
            data->totalFolders++;
        }
        else
        {
            data->totalFiles++;
        }

        uploader->upload(filePath, node, transferId);
    }
    delete node;
}

void MegaApplication::processDownloadQueue(QString path)
{
    if (appfinished)
    {
        return;
    }

    QDir dir(path);
    if (!dir.exists() && !dir.mkpath(QString::fromAscii(".")))
    {
        QQueue<MegaNode *>::iterator it;
        for (it = downloadQueue.begin(); it != downloadQueue.end(); ++it)
        {
            HTTPServer::onTransferDataUpdate((*it)->getHandle(), MegaTransfer::STATE_CANCELLED, 0, 0, 0, QString());
        }

        qDeleteAll(downloadQueue);
        downloadQueue.clear();
        showErrorMessage(tr("Error: Invalid destination folder. The download has been cancelled"));
        return;
    }

    unsigned long long transferId = preferences->transferIdentifier();
    TransferMetaData *transferData =  new TransferMetaData(MegaTransfer::TYPE_DOWNLOAD, downloadQueue.size(), downloadQueue.size());
    transferAppData.insert(transferId, transferData);
    if (!downloader->processDownloadQueue(&downloadQueue, path, transferId))
    {
        transferAppData.remove(transferId);
        delete transferData;
    }
}

void MegaApplication::unityFix()
{
    static QMenu *dummyMenu = NULL;
    if (!dummyMenu)
    {
        dummyMenu = new QMenu();
        connect(this, SIGNAL(unityFixSignal()), dummyMenu, SLOT(close()), Qt::QueuedConnection);
    }

    emit unityFixSignal();
    dummyMenu->exec();
}

void MegaApplication::closeDialogs(bool bwoverquota)
{
    delete transferManager;
    transferManager = NULL;

    delete setupWizard;
    setupWizard = NULL;

    delete settingsDialog;
    settingsDialog = NULL;

    delete streamSelector;
    streamSelector = NULL;

    delete uploadFolderSelector;
    uploadFolderSelector = NULL;

    delete downloadFolderSelector;
    downloadFolderSelector = NULL;

    delete multiUploadFileDialog;
    multiUploadFileDialog = NULL;

    delete fileUploadSelector;
    fileUploadSelector = NULL;

    delete folderUploadSelector;
    folderUploadSelector = NULL;

    delete pasteMegaLinksDialog;
    pasteMegaLinksDialog = NULL;

    delete changeLogDialog;
    changeLogDialog = NULL;

    delete importDialog;
    importDialog = NULL;

    delete downloadNodeSelector;
    downloadNodeSelector = NULL;

    delete sslKeyPinningError;
    sslKeyPinningError = NULL;

    if(transferQuota)
    {
        transferQuota->closeDialogs();
    }

    delete storageOverquotaDialog;
    storageOverquotaDialog = NULL;

    verifyEmail.reset(nullptr);
}

void MegaApplication::rebootApplication(bool update)
{
    if (appfinished)
    {
        return;
    }

    reboot = true;
    if (update && (megaApi->getNumPendingDownloads() || megaApi->getNumPendingUploads() || megaApi->isWaiting()))
    {
        if (!updateBlocked)
        {
            updateBlocked = true;
            showInfoMessage(tr("An update will be applied during the next application restart"));
        }
        return;
    }

    trayIcon->hide();
    closeDialogs();

#ifdef __APPLE__
    cleanAll();
    ::exit(0);
#endif

    QApplication::exit();
}

void MegaApplication::exitApplication(bool force)
{
    if (appfinished)
    {
        return;
    }

#ifndef __APPLE__
    if (force || !megaApi->isLoggedIn())
    {
#endif
        reboot = false;
        trayIcon->hide();
        closeDialogs();
        #ifdef __APPLE__
            cleanAll();
            ::exit(0);
        #endif

        QApplication::exit();
        return;
#ifndef __APPLE__
    }
#endif

    if (!exitDialog)
    {
        exitDialog = new QMessageBox(QMessageBox::Question, tr("MEGAsync"),
                                     tr("Are you sure you want to exit?"), QMessageBox::Yes|QMessageBox::No);
        HighDpiResize hDpiResizer(exitDialog);
        int button = exitDialog->exec();
        if (!exitDialog)
        {
            return;
        }

        exitDialog->deleteLater();
        exitDialog = NULL;
        if (button == QMessageBox::Yes)
        {
            reboot = false;
            trayIcon->hide();
            closeDialogs();

            #ifdef __APPLE__
                cleanAll();
                ::exit(0);
            #endif

            QApplication::exit();
        }
    }
    else
    {
        exitDialog->activateWindow();
        exitDialog->raise();
    }
}

void MegaApplication::highLightMenuEntry(QAction *action)
{
    if (!action)
    {
        return;
    }

    MenuItemAction* pAction = (MenuItemAction*)action;
    if (lastHovered)
    {
        lastHovered->setHighlight(false);
    }
    pAction->setHighlight(true);
    lastHovered = pAction;
}

void MegaApplication::pauseTransfers(bool pause)
{
    if (appfinished)
    {
        return;
    }

    megaApi->pauseTransfers(pause);
}

void MegaApplication::checkNetworkInterfaces()
{
    if (appfinished)
    {
        return;
    }

    bool disconnect = false;
    QList<QNetworkInterface> newNetworkInterfaces;
    QList<QNetworkInterface> configs = QNetworkInterface::allInterfaces();

    //Filter interfaces (QT provides interfaces with loopback IP addresses)
    for (int i = 0; i < configs.size(); i++)
    {
        QNetworkInterface networkInterface = configs.at(i);
        QString interfaceName = networkInterface.humanReadableName();
        QNetworkInterface::InterfaceFlags flags = networkInterface.flags();
        if ((flags & (QNetworkInterface::IsUp | QNetworkInterface::IsRunning))
                && !(interfaceName == QString::fromUtf8("Teredo Tunneling Pseudo-Interface")))
        {
            MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, QString::fromUtf8("Active network interface: %1").arg(interfaceName).toUtf8().constData());

            int numActiveIPs = 0;
            QList<QNetworkAddressEntry> addresses = networkInterface.addressEntries();
            for (int i = 0; i < addresses.size(); i++)
            {
                QHostAddress ip = addresses.at(i).ip();
                switch (ip.protocol())
                {
                case QAbstractSocket::IPv4Protocol:
                    if (!ip.toString().startsWith(QString::fromUtf8("127."), Qt::CaseInsensitive)
                            && !ip.toString().startsWith(QString::fromUtf8("169.254."), Qt::CaseInsensitive))
                    {
                        MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, QString::fromUtf8("IPv4: %1").arg(ip.toString()).toUtf8().constData());
                        numActiveIPs++;
                    }
                    else
                    {
                        MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, QString::fromUtf8("Ignored IPv4: %1").arg(ip.toString()).toUtf8().constData());
                    }
                    break;
                case QAbstractSocket::IPv6Protocol:
                    if (!ip.toString().startsWith(QString::fromUtf8("FE80:"), Qt::CaseInsensitive)
                            && !ip.toString().startsWith(QString::fromUtf8("FD00:"), Qt::CaseInsensitive)
                            && !(ip.toString() == QString::fromUtf8("::1")))
                    {
                        MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, QString::fromUtf8("IPv6: %1").arg(ip.toString()).toUtf8().constData());
                        numActiveIPs++;
                    }
                    else
                    {
                        MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, QString::fromUtf8("Ignored IPv6: %1").arg(ip.toString()).toUtf8().constData());
                    }
                    break;
                default:
                    MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, QString::fromUtf8("Ignored IP: %1").arg(ip.toString()).toUtf8().constData());
                    break;
                }
            }

            if (!numActiveIPs)
            {
                continue;
            }

            lastActiveTime = QDateTime::currentMSecsSinceEpoch();
            newNetworkInterfaces.append(networkInterface);

            if (!networkConnectivity)
            {
                disconnect = true;
                networkConnectivity = true;
            }
        }
        else
        {
            MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, QString::fromUtf8("Ignored network interface: %1 Flags: %2")
                         .arg(interfaceName)
                         .arg(QString::number(flags)).toUtf8().constData());
        }
    }

    if (!newNetworkInterfaces.size())
    {
        MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, "No active network interfaces found");
        networkConnectivity = false;
        networkConfigurationManager.updateConfigurations();
    }
    else if (!activeNetworkInterfaces.size())
    {
        activeNetworkInterfaces = newNetworkInterfaces;
    }
    else if (activeNetworkInterfaces.size() != newNetworkInterfaces.size())
    {
        disconnect = true;
        MegaApi::log(MegaApi::LOG_LEVEL_INFO, "Local network interface change detected");
    }
    else
    {
        for (int i = 0; i < newNetworkInterfaces.size(); i++)
        {
            QNetworkInterface networkInterface = newNetworkInterfaces.at(i);

            int j = 0;
            while (j < activeNetworkInterfaces.size())
            {
                if (activeNetworkInterfaces.at(j).name() == networkInterface.name())
                {
                    break;
                }
                j++;
            }

            if (j == activeNetworkInterfaces.size())
            {
                //New interface
                MegaApi::log(MegaApi::LOG_LEVEL_INFO, QString::fromUtf8("New working network interface detected (%1)").arg(networkInterface.humanReadableName()).toUtf8().constData());
                disconnect = true;
            }
            else
            {
                QNetworkInterface oldNetworkInterface = activeNetworkInterfaces.at(j);
                QList<QNetworkAddressEntry> addresses = networkInterface.addressEntries();
                if (addresses.size() != oldNetworkInterface.addressEntries().size())
                {
                    MegaApi::log(MegaApi::LOG_LEVEL_INFO, "Local IP change detected");
                    disconnect = true;
                }
                else
                {
                    for (int k = 0; k < addresses.size(); k++)
                    {
                        QHostAddress ip = addresses.at(k).ip();
                        switch (ip.protocol())
                        {
                            case QAbstractSocket::IPv4Protocol:
                            case QAbstractSocket::IPv6Protocol:
                            {
                                QList<QNetworkAddressEntry> oldAddresses = oldNetworkInterface.addressEntries();
                                int l = 0;
                                while (l < oldAddresses.size())
                                {
                                    if (oldAddresses.at(l).ip().toString() == ip.toString())
                                    {
                                        break;
                                    }
                                    l++;
                                }

                                if (l == oldAddresses.size())
                                {
                                    //New IP
                                    MegaApi::log(MegaApi::LOG_LEVEL_INFO, QString::fromUtf8("New IP detected (%1) for interface %2").arg(ip.toString()).arg(networkInterface.name()).toUtf8().constData());
                                    disconnect = true;
                                }
                            }
                            default:
                                break;
                        }
                    }
                }
            }
        }
    }

    if (disconnect || (QDateTime::currentMSecsSinceEpoch() - lastActiveTime) > Preferences::MAX_IDLE_TIME_MS)
    {
        MegaApi::log(MegaApi::LOG_LEVEL_INFO, "Reconnecting due to local network changes");
        megaApi->retryPendingConnections(true, true);
        activeNetworkInterfaces = newNetworkInterfaces;
        lastActiveTime = QDateTime::currentMSecsSinceEpoch();
    }
    else
    {
        MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, "Local network adapters haven't changed");
    }
}

void MegaApplication::checkMemoryUsage()
{
    long long numNodes = megaApi->getNumNodes();
    long long numLocalNodes = megaApi->getNumLocalNodes();
    long long totalNodes = numNodes + numLocalNodes;
    long long totalTransfers =  megaApi->getNumPendingUploads() + megaApi->getNumPendingDownloads();
    long long procesUsage = 0;

    if (!totalNodes)
    {
        totalNodes++;
    }

#ifdef _WIN32
    PROCESS_MEMORY_COUNTERS_EX pmc;
    if (!GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc)))
    {
        return;
    }
    procesUsage = pmc.PrivateUsage;
#else
    #ifdef __APPLE__
        struct task_basic_info t_info;
        mach_msg_type_number_t t_info_count = TASK_BASIC_INFO_COUNT;

        if (KERN_SUCCESS == task_info(mach_task_self(),
                                      TASK_BASIC_INFO, (task_info_t)&t_info,
                                      &t_info_count))
        {
            procesUsage = t_info.resident_size;
        }
        else
        {
            return;
        }
    #endif
#endif

    MegaApi::log(MegaApi::LOG_LEVEL_DEBUG,
                 QString::fromUtf8("Memory usage: %1 MB / %2 Nodes / %3 LocalNodes / %4 B/N / %5 transfers")
                 .arg(procesUsage / (1024 * 1024))
                 .arg(numNodes).arg(numLocalNodes)
                 .arg((float)procesUsage / totalNodes)
                 .arg(totalTransfers).toUtf8().constData());

    if (procesUsage > maxMemoryUsage)
    {
        maxMemoryUsage = procesUsage;
    }

    if (maxMemoryUsage > preferences->getMaxMemoryUsage()
            && maxMemoryUsage > 268435456 //256MB
            + 2028 * totalNodes // 2KB per node
            + 5120 * totalTransfers) // 5KB per transfer
    {
        long long currentTime = QDateTime::currentMSecsSinceEpoch();
        if (currentTime - preferences->getMaxMemoryReportTime() > 86400000)
        {
            preferences->setMaxMemoryUsage(maxMemoryUsage);
            preferences->setMaxMemoryReportTime(currentTime);
            megaApi->sendEvent(99509, QString::fromUtf8("%1 %2 %3")
                               .arg(maxMemoryUsage)
                               .arg(numNodes)
                               .arg(numLocalNodes).toUtf8().constData());
        }
    }
}

void MegaApplication::checkOverStorageStates()
{
    if (!preferences->logged() || ((!infoDialog || !infoDialog->isVisible()) && !storageOverquotaDialog && !Platform::isUserActive()))
    {
        return;
    }

    if (appliedStorageState == MegaApi::STORAGE_STATE_RED)
    {
        if (!preferences->getOverStorageDialogExecution()
                || ((QDateTime::currentMSecsSinceEpoch() - preferences->getOverStorageDialogExecution()) > Preferences::OQ_DIALOG_INTERVAL_MS))
        {
            preferences->setOverStorageDialogExecution(QDateTime::currentMSecsSinceEpoch());
            megaApi->sendEvent(99518, "Overstorage dialog shown");
            if (!storageOverquotaDialog)
            {
                storageOverquotaDialog = new UpgradeOverStorage(megaApi, pricing);
                connect(storageOverquotaDialog, SIGNAL(finished(int)), this, SLOT(storageOverquotaDialogFinished(int)));
                storageOverquotaDialog->show();
            }
            else
            {
                storageOverquotaDialog->activateWindow();
                storageOverquotaDialog->raise();
            }
        }
        else if (((QDateTime::currentMSecsSinceEpoch() - preferences->getOverStorageDialogExecution()) > Preferences::OQ_NOTIFICATION_INTERVAL_MS)
                     && (!preferences->getOverStorageNotificationExecution() || ((QDateTime::currentMSecsSinceEpoch() - preferences->getOverStorageNotificationExecution()) > Preferences::OQ_NOTIFICATION_INTERVAL_MS)))
        {
            preferences->setOverStorageNotificationExecution(QDateTime::currentMSecsSinceEpoch());
            megaApi->sendEvent(99519, "Overstorage notification shown");
            sendOverStorageNotification(Preferences::STATE_OVER_STORAGE);
        }

        if (infoDialog)
        {
            if (!preferences->getOverStorageDismissExecution()
                    || ((QDateTime::currentMSecsSinceEpoch() - preferences->getOverStorageDismissExecution()) > Preferences::OQ_UI_MESSAGE_INTERVAL_MS))
            {
                if (infoDialog->updateOverStorageState(Preferences::STATE_OVER_STORAGE))
                {
                    megaApi->sendEvent(99520, "Overstorage warning shown");
                }
            }
            else
            {
                infoDialog->updateOverStorageState(Preferences::STATE_OVER_STORAGE_DISMISSED);
            }
        }
    }
    else if (appliedStorageState == MegaApi::STORAGE_STATE_ORANGE)
    {
        if (infoDialog)
        {
            if (((QDateTime::currentMSecsSinceEpoch() - preferences->getOverStorageDismissExecution()) > Preferences::ALMOST_OQ_UI_MESSAGE_INTERVAL_MS)
                         && (!preferences->getAlmostOverStorageDismissExecution() || ((QDateTime::currentMSecsSinceEpoch() - preferences->getAlmostOverStorageDismissExecution()) > Preferences::ALMOST_OQ_UI_MESSAGE_INTERVAL_MS)))
            {
                if (infoDialog->updateOverStorageState(Preferences::STATE_ALMOST_OVER_STORAGE))
                {
                    megaApi->sendEvent(99521, "Almost overstorage warning shown");
                }
            }
            else
            {
                infoDialog->updateOverStorageState(Preferences::STATE_OVER_STORAGE_DISMISSED);
            }
        }

        bool pendingTransfers = megaApi->getNumPendingDownloads() || megaApi->getNumPendingUploads();
        if (!pendingTransfers && ((QDateTime::currentMSecsSinceEpoch() - preferences->getOverStorageNotificationExecution()) > Preferences::ALMOST_OQ_UI_MESSAGE_INTERVAL_MS)
                              && ((QDateTime::currentMSecsSinceEpoch() - preferences->getOverStorageDialogExecution()) > Preferences::ALMOST_OQ_UI_MESSAGE_INTERVAL_MS)
                              && (!preferences->getAlmostOverStorageNotificationExecution() || (QDateTime::currentMSecsSinceEpoch() - preferences->getAlmostOverStorageNotificationExecution()) > Preferences::ALMOST_OQ_UI_MESSAGE_INTERVAL_MS))
        {
            preferences->setAlmostOverStorageNotificationExecution(QDateTime::currentMSecsSinceEpoch());
            megaApi->sendEvent(99522, "Almost overstorage notification shown");
            sendOverStorageNotification(Preferences::STATE_ALMOST_OVER_STORAGE);
        }

        if (storageOverquotaDialog)
        {
            storageOverquotaDialog->deleteLater();
            storageOverquotaDialog = NULL;
        }
    }
    else if (appliedStorageState == MegaApi::STORAGE_STATE_PAYWALL)
    {
        if (getUserDataRequestReady)
        {
            if (infoDialog)
            {
                infoDialog->updateOverStorageState(Preferences::STATE_PAYWALL);
            }

            if ((!preferences->getPayWallNotificationExecution() || ((QDateTime::currentMSecsSinceEpoch() - preferences->getPayWallNotificationExecution()) > Preferences::PAYWALL_NOTIFICATION_INTERVAL_MS)))
            {
                const auto daysToExpire{Utilities::getDaysToTimestamp(megaApi->getOverquotaDeadlineTs() * 1000)};
                if (daysToExpire > 0) //Only show notification if at least there is one day left
                {
                    preferences->setPayWallNotificationExecution(QDateTime::currentMSecsSinceEpoch());
                    megaApi->sendEvent(99530, "Paywall notification shown");
                    sendOverStorageNotification(Preferences::STATE_PAYWALL);
                }
            }

            if (storageOverquotaDialog)
            {
                storageOverquotaDialog->deleteLater();
                storageOverquotaDialog = NULL;
            }
        }
    }
    else
    {
        if (infoDialog)
        {
            infoDialog->updateOverStorageState(Preferences::STATE_BELOW_OVER_STORAGE);
        }

        if (storageOverquotaDialog)
        {
            storageOverquotaDialog->deleteLater();
            storageOverquotaDialog = NULL;
        }
    }

    if (infoDialog)
    {
        infoDialog->setOverQuotaMode(appliedStorageState == MegaApi::STORAGE_STATE_RED
                                     || appliedStorageState == MegaApi::STORAGE_STATE_PAYWALL);
    }
}

void MegaApplication::periodicTasks()
{
    if (appfinished)
    {
        return;
    }

    if (!cleaningSchedulerExecution || ((QDateTime::currentMSecsSinceEpoch() - cleaningSchedulerExecution) > Preferences::MIN_UPDATE_CLEANING_INTERVAL_MS))
    {
        cleaningSchedulerExecution = QDateTime::currentMSecsSinceEpoch();
        MegaApi::log(MegaApi::LOG_LEVEL_INFO, "Cleaning local cache folders");
        cleanLocalCaches();
    }

    if (queuedUserStats[0] || queuedUserStats[1] || queuedUserStats[2])
    {
        bool storage = queuedUserStats[0], transfer = queuedUserStats[1], pro = queuedUserStats[2];
        queuedUserStats[0] = queuedUserStats[1] = queuedUserStats[3] = false;
        updateUserStats(storage, transfer, pro, false, -1);
    }

    checkNetworkInterfaces();
    initLocalServer();

    static int counter = 0;
    counter++;
    if (megaApi)
    {
        if (!(counter % 6))
        {
            HTTPServer::checkAndPurgeRequests();

            if (checkupdate)
            {
                checkupdate = false;
                megaApi->sendEvent(99511, "MEGAsync updated OK");
            }

            networkConfigurationManager.updateConfigurations();
            checkMemoryUsage();
            megaApi->update();

            checkOverStorageStates();
            transferQuota->checkQuotaAndAlerts();
        }

        onGlobalSyncStateChanged(megaApi);

        if (isLinux)
        {
            updateTrayIcon();
        }

        if (isLinux && blockState && !(counter%10))
        {
            whyAmIBlocked(true);
        }
    }

    if (trayIcon)
    {
#ifdef Q_OS_LINUX
        if (counter==4 && getenv("XDG_CURRENT_DESKTOP") && !strcmp(getenv("XDG_CURRENT_DESKTOP"),"XFCE"))
        {
            trayIcon->hide();
        }
#endif
        trayIcon->show();
    }
}

void MegaApplication::cleanAll()
{
    if (appfinished)
    {
        return;
    }
    appfinished = true;

#ifndef DEBUG
    CrashHandler::instance()->Disable();
#endif

    qInstallMsgHandler(0);
#if QT_VERSION >= 0x050000
    qInstallMessageHandler(0);
#endif

    periodicTasksTimer->stop();
    stopUpdateTask();
    Platform::stopShellDispatcher();
    for (int i = 0; i < model->getNumSyncedFolders(); i++)
    {
        auto syncSetting = model->getSyncSetting(i);
        notifyItemChange(syncSetting->getLocalFolder(), MegaApi::STATE_NONE);
    }

    closeDialogs();
    removeAllFinishedTransfers();
    clearViewedTransfers();

    delete storageOverquotaDialog;
    storageOverquotaDialog = NULL;
    delete infoWizard;
    infoWizard = NULL;
    delete infoDialog;
    infoDialog = NULL;
    delete httpServer;
    httpServer = NULL;
    delete httpsServer;
    httpsServer = NULL;
    delete uploader;
    uploader = NULL;
    delete downloader;
    downloader = NULL;
    delete delegateListener;
    delegateListener = NULL;
    delete pricing;
    pricing = NULL;

    // Delete notifications stuff
    delete notificationsModel;
    notificationsModel = NULL;
    delete notificationsProxyModel;
    notificationsProxyModel = NULL;
    delete notificationsDelegate;
    notificationsDelegate = NULL;

    // Delete menus and menu items
    deleteMenu(initialMenu.release());
    deleteMenu(infoDialogMenu.release());
    deleteMenu(syncsMenu.release());
    deleteMenu(guestMenu.release());
#ifdef _WIN32
    deleteMenu(windowsMenu.release());
#endif

    // Ensure that there aren't objects deleted with deleteLater()
    // that may try to access megaApi after
    // their deletion
    QApplication::processEvents();

    delete megaApi;
    megaApi = NULL;

    delete megaApiFolders;
    megaApiFolders = NULL;

    preferences->setLastExit(QDateTime::currentMSecsSinceEpoch());
    trayIcon->deleteLater();
    trayIcon = NULL;

    logger.reset();

    if (reboot)
    {
#ifndef __APPLE__
        QString app = QString::fromUtf8("\"%1\"").arg(MegaApplication::applicationFilePath());
        QProcess::startDetached(app);
#else
        QString app = MegaApplication::applicationDirPath();
        QString launchCommand = QString::fromUtf8("open");
        QStringList args = QStringList();

        QDir appPath(app);
        appPath.cdUp();
        appPath.cdUp();

        args.append(QString::fromAscii("-n"));
        args.append(appPath.absolutePath());
        QProcess::startDetached(launchCommand, args);
#endif

#ifdef WIN32
        Sleep(2000);
#else
        sleep(2);
#endif
    }

    //QFontDatabase::removeAllApplicationFonts();
}

void MegaApplication::onDupplicateLink(QString, QString name, MegaHandle handle)
{
    if (appfinished)
    {
        return;
    }

    addRecentFile(name, handle);
}

void MegaApplication::onInstallUpdateClicked()
{
    if (appfinished)
    {
        return;
    }

    if (updateAvailable)
    {
        showInfoMessage(tr("Installing update..."));
        emit installUpdate();
    }
    else
    {
        showChangeLog();
    }
}

void MegaApplication::repositionInfoDialog()
{
    if (!infoDialog)
    {
        return;
    }

    int posx, posy;
    calculateInfoDialogCoordinates(infoDialog, &posx, &posy);

    // An issue occurred with certain multiscreen setup that caused Qt to missplace the info dialog.
    // This works around that by ensuring infoDialog does not get incorrectly resized. in which case,
    // it is reverted to the correct size.
    infoDialog->ensurePolished();
    auto initialDialogWidth  = infoDialog->width();
    auto initialDialogHeight = infoDialog->height();
    QTimer::singleShot(1, this, [this, initialDialogWidth, initialDialogHeight, posx, posy](){
        if (infoDialog->width() > initialDialogWidth || infoDialog->height() > initialDialogHeight) //miss scaling detected
        {
            MegaApi::log(MegaApi::LOG_LEVEL_ERROR,
                         QString::fromUtf8("A dialog. New size = %1,%2. should be %3,%4 ")
                         .arg(infoDialog->width()).arg(infoDialog->height()).arg(initialDialogWidth).arg(initialDialogHeight)
                         .toUtf8().constData());

            infoDialog->resize(initialDialogWidth,initialDialogHeight);

            auto iDPos = infoDialog->pos();
            if (iDPos.x() != posx || iDPos.y() != posy )
            {
                MegaApi::log(MegaApi::LOG_LEVEL_ERROR,
                             QString::fromUtf8("Missplaced info dialog. New pos = %1,%2. should be %3,%4 ")
                             .arg(iDPos.x()).arg(iDPos.y()).arg(posx).arg(posy)
                             .toUtf8().constData());
                infoDialog->move(posx, posy);

                QTimer::singleShot(1, this, [this, initialDialogWidth, initialDialogHeight, posx, posy](){
                    if (infoDialog->width() > initialDialogWidth || infoDialog->height() > initialDialogHeight) //miss scaling detected
                    {
                        MegaApi::log(MegaApi::LOG_LEVEL_ERROR,
                                     QString::fromUtf8("Missscaled info dialog after second move. New size = %1,%2. should be %3,%4 ")
                                     .arg(infoDialog->width()).arg(infoDialog->height()).arg(initialDialogWidth).arg(initialDialogHeight)
                                     .toUtf8().constData());

                        infoDialog->resize(initialDialogWidth,initialDialogHeight);
                    }
                });
            }
        }
    });

    if (isLinux)
    {
        unityFix();
    }

    infoDialog->move(posx, posy);

#ifdef __APPLE__
    QPoint positionTrayIcon = trayIcon->geometry().topLeft();
    QPoint globalCoordinates(positionTrayIcon.x() + trayIcon->geometry().width()/2, posy);

    //Work-Around to paint the arrow correctly
    infoDialog->show();
    QPixmap px = QPixmap::grabWidget(infoDialog);
    infoDialog->hide();
    QPoint localCoordinates = infoDialog->mapFromGlobal(globalCoordinates);
    infoDialog->moveArrow(localCoordinates);
#endif
}

void MegaApplication::showInfoDialog()
{
    if (appfinished)
    {
        return;
    }

    if (isLinux && showStatusAction && megaApi)
    {
        megaApi->retryPendingConnections();
    }

#ifdef WIN32

    if (QWidget *anyModalWindow = QApplication::activeModalWidget())
    {
        // If the InfoDialog has opened any MessageBox (eg. enter your email), those must be closed first (as we are executing from that dialog's message loop!)
        // Bring that dialog to the front for the user to dismiss.
        anyModalWindow->activateWindow();
        return;
    }

    if (infoDialog)
    {
        // in case the screens have changed, eg. laptop with 2 monitors attached (200%, main:100%, 150%), lock screen, unplug monitors, wait 30s, plug monitors, unlock screen:  infoDialog may be double size and only showing 1/4 or 1/2
        infoDialog->setWindowFlags(Qt::FramelessWindowHint);
        infoDialog->setWindowFlags(Qt::FramelessWindowHint | Qt::Popup);
    }
#endif

    const auto transferQuotaWaitTimeExpired{transferOverQuotaWaitTimeExpiredReceived && !transferQuota->isOverQuota()};
    const auto loggedAndNotBandwidthOverquota{preferences && preferences->logged()};
    if (loggedAndNotBandwidthOverquota && transferQuotaWaitTimeExpired)
    {
        transferOverQuotaWaitTimeExpiredReceived = false;
        updateUserStats(false, true, false, true, USERSTATS_BANDWIDTH_TIMEOUT_SHOWINFODIALOG);
    }

    if (infoDialog)
    {
        if (!infoDialog->isVisible() || ((infoDialog->windowState() & Qt::WindowMinimized)) )
        {
            if (storageState == MegaApi::STORAGE_STATE_RED)
            {
                megaApi->sendEvent(99523, "Main dialog shown while overquota");
            }
            else if (storageState == MegaApi::STORAGE_STATE_ORANGE)
            {
                megaApi->sendEvent(99524, "Main dialog shown while almost overquota");
            }

            repositionInfoDialog();

            infoDialog->show();
            infoDialog->updateDialogState();
            infoDialog->raise();
            infoDialog->activateWindow();
            infoDialog->highDpiResize.queueRedraw();
        }
        else
        {
            infoDialog->closeSyncsMenu();
            if (infoDialogMenu && infoDialogMenu->isVisible())
            {
                infoDialogMenu->close();
            }
            if (guestMenu && guestMenu->isVisible())
            {
                guestMenu->close();
            }

            infoDialog->hide();
        }
    }

    updateUserStats(false, true, false, true, USERSTATS_SHOWMAINDIALOG);
}

void MegaApplication::calculateInfoDialogCoordinates(QDialog *dialog, int *posx, int *posy)
{
    if (appfinished)
    {
        return;
    }

    int xSign = 1;
    int ySign = 1;
    QPoint position, positionTrayIcon;
    QRect screenGeometry;

    #ifdef __APPLE__
        positionTrayIcon = trayIcon->geometry().topLeft();
    #endif

    position = QCursor::pos();
    QDesktopWidget *desktop = QApplication::desktop();
    int screenIndex = desktop->screenNumber(position);
    screenGeometry = desktop->availableGeometry(screenIndex);
    if (!screenGeometry.isValid())
    {
        screenGeometry = desktop->screenGeometry(screenIndex);
        if (screenGeometry.isValid())
        {
            screenGeometry.setTop(28);
        }
        else
        {
            screenGeometry = dialog->rect();
            screenGeometry.setBottom(screenGeometry.bottom() + 4);
            screenGeometry.setRight(screenGeometry.right() + 4);
        }
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


    #ifdef __APPLE__
        if (positionTrayIcon.x() || positionTrayIcon.y())
        {
            if ((positionTrayIcon.x() + dialog->width() / 2) > screenGeometry.right())
            {
                *posx = screenGeometry.right() - dialog->width() - 1;
            }
            else
            {
                *posx = positionTrayIcon.x() + trayIcon->geometry().width() / 2 - dialog->width() / 2 - 1;
            }
        }
        else
        {
            *posx = screenGeometry.right() - dialog->width() - 1;
        }
        *posy = screenIndex ? screenGeometry.top() + 22: screenGeometry.top();

        if (*posy == 0)
        {
            *posy = 22;
        }
    #else
        #ifdef WIN32
            QRect totalGeometry = QApplication::desktop()->screenGeometry();
            APPBARDATA pabd;
            pabd.cbSize = sizeof(APPBARDATA);
            pabd.hWnd = FindWindow(L"Shell_TrayWnd", NULL);
            //TODO: the following only takes into account the position of the tray for the main screen.
            //Alternatively we might want to do that according to where the taskbar is for the targetted screen.
            if (pabd.hWnd && SHAppBarMessage(ABM_GETTASKBARPOS, &pabd)
                    && pabd.rc.right != pabd.rc.left && pabd.rc.bottom != pabd.rc.top)
            {
                int size;
                switch (pabd.uEdge)
                {
                    case ABE_LEFT:
                        position = screenGeometry.bottomLeft();
                        if (totalGeometry == screenGeometry)
                        {
                            size = pabd.rc.right - pabd.rc.left;
                            size = size * screenGeometry.height() / (pabd.rc.bottom - pabd.rc.top);
                            screenGeometry.setLeft(screenGeometry.left() + size);
                        }
                        break;
                    case ABE_RIGHT:
                        position = screenGeometry.bottomRight();
                        if (totalGeometry == screenGeometry)
                        {
                            size = pabd.rc.right - pabd.rc.left;
                            size = size * screenGeometry.height() / (pabd.rc.bottom - pabd.rc.top);
                            screenGeometry.setRight(screenGeometry.right() - size);
                        }
                        break;
                    case ABE_TOP:
                        position = screenGeometry.topRight();
                        if (totalGeometry == screenGeometry)
                        {
                            size = pabd.rc.bottom - pabd.rc.top;
                            size = size * screenGeometry.width() / (pabd.rc.right - pabd.rc.left);
                            screenGeometry.setTop(screenGeometry.top() + size);
                        }
                        break;
                    case ABE_BOTTOM:
                        position = screenGeometry.bottomRight();
                        if (totalGeometry == screenGeometry)
                        {
                            size = pabd.rc.bottom - pabd.rc.top;
                            size = size * screenGeometry.width() / (pabd.rc.right - pabd.rc.left);
                            screenGeometry.setBottom(screenGeometry.bottom() - size);
                        }
                        break;
                }
            }
        #endif

        if (position.x() * xSign > (screenGeometry.right() / 2) * xSign)
        {
            *posx = screenGeometry.right() - dialog->width() - 2;
        }
        else
        {
            *posx = screenGeometry.left() + 2;
        }

        if (position.y() * ySign > (screenGeometry.bottom() / 2) * ySign)
        {
            *posy = screenGeometry.bottom() - dialog->height() - 2;
        }
        else
        {
            *posy = screenGeometry.top() + 2;
        }
    #endif

}

void MegaApplication::deleteMenu(QMenu *menu)
{
    if (menu)
    {
        QList<QAction *> actions = menu->actions();
        for (int i = 0; i < actions.size(); i++)
        {
            menu->removeAction(actions[i]);
            delete actions[i];
        }
        delete menu;
    }
}

void MegaApplication::startHttpServer()
{
    if (!httpServer)
    {
        //Start the HTTP server
        httpServer = new HTTPServer(megaApi, Preferences::HTTP_PORT, false);
        connect(httpServer, SIGNAL(onLinkReceived(QString, QString)), this, SLOT(externalDownload(QString, QString)), Qt::QueuedConnection);
        connect(httpServer, SIGNAL(onExternalDownloadRequested(QQueue<mega::MegaNode *>)), this, SLOT(externalDownload(QQueue<mega::MegaNode *>)));
        connect(httpServer, SIGNAL(onExternalDownloadRequestFinished()), this, SLOT(processDownloads()), Qt::QueuedConnection);
        connect(httpServer, SIGNAL(onExternalFileUploadRequested(qlonglong)), this, SLOT(externalFileUpload(qlonglong)), Qt::QueuedConnection);
        connect(httpServer, SIGNAL(onExternalFolderUploadRequested(qlonglong)), this, SLOT(externalFolderUpload(qlonglong)), Qt::QueuedConnection);
        connect(httpServer, SIGNAL(onExternalFolderSyncRequested(qlonglong)), this, SLOT(externalFolderSync(qlonglong)), Qt::QueuedConnection);
        connect(httpServer, SIGNAL(onExternalOpenTransferManagerRequested(int)), this, SLOT(externalOpenTransferManager(int)), Qt::QueuedConnection);
        connect(httpServer, SIGNAL(onExternalShowInFolderRequested(QString)), this, SLOT(openFolderPath(QString)), Qt::QueuedConnection);

        MegaApi::log(MegaApi::LOG_LEVEL_INFO, "Local HTTP server started");
    }
}

void MegaApplication::startHttpsServer()
{
    if (!httpsServer)
    {
        //Start the HTTPS server
        httpsServer = new HTTPServer(megaApi, Preferences::HTTPS_PORT, true);
        connect(httpsServer, SIGNAL(onLinkReceived(QString, QString)), this, SLOT(externalDownload(QString, QString)), Qt::QueuedConnection);
        connect(httpsServer, SIGNAL(onExternalDownloadRequested(QQueue<mega::MegaNode *>)), this, SLOT(externalDownload(QQueue<mega::MegaNode *>)));
        connect(httpsServer, SIGNAL(onExternalDownloadRequestFinished()), this, SLOT(processDownloads()), Qt::QueuedConnection);
        connect(httpsServer, SIGNAL(onExternalFileUploadRequested(qlonglong)), this, SLOT(externalFileUpload(qlonglong)), Qt::QueuedConnection);
        connect(httpsServer, SIGNAL(onExternalFolderUploadRequested(qlonglong)), this, SLOT(externalFolderUpload(qlonglong)), Qt::QueuedConnection);
        connect(httpsServer, SIGNAL(onExternalFolderSyncRequested(qlonglong)), this, SLOT(externalFolderSync(qlonglong)), Qt::QueuedConnection);
        connect(httpsServer, SIGNAL(onExternalOpenTransferManagerRequested(int)), this, SLOT(externalOpenTransferManager(int)), Qt::QueuedConnection);
        connect(httpsServer, SIGNAL(onExternalShowInFolderRequested(QString)), this, SLOT(openFolderPath(QString)), Qt::QueuedConnection);
        connect(httpsServer, SIGNAL(onConnectionError()), this, SLOT(onHttpServerConnectionError()), Qt::QueuedConnection);

        MegaApi::log(MegaApi::LOG_LEVEL_INFO, "Local HTTPS server started");
    }
}

void MegaApplication::initLocalServer()
{
    // Run both servers for now, until we receive the confirmation of the criteria to start them dynamically
    if (!httpServer) // && Platform::shouldRunHttpServer())
    {
        startHttpServer();
    }

    if (!updatingSSLcert) // && (httpsServer || Platform::shouldRunHttpsServer()))
    {
        long long currentTime = QDateTime::currentMSecsSinceEpoch() / 1000;
        if ((currentTime - lastSSLcertUpdate) > Preferences::LOCAL_HTTPS_CERT_RENEW_INTERVAL_SECS)
        {
            renewLocalSSLcert();
        }
    }
}

void MegaApplication::sendOverStorageNotification(int state)
{
    switch (state)
    {
        case Preferences::STATE_ALMOST_OVER_STORAGE:
        {
            MegaNotification *notification = new MegaNotification();
            notification->setTitle(tr("Your account is almost full."));
            notification->setText(tr("Upgrade now to a PRO account."));
            notification->setActions(QStringList() << tr("Get PRO"));
            connect(notification, SIGNAL(activated(int)), this, SLOT(redirectToUpgrade(int)));
            notificator->notify(notification);
            break;
        }
        case Preferences::STATE_OVER_STORAGE:
        {
            MegaNotification *notification = new MegaNotification();
            notification->setTitle(tr("Your account is full."));
            notification->setText(tr("Upgrade now to a PRO account."));
            notification->setActions(QStringList() << tr("Get PRO"));
            connect(notification, SIGNAL(activated(int)), this, SLOT(redirectToUpgrade(int)));
            notificator->notify(notification);
            break;
        }
        case Preferences::STATE_PAYWALL:
        {
            MegaNotification *notification = new MegaNotification();
            notification->setTitle(tr("Your data is at risk"));
            notification->setText(tr("You have [A] days left to save your data").replace(QString::fromUtf8("[A]"), QString::number(Utilities::getDaysToTimestamp(megaApi->getOverquotaDeadlineTs() * 1000))));
            notification->setActions(QStringList() << tr("Get PRO"));
            connect(notification, SIGNAL(activated(int)), this, SLOT(redirectToUpgrade(int)));
            notificator->notify(notification);
            break;
        }
        default:
            break;
    }
}

void MegaApplication::sendBusinessWarningNotification()
{
    switch (businessStatus)
    {
        case MegaApi::BUSINESS_STATUS_GRACE_PERIOD:
        {
            if (megaApi->isMasterBusinessAccount())
            {
                MegaNotification *notification = new MegaNotification();
                notification->setTitle(tr("Payment Failed"));
                notification->setText(tr("Please resolve your payment issue to avoid suspension of your account."));
                notification->setActions(QStringList() << tr("Pay Now"));
                connect(notification, SIGNAL(activated(int)), this, SLOT(redirectToPayBusiness(int)));
                notificator->notify(notification);
            }
            break;
        }
        case MegaApi::BUSINESS_STATUS_EXPIRED:
        {
            MegaNotification *notification = new MegaNotification();

            if (megaApi->isMasterBusinessAccount())
            {
                notification->setTitle(tr("Your Business account is expired"));
                notification->setText(tr("Your account is suspended as read only until you proceed with the needed payments."));
                notification->setActions(QStringList() << tr("Pay Now"));
                connect(notification, SIGNAL(activated(int)), this, SLOT(redirectToPayBusiness(int)));
            }
            else
            {
                notification->setTitle(tr("Account Suspended"));
                notification->setText(tr("Contact your business account administrator to resolve the issue and activate your account."));
            }

            notificator->notify(notification);
            break;
        }
        default:
            break;
    }
}

bool MegaApplication::eventFilter(QObject *obj, QEvent *e)
{
    if (obj == infoDialogMenu.get())
    {
        if (e->type() == QEvent::Leave)
        {
            if (lastHovered)
            {
                lastHovered->setHighlight(false);
                lastHovered = NULL;
            }
        }
    }

    return QApplication::eventFilter(obj, e);
}

void MegaApplication::createInfoDialog()
{
    infoDialog = new InfoDialog(this);
    connect(infoDialog, &InfoDialog::dismissStorageOverquota, this, &MegaApplication::onDismissStorageOverquota);
    connect(infoDialog, &InfoDialog::dismissTransferOverquota, transferQuota.get(), &TransferQuota::onDismissOverQuotaUiAlert);
    connect(infoDialog, &InfoDialog::dismissTransferAlmostOverquota, transferQuota.get(), &TransferQuota::onDismissAlmostOverQuotaUiMessage);
    connect(infoDialog, &InfoDialog::userActivity, this, &MegaApplication::registerUserActivity);
    connect(transferQuota.get(), &TransferQuota::sendState, infoDialog, &InfoDialog::setBandwidthOverquotaState);
    connect(transferQuota.get(), &TransferQuota::overQuotaUiMessage, infoDialog, &InfoDialog::enableTransferOverquotaAlert);
    connect(transferQuota.get(), &TransferQuota::almostOverQuotaUiMessage, infoDialog, &InfoDialog::enableTransferAlmostOverquotaAlert);
}

int MegaApplication::getAppliedStorageState() const
{
    return appliedStorageState;
}

bool MegaApplication::isAppliedStorageOverquota() const
{
    return appliedStorageState == MegaApi::STORAGE_STATE_RED || appliedStorageState == MegaApi::STORAGE_STATE_PAYWALL;
}

MegaPricing *MegaApplication::getPricing() const
{
    return pricing;
}

int MegaApplication::getBlockState() const
{
    return blockState;
}

SetupWizard *MegaApplication::getSetupWizard() const
{
    return setupWizard;
}

TransferMetaData* MegaApplication::getTransferAppData(unsigned long long appDataID)
{
    QHash<unsigned long long, TransferMetaData*>::const_iterator it = transferAppData.find(appDataID);
    if(it == transferAppData.end())
    {
        return NULL;
    }

    TransferMetaData* value = it.value();
    return value;
}

void MegaApplication::renewLocalSSLcert()
{
    if (!updatingSSLcert)
    {
        lastSSLcertUpdate = QDateTime::currentMSecsSinceEpoch() / 1000;
        megaApi->getLocalSSLCertificate();
    }
}


void MegaApplication::onHttpServerConnectionError()
{
    auto now = QDateTime::currentDateTime().toMSecsSinceEpoch() / 1000;
    if (now - this->lastTsConnectionError > 10)
    {
        this->lastTsConnectionError = now;
        this->renewLocalSSLcert();
    }
    else
    {
        MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, "Local SSL cert renewal discarded");
    }
}
void MegaApplication::triggerInstallUpdate()
{
    if (appfinished)
    {
        return;
    }

    emit installUpdate();
}

void MegaApplication::scanningAnimationStep()
{
    if (appfinished)
    {
        return;
    }

    scanningAnimationIndex = scanningAnimationIndex%4;
    scanningAnimationIndex++;
    QIcon ic = QIcon(QString::fromAscii("://images/icon_syncing_mac") +
                     QString::number(scanningAnimationIndex) + QString::fromAscii(".png"));
#ifdef __APPLE__
    ic.setIsMask(true);
#endif
    trayIcon->setIcon(ic);
}

void MegaApplication::runConnectivityCheck()
{
    if (appfinished)
    {
        return;
    }

    QNetworkProxy proxy;
    proxy.setType(QNetworkProxy::NoProxy);
    if (preferences->proxyType() == Preferences::PROXY_TYPE_CUSTOM)
    {
        int proxyProtocol = preferences->proxyProtocol();
        switch (proxyProtocol)
        {
        case Preferences::PROXY_PROTOCOL_SOCKS5H:
            proxy.setType(QNetworkProxy::Socks5Proxy);
            break;
        default:
            proxy.setType(QNetworkProxy::HttpProxy);
            break;
        }

        proxy.setHostName(preferences->proxyServer());
        proxy.setPort(preferences->proxyPort());
        if (preferences->proxyRequiresAuth())
        {
            proxy.setUser(preferences->getProxyUsername());
            proxy.setPassword(preferences->getProxyPassword());
        }
    }
    else if (preferences->proxyType() == MegaProxy::PROXY_AUTO)
    {
        MegaProxy* autoProxy = megaApi->getAutoProxySettings();
        if (autoProxy && autoProxy->getProxyType()==MegaProxy::PROXY_CUSTOM)
        {
            string sProxyURL = autoProxy->getProxyURL();
            QString proxyURL = QString::fromUtf8(sProxyURL.data());

            QStringList parts = proxyURL.split(QString::fromAscii("://"));
            if (parts.size() == 2 && parts[0].startsWith(QString::fromUtf8("socks")))
            {
                proxy.setType(QNetworkProxy::Socks5Proxy);
            }
            else
            {
                proxy.setType(QNetworkProxy::HttpProxy);
            }

            QStringList arguments = parts[parts.size()-1].split(QString::fromAscii(":"));
            if (arguments.size() == 2)
            {
                proxy.setHostName(arguments[0]);
                proxy.setPort(arguments[1].toInt());
            }
        }
        delete autoProxy;
    }

    ConnectivityChecker *connectivityChecker = new ConnectivityChecker(Preferences::PROXY_TEST_URL);
    connectivityChecker->setProxy(proxy);
    connectivityChecker->setTestString(Preferences::PROXY_TEST_SUBSTRING);
    connectivityChecker->setTimeout(Preferences::PROXY_TEST_TIMEOUT_MS);

    connect(connectivityChecker, SIGNAL(testError()), this, SLOT(onConnectivityCheckError()));
    connect(connectivityChecker, SIGNAL(testSuccess()), this, SLOT(onConnectivityCheckSuccess()));
    connect(connectivityChecker, SIGNAL(testFinished()), connectivityChecker, SLOT(deleteLater()));

    connectivityChecker->startCheck();
    MegaApi::log(MegaApi::LOG_LEVEL_INFO, "Running connectivity test...");
}

void MegaApplication::onConnectivityCheckSuccess()
{
    if (appfinished)
    {
        return;
    }

    MegaApi::log(MegaApi::LOG_LEVEL_INFO, "Connectivity test finished OK");
}

void MegaApplication::onConnectivityCheckError()
{
    if (appfinished)
    {
        return;
    }

    showErrorMessage(tr("MEGAsync is unable to connect. Please check your Internet connectivity and local firewall configuration. Note that most antivirus software includes a firewall."));
}

void MegaApplication::proExpirityTimedOut()
{
    updateUserStats(true, true, true, true, USERSTATS_PRO_EXPIRED);
}

void MegaApplication::setupWizardFinished(int result)
{
    if (appfinished)
    {
        return;
    }

    QList<PreConfiguredSync> syncs;
    if (setupWizard)
    {
        syncs = setupWizard->preconfiguredSyncs();
        setupWizard->deleteLater();
        setupWizard = NULL;
    }

    if (result == QDialog::Rejected)
    {
        if (!infoWizard && (downloadQueue.size() || pendingLinks.size()))
        {
            QQueue<MegaNode *>::iterator it;
            for (it = downloadQueue.begin(); it != downloadQueue.end(); ++it)
            {
                HTTPServer::onTransferDataUpdate((*it)->getHandle(), MegaTransfer::STATE_CANCELLED, 0, 0, 0, QString());
            }

            for (QMap<QString, QString>::iterator it = pendingLinks.begin(); it != pendingLinks.end(); it++)
            {
                QString link = it.key();
                QString handle = link.mid(18, 8);
                HTTPServer::onTransferDataUpdate(megaApi->base64ToHandle(handle.toUtf8().constData()),
                                                 MegaTransfer::STATE_CANCELLED, 0, 0, 0, QString());
            }

            qDeleteAll(downloadQueue);
            downloadQueue.clear();
            pendingLinks.clear();
            showInfoMessage(tr("Transfer canceled"));
        }
        return;
    }

    QStringList exclusions = preferences->getExcludedSyncNames();
    vector<string> vExclusions;
    for (int i = 0; i < exclusions.size(); i++)
    {
        vExclusions.push_back(exclusions[i].toUtf8().constData());
    }
    megaApi->setExcludedNames(&vExclusions);

    QStringList exclusionPaths = preferences->getExcludedSyncPaths();
    vector<string> vExclusionPaths;
    for (int i = 0; i < exclusionPaths.size(); i++)
    {
        vExclusionPaths.push_back(exclusionPaths[i].toUtf8().constData());
    }
    megaApi->setExcludedPaths(&vExclusionPaths);

    if (preferences->lowerSizeLimit())
    {
        megaApi->setExclusionLowerSizeLimit(preferences->lowerSizeLimitValue() * pow((float)1024, preferences->lowerSizeLimitUnit()));
    }
    else
    {
        megaApi->setExclusionLowerSizeLimit(0);
    }

    if (preferences->upperSizeLimit())
    {
        megaApi->setExclusionUpperSizeLimit(preferences->upperSizeLimitValue() * pow((float)1024, preferences->upperSizeLimitUnit()));
    }
    else
    {
        megaApi->setExclusionUpperSizeLimit(0);
    }

    if (infoDialog && infoDialog->isVisible())
    {
        infoDialog->hide();
    }

    loggedIn(true);
    startSyncs(syncs);
}

void MegaApplication::storageOverquotaDialogFinished(int)
{
    if (appfinished)
    {
        return;
    }

    if (storageOverquotaDialog)
    {
        storageOverquotaDialog->deleteLater();
        storageOverquotaDialog = NULL;
    }
}

void MegaApplication::infoWizardDialogFinished(int result)
{
    if (appfinished)
    {
        return;
    }

    if (infoWizard)
    {
        infoWizard->deleteLater();
        infoWizard = NULL;
    }

    if (result != QDialog::Accepted)
    {
        if (!setupWizard && (downloadQueue.size() || pendingLinks.size()))
        {
            QQueue<MegaNode *>::iterator it;
            for (it = downloadQueue.begin(); it != downloadQueue.end(); ++it)
            {
                HTTPServer::onTransferDataUpdate((*it)->getHandle(), MegaTransfer::STATE_CANCELLED, 0, 0, 0, QString());
            }

            for (QMap<QString, QString>::iterator it = pendingLinks.begin(); it != pendingLinks.end(); it++)
            {
                QString link = it.key();
                QString handle = link.mid(18, 8);
                HTTPServer::onTransferDataUpdate(megaApi->base64ToHandle(handle.toUtf8().constData()),
                                                 MegaTransfer::STATE_CANCELLED, 0, 0, 0, QString());
            }

            qDeleteAll(downloadQueue);
            downloadQueue.clear();
            pendingLinks.clear();
            showInfoMessage(tr("Transfer canceled"));
        }
    }
}

void MegaApplication::unlink(bool keepLogs)
{
    if (appfinished)
    {
        return;
    }

    //Reset fields that will be initialized again upon login
    qDeleteAll(downloadQueue);
    downloadQueue.clear();
    mRootNode.reset();
    mFetchingNodes = false;
    mQueringWhyAmIBlocked = false;
    whyamiblockedPeriodicPetition = false;
    megaApi->logout();
    Platform::notifyAllSyncFoldersRemoved();

    for (unsigned i = 3; i--; )
    {
        inflightUserStats[i] = false;
        userStatsLastRequest[i] = 0;
        queuedUserStats[i] = false;
    }
    queuedStorageUserStatsReason = 0;

    if (!keepLogs)
    {
        logger->cleanLogs();
    }
}

void MegaApplication::cleanLocalCaches(bool all)
{
    if (!preferences->logged())
    {
        return;
    }

    if (all || preferences->cleanerDaysLimit())
    {
        int timeLimitDays = preferences->cleanerDaysLimitValue();
        for (int i = 0; i < model->getNumSyncedFolders(); i++)
        {
            auto syncSetting = model->getSyncSetting(i);
            QString syncPath = syncSetting->getLocalFolder();
            if (!syncPath.isEmpty())
            {
                QDir cacheDir(syncPath + QDir::separator() + QString::fromAscii(MEGA_DEBRIS_FOLDER));
                if (cacheDir.exists())
                {
                    QFileInfoList dailyCaches = cacheDir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot);
                    for (int i = 0; i < dailyCaches.size(); i++)
                    {
                        QFileInfo cacheFolder = dailyCaches[i];
                        if (!cacheFolder.fileName().compare(QString::fromUtf8("tmp"))) //DO NOT REMOVE tmp subfolder
                        {
                            continue;
                        }

                        QDateTime creationTime(cacheFolder.created());
                        if (all || (creationTime.isValid() && creationTime.daysTo(QDateTime::currentDateTime()) > timeLimitDays) )
                        {
                            Utilities::removeRecursively(cacheFolder.canonicalFilePath());
                        }
                    }
                }
            }
        }
    }
}

void MegaApplication::showInfoMessage(QString message, QString title)
{
    if (appfinished)
    {
        return;
    }

    MegaApi::log(MegaApi::LOG_LEVEL_INFO, message.toUtf8().constData());

    if (notificator)
    {
#ifdef __APPLE__
        if (infoDialog && infoDialog->isVisible())
        {
            infoDialog->hide();
        }
#endif
        lastTrayMessage = message;
        notificator->notify(Notificator::Information, title, message,
                            QIcon(QString::fromUtf8("://images/app_128.png")));
    }
    else
    {
        QMegaMessageBox::information(nullptr, title, message);
    }
}

void MegaApplication::showWarningMessage(QString message, QString title)
{
    if (appfinished)
    {
        return;
    }

    MegaApi::log(MegaApi::LOG_LEVEL_WARNING, message.toUtf8().constData());

    if (!preferences->showNotifications())
    {
        return;
    }

    if (notificator)
    {
        lastTrayMessage = message;
        notificator->notify(Notificator::Warning, title, message,
                                    QIcon(QString::fromUtf8("://images/app_128.png")));
    }
    else QMegaMessageBox::warning(nullptr, title, message);
}

void MegaApplication::showErrorMessage(QString message, QString title)
{
    if (appfinished)
    {
        return;
    }

    // Avoid spamming user with repeated notifications.
    if ((lastTsErrorMessageShown && (QDateTime::currentMSecsSinceEpoch() - lastTsErrorMessageShown) < 3000)
            && !lastNotificationError.compare(message))

    {
        return;
    }

    lastNotificationError = message;
    lastTsErrorMessageShown = QDateTime::currentMSecsSinceEpoch();

    MegaApi::log(MegaApi::LOG_LEVEL_ERROR, message.toUtf8().constData());
    if (notificator)
    {
#ifdef __APPLE__
        if (infoDialog && infoDialog->isVisible())
        {
            infoDialog->hide();
        }
#endif
        notificator->notify(Notificator::Critical, title, message,
                            QIcon(QString::fromUtf8("://images/app_128.png")));
    }
    else
    {
        QMegaMessageBox::critical(nullptr, title, message);
    }
}

void MegaApplication::showNotificationMessage(QString message, QString title)
{
    if (appfinished)
    {
        return;
    }

    MegaApi::log(MegaApi::LOG_LEVEL_INFO, message.toUtf8().constData());

    if (!preferences->showNotifications())
    {
        return;
    }

    if (notificator)
    {
        lastTrayMessage = message;
        notificator->notify(Notificator::Information, title, message,
                                    QIcon(QString::fromUtf8("://images/app_128.png")));
    }
}

//KB/s
void MegaApplication::setUploadLimit(int limit)
{
    if (appfinished)
    {
        return;
    }

    if (limit < 0)
    {
        megaApi->setUploadLimit(-1);
    }
    else
    {
        megaApi->setUploadLimit(limit * 1024);
    }
}

void MegaApplication::setMaxUploadSpeed(int limit)
{
    if (appfinished)
    {
        return;
    }

    if (limit <= 0)
    {
        megaApi->setMaxUploadSpeed(0);
    }
    else
    {
        megaApi->setMaxUploadSpeed(limit * 1024);
    }
}

void MegaApplication::setMaxDownloadSpeed(int limit)
{
    if (appfinished)
    {
        return;
    }

    if (limit <= 0)
    {
        megaApi->setMaxDownloadSpeed(0);
    }
    else
    {
        megaApi->setMaxDownloadSpeed(limit * 1024);
    }
}

void MegaApplication::setMaxConnections(int direction, int connections)
{
    if (appfinished)
    {
        return;
    }

    if (connections > 0 && connections <= 6)
    {
        megaApi->setMaxConnections(direction, connections);
    }
}

void MegaApplication::setUseHttpsOnly(bool httpsOnly)
{
    if (appfinished)
    {
        return;
    }

    megaApi->useHttpsOnly(httpsOnly);
}

void MegaApplication::startUpdateTask()
{
    if (appfinished)
    {
        return;
    }

#if defined(WIN32) || defined(__APPLE__)
    if (!updateThread && preferences->canUpdate(MegaApplication::applicationFilePath()))
    {
        updateThread = new QThread();
        updateTask = new UpdateTask(megaApi, MegaApplication::applicationDirPath(), isPublic);
        updateTask->moveToThread(updateThread);

        connect(this, SIGNAL(startUpdaterThread()), updateTask, SLOT(startUpdateThread()), Qt::UniqueConnection);
        connect(this, SIGNAL(tryUpdate()), updateTask, SLOT(checkForUpdates()), Qt::UniqueConnection);
        connect(this, SIGNAL(installUpdate()), updateTask, SLOT(installUpdate()), Qt::UniqueConnection);

        connect(updateTask, SIGNAL(updateCompleted()), this, SLOT(onUpdateCompleted()), Qt::UniqueConnection);
        connect(updateTask, SIGNAL(updateAvailable(bool)), this, SLOT(onUpdateAvailable(bool)), Qt::UniqueConnection);
        connect(updateTask, SIGNAL(installingUpdate(bool)), this, SLOT(onInstallingUpdate(bool)), Qt::UniqueConnection);
        connect(updateTask, SIGNAL(updateNotFound(bool)), this, SLOT(onUpdateNotFound(bool)), Qt::UniqueConnection);
        connect(updateTask, SIGNAL(updateError()), this, SLOT(onUpdateError()), Qt::UniqueConnection);

        connect(updateThread, SIGNAL(finished()), updateTask, SLOT(deleteLater()), Qt::UniqueConnection);
        connect(updateThread, SIGNAL(finished()), updateThread, SLOT(deleteLater()), Qt::UniqueConnection);

        updateThread->start();
        emit startUpdaterThread();
    }
#endif
}

void MegaApplication::stopUpdateTask()
{
    if (updateThread)
    {
        updateThread->quit();
        updateThread = NULL;
        updateTask = NULL;
    }
}

void MegaApplication::applyProxySettings()
{
    if (appfinished)
    {
        return;
    }

    QNetworkProxy proxy(QNetworkProxy::NoProxy);
    MegaProxy *proxySettings = new MegaProxy();
    proxySettings->setProxyType(preferences->proxyType());

    if (preferences->proxyType() == MegaProxy::PROXY_CUSTOM)
    {
        int proxyProtocol = preferences->proxyProtocol();
        QString proxyString = preferences->proxyHostAndPort();
        switch (proxyProtocol)
        {
            case Preferences::PROXY_PROTOCOL_SOCKS5H:
                proxy.setType(QNetworkProxy::Socks5Proxy);
                proxyString.insert(0, QString::fromUtf8("socks5h://"));
                break;
            default:
                proxy.setType(QNetworkProxy::HttpProxy);
                break;
        }

        proxySettings->setProxyURL(proxyString.toUtf8().constData());

        proxy.setHostName(preferences->proxyServer());
        proxy.setPort(preferences->proxyPort());
        if (preferences->proxyRequiresAuth())
        {
            QString username = preferences->getProxyUsername();
            QString password = preferences->getProxyPassword();
            proxySettings->setCredentials(username.toUtf8().constData(), password.toUtf8().constData());

            proxy.setUser(preferences->getProxyUsername());
            proxy.setPassword(preferences->getProxyPassword());
        }
    }
    else if (preferences->proxyType() == MegaProxy::PROXY_AUTO)
    {
        MegaProxy* autoProxy = megaApi->getAutoProxySettings();
        delete proxySettings;
        proxySettings = autoProxy;

        if (proxySettings->getProxyType()==MegaProxy::PROXY_CUSTOM)
        {
            string sProxyURL = proxySettings->getProxyURL();
            QString proxyURL = QString::fromUtf8(sProxyURL.data());

            QStringList arguments = proxyURL.split(QString::fromAscii(":"));
            if (arguments.size() == 2)
            {
                proxy.setType(QNetworkProxy::HttpProxy);
                proxy.setHostName(arguments[0]);
                proxy.setPort(arguments[1].toInt());
            }
        }
    }

    megaApi->setProxySettings(proxySettings);
    megaApiFolders->setProxySettings(proxySettings);
    delete proxySettings;
    QNetworkProxy::setApplicationProxy(proxy);
    megaApi->retryPendingConnections(true, true);
    megaApiFolders->retryPendingConnections(true, true);
}

void MegaApplication::showUpdatedMessage(int lastVersion)
{
    updated = true;
    prevVersion = lastVersion;
}

void MegaApplication::handleMEGAurl(const QUrl &url)
{
    if (appfinished)
    {
        return;
    }

    megaApi->getSessionTransferURL(url.fragment().toUtf8().constData());
}

void MegaApplication::handleLocalPath(const QUrl &url)
{
    if (appfinished)
    {
        return;
    }

    QString path = QDir::toNativeSeparators(url.fragment());
    if (path.endsWith(QDir::separator()))
    {
        path.truncate(path.size() - 1);
        QtConcurrent::run(QDesktopServices::openUrl, QUrl::fromLocalFile(path));
    }
    else
    {
        #ifdef WIN32
        if (path.startsWith(QString::fromAscii("\\\\?\\")))
        {
            path = path.mid(4);
        }
        #endif
        Platform::showInFolder(path);
    }
}

void MegaApplication::clearUserAttributes()
{
    if (infoDialog)
    {
        infoDialog->clearUserAttributes();
    }

    QString pathToAvatar = Utilities::getAvatarPath(preferences->email());
    if (QFileInfo(pathToAvatar).exists())
    {
        QFile::remove(pathToAvatar);
    }
}

void MegaApplication::clearViewedTransfers()
{
    nUnviewedTransfers = 0;
    if (transferManager)
    {
        transferManager->updateNumberOfCompletedTransfers(nUnviewedTransfers);
    }
}

void MegaApplication::onCompletedTransfersTabActive(bool active)
{
    completedTabActive = active;
}

void MegaApplication::checkFirstTransfer()
{
    if (appfinished || !megaApi)
    {
        return;
    }

    if (numTransfers[MegaTransfer::TYPE_DOWNLOAD] && activeTransferPriority[MegaTransfer::TYPE_DOWNLOAD] == 0xFFFFFFFFFFFFFFFFULL)
    {
        MegaTransfer *nextTransfer = megaApi->getFirstTransfer(MegaTransfer::TYPE_DOWNLOAD);
        if (nextTransfer)
        {
            onTransferUpdate(megaApi, nextTransfer);
            delete nextTransfer;
        }
    }

    if (numTransfers[MegaTransfer::TYPE_UPLOAD] && activeTransferPriority[MegaTransfer::TYPE_UPLOAD] == 0xFFFFFFFFFFFFFFFFULL)
    {
        MegaTransfer *nextTransfer = megaApi->getFirstTransfer(MegaTransfer::TYPE_UPLOAD);
        if (nextTransfer)
        {
            onTransferUpdate(megaApi, nextTransfer);
            delete nextTransfer;
        }
    }
}

void MegaApplication::checkOperatingSystem()
{
    if (!preferences->isOneTimeActionDone(Preferences::ONE_TIME_ACTION_OS_TOO_OLD))
    {
        bool isOSdeprecated = false;
#ifdef MEGASYNC_DEPRECATED_OS
        isOSdeprecated = true;
#endif

#ifdef __APPLE__
        char releaseStr[256];
        size_t size = sizeof(releaseStr);
        if (!sysctlbyname("kern.osrelease", releaseStr, &size, NULL, 0)  && size > 0)
        {
            if (strchr(releaseStr,'.'))
            {
                char *token = strtok(releaseStr, ".");
                if (token)
                {
                    errno = 0;
                    char *endPtr = NULL;
                    long majorVersion = strtol(token, &endPtr, 10);
                    if (endPtr != token && errno != ERANGE && majorVersion >= INT_MIN && majorVersion <= INT_MAX)
                    {
                        if((int)majorVersion < 13) // Older versions from 10.9 (mavericks)
                        {
                            isOSdeprecated = true;
                        }
                    }
                }
            }
        }
#endif

#ifdef WIN32
        DWORD dwVersion = GetVersion();
        DWORD dwMajorVersion = (DWORD)(LOBYTE(LOWORD(dwVersion)));
        DWORD dwMinorVersion = (DWORD) (HIBYTE(LOWORD(dwVersion)));
        isOSdeprecated = (dwMajorVersion < 6) || ((dwMajorVersion == 6) && (dwMinorVersion == 0));
#endif

        if (isOSdeprecated)
        {
            QMegaMessageBox::warning(nullptr, tr("MEGAsync"),
                                 tr("Please consider updating your operating system.") + QString::fromUtf8("\n")
#ifdef __APPLE__
                                 + tr("MEGAsync will continue to work, however updates will no longer be supported for versions prior to OS X Mavericks soon.")
#elif defined(_WIN32)
                                 + tr("MEGAsync will continue to work, however, updates will no longer be supported for Windows Vista and older operating systems soon.")
#else
                                 + tr("MEGAsync will continue to work, however you might not receive new updates.")
#endif
                                 );
            preferences->setOneTimeActionDone(Preferences::ONE_TIME_ACTION_OS_TOO_OLD, true);
        }
    }
}

void MegaApplication::notifyItemChange(QString path, int newState)
{
    string localPath;
#ifdef _WIN32
    if (path.startsWith(QString::fromAscii("\\\\?\\")))
    {
        path = path.mid(4);
    }

    localPath.assign((const char *)path.utf16(), path.size() * sizeof(wchar_t));
#else
    localPath = path.toUtf8().constData();
#endif
    Platform::notifyItemChange(&localPath, newState);
}

int MegaApplication::getPrevVersion()
{
    return prevVersion;
}

void MegaApplication::showNotificationFinishedTransfers(unsigned long long appDataId)
{
    QHash<unsigned long long, TransferMetaData*>::iterator it
           = transferAppData.find(appDataId);
    if (it == transferAppData.end())
    {
        return;
    }

    TransferMetaData *data = it.value();
    if (!preferences->showNotifications())
    {
        if (data->pendingTransfers == 0)
        {
            transferAppData.erase(it);
            delete data;
        }

        return;
    }

    if (data->pendingTransfers == 0)
    {
        MegaNotification *notification = new MegaNotification();
        QString title;
        QString message;

        if (data->transfersFileOK || data->transfersFolderOK)
        {
            switch (data->transferDirection)
            {
                case MegaTransfer::TYPE_UPLOAD:
                {
                    if (data->transfersFileOK && data->transfersFolderOK)
                    {
                        title = tr("Upload");
                        if (data->transfersFolderOK == 1)
                        {
                            if (data->transfersFileOK == 1)
                            {
                                message = tr("1 file and 1 folder were successfully uploaded");
                            }
                            else
                            {
                                message = tr("%1 files and 1 folder were successfully uploaded").arg(data->transfersFileOK);
                            }
                        }
                        else
                        {
                            if (data->transfersFileOK == 1)
                            {
                                message = tr("1 file and %1 folders were successfully uploaded").arg(data->transfersFolderOK);
                            }
                            else
                            {
                                message = tr("%1 files and %2 folders were successfully uploaded").arg(data->transfersFileOK).arg(data->transfersFolderOK);
                            }
                        }
                    }
                    else if (!data->transfersFileOK)
                    {
                        title = tr("Folder Upload");
                        if (data->transfersFolderOK == 1)
                        {
                            message = tr("1 folder was successfully uploaded");
                        }
                        else
                        {
                            message = tr("%1 folders were successfully uploaded").arg(data->transfersFolderOK);
                        }
                    }
                    else
                    {
                        title = tr("File Upload");
                        if (data->transfersFileOK == 1)
                        {
                            message = tr("1 file was successfully uploaded");
                        }
                        else
                        {
                            message = tr("%1 files were successfully uploaded").arg(data->transfersFileOK);
                        }
                    }
                    break;
                }
                case MegaTransfer::TYPE_DOWNLOAD:
                {
                    if (data->transfersFileOK && data->transfersFolderOK)
                    {
                        title = tr("Download");
                        if (data->transfersFolderOK == 1)
                        {
                            if (data->transfersFileOK == 1)
                            {
                                message = tr("1 file and 1 folder were successfully downloaded");
                            }
                            else
                            {
                                message = tr("%1 files and 1 folder were successfully downloaded").arg(data->transfersFileOK);
                            }
                        }
                        else
                        {
                            if (data->transfersFileOK == 1)
                            {
                                message = tr("1 file and %1 folders were successfully downloaded").arg(data->transfersFolderOK);
                            }
                            else
                            {
                                message = tr("%1 files and %2 folders were successfully downloaded").arg(data->transfersFileOK).arg(data->transfersFolderOK);
                            }
                        }
                    }
                    else if (!data->transfersFileOK)
                    {
                        title = tr("Folder Download");
                        if (data->transfersFolderOK == 1)
                        {
                            message = tr("1 folder was successfully downloaded");
                        }
                        else
                        {
                            message = tr("%1 folders were successfully downloaded").arg(data->transfersFolderOK);
                        }
                    }
                    else
                    {
                        title = tr("File Download");
                        if (data->transfersFileOK == 1)
                        {
                            message = tr("1 file was successfully downloaded");
                        }
                        else
                        {
                            message = tr("%1 files were successfully downloaded").arg(data->transfersFileOK);
                        }
                    }
                    break;
                }
                default:
                    break;
            }
        }

        if (notificator && !message.isEmpty())
        {           
            preferences->setLastTransferNotificationTimestamp();
            notification->setTitle(title);
            notification->setText(message);
            notification->setActions(QStringList() << tr("Show in folder"));
            notification->setData(((data->totalTransfers == 1) ? QString::number(1) : QString::number(0)) + data->localPath);
            connect(notification, SIGNAL(activated(int)), this, SLOT(showInFolder(int)));
            notificator->notify(notification);
        }

        transferAppData.erase(it);
        delete data;
    }
}

#ifdef __APPLE__
void MegaApplication::enableFinderExt()
{
    // We need to wait from OS X El capitan to reload system db before enable the extension
    Platform::enableFinderExtension(true);
    preferences->setOneTimeActionDone(Preferences::ONE_TIME_ACTION_ACTIVE_FINDER_EXT, true);
}
#endif

void MegaApplication::showInFolder(int activationButton)
{
    MegaNotification *notification = ((MegaNotification *)QObject::sender());

    if ((activationButton == MegaNotification::ActivationActionButtonClicked
         || activationButton == MegaNotification::ActivationLegacyNotificationClicked
     #ifndef _WIN32
         || activationButton == MegaNotification::ActivationContentClicked
     #endif
         )
            && notification->getData().size() > 1)
    {
        QString localPath = QDir::toNativeSeparators(notification->getData().mid(1));
        if (notification->getData().at(0) == QChar::fromAscii('1'))
        {
            Platform::showInFolder(localPath);
        }
        else
        {
            QtConcurrent::run(QDesktopServices::openUrl, QUrl::fromLocalFile(localPath));
        }
    }
}

void MegaApplication::openFolderPath(QString localPath)
{
    if (!localPath.isEmpty())
    {
        #ifdef WIN32
        if (localPath.startsWith(QString::fromAscii("\\\\?\\")))
        {
            localPath = localPath.mid(4);
        }
        #endif
        Platform::showInFolder(localPath);
    }
}

void MegaApplication::redirectToUpgrade(int activationButton)
{
    if (activationButton == MegaNotification::ActivationActionButtonClicked
            || activationButton == MegaNotification::ActivationLegacyNotificationClicked
        #ifndef _WIN32
            || activationButton == MegaNotification::ActivationContentClicked
        #endif
            )
    {
        QString url = QString::fromUtf8("mega://#pro");
        Utilities::getPROurlWithParameters(url);
        QtConcurrent::run(QDesktopServices::openUrl, QUrl(url));
    }
}

void MegaApplication::updateStatesAfterTransferOverQuotaTimeHasExpired()
{
    transferOverQuotaWaitTimeExpiredReceived = true;
}

void MegaApplication::redirectToPayBusiness(int activationButton)
{
    if (activationButton == MegaNotification::ActivationActionButtonClicked
            || activationButton == MegaNotification::ActivationLegacyNotificationClicked
        #ifndef _WIN32
            || activationButton == MegaNotification::ActivationContentClicked
        #endif
            )
    {      
        QString url = QString::fromUtf8("mega://#repay");
        Utilities::getPROurlWithParameters(url);
        QtConcurrent::run(QDesktopServices::openUrl, QUrl(url));
    }
}

void MegaApplication::registerUserActivity()
{
    lastUserActivityExecution = QDateTime::currentMSecsSinceEpoch();
}

void MegaApplication::PSAseen(int id)
{
    if (id >= 0)
    {
        megaApi->setPSA(id);
    }
}

void MegaApplication::onSyncStateChanged(std::shared_ptr<SyncSetting> syncSettings)
{
    createAppMenus();
}

void MegaApplication::onSyncDeleted(std::shared_ptr<SyncSetting> syncSettings)
{
    createAppMenus();
}

void MegaApplication::migrateSyncConfToSdk(QString email)
{
    bool needsMigratingFromOldSession = !preferences->logged();
    assert(preferences->logged() || !email.isEmpty());


    int cachedBusinessState = 999;
    int cachedBlockedState = 999;
    int cachedStorageState = 999;

    auto oldCachedSyncs = preferences->readOldCachedSyncs(&cachedBusinessState, &cachedBlockedState, &cachedStorageState, email);
    int oldCacheSyncsCount = oldCachedSyncs.size();
    if (oldCacheSyncsCount > 0)
    {
        if (cachedBusinessState == -2)
        {
            cachedBusinessState = 999;
        }
        if (cachedBlockedState == -2)
        {
            cachedBlockedState = 999;
        }
        if (cachedStorageState == MegaApi::STORAGE_STATE_UNKNOWN)
        {
            cachedStorageState = 999;
        }

        megaApi->copyCachedStatus(cachedStorageState, cachedBlockedState, cachedBusinessState);
    }

    foreach(SyncData osd, oldCachedSyncs)
    {
        MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, QString::fromUtf8("Copying sync data to SDK cache: %1. Name: %2")
                     .arg(osd.mLocalFolder).arg(osd.mName).toUtf8().constData());

        megaApi->copySyncDataToCache(osd.mLocalFolder.toUtf8().constData(), osd.mName.toUtf8().constData(),
                                     osd.mMegaHandle, osd.mMegaFolder.toUtf8().constData(),
                                     osd.mLocalfp, osd.mEnabled, osd.mTemporarilyDisabled,
                                     new MegaListenerFuncExecuter(true, [this, osd, oldCacheSyncsCount, needsMigratingFromOldSession, email](MegaApi* api,  MegaRequest *request, MegaError *e)
        {
            if (e->getErrorCode() == MegaError::API_OK)
            {
                //preload the model with the restored configuration: that includes info that the SDK does not handle (e.g: syncID)
                model->pickInfoFromOldSync(osd, request->getTransferTag(), needsMigratingFromOldSession);
                preferences->removeOldCachedSync(osd.mPos, email);
            }
            else
            {
                MegaApi::log(MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("Failed to copy sync %1: %2").arg(osd.mLocalFolder).arg(QString::fromUtf8(e->getErrorString())).toUtf8().constData());
            }

         }));
    }
}

void MegaApplication::onBlocked()
{
    updateTrayIconMenu();
}

void MegaApplication::onUnblocked()
{
    updateTrayIconMenu();
}

void MegaApplication::fetchNodes(QString email)
{
    assert(!mFetchingNodes);
    mFetchingNodes = true;
    migrateSyncConfToSdk(email);
    megaApi->fetchNodes();
}

void MegaApplication::whyAmIBlocked(bool periodicCall)
{
    if (!mQueringWhyAmIBlocked)
    {
        whyamiblockedPeriodicPetition = periodicCall;
        mQueringWhyAmIBlocked = true;
        megaApi->whyAmIBlocked();
    }
}

std::shared_ptr<MegaNode> MegaApplication::getRootNode(bool forceReset)
{
    if (megaApi && (forceReset || !mRootNode) )
    {
        mRootNode.reset(megaApi->getRootNode());
    }
    return mRootNode;
}

void MegaApplication::onDismissStorageOverquota(bool overStorage)
{
    if (overStorage)
    {
        preferences->setOverStorageDismissExecution(QDateTime::currentMSecsSinceEpoch());
    }
    else
    {
        preferences->setAlmostOverStorageDismissExecution(QDateTime::currentMSecsSinceEpoch());
    }
}

void MegaApplication::updateUserStats(bool storage, bool transfer, bool pro, bool force, int source)
{
    if (appfinished)
    {
        return;
    }

    // if any are already pending, we don't need to fetch again
    if (inflightUserStats[0]) storage = false;
    if (inflightUserStats[1]) transfer = false;
    if (inflightUserStats[2]) pro = false;

    if (!storage && !transfer && !pro)
    {
        MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, "Skipped call to getSpecificAccountDetails()");
        return;
    }

    // if the oldest of the ones we want is too recent, skip (unless force)
    long long lastRequest = 0;
    if (storage  && (!lastRequest || lastRequest > userStatsLastRequest[0])) lastRequest = userStatsLastRequest[0];
    if (transfer && (!lastRequest || lastRequest > userStatsLastRequest[1])) lastRequest = userStatsLastRequest[1];
    if (pro      && (!lastRequest || lastRequest > userStatsLastRequest[2])) lastRequest = userStatsLastRequest[2];

    if (storage && source >= 0) queuedStorageUserStatsReason |= (1 << source);

    if (force || !lastRequest || (QDateTime::currentMSecsSinceEpoch() - lastRequest) > Preferences::MIN_UPDATE_STATS_INTERVAL)
    {
        megaApi->getSpecificAccountDetails(storage, transfer, pro, storage ? queuedStorageUserStatsReason : -1);
        if (storage) queuedStorageUserStatsReason = 0;

        if (storage)  inflightUserStats[0] = true;
        if (transfer) inflightUserStats[1] = true;
        if (pro)      inflightUserStats[2] = true;

        if (storage)  userStatsLastRequest[0] = QDateTime::currentMSecsSinceEpoch();
        if (transfer) userStatsLastRequest[1] = QDateTime::currentMSecsSinceEpoch();
        if (pro)      userStatsLastRequest[2] = QDateTime::currentMSecsSinceEpoch();
    }
    else
    {
        if (storage)  queuedUserStats[0] = true;
        if (transfer) queuedUserStats[1] = true;
        if (pro)      queuedUserStats[2] = true;
    }
}

void MegaApplication::addRecentFile(QString fileName, long long fileHandle, QString localPath, QString nodeKey)
{
    if (appfinished)
    {
        return;
    }
}

void MegaApplication::checkForUpdates()
{
    if (appfinished)
    {
        return;
    }

    this->showInfoMessage(tr("Checking for updates..."));
    emit tryUpdate();
}

void MegaApplication::showTrayMenu(QPoint *point)
{
    if (appfinished)
    {
        return;
    }
#ifdef _WIN32
    // recreate menus to fix some qt scaling issues in windows
    createAppMenus();
    createGuestMenu();
#endif
    QMenu *displayedMenu = nullptr;
    int menuWidthInitialPopup = -1;
    if (!preferences->logged() || blockState) // if not logged or blocked account
    {
        if (guestMenu)
        {
            if (guestMenu->isVisible())
            {
                guestMenu->close();
            }

            menuWidthInitialPopup = guestMenu->sizeHint().width();
            QPoint p = point ? (*point) - QPoint(guestMenu->sizeHint().width(), 0)
                             : QCursor::pos();

            guestMenu->update();
            guestMenu->popup(p);
            displayedMenu = guestMenu.get();
        }
    }
    else // logged in
    {
        if (infoDialogMenu)
        {
            if (infoDialogMenu->isVisible())
            {
                infoDialogMenu->close();
            }

            menuWidthInitialPopup = infoDialogMenu->sizeHint().width();
            QPoint p = point ? (*point) - QPoint(infoDialogMenu->sizeHint().width(), 0)
                                     : QCursor::pos();
            infoDialogMenu->update();
            infoDialogMenu->popup(p);
            displayedMenu = infoDialogMenu.get();
        }
    }

    // Menu width might be incorrect the first time it's shown. This works around that and repositions the menu at the expected position afterwards
    if (point)
    {
        QPoint pointValue= *point;
        QTimer::singleShot(1, displayedMenu, [displayedMenu, pointValue, menuWidthInitialPopup] () {
            displayedMenu->update();
            displayedMenu->ensurePolished();
            if (menuWidthInitialPopup != displayedMenu->sizeHint().width())
            {
                QPoint p = pointValue  - QPoint(displayedMenu->sizeHint().width(), 0);
                displayedMenu->update();
                displayedMenu->popup(p);
            }
        });
    }
}

void MegaApplication::toggleLogging()
{
    if (appfinished)
    {
        return;
    }

    if (logger->isDebug())
    {
        Preferences::HTTPS_ORIGIN_CHECK_ENABLED = true;
        logger->setDebug(false);
        showInfoMessage(tr("DEBUG mode disabled"));
    }
    else
    {
        Preferences::HTTPS_ORIGIN_CHECK_ENABLED = false;
        logger->setDebug(true);
        showInfoMessage(tr("DEBUG mode enabled. A log is being created in your desktop (MEGAsync.log)"));
        if (megaApi)
        {
            MegaApi::log(MegaApi::LOG_LEVEL_INFO, QString::fromUtf8("Version string: %1   Version code: %2.%3   User-Agent: %4").arg(Preferences::VERSION_STRING)
                     .arg(Preferences::VERSION_CODE).arg(Preferences::BUILD_ID).arg(QString::fromUtf8(megaApi->getUserAgent())).toUtf8().constData());
        }
    }
}

void MegaApplication::removeFinishedTransfer(int transferTag)
{
    QMap<int, MegaTransfer*>::iterator it = finishedTransfers.find(transferTag);
    if (it != finishedTransfers.end())
    {     
        for (QList<MegaTransfer*>::iterator it2 = finishedTransferOrder.begin(); it2 != finishedTransferOrder.end(); it2++)
        {
            if ((*it2)->getTag() == transferTag)
            {
                finishedTransferOrder.erase(it2);
                break;
            }
        }
        delete it.value();
        finishedTransfers.erase(it);

        emit clearFinishedTransfer(transferTag);

        if (!finishedTransfers.size() && infoDialog)
        {
            infoDialog->updateDialogState();
        }
    }
}

void MegaApplication::removeFinishedBlockedTransfer(int transferTag)
{
    finishedBlockedTransfers.remove(transferTag);
}

bool MegaApplication::finishedTransfersWhileBlocked(int transferTag)
{
    return finishedBlockedTransfers.contains(transferTag);
}

void MegaApplication::removeAllFinishedTransfers()
{
    qDeleteAll(finishedTransfers);
    finishedTransferOrder.clear();
    finishedTransfers.clear();

    emit clearAllFinishedTransfers();

    if (infoDialog)
    {
        infoDialog->updateDialogState();
    }
}

void MegaApplication::showVerifyAccountInfo()
{
    if (!verifyEmail)
    {
        verifyEmail.reset(new VerifyLockMessage(blockState, infoDialog ? true : false));
        connect(verifyEmail.get(), SIGNAL(logout()), this, SLOT(unlink()));
    }
    else
    {
        verifyEmail->regenerateUI(blockState);
    }

    verifyEmail->show();
    verifyEmail->activateWindow();
    verifyEmail->raise();
}

QList<MegaTransfer*> MegaApplication::getFinishedTransfers()
{
    return finishedTransferOrder;
}

int MegaApplication::getNumUnviewedTransfers()
{
    return nUnviewedTransfers;
}

MegaTransfer* MegaApplication::getFinishedTransferByTag(int tag)
{
    if (!finishedTransfers.contains(tag))
    {
        return NULL;
    }
    return finishedTransfers.value(tag);
}

void MegaApplication::pauseTransfers()
{
    pauseTransfers(!preferences->getGlobalPaused());
}

void MegaApplication::officialWeb()
{
    QString webUrl = Preferences::BASE_URL;
    QtConcurrent::run(QDesktopServices::openUrl, QUrl(webUrl));
}

void MegaApplication::goToMyCloud()
{
    QString url = QString::fromUtf8("");
    megaApi->getSessionTransferURL(url.toUtf8().constData());
}

//Called when the "Import links" menu item is clicked
void MegaApplication::importLinks()
{
    if (appfinished)
    {
        return;
    }

    if(!transferQuota->checkImportLinksAlertDismissed())
    {
        return;
    }

    if (!preferences->logged())
    {
        openInfoWizard();
        return;
    }

    if (pasteMegaLinksDialog)
    {
        pasteMegaLinksDialog->activateWindow();
        pasteMegaLinksDialog->raise();
        return;
    }

    if (importDialog)
    {
        importDialog->activateWindow();
        importDialog->raise();
        return;
    }

    //Show the dialog to paste public links
    pasteMegaLinksDialog = new PasteMegaLinksDialog();
    pasteMegaLinksDialog->exec();
    if (!pasteMegaLinksDialog)
    {
        return;
    }

    //If the dialog isn't accepted, return
    if (pasteMegaLinksDialog->result()!=QDialog::Accepted)
    {
        delete pasteMegaLinksDialog;
        pasteMegaLinksDialog = NULL;
        return;
    }

    //Get the list of links from the dialog
    QStringList linkList = pasteMegaLinksDialog->getLinks();
    delete pasteMegaLinksDialog;
    pasteMegaLinksDialog = NULL;

    //Send links to the link processor
    LinkProcessor *linkProcessor = new LinkProcessor(linkList, megaApi, megaApiFolders);

    //Open the import dialog
    importDialog = new ImportMegaLinksDialog(megaApi, preferences, linkProcessor);
    importDialog->exec();
    if (!importDialog)
    {
        return;
    }

    if (importDialog->result() != QDialog::Accepted)
    {
        delete importDialog;
        importDialog = NULL;
        return;
    }

    //If the user wants to download some links, do it
    if (importDialog->shouldDownload())
    {
        if (!preferences->hasDefaultDownloadFolder())
        {
            preferences->setDownloadFolder(importDialog->getDownloadPath());
        }
        linkProcessor->downloadLinks(importDialog->getDownloadPath());
    }

    //If the user wants to import some links, do it
    if (preferences->logged() && importDialog->shouldImport())
    {
        preferences->setOverStorageDismissExecution(0);

        connect(linkProcessor, SIGNAL(onLinkImportFinish()), this, SLOT(onLinkImportFinished()));
        connect(linkProcessor, SIGNAL(onDupplicateLink(QString, QString, mega::MegaHandle)),
                this, SLOT(onDupplicateLink(QString, QString, mega::MegaHandle)));
        linkProcessor->importLinks(importDialog->getImportPath());
    }
    else
    {
        //If importing links isn't needed, we can delete the link processor
        //It doesn't track transfers, only the importation of links
        delete linkProcessor;
    }

    delete importDialog;
    importDialog = NULL;
}

void MegaApplication::showChangeLog()
{
    if (appfinished)
    {
        return;
    }

    if (changeLogDialog)
    {
        changeLogDialog->show();
        return;
    }

    changeLogDialog = new ChangeLogDialog(Preferences::VERSION_STRING, Preferences::SDK_ID, Preferences::CHANGELOG);
    changeLogDialog->show();
}

void MegaApplication::uploadActionClicked()
{
    if (appfinished)
    {
        return;
    }

    const auto disabledUntil{preferences->getStorageOverQuotaUploadsDialogLastExecution() + Preferences::OVER_QUOTA_ACTION_DIALOGS_DISABLE_TIME};
    const auto dialogEnabled{std::chrono::system_clock::now() >= disabledUntil};
    const auto storageIsOverQuota{storageState == MegaApi::STORAGE_STATE_RED || storageState == MegaApi::STORAGE_STATE_PAYWALL};
    if(storageIsOverQuota && dialogEnabled)
    {
        preferences->setStorageOverQuotaUploadsDialogLastExecution(std::chrono::system_clock::now());
        const auto storageFullDialog{OverQuotaDialog::createDialog(OverQuotaDialogType::STORAGE_UPLOAD)};
        const auto upgradeButtonClicked{storageFullDialog->exec() == QDialog::Accepted};
        if(upgradeButtonClicked)
        {
            return;
        }
    }

    #ifdef __APPLE__
         if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_7)
         {
                infoDialog->hide();
                QApplication::processEvents();
                if (appfinished)
                {
                    return;
                }

                QStringList files = MacXPlatform::multipleUpload(QCoreApplication::translate("ShellExtension", "Upload to MEGA"));
                if (files.size())
                {
                    QQueue<QString> qFiles;
                    foreach(QString file, files)
                    {
                        qFiles.append(file);
                    }

                    shellUpload(qFiles);
                }
         return;
         }
    #endif

    if (multiUploadFileDialog)
    {
        multiUploadFileDialog->activateWindow();
        multiUploadFileDialog->raise();
        return;
    }

#if QT_VERSION < 0x050000
    QString defaultFolderPath = QDesktopServices::storageLocation(QDesktopServices::HomeLocation);
#else
    QString  defaultFolderPath;
    QStringList paths = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
    if (paths.size())
    {
        defaultFolderPath = paths.at(0);
    }
#endif

    multiUploadFileDialog = new MultiQFileDialog(NULL,
           QCoreApplication::translate("ShellExtension", "Upload to MEGA"),
           defaultFolderPath);

    int result = multiUploadFileDialog->exec();
    if (!multiUploadFileDialog)
    {
        return;
    }

    if (result == QDialog::Accepted)
    {
        QStringList files = multiUploadFileDialog->selectedFiles();
        if (files.size())
        {
            QQueue<QString> qFiles;
            foreach(QString file, files)
                qFiles.append(file);
            shellUpload(qFiles);
        }
    }

    delete multiUploadFileDialog;
    multiUploadFileDialog = NULL;
}

bool MegaApplication::showSyncOverquotaDialog()
{
    const auto storageFull{storageState == MegaApi::STORAGE_STATE_RED};
    const auto disabledUntil{preferences->getStorageOverQuotaSyncsDialogLastExecution() + Preferences::OVER_QUOTA_ACTION_DIALOGS_DISABLE_TIME};
    const auto dialogStorageEnabled{std::chrono::system_clock::now() >= disabledUntil};
    if(dialogStorageEnabled && storageFull)
    {
        preferences->setStorageOverQuotaSyncsDialogLastExecution(std::chrono::system_clock::now());
        const auto dialog{OverQuotaDialog::createDialog(OverQuotaDialogType::STORAGE_SYNCS)};
        const auto upgradeButtonClicked{dialog->exec() == QDialog::Accepted};
        if(upgradeButtonClicked)
        {
            return false;
        }
    }

    const auto transferDialogDisabledUntil{preferences->getTransferOverQuotaSyncDialogLastExecution() + Preferences::OVER_QUOTA_ACTION_DIALOGS_DISABLE_TIME};
    const auto dialogTransferEnabled{std::chrono::system_clock::now() >= transferDialogDisabledUntil};
    if(dialogTransferEnabled && transferQuota->isOverQuota() && !storageFull)
    {
        preferences->setTransferOverQuotaSyncDialogLastExecution(std::chrono::system_clock::now());
        const auto dialog{OverQuotaDialog::createDialog(OverQuotaDialogType::BANDWITH_SYNC)};
        const auto upgradeButtonClicked{dialog->exec() == QDialog::Accepted};
        if(upgradeButtonClicked)
        {
            return false;
        }
    }
    return true;
}

bool MegaApplication::finished() const
{
    return appfinished;
}

bool MegaApplication::isInfoDialogVisible() const
{
    return infoDialog && infoDialog->isVisible();
}

void MegaApplication::downloadActionClicked()
{
    if (appfinished)
    {
        return;
    }

    if(!transferQuota->checkDownloadAlertDismissed())
    {
        return;
    }

    if (downloadNodeSelector)
    {
        downloadNodeSelector->activateWindow();
        downloadNodeSelector->raise();
        return;
    }

    downloadNodeSelector = new NodeSelector(megaApi, NodeSelector::DOWNLOAD_SELECT, NULL);
    int result = downloadNodeSelector->exec();
    if (!downloadNodeSelector)
    {
        return;
    }

    if (result != QDialog::Accepted)
    {
        delete downloadNodeSelector;
        downloadNodeSelector = NULL;
        return;
    }

    long long selectedMegaFolderHandle = downloadNodeSelector->getSelectedFolderHandle();
    MegaNode *selectedNode = megaApi->getNodeByHandle(selectedMegaFolderHandle);
    delete downloadNodeSelector;
    downloadNodeSelector = NULL;
    if (!selectedNode)
    {
        selectedMegaFolderHandle = INVALID_HANDLE;
        return;
    }

    if (selectedNode)
    {
        downloadQueue.append(selectedNode);
        processDownloads();
    }
}

void MegaApplication::streamActionClicked()
{
    if (appfinished)
    {
        return;
    }

    if(!transferQuota->checkStreamingAlertDismissed())
    {
        return;
    }

    if (streamSelector)
    {
        streamSelector->showNormal();
        streamSelector->activateWindow();
        streamSelector->raise();
        return;
    }

    streamSelector = new StreamingFromMegaDialog(megaApi);
    connect(transferQuota.get(), &TransferQuota::waitTimeIsOver, streamSelector, &StreamingFromMegaDialog::updateStreamingState);
    streamSelector->show();
}

void MegaApplication::transferManagerActionClicked(int tab)
{
    if (appfinished)
    {
        return;
    }

    if (transferManager)
    {
        transferManager->setActiveTab(tab);
        transferManager->showNormal();
        transferManager->activateWindow();
        transferManager->raise();
        transferManager->updateState();
        return;
    }

    transferManager = new TransferManager(megaApi);
    // Signal/slot to notify the tracking of unseen completed transfers of Transfer Manager. If Completed tab is
    // active, tracking is disabled
    connect(transferManager, SIGNAL(viewedCompletedTransfers()), this, SLOT(clearViewedTransfers()));
    connect(transferManager, SIGNAL(completedTransfersTabActive(bool)), this, SLOT(onCompletedTransfersTabActive(bool)));
    connect(transferManager, SIGNAL(userActivity()), this, SLOT(registerUserActivity()));
    transferManager->setActiveTab(tab);

    Platform::activateBackgroundWindow(transferManager);
    transferManager->show();
}

void MegaApplication::loginActionClicked()
{
    if (appfinished)
    {
        return;
    }

    userAction(SetupWizard::PAGE_LOGIN);
}

void MegaApplication::showSetupWizard(int action)
{
    if (setupWizard)
    {
        setupWizard->goToStep(action);
        setupWizard->activateWindow();
        setupWizard->raise();
        return;
    }
    setupWizard = new SetupWizard(this);
    emit setupWizardCreated();
    setupWizard->setModal(false);
    connect(setupWizard, SIGNAL(finished(int)), this, SLOT(setupWizardFinished(int)));
    setupWizard->goToStep(action);
    setupWizard->show();
}

void MegaApplication::userAction(int action)
{
    if (appfinished)
    {
        return;
    }

    if (!preferences->logged())
    {
        switch (action)
        {
            case InfoWizard::LOGIN_CLICKED:
                showInfoDialog();
                break;
            default:
                showSetupWizard(action);
                break;
        }
    }
}

void MegaApplication::applyNotificationFilter(int opt)
{
    if (notificationsProxyModel)
    {
        notificationsProxyModel->setFilterAlertType(opt);
    }
}

void MegaApplication::changeState()
{
    if (appfinished)
    {
        return;
    }

    if (infoDialog)
    {
        infoDialog->regenerateLayout();
    }
    updateTrayIconMenu();
}

#ifdef _WIN32
void MegaApplication::changeDisplay(QScreen *disp)
{
    MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, QString::fromAscii("DISPLAY CHANGED").toUtf8().constData());

    if (infoDialog)
    {
        infoDialog->setWindowFlags(Qt::FramelessWindowHint);
        infoDialog->setWindowFlags(Qt::FramelessWindowHint | Qt::Popup);
    }
    if (transferManager && transferManager->isVisible())
    {
        //hack to force qt to reconsider zoom/sizes/etc ...
        //this closes the window
        transferManager->setWindowFlags(Qt::Window);
        transferManager->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    }
}
#endif

void MegaApplication::updateTrayIconMenu()
{
    if (trayIcon)
    {
#if defined(Q_OS_MACX)
        if (infoDialog)
        {
            trayIcon->setContextMenu(&emptyMenu);
        }
        else
        {
            trayIcon->setContextMenu(initialMenu?initialMenu.get():&emptyMenu);
        }
#else

        trayIcon->setContextMenu(nullptr); //prevents duplicated context menu in qt 5.12.8 64 bits

        if (preferences && preferences->logged() && getRootNode() && !blockState)
        { //regular situation: fully logged and without any blocking status
#ifdef _WIN32
            trayIcon->setContextMenu(windowsMenu?windowsMenu.get():&emptyMenu);
#else
            trayIcon->setContextMenu(initialMenu?initialMenu.get():&emptyMenu);
#endif
        }
        else
        {
            trayIcon->setContextMenu(initialMenu?initialMenu.get():&emptyMenu);
        }
#endif
    }
}

void MegaApplication::createTrayIcon()
{
    if (appfinished)
    {
        return;
    }

    createAppMenus();
    createGuestMenu();

    if (!trayIcon)
    {
        trayIcon = new QSystemTrayIcon();

        connect(trayIcon, SIGNAL(messageClicked()), this, SLOT(onMessageClicked()));
        connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
                this, SLOT(trayIconActivated(QSystemTrayIcon::ActivationReason)));

    #ifdef __APPLE__
        scanningTimer = new QTimer();
        scanningTimer->setSingleShot(false);
        scanningTimer->setInterval(500);
        scanningAnimationIndex = 1;
        connect(scanningTimer, SIGNAL(timeout()), this, SLOT(scanningAnimationStep()));
    #endif
    }

    updateTrayIconMenu();

    if (isLinux)
    {
        return;
    }


    trayIcon->setToolTip(QCoreApplication::applicationName()
                     + QString::fromAscii(" ")
                     + Preferences::VERSION_STRING
                     + QString::fromAscii("\n")
                     + tr("Starting"));

#ifndef __APPLE__
    #ifdef _WIN32
        trayIcon->setIcon(QIcon(QString::fromAscii("://images/tray_sync.ico")));
    #else
        setTrayIconFromTheme(QString::fromAscii("://images/synching.svg"));
    #endif
#else
    QIcon ic = QIcon(QString::fromAscii("://images/icon_syncing_mac.png"));
    ic.setIsMask(true);
    trayIcon->setIcon(ic);

    if (!scanningTimer->isActive())
    {
        scanningAnimationIndex = 1;
        scanningTimer->start();
    }
#endif
}

void MegaApplication::processUploads()
{
    if (appfinished)
    {
        return;
    }

    if (!uploadQueue.size())
    {
        return;
    }

    if (!preferences->logged())
    {
        openInfoWizard();
        return;
    }

    //If the dialog to select the upload folder is active, return.
    //Files will be uploaded when the user selects the upload folder
    if (uploadFolderSelector)
    {
        uploadFolderSelector->activateWindow();
        uploadFolderSelector->raise();
        return;
    }

    //If there is a default upload folder in the preferences
    MegaNode *node = megaApi->getNodeByHandle(preferences->uploadFolder());
    if (node)
    {
        const char *path = megaApi->getNodePath(node);
        if (path && !strncmp(path, "//bin", 5))
        {
            preferences->setHasDefaultUploadFolder(false);
            preferences->setUploadFolder(INVALID_HANDLE);
        }

        if (preferences->hasDefaultUploadFolder())
        {
            //use it to upload the list of files
            processUploadQueue(node->getHandle());
            delete node;
            delete [] path;
            return;
        }

        delete node;
        delete [] path;
    }
    uploadFolderSelector = new UploadToMegaDialog(megaApi);
    uploadFolderSelector->setDefaultFolder(preferences->uploadFolder());
    Platform::activateBackgroundWindow(uploadFolderSelector);
    uploadFolderSelector->exec();
    if (!uploadFolderSelector)
    {
        return;
    }

    if (uploadFolderSelector->result()==QDialog::Accepted)
    {
        //If the dialog is accepted, get the destination node
        MegaHandle nodeHandle = uploadFolderSelector->getSelectedHandle();
        preferences->setHasDefaultUploadFolder(uploadFolderSelector->isDefaultFolder());
        preferences->setUploadFolder(nodeHandle);
        if (settingsDialog)
        {
            settingsDialog->updateUploadFolder(); //this could be done via observer
        }
        processUploadQueue(nodeHandle);
    }
    //If the dialog is rejected, cancel uploads
    else uploadQueue.clear();

    delete uploadFolderSelector;
    uploadFolderSelector = NULL;
    return;

}

void MegaApplication::processDownloads()
{
    if (appfinished)
    {
        return;
    }

    if (!downloadQueue.size())
    {
        return;
    }

    if (!preferences->logged())
    {
        openInfoWizard();
        return;
    }

    if (downloadFolderSelector)
    {
        downloadFolderSelector->activateWindow();
        downloadFolderSelector->raise();
        return;
    }

    QString defaultPath = preferences->downloadFolder();
    if (preferences->hasDefaultDownloadFolder()
            && QFile(defaultPath).exists())
    {
        QTemporaryFile *test = new QTemporaryFile(defaultPath + QDir::separator());
        if (test->open())
        {
            delete test;

            HTTPServer *webCom = qobject_cast<HTTPServer *>(sender());
            if (webCom)
            {
                showInfoDialog();
            }

            processDownloadQueue(defaultPath);
            return;
        }
        delete test;

        preferences->setHasDefaultDownloadFolder(false);
        preferences->setDownloadFolder(QString());
    }

    downloadFolderSelector = new DownloadFromMegaDialog(preferences->downloadFolder());
    Platform::activateBackgroundWindow(downloadFolderSelector);
    downloadFolderSelector->exec();
    if (!downloadFolderSelector)
    {
        return;
    }

    if (downloadFolderSelector->result()==QDialog::Accepted)
    {
        //If the dialog is accepted, get the destination node
        QString path = downloadFolderSelector->getPath();
        preferences->setHasDefaultDownloadFolder(downloadFolderSelector->isDefaultDownloadOption());
        preferences->setDownloadFolder(path);
        if (settingsDialog)
        {
            settingsDialog->updateDownloadFolder(); // this could use observer pattern
        }

        HTTPServer *webCom = qobject_cast<HTTPServer *>(sender());
        if (webCom)
        {
            showInfoDialog();
        }

        processDownloadQueue(path);
    }
    else
    {
        QQueue<MegaNode *>::iterator it;
        for (it = downloadQueue.begin(); it != downloadQueue.end(); ++it)
        {
            HTTPServer::onTransferDataUpdate((*it)->getHandle(), MegaTransfer::STATE_CANCELLED, 0, 0, 0, QString());
        }

        //If the dialog is rejected, cancel uploads
        qDeleteAll(downloadQueue);
        downloadQueue.clear();
    }

    delete downloadFolderSelector;
    downloadFolderSelector = NULL;
    return;
}

void MegaApplication::logoutActionClicked()
{
    if (appfinished)
    {
        return;
    }

    unlink();
}

//Called when the user wants to generate the public link for a node
void MegaApplication::copyFileLink(MegaHandle fileHandle, QString nodeKey)
{
    if (appfinished)
    {
        return;
    }

    if (nodeKey.size())
    {
        //Public node
        const char* base64Handle = MegaApi::handleToBase64(fileHandle);
        QString handle = QString::fromUtf8(base64Handle);
        QString linkForClipboard = Preferences::BASE_URL + QString::fromUtf8("/#!%1!%2").arg(handle).arg(nodeKey);
        delete [] base64Handle;
        QApplication::clipboard()->setText(linkForClipboard);
        showInfoMessage(tr("The link has been copied to the clipboard"));
        return;
    }

    MegaNode *node = megaApi->getNodeByHandle(fileHandle);
    if (!node)
    {
        showErrorMessage(tr("Error getting link:") + QString::fromUtf8(" ") + tr("File not found"));
        return;
    }

    char *path = megaApi->getNodePath(node);
    if (path && strncmp(path, "//bin/", 6) && megaApi->checkAccess(node, MegaShare::ACCESS_OWNER).getErrorCode() == MegaError::API_OK)
    {
        //Launch the creation of the import link, it will be handled in the "onRequestFinish" callback
        megaApi->exportNode(node);

        delete node;
        delete [] path;
        return;
    }
    delete [] path;

    const char *fp = megaApi->getFingerprint(node);
    if (!fp)
    {
        showErrorMessage(tr("Error getting link:") + QString::fromUtf8(" ") + tr("File not found"));
        delete node;
        return;
    }
    MegaNode *exportableNode = megaApi->getExportableNodeByFingerprint(fp, node->getName());
    if (exportableNode)
    {
        //Launch the creation of the import link, it will be handled in the "onRequestFinish" callback
        megaApi->exportNode(exportableNode);

        delete node;
        delete [] fp;
        delete exportableNode;
        return;
    }

    delete node;
    delete [] fp;
    showErrorMessage(tr("The link can't be generated because the file is in an incoming shared folder or in your Rubbish Bin"));
}

//Called when the user wants to upload a list of files and/or folders from the shell
void MegaApplication::shellUpload(QQueue<QString> newUploadQueue)
{
    if (appfinished)
    {
        return;
    }

    //Append the list of files to the upload queue
    uploadQueue.append(newUploadQueue);
    processUploads();
}

void MegaApplication::shellExport(QQueue<QString> newExportQueue)
{
    if (appfinished || !megaApi->isLoggedIn())
    {
        return;
    }

    ExportProcessor *processor = new ExportProcessor(megaApi, newExportQueue);
    connect(processor, SIGNAL(onRequestLinksFinished()), this, SLOT(onRequestLinksFinished()));
    processor->requestLinks();
    exportOps++;
}

void MegaApplication::shellViewOnMega(QByteArray localPath, bool versions)
{
    MegaNode *node = NULL;

#ifdef WIN32   
    if (!localPath.startsWith(QByteArray((const char *)L"\\\\", 4)))
    {
        localPath.insert(0, QByteArray((const char *)L"\\\\?\\", 8));
    }

    string tmpPath((const char*)localPath.constData(), localPath.size() - 2);
#else
    string tmpPath((const char*)localPath.constData());
#endif

    node = megaApi->getSyncedNode(&tmpPath);
    if (!node)
    {
        return;
    }

    shellViewOnMega(node->getHandle(), versions);
    delete node;
}

void MegaApplication::shellViewOnMega(MegaHandle handle, bool versions)
{
    const auto handleBase64Pointer{MegaApi::handleToBase64(handle)};
    const auto handleArgument{QString::fromUtf8(handleBase64Pointer)};
    delete [] handleBase64Pointer;
    const auto versionsArgument{versions ? QString::fromUtf8("/versions") : QString::fromUtf8("")};
    const auto url{QString::fromUtf8("fm%1/%2").arg(versionsArgument).arg(handleArgument)};
    megaApi->getSessionTransferURL(url.toUtf8().constData());

}

void MegaApplication::exportNodes(QList<MegaHandle> exportList, QStringList extraLinks)
{
    if (appfinished || !megaApi->isLoggedIn())
    {
        return;
    }

    this->extraLinks.append(extraLinks);
    ExportProcessor *processor = new ExportProcessor(megaApi, exportList);
    connect(processor, SIGNAL(onRequestLinksFinished()), this, SLOT(onRequestLinksFinished()));
    processor->requestLinks();
    exportOps++;
}

void MegaApplication::externalDownload(QQueue<MegaNode *> newDownloadQueue)
{
    if (appfinished)
    {
        return;
    }

    downloadQueue.append(newDownloadQueue);

    if (preferences->getDownloadsPaused())
    {
        megaApi->pauseTransfers(false, MegaTransfer::TYPE_DOWNLOAD);
    }
}

void MegaApplication::externalDownload(QString megaLink, QString auth)
{
    if (appfinished)
    {
        return;
    }

    pendingLinks.insert(megaLink, auth);

    if (preferences->logged())
    {
        if (preferences->getDownloadsPaused())
        {
            megaApi->pauseTransfers(false, MegaTransfer::TYPE_DOWNLOAD);
        }

        megaApi->getPublicNode(megaLink.toUtf8().constData());
    }
    else
    {
        openInfoWizard();
    }
}

void MegaApplication::externalFileUpload(qlonglong targetFolder)
{
    if (appfinished)
    {
        return;
    }

    if (!preferences->logged())
    {
        openInfoWizard();
        return;
    }

    if (folderUploadSelector)
    {
        folderUploadSelector->activateWindow();
        folderUploadSelector->raise();
        return;
    }

    fileUploadTarget = targetFolder;
    if (fileUploadSelector)
    {
        fileUploadSelector->activateWindow();
        fileUploadSelector->raise();
        return;
    }

    fileUploadSelector = new QFileDialog();
    fileUploadSelector->setFileMode(QFileDialog::ExistingFiles);
    fileUploadSelector->setOption(QFileDialog::DontUseNativeDialog, false);

#if QT_VERSION < 0x050000
    QString defaultFolderPath = QDesktopServices::storageLocation(QDesktopServices::HomeLocation);
#else
    QString  defaultFolderPath;
    QStringList paths = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
    if (paths.size())
    {
        defaultFolderPath = paths.at(0);
    }
#endif
    fileUploadSelector->setDirectory(defaultFolderPath);

    Platform::execBackgroundWindow(fileUploadSelector);
    if (!fileUploadSelector)
    {
        return;
    }

    if (fileUploadSelector->result() == QDialog::Accepted)
    {
        QStringList paths = fileUploadSelector->selectedFiles();
        MegaNode *target = megaApi->getNodeByHandle(fileUploadTarget);
        int files = 0;
        for (int i = 0; i < paths.size(); i++)
        {
            files++;
            megaApi->startUpload(QDir::toNativeSeparators(paths[i]).toUtf8().constData(), target);
        }
        delete target;

        HTTPServer::onUploadSelectionAccepted(files, 0);
    }
    else
    {
        HTTPServer::onUploadSelectionDiscarded();
    }

    delete fileUploadSelector;
    fileUploadSelector = NULL;
    return;
}

void MegaApplication::externalFolderUpload(qlonglong targetFolder)
{
    if (appfinished)
    {
        return;
    }

    if (!preferences->logged())
    {
        openInfoWizard();
        return;
    }

    if (fileUploadSelector)
    {
        fileUploadSelector->activateWindow();
        fileUploadSelector->raise();
        return;
    }

    folderUploadTarget = targetFolder;
    if (folderUploadSelector)
    {
        folderUploadSelector->activateWindow();
        folderUploadSelector->raise();
        return;
    }

    folderUploadSelector = new QFileDialog();
    folderUploadSelector->setFileMode(QFileDialog::Directory);
    folderUploadSelector->setOption(QFileDialog::ShowDirsOnly, true);

#if QT_VERSION < 0x050000
    QString defaultFolderPath = QDesktopServices::storageLocation(QDesktopServices::HomeLocation);
#else
    QString  defaultFolderPath;
    QStringList paths = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
    if (paths.size())
    {
        defaultFolderPath = paths.at(0);
    }
#endif
    folderUploadSelector->setDirectory(defaultFolderPath);

    Platform::execBackgroundWindow(folderUploadSelector);
    if (!folderUploadSelector)
    {
        return;
    }

    if (folderUploadSelector->result() == QDialog::Accepted)
    {
        QStringList paths = folderUploadSelector->selectedFiles();
        MegaNode *target = megaApi->getNodeByHandle(folderUploadTarget);
        int files = 0;
        int folders = 0;
        for (int i = 0; i < paths.size(); i++)
        {
            folders++;
            QDirIterator it (paths[i], QDir::AllEntries | QDir::Hidden | QDir::System | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
            while (it.hasNext())
            {
                it.next();
                if (it.fileInfo().isDir())
                {
                    folders++;
                }
                else if (it.fileInfo().isFile())
                {
                    files++;
                }
            }
            megaApi->startUpload(QDir::toNativeSeparators(paths[i]).toUtf8().constData(), target);
        }
        delete target;

        HTTPServer::onUploadSelectionAccepted(files, folders);
    }
    else
    {
        HTTPServer::onUploadSelectionDiscarded();
    }

    delete folderUploadSelector;
    folderUploadSelector = NULL;
    return;
}

void MegaApplication::externalFolderSync(qlonglong targetFolder)
{
    if (appfinished)
    {
        return;
    }

    if (!preferences->logged())
    {
        openInfoWizard();
        return;
    }

    const auto upgradingDissmised{showSyncOverquotaDialog()};
    if (infoDialog && upgradingDissmised)
    {
        infoDialog->addSync(targetFolder);
    }
}

void MegaApplication::externalOpenTransferManager(int tab)
{
    if (appfinished)
    {
        return;
    }

    if (!preferences->logged())
    {
        openInfoWizard();
        return;
    }
    transferManagerActionClicked(tab);
}

void MegaApplication::internalDownload(long long handle)
{
    if (appfinished)
    {
        return;
    }

    MegaNode *node = megaApi->getNodeByHandle(handle);
    if (!node)
    {
        return;
    }

    downloadQueue.append(node);
    processDownloads();
}

//Called when the link import finishes
void MegaApplication::onLinkImportFinished()
{
    if (appfinished)
    {
        return;
    }

    LinkProcessor *linkProcessor = ((LinkProcessor *)QObject::sender());
    preferences->setImportFolder(linkProcessor->getImportParentFolder());
    linkProcessor->deleteLater();
}

void MegaApplication::onRequestLinksFinished()
{
    if (appfinished)
    {
        return;
    }

    ExportProcessor *exportProcessor = ((ExportProcessor *)QObject::sender());
    QStringList links = exportProcessor->getValidLinks();
    links.append(extraLinks);
    extraLinks.clear();

    if (!links.size())
    {
        exportOps--;
        return;
    }
    QString linkForClipboard(links.join(QChar::fromAscii('\n')));
    QApplication::clipboard()->setText(linkForClipboard);
    if (links.size() == 1)
    {
        showInfoMessage(tr("The link has been copied to the clipboard"));
    }
    else
    {
        showInfoMessage(tr("The links have been copied to the clipboard"));
    }
    exportProcessor->deleteLater();
    exportOps--;
}

void MegaApplication::onUpdateCompleted()
{
    if (appfinished)
    {
        return;
    }

#ifdef __APPLE__
    QFile exeFile(MegaApplication::applicationFilePath());
    exeFile.setPermissions(QFile::ExeOwner | QFile::ReadOwner | QFile::WriteOwner |
                              QFile::ExeGroup | QFile::ReadGroup |
                              QFile::ExeOther | QFile::ReadOther);
#endif

    updateAvailable = false;

    if (infoDialogMenu)
    {
        createTrayIcon();
    }

    if (guestMenu)
    {
        createGuestMenu();
    }

    rebootApplication();
}

void MegaApplication::onUpdateAvailable(bool requested)
{
    if (appfinished)
    {
        return;
    }

    updateAvailable = true;

    if (infoDialogMenu)
    {
        createTrayIcon();
    }

    if (guestMenu)
    {
        createGuestMenu();
    }

    if (settingsDialog)
    {
        settingsDialog->setUpdateAvailable(true);
    }

    if (requested)
    {
#ifdef WIN32
        showInfoMessage(tr("A new version of MEGAsync is available! Click on this message to install it"));
#else
        showInfoMessage(tr("A new version of MEGAsync is available!"));
#endif
    }
}

void MegaApplication::onInstallingUpdate(bool requested)
{
    if (appfinished)
    {
        return;
    }

    if (requested)
    {
        showInfoMessage(tr("Update available. Downloading..."));
    }
}

void MegaApplication::onUpdateNotFound(bool requested)
{
    if (appfinished)
    {
        return;
    }

    if (requested)
    {
        if (!updateAvailable)
        {
            showInfoMessage(tr("No update available at this time"));
        }
        else
        {
            showInfoMessage(tr("There was a problem installing the update. Please try again later or download the last version from:\nhttps://mega.co.nz/#sync")
                            .replace(QString::fromUtf8("mega.co.nz"), QString::fromUtf8("mega.nz"))
                            .replace(QString::fromUtf8("#sync"), QString::fromUtf8("sync")));
        }
    }
}

void MegaApplication::onUpdateError()
{
    if (appfinished)
    {
        return;
    }

    showInfoMessage(tr("There was a problem installing the update. Please try again later or download the last version from:\nhttps://mega.co.nz/#sync")
                    .replace(QString::fromUtf8("mega.co.nz"), QString::fromUtf8("mega.nz"))
                    .replace(QString::fromUtf8("#sync"), QString::fromUtf8("sync")));
}

//Called when users click in the tray icon
void MegaApplication::trayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (appfinished)
    {
        return;
    }

    //If account is suspended chech status
    if (blockState)
    {
        whyAmIBlocked();
    }

#ifdef Q_OS_LINUX
    if (getenv("XDG_CURRENT_DESKTOP") && (
                !strcmp(getenv("XDG_CURRENT_DESKTOP"),"ubuntu:GNOME")
                || !strcmp(getenv("XDG_CURRENT_DESKTOP"),"LXDE")
                                          )
            )
    {
        MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, QString::fromUtf8("Ignoring unexpected trayIconActivated detected in %1")
                     .arg(QString::fromUtf8(getenv("XDG_CURRENT_DESKTOP"))).toUtf8().constData());
        return;
    }
#endif


    // Code temporarily preserved here for testing
    /*if (httpServer)
    {
        HTTPRequest request;
        request.data = QString::fromUtf8("{\"a\":\"ufi\",\"h\":\"%1\"}")
        //request.data = QString::fromUtf8("{\"a\":\"ufo\",\"h\":\"%1\"}")
        //request.data = QString::fromUtf8("{\"a\":\"s\",\"h\":\"%1\"}")
        //request.data = QString::fromUtf8("{\"a\":\"t\",\"h\":\"908TDC6J\"}")
                .arg(QString::fromUtf8(megaApi->getNodeByPath("/MEGAsync Uploads")->getBase64Handle()));
        httpServer->processRequest(NULL, request);
    }*/

    registerUserActivity();
    megaApi->retryPendingConnections();

    if (reason == QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::Context)
    {
        if (!infoDialog)
        {
            if (setupWizard)
            {
                setupWizard->activateWindow();
                setupWizard->raise();
            }
            else if (reason == QSystemTrayIcon::Trigger)
            {
                if (blockState)
                {
                    showInfoMessage(tr("Locked account"));
                }
                else if (!megaApi->isLoggedIn())
                {
                    showInfoMessage(tr("Logging in..."));
                }
                else
                {
                    showInfoMessage(tr("Fetching file list..."));
                }
            }
            return;
        }

#ifndef __APPLE__
        if (reason == QSystemTrayIcon::Context)
        {
            return;
        }
#endif /* ! __APPLE__ */

#ifndef __APPLE__
        if (isLinux)
        {
            if (infoDialogMenu && infoDialogMenu->isVisible())
            {
                infoDialogMenu->close();
            }
        }
#ifdef _WIN32
        // in windows, a second click on the task bar icon first deactivates the app which closes the infoDialg.  
        // This statement prevents us opening it again, so that we have one-click to open the infoDialog, and a second closes it.
        if (!infoDialog || (chrono::steady_clock::now() - infoDialog->lastWindowHideTime > 100ms))
#endif
        {
            infoDialogTimer->start(200);
        }
#else
        showInfoDialog();
#endif
    }
#ifndef __APPLE__
    else if (reason == QSystemTrayIcon::DoubleClick)
    {
        if (!infoDialog)
        {
            if (setupWizard)
            {
                setupWizard->activateWindow();
                setupWizard->raise();
            }
            else
            {
                if (blockState)
                {
                    showInfoMessage(tr("Locked account"));
                }
                else if (!megaApi->isLoggedIn())
                {
                    showInfoMessage(tr("Logging in..."));
                }
                else
                {
                    showInfoMessage(tr("Fetching file list..."));
                }
            }

            return;
        }

        // open local folder for the first active setting
        std::shared_ptr<SyncSetting> firstActiveSyncSetting;

        for (int i = 0; i < model->getNumSyncedFolders(); i++)
        {
            auto syncSetting = model->getSyncSetting(i);

            if (syncSetting->isActive())
            {
                firstActiveSyncSetting = syncSetting;
                break;
            }
        }

        if (!firstActiveSyncSetting)
        {
            return;
        }

        infoDialogTimer->stop();
        infoDialog->hide();
        QString localFolderPath = firstActiveSyncSetting->getLocalFolder();
        if (!localFolderPath.isEmpty())
        {
            QtConcurrent::run(QDesktopServices::openUrl, QUrl::fromLocalFile(localFolderPath));
        }
    }
    else if (reason == QSystemTrayIcon::MiddleClick)
    {
        showTrayMenu();
    }
#endif
}

void MegaApplication::onMessageClicked()
{
    if (appfinished)
    {
        return;
    }

    if (lastTrayMessage == tr("A new version of MEGAsync is available! Click on this message to install it"))
    {
        triggerInstallUpdate();
    }
    else if (lastTrayMessage == tr("MEGAsync is now running. Click here to open the status window."))
    {
        trayIconActivated(QSystemTrayIcon::Trigger);
    }
}

void MegaApplication::openInfoWizard()
{
    if (appfinished)
    {
        return;
    }

    if (infoWizard)
    {
        infoWizard->activateWindow();
        infoWizard->raise();
        return;
    }

    infoWizard = new InfoWizard();
    connect(infoWizard, SIGNAL(actionButtonClicked(int)), this, SLOT(userAction(int)));
    connect(infoWizard, SIGNAL(finished(int)), this, SLOT(infoWizardDialogFinished(int)));
    Platform::activateBackgroundWindow(infoWizard);
    infoWizard->show();
}

void MegaApplication::openSettings(int tab)
{
    if (appfinished)
    {
        return;
    }

    bool proxyOnly = true;

    if (megaApi)
    {
        proxyOnly = !megaApi->isFilesystemAvailable() || !preferences->logged() || blockState;
        megaApi->retryPendingConnections();
    }

    if (isLinux && blockState) //we force a whyamiblocked here since trayIconActivated might not be available
    {
        whyAmIBlocked();
    }

#ifndef __MACH__
    if (preferences && !proxyOnly)
    {
        updateUserStats(true, true, true, true, USERSTATS_OPENSETTINGSDIALOG);
    }
#endif

    if (settingsDialog)
    {
        settingsDialog->setProxyOnly(proxyOnly);

        //If the dialog is active
        if (settingsDialog->isVisible())
        {
            if (!proxyOnly)
            {
                settingsDialog->openSettingsTab(tab);
            }

            //and visible -> show it
            settingsDialog->activateWindow();
            settingsDialog->raise();
            return;
        }

        //Otherwise, delete it
        delete settingsDialog;
        settingsDialog = NULL;
    }

    //Show a new settings dialog
    settingsDialog = new SettingsDialog(this, proxyOnly);
    connect(settingsDialog, SIGNAL(userActivity()), this, SLOT(registerUserActivity()));

    if (!proxyOnly)
    {
        settingsDialog->openSettingsTab(tab);
    }

    settingsDialog->setUpdateAvailable(updateAvailable);
    settingsDialog->setModal(false);
    settingsDialog->loadSettings();
    settingsDialog->show();
}

void MegaApplication::createAppMenus()
{
    if (appfinished)
    {
        return;
    }

    lastHovered = NULL;

    if (initialMenu)
    {
        QList<QAction *> actions = initialMenu->actions();
        for (int i = 0; i < actions.size(); i++)
        {
            initialMenu->removeAction(actions[i]);
        }
    }
#ifndef _WIN32 // win32 needs to recreate menu to fix scaling qt issue
    else
#endif
    {
        initialMenu.reset(new QMenu());
    }


    if (changeProxyAction)
    {
        changeProxyAction->deleteLater();
        changeProxyAction = NULL;
    }
    changeProxyAction = new QAction(tr("Settings"), this);
    connect(changeProxyAction, SIGNAL(triggered()), this, SLOT(openSettings()));

    if (initialExitAction)
    {
        initialExitAction->deleteLater();
        initialExitAction = NULL;
    }
    initialExitAction = new QAction(tr("Exit"), this);
    connect(initialExitAction, SIGNAL(triggered()), this, SLOT(exitApplication()));

    initialMenu->addAction(changeProxyAction);
    initialMenu->addAction(initialExitAction);


    if (isLinux && infoDialog)
    {
        if (showStatusAction)
        {
            showStatusAction->deleteLater();
            showStatusAction = NULL;
        }

        showStatusAction = new QAction(tr("Show status"), this);
        connect(showStatusAction, SIGNAL(triggered()), this, SLOT(showInfoDialog()));

        initialMenu->insertAction(changeProxyAction, showStatusAction);
    }

#ifdef _WIN32
    //The following should not be required, but
    //prevents it from being truncated on the first display
    initialMenu->show();
    initialMenu->hide();
#endif


#ifdef _WIN32
    if (!windowsMenu)
    {
        windowsMenu.reset(new QMenu());
    }
    else
    {
        QList<QAction *> actions = windowsMenu->actions();
        for (int i = 0; i < actions.size(); i++)
        {
            windowsMenu->removeAction(actions[i]);
        }
    }

    if (windowsExitAction)
    {
        windowsExitAction->deleteLater();
        windowsExitAction = NULL;
    }

    windowsExitAction = new QAction(tr("Exit"), this);
    connect(windowsExitAction, SIGNAL(triggered()), this, SLOT(exitApplication()));

    if (windowsSettingsAction)
    {
        windowsSettingsAction->deleteLater();
        windowsSettingsAction = NULL;
    }

    windowsSettingsAction = new QAction(tr("Settings"), this);
    connect(windowsSettingsAction, SIGNAL(triggered()), this, SLOT(openSettings()));

    if (windowsImportLinksAction)
    {
        windowsImportLinksAction->deleteLater();
        windowsImportLinksAction = NULL;
    }

    windowsImportLinksAction = new QAction(tr("Open links"), this);
    connect(windowsImportLinksAction, SIGNAL(triggered()), this, SLOT(importLinks()));

    if (windowsUploadAction)
    {
        windowsUploadAction->deleteLater();
        windowsUploadAction = NULL;
    }

    windowsUploadAction = new QAction(tr("Upload"), this);
    connect(windowsUploadAction, SIGNAL(triggered()), this, SLOT(uploadActionClicked()));

    if (windowsDownloadAction)
    {
        windowsDownloadAction->deleteLater();
        windowsDownloadAction = NULL;
    }

    windowsDownloadAction = new QAction(tr("Download"), this);
    connect(windowsDownloadAction, SIGNAL(triggered()), this, SLOT(downloadActionClicked()));

    if (windowsStreamAction)
    {
        windowsStreamAction->deleteLater();
        windowsStreamAction = NULL;
    }

    windowsStreamAction = new QAction(tr("Stream"), this);
    connect(windowsStreamAction, SIGNAL(triggered()), this, SLOT(streamActionClicked()));

    if (windowsTransferManagerAction)
    {
        windowsTransferManagerAction->deleteLater();
        windowsTransferManagerAction = NULL;
    }

    windowsTransferManagerAction = new QAction(tr("Transfer manager"), this);
    connect(windowsTransferManagerAction, SIGNAL(triggered()), this, SLOT(transferManagerActionClicked()));

    if (windowsUpdateAction)
    {
        windowsUpdateAction->deleteLater();
        windowsUpdateAction = NULL;
    }

    if (updateAvailable)
    {
        windowsUpdateAction = new QAction(tr("Install update"), this);
    }
    else
    {
        windowsUpdateAction = new QAction(tr("About"), this);
    }
    connect(windowsUpdateAction, SIGNAL(triggered()), this, SLOT(onInstallUpdateClicked()));

    windowsMenu->addAction(windowsUpdateAction);
    windowsMenu->addSeparator();
    windowsMenu->addAction(windowsImportLinksAction);
    windowsMenu->addAction(windowsUploadAction);
    windowsMenu->addAction(windowsDownloadAction);
    windowsMenu->addAction(windowsStreamAction);
    windowsMenu->addAction(windowsTransferManagerAction);
    windowsMenu->addAction(windowsSettingsAction);
    windowsMenu->addSeparator();
    windowsMenu->addAction(windowsExitAction);

    //The following should not be required, but
    //prevents it from being truncated on the first display
    windowsMenu->show();
    windowsMenu->hide();

#endif

    if (infoDialogMenu)
    {
        QList<QAction *> actions = infoDialogMenu->actions();
        for (int i = 0; i < actions.size(); i++)
        {
            infoDialogMenu->removeAction(actions[i]);
        }
    }
#ifndef _WIN32 // win32 needs to recreate menu to fix scaling qt issue
    else
#endif
    {
        infoDialogMenu.reset(new QMenu());
#ifdef __APPLE__
        infoDialogMenu->setStyleSheet(QString::fromAscii("QMenu {background: #ffffff; padding-top: 8px; padding-bottom: 8px;}"));
#else
        infoDialogMenu->setStyleSheet(QString::fromAscii("QMenu { border: 1px solid #B8B8B8; border-radius: 5px; background: #ffffff; padding-top: 5px; padding-bottom: 5px;}"));
#endif

        //Highlight menu entry on mouse over
        connect(infoDialogMenu.get(), SIGNAL(hovered(QAction*)), this, SLOT(highLightMenuEntry(QAction*)), Qt::QueuedConnection);

        //Hide highlighted menu entry when mouse over
        infoDialogMenu->installEventFilter(this);
    }

    if (exitAction)
    {
        exitAction->deleteLater();
        exitAction = NULL;
    }

#ifndef __APPLE__
    exitAction = new MenuItemAction(tr("Exit"), QIcon(QString::fromAscii("://images/ico_quit.png")), true);
#else
    exitAction = new MenuItemAction(tr("Quit"), QIcon(QString::fromAscii("://images/ico_quit.png")), true);
#endif
    connect(exitAction, SIGNAL(triggered()), this, SLOT(exitApplication()), Qt::QueuedConnection);

    if (settingsAction)
    {
        settingsAction->deleteLater();
        settingsAction = NULL;
    }

#ifndef __APPLE__
    settingsAction = new MenuItemAction(tr("Settings"), QIcon(QString::fromAscii("://images/ico_preferences.png")), true);
#else
    settingsAction = new MenuItemAction(tr("Preferences"), QIcon(QString::fromAscii("://images/ico_preferences.png")), true);
#endif
    connect(settingsAction, SIGNAL(triggered()), this, SLOT(openSettings()), Qt::QueuedConnection);

    if (myCloudAction)
    {
        myCloudAction->deleteLater();
        myCloudAction = NULL;
    }

    myCloudAction = new MenuItemAction(tr("Cloud drive"), QIcon(QString::fromAscii("://images/ico_cloud_drive.png")), true);
    connect(myCloudAction, SIGNAL(triggered()), this, SLOT(goToMyCloud()), Qt::QueuedConnection);

    if (addSyncAction)
    {
        addSyncAction->deleteLater();
        addSyncAction = NULL;
    }

    int num = (megaApi && preferences->logged()) ? model->getNumSyncedFolders() : 0;
    if (num == 0)
    {
        addSyncAction = new MenuItemAction(tr("Add Sync"), QIcon(QString::fromAscii("://images/ico_add_sync_folder.png")), true);

#if QT_VERSION > QT_VERSION_CHECK(5, 7, 0)
        connect(addSyncAction, &MenuItemAction::triggered, infoDialog,
                QOverload<>::of(&InfoDialog::addSync), Qt::QueuedConnection);
#else
        connect(addSyncAction, SIGNAL(triggered()), infoDialog, SLOT(addSync()), Qt::QueuedConnection);
#endif

    }
    else
    {
        addSyncAction = new MenuItemAction(tr("Syncs"), QIcon(QString::fromAscii("://images/ico_add_sync_folder.png")), true);
        if (syncsMenu)
        {
            for (QAction *a: syncsMenu->actions())
            {
                a->deleteLater();
            }

            syncsMenu->deleteLater();
            syncsMenu.release();
        }

        syncsMenu.reset(new QMenu());

#ifdef __APPLE__
        syncsMenu->setStyleSheet(QString::fromAscii("QMenu {background: #ffffff; padding-top: 8px; padding-bottom: 8px;}"));
#else
        syncsMenu->setStyleSheet(QString::fromAscii("QMenu { border: 1px solid #B8B8B8; border-radius: 5px; background: #ffffff; padding-top: 8px; padding-bottom: 8px;}"));
#endif


        if (menuSignalMapper)
        {
            menuSignalMapper->deleteLater();
            menuSignalMapper = NULL;
        }

        menuSignalMapper = new QSignalMapper();
        connect(menuSignalMapper, SIGNAL(mapped(QString)), infoDialog, SLOT(openFolder(QString)), Qt::QueuedConnection);

        int activeFolders = 0;
        for (int i = 0; i < num; i++)
        {
            auto syncSetting = model->getSyncSetting(i);

            if (!syncSetting->isActive())
            {
                continue;
            }

            activeFolders++;
            MenuItemAction *action = new MenuItemAction(syncSetting->name(), QIcon(QString::fromAscii("://images/ico_drop_synched_folder.png")), true);
            connect(action, SIGNAL(triggered()), menuSignalMapper, SLOT(map()), Qt::QueuedConnection);

            syncsMenu->addAction(action);
            menuSignalMapper->setMapping(action, syncSetting->getLocalFolder());
        }

        if (!activeFolders)
        {
            addSyncAction->setLabelText(tr("Add Sync"));
#if QT_VERSION > QT_VERSION_CHECK(5, 7, 0)
            connect(addSyncAction, &MenuItemAction::triggered, infoDialog,
                    QOverload<>::of(&InfoDialog::addSync), Qt::QueuedConnection);
#else
            connect(addSyncAction, SIGNAL(triggered()), infoDialog, SLOT(addSync()), Qt::QueuedConnection);
#endif
        }
        else
        {
            auto rootNode = getRootNode();
            if (rootNode)
            {
                bool fullSync = num == 1 && model->getSyncSetting(0)->getMegaHandle() == rootNode->getHandle();
                if ((num > 1) || !fullSync)
                {
                    MenuItemAction *addAction = new MenuItemAction(tr("Add Sync"), QIcon(QString::fromAscii("://images/ico_drop_add_sync.png")), true);
#if QT_VERSION > QT_VERSION_CHECK(5, 7, 0)
                    connect(addAction, &MenuItemAction::triggered, infoDialog,
                            QOverload<>::of(&InfoDialog::addSync), Qt::QueuedConnection);
#else
                    connect(addAction, &MenuItemAction::triggered, infoDialog,
                            static_cast<void(InfoDialog::*)()>(&InfoDialog::addSync), Qt::QueuedConnection);
#endif

                    if (activeFolders)
                    {
                        syncsMenu->addSeparator();
                    }
                    syncsMenu->addAction(addAction);
                }
            }

            addSyncAction->setMenu(syncsMenu.get());
        }
    }

    if (importLinksAction)
    {
        importLinksAction->deleteLater();
        importLinksAction = NULL;
    }

    importLinksAction = new MenuItemAction(tr("Open links"), QIcon(QString::fromAscii("://images/ico_Import_links.png")), true);
    connect(importLinksAction, SIGNAL(triggered()), this, SLOT(importLinks()), Qt::QueuedConnection);

    if (uploadAction)
    {
        uploadAction->deleteLater();
        uploadAction = NULL;
    }

    uploadAction = new MenuItemAction(tr("Upload"), QIcon(QString::fromAscii("://images/ico_upload.png")), true);
    connect(uploadAction, SIGNAL(triggered()), this, SLOT(uploadActionClicked()), Qt::QueuedConnection);

    if (downloadAction)
    {
        downloadAction->deleteLater();
        downloadAction = NULL;
    }

    downloadAction = new MenuItemAction(tr("Download"), QIcon(QString::fromAscii("://images/ico_download.png")), true);
    connect(downloadAction, SIGNAL(triggered()), this, SLOT(downloadActionClicked()), Qt::QueuedConnection);

    if (streamAction)
    {
        streamAction->deleteLater();
        streamAction = NULL;
    }

    streamAction = new MenuItemAction(tr("Stream"), QIcon(QString::fromAscii("://images/ico_stream.png")), true);
    connect(streamAction, SIGNAL(triggered()), this, SLOT(streamActionClicked()), Qt::QueuedConnection);

    if (updateAction)
    {
        updateAction->deleteLater();
        updateAction = NULL;
    }

    if (updateAvailable)
    {
        updateAction = new MenuItemAction(tr("Install update"), QIcon(QString::fromAscii("://images/ico_about_MEGA.png")), true);
    }
    else
    {
        updateAction = new MenuItemAction(tr("About MEGAsync"), QIcon(QString::fromAscii("://images/ico_about_MEGA.png")), true);
    }
    connect(updateAction, SIGNAL(triggered()), this, SLOT(onInstallUpdateClicked()), Qt::QueuedConnection);

    infoDialogMenu->addAction(updateAction);
    infoDialogMenu->addAction(myCloudAction);
    infoDialogMenu->addSeparator();
    infoDialogMenu->addAction(addSyncAction);
    infoDialogMenu->addAction(importLinksAction);
    infoDialogMenu->addAction(uploadAction);
    infoDialogMenu->addAction(downloadAction);
    infoDialogMenu->addAction(streamAction);
    infoDialogMenu->addAction(settingsAction);
    infoDialogMenu->addSeparator();
    infoDialogMenu->addAction(exitAction);
#ifdef _WIN32
    //The following should not be required, but
    //prevents it from being truncated on the first display
    infoDialogMenu->show();
    infoDialogMenu->hide();
#endif

    updateTrayIconMenu();
}

void MegaApplication::createGuestMenu()
{
    if (appfinished)
    {
        return;
    }

    if (guestMenu)
    {
        QList<QAction *> actions = guestMenu->actions();
        for (int i = 0; i < actions.size(); i++)
        {
            guestMenu->removeAction(actions[i]);
        }
    }
#ifndef _WIN32 // win32 needs to recreate menu to fix scaling qt issue
    else
#endif
    {
        guestMenu.reset(new QMenu());

#ifdef __APPLE__
        guestMenu->setStyleSheet(QString::fromAscii("QMenu {background: #ffffff; padding-top: 8px; padding-bottom: 8px;}"));
#else
        guestMenu->setStyleSheet(QString::fromAscii("QMenu { border: 1px solid #B8B8B8; border-radius: 5px; background: #ffffff; padding-top: 5px; padding-bottom: 5px;}"));
#endif
    }

    if (exitActionGuest)
    {
        exitActionGuest->deleteLater();
        exitActionGuest = NULL;
    }

#ifndef __APPLE__
    exitActionGuest = new MenuItemAction(tr("Exit"), QIcon(QString::fromAscii("://images/ico_quit.png")));
#else
    exitActionGuest = new MenuItemAction(tr("Quit"), QIcon(QString::fromAscii("://images/ico_quit.png")));
#endif
    connect(exitActionGuest, SIGNAL(triggered()), this, SLOT(exitApplication()));

    if (updateActionGuest)
    {
        updateActionGuest->deleteLater();
        updateActionGuest = NULL;
    }

    if (updateAvailable)
    {
        updateActionGuest = new MenuItemAction(tr("Install update"), QIcon(QString::fromAscii("://images/ico_about_MEGA.png")));
    }
    else
    {
        updateActionGuest = new MenuItemAction(tr("About MEGAsync"), QIcon(QString::fromAscii("://images/ico_about_MEGA.png")));
    }
    connect(updateActionGuest, SIGNAL(triggered()), this, SLOT(onInstallUpdateClicked()));

    if (settingsActionGuest)
    {
        settingsActionGuest->deleteLater();
        settingsActionGuest = NULL;
    }

#ifndef __APPLE__
    settingsActionGuest = new MenuItemAction(tr("Settings"), QIcon(QString::fromAscii("://images/ico_preferences.png")));
#else
    settingsActionGuest = new MenuItemAction(tr("Preferences"), QIcon(QString::fromAscii("://images/ico_preferences.png")));
#endif
    connect(settingsActionGuest, SIGNAL(triggered()), this, SLOT(openSettings()));

    guestMenu->addAction(updateActionGuest);
    guestMenu->addSeparator();
    guestMenu->addAction(settingsActionGuest);
    guestMenu->addSeparator();
    guestMenu->addAction(exitActionGuest);

#ifdef _WIN32
    //The following should not be required, but
    //prevents it from being truncated on the first display
    guestMenu->show();
    guestMenu->hide();
#endif
}

void MegaApplication::refreshStorageUIs()
{
    if (infoDialog)
    {
        infoDialog->setUsage();
    }

    notifyStorageObservers(); //Ideally this should be the only call here

    transferQuota->refreshOverQuotaDialogDetails();

    if (storageOverquotaDialog)
    {
        storageOverquotaDialog->refreshUsedStorage();
    }
}

void MegaApplication::onEvent(MegaApi *api, MegaEvent *event)
{
    DeferPreferencesSyncForScope deferrer(this);

    if (event->getType() == MegaEvent::EVENT_CHANGE_TO_HTTPS)
    {
        preferences->setUseHttpsOnly(true);
    }
    else if (event->getType() == MegaEvent::EVENT_SYNCS_RESTORED)
    {
        Platform::notifyAllSyncFoldersAdded();
        showNotificationMessage(tr("Your syncs have been enabled"));
    }
    else if (event->getType() == MegaEvent::EVENT_SYNCS_DISABLED && event->getNumber() != MegaSync::Error::LOGGED_OUT)
    {
        showErrorMessage(tr("Your syncs have been temporarily disabled").append(QString::fromUtf8(": "))
                         .append(QCoreApplication::translate("MegaSyncError", MegaSync::getMegaSyncErrorCode(event->getNumber()))));
    }
    else if (event->getType() == MegaEvent::EVENT_ACCOUNT_BLOCKED)
    {
        switch (event->getNumber())
        {
            case MegaApi::ACCOUNT_BLOCKED_VERIFICATION_EMAIL:
            case MegaApi::ACCOUNT_BLOCKED_VERIFICATION_SMS:
            {
                blockState = event->getNumber();
                emit blocked();
                blockStateSet = true;
                if (preferences->logged())
                {
                    preferences->setBlockedState(blockState);
                }

                if (verifyEmail)
                {
                    verifyEmail->regenerateUI(blockState);
                }

                if (infoDialog)
                {
                    if (infoDialog->getLoggedInMode() != blockState)
                    {
                        infoDialog->regenerateLayout(blockState);
                        closeDialogs();
                    }
                }
                else if (!whyamiblockedPeriodicPetition) //Do not force show on periodic whyamiblocked call
                {
                    showVerifyAccountInfo();
                }

                whyamiblockedPeriodicPetition = false;
                break;
            }
            case MegaApi::ACCOUNT_BLOCKED_SUBUSER_DISABLED:
            {
                QMegaMessageBox::warning(nullptr, tr("MEGAsync"), tr("Your account has been disabled by your administrator. Please contact your business account administrator for further details."));
                break;
            }
            default:
                QMegaMessageBox::critical(nullptr, QString::fromUtf8("MEGAsync"),
                                          QCoreApplication::translate("MegaError", event->getText()));
                break;
        }

    }
    else if (event->getType() == MegaEvent::EVENT_NODES_CURRENT)
    {
        nodescurrent = true;
    }
    else if (event->getType() == MegaEvent::EVENT_STORAGE)
    {
        if (preferences->logged())
        {
            applyStorageState(event->getNumber());
        }
        else //event arrived too soon, we will apply it later
        {
            std::unique_ptr<MegaEvent> eventCopy{event->copy()};
            eventsPendingLoggedIn.push_back(std::move(eventCopy));
        }
    }
    else if (event->getType() == MegaEvent::EVENT_STORAGE_SUM_CHANGED)
    {
        receivedStorageSum = event->getNumber();
        if (!preferences->logged())
        {
            return;
        }

        if (storageState == MegaApi::STORAGE_STATE_RED && receivedStorageSum < preferences->totalStorage())
        {
            preferences->setUsedStorage(preferences->totalStorage());
        }
        else
        {
            preferences->setUsedStorage(receivedStorageSum);
        }
        preferences->sync();

        refreshStorageUIs();
    }
    else if (event->getType() == MegaEvent::EVENT_BUSINESS_STATUS)
    {
        switch (event->getNumber())
        {
            case MegaApi::BUSINESS_STATUS_GRACE_PERIOD:
            {
                if (megaApi->isMasterBusinessAccount())
                {
                    QMessageBox msgBox;
                    HighDpiResize hDpiResizer(&msgBox);
                    msgBox.setIcon(QMessageBox::Warning);
                    // Remove ifdef code for window modality when upgrade to QT 5.9. Issue seems to be fixed.
                    #ifdef __APPLE__
                        msgBox.setWindowModality(Qt::WindowModal);
                    #endif
                    msgBox.setText(tr("Payment Failed"));
                    msgBox.setInformativeText(tr("This month's payment has failed. Please resolve your payment issue as soon as possible to avoid any suspension of your business account."));
                    msgBox.addButton(tr("Pay Now"), QMessageBox::AcceptRole);
                    msgBox.addButton(tr("Dismiss"), QMessageBox::RejectRole);
                    msgBox.setDefaultButton(QMessageBox::Yes);
                    int ret = msgBox.exec();
                    if (ret == QMessageBox::AcceptRole)
                    {                        
                        QString url = QString::fromUtf8("mega://#repay");
                        Utilities::getPROurlWithParameters(url);
                        QtConcurrent::run(QDesktopServices::openUrl, QUrl(url));
                    }
                }

                if (preferences->logged() &&
                        ( ( businessStatus != -2 && businessStatus == MegaApi::BUSINESS_STATUS_EXPIRED) // transitioning from expired
                          || preferences->getBusinessState() == MegaApi::BUSINESS_STATUS_EXPIRED // last known was expired (in cache: previous execution)
                        ))
                {
                    MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, QString::fromUtf8("no longer BUSINESS_STATUS_EXPIRED").toUtf8().constData());
                }
                break;
            }
            case MegaApi::BUSINESS_STATUS_EXPIRED:
            {
                QMessageBox msgBox;
                HighDpiResize hDpiResizer(&msgBox);
                msgBox.setIcon(QMessageBox::Warning);
                // Remove ifdef code for window modality when upgrade to QT 5.9. Issue seems to be fixed.
                #ifdef __APPLE__
                    msgBox.setWindowModality(Qt::WindowModal);
                #endif

                if (megaApi->isMasterBusinessAccount())
                {
                    msgBox.setText(tr("Your Business account is expired"));
                    msgBox.setInformativeText(tr("It seems the payment for your business account has failed. Your account is suspended as read only until you proceed with the needed payments."));
                    msgBox.addButton(tr("Pay Now"), QMessageBox::AcceptRole);
                    msgBox.addButton(tr("Dismiss"), QMessageBox::RejectRole);
                    msgBox.setDefaultButton(QMessageBox::Yes);
                    int ret = msgBox.exec();
                    if (ret == QMessageBox::AcceptRole)
                    {
                        QString url = QString::fromUtf8("mega://#repay");
                        Utilities::getPROurlWithParameters(url);
                        QtConcurrent::run(QDesktopServices::openUrl, QUrl(url));
                    }
                }
                else
                {
                    msgBox.setText(tr("Account Suspended"));
                    msgBox.setTextFormat(Qt::RichText);
                    msgBox.setInformativeText(
                                tr("Your account is currently [A]suspended[/A]. You can only browse your data.")
                                    .replace(QString::fromUtf8("[A]"), QString::fromUtf8("<span style=\"font-weight: bold; text-decoration:none;\">"))
                                    .replace(QString::fromUtf8("[/A]"), QString::fromUtf8("</span>"))
                                + QString::fromUtf8("<br>") + QString::fromUtf8("<br>") +
                                tr("[A]Important:[/A] Contact your business account administrator to resolve the issue and activate your account.")
                                    .replace(QString::fromUtf8("[A]"), QString::fromUtf8("<span style=\"font-weight: bold; color:#DF4843; text-decoration:none;\">"))
                                    .replace(QString::fromUtf8("[/A]"), QString::fromUtf8("</span>")) + QString::fromAscii("\n"));

                    msgBox.addButton(tr("Dismiss"), QMessageBox::RejectRole);
                    msgBox.exec();
                }

                break;
            }
            case MegaApi::BUSINESS_STATUS_ACTIVE:
            case MegaApi::BUSINESS_STATUS_INACTIVE:
            {
            if (preferences->logged() &&
                    ( ( businessStatus != -2 && businessStatus == MegaApi::BUSINESS_STATUS_EXPIRED) // transitioning from expired
                      || preferences->getBusinessState() == MegaApi::BUSINESS_STATUS_EXPIRED // last known was expired (in cache: previous execution)
                    ))
                {
                    MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, QString::fromUtf8("no longer BUSINESS_STATUS_EXPIRED").toUtf8().constData());
                }
                break;
            }
            default:
                break;
        }

        businessStatus = event->getNumber();
        if (preferences->logged())
        {
            preferences->setBusinessState(businessStatus);
        }
    }
}

//Called when a request is about to start
void MegaApplication::onRequestStart(MegaApi* , MegaRequest *request)
{
    if (appfinished)
    {
        return;
    }

    if (request->getType() == MegaRequest::TYPE_LOGIN)
    {
        connectivityTimer->start();
    }
    else if (request->getType() == MegaRequest::TYPE_GET_LOCAL_SSL_CERT)
    {
        updatingSSLcert = true;
    }
}

//Called when a request has finished
void MegaApplication::onRequestFinish(MegaApi*, MegaRequest *request, MegaError* e)
{
    if (appfinished)
    {
        return;
    }

    DeferPreferencesSyncForScope deferrer(this);

    if (sslKeyPinningError && request->getType() != MegaRequest::TYPE_LOGOUT)
    {
        delete sslKeyPinningError;
        sslKeyPinningError = NULL;
    }

    if (e->getErrorCode() == MegaError::API_EBUSINESSPASTDUE
            && (!lastTsBusinessWarning || (QDateTime::currentMSecsSinceEpoch() - lastTsBusinessWarning) > 3000))//Notify only once within last five seconds
    {
        lastTsBusinessWarning = QDateTime::currentMSecsSinceEpoch();
        sendBusinessWarningNotification();
    }

    if (e->getErrorCode() == MegaError::API_EPAYWALL)
    {
        if (appliedStorageState != MegaApi::STORAGE_STATE_PAYWALL)
        {
            applyStorageState(MegaApi::STORAGE_STATE_PAYWALL);
        }
    }
    
    switch (request->getType())
    {
    case MegaRequest::TYPE_EXPORT:
    {
        if (!exportOps && e->getErrorCode() == MegaError::API_OK)
        {
            //A public link has been created, put it in the clipboard and inform users
            QString linkForClipboard(QString::fromUtf8(request->getLink()));
            QApplication::clipboard()->setText(linkForClipboard);
            showInfoMessage(tr("The link has been copied to the clipboard"));
        }

        if (e->getErrorCode() != MegaError::API_OK
                && e->getErrorCode() != MegaError::API_EBUSINESSPASTDUE)
        {
            showErrorMessage(tr("Error getting link: ") + QString::fromUtf8(" ") + QCoreApplication::translate("MegaError", e->getErrorString()));
        }

        break;
    }
    case MegaRequest::TYPE_GET_PRICING:
    {
        if (e->getErrorCode() == MegaError::API_OK)
        {
            if (pricing)
            {
                delete pricing;
            }
            pricing = request->getPricing();
            transferQuota->setOverQuotaDialogPricing(pricing);

            if (storageOverquotaDialog)
            {
                storageOverquotaDialog->setPricing(pricing);
            }
        }
        break;
    }
    case MegaRequest::TYPE_SET_ATTR_USER:
    {
        if (!preferences->logged())
        {
            break;
        }

        if (e->getErrorCode() != MegaError::API_OK)
        {
            break;
        }

        if (request->getParamType() == MegaApi::USER_ATTR_DISABLE_VERSIONS)
        {
            preferences->disableFileVersioning(!strcmp(request->getText(), "1"));
        }
        else if (request->getParamType() == MegaApi::USER_ATTR_LAST_PSA)
        {
            megaApi->getPSA();
        }

        break;
    }
    case MegaRequest::TYPE_GET_ATTR_USER:
    {
        if (!preferences->logged())
        {
            break;
        }

        if (e->getErrorCode() != MegaError::API_OK && e->getErrorCode() != MegaError::API_ENOENT)
        {
            break;
        }

        if (request->getParamType() == MegaApi::USER_ATTR_FIRSTNAME)
        {
            QString firstname(QString::fromUtf8(""));
            if (e->getErrorCode() == MegaError::API_OK && request->getText())
            {
                firstname = QString::fromUtf8(request->getText());
            }
            preferences->setFirstName(firstname);
        }
        else if (request->getParamType() == MegaApi::USER_ATTR_LASTNAME)
        {
            QString lastName(QString::fromUtf8(""));
            if (e->getErrorCode() == MegaError::API_OK && request->getText())
            {
                lastName = QString::fromUtf8(request->getText());
            }
            preferences->setLastName(lastName);
        }
        else if (request->getParamType() == MegaApi::USER_ATTR_AVATAR)
        {
            if (e->getErrorCode() == MegaError::API_ENOENT)
            {
                const char *email = megaApi->getMyEmail();
                if (email)
                {
                    QFile::remove(Utilities::getAvatarPath(QString::fromUtf8(email)));
                    delete [] email;
                }
            }

            if (infoDialog)
            {
                infoDialog->setAvatar();
            }
        }
        else if (request->getParamType() == MegaApi::USER_ATTR_DISABLE_VERSIONS)
        {
            if (e->getErrorCode() == MegaError::API_OK
                    || e->getErrorCode() == MegaError::API_ENOENT)
            {
                // API_ENOENT is expected when the user has never disabled versioning
                preferences->disableFileVersioning(request->getFlag());
            }
        }

        break;
    }
    case MegaRequest::TYPE_LOGIN:
    {
        connectivityTimer->stop();

        // We do this after login to ensure the request to get the local SSL certs is not in the queue
        // while login request is being processed. This way, the local SSL certs request is not aborted.
        initLocalServer();

        if (e->getErrorCode() == MegaError::API_OK)
        {
            preferences->setAccountStateInGeneral(Preferences::STATE_LOGGED_OK); //TODO: setGlobalAccountState

            auto needsFetchNodes = preferences->needsFetchNodesInGeneral();

            std::unique_ptr<char []> session(megaApi->dumpSession());
            if (session)
            {
                preferences->setSession(QString::fromUtf8(session.get()));
            }

            // In case fetchnode fails in previous request,
            // but we have an active session, we will need to launch a fetchnodes
            if (!preferences->logged()
                    && needsFetchNodes)
            {
                auto email = request->getEmail();
                fetchNodes(QString::fromUtf8(email ? email : ""));
            }
        }

        //This prevents to handle logins in the initial setup wizard
        if (preferences->logged())
        {
            Platform::prepareForSync();
            int errorCode = e->getErrorCode();
            if (errorCode == MegaError::API_OK)
            {
                if (!preferences->getSession().isEmpty())
                {
                    //Successful login, fetch nodes
                    fetchNodes();
                    break;
                }
            }
            else if (errorCode == MegaError::API_EBLOCKED)
            {
                QMegaMessageBox::critical(nullptr, tr("MEGAsync"), tr("Your account has been blocked. Please contact support@mega.co.nz"));
            }
            else if (errorCode != MegaError::API_ESID && errorCode != MegaError::API_ESSL)
            //Invalid session or public key, already managed in TYPE_LOGOUT
            {
                QMegaMessageBox::warning(nullptr, tr("MEGAsync"), tr("Login error: %1").arg(QCoreApplication::translate("MegaError", e->getErrorString())));
            }

            //Wrong login -> logout
            unlink(true);
        }
        onGlobalSyncStateChanged(megaApi);
        break;
    }
    case MegaRequest::TYPE_LOGOUT:
    {
        int errorCode = e->getErrorCode();
        if (errorCode)
        {
            if (errorCode == MegaError::API_EINCOMPLETE && request->getParamType() == MegaError::API_ESSL)
            {
                if (!sslKeyPinningError)
                {
                    sslKeyPinningError = new QMessageBox(QMessageBox::Critical, QString::fromAscii("MEGAsync"),
                                                tr("MEGA is unable to connect securely through SSL. You might be on public WiFi with additional requirements.")
                                                + QString::fromUtf8(" (Issuer: %1)").arg(QString::fromUtf8(request->getText() ? request->getText() : "Unknown")),
                                                         QMessageBox::Retry | QMessageBox::Yes | QMessageBox::Cancel);
                    HighDpiResize hDpiResizer(sslKeyPinningError);

            //        TO-DO: Uncomment when asset is included to the project
            //        sslKeyPinningError->setIconPixmap(QPixmap(Utilities::getDevicePixelRatio() < 2 ? QString::fromUtf8(":/images/mbox-critical.png")
            //                                                    : QString::fromUtf8(":/images/mbox-critical@2x.png")));

                    sslKeyPinningError->setButtonText(QMessageBox::Yes, trUtf8("I don't care"));
                    sslKeyPinningError->setButtonText(QMessageBox::Cancel, trUtf8("Logout"));
                    sslKeyPinningError->setButtonText(QMessageBox::Retry, trUtf8("Retry"));
                    sslKeyPinningError->setDefaultButton(QMessageBox::Retry);
                    int result = sslKeyPinningError->exec();
                    if (!sslKeyPinningError)
                    {
                        return;
                    }

                    if (result == QMessageBox::Cancel)
                    {
                        // Logout
                        megaApi->localLogout();
                        delete sslKeyPinningError;
                        sslKeyPinningError = NULL;
                        return;
                    }
                    else if (result == QMessageBox::Retry)
                    {
                        // Retry
                        megaApi->retryPendingConnections();
                        delete sslKeyPinningError;
                        sslKeyPinningError = NULL;
                        return;
                    }

                    // Ignore
                    QPointer<ConfirmSSLexception> ex = new ConfirmSSLexception(sslKeyPinningError);
                    result = ex->exec();
                    if (!ex || !result)
                    {
                        megaApi->retryPendingConnections();
                        delete sslKeyPinningError;
                        sslKeyPinningError = NULL;
                        return;
                    }

                    if (ex->dontAskAgain())
                    {
                        preferences->setSSLcertificateException(true);
                    }

                    megaApi->setPublicKeyPinning(false);
                    megaApi->retryPendingConnections(true);
                    delete sslKeyPinningError;
                    sslKeyPinningError = NULL;
                }

                break;
            }

            if (errorCode == MegaError::API_ESID)
            {
                QMegaMessageBox::information(nullptr, QString::fromAscii("MEGAsync"), tr("You have been logged out on this computer from another location"));
            }
            else if (errorCode == MegaError::API_ESSL)
            {
                QMegaMessageBox::critical(nullptr, QString::fromAscii("MEGAsync"),
                                      tr("Our SSL key can't be verified. You could be affected by a man-in-the-middle attack or your antivirus software could be intercepting your communications and causing this problem. Please disable it and try again.")
                                       + QString::fromUtf8(" (Issuer: %1)").arg(QString::fromUtf8(request->getText() ? request->getText() : "Unknown")));
            }
            else if (errorCode != MegaError::API_EACCESS)
            {
                QMegaMessageBox::information(nullptr, QString::fromAscii("MEGAsync"), tr("You have been logged out because of this error: %1")
                                         .arg(QCoreApplication::translate("MegaError", e->getErrorString())));
            }
            unlink();
        }

        model->reset();

        if (preferences)
        {
            if (preferences->logged())
            {
                clearUserAttributes();
                preferences->unlink();
                removeAllFinishedTransfers();
                clearViewedTransfers();
                preferences->setFirstStartDone();
            }
            else
            {
                preferences->resetGlobalSettings();
            }

            closeDialogs();
            start();
            periodicTasks();
        }
        break;
    }
    case MegaRequest::TYPE_GET_LOCAL_SSL_CERT:
    {
        updatingSSLcert = false;
        bool retry = false;
        if (e->getErrorCode() == MegaError::API_OK)
        {
            MegaStringMap *data = request->getMegaStringMap();
            if (data)
            {
                preferences->setHttpsKey(QString::fromUtf8(data->get("key")));
                preferences->setHttpsCert(QString::fromUtf8(data->get("cert")));

                QString intermediates;
                QString key = QString::fromUtf8("intermediate_");
                const char *value;
                int i = 1;
                while ((value = data->get((key + QString::number(i)).toUtf8().constData())))
                {
                    if (i != 1)
                    {
                        intermediates.append(QString::fromUtf8(";"));
                    }
                    intermediates.append(QString::fromUtf8(value));
                    i++;
                }

                preferences->setHttpsCertIntermediate(intermediates);
                preferences->setHttpsCertExpiration(request->getNumber());
                megaApi->sendEvent(99517, "Local SSL certificate renewed");
                delete httpsServer;
                httpsServer = NULL;
                startHttpsServer();
                break;
            }
            else // Request aborted
            {
                retry=true;
            }
        }

        MegaApi::log(MegaApi::LOG_LEVEL_INFO, "Error renewing the local SSL certificate");
        if (e->getErrorCode() == MegaError::API_EACCESS || retry)
        {
            static bool retried = false;
            if (!retried)
            {
                retried = true;
                MegaApi::log(MegaApi::LOG_LEVEL_INFO, "Trying to renew the local SSL certificate again");
                renewLocalSSLcert();
                break;
            }
        }

        break;
    }
    case MegaRequest::TYPE_FETCH_NODES:
    {
        mFetchingNodes = false;
        if (e->getErrorCode() == MegaError::API_OK)
        {
            //Update/set root node
            getRootNode(true); //TODO: move this to thread pool

            preferences->setAccountStateInGeneral(Preferences::STATE_FETCHNODES_OK);
            preferences->setNeedsFetchNodesInGeneral(false);

            std::unique_ptr<char[]> email(megaApi->getMyEmail());
            bool logged = preferences->logged();
            bool firstTime = !logged && email && !preferences->hasEmail(QString::fromUtf8(email.get()));
            bool setupWizardContinues = false;
            if (!logged) //session resumed from general storage (or logged in via user/pass)
            {
                if (firstTime)
                {
                    showSetupWizard(SetupWizard::PAGE_MODE);
                    setupWizardContinues = true;
                }
                else
                {
                    preferences->setEmailAndGeneralSettings(QString::fromUtf8(email.get()));
                    model->rewriteSyncSettings(); //write sync settings into user's preferences
                    setupWizardFinished(QDialog::Accepted);
                }
            }

            if (!firstTime)
            {
                if (mRootNode)
                {
                    //If we have got the filesystem, start the app
                    loggedIn(false);


                    // onEvent with EVENT_BUSINESS_STATUS might have been received before logged, hence not written to cache yet.
                    // we fix that here:
                    auto cachedBusinessState = preferences->getBusinessState();
                    if (businessStatus != -2 && cachedBusinessState != businessStatus)
                    {
                        MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, QString::fromUtf8("cached business states %1 differs from applied businessStatus %2. Overriding cache")
                                     .arg(cachedBusinessState).arg(businessStatus).toUtf8().constData());
                        preferences->setBusinessState(businessStatus);
                    }

                    auto cachedBlockedState = preferences->getBlockedState();
                    if (blockStateSet && cachedBlockedState != blockState) // blockstate received and needs to be updated in cache
                    {
                        MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, QString::fromUtf8("cached blocked states %1 differs from applied blockedStatus %2. Overriding cache")
                                     .arg(cachedBlockedState).arg(blockState).toUtf8().constData());
                        preferences->setBlockedState(blockState);
                    }
                    else if (!blockStateSet && cachedBlockedState != -2 && cachedBlockedState) //block state not received in this execution, and cached says we were blocked last time
                    {
                        MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, QString::fromUtf8("cached blocked states %1 reports blocked, and no block state has been received before, lets query the block status")
                                     .arg(cachedBlockedState).toUtf8().constData());

                        whyAmIBlocked();// lets query again, to trigger transition and restoreSyncs
                    }

                    auto businessState = preferences->getBusinessState();
                    bool businessExpired = businessState == MegaApi::BUSINESS_STATUS_EXPIRED;
                    auto blockedState = preferences->getBlockedState();
                    bool accountBlocked = blockedState != -2 && blockedState;

                    //Restore temporarily disabled syncs for cases that don't have a transition that triggers restoreSyncs
                    if (!isAppliedStorageOverquota()
                            && !accountBlocked
                            && !businessExpired)
                    {
                        MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, QString::fromUtf8("no blocking situation after loggedIn").toUtf8().constData());
                    }
                }
                else
                {
                    QMegaMessageBox::warning(nullptr, tr("Error"), tr("Unable to get the filesystem.\n"
                                                           "Please, try again. If the problem persists "
                                                           "please contact bug@mega.co.nz"), QMessageBox::Ok);

                    setupWizardFinished(QDialog::Rejected);

                    preferences->setCrashed(true);

                    rebootApplication(false);
                }
            }

            if (!setupWizardContinues) //otherwise it needs to close
            {
                  emit closeSetupWizard(QDialog::Accepted);
            }
        }
        else
        {
            preferences->setAccountStateInGeneral(Preferences::STATE_FETCHNODES_FAILED);
            preferences->setNeedsFetchNodesInGeneral(true);
            MegaApi::log(MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("Error fetching nodes: %1")
                         .arg(QString::fromUtf8(e->getErrorString())).toUtf8().constData());
        }

        break;
    }
    case MegaRequest::TYPE_CHANGE_PW:
    {
        if (e->getErrorCode() == MegaError::API_OK)
        {
            QMegaMessageBox::information(nullptr, tr("Password changed"), tr("Your password has been changed."));
        }
        break;
    }
    case MegaRequest::TYPE_ACCOUNT_DETAILS:
    {
        bool storage = (request->getNumDetails() & 0x01) != 0;
        bool transfer = (request->getNumDetails() & 0x02) != 0;
        bool pro = (request->getNumDetails() & 0x04) != 0;

        if (storage)  inflightUserStats[0] = false;
        if (transfer) inflightUserStats[1] = false;
        if (pro)      inflightUserStats[2] = false;

        if (!preferences->logged())
        {
            break;
        }

        if (e->getErrorCode() != MegaError::API_OK)
        {
            break;
        }


        auto root = getRootNode();
        unique_ptr<MegaNode> inbox(megaApi->getInboxNode());
        unique_ptr<MegaNode> rubbish(megaApi->getRubbishNode());
        unique_ptr<MegaNodeList> inShares(megaApi->getInShares());

        if (!root || !inbox || !rubbish || !inShares)
        {
            preferences->setCrashed(true);
            break;
        }

        //Account details retrieved, update the preferences and the information dialog
        unique_ptr<MegaAccountDetails> details(request->getMegaAccountDetails());

        if (pro)
        {
            preferences->setAccountType(details->getProLevel());
            if (details->getProLevel() != Preferences::ACCOUNT_TYPE_FREE)
            {
                if (details->getProExpiration() && preferences->proExpirityTime() != details->getProExpiration())
                {
                    preferences->setProExpirityTime(details->getProExpiration());
                    proExpirityTimer.stop();
                    proExpirityTimer.setInterval(qMax(0LL, details->getProExpiration() * 1000 - QDateTime::currentMSecsSinceEpoch()));
                    proExpirityTimer.start();
                }
            }
            else
            {
                preferences->setProExpirityTime(0);
                proExpirityTimer.stop();
            }

            notifyAccountObservers();
        }

        if (storage)
        {
            preferences->setTotalStorage(details->getStorageMax());

            if (storageState == MegaApi::STORAGE_STATE_RED && receivedStorageSum < preferences->totalStorage())
            {
                megaApi->sendEvent(99525, "Red light does not match used storage");
                preferences->setUsedStorage(preferences->totalStorage());
            }
            else
            {
                preferences->setUsedStorage(receivedStorageSum);
            }

            MegaHandle rootHandle = root->getHandle();
            MegaHandle inboxHandle = inbox->getHandle();
            MegaHandle rubbishHandle = rubbish->getHandle();

            // For versions, match the webclient by only counting the user's own nodes.  Versions in inshares are not cleared by 'clear versions'
            // Also the no-parameter getVersionStorageUsed() double counts the versions in outshares.  Inshare storage count should include versions.
            preferences->setVersionsStorage(details->getVersionStorageUsed(rootHandle) 
                                          + details->getVersionStorageUsed(inboxHandle) 
                                          + details->getVersionStorageUsed(rubbishHandle));

            preferences->setCloudDriveStorage(details->getStorageUsed(rootHandle));
            preferences->setCloudDriveFiles(details->getNumFiles(rootHandle));
            preferences->setCloudDriveFolders(details->getNumFolders(rootHandle));

            preferences->setInboxStorage(details->getStorageUsed(inboxHandle));
            preferences->setInboxFiles(details->getNumFiles(inboxHandle));
            preferences->setInboxFolders(details->getNumFolders(inboxHandle));

            preferences->setRubbishStorage(details->getStorageUsed(rubbishHandle));
            preferences->setRubbishFiles(details->getNumFiles(rubbishHandle));
            preferences->setRubbishFolders(details->getNumFolders(rubbishHandle));

            long long inShareSize = 0, inShareFiles = 0, inShareFolders = 0;
            for (int i = 0; i < inShares->size(); i++)
            {
                MegaNode *node = inShares->get(i);
                if (!node)
                {
                    continue;
                }

                MegaHandle handle = node->getHandle();
                inShareSize += details->getStorageUsed(handle);
                inShareFiles += details->getNumFiles(handle);
                inShareFolders += details->getNumFolders(handle);
            }
            preferences->setInShareStorage(inShareSize);
            preferences->setInShareFiles(inShareFiles);
            preferences->setInShareFolders(inShareFolders);

            // update settings dialog if it exists, to show the correct versions size
            if (settingsDialog)
            {
                settingsDialog->storageChanged();
            }

            notifyStorageObservers();
        }

        const auto proUserIsNotOverquota{!megaApi->getBandwidthOverquotaDelay() &&
                    preferences->accountType() != Preferences::ACCOUNT_TYPE_FREE};
        if (proUserIsNotOverquota)
        {
            transferQuota->setQuotaOk();
        }

        if (transfer)
        {            
            preferences->setTotalBandwidth(details->getTransferMax());
            preferences->setBandwidthInterval(details->getTemporalBandwidthInterval());
            preferences->setUsedBandwidth(details->getTransferUsed());

            preferences->setTemporalBandwidthInterval(details->getTemporalBandwidthInterval());
            preferences->setTemporalBandwidth(details->getTemporalBandwidth());
            preferences->setTemporalBandwidthValid(details->isTemporalBandwidthValid());

            notifyBandwidthObservers();

            const auto userIsPro{preferences->accountType() != Preferences::ACCOUNT_TYPE_FREE};
            if(userIsPro)
            {
                transferQuota->setUserProUsages(preferences->usedBandwidth(), preferences->totalBandwidth());
            }
        }

        preferences->sync();

        if (infoDialog)
        {
            infoDialog->setUsage();
            infoDialog->setAccountType(preferences->accountType());
        }

        transferQuota->refreshOverQuotaDialogDetails();

        if (storageOverquotaDialog)
        {
            storageOverquotaDialog->refreshUsedStorage();
        }
        break;
    }
    case MegaRequest::TYPE_PAUSE_TRANSFERS:
    {
        bool paused = request->getFlag();
        switch (request->getNumber())
        {
            case MegaTransfer::TYPE_DOWNLOAD:
                preferences->setDownloadsPaused(paused);
                break;
            case MegaTransfer::TYPE_UPLOAD:
                preferences->setUploadsPaused(paused);
                break;
            default:
                preferences->setUploadsPaused(paused);
                preferences->setDownloadsPaused(paused);
                preferences->setGlobalPaused(paused);
                this->paused = paused;
                break;
        }
        if (preferences->getDownloadsPaused() == preferences->getUploadsPaused())
        {
            preferences->setGlobalPaused(paused);
            this->paused = paused;
        }
        else
        {
            preferences->setGlobalPaused(false);
            this->paused = false;
        }

        if (transferManager)
        {
            transferManager->updatePauseState();
        }

        if (infoDialog)
        {
            infoDialog->refreshTransferItems();
            infoDialog->updateDialogState();
        }

        onGlobalSyncStateChanged(megaApi);
        break;
    }
    case MegaRequest::TYPE_ADD_SYNC:
    {
        if (e->getErrorCode() == MegaError::API_EACCESS)
        {
            MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, QString::fromUtf8("Sync addition returns API_EACCESS").toUtf8().constData());
            megaApi->sendEvent(99531, "Sync addition fails with API_EACCESS"); //this would enforce a fetchNodes in the past
        }
        break;
    }
    case MegaRequest::TYPE_REMOVE_SYNC:
    {
        onGlobalSyncStateChanged(megaApi);
        break;
    }
    case MegaRequest::TYPE_GET_SESSION_TRANSFER_URL:
    {
        const char *url = request->getText();
        if (url && !memcmp(url, "pro", 3))
        {
            megaApi->sendEvent(99508, "Redirection to PRO");
        }

        QtConcurrent::run(QDesktopServices::openUrl, QUrl(QString::fromUtf8(request->getLink())));
        break;
    }
    case MegaRequest::TYPE_GET_PUBLIC_NODE:
    {
        MegaNode *node = NULL;
        QString link = QString::fromUtf8(request->getLink());
        QMap<QString, QString>::iterator it = pendingLinks.find(link);
        if (e->getErrorCode() == MegaError::API_OK)
        {
            node = request->getPublicMegaNode();
            if (node)
            {
                preferences->setLastPublicHandle(node->getHandle(), MegaApi::AFFILIATE_TYPE_FILE_FOLDER);
            }
        }

        if (it != pendingLinks.end())
        {
            QString auth = it.value();
            pendingLinks.erase(it);
            if (e->getErrorCode() == MegaError::API_OK && node)
            {
                if (auth.size())
                {
                    node->setPrivateAuth(auth.toUtf8().constData());
                }

                downloadQueue.append(node);
                processDownloads();
                break;
            }
            else if (e->getErrorCode() != MegaError::API_EBUSINESSPASTDUE)
            {
                showErrorMessage(tr("Error getting link information"));
            }
        }
        delete node;
        break;
    }
    case MegaRequest::TYPE_GET_PSA:
    {
        if (!preferences->logged())
        {
            break;
        }

        if (e->getErrorCode() == MegaError::API_OK)
        {
            if (infoDialog)
            {
                infoDialog->setPSAannouncement(request->getNumber(),
                                               QString::fromUtf8(request->getName() ? request->getName() : ""),
                                               QString::fromUtf8(request->getText() ? request->getText() : ""),
                                               QString::fromUtf8(request->getFile() ? request->getFile() : ""),
                                               QString::fromUtf8(request->getPassword() ? request->getPassword() : ""),
                                               QString::fromUtf8(request->getLink() ? request->getLink() : ""));
            }
        }

        break;
    }
    case MegaRequest::TYPE_WHY_AM_I_BLOCKED:
    {
        if (e->getErrorCode() == MegaError::API_OK
                && request->getNumber() == MegaApi::ACCOUNT_NOT_BLOCKED)
        {
            // if we received a block before nodes were fetch,
            // we want to try again now that we are no longer blocked
            if (!mFetchingNodes && !getRootNode())
            {
                fetchNodes();
                emit fetchNodesAfterBlock(); //so that guest widget notice and loads fetch noding page
            }

            blockState = MegaApi::ACCOUNT_NOT_BLOCKED;
            emit unblocked();
            blockStateSet = true;
            if (preferences->logged())
            {
                preferences->setBlockedState(blockState);
            }

            requestUserData(); // querying some user attributes might have been rejected: we query them again            

            MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, QString::fromUtf8("no longer blocked").toUtf8().constData());

            //in any case we reflect the change in the InfoDialog
            if (infoDialog)
            {
                infoDialog->regenerateLayout(MegaApi::ACCOUNT_NOT_BLOCKED);
            }

            if (settingsDialog)
            {
                settingsDialog->setProxyOnly(false);
            }
        }

        mQueringWhyAmIBlocked = false;
        break;
    }
    case MegaRequest::TYPE_GET_USER_DATA:
    {
        if (e->getErrorCode() == MegaError::API_OK)
        {
            getUserDataRequestReady = true;
            checkOverStorageStates();
        }
        break;
    }
    case MegaRequest::TYPE_SEND_EVENT:
    {
        switch (request->getNumber())
        {
            case 99500:
                preferences->setFirstStartDone();
                break;
            case 99501:
                preferences->setFirstSyncDone();
                break;
            case 99502:
                preferences->setFirstFileSynced();
                break;
            case 99503:
                preferences->setFirstWebDownloadDone();
                break;
            default:
                break;
        }
    }
    default:
        break;
    }
}

//Called when a transfer is about to start
void MegaApplication::onTransferStart(MegaApi *api, MegaTransfer *transfer)
{
    if (appfinished || transfer->isStreamingTransfer() || transfer->isFolderTransfer())
    {
        return;
    }

    DeferPreferencesSyncForScope deferrer(this);

    if (transfer->getType() == MegaTransfer::TYPE_DOWNLOAD)
    {
        HTTPServer::onTransferDataUpdate(transfer->getNodeHandle(),
                                             transfer->getState(),
                                             transfer->getTransferredBytes(),
                                             transfer->getTotalBytes(),
                                             transfer->getSpeed(),
                                             QString::fromUtf8(transfer->getPath()));
    }

    if (transferManager)
    {
        transferManager->onTransferStart(megaApi, transfer);
    }
    if (infoDialog)
    {
        infoDialog->onTransferStart(megaApi, transfer);
    }


    onTransferUpdate(api, transfer);
    if (!numTransfers[MegaTransfer::TYPE_DOWNLOAD]
            && !numTransfers[MegaTransfer::TYPE_UPLOAD])
    {
        onGlobalSyncStateChanged(megaApi);
    }
    numTransfers[transfer->getType()]++;
}

//Called when there is a temporal problem in a request
void MegaApplication::onRequestTemporaryError(MegaApi *, MegaRequest *, MegaError* )
{
}

//Called when a transfer has finished
void MegaApplication::onTransferFinish(MegaApi* , MegaTransfer *transfer, MegaError* e)
{
    if (appfinished || transfer->isStreamingTransfer())
    {
        return;
    }

    DeferPreferencesSyncForScope deferrer(this);

    // check if it's a top level transfer
    int folderTransferTag = transfer->getFolderTransferTag();
    if (folderTransferTag == 0 // file transfer
            || folderTransferTag == -1) // folder transfer
    {
        const char *notificationKey = transfer->getAppData();
        if (notificationKey)
        {
            char *endptr;
            unsigned long long notificationId = strtoll(notificationKey, &endptr, 10);
            QHash<unsigned long long, TransferMetaData*>::iterator it
                   = transferAppData.find(notificationId);
            if (it != transferAppData.end())
            {
                TransferMetaData *data = it.value();
                if ((endptr - notificationKey) != (int64_t)strlen(notificationKey))
                {
                    if (e->getErrorCode() == MegaError::API_EINCOMPLETE)
                    {
                        data->transfersCancelled++;
                    }
                    else if (e->getErrorCode() != MegaError::API_OK)
                    {
                        data->transfersFailed++;
                    }
                    else
                    {
                        !folderTransferTag ? data->transfersFileOK++ : data->transfersFolderOK++;
                    }
                }

                data->pendingTransfers--;
                showNotificationFinishedTransfers(notificationId);
            }
        }
    }

    if (transfer->isFolderTransfer())
    {
        if (e->getErrorCode() != MegaError::API_OK)
        {
            showErrorMessage(tr("Error transferring folder: ") + QString::fromUtf8(" ") + QCoreApplication::translate("MegaError", MegaError::getErrorString(e->getErrorCode(), MegaError::API_EC_UPLOAD)));
        }

        return;
    }

    if (transfer->getState() == MegaTransfer::STATE_COMPLETED || transfer->getState() == MegaTransfer::STATE_FAILED)
    {
        MegaTransfer *t = transfer->copy();
        if (finishedTransfers.count(transfer->getTag()))
        {
            assert(false);
            megaApi->sendEvent(99512, QString::fromUtf8("Duplicated finished transfer: %1").arg(QString::number(transfer->getTag())).toUtf8().constData());
            removeFinishedTransfer(transfer->getTag());
        }

        finishedTransfers.insert(transfer->getTag(), t);
        finishedTransferOrder.push_back(t);

        if (!transferManager)
        {
            completedTabActive = false;
        }

        if (!completedTabActive)
        {
            ++nUnviewedTransfers;
        }

        if (transferManager)
        {
            transferManager->updateNumberOfCompletedTransfers(nUnviewedTransfers);
        }
    }

    if (transfer->getType() == MegaTransfer::TYPE_DOWNLOAD)
    {
        HTTPServer::onTransferDataUpdate(transfer->getNodeHandle(),
                                             transfer->getState(),
                                             transfer->getTransferredBytes(),
                                             transfer->getTotalBytes(),
                                             transfer->getSpeed(),
                                             QString::fromUtf8(transfer->getPath()));
    }

    if (blockState)
    {
        finishedBlockedTransfers.insert(transfer->getTag());
    }

    if (transferManager)
    {
        transferManager->onTransferFinish(megaApi, transfer, e);
    }

    if (infoDialog)
    {
        infoDialog->onTransferFinish(megaApi, transfer, e);
    }

    if (finishedTransferOrder.size() > (int)Preferences::MAX_COMPLETED_ITEMS)
    {
        removeFinishedTransfer(finishedTransferOrder.first()->getTag());
    }

    if (e->getErrorCode() == MegaError::API_EBUSINESSPASTDUE
            && (!lastTsBusinessWarning || (QDateTime::currentMSecsSinceEpoch() - lastTsBusinessWarning) > 3000))//Notify only once within last five seconds
    {
        lastTsBusinessWarning = QDateTime::currentMSecsSinceEpoch();
        sendBusinessWarningNotification();
    }

    //Show the transfer in the "recently updated" list
    if (e->getErrorCode() == MegaError::API_OK && transfer->getNodeHandle() != INVALID_HANDLE)
    {
        QString localPath;
        if (transfer->getPath())
        {
            localPath = QString::fromUtf8(transfer->getPath());
        }

#ifdef WIN32
        if (localPath.startsWith(QString::fromAscii("\\\\?\\")))
        {
            localPath = localPath.mid(4);
        }
#endif

        MegaNode *node = transfer->getPublicMegaNode();
        QString publicKey;
        if (node)
        {
            const char* key = node->getBase64Key();
            publicKey = QString::fromUtf8(key);
            delete [] key;
            delete node;
        }

        addRecentFile(QString::fromUtf8(transfer->getFileName()), transfer->getNodeHandle(), localPath, publicKey);
    }

    if (e->getErrorCode() == MegaError::API_OK
            && transfer->isSyncTransfer()
            && !isFirstFileSynced
            && !preferences->isFirstFileSynced())
    {
        megaApi->sendEvent(99502, "MEGAsync first synced file");
        isFirstFileSynced = true;
    }

    int type = transfer->getType();
    numTransfers[type]--;

    unsigned long long priority = transfer->getPriority();
    if (!priority)
    {
        priority = 0xFFFFFFFFFFFFFFFFULL;
    }
    if (priority <= activeTransferPriority[type]
            || activeTransferState[type] == MegaTransfer::STATE_PAUSED
            || transfer->getTag() == activeTransferTag[type])
    {
        activeTransferPriority[type] = 0xFFFFFFFFFFFFFFFFULL;
        activeTransferState[type] = MegaTransfer::STATE_NONE;
        activeTransferTag[type] = 0;

        //Send updated statics to the information dialog
        if (infoDialog)
        {
            infoDialog->setTransfer(transfer);
            infoDialog->transferFinished(e->getErrorCode());
            infoDialog->updateDialogState();
        }

        if (!firstTransferTimer->isActive())
        {
            firstTransferTimer->start();
        }
    }

    //If there are no pending transfers, reset the statics and update the state of the tray icon
    if (!numTransfers[MegaTransfer::TYPE_DOWNLOAD]
            && !numTransfers[MegaTransfer::TYPE_UPLOAD])
    {
        onGlobalSyncStateChanged(megaApi);
    }
}

//Called when a transfer has been updated
void MegaApplication::onTransferUpdate(MegaApi *, MegaTransfer *transfer)
{
    if (appfinished || transfer->isStreamingTransfer() || transfer->isFolderTransfer())
    {
        return;
    }

    DeferPreferencesSyncForScope deferrer(this);

    if (transferManager)
    {
        transferManager->onTransferUpdate(megaApi, transfer);
    }

    if (infoDialog)
    {
        infoDialog->onTransferUpdate(megaApi, transfer);
    }

    int type = transfer->getType();
    if (type == MegaTransfer::TYPE_DOWNLOAD)
    {
        HTTPServer::onTransferDataUpdate(transfer->getNodeHandle(),
                                             transfer->getState(),
                                             transfer->getTransferredBytes(),
                                             transfer->getTotalBytes(),
                                             transfer->getSpeed(),
                                             QString::fromUtf8(transfer->getPath()));
    }

    unsigned long long priority = transfer->getPriority();
    if (!priority)
    {
        priority = 0xFFFFFFFFFFFFFFFFULL;
    }
    if (priority <= activeTransferPriority[type]
            || activeTransferState[type] == MegaTransfer::STATE_PAUSED)
    {
        activeTransferPriority[type] = priority;
        activeTransferState[type] = transfer->getState();
        activeTransferTag[type] = transfer->getTag();

        if (infoDialog)
        {
            infoDialog->setTransfer(transfer);
        }
    }
    else if (activeTransferTag[type] == transfer->getTag())
    {
        // First transfer moved to a lower priority
        activeTransferPriority[type] = 0xFFFFFFFFFFFFFFFFULL;
        activeTransferState[type] = MegaTransfer::STATE_NONE;
        activeTransferTag[type] = 0;
        if (!firstTransferTimer->isActive())
        {
            firstTransferTimer->start();
        }
    }
}

void MegaApplication::onCheckDeferredPreferencesSyncTimeout()
{
    onCheckDeferredPreferencesSync(true);
}

void MegaApplication::onCheckDeferredPreferencesSync(bool timeout)
{
    if (appfinished)
    {
        return;
    }

    // don't execute too often or the dialog locks up, eg. queueing a folder with 1k items for upload/download
    if (timeout)
    {
        onDeferredPreferencesSyncTimer.reset();
        if (preferences->needsDeferredSync())
        {
            preferences->sync();
        }
    }
    else
    {
        if (!onDeferredPreferencesSyncTimer)
        {
            onDeferredPreferencesSyncTimer.reset(new QTimer(this));
            connect(onDeferredPreferencesSyncTimer.get(), SIGNAL(timeout()), this, SLOT(onCheckDeferredPreferencesSyncTimeout()));

            onDeferredPreferencesSyncTimer->setSingleShot(true);
            onDeferredPreferencesSyncTimer->setInterval(100);
            onDeferredPreferencesSyncTimer->start();
        }
    }
}

void MegaApplication::showAddSyncError(MegaRequest *request, MegaError* e, QString localpath, QString remotePath)
{
    if (e->getErrorCode() != MegaError::API_OK)
    {
        showAddSyncError(request->getNumDetails(), localpath, remotePath);
    }
}

void MegaApplication::showAddSyncError(int errorCode, QString localpath, QString remotePath)
{
    if (errorCode != MegaError::API_OK)
    {
        QMegaMessageBox::critical(nullptr, tr("Error adding sync"),
                                  tr("This sync can't be added: %1. Reason: %2").arg(localpath)
                                  .arg( errorCode > 0 ? QCoreApplication::translate("MegaSyncError", MegaSync::getMegaSyncErrorCode(errorCode))
                                                      : QCoreApplication::translate("MegaError", MegaError::getErrorString(errorCode))));
    }
}

//Called when there is a temporal problem in a transfer
void MegaApplication::onTransferTemporaryError(MegaApi *api, MegaTransfer *transfer, MegaError* e)
{
    if (appfinished)
    {
        return;
    }

    DeferPreferencesSyncForScope deferrer(this);

    if (transferManager)
    {
        transferManager->onTransferTemporaryError(megaApi, transfer, e);
    }

    onTransferUpdate(api, transfer);
    preferences->setTransferDownloadMethod(api->getDownloadMethod());
    preferences->setTransferUploadMethod(api->getUploadMethod());

    if (e->getErrorCode() == MegaError::API_EOVERQUOTA)
    {
        if (transfer->isForeignOverquota())
        {
            MegaUser *contact =  megaApi->getUserFromInShare(megaApi->getNodeByHandle(transfer->getParentHandle()), true);
            showErrorMessage(tr("Your upload(s) cannot proceed because %1's account is full")
                             .arg(contact?QString::fromUtf8(contact->getEmail()):tr("contact")));

        }
        else if (e->getValue() && !transferQuota->isOverQuota())
        {
            const auto waitTime{std::chrono::seconds(e->getValue())};
            preferences->clearTemporalBandwidth();
            megaApi->getPricing();
            updateUserStats(false, true, true, true, USERSTATS_TRANSFERTEMPERROR);  // get udpated transfer quota (also pro status in case out of quota is due to account paid period expiry)
            transferQuota->setOverQuota(waitTime);
        }
    }
}

void MegaApplication::onAccountUpdate(MegaApi *)
{
    if (appfinished || !preferences->logged())
    {
        return;
    }

    preferences->clearTemporalBandwidth();
    transferQuota->refreshOverQuotaDialogDetails();
    updateUserStats(true, true, true, true, USERSTATS_ACCOUNTUPDATE);
}


bool MegaApplication::notificationsAreFiltered()
{
    return notificationsProxyModel && notificationsProxyModel->filterAlertType() != QFilterAlertsModel::NO_FILTER;
}

bool MegaApplication::hasNotifications()
{
    return notificationsModel && notificationsModel->rowCount(QModelIndex());
}

bool MegaApplication::hasNotificationsOfType(int type)
{
    return notificationsModel && notificationsModel->existsNotifications(type);
}

MegaSyncLogger& MegaApplication::getLogger() const
{
    return *logger;
}

void MegaApplication::onUserAlertsUpdate(MegaApi *api, MegaUserAlertList *list)
{
    if (appfinished)
    {
        return;
    }

    bool copyRequired = true;
    if (!list)//User alerts already loaded: get the list from MegaApi::getUserAlerts
    {
        list = megaApi->getUserAlerts();
        copyRequired = false;
    }
    else
    {
        assert(notificationsModel && "onUserAlertsUpdate with !alerts should have happened before!");
    }

    if (!notificationsModel)
    {
        notificationsModel = new QAlertsModel(list, copyRequired);
        notificationsProxyModel = new QFilterAlertsModel();
        notificationsProxyModel->setSourceModel(notificationsModel);
        notificationsProxyModel->setSortRole(Qt::UserRole); //Role used to sort the model by date.

        notificationsDelegate = new MegaAlertDelegate(notificationsModel, true, this);

        if (infoDialog)
        {
            infoDialog->updateNotificationsTreeView(notificationsProxyModel, notificationsDelegate);
        }
    }
    else
    {
        notificationsModel->insertAlerts(list, copyRequired);
    }

    if (infoDialog)
    {
        infoDialog->setUnseenNotifications(notificationsModel->getUnseenNotifications(QAlertsModel::ALERT_ALL));
        infoDialog->setUnseenTypeNotifications(notificationsModel->getUnseenNotifications(QAlertsModel::ALERT_ALL),
                                           notificationsModel->getUnseenNotifications(QAlertsModel::ALERT_CONTACTS),
                                           notificationsModel->getUnseenNotifications(QAlertsModel::ALERT_SHARES),
                                           notificationsModel->getUnseenNotifications(QAlertsModel::ALERT_PAYMENT));
    }

    if (!copyRequired)
    {
        list->clear(); //empty the list otherwise they will be deleted
        delete list;
    }
}


//Called when contacts have been updated in MEGA
void MegaApplication::onUsersUpdate(MegaApi *, MegaUserList *userList)
{
    if (appfinished || !infoDialog || !userList || !preferences->logged())
    {
        return;
    }

    DeferPreferencesSyncForScope deferrer(this);

    MegaHandle myHandle = megaApi->getMyUserHandleBinary();
    for (int i = 0; i < userList->size(); i++)
    {
        MegaUser *user = userList->get(i);
        if (!user->isOwnChange() && user->getHandle() == myHandle)
        {
            if (user->hasChanged(MegaUser::CHANGE_TYPE_FIRSTNAME))
            {
                megaApi->getUserAttribute(MegaApi::USER_ATTR_FIRSTNAME);
            }

            if (user->hasChanged(MegaUser::CHANGE_TYPE_LASTNAME))
            {
                megaApi->getUserAttribute(MegaApi::USER_ATTR_LASTNAME);
            }

            if (user->hasChanged(MegaUser::CHANGE_TYPE_AVATAR))
            {
                const char* email = megaApi->getMyEmail();
                if (email)
                {
                    megaApi->getUserAvatar(Utilities::getAvatarPath(QString::fromUtf8(email)).toUtf8().constData());
                    delete [] email;
                }
            }

            if (user->hasChanged(MegaUser::CHANGE_TYPE_DISABLE_VERSIONS))
            {
                megaApi->getFileVersionsOption();
            }
            break;
        }
    }
}

//Called when nodes have been updated in MEGA
void MegaApplication::onNodesUpdate(MegaApi* , MegaNodeList *nodes)
{
    if (appfinished || !infoDialog || !nodes || !preferences->logged())
    {
        return;
    }

    DeferPreferencesSyncForScope deferrer(this);

    MegaApi::log(MegaApi::LOG_LEVEL_INFO, QString::fromUtf8("%1 updated files/folders").arg(nodes->size()).toUtf8().constData());

    //Check all modified nodes
    QString localPath;
    for (int i = 0; i < nodes->size(); i++)
    {
        localPath.clear();
        MegaNode *node = nodes->get(i);
        if (node->getChanges() & MegaNode::CHANGE_TYPE_PARENT)
        {
            emit nodeMoved(node->getHandle());
        }
        if (node->getChanges() & MegaNode::CHANGE_TYPE_ATTRIBUTES)
        {
            emit nodeAttributesChanged(node->getHandle());
        }

        if (!node->isRemoved() && node->getTag()
                && !node->isSyncDeleted()
                && (node->getType() == MegaNode::TYPE_FILE)
                && node->getAttrString()->size())
        {
            //NO_KEY node created by this client detected
            if (!noKeyDetected)
            {
                if (megaApi->isLoggedIn())
                {
                    fetchNodes();
                }
            }
            else if (noKeyDetected > 20)
            {
                QMegaMessageBox::critical(nullptr, QString::fromUtf8("MEGAsync"),
                    QString::fromUtf8("Something went wrong. MEGAsync will restart now. If the problem persists please contact bug@mega.co.nz"));
                preferences->setCrashed(true);
                rebootApplication(false);
            }
            noKeyDetected++;
        }
    }
}

void MegaApplication::onReloadNeeded(MegaApi*)
{
    if (appfinished)
    {
        return;
    }

    //Don't reload the filesystem here because it's unsafe
    //and the most probable cause for this callback is a false positive.
    //Simply set the crashed flag to force a filesystem reload in the next execution.
    preferences->setCrashed(true);
}

void MegaApplication::onGlobalSyncStateChangedTimeout()
{
    onGlobalSyncStateChanged(NULL, true);
}

void MegaApplication::onGlobalSyncStateChanged(MegaApi *, bool timeout)
{
    if (appfinished)
    {
        return;
    }

    // don't execute too often or the dialog locks up, eg. queueing a folder with 1k items for upload/download
    if (timeout)
    {
        onGlobalSyncStateChangedTimer.reset();
    }
    else 
    {
        if (!onGlobalSyncStateChangedTimer)
        {
            onGlobalSyncStateChangedTimer.reset(new QTimer(this));
            connect(onGlobalSyncStateChangedTimer.get(), SIGNAL(timeout()), this, SLOT(onGlobalSyncStateChangedTimeout()));

            onGlobalSyncStateChangedTimer->setSingleShot(true);
            onGlobalSyncStateChangedTimer->setInterval(200);
            onGlobalSyncStateChangedTimer->start();
        }
        return;
    }

    if (megaApi && infoDialog)
    {
        indexing = megaApi->isScanning();
        waiting = megaApi->isWaiting();
        syncing = megaApi->isSyncing();
        transferring = megaApi->getNumPendingUploads() || megaApi->getNumPendingDownloads();

        int pendingUploads = megaApi->getNumPendingUploads();
        int pendingDownloads = megaApi->getNumPendingDownloads();
        if (pendingUploads)
        {
            MegaApi::log(MegaApi::LOG_LEVEL_INFO, QString::fromUtf8("Pending uploads: %1").arg(pendingUploads).toUtf8().constData());
        }

        if (pendingDownloads)
        {
            MegaApi::log(MegaApi::LOG_LEVEL_INFO, QString::fromUtf8("Pending downloads: %1").arg(pendingDownloads).toUtf8().constData());
        }

        infoDialog->setIndexing(indexing);
        infoDialog->setWaiting(waiting);
        infoDialog->setSyncing(syncing);
        infoDialog->setTransferring(transferring);
        infoDialog->updateDialogState();
        infoDialog->transferFinished(MegaError::API_OK);
    }

    if (transferManager)
    {
        transferManager->updateState();
    }

    MegaApi::log(MegaApi::LOG_LEVEL_INFO, QString::fromUtf8("Current state. Paused = %1 Indexing = %2 Waiting = %3 Syncing = %4")
                 .arg(paused).arg(indexing).arg(waiting).arg(syncing).toUtf8().constData());

    updateTrayIcon();
}

void MegaApplication::onSyncStateChanged(MegaApi *api, MegaSync *sync)
{
    if (appfinished)
    {
        return;
    }

    model->updateSyncSettings(sync); //Note, we are not updating the remote sync path
    // we asume that cannot change for existing syncs.

    onGlobalSyncStateChanged(api);
}

void MegaApplication::onSyncFileStateChanged(MegaApi *, MegaSync *, string *localPath, int newState)
{
    if (appfinished)
    {
        return;
    }

    DeferPreferencesSyncForScope deferrer(this);

    Platform::notifyItemChange(localPath, newState);
}

void MegaApplication::onSyncDisabled(std::shared_ptr<SyncSetting> syncSetting, bool newSync)
{
    if (!syncSetting)
    {
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("onSyncDisabled for non existing sync").toUtf8().constData());
        return;
    }

    MegaApi::log(MegaApi::LOG_LEVEL_WARNING, QString::fromUtf8("Your sync \"%1\" has been disabled. State = %2. Error = %3")
                 .arg(syncSetting->name()).arg(syncSetting->getState()).arg(syncSetting->getError()).toUtf8().constData());

    if (syncSetting->isTemporaryDisabled() && syncSetting->getError() != MegaSync::Error::LOGGED_OUT)
    {
        showErrorMessage(tr("Your sync \"%1\" has been temporarily disabled").arg(syncSetting->name()).append(QString::fromUtf8(": "))
                         .append(QCoreApplication::translate("MegaSyncError", MegaSync::getMegaSyncErrorCode(syncSetting->getError()))));
    }

    if (syncSetting->getState() == MegaSync::SYNC_FAILED)
    {
        switch(syncSetting->getError())
        {
        case MegaSync::Error::NO_SYNC_ERROR:
        {
            assert(false && "unexpected no error after onSyncAdded failed");
            return;
        }
        case MegaSync::Error::LOCAL_PATH_UNAVAILABLE:
        {
            showErrorMessage(tr("Your sync \"%1\" has been disabled because the local folder doesn't exist")
                             .arg(syncSetting->name()));

            break;
        }
        case MegaSync::Error::REMOTE_PATH_DELETED:
        {
            showErrorMessage(tr("Your sync \"%1\" has been disabled because the remote folder doesn't exist")
                            .arg(syncSetting->name()));
            break;
        }
        case MegaSync::Error::VBOXSHAREDFOLDER_UNSUPPORTED:
            showErrorMessage(tr("Your sync \"%1\" has been disabled because the synchronization of VirtualBox shared folders is not supported due to deficiencies in that filesystem.")
                            .arg(syncSetting->name()));
            break;
        case MegaSync::Error::REMOTE_NODE_MOVED_TO_RUBBISH:
            showErrorMessage(tr("Your sync \"%1\" has been disabled because the remote folder is in the rubbish bin")
                            .arg(syncSetting->name()));
            break;
        case MegaSync::Error::SHARE_NON_FULL_ACCESS:
            showErrorMessage(tr("Your sync \"%1\" has been disabled. The remote folder (or part of it) doesn't have full access")
                             .arg(syncSetting->name()));
            break;
        case MegaSync::Error::LOCAL_FINGERPRINT_MISMATCH:
            showErrorMessage(tr("Your sync \"%1\" has been disabled because the local folder has changed")
                            .arg(syncSetting->name()));
            break;
        case MegaSync::Error::PUT_NODES_ERROR:
        default:
        {
            showErrorMessage(tr("Your sync \"%1\" has been disabled. Reason: %2").arg(syncSetting->name())
                             .arg(QCoreApplication::translate("MegaSyncError", MegaSync::getMegaSyncErrorCode(syncSetting->getError()))));
            break;
        }
        }
        if (newSync) //TODO: note for reviewer: this may need to be opened always
        {
            openSettings(SettingsDialog::SYNCS_TAB);
        }
    }
}

void MegaApplication::onSyncDisabled(MegaApi *api, MegaSync *sync)
{
    if (appfinished || !sync)
    {
        return;
    }

    onSyncDisabled(model->getSyncSettingByTag(sync->getTag()));
}

void MegaApplication::onSyncEnabled(std::shared_ptr<SyncSetting> syncSetting)
{
    if (!syncSetting)
    {
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("onSyncEnabled for non existing sync").toUtf8().constData());
        return;
    }

    MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, tr("Your sync \"%1\" has been re-enabled. State = %2. Errror = %3")
                 .arg(syncSetting->name()).arg(syncSetting->getState()).arg(syncSetting->getError()).toUtf8().constData());


    showErrorMessage(tr("Your sync \"%1\" has been enabled")
                     .arg(syncSetting->name()));
}

void MegaApplication::onSyncEnabled(MegaApi *api, MegaSync *sync)
{
    if (appfinished || !sync)
    {
        return;
    }

    onSyncEnabled(model->getSyncSettingByTag(sync->getTag()));
}

void MegaApplication::onSyncAdded(MegaApi *api, MegaSync *sync, int additionState)
{
    if (appfinished || !sync)
    {
        return;
    }

    auto syncSetting = model->updateSyncSettings(sync, additionState);

    if (additionState == MegaSync::SyncAdded::FROM_CACHE_FAILED_TO_RESUME
            || additionState == MegaSync::SyncAdded::NEW_TEMP_DISABLED)
    {
        onSyncDisabled(syncSetting);
    }

    if (settingsDialog)
    {
        settingsDialog->loadSyncSettings();
    }

    onGlobalSyncStateChanged(api);
}

void MegaApplication::onSyncDeleted(MegaApi *api, MegaSync *sync)
{
    if (appfinished || !sync)
    {
        return;
    }

    model->removeSyncedFolderByTag(sync->getTag());

    onGlobalSyncStateChanged(api);
}

MEGASyncDelegateListener::MEGASyncDelegateListener(MegaApi *megaApi, MegaListener *parent, MegaApplication *app)
    : QTMegaListener(megaApi, parent)
{
    this->app = app;
}

void MEGASyncDelegateListener::onRequestFinish(MegaApi *api, MegaRequest *request, MegaError *e)
{
    QTMegaListener::onRequestFinish(api, request, e);

    if (request->getType() != MegaRequest::TYPE_FETCH_NODES
            || e->getErrorCode() != MegaError::API_OK)
    {
        return;
    }
}

void MEGASyncDelegateListener::onEvent(MegaApi *api, MegaEvent *e)
{
    if (e->getType() == MegaEvent::EVENT_FIRST_SYNC_RESUMING )
    {
        Utilities::delayFirstSyncStart();
    }
    QTMegaListener::onEvent(api, e);
}

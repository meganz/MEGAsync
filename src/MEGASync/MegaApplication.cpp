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

#include <QTranslator>
#include <QClipboard>
#include <QDesktopWidget>
#include <QFontDatabase>
#include <QNetworkProxy>
#include <assert.h>

#ifdef Q_OS_LINUX
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
#endif

using namespace mega;
using namespace std;

QString MegaApplication::appPath = QString();
QString MegaApplication::appDirPath = QString();
QString MegaApplication::dataPath = QString();

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

int main(int argc, char *argv[])
{
    // adds thread-safety to OpenSSL
    QSslSocket::supportsSsl();

#ifdef _WIN32
    HINSTANCE shcore = NULL;
    WCHAR systemPath[MAX_PATH];
    UINT len = GetSystemDirectory(systemPath, MAX_PATH);
    if (len + 20 >= MAX_PATH)
    {
        shcore = LoadLibrary(L"shcore.dll");
    }
    else
    {
        StringCchPrintfW(systemPath + len, MAX_PATH - len, L"\\shcore.dll");
        shcore = LoadLibrary(systemPath);
    }

    if (shcore)
    {
        enum MEGA_PROCESS_DPI_AWARENESS
        {
            MEGA_PROCESS_DPI_UNAWARE            = 0,
            MEGA_PROCESS_SYSTEM_DPI_AWARE       = 1,
            MEGA_PROCESS_PER_MONITOR_DPI_AWARE  = 2
        };
        typedef HRESULT (WINAPI* MEGA_SetProcessDpiAwarenessType)(MEGA_PROCESS_DPI_AWARENESS);
        MEGA_SetProcessDpiAwarenessType MEGA_SetProcessDpiAwareness = reinterpret_cast<MEGA_SetProcessDpiAwarenessType>(GetProcAddress(shcore, "SetProcessDpiAwareness"));
        if (MEGA_SetProcessDpiAwareness)
        {
            MEGA_SetProcessDpiAwareness(MEGA_PROCESS_DPI_UNAWARE);
        }
        FreeLibrary(shcore);
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

#ifdef Q_OS_LINUX
#if QT_VERSION >= 0x050600
    qreal ratio = 1;
    int xrdbdpi = 0;
    if (!(getenv("DO_NOT_OVERRIDE_XDG_CURRENT_DESKTOP")))
    {
        if (getenv("XDG_CURRENT_DESKTOP") && !strcmp(getenv("XDG_CURRENT_DESKTOP"),"KDE") && (!getenv("XDG_SESSION_TYPE") || strcmp(getenv("XDG_SESSION_TYPE"),"wayland") ) )
        {
            qputenv("XDG_CURRENT_DESKTOP","GNOME");
        }
    }
    if (!getenv("QT_SCALE_FACTOR"))
    {
        QProcess p;
        p.start(QString::fromUtf8("bash -c \"xrdb -query | grep dpi | awk '{print $2}'\""));
        p.waitForFinished(2000);
        QString output = QString::fromUtf8(p.readAllStandardOutput().constData()).trimmed();
        QString e = QString::fromUtf8(p.readAllStandardError().constData());
        if (e.size())
        {
            MegaApi::log(MegaApi::LOG_LEVEL_ERROR, "Error for \"xrdb -query\" command:");
            MegaApi::log(MegaApi::LOG_LEVEL_ERROR, e.toUtf8().constData());
        }

        xrdbdpi = output.toInt();
        if ( xrdbdpi > 96)
        {
            ratio = output.toDouble()/96.0;
            if (ratio > 3)
            {
                ratio = 3;
            }
        }
        else
        {
            MegaApplication appaux(argc,argv); //needed to get geometry (it needs to be instantiated a second time to actually use scale factor)
            QRect geom = appaux.desktop()->availableGeometry(QCursor::pos());
            ratio = min(geom.width()/(1920.0),geom.height()/(1080.0))*0.75;
            ratio = max(1.0,ratio);
        }

        qputenv("QT_SCALE_FACTOR", QString::number(ratio).toUtf8());
    }
#endif

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

#if defined(Q_OS_LINUX) && QT_VERSION >= 0x050600
    MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, QString::fromUtf8("QT_SCALE_FACTOR = %1").arg(QString::fromUtf8(getenv("QT_SCALE_FACTOR"))).toUtf8().constData() );
    MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, QString::fromUtf8("xrdb dpi read = %1").arg(QString::number(xrdbdpi)).toUtf8().constData() );

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
    if ((argc == 2) && !strcmp("/uninstall", argv[1]))
    {
        Preferences *preferences = Preferences::instance();
        preferences->initialize(app.applicationDataPath());
        if (!preferences->error())
        {
            if (preferences->logged())
            {
                preferences->unlink();
            }

            for (int i = 0; i < preferences->getNumUsers(); i++)
            {
                preferences->enterUser(i);
                for (int j = 0; j < preferences->getNumSyncedFolders(); j++)
                {
                    Platform::syncFolderRemoved(preferences->getLocalFolder(j),
                                                preferences->getSyncName(j),
                                                preferences->getSyncID(j));

                    #ifdef WIN32
                        QString debrisPath = QDir::toNativeSeparators(preferences->getLocalFolder(j) +
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

    QtLockedFile singleInstanceChecker(appLockPath);
    bool alreadyStarted = true;
    for (int i = 0; i < 10; i++)
    {
        singleInstanceChecker.open(QtLockedFile::ReadWrite);
        if (singleInstanceChecker.lock(QtLockedFile::WriteLock, false))
        {
            alreadyStarted = false;
            break;
        }
        else if (i == 0)
        {
             QString appShowInterfacePath = dataDir.filePath(QString::fromAscii("megasync.show"));
             QFile fappShowInterfacePath(appShowInterfacePath);
             if (fappShowInterfacePath.open(QIODevice::WriteOnly))
             {
                 fappShowInterfacePath.close();
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

    app.initialize();
    app.start();
    return app.exec();

#if 0 //Strings for the translation system. These lines don't need to be built
    QT_TRANSLATE_NOOP("QDialogButtonBox", "&Yes");
    QT_TRANSLATE_NOOP("QDialogButtonBox", "&No");
    QT_TRANSLATE_NOOP("QDialogButtonBox", "&OK");
    QT_TRANSLATE_NOOP("QDialogButtonBox", "&Cancel");
    QT_TRANSLATE_NOOP("QPlatformTheme", "&Yes");
    QT_TRANSLATE_NOOP("QPlatformTheme", "&No");
    QT_TRANSLATE_NOOP("QPlatformTheme", "OK");
    QT_TRANSLATE_NOOP("QPlatformTheme", "Cancel"):
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
    QT_TRANSLATE_NOOP("MegaError", "Out of range");
    QT_TRANSLATE_NOOP("MegaError", "Expired");
    QT_TRANSLATE_NOOP("MegaError", "Not found");
    QT_TRANSLATE_NOOP("MegaError", "Circular linkage detected");
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
#endif
}

MegaApplication::MegaApplication(int &argc, char **argv) :
    QApplication(argc, argv)
{
    appfinished = false;
    logger = new MegaSyncLogger(this);

    #if defined(LOG_TO_STDOUT) || defined(LOG_TO_FILE) || defined(LOG_TO_LOGGER)
    #if defined(LOG_TO_STDOUT)
        logger->sendLogsToStdout(true);
    #endif

    #if defined(LOG_TO_FILE)
        logger->sendLogsToFile(true);
    #endif

    #ifdef DEBUG
        MegaApi::setLogLevel(MegaApi::LOG_LEVEL_MAX);
    #else
        MegaApi::setLogLevel(MegaApi::LOG_LEVEL_DEBUG);
    #endif
#else
    MegaApi::setLogLevel(MegaApi::LOG_LEVEL_WARNING);
#endif

#ifdef Q_OS_LINUX
    if (argc == 2)
    {
         if (!strcmp("--debug", argv[1]))
         {
             logger->sendLogsToStdout(true);
             MegaApi::setLogLevel(MegaApi::LOG_LEVEL_MAX);
         }
         else if (!strcmp("--version", argv[1]))
         {
            QTextStream(stdout) << "MEGAsync" << " v" << Preferences::VERSION_STRING << " (" << Preferences::SDK_ID << ")" << endl;
            ::exit(0);
         }
    }
#endif

    MegaApi::addLoggerObject(logger);

    //Set QApplication fields
    setOrganizationName(QString::fromAscii("Mega Limited"));
    setOrganizationDomain(QString::fromAscii("mega.co.nz"));
    setApplicationName(QString::fromAscii("MEGAsync"));
    setApplicationVersion(QString::number(Preferences::VERSION_CODE));
    appPath = QDir::toNativeSeparators(QCoreApplication::applicationFilePath());
    appDirPath = QDir::toNativeSeparators(QCoreApplication::applicationDirPath());

    //Set the working directory
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
    QDir::setCurrent(dataPath);

    updateAvailable = false;
    networkConnectivity = true;
    activeTransferPriority[MegaTransfer::TYPE_DOWNLOAD] = 0xFFFFFFFFFFFFFFFFULL;
    activeTransferPriority[MegaTransfer::TYPE_UPLOAD] = 0xFFFFFFFFFFFFFFFFULL;
    activeTransferState[MegaTransfer::TYPE_DOWNLOAD] = MegaTransfer::STATE_NONE;
    activeTransferState[MegaTransfer::TYPE_UPLOAD] = MegaTransfer::STATE_NONE;
    activeTransferTag[MegaTransfer::TYPE_DOWNLOAD] = 0;
    activeTransferTag[MegaTransfer::TYPE_UPLOAD] = 0;
    trayIcon = NULL;
    trayMenu = NULL;
    trayGuestMenu = NULL;
    syncsMenu = NULL;
    megaApi = NULL;
    megaApiFolders = NULL;
    delegateListener = NULL;
    httpServer = NULL;
    httpsServer = NULL;
    numTransfers[MegaTransfer::TYPE_DOWNLOAD] = 0;
    numTransfers[MegaTransfer::TYPE_UPLOAD] = 0;
    exportOps = 0;
    infoDialog = NULL;
    infoOverQuota = false;
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
    webAction = NULL;
    addSyncAction = NULL;
    waiting = false;
    updated = false;
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
    bwOverquotaTimestamp = 0;
    bwOverquotaDialog = NULL;
    storageOverquotaDialog = NULL;
    bwOverquotaEvent = false;
    infoWizard = NULL;
    externalNodesTimestamp = 0;
    noKeyDetected = 0;
    isFirstSyncDone = false;
    isFirstFileSynced = false;
    transferManager = NULL;
    queuedUserStats = 0;
    cleaningSchedulerExecution = 0;
    lastUserActivityExecution = 0;
    maxMemoryUsage = 0;
    nUnviewedTransfers = 0;
    completedTabActive = false;
    inflightUserStats = false;
    nodescurrent = false;
    almostOQ = false;
    storageState = MegaApi::STORAGE_STATE_GREEN;
    appliedStorageState = MegaApi::STORAGE_STATE_GREEN;;

#ifdef __APPLE__
    scanningTimer = NULL;
#endif
}

MegaApplication::~MegaApplication()
{
    if (logger)
    {
        MegaApi::removeLoggerObject(logger);
        delete logger;
    }

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
    showInfoDialog();
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
    if (preferences->error())
    {
        QMegaMessageBox::critical(NULL, QString::fromAscii("MEGAsync"), tr("Your config is corrupt, please start over"), Utilities::getDevicePixelRatio());
    }

    preferences->setLastStatsRequest(0);
    lastExit = preferences->getLastExit();

    installTranslator(&translator);
    QString language = preferences->language();
    changeLanguage(language);
    trayIcon->show();

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

    QString stagingPath = QDir(dataPath).filePath(QString::fromAscii("megasync.staging"));
    QFile fstagingPath(stagingPath);
    if (fstagingPath.exists())
    {
        megaApi->changeApiUrl("https://staging.api.mega.co.nz/");
        megaApiFolders->changeApiUrl("https://staging.api.mega.co.nz/");
        QMegaMessageBox::warning(NULL, QString::fromUtf8("MEGAsync"), QString::fromUtf8("API URL changed to staging"), Utilities::getDevicePixelRatio());
        Preferences::SDK_ID.append(QString::fromUtf8(" - STAGING"));
    }

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
            if (fi.fileName().endsWith(QString::fromAscii(".db"))
                    || fi.fileName().endsWith(QString::fromAscii(".db-wal"))
                    || fi.fileName().endsWith(QString::fromAscii(".db-shm")))
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
#ifndef __APPLE__
                QMegaMessageBox::information(NULL, QString::fromAscii("MEGAsync"), tr("Thank you for your collaboration!"), Utilities::getDevicePixelRatio());
#endif
            }
        }
    }

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
        if (fappShowInterfacePath.open(QIODevice::WriteOnly))
        {
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

    if (infoOverQuota)
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
    else if (indexing || waiting
             || megaApi->getNumPendingUploads()
             || megaApi->getNumPendingDownloads())
    {
        if (indexing)
        {
            tooltip = QCoreApplication::applicationName()
                    + QString::fromAscii(" ")
                    + Preferences::VERSION_STRING
                    + QString::fromAscii("\n")
                    + tr("Scanning");
        }
        else if (waiting || (bwOverquotaTimestamp > QDateTime::currentMSecsSinceEpoch() / 1000))
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
                    + tr("Syncing");
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

    indexing = false;
    paused = false;
    inflightUserStats = false;
    nodescurrent = false;
    infoOverQuota = false;
    almostOQ = false;
    storageState = MegaApi::STORAGE_STATE_GREEN;
    appliedStorageState = MegaApi::STORAGE_STATE_GREEN;;
    bwOverquotaTimestamp = 0;

    if (!isLinux || !trayIcon->contextMenu())
    {
        trayIcon->setContextMenu(initialMenu);
    }

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
    if (!preferences->logged())
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
            infoDialog = new InfoDialog(this);
            connect(infoDialog, SIGNAL(dismissOQ(bool)), this, SLOT(onDismissOQ(bool)));
            connect(infoDialog, SIGNAL(userActivity()), this, SLOT(registerUserActivity()));

            if (!QSystemTrayIcon::isSystemTrayAvailable())
            {
                if (!preferences->isOneTimeActionDone(Preferences::ONE_TIME_ACTION_NO_SYSTRAY_AVAILABLE))
                {
                    QMessageBox::warning(NULL, tr("MEGAsync"),
                                         tr("Could not find a system tray to place MEGAsync tray icon. "
                                            "MEGAsync is intended to be used with a system tray icon but it can work fine without it. "
                                            "If you want to open the interface, just try to open MEGAsync again."));
                    preferences->setOneTimeActionDone(Preferences::ONE_TIME_ACTION_NO_SYSTRAY_AVAILABLE, true);
                }
            }
            createTrayMenu();
        }


        if (!preferences->isFirstStartDone())
        {
            megaApi->sendEvent(99500, "MEGAsync first start");
            openInfoWizard();
        }

        onGlobalSyncStateChanged(megaApi);
        return;
    }
    else
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

        //Otherwise, login in the account
        if (preferences->getSession().size())
        {
            megaApi->fastLogin(preferences->getSession().toUtf8().constData());
        }
        else
        {
            megaApi->fastLogin(preferences->email().toUtf8().constData(),
                       preferences->emailHash().toUtf8().constData(),
                       preferences->privatePw().toUtf8().constData());
        }

        initLocalServer();
        if (updated)
        {
            megaApi->sendEvent(99510, "MEGAsync update");
            checkupdate = true;
        }
    }
}

void MegaApplication::loggedIn()
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

    registerUserActivity();
    pauseTransfers(paused);
    inflightUserStats = false;
    updateUserStats(true);
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

#ifdef WIN32
    if (!preferences->lastExecutionTime())
    {
        showInfoMessage(tr("MEGAsync is now running. Click here to open the status window."));
    }
#else
    #ifdef __APPLE__
        if (!preferences->lastExecutionTime())
        {
            showInfoMessage(tr("MEGAsync is now running. Click the menu bar icon to open the status window."));
        }
    #else
        if (!preferences->lastExecutionTime())
        {
            showInfoMessage(tr("MEGAsync is now running. Click the system tray icon to open the status window."));
        }
    #endif
#endif

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
        infoDialog = new InfoDialog(this);
        connect(infoDialog, SIGNAL(dismissOQ(bool)), this, SLOT(onDismissOQ(bool)));
        connect(infoDialog, SIGNAL(userActivity()), this, SLOT(registerUserActivity()));

        if (!QSystemTrayIcon::isSystemTrayAvailable())
        {
            if (!preferences->isOneTimeActionDone(Preferences::ONE_TIME_ACTION_NO_SYSTRAY_AVAILABLE))
            {
                QMessageBox::warning(NULL, tr("MEGAsync"),
                                     tr("Could not find a system tray to place MEGAsync tray icon. "
                                        "MEGAsync is intended to be used with a system tray icon but it can work fine without it. "
                                        "If you want to open the interface, just try to open MEGAsync again."));
                preferences->setOneTimeActionDone(Preferences::ONE_TIME_ACTION_NO_SYSTRAY_AVAILABLE, true);
            }
            showInfoDialog();
        }
    }
    infoDialog->setUsage();

    createTrayMenu();

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

    onGlobalSyncStateChanged(megaApi);
}

void MegaApplication::startSyncs()
{
    if (appfinished)
    {
        return;
    }

    bool syncsModified = false;

    //Start syncs
    MegaNode *rubbishNode =  megaApi->getRubbishNode();
    for (int i = 0; i < preferences->getNumSyncedFolders(); i++)
    {
        if (!preferences->isFolderActive(i))
        {
            continue;
        }

        MegaNode *node = megaApi->getNodeByHandle(preferences->getMegaFolderHandle(i));
        if (!node)
        {
            showErrorMessage(tr("Your sync \"%1\" has been disabled because the remote folder doesn't exist")
                             .arg(preferences->getSyncName(i)));
            preferences->setSyncState(i, false);
            syncsModified = true;
            openSettings(SettingsDialog::SYNCS_TAB);
            continue;
        }

        QString localFolder = preferences->getLocalFolder(i);
        if (!QFileInfo(localFolder).isDir())
        {
            showErrorMessage(tr("Your sync \"%1\" has been disabled because the local folder doesn't exist")
                             .arg(preferences->getSyncName(i)));
            preferences->setSyncState(i, false);
            syncsModified = true;
            openSettings(SettingsDialog::SYNCS_TAB);
            continue;
        }

        MegaApi::log(MegaApi::LOG_LEVEL_INFO, QString::fromAscii("Sync  %1 added.").arg(i).toUtf8().constData());
        megaApi->syncFolder(localFolder.toUtf8().constData(), node);
        delete node;
    }
    delete rubbishNode;

    if (syncsModified)
    {
        createTrayMenu();
    }
}

void MegaApplication::applyStorageState(int state)
{
    if (state == MegaApi::STORAGE_STATE_CHANGE)
    {
        updateUserStats(true);
        return;
    }

    storageState = state;
    if (preferences->logged())
    {
        if (storageState != appliedStorageState)
        {
            updateUserStats(true);
            appliedStorageState = storageState;
            if (state == MegaApi::STORAGE_STATE_RED)
            {
                almostOQ = false;

                //Disable syncs
                disableSyncs();
                if (!infoOverQuota)
                {
                    infoOverQuota = true;

                    if (preferences->usedStorage() < preferences->totalStorage())
                    {
                        preferences->setUsedStorage(preferences->totalStorage());
                        preferences->sync();

                        if (infoDialog)
                        {
                            infoDialog->setUsage();
                        }

                        if (settingsDialog)
                        {
                            settingsDialog->refreshAccountDetails();
                        }

                        if (bwOverquotaDialog)
                        {
                            bwOverquotaDialog->refreshAccountDetails();
                        }

                        if (storageOverquotaDialog)
                        {
                            storageOverquotaDialog->refreshUsedStorage();
                        }
                    }

                    if (trayMenu && trayMenu->isVisible())
                    {
                        trayMenu->close();
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
                onGlobalSyncStateChanged(megaApi);
            }
            else
            {
                if (state == MegaApi::STORAGE_STATE_GREEN)
                {
                    almostOQ = false;
                }
                else if (state == MegaApi::STORAGE_STATE_ORANGE)
                {
                    almostOQ = true;
                }

                if (infoOverQuota)
                {
                    if (settingsDialog)
                    {
                        settingsDialog->setOverQuotaMode(false);
                    }
                    infoOverQuota = false;

                    if (trayMenu && trayMenu->isVisible())
                    {
                        trayMenu->close();
                    }

                    restoreSyncs();
                    onGlobalSyncStateChanged(megaApi);
                }
            }
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
            HTTPServer::onTransferDataUpdate((*it)->getHandle(), MegaTransfer::STATE_CANCELLED, 0, 0, 0);
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

void MegaApplication::disableSyncs()
{
    if (appfinished)
    {
        return;
    }

    bool syncsModified = false;
    for (int i = 0; i < preferences->getNumSyncedFolders(); i++)
    {
       if (!preferences->isFolderActive(i))
       {
           continue;
       }

       Platform::syncFolderRemoved(preferences->getLocalFolder(i),
                                   preferences->getSyncName(i),
                                   preferences->getSyncID(i));
       notifyItemChange(preferences->getLocalFolder(i), MegaApi::STATE_NONE);
       preferences->setSyncState(i, false, true);
       syncsModified = true;
       MegaNode *node = megaApi->getNodeByHandle(preferences->getMegaFolderHandle(i));
       megaApi->disableSync(node);
       delete node;
    }

    if (syncsModified)
    {
        createTrayMenu();
    }
}

void MegaApplication::restoreSyncs()
{
    if (appfinished)
    {
        return;
    }

    bool syncsModified = false;
    for (int i = 0; i < preferences->getNumSyncedFolders(); i++)
    {
       if (!preferences->isTemporaryInactiveFolder(i) || preferences->isFolderActive(i))
       {
           continue;
       }

       syncsModified = true;
       MegaNode *node = megaApi->getNodeByPath(preferences->getMegaFolder(i).toUtf8().constData());
       if (!node)
       {
           preferences->setSyncState(i, false, false);
           continue;
       }

       QFileInfo localFolderInfo(preferences->getLocalFolder(i));
       QString localFolderPath = QDir::toNativeSeparators(localFolderInfo.canonicalFilePath());
       if (!localFolderPath.size() || !localFolderInfo.isDir())
       {
           delete node;
           preferences->setSyncState(i, false, false);
           continue;
       }

       preferences->setMegaFolderHandle(i, node->getHandle());
       preferences->setSyncState(i, true, false);
       megaApi->syncFolder(localFolderPath.toUtf8().constData(), node);
       delete node;
    }
    Platform::notifyAllSyncFoldersAdded();

    if (syncsModified)
    {
        createTrayMenu();
    }
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

    if (!bwoverquota)
    {
        delete bwOverquotaDialog;
        bwOverquotaDialog = NULL;
    }

    delete storageOverquotaDialog;
    storageOverquotaDialog = NULL;
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

void MegaApplication::exitApplication()
{
    if (appfinished)
    {
        return;
    }

#ifndef __APPLE__
    if (!megaApi->isLoggedIn())
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
    if (lastHovered && lastHovered != pAction)
    {
        lastHovered->setHighlight(false);
        pAction->setHighlight(true);

        lastHovered = pAction;
    }
    else
    {
        lastHovered = pAction;
        lastHovered->setHighlight(true);
    }
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

    if (infoOverQuota)
    {
        if (!preferences->getOverStorageDialogExecution()
                || ((QDateTime::currentMSecsSinceEpoch() - preferences->getOverStorageDialogExecution()) > Preferences::OQ_DIALOG_INTERVAL_MS))
        {
            preferences->setOverStorageDialogExecution(QDateTime::currentMSecsSinceEpoch());
            megaApi->sendEvent(99518, "Overstorage dialog shown");
            if (!storageOverquotaDialog)
            {
                storageOverquotaDialog = new UpgradeOverStorage(megaApi, pricing);
                connect(storageOverquotaDialog, SIGNAL(finished(int)), this, SLOT(overquotaDialogFinished(int)));
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
                    || ((QDateTime::currentMSecsSinceEpoch() - preferences->getOverStorageDismissExecution()) > Preferences::OS_INTERVAL_MS))
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
    else if (almostOQ)
    {
        if (infoDialog)
        {
            if (((QDateTime::currentMSecsSinceEpoch() - preferences->getOverStorageDismissExecution()) > Preferences::ALMOST_OS_INTERVAL_MS)
                         && (!preferences->getAlmostOverStorageDismissExecution() || ((QDateTime::currentMSecsSinceEpoch() - preferences->getAlmostOverStorageDismissExecution()) > Preferences::ALMOST_OS_INTERVAL_MS)))
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
        if (!pendingTransfers && ((QDateTime::currentMSecsSinceEpoch() - preferences->getOverStorageNotificationExecution()) > Preferences::ALMOST_OS_INTERVAL_MS)
                              && ((QDateTime::currentMSecsSinceEpoch() - preferences->getOverStorageDialogExecution()) > Preferences::ALMOST_OS_INTERVAL_MS)
                              && (!preferences->getAlmostOverStorageNotificationExecution() || (QDateTime::currentMSecsSinceEpoch() - preferences->getAlmostOverStorageNotificationExecution()) > Preferences::ALMOST_OS_INTERVAL_MS))
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
        infoDialog->setOverQuotaMode(infoOverQuota);
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

    if (queuedUserStats && queuedUserStats < QDateTime::currentMSecsSinceEpoch())
    {
        queuedUserStats = 0;
        updateUserStats(true);
    }

    checkNetworkInterfaces();
    initLocalServer();

    static int counter = 0;
    if (megaApi)
    {
        if (!(++counter % 6))
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
        }

        megaApi->updateStats();
        onGlobalSyncStateChanged(megaApi);

        if (isLinux)
        {
            updateTrayIcon();
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
    for (int i = 0; i < preferences->getNumSyncedFolders(); i++)
    {
        notifyItemChange(preferences->getLocalFolder(i), MegaApi::STATE_NONE);
    }

    closeDialogs();
    removeAllFinishedTransfers();
    clearViewedTransfers();

    delete bwOverquotaDialog;
    bwOverquotaDialog = NULL;
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

    // Delete menus and menu items
    deleteMenu(initialMenu);
    initialMenu = NULL;
    deleteMenu(trayMenu);
    trayMenu = NULL;
    deleteMenu(syncsMenu);
    syncsMenu = NULL;
    deleteMenu(trayGuestMenu);
    trayGuestMenu = NULL;
#ifdef _WIN32
    deleteMenu(windowsMenu);
    windowsMenu = NULL;
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

    MegaApi::removeLoggerObject(logger);
    delete logger;
    logger = NULL;

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

        Platform::reloadFinderExtension();
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

    if (preferences && preferences->logged())
    {
        updateUserStats(true);
        if (bwOverquotaTimestamp > QDateTime::currentMSecsSinceEpoch() / 1000)
        {
            openBwOverquotaDialog();
            return;
        }
        else if (bwOverquotaTimestamp)
        {
            bwOverquotaTimestamp = 0;
            preferences->clearTemporalBandwidth();
            if (bwOverquotaDialog)
            {
                bwOverquotaDialog->refreshAccountDetails();
            }
    #ifdef __MACH__
            trayIcon->setContextMenu(&emptyMenu);
    #elif defined(_WIN32)
            trayIcon->setContextMenu(windowsMenu);
    #endif
        }
    }

    if (infoDialog)
    {
        if (!infoDialog->isVisible())
        {
            if (storageState == MegaApi::STORAGE_STATE_RED)
            {
                megaApi->sendEvent(99523, "Main dialog shown while overquota");
            }
            else if (storageState == MegaApi::STORAGE_STATE_ORANGE)
            {
                megaApi->sendEvent(99524, "Main dialog shown while almost overquota");
            }

            int posx, posy;
            calculateInfoDialogCoordinates(infoDialog, &posx, &posy);

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

            infoDialog->show();
            infoDialog->updateDialogState();
            infoDialog->raise();
            infoDialog->activateWindow();
        }
        else
        {
            if (trayMenu && trayMenu->isVisible())
            {
                trayMenu->close();
            }
            if (trayGuestMenu && trayGuestMenu->isVisible())
            {
                trayGuestMenu->close();
            }

            infoDialog->hide();
        }
    }
}

void MegaApplication::calculateInfoDialogCoordinates(QDialog *dialog, int *posx, int *posy)
{
    if (appfinished)
    {
        return;
    }

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

        if (position.x() > (screenGeometry.right() / 2))
        {
            *posx = screenGeometry.right() - dialog->width() - 2;
        }
        else
        {
            *posx = screenGeometry.left() + 2;
        }

        if (position.y() > (screenGeometry.bottom() / 2))
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
        connect(httpsServer, SIGNAL(onConnectionError()), this, SLOT(renewLocalSSLcert()), Qt::QueuedConnection);

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
        default:
            break;
    }
}

bool MegaApplication::eventFilter(QObject *obj, QEvent *e)
{
    if (obj == trayMenu)
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
        updatingSSLcert = true;
        lastSSLcertUpdate = QDateTime::currentMSecsSinceEpoch() / 1000;
        megaApi->getLocalSSLCertificate();
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

void MegaApplication::setupWizardFinished(int result)
{
    if (appfinished)
    {
        return;
    }

    if (setupWizard)
    {
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
                HTTPServer::onTransferDataUpdate((*it)->getHandle(), MegaTransfer::STATE_CANCELLED, 0, 0, 0);
            }

            for (QMap<QString, QString>::iterator it = pendingLinks.begin(); it != pendingLinks.end(); it++)
            {
                QString link = it.key();
                QString handle = link.mid(18, 8);
                HTTPServer::onTransferDataUpdate(megaApi->base64ToHandle(handle.toUtf8().constData()),
                                                 MegaTransfer::STATE_CANCELLED, 0, 0, 0);
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

    loggedIn();
    startSyncs();
    applyStorageState(storageState);
}

void MegaApplication::overquotaDialogFinished(int)
{
    if (appfinished)
    {
        return;
    }

    if (bwOverquotaDialog)
    {
        bwOverquotaDialog->deleteLater();
        bwOverquotaDialog = NULL;
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
                HTTPServer::onTransferDataUpdate((*it)->getHandle(), MegaTransfer::STATE_CANCELLED, 0, 0, 0);
            }

            for (QMap<QString, QString>::iterator it = pendingLinks.begin(); it != pendingLinks.end(); it++)
            {
                QString link = it.key();
                QString handle = link.mid(18, 8);
                HTTPServer::onTransferDataUpdate(megaApi->base64ToHandle(handle.toUtf8().constData()),
                                                 MegaTransfer::STATE_CANCELLED, 0, 0, 0);
            }

            qDeleteAll(downloadQueue);
            downloadQueue.clear();
            pendingLinks.clear();
            showInfoMessage(tr("Transfer canceled"));
        }
    }
}

void MegaApplication::unlink()
{
    if (appfinished)
    {
        return;
    }

    //Reset fields that will be initialized again upon login
    qDeleteAll(downloadQueue);
    downloadQueue.clear();
    megaApi->logout();
    Platform::notifyAllSyncFoldersRemoved();
}

void MegaApplication::cleanLocalCaches()
{
    if (!preferences->logged())
    {
        return;
    }

    if (preferences->cleanerDaysLimit())
    {
        int timeLimitDays = preferences->cleanerDaysLimitValue();
        for (int i = 0; i < preferences->getNumSyncedFolders(); i++)
        {
            QString syncPath = preferences->getLocalFolder(i);
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
                        if (creationTime.isValid() && creationTime.daysTo(QDateTime::currentDateTime()) > timeLimitDays)
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
        QMegaMessageBox::information(NULL, title, message, Utilities::getDevicePixelRatio());
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
    else QMegaMessageBox::warning(NULL, title, message, Utilities::getDevicePixelRatio());
}

void MegaApplication::showErrorMessage(QString message, QString title)
{
    if (appfinished)
    {
        return;
    }

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
        QMegaMessageBox::critical(NULL, title, message, Utilities::getDevicePixelRatio());
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
        if (isOSdeprecated)
        {
            QMessageBox::warning(NULL, tr("MEGAsync"),
                                 tr("Please consider updating your operating system.") + QString::fromUtf8("\n")
#ifdef __APPLE__
                                 + tr("MEGAsync will continue to work, however updates will no longer be supported for versions prior to OS X Mavericks soon.")
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

void MegaApplication::redirectToUpgrade(int activationButton)
{
    if (activationButton == MegaNotification::ActivationActionButtonClicked
            || activationButton == MegaNotification::ActivationLegacyNotificationClicked
        #ifndef _WIN32
            || activationButton == MegaNotification::ActivationContentClicked
        #endif
            )
    {
        QString userAgent = QString::fromUtf8(QUrl::toPercentEncoding(QString::fromUtf8(megaApi->getUserAgent())));
        QString url = QString::fromUtf8("pro/uao=%1").arg(userAgent);
        Preferences *preferences = Preferences::instance();
        if (preferences->lastPublicHandleTimestamp() && (QDateTime::currentMSecsSinceEpoch() - preferences->lastPublicHandleTimestamp()) < 86400000)
        {
            MegaHandle aff = preferences->lastPublicHandle();
            if (aff != INVALID_HANDLE)
            {
                char *base64aff = MegaApi::handleToBase64(aff);
                url.append(QString::fromUtf8("/aff=%1/aff_time=%2").arg(QString::fromUtf8(base64aff)).arg(preferences->lastPublicHandleTimestamp() / 1000));
                delete [] base64aff;
            }
        }

        megaApi->getSessionTransferURL(url.toUtf8().constData());
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

void MegaApplication::onDismissOQ(bool overStorage)
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

void MegaApplication::updateUserStats(bool force)
{
    if (appfinished)
    {
        return;
    }

    long long interval = Preferences::MIN_UPDATE_STATS_INTERVAL;
    long long lastRequest = preferences->lastStatsRequest();
    if (force || (QDateTime::currentMSecsSinceEpoch() - lastRequest) > interval)
    {
        preferences->setLastStatsRequest(QDateTime::currentMSecsSinceEpoch());
        if (!inflightUserStats)
        {
            inflightUserStats = true;
            megaApi->getAccountDetails();
        }
        else
        {
            MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, "Skipped call to getAccountDetails()");
        }
        queuedUserStats = 0;
    }
    else
    {
        queuedUserStats = lastRequest + interval;
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

    if (trayGuestMenu && !preferences->logged())
    {
        if (trayGuestMenu->isVisible())
        {
            trayGuestMenu->close();
        }

        QPoint p = point ? (*point) - QPoint(trayGuestMenu->sizeHint().width(), 0)
                         : QCursor::pos();

        trayGuestMenu->update();
        trayGuestMenu->popup(p);
    }
    else if (trayMenu)
    {
        if (trayMenu->isVisible())
        {
            trayMenu->close();
        }

        QPoint p = point ? (*point) - QPoint(trayMenu->sizeHint().width(), 0)
                                 : QCursor::pos();
        trayMenu->update();
        trayMenu->popup(p);
    }
}

void MegaApplication::toggleLogging()
{
    if (appfinished)
    {
        return;
    }

    if (logger->isLogToFileEnabled() || logger->isLogToStdoutEnabled())
    {
        Preferences::HTTPS_ORIGIN_CHECK_ENABLED = true;
        logger->sendLogsToFile(false);
        logger->sendLogsToStdout(false);
        MegaApi::setLogLevel(MegaApi::LOG_LEVEL_WARNING);
        showInfoMessage(tr("DEBUG mode disabled"));
    }
    else
    {
        Preferences::HTTPS_ORIGIN_CHECK_ENABLED = false;
        logger->sendLogsToFile(true);
        MegaApi::setLogLevel(MegaApi::LOG_LEVEL_MAX);
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
    QString webUrl = QString::fromAscii("https://mega.nz/");
    QtConcurrent::run(QDesktopServices::openUrl, QUrl(webUrl));
}

//Called when the "Import links" menu item is clicked
void MegaApplication::importLinks()
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

void MegaApplication::downloadActionClicked()
{
    if (appfinished)
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

    if (streamSelector)
    {
        streamSelector->showNormal();
        streamSelector->activateWindow();
        streamSelector->raise();
        return;
    }

    streamSelector = new StreamingFromMegaDialog(megaApi);
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

    userAction(GuestWidget::LOGIN_CLICKED);
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
                if (setupWizard)
                {
                    setupWizard->goToStep(action);
                    setupWizard->activateWindow();
                    setupWizard->raise();
                    return;
                }
                setupWizard = new SetupWizard(this);
                setupWizard->setModal(false);
                connect(setupWizard, SIGNAL(finished(int)), this, SLOT(setupWizardFinished(int)));
                setupWizard->goToStep(action);
                setupWizard->show();
                break;
        }
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
}

void MegaApplication::createTrayIcon()
{
    if (appfinished)
    {
        return;
    }

    createTrayMenu();
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

    if (isLinux)
    {
        return;
    }

#ifdef _WIN32
    if (preferences && preferences->logged() && megaApi && megaApi->isFilesystemAvailable()
            && bwOverquotaTimestamp <= QDateTime::currentMSecsSinceEpoch() / 1000)
    {
        trayIcon->setContextMenu(windowsMenu);
    }
    else
    {
        trayIcon->setContextMenu(initialMenu);
    }
#else
    trayIcon->setContextMenu(&emptyMenu);
#endif

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
        if (path && !strncmp(path, "//bin/", 6))
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
            settingsDialog->loadSettings();
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
            settingsDialog->loadSettings();
        }
        processDownloadQueue(path);
    }
    else
    {
        QQueue<MegaNode *>::iterator it;
        for (it = downloadQueue.begin(); it != downloadQueue.end(); ++it)
        {
            HTTPServer::onTransferDataUpdate((*it)->getHandle(), MegaTransfer::STATE_CANCELLED, 0, 0, 0);
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
        QString linkForClipboard = QString::fromUtf8("https://mega.nz/#!%1!%2").arg(handle).arg(nodeKey);
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

    char *base64handle = node->getBase64Handle();
    QString url = QString::fromUtf8("fm%1/%2").arg(versions ? QString::fromUtf8("/versions") : QString::fromUtf8(""))
                                              .arg(QString::fromUtf8(base64handle));
    megaApi->getSessionTransferURL(url.toUtf8().constData());
    delete [] base64handle;
    delete node;
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

    if (infoDialog)
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

    if (trayMenu)
    {
        createTrayIcon();
    }

    if (trayGuestMenu)
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

    if (trayMenu)
    {
        createTrayIcon();
    }

    if (trayGuestMenu)
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

    if (appfinished)
    {
        return;
    }

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
                if (!megaApi->isLoggedIn())
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

#ifdef _WIN32
        if (reason == QSystemTrayIcon::Context)
        {
            return;
        }
#endif

#ifndef __APPLE__
        if (isLinux)
        {
            if (trayMenu && trayMenu->isVisible())
            {
                trayMenu->close();
            }
        }
        infoDialogTimer->start(200);
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
                if (!megaApi->isLoggedIn())
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

        int i;
        for (i = 0; i < preferences->getNumSyncedFolders(); i++)
        {
            if (preferences->isFolderActive(i))
            {
                break;
            }
        }
        if (i == preferences->getNumSyncedFolders())
        {
            return;
        }

        infoDialogTimer->stop();
        infoDialog->hide();
        QString localFolderPath = preferences->getLocalFolder(i);
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

//Called when the user wants to open the settings dialog
void MegaApplication::openSettings(int tab)
{
    if (appfinished)
    {
        return;
    }

    if (settingsDialog)
    {
        //If the dialog is active
        if (settingsDialog->isVisible())
        {
            //and visible -> show it
            if (infoOverQuota)
            {
                settingsDialog->setOverQuotaMode(true);
            }
            else
            {
                settingsDialog->setOverQuotaMode(false);
                settingsDialog->openSettingsTab(tab);
            }
            settingsDialog->loadSettings();
            settingsDialog->activateWindow();
            settingsDialog->raise();
            return;
        }

        //Otherwise, delete it
        delete settingsDialog;
        settingsDialog = NULL;
    }

    //Show a new settings dialog
    settingsDialog = new SettingsDialog(this);
    connect(settingsDialog, SIGNAL(userActivity()), this, SLOT(registerUserActivity()));
    if (infoOverQuota)
    {
        settingsDialog->setOverQuotaMode(true);
    }
    else
    {
        if (!megaApi->isFilesystemAvailable() || !preferences->logged())
        {
            changeProxy();
            return;
        }
        settingsDialog->setOverQuotaMode(false);
        settingsDialog->openSettingsTab(tab);
    }
    settingsDialog->setUpdateAvailable(updateAvailable);
    settingsDialog->setModal(false);
    settingsDialog->show();
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

void MegaApplication::openBwOverquotaDialog()
{
    if (appfinished)
    {
        return;
    }

    if (!bwOverquotaDialog)
    {
        bwOverquotaDialog = new UpgradeDialog(megaApi, pricing);
        connect(bwOverquotaDialog, SIGNAL(finished(int)), this, SLOT(overquotaDialogFinished(int)));
        Platform::activateBackgroundWindow(bwOverquotaDialog);
        bwOverquotaDialog->show();

        if (!bwOverquotaEvent)
        {
            megaApi->sendEvent(99506, "Bandwidth overquota");
            bwOverquotaEvent = true;
        }
    }
    else if (!bwOverquotaDialog->isVisible())
    {
        bwOverquotaDialog->activateWindow();
        bwOverquotaDialog->raise();
    }

    bwOverquotaDialog->setTimestamp(bwOverquotaTimestamp);
    bwOverquotaDialog->refreshAccountDetails();
}

void MegaApplication::changeProxy()
{
    if (appfinished)
    {
        return;
    }

    bool proxyOnly = true;

    if (megaApi)
    {
        proxyOnly = !megaApi->isFilesystemAvailable() || !preferences->logged();
        megaApi->retryPendingConnections();
    }

    if (settingsDialog)
    {
        settingsDialog->setProxyOnly(proxyOnly);

        //If the dialog is active
        if (settingsDialog->isVisible())
        {
            if (isLinux && !proxyOnly)
            {
                if (infoOverQuota)
                {
                    settingsDialog->setOverQuotaMode(true);
                }
                else
                {
                    settingsDialog->setOverQuotaMode(false);
                }
            }

            //and visible -> show it
            settingsDialog->loadSettings();
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
    if (isLinux && !proxyOnly)
    {
        if (infoOverQuota)
        {
            settingsDialog->setOverQuotaMode(true);
        }
        else
        {
            settingsDialog->setOverQuotaMode(false);
        }
    }
    settingsDialog->setModal(false);
    settingsDialog->show();
}

//This function creates the tray icon
void MegaApplication::createTrayMenu()
{
    if (appfinished)
    {
        return;
    }

    lastHovered = NULL;

    if (!initialMenu)
    {
        initialMenu = new QMenu();
    }
    else
    {
        QList<QAction *> actions = initialMenu->actions();
        for (int i = 0; i < actions.size(); i++)
        {
            initialMenu->removeAction(actions[i]);
        }
    }

    if (changeProxyAction)
    {
        changeProxyAction->deleteLater();
        changeProxyAction = NULL;
    }
    changeProxyAction = new QAction(tr("Settings"), this);
    connect(changeProxyAction, SIGNAL(triggered()), this, SLOT(changeProxy()));

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
    if (!windowsMenu)
    {
        windowsMenu = new QMenu();
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

    windowsImportLinksAction = new QAction(tr("Import links"), this);
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
#endif

    if (!trayMenu)
    {
        trayMenu = new QMenu();
#ifdef __APPLE__
        trayMenu->setStyleSheet(QString::fromAscii("QMenu {background: #ffffff; padding-top: 8px; padding-bottom: 8px;}"));
#else
        trayMenu->setStyleSheet(QString::fromAscii("QMenu { border: 1px solid #B8B8B8; border-radius: 5px; background: #ffffff; padding-top: 5px; padding-bottom: 5px;}"));
#endif

        //Highlight menu entry on mouse over
        connect(trayMenu, SIGNAL(hovered(QAction*)), this, SLOT(highLightMenuEntry(QAction*)), Qt::QueuedConnection);

        //Hide highlighted menu entry when mouse over
        trayMenu->installEventFilter(this);
    }
    else
    {
        QList<QAction *> actions = trayMenu->actions();
        for (int i = 0; i < actions.size(); i++)
        {
            trayMenu->removeAction(actions[i]);
        }
    }

    if (exitAction)
    {
        exitAction->deleteLater();
        exitAction = NULL;
    }

#ifndef __APPLE__
    exitAction = new MenuItemAction(tr("Exit"), QIcon(QString::fromAscii("://images/ico_quit_out.png")), QIcon(QString::fromAscii("://images/ico_quit_over.png")), true);
#else
    exitAction = new MenuItemAction(tr("Quit"), QIcon(QString::fromAscii("://images/ico_quit_out.png")), QIcon(QString::fromAscii("://images/ico_quit_over.png")), true);
#endif
    connect(exitAction, SIGNAL(triggered()), this, SLOT(exitApplication()), Qt::QueuedConnection);

    if (settingsAction)
    {
        settingsAction->deleteLater();
        settingsAction = NULL;
    }

#ifndef __APPLE__
    settingsAction = new MenuItemAction(tr("Settings"), QIcon(QString::fromAscii("://images/ico_preferences_out.png")), QIcon(QString::fromAscii("://images/ico_preferences_over.png")), true);
#else
    settingsAction = new MenuItemAction(tr("Preferences"), QIcon(QString::fromAscii("://images/ico_preferences_out.png")), QIcon(QString::fromAscii("://images/ico_preferences_over.png")), true);
#endif
    connect(settingsAction, SIGNAL(triggered()), this, SLOT(openSettings()), Qt::QueuedConnection);

    if (webAction)
    {
        webAction->deleteLater();
        webAction = NULL;
    }

    webAction = new MenuItemAction(tr("MEGA website"), QIcon(QString::fromAscii("://images/ico_MEGA_website_out.png")), QIcon(QString::fromAscii("://images/ico_MEGA_website_over.png")), true);
    connect(webAction, SIGNAL(triggered()), this, SLOT(officialWeb()), Qt::QueuedConnection);

    if (addSyncAction)
    {
        addSyncAction->deleteLater();
        addSyncAction = NULL;
    }

    int num = (megaApi && preferences->logged()) ? preferences->getNumSyncedFolders() : 0;
    if (num == 0)
    {
        addSyncAction = new MenuItemAction(tr("Add Sync"), QIcon(QString::fromAscii("://images/ico_syncs_out.png")), QIcon(QString::fromAscii("://images/ico_syncs_over.png")), true);
        connect(addSyncAction, SIGNAL(triggered()), infoDialog, SLOT(addSync()),Qt::QueuedConnection);
    }
    else
    {
        addSyncAction = new MenuItemAction(tr("Syncs"), QIcon(QString::fromAscii("://images/ico_syncs_out.png")), QIcon(QString::fromAscii("://images/ico_syncs_over.png")), true);

        if (syncsMenu)
        {
            syncsMenu->deleteLater();
            syncsMenu = NULL;
        }

        syncsMenu = new QMenu();

#ifdef __APPLE__
        syncsMenu->setStyleSheet(QString::fromAscii("QMenu {background: #ffffff; padding-top: 8px; padding-bottom: 8px;}"));
#else
        syncsMenu->setStyleSheet(QString::fromAscii("QMenu { border: 1px solid #B8B8B8; border-radius: 5px; background: #ffffff; padding-top: 8px; padding-bottom: 8px;}"));
#endif

        QSignalMapper *menuSignalMapper = new QSignalMapper();
        connect(menuSignalMapper, SIGNAL(mapped(QString)), infoDialog, SLOT(openFolder(QString)));

        int activeFolders = 0;
        for (int i = 0; i < num; i++)
        {
            if (!preferences->isFolderActive(i))
            {
                continue;
            }

            activeFolders++;
            MenuItemAction *action = new MenuItemAction(preferences->getSyncName(i), QIcon(QString::fromAscii("://images/ico_drop_synched_folder.png")),
                                                        QIcon(QString::fromAscii("://images/ico_drop_synched_folder_over.png")), true);
            connect(action, SIGNAL(triggered()), menuSignalMapper, SLOT(map()));

            syncsMenu->addAction(action);
            menuSignalMapper->setMapping(action, preferences->getLocalFolder(i));
        }

        if (!activeFolders)
        {
            addSyncAction->setLabelText(tr("Add Sync"));
            connect(addSyncAction, SIGNAL(triggered()), infoDialog, SLOT(addSync()), Qt::QueuedConnection);
        }
        else
        {
            long long firstSyncHandle = INVALID_HANDLE;
            if (num == 1)
            {
                firstSyncHandle = preferences->getMegaFolderHandle(0);
            }

            MegaNode *rootNode = megaApi->getRootNode();
            if (rootNode)
            {
                long long rootHandle = rootNode->getHandle();
                if ((num > 1) || (firstSyncHandle != rootHandle))
                {
                    MenuItemAction *addAction = new MenuItemAction(tr("Add Sync"), QIcon(QString::fromAscii("://images/ico_add_sync.png")),
                                                                       QIcon(QString::fromAscii("://images/ico_drop_add_sync_over.png")), true);
                    connect(addAction, SIGNAL(triggered()), infoDialog, SLOT(addSync()));

                    if (activeFolders)
                    {
                        syncsMenu->addSeparator();
                    }
                    syncsMenu->addAction(addAction);
                }
                delete rootNode;
            }

            addSyncAction->setMenu(syncsMenu);
        }
    }

    if (importLinksAction)
    {
        importLinksAction->deleteLater();
        importLinksAction = NULL;
    }

    importLinksAction = new MenuItemAction(tr("Import links"), QIcon(QString::fromAscii("://images/ico_Import_links_out.png")), QIcon(QString::fromAscii("://images/ico_Import_links_over.png")), true);
    connect(importLinksAction, SIGNAL(triggered()), this, SLOT(importLinks()), Qt::QueuedConnection);

    if (uploadAction)
    {
        uploadAction->deleteLater();
        uploadAction = NULL;
    }

    uploadAction = new MenuItemAction(tr("Upload"), QIcon(QString::fromAscii("://images/ico_upload_out.png")), QIcon(QString::fromAscii("://images/ico_upload_over.png")), true);
    connect(uploadAction, SIGNAL(triggered()), this, SLOT(uploadActionClicked()), Qt::QueuedConnection);

    if (downloadAction)
    {
        downloadAction->deleteLater();
        downloadAction = NULL;
    }

    downloadAction = new MenuItemAction(tr("Download"), QIcon(QString::fromAscii("://images/ico_download_out.png")), QIcon(QString::fromAscii("://images/ico_download_over.png")), true);
    connect(downloadAction, SIGNAL(triggered()), this, SLOT(downloadActionClicked()), Qt::QueuedConnection);

    if (streamAction)
    {
        streamAction->deleteLater();
        streamAction = NULL;
    }

    streamAction = new MenuItemAction(tr("Stream"), QIcon(QString::fromAscii("://images/ico_stream_out.png")), QIcon(QString::fromAscii("://images/ico_stream_over.png")), true);
    connect(streamAction, SIGNAL(triggered()), this, SLOT(streamActionClicked()), Qt::QueuedConnection);

    if (updateAction)
    {
        updateAction->deleteLater();
        updateAction = NULL;
    }

    if (updateAvailable)
    {
        updateAction = new MenuItemAction(tr("Install update"), QIcon(QString::fromAscii("://images/ico_about_MEGA_out.png")), QIcon(QString::fromAscii("://images/ico_about_MEGA_over.png")), true);
    }
    else
    {
        updateAction = new MenuItemAction(tr("About MEGAsync"), QIcon(QString::fromAscii("://images/ico_about_MEGA_out.png")), QIcon(QString::fromAscii("://images/ico_about_MEGA_over.png")), true);
    }
    connect(updateAction, SIGNAL(triggered()), this, SLOT(onInstallUpdateClicked()), Qt::QueuedConnection);

    trayMenu->addAction(updateAction);
    trayMenu->addAction(webAction);
    trayMenu->addSeparator();
    trayMenu->addAction(addSyncAction);
    trayMenu->addAction(importLinksAction);
    trayMenu->addAction(uploadAction);
    trayMenu->addAction(downloadAction);
    trayMenu->addAction(streamAction);
    trayMenu->addAction(settingsAction);
    trayMenu->addSeparator();
    trayMenu->addAction(exitAction);
}

void MegaApplication::createGuestMenu()
{
    if (appfinished)
    {
        return;
    }

    if (!trayGuestMenu)
    {
        trayGuestMenu = new QMenu();

#ifdef __APPLE__
        trayGuestMenu->setStyleSheet(QString::fromAscii("QMenu {background: #ffffff; padding-top: 8px; padding-bottom: 8px;}"));
#else
        trayGuestMenu->setStyleSheet(QString::fromAscii("QMenu { border: 1px solid #B8B8B8; border-radius: 5px; background: #ffffff; padding-top: 5px; padding-bottom: 5px;}"));
#endif
    }
    else
    {
        QList<QAction *> actions = trayGuestMenu->actions();
        for (int i = 0; i < actions.size(); i++)
        {
            trayGuestMenu->removeAction(actions[i]);
        }
    }

    if (exitActionGuest)
    {
        exitActionGuest->deleteLater();
        exitActionGuest = NULL;
    }

#ifndef __APPLE__
    exitActionGuest = new MenuItemAction(tr("Exit"), QIcon(QString::fromAscii("://images/ico_quit_out.png")), QIcon(QString::fromAscii("://images/ico_quit_over.png")));
#else
    exitActionGuest = new MenuItemAction(tr("Quit"), QIcon(QString::fromAscii("://images/ico_quit_out.png")), QIcon(QString::fromAscii("://images/ico_quit_over.png")));
#endif
    connect(exitActionGuest, SIGNAL(triggered()), this, SLOT(exitApplication()));

    if (updateActionGuest)
    {
        updateActionGuest->deleteLater();
        updateActionGuest = NULL;
    }

    if (updateAvailable)
    {
        updateActionGuest = new MenuItemAction(tr("Install update"), QIcon(QString::fromAscii("://images/ico_about_MEGA_out.png")), QIcon(QString::fromAscii("://images/ico_about_MEGA_over.png")));
    }
    else
    {
        updateActionGuest = new MenuItemAction(tr("About MEGAsync"), QIcon(QString::fromAscii("://images/ico_about_MEGA_out.png")), QIcon(QString::fromAscii("://images/ico_about_MEGA_over.png")));
    }
    connect(updateActionGuest, SIGNAL(triggered()), this, SLOT(onInstallUpdateClicked()));

    if (settingsActionGuest)
    {
        settingsActionGuest->deleteLater();
        settingsActionGuest = NULL;
    }

#ifndef __APPLE__
    settingsActionGuest = new MenuItemAction(tr("Settings"), QIcon(QString::fromAscii("://images/ico_preferences_out.png")), QIcon(QString::fromAscii("://images/ico_preferences_over.png")));
#else
    settingsActionGuest = new MenuItemAction(tr("Preferences"), QIcon(QString::fromAscii("://images/ico_preferences_out.png")), QIcon(QString::fromAscii("://images/ico_preferences_over.png")));
#endif
    connect(settingsActionGuest, SIGNAL(triggered()), this, SLOT(changeProxy()));

    trayGuestMenu->addAction(updateActionGuest);
    trayGuestMenu->addSeparator();
    trayGuestMenu->addAction(settingsActionGuest);
    trayGuestMenu->addSeparator();
    trayGuestMenu->addAction(exitActionGuest);
}

void MegaApplication::onEvent(MegaApi *api, MegaEvent *event)
{
    if (event->getType() == MegaEvent::EVENT_CHANGE_TO_HTTPS)
    {
        preferences->setUseHttpsOnly(true);
    }
    else if (event->getType() == MegaEvent::EVENT_ACCOUNT_BLOCKED)
    {
        QMegaMessageBox::critical(NULL, QString::fromUtf8("MEGAsync"),
                                  QCoreApplication::translate("MegaError", event->getText()),
                                  Utilities::getDevicePixelRatio());
    }
    else if (event->getType() == MegaEvent::EVENT_NODES_CURRENT)
    {
        nodescurrent = true;
    }
    else if (event->getType() == MegaEvent::EVENT_STORAGE)
    {
        applyStorageState(event->getNumber());
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
}

//Called when a request has finished
void MegaApplication::onRequestFinish(MegaApi*, MegaRequest *request, MegaError* e)
{
    if (appfinished)
    {
        return;
    }

    if (sslKeyPinningError && request->getType() != MegaRequest::TYPE_LOGOUT)
    {
        delete sslKeyPinningError;
        sslKeyPinningError = NULL;
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

        if (e->getErrorCode() != MegaError::API_OK)
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
            if (bwOverquotaDialog)
            {
                bwOverquotaDialog->setPricing(pricing);
            }

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

        //This prevents to handle logins in the initial setup wizard
        if (preferences->logged())
        {
            Platform::prepareForSync();
            int errorCode = e->getErrorCode();
            if (errorCode == MegaError::API_OK)
            {
                const char *session = megaApi->dumpSession();
                if (session)
                {
                    QString sessionKey = QString::fromUtf8(session);
                    preferences->setSession(sessionKey);
                    delete [] session;

                    //Successful login, fetch nodes
                    megaApi->fetchNodes();
                    break;
                }
            }
            else if (errorCode == MegaError::API_EBLOCKED)
            {
                QMegaMessageBox::critical(NULL, tr("MEGAsync"), tr("Your account has been blocked. Please contact support@mega.co.nz"), Utilities::getDevicePixelRatio());
            }
            else if (errorCode != MegaError::API_ESID && errorCode != MegaError::API_ESSL)
            //Invalid session or public key, already managed in TYPE_LOGOUT
            {
                QMegaMessageBox::warning(NULL, tr("MEGAsync"), tr("Login error: %1").arg(QCoreApplication::translate("MegaError", e->getErrorString())), Utilities::getDevicePixelRatio());
            }

            //Wrong login -> logout
            unlink();
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
                                                tr("Our SSL key can't be verified. You could be affected by a man-in-the-middle attack or your antivirus software could be intercepting your communications and causing this problem. Please disable it and try again.")
                                                + QString::fromUtf8(" (Issuer: %1)").arg(QString::fromUtf8(request->getText() ? request->getText() : "Unknown")),
                                                         QMessageBox::Retry | QMessageBox::Yes | QMessageBox::Cancel);

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
                QMegaMessageBox::information(NULL, QString::fromAscii("MEGAsync"), tr("You have been logged out on this computer from another location"), Utilities::getDevicePixelRatio());
            }
            else if (errorCode == MegaError::API_ESSL)
            {
                QMegaMessageBox::critical(NULL, QString::fromAscii("MEGAsync"),
                                      tr("Our SSL key can't be verified. You could be affected by a man-in-the-middle attack or your antivirus software could be intercepting your communications and causing this problem. Please disable it and try again.")
                                       + QString::fromUtf8(" (Issuer: %1)").arg(QString::fromUtf8(request->getText() ? request->getText() : "Unknown")), Utilities::getDevicePixelRatio());
            }
            else if (errorCode != MegaError::API_EACCESS)
            {
                QMegaMessageBox::information(NULL, QString::fromAscii("MEGAsync"), tr("You have been logged out because of this error: %1")
                                         .arg(QCoreApplication::translate("MegaError", e->getErrorString())), Utilities::getDevicePixelRatio());
            }
            unlink();
        }

        if (preferences && preferences->logged())
        {
            clearUserAttributes();
            preferences->unlink();
            closeDialogs();
            removeAllFinishedTransfers();
            clearViewedTransfers();

            preferences->setFirstStartDone();
            start();
            periodicTasks();
        }
        break;
    }
    case MegaRequest::TYPE_GET_LOCAL_SSL_CERT:
    {
        updatingSSLcert = false;
        if (e->getErrorCode() == MegaError::API_OK)
        {
            MegaStringMap *data = request->getMegaStringMap();
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

        MegaApi::log(MegaApi::LOG_LEVEL_INFO, "Error renewing the local SSL certificate");
        if (e->getErrorCode() == MegaError::API_EACCESS)
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
        //This prevents to handle node requests in the initial setup wizard
        if (preferences->logged())
        {
            if (e->getErrorCode() == MegaError::API_OK)
            {
                if (megaApi->isFilesystemAvailable())
                {
                    //If we have got the filesystem, start the app
                    loggedIn();
                    restoreSyncs();
                }
                else
                {
                    preferences->setCrashed(true);
                }
            }
            else
            {
                MegaApi::log(MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("Error fetching nodes: %1")
                             .arg(QString::fromUtf8(e->getErrorString())).toUtf8().constData());
            }
        }

        break;
    }
    case MegaRequest::TYPE_CHANGE_PW:
    {
        if (e->getErrorCode() == MegaError::API_OK)
        {
            QMessageBox::information(NULL, tr("Password changed"), tr("Your password has been changed."));
        }
        break;
    }
    case MegaRequest::TYPE_ACCOUNT_DETAILS:
    {
        inflightUserStats = false;
        if (!preferences->logged())
        {
            break;
        }

        if (e->getErrorCode() != MegaError::API_OK)
        {
            break;
        }

        MegaNode *root = megaApi->getRootNode();
        MegaNode *inbox = megaApi->getInboxNode();
        MegaNode *rubbish = megaApi->getRubbishNode();
        MegaNodeList *inShares = megaApi->getInShares();
        if (!root || !inbox || !rubbish || !inShares)
        {
            preferences->setCrashed(true);
            delete root;
            delete inbox;
            delete rubbish;
            delete inShares;
            break;
        }

        //Account details retrieved, update the preferences and the information dialog
        MegaAccountDetails *details = request->getMegaAccountDetails();
        preferences->setAccountType(details->getProLevel());
        preferences->setTotalStorage(details->getStorageMax());
        preferences->setUsedStorage(details->getStorageUsed());
        preferences->setTotalBandwidth(details->getTransferMax());
        preferences->setBandwidthInterval(details->getTemporalBandwidthInterval());
        preferences->setUsedBandwidth(details->getProLevel() ? details->getTransferOwnUsed() : details->getTemporalBandwidth());
        preferences->setVersionsStorage(details->getVersionStorageUsed());

        MegaHandle rootHandle = root->getHandle();
        preferences->setCloudDriveStorage(details->getStorageUsed(rootHandle));
        preferences->setCloudDriveFiles(details->getNumFiles(rootHandle));
        preferences->setCloudDriveFolders(details->getNumFolders(rootHandle));
        delete root;

        MegaHandle inboxHandle = inbox->getHandle();
        preferences->setInboxStorage(details->getStorageUsed(inboxHandle));
        preferences->setInboxFiles(details->getNumFiles(inboxHandle));
        preferences->setInboxFolders(details->getNumFolders(inboxHandle));
        delete inbox;

        MegaHandle rubbishHandle = rubbish->getHandle();
        preferences->setRubbishStorage(details->getStorageUsed(rubbishHandle));
        preferences->setRubbishFiles(details->getNumFiles(rubbishHandle));
        preferences->setRubbishFolders(details->getNumFolders(rubbishHandle));
        delete rubbish;

        if (!megaApi->getBandwidthOverquotaDelay() && preferences->accountType() != Preferences::ACCOUNT_TYPE_FREE)
        {
            bwOverquotaTimestamp = 0;
            preferences->clearTemporalBandwidth();
#ifdef __MACH__
            trayIcon->setContextMenu(&emptyMenu);
#elif defined(_WIN32)
            trayIcon->setContextMenu(windowsMenu);
#endif
            if (bwOverquotaDialog)
            {
                bwOverquotaDialog->close();
            }
        }

        preferences->setTemporalBandwidthInterval(details->getTemporalBandwidthInterval());
        preferences->setTemporalBandwidth(details->getTemporalBandwidth());
        preferences->setTemporalBandwidthValid(details->isTemporalBandwidthValid());

        long long inShareSize = 0, inShareFiles = 0, inShareFolders  = 0;
        for (int i = 0; i < inShares->size(); i++)
        {
            MegaNode *node = inShares->get(i);
            if (!node)
            {
                continue;
            }

            MegaHandle handle = node->getHandle();
            inShareSize    += details->getStorageUsed(handle);
            inShareFiles   += details->getNumFiles(handle);
            inShareFolders += details->getNumFolders(handle);
        }
        preferences->setInShareStorage(inShareSize);
        preferences->setInShareFiles(inShareFiles);
        preferences->setInShareFolders(inShareFolders);
        delete inShares;

        preferences->sync();

        if (infoDialog)
        {
            infoDialog->setUsage();
        }

        if (settingsDialog)
        {
            settingsDialog->loadSettings();
        }

        if (bwOverquotaDialog)
        {
            bwOverquotaDialog->refreshAccountDetails();
        }

        if (storageOverquotaDialog)
        {
            storageOverquotaDialog->refreshUsedStorage();
        }

        delete details;
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
        for (int i = preferences->getNumSyncedFolders() - 1; i >= 0; i--)
        {
            if ((request->getNodeHandle() == preferences->getMegaFolderHandle(i)))
            {
                QString localFolder = preferences->getLocalFolder(i);

        #ifdef WIN32
                string path, fsname;
                path.resize(MAX_PATH * sizeof(WCHAR));
                if (GetVolumePathNameW((LPCWSTR)localFolder.utf16(), (LPWSTR)path.data(), MAX_PATH))
                {
                    fsname.resize(MAX_PATH * sizeof(WCHAR));
                    if (!GetVolumeInformationW((LPCWSTR)path.data(), NULL, 0, NULL, NULL, NULL, (LPWSTR)fsname.data(), MAX_PATH))
                    {
                        fsname.clear();
                    }
                }
        #endif

                if (e->getErrorCode() != MegaError::API_OK)
                {
                    MegaNode *node = megaApi->getNodeByHandle(preferences->getMegaFolderHandle(i));
                    const char *nodePath = megaApi->getNodePath(node);
                    delete node;

                    if (!QFileInfo(localFolder).isDir())
                    {
                        showErrorMessage(tr("Your sync \"%1\" has been disabled because the local folder doesn't exist")
                                         .arg(preferences->getSyncName(i)));
                    }
                    else if (nodePath && QString::fromUtf8(nodePath).startsWith(QString::fromUtf8("//bin")))
                    {
                        showErrorMessage(tr("Your sync \"%1\" has been disabled because the remote folder is in the rubbish bin")
                                         .arg(preferences->getSyncName(i)));
                    }
                    else if (!nodePath || preferences->getMegaFolder(i).compare(QString::fromUtf8(nodePath)))
                    {
                        showErrorMessage(tr("Your sync \"%1\" has been disabled because the remote folder doesn't exist")
                                         .arg(preferences->getSyncName(i)));
                    }
                    else if (e->getErrorCode() == MegaError::API_EFAILED)
                    {
#ifdef WIN32
                        WCHAR VBoxSharedFolderFS[] = L"VBoxSharedFolderFS";
                        if (fsname.size() && !memcmp(fsname.data(), VBoxSharedFolderFS, sizeof(VBoxSharedFolderFS)))
                        {
                            QMegaMessageBox::critical(NULL, tr("MEGAsync"),
                                tr("Your sync \"%1\" has been disabled because the synchronization of VirtualBox shared folders is not supported due to deficiencies in that filesystem.")
                                .arg(preferences->getSyncName(i)), Utilities::getDevicePixelRatio());
                        }
                        else
                        {
#endif
                            showErrorMessage(tr("Your sync \"%1\" has been disabled because the local folder has changed")
                                         .arg(preferences->getSyncName(i)));
#ifdef WIN32
                        }
#endif
                    }
                    else if (e->getErrorCode() == MegaError::API_EACCESS)
                    {
                        showErrorMessage(tr("Your sync \"%1\" has been disabled. The remote folder (or part of it) doesn't have full access")
                                         .arg(preferences->getSyncName(i)));

                        if (megaApi->isLoggedIn())
                        {
                            megaApi->fetchNodes();
                        }
                    }
                    else if (e->getErrorCode() != MegaError::API_ENOENT) // Managed in onNodesUpdate
                    {
                        showErrorMessage(QCoreApplication::translate("MegaError", e->getErrorString()));
                    }

                    delete[] nodePath;

                    MegaApi::log(MegaApi::LOG_LEVEL_ERROR, "Error adding sync");
                    Platform::syncFolderRemoved(localFolder,
                                                preferences->getSyncName(i),
                                                preferences->getSyncID(i));

                    if (preferences->isFolderActive(i))
                    {
                        preferences->setSyncState(i, false);
                        createTrayMenu();
                    }

                    if (settingsDialog)
                    {
                        settingsDialog->loadSettings();
                    }
                }
                else
                {
                    preferences->setLocalFingerprint(i, request->getNumber());
                    if (!isFirstSyncDone && !preferences->isFirstSyncDone())
                    {
                        megaApi->sendEvent(99501, "MEGAsync first sync");
                        isFirstSyncDone = true;
                    }

#ifdef _WIN32
                    QString debrisPath = QDir::toNativeSeparators(preferences->getLocalFolder(i) +
                            QDir::separator() + QString::fromAscii(MEGA_DEBRIS_FOLDER));

                    WIN32_FILE_ATTRIBUTE_DATA fad;
                    if (GetFileAttributesExW((LPCWSTR)debrisPath.utf16(),
                                             GetFileExInfoStandard, &fad))
                    {
                        SetFileAttributesW((LPCWSTR)debrisPath.utf16(),
                                           fad.dwFileAttributes | FILE_ATTRIBUTE_HIDDEN);
                    }

                    if (fsname.size())
                    {
                        if ((!memcmp(fsname.data(), L"FAT", 6) || !memcmp(fsname.data(), L"exFAT", 10)) && !preferences->isFatWarningShown())
                        {
                            QMessageBox::warning(NULL, tr("MEGAsync"),
                                             tr("You are syncing a local folder formatted with a FAT filesystem. That filesystem has deficiencies managing big files and modification times that can cause synchronization problems (e.g. when daylight saving changes), so it's strongly recommended that you only sync folders formatted with more reliable filesystems like NTFS (more information [A]here[/A]).")
                                                 .replace(QString::fromUtf8("[A]"), QString::fromUtf8("<a href=\"https://help.mega.nz/megasync/syncing.html#can-i-sync-fat-fat32-partitions-under-windows\">"))
                                                 .replace(QString::fromUtf8("[/A]"), QString::fromUtf8("</a>")));
                            preferences->setFatWarningShown();
                        }
                        else if (!memcmp(fsname.data(), L"HGFS", 8) && !preferences->isOneTimeActionDone(Preferences::ONE_TIME_ACTION_HGFS_WARNING))
                        {
                            QMessageBox::warning(NULL, tr("MEGAsync"),
                                tr("You are syncing a local folder shared with VMWare. Those folders do not support filesystem notifications so MEGAsync will have to be continuously scanning to detect changes in your files and folders. Please use a different folder if possible to reduce the CPU usage."));
                            preferences->setOneTimeActionDone(Preferences::ONE_TIME_ACTION_HGFS_WARNING, true);
                        }
                    }
#endif
                }
                break;
            }
        }

        if (settingsDialog)
        {
            settingsDialog->loadSettings();
        }

        break;
    }
    case MegaRequest::TYPE_REMOVE_SYNC:
    {
        if (e->getErrorCode() == MegaError::API_OK)
        {
            QString syncPath = QString::fromUtf8(request->getFile());

            #ifdef WIN32
            if (syncPath.startsWith(QString::fromAscii("\\\\?\\")))
            {
                syncPath = syncPath.mid(4);
            }
            #endif

            notifyItemChange(syncPath, MegaApi::STATE_NONE);
        }

        if (settingsDialog)
        {
            settingsDialog->loadSettings();
        }

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
                preferences->setLastPublicHandle(node->getHandle());
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
            else
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

    if (transfer->getType() == MegaTransfer::TYPE_DOWNLOAD)
    {
        HTTPServer::onTransferDataUpdate(transfer->getNodeHandle(),
                                             transfer->getState(),
                                             transfer->getTransferredBytes(),
                                             transfer->getTotalBytes(),
                                             transfer->getSpeed());
    }

    if (transferManager)
    {
        transferManager->onTransferStart(megaApi, transfer);
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
                if ((endptr - notificationKey) != strlen(notificationKey))
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
                                             transfer->getSpeed());
    }

    if (transferManager)
    {
        transferManager->onTransferFinish(megaApi, transfer, e);
    }

    if (infoDialog)
    {
        infoDialog->onTransferFinish(megaApi, transfer, e);
    }

    if (finishedTransferOrder.size() > Preferences::MAX_COMPLETED_ITEMS)
    {
        removeFinishedTransfer(finishedTransferOrder.first()->getTag());
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

    if (transferManager)
    {
        transferManager->onTransferUpdate(megaApi, transfer);
    }

    int type = transfer->getType();
    if (type == MegaTransfer::TYPE_DOWNLOAD)
    {
        HTTPServer::onTransferDataUpdate(transfer->getNodeHandle(),
                                             transfer->getState(),
                                             transfer->getTransferredBytes(),
                                             transfer->getTotalBytes(),
                                             transfer->getSpeed());
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

//Called when there is a temporal problem in a transfer
void MegaApplication::onTransferTemporaryError(MegaApi *api, MegaTransfer *transfer, MegaError* e)
{
    if (appfinished)
    {
        return;
    }

    if (transferManager)
    {
        transferManager->onTransferTemporaryError(megaApi, transfer, e);
    }

    onTransferUpdate(api, transfer);
    preferences->setTransferDownloadMethod(api->getDownloadMethod());
    preferences->setTransferUploadMethod(api->getUploadMethod());

    if (e->getErrorCode() == MegaError::API_EOVERQUOTA && e->getValue() && bwOverquotaTimestamp <= (QDateTime::currentMSecsSinceEpoch() / 1000))
    {
        preferences->clearTemporalBandwidth();
        megaApi->getPricing();
        updateUserStats(true);
        bwOverquotaTimestamp = (QDateTime::currentMSecsSinceEpoch() / 1000) + e->getValue();
#ifdef __MACH__
        trayIcon->setContextMenu(initialMenu);
#elif defined(_WIN32)
        trayIcon->setContextMenu(initialMenu);
#endif
        closeDialogs(true);
        openBwOverquotaDialog();
    }
}

void MegaApplication::onAccountUpdate(MegaApi *)
{
    if (appfinished || !preferences->logged())
    {
        return;
    }

    preferences->clearTemporalBandwidth();
    if (bwOverquotaDialog)
    {
        bwOverquotaDialog->refreshAccountDetails();
    }

    updateUserStats(true);
}

//Called when contacts have been updated in MEGA
void MegaApplication::onUsersUpdate(MegaApi *, MegaUserList *userList)
{
    if (appfinished || !infoDialog || !userList || !preferences->logged())
    {
        return;
    }

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

    bool externalNodes = false;
    bool newNodes = false;
    bool nodesRemoved = false;
    long long usedStorage = preferences->usedStorage();
    MegaApi::log(MegaApi::LOG_LEVEL_INFO, QString::fromUtf8("%1 updated files/folders").arg(nodes->size()).toUtf8().constData());

    //Check all modified nodes
    QString localPath;
    for (int i = 0; i < nodes->size(); i++)
    {
        localPath.clear();
        MegaNode *node = nodes->get(i);

        for (int i = 0; i < preferences->getNumSyncedFolders(); i++)
        {
            if (!preferences->isFolderActive(i))
            {
                continue;
            }

            if (node->getType() == MegaNode::TYPE_FOLDER
                    && (node->getHandle() == preferences->getMegaFolderHandle(i)))
            {
                MegaNode *nodeByHandle = megaApi->getNodeByHandle(preferences->getMegaFolderHandle(i));
                const char *nodePath = megaApi->getNodePath(nodeByHandle);

                if (!nodePath || preferences->getMegaFolder(i).compare(QString::fromUtf8(nodePath)))
                {
                    if (nodePath && QString::fromUtf8(nodePath).startsWith(QString::fromUtf8("//bin")))
                    {
                        showErrorMessage(tr("Your sync \"%1\" has been disabled because the remote folder is in the rubbish bin")
                                         .arg(preferences->getSyncName(i)));
                    }
                    else
                    {
                        showErrorMessage(tr("Your sync \"%1\" has been disabled because the remote folder doesn't exist")
                                         .arg(preferences->getSyncName(i)));
                    }
                    Platform::syncFolderRemoved(preferences->getLocalFolder(i),
                                                preferences->getSyncName(i),
                                                preferences->getSyncID(i));
                    notifyItemChange(preferences->getLocalFolder(i), MegaApi::STATE_NONE);
                    MegaNode *node = megaApi->getNodeByHandle(preferences->getMegaFolderHandle(i));
                    megaApi->removeSync(node);
                    delete node;
                    preferences->setSyncState(i, false);
                    openSettings(SettingsDialog::SYNCS_TAB);
                    createTrayMenu();
                }

                delete nodeByHandle;
                delete [] nodePath;
            }
        }

        if (nodescurrent && node->isRemoved() && (node->getType() == MegaNode::TYPE_FILE) && node->getSize())
        {
            usedStorage -= node->getSize();
            nodesRemoved = true;
        }

        if (nodescurrent && !node->isRemoved() && !node->isSyncDeleted()
                && (node->getType() == MegaNode::TYPE_FILE)
                && node->getSize() && node->hasChanged(MegaNode::CHANGE_TYPE_NEW))
        {
            long long bytes = node->getSize();
            if (!megaApi->isInCloud(node))
            {
                preferences->setInShareStorage(preferences->inShareStorage() + bytes);
            }
            else
            {
                preferences->setCloudDriveStorage(preferences->cloudDriveStorage() + bytes);
            }

            usedStorage += bytes;
            newNodes = true;

            if (!externalNodes && !node->getTag()
                    && ((lastExit / 1000) < node->getCreationTime())
                    && megaApi->isInsideSync(node))
            {
                externalNodes = true;
            }
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
                    megaApi->fetchNodes();
                }
            }
            else if (noKeyDetected > 20)
            {
                QMegaMessageBox::critical(NULL, QString::fromUtf8("MEGAsync"),
                    QString::fromUtf8("Something went wrong. MEGAsync will restart now. If the problem persists please contact bug@mega.co.nz"), Utilities::getDevicePixelRatio());
                preferences->setCrashed(true);
                rebootApplication(false);
            }
            noKeyDetected++;
        }
    }

    if (nodesRemoved || newNodes)
    {
        preferences->setUsedStorage(usedStorage);
        preferences->sync();

        if (infoDialog)
        {
            infoDialog->setUsage();
        }

        if (settingsDialog)
        {
            settingsDialog->refreshAccountDetails();
        }

        if (bwOverquotaDialog)
        {
            bwOverquotaDialog->refreshAccountDetails();
        }

        if (storageOverquotaDialog)
        {
            storageOverquotaDialog->refreshUsedStorage();
        }
    }

    if (externalNodes)
    {
        if (QDateTime::currentMSecsSinceEpoch() - externalNodesTimestamp > Preferences::MIN_EXTERNAL_NODES_WARNING_MS)
        {
            externalNodesTimestamp = QDateTime::currentMSecsSinceEpoch();
            showNotificationMessage(tr("You have new or updated files in your account"));
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

void MegaApplication::onGlobalSyncStateChanged(MegaApi *)
{
    if (appfinished)
    {
        return;
    }

    if (megaApi && infoDialog)
    {
        indexing = megaApi->isScanning();
        waiting = megaApi->isWaiting();

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
        infoDialog->updateDialogState();
        infoDialog->transferFinished(MegaError::API_OK);
    }

    if (transferManager)
    {
        transferManager->updateState();
    }

    MegaApi::log(MegaApi::LOG_LEVEL_INFO, QString::fromUtf8("Current state. Paused = %1   Indexing = %2   Waiting = %3")
                 .arg(paused).arg(indexing).arg(waiting).toUtf8().constData());

    updateTrayIcon();
}

void MegaApplication::onSyncStateChanged(MegaApi *api, MegaSync *)
{
    if (appfinished)
    {
        return;
    }

    onGlobalSyncStateChanged(api);
}

void MegaApplication::onSyncFileStateChanged(MegaApi *, MegaSync *, string *localPath, int newState)
{
    if (appfinished)
    {
        return;
    }

    Platform::notifyItemChange(localPath, newState);
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

    megaApi->enableTransferResumption();
    Preferences *preferences = Preferences::instance();
    if (preferences->logged() && !api->getNumActiveSyncs())
    {
#ifdef _WIN32
        bool addToLeftPane = false;
        if (app && app->getPrevVersion() && app->getPrevVersion() <= 3001 && !preferences->leftPaneIconsDisabled())
        {
            addToLeftPane = true;
        }
#endif

        //Start syncs
        for (int i = 0; i < preferences->getNumSyncedFolders(); i++)
        {
            if (!preferences->isFolderActive(i))
            {
                continue;
            }

            MegaNode *node = api->getNodeByHandle(preferences->getMegaFolderHandle(i));
            if (!node)
            {
                preferences->setSyncState(i, false);
                continue;
            }

            QString localFolder = preferences->getLocalFolder(i);

#ifdef _WIN32
            if (addToLeftPane)
            {
                QString name = preferences->getSyncName(i);
                QString uuid = preferences->getSyncID(i);
                Platform::addSyncToLeftPane(localFolder, name, uuid);
            }
#endif

            api->resumeSync(localFolder.toUtf8().constData(), node, preferences->getLocalFingerprint(i));
            delete node;
        }
    }
}

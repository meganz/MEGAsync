#include "MegaApplication.h"
#include "gui/CrashReportDialog.h"
#include "gui/MegaProxyStyle.h"
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

#ifndef WIN32
//sleep
#include <unistd.h>
#endif

using namespace mega;

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
    MegaApplication app(argc, argv);

    qInstallMsgHandler(msgHandler);

#if QT_VERSION >= 0x050000
    qInstallMessageHandler(messageHandler);
#endif

    app.setStyle(new MegaProxyStyle());

#ifdef Q_OS_MACX
    if ( QSysInfo::MacintoshVersion > QSysInfo::MV_10_8 )
    {
        // fix Mac OS X 10.9 (mavericks) font issue
        // https://bugreports.qt-project.org/browse/QTBUG-32789
        QFont::insertSubstitution(QString::fromUtf8(".Lucida Grande UI"), QString::fromUtf8("Lucida Grande"));
    }

    app.setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

    QString crashPath = QDir::current().filePath(QString::fromAscii("crashDumps"));
    QString appLockPath = QDir::current().filePath(QString::fromAscii("megasync.lock"));
    QDir crashDir(crashPath);
    if(!crashDir.exists()) crashDir.mkpath(QString::fromAscii("."));

#ifndef DEBUG
    CrashHandler::instance()->Init(QDir::toNativeSeparators(crashPath));
#endif
    if((argc == 2) && !strcmp("/uninstall", argv[1]))
    {
        Preferences *preferences = Preferences::instance();
        if(!preferences->error())
        {
            if(preferences->logged())
                preferences->unlink();

            for(int i=0; i<preferences->getNumUsers(); i++)
            {
                preferences->enterUser(i);
                for(int j=0; j<preferences->getNumSyncedFolders(); j++)
                {
                    Platform::syncFolderRemoved(preferences->getLocalFolder(j), preferences->getSyncName(j));

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

        QDirIterator di(MegaApplication::applicationDataPath(), QDir::Files | QDir::NoDotAndDotDot);
        while (di.hasNext()) {
            di.next();
            const QFileInfo& fi = di.fileInfo();
            if(fi.fileName().endsWith(QString::fromAscii(".db")) || !fi.fileName().compare(QString::fromUtf8("MEGAsync.cfg")) ||
                    !fi.fileName().compare(QString::fromUtf8("MEGAsync.cfg.bak")))
                QFile::remove(di.filePath());
        }

        //QDir dataDir(dataPath);
        //Utilities::removeRecursively(dataDir);

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
            while(base64stats.size() && base64stats[base64stats.size() - 1] == '=')
            {
                base64stats.resize(base64stats.size() - 1);
            }

            megaApi->sendEvent(99504, base64stats.constData());
            Sleep(5000);
        }
#endif
        return 0;
    }

    SharedTools::QtLockedFile singleInstanceChecker(appLockPath);
    bool alreadyStarted = true;
    for(int i=0; i<10; i++)
    {
        singleInstanceChecker.open(SharedTools::QtLockedFile::ReadWrite);
        if(singleInstanceChecker.lock(SharedTools::QtLockedFile::WriteLock, false))
        {
            alreadyStarted = false;
            break;
        }

        #ifdef WIN32
            Sleep(1000);
        #else
            sleep(1);
        #endif
    }

    if(alreadyStarted)
    {
        MegaApi::log(MegaApi::LOG_LEVEL_WARNING, "MEGAsync is already started");
        return 0;
    }
    Platform::initialize(argc, argv);

#ifndef WIN32
#ifndef __APPLE__
    QFontDatabase::addApplicationFont(QString::fromAscii("://fonts/OpenSans-Regular.ttf"));
    QFontDatabase::addApplicationFont(QString::fromAscii("://fonts/OpenSans-Semibold.ttf"));

    QFont font(QString::fromAscii("Open Sans"), 8);
    app.setFont(font);
#endif
#endif

    //QDate betaLimit(2014, 1, 21);
    //long long now = QDateTime::currentDateTime().toMSecsSinceEpoch();
    //long long betaLimitTime = QDateTime(betaLimit).toMSecsSinceEpoch();
    //if(now > betaLimitTime)
    //{
    //    QMessageBox::information(NULL, QCoreApplication::translate("MegaApplication", "MEGAsync BETA"),
    //           QCoreApplication::translate("MegaApplication", "Thank you for testing MEGAsync.<br>"
    //       "This beta version is no longer current and has expired.<br>"
    //       "Please follow <a href=\"https://twitter.com/MEGAprivacy\">@MEGAprivacy</a> on Twitter for updates."));
    //    return 0;
    //}

    app.initialize();
    app.start();
    return app.exec();

#if 0 //Strings for the translation system. These lines don't need to be built
    QT_TRANSLATE_NOOP("QDialogButtonBox", "&Yes");
    QT_TRANSLATE_NOOP("QDialogButtonBox", "&No");
    QT_TRANSLATE_NOOP("QDialogButtonBox", "&OK");
    QT_TRANSLATE_NOOP("QDialogButtonBox", "&Cancel");
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
    QT_TRANSLATE_NOOP("MegaError", "Unknown error");
#endif

}

MegaApplication::MegaApplication(int &argc, char **argv) :
    QApplication(argc, argv)
{
    logger = new MegaSyncLogger();

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
    if(argc == 2)
    {
         if(!strcmp("--debug", argv[1]))
         {
             logger->sendLogsToStdout(true);
             MegaApi::setLogLevel(MegaApi::LOG_LEVEL_MAX);
         }
    }
#endif

    MegaApi::setLoggerObject(logger);

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
    dataPath = QStandardPaths::standardLocations(QStandardPaths::DataLocation)[0];
#endif

    QDir currentDir(dataPath);
    if(!currentDir.exists()) currentDir.mkpath(QString::fromAscii("."));
    QDir::setCurrent(dataPath);

    appfinished = false;
    updateAvailable = false;
    lastStartedDownload = 0;
    lastStartedUpload = 0;
    trayIcon = NULL;
    trayMenu = NULL;
    trayOverQuotaMenu = NULL;
    trayGuestMenu = NULL;
    megaApi = NULL;
    megaApiLinks = NULL;
    delegateListener = NULL;
    delegateLinksListener = NULL;
    httpServer = NULL;
    totalDownloadSize = totalUploadSize = 0;
    totalDownloadedSize = totalUploadedSize = 0;
    uploadSpeed = downloadSpeed = 0;
    exportOps = 0;
    infoDialog = NULL;
    infoOverQuota = NULL;
    setupWizard = NULL;
    settingsDialog = NULL;
    reboot = false;
    translator = NULL;
    exitAction = NULL;
    exitActionOverquota = NULL;
    exitGuestAction = NULL;
    settingsAction = NULL;
    settingsActionOverquota = NULL;
    settingsActionGuest = NULL;
    importLinksAction = NULL;
    importLinksGuestAction = NULL;
    uploadAction = NULL;
    downloadAction = NULL;
    loginAction = NULL;
    waiting = false;
    updated = false;
    updateAction = NULL;
    updateActionOverquota = NULL;
    updateGuestAction = NULL;
    logoutActionOverquota = NULL;
    showStatusAction = NULL;
    pasteMegaLinksDialog = NULL;
    changeLogDialog = NULL;
    importDialog = NULL;
    uploadFolderSelector = NULL;
    downloadFolderSelector = NULL;
    updateBlocked = false;
    updateThread = NULL;
    updateTask = NULL;
    multiUploadFileDialog = NULL;
    exitDialog = NULL;
    downloadNodeSelector = NULL;
    notificator = NULL;
    externalNodesTimestamp = 0;
    enableDebug = false;
    overquotaCheck = false;
    guestModeEnabled = false;
    noKeyDetected = 0;
    isFirstSyncDone = false;
    isFirstFileSynced = false;
}

MegaApplication::~MegaApplication()
{

}

void MegaApplication::initialize()
{
    if(megaApi != NULL || megaApiLinks != NULL) return;

    paused = false;
    indexing = false;
    setQuitOnLastWindowClosed(false);

#ifdef Q_OS_LINUX
    isLinux = true;
#else
    isLinux = false;
#endif

    //Register metatypes to use them in signals/slots
    qRegisterMetaType<QQueue<QString> >("QQueueQString");
    qRegisterMetaTypeStreamOperators<QQueue<QString> >("QQueueQString");

    preferences = Preferences::instance();
    if(preferences->error())
        QMessageBox::critical(NULL, QString::fromAscii("MEGAsync"), tr("Your config is corrupt, please start over"));

    preferences->setLastStatsRequest(0);
    lastExit = preferences->getLastExit();

    QString basePath = QDir::toNativeSeparators(QDir::currentPath()+QString::fromAscii("/"));

#ifdef WIN32
    //Backwards compatibility code
    QDirIterator di(dataPath, QDir::Files | QDir::NoDotAndDotDot);
    while (di.hasNext()) {
        di.next();
        const QFileInfo& fi = di.fileInfo();
        if(fi.fileName().startsWith(QString::fromAscii(".tmp.")))
            QFile::remove(di.filePath());
    }
#endif

#ifndef __APPLE__
    megaApi = new MegaApi(Preferences::CLIENT_KEY, basePath.toUtf8().constData(), Preferences::USER_AGENT);
    megaApiLinks = new MegaApi(Preferences::CLIENT_KEY, basePath.toUtf8().constData(), Preferences::USER_AGENT);
#else
    megaApi = new MegaApi(Preferences::CLIENT_KEY, basePath.toUtf8().constData(), Preferences::USER_AGENT, MacXPlatform::fd);
    megaApiLinks = new MegaApi(Preferences::CLIENT_KEY, basePath.toUtf8().constData(), Preferences::USER_AGENT, MacXPlatform::fd);
#endif

    delegateListener = new MEGASyncDelegateListener(megaApi, this);
    megaApi->addListener(delegateListener);
    delegateLinksListener = new MEGASyncDelegateListener(megaApiLinks, this);
    megaApiLinks->addListener(delegateLinksListener);
    uploader = new MegaUploader(megaApi);
    downloader = new MegaDownloader(megaApi, megaApiLinks);
    scanningTimer = new QTimer();
    scanningTimer->setSingleShot(false);
    scanningTimer->setInterval(500);
    scanningAnimationIndex = 1;
    connect(scanningTimer, SIGNAL(timeout()), this, SLOT(scanningAnimationStep()));

    //Start the HTTP server
    httpServer = new HTTPServer(megaApiLinks, Preferences::HTTPS_PORT, true);
    connect(httpServer, SIGNAL(onLinkReceived(QString)), this, SLOT(externalDownload(QString)), Qt::QueuedConnection);
    connect(httpServer, SIGNAL(onExternalDownloadRequested(QQueue<mega::MegaNode *>)), this, SLOT(externalDownload(QQueue<mega::MegaNode *>)));
    connect(httpServer, SIGNAL(onExternalDownloadRequestFinished()), this, SLOT(processDownloads()), Qt::QueuedConnection);
    connect(httpServer, SIGNAL(onSyncRequested(long long)), this, SLOT(syncFolder(long long)), Qt::QueuedConnection);

    connectivityTimer = new QTimer(this);
    connectivityTimer->setSingleShot(true);
    connectivityTimer->setInterval(Preferences::MAX_LOGIN_TIME_MS);
    connect(connectivityTimer, SIGNAL(timeout()), this, SLOT(runConnectivityCheck()));

    connect(uploader, SIGNAL(dupplicateUpload(QString, QString, mega::MegaHandle)), this, SLOT(onDupplicateUpload(QString, QString, mega::MegaHandle)));
    connect(downloader, SIGNAL(dupplicateDownload(QString, QString, mega::MegaHandle)), this, SLOT(onDupplicateUpload(QString, QString, mega::MegaHandle)));

    if(preferences->isCrashed())
    {
        preferences->setCrashed(false);
        QDirIterator di(dataPath, QDir::Files | QDir::NoDotAndDotDot);
        while (di.hasNext())
        {
            di.next();
            const QFileInfo& fi = di.fileInfo();
            if(fi.fileName().endsWith(QString::fromAscii(".db")) ||
               fi.fileName().endsWith(QString::fromAscii(".db-wal")) ||
               fi.fileName().endsWith(QString::fromAscii(".db-shm")))
            {
                QFile::remove(di.filePath());
            }
        }

        QStringList reports = CrashHandler::instance()->getPendingCrashReports();
        if(reports.size())
        {
            CrashReportDialog crashDialog(reports.join(QString::fromAscii("------------------------------\n")));
            if(crashDialog.exec() == QDialog::Accepted)
            {
                applyProxySettings();
                CrashHandler::instance()->sendPendingCrashReports(crashDialog.getUserMessage());
                #ifndef __APPLE__
                    QMessageBox::information(NULL, QString::fromAscii("MEGAsync"), tr("Thank you for your collaboration!"));
                #endif
            }
        }
    }

    //Create GUI elements
#ifdef __APPLE__
    trayIcon = new MegaSystemTrayIcon();
#else
    trayIcon = new QSystemTrayIcon();
#endif

    connect(trayIcon, SIGNAL(messageClicked()), this, SLOT(onMessageClicked()));
    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(trayIconActivated(QSystemTrayIcon::ActivationReason)));

    initialMenu = new QMenu();

#if (QT_VERSION == 0x050500) && defined(_WIN32)
    initialMenu->installEventFilter(this);
#endif

#ifdef _WIN32
    windowsMenu = new QMenu();

    #if (QT_VERSION == 0x050500)
    windowsMenu->installEventFilter(this);
    #endif
#endif
    QString language = preferences->language();
    changeLanguage(language);

#ifdef __APPLE__
    notificator = new Notificator(applicationName(), NULL, NULL);
#else
    notificator = new Notificator(applicationName(), trayIcon, NULL);
#endif
    changeProxyAction = new QAction(tr("Settings"), this);
    connect(changeProxyAction, SIGNAL(triggered()), this, SLOT(changeProxy()));
    initialExitAction = new QAction(tr("Exit"), this);
    connect(initialExitAction, SIGNAL(triggered()), this, SLOT(exitApplication()));
    initialMenu->addAction(changeProxyAction);
    initialMenu->addAction(initialExitAction);

#ifdef _WIN32
    windowsExitAction = new QAction(tr("Exit"), this);
    connect(windowsExitAction, SIGNAL(triggered()), this, SLOT(exitApplication()));
    windowsMenu->addAction(windowsExitAction);
#endif

    refreshTimer = new QTimer();
    refreshTimer->start(Preferences::STATE_REFRESH_INTERVAL_MS);
    connect(refreshTimer, SIGNAL(timeout()), this, SLOT(refreshTrayIcon()));

    infoDialogTimer = new QTimer();
    infoDialogTimer->setSingleShot(true);
    connect(infoDialogTimer, SIGNAL(timeout()), this, SLOT(showInfoDialog()));

    connect(this, SIGNAL(aboutToQuit()), this, SLOT(cleanAll()));

    Qt::KeyboardModifiers modifiers = queryKeyboardModifiers();
    if(modifiers.testFlag(Qt::ControlModifier) && modifiers.testFlag(Qt::ShiftModifier))
    {
        enableDebug = true;
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

void MegaApplication::changeLanguage(QString languageCode)
{
    if(translator)
    {
        removeTranslator(translator);
        delete translator;
        translator = NULL;
    }

    QTranslator *newTranslator = new QTranslator();
    if(newTranslator->load(Preferences::TRANSLATION_FOLDER + Preferences::TRANSLATION_PREFIX + languageCode))
    {
        installTranslator(newTranslator);
        translator = newTranslator;
    }
    else delete newTranslator;

    if(notificator)
    {
        createTrayIcon();
        createOverQuotaMenu();
        createGuestMenu();
    }
}

void MegaApplication::updateTrayIcon()
{
    if(!trayIcon) return;
    if(!infoDialog)
    {
        MegaApi::log(MegaApi::LOG_LEVEL_INFO, "Logging in...");
        #ifndef __APPLE__
            #ifdef _WIN32
                trayIcon->setIcon(QIcon(QString::fromAscii("://images/login_ico.ico")));
            #else
                trayIcon->setIcon(QIcon(QString::fromAscii("://images/22_logging.png")));
            #endif
        #else
            trayIcon->setIcon(QIcon(QString::fromAscii("://images/icon_logging_mac.png")),QIcon(QString::fromAscii("://images/icon_logging_mac_white.png")));
            if(scanningTimer->isActive())
                scanningTimer->stop();
        #endif
        trayIcon->setToolTip(QCoreApplication::applicationName() + QString::fromAscii(" ") + Preferences::VERSION_STRING + QString::fromAscii("\n") + tr("Logging in"));
    }
    else if(infoOverQuota)
    {
        if(preferences->usedStorage() < preferences->totalStorage())
        {
            if(!overquotaCheck)
            {
                megaApi->getAccountDetails();
                overquotaCheck = true;
            }
            else
            {
                updateUserStats();
            }
        }

        #ifndef __APPLE__
            #ifdef _WIN32
                trayIcon->setIcon(QIcon(QString::fromAscii("://images/warning_ico.ico")));
            #else
                trayIcon->setIcon(QIcon(QString::fromAscii("://images/22_warning.png")));
            #endif
        #else
            trayIcon->setIcon(QIcon(QString::fromAscii("://images/icon_overquota_mac.png")),QIcon(QString::fromAscii("://images/icon_overquota_mac_white.png")));
            if(scanningTimer->isActive())
                scanningTimer->stop();
        #endif
        trayIcon->setToolTip(QCoreApplication::applicationName() + QString::fromAscii(" ") + Preferences::VERSION_STRING + QString::fromAscii("\n") + tr("Over quota"));
    }
    else if(paused)
    {
        QString tooltip = QCoreApplication::applicationName() + QString::fromAscii(" ") + Preferences::VERSION_STRING + QString::fromAscii("\n") + tr("Paused");
        if(!updateAvailable)
        {
            #ifndef __APPLE__
                #ifdef _WIN32
                    trayIcon->setIcon(QIcon(QString::fromAscii("://images/tray_pause.ico")));
                #else
                    trayIcon->setIcon(QIcon(QString::fromAscii("://images/22_paused.png")));
                #endif
            #else
                trayIcon->setIcon(QIcon(QString::fromAscii("://images/icon_paused_mac.png")),QIcon(QString::fromAscii("://images/icon_paused_mac_white.png")));
                if(scanningTimer->isActive())
                    scanningTimer->stop();
            #endif
            trayIcon->setToolTip(tooltip);
        }
        else
        {
            //TODO: Change icon
            #ifndef __APPLE__
                #ifdef _WIN32
                    trayIcon->setIcon(QIcon(QString::fromAscii("://images/tray_pause.ico")));
                #else
                    trayIcon->setIcon(QIcon(QString::fromAscii("://images/22_paused.png")));
                #endif
            #else
                trayIcon->setIcon(QIcon(QString::fromAscii("://images/icon_paused_mac.png")),QIcon(QString::fromAscii("://images/icon_paused_mac_white.png")));
                if(scanningTimer->isActive())
                    scanningTimer->stop();
            #endif
            tooltip += QString::fromAscii("\n") + tr("Update available!");
            trayIcon->setToolTip(tooltip);
        }
    }
    else if(indexing || waiting || megaApi->getNumPendingUploads() || megaApi->getNumPendingDownloads()
            || megaApiLinks->getNumPendingUploads() || megaApiLinks->getNumPendingDownloads())
    {
        QString tooltip;
        if(indexing) tooltip = QCoreApplication::applicationName() + QString::fromAscii(" ") + Preferences::VERSION_STRING + QString::fromAscii("\n") + tr("Scanning");
        else if(waiting) tooltip = QCoreApplication::applicationName() + QString::fromAscii(" ") + Preferences::VERSION_STRING + QString::fromAscii("\n") + tr("Waiting");
        else tooltip = QCoreApplication::applicationName() + QString::fromAscii(" ") + Preferences::VERSION_STRING + QString::fromAscii("\n") + tr("Syncing");

        if(!updateAvailable)
        {
            #ifndef __APPLE__
                #ifdef _WIN32
                    trayIcon->setIcon(QIcon(QString::fromAscii("://images/tray_sync.ico")));
                #else
                    trayIcon->setIcon(QIcon(QString::fromAscii("://images/22_synching.png")));
                #endif
            #else
                trayIcon->setIcon(QIcon(QString::fromAscii("://images/icon_syncing_mac.png")),QIcon(QString::fromAscii("://images/icon_syncing_mac_white.png")));
                if(!scanningTimer->isActive())
                {
                    scanningAnimationIndex = 1;
                    scanningTimer->start();
                }
            #endif
            trayIcon->setToolTip(tooltip);
        }
        else
        {
            //TODO: Change icon
            #ifndef __APPLE__
                #ifdef _WIN32
                    trayIcon->setIcon(QIcon(QString::fromAscii("://images/tray_sync.ico")));
                #else
                    trayIcon->setIcon(QIcon(QString::fromAscii("://images/22_synching.png")));
                #endif
            #else
                trayIcon->setIcon(QIcon(QString::fromAscii("://images/icon_syncing_mac.png")),QIcon(QString::fromAscii("://images/icon_syncing_mac_white.png")));
                if(!scanningTimer->isActive())
                {
                    scanningAnimationIndex = 1;
                    scanningTimer->start();
                }
            #endif
            tooltip += QString::fromAscii("\n") + tr("Update available!");
            trayIcon->setToolTip(tooltip);
        }
    }
    else
    {
        if(!updateAvailable)
        {
            #ifndef __APPLE__
                #ifdef _WIN32
                    trayIcon->setIcon(QIcon(QString::fromAscii("://images/app_ico.ico")));
                #else
                    trayIcon->setIcon(QIcon(QString::fromAscii("://images/22_uptodate.png")));
                #endif
            #else
                trayIcon->setIcon(QIcon(QString::fromAscii("://images/icon_synced_mac.png")),QIcon(QString::fromAscii("://images/icon_synced_mac_white.png")));
                if(scanningTimer->isActive())
                    scanningTimer->stop();
            #endif
            trayIcon->setToolTip(QCoreApplication::applicationName() + QString::fromAscii(" ") + Preferences::VERSION_STRING + QString::fromAscii("\n") + tr("Up to date"));
        }
        else
        {
            //TODO: Change icon
            #ifndef __APPLE__
                #ifdef _WIN32
                    trayIcon->setIcon(QIcon(QString::fromAscii("://images/app_ico.ico")));
                #else
                    trayIcon->setIcon(QIcon(QString::fromAscii("://images/22_uptodate.png")));
                #endif
            #else
                trayIcon->setIcon(QIcon(QString::fromAscii("://images/icon_synced_mac.png")),QIcon(QString::fromAscii("://images/icon_synced_mac_white.png")));
                if(scanningTimer->isActive())
                    scanningTimer->stop();
            #endif
            trayIcon->setToolTip(QCoreApplication::applicationName() + QString::fromAscii(" ") + Preferences::VERSION_STRING + QString::fromAscii("\n") + tr("Update available!"));
        }

        if(reboot)
            rebootApplication();
    }
}

void MegaApplication::start()
{
    indexing = false;
    overquotaCheck = false;

#ifdef Q_OS_LINUX
    if(isLinux && trayIcon->contextMenu())
    {
        if(showStatusAction)
        {
            initialMenu->removeAction(showStatusAction);

            delete showStatusAction;
            showStatusAction = NULL;
        }
    }
    else
    {
        trayIcon->setContextMenu(initialMenu);
    }
#else
        trayIcon->setContextMenu(initialMenu);
#endif

    trayIcon->setToolTip(QCoreApplication::applicationName() + QString::fromAscii(" ") + Preferences::VERSION_STRING + QString::fromAscii("\n") + tr("Logging in"));
    trayIcon->show();

    if(!preferences->lastExecutionTime())
    {
        Platform::enableTrayIcon(QFileInfo(MegaApplication::applicationFilePath()).fileName());
    }

    if(updated)
    {
        showInfoMessage(tr("MEGAsync has been updated"));
        preferences->setFirstSyncDone();
        preferences->setFirstFileSynced();
        preferences->setFirstWebDownloadDone();

        if(!preferences->installationTime())
        {
            preferences->setInstallationTime(-1);
        }
    }

    if(enableDebug)
    {
        toggleLogging();
        enableDebug = false;
    }

    applyProxySettings();

    //Start the initial setup wizard if needed
    if(!preferences->logged())
    {

        if(!preferences->installationTime())
        {
            preferences->setInstallationTime(QDateTime::currentDateTime().toMSecsSinceEpoch() / 1000);
        }

        updated = false;
        guestModeEnabled = true;
        guestMode();

        if(!preferences->isFirstStartDone())
        {
            megaApi->sendEvent(99500, "MEGAsync first start");
            userAction(SetupWizard::PAGE_SETUP);
        }

        return;
    }
	else
	{
        QStringList exclusions = preferences->getExcludedSyncNames();
        vector<string> vExclusions;
        for(int i=0; i<exclusions.size(); i++)
            vExclusions.push_back(exclusions[i].toUtf8().constData());
        megaApi->setExcludedNames(&vExclusions);

        if(preferences->lowerSizeLimit())
        {
            megaApi->setExclusionLowerSizeLimit(preferences->lowerSizeLimitValue() * pow((float)1024, preferences->lowerSizeLimitUnit()));
        }
        else
        {
            megaApi->setExclusionLowerSizeLimit(0);
        }

        if(preferences->upperSizeLimit())
        {
            megaApi->setExclusionUpperSizeLimit(preferences->upperSizeLimitValue() * pow((float)1024, preferences->upperSizeLimitUnit()));
        }
        else
        {
            megaApi->setExclusionUpperSizeLimit(0);
        }

        //Otherwise, login in the account
        if(preferences->getSession().size())
        {
            megaApi->fastLogin(preferences->getSession().toUtf8().constData());
        }
        else
        {
            megaApi->fastLogin(preferences->email().toUtf8().constData(),
                       preferences->emailHash().toUtf8().constData(),
                       preferences->privatePw().toUtf8().constData());
        }
    }
}

void MegaApplication::guestMode()
{
    if(infoDialog)
    {
        infoDialog->setGuestMode(true);
        infoDialog->init();
        return;
    }

    startUpdateTask();

    QString language = preferences->language();
    changeLanguage(language);

    updated = false;
    infoDialog = new InfoDialog(this, true);

}
void MegaApplication::loggedIn()
{
    megaApi->getAccountDetails();

    if(settingsDialog)
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

    startUpdateTask();

    QString language = preferences->language();
    changeLanguage(language);

#ifdef WIN32
    if(!preferences->lastExecutionTime()) showInfoMessage(tr("MEGAsync is now running. Click here to open the status window."));
    else if(!updated) showNotificationMessage(tr("MEGAsync is now running. Click here to open the status window."));
#else
    #ifdef __APPLE__
        if(!preferences->lastExecutionTime()) showInfoMessage(tr("MEGAsync is now running. Click the menu bar icon to open the status window."));
        else if(!updated) showNotificationMessage(tr("MEGAsync is now running. Click the menu bar icon to open the status window."));
    #else
        if(!preferences->lastExecutionTime()) showInfoMessage(tr("MEGAsync is now running. Click the system tray icon to open the status window."));
        else if(!updated) showNotificationMessage(tr("MEGAsync is now running. Click the system tray icon to open the status window."));
    #endif
#endif

    preferences->setLastExecutionTime(QDateTime::currentDateTime().toMSecsSinceEpoch());

    updated = false;

    if(!infoDialog)
    {
        infoDialog = new InfoDialog(this);
    }
    else
    {
        infoDialog->init();
    }

    //Set the upload limit
    setUploadLimit(preferences->uploadLimitKB());
    Platform::startShellDispatcher(this);
}

void MegaApplication::startSyncs()
{
    //Start syncs
    MegaNode *rubbishNode =  megaApi->getRubbishNode();
	for(int i=0; i<preferences->getNumSyncedFolders(); i++)
	{
        if(!preferences->isFolderActive(i))
            continue;

        MegaNode *node = megaApi->getNodeByHandle(preferences->getMegaFolderHandle(i));
        if(!node)
        {
            showErrorMessage(tr("Your sync \"%1\" has been disabled because the remote folder doesn't exist")
                             .arg(preferences->getSyncName(i)));
            preferences->setSyncState(i, false);
            openSettings(SettingsDialog::SYNCS_TAB);
            continue;
        }

        QString localFolder = preferences->getLocalFolder(i);
        if(!QFileInfo(localFolder).isDir())
        {
            showErrorMessage(tr("Your sync \"%1\" has been disabled because the local folder doesn't exist")
                             .arg(preferences->getSyncName(i)));
            preferences->setSyncState(i, false);
            openSettings(SettingsDialog::SYNCS_TAB);
            continue;
        }

        MegaApi::log(MegaApi::LOG_LEVEL_INFO, QString::fromAscii("Sync  %1 added.").arg(i).toUtf8().constData());
        megaApi->syncFolder(localFolder.toUtf8().constData(), node);
        delete node;
	}
    delete rubbishNode;
}

//This function is called to upload all files in the uploadQueue field
//to the Mega node that is passed as parameter
void MegaApplication::processUploadQueue(mega::MegaHandle nodeHandle)
{
    MegaNode *node = megaApi->getNodeByHandle(nodeHandle);

    //If the destination node doesn't exist in the current filesystem, clear the queue and show an error message
    if(!node || node->isFile())
	{
		uploadQueue.clear();
        showErrorMessage(tr("Error: Invalid destination folder. The upload has been cancelled"));
        delete node;
        return;
	}

    //Process the upload queue using the MegaUploader object
	while(!uploadQueue.isEmpty())
	{
		QString filePath = uploadQueue.dequeue();
        uploader->upload(filePath, node);
    }
    delete node;
}

void MegaApplication::processDownloadQueue(QString path)
{
    QDir dir(path);
    if(!dir.exists() && !dir.mkpath(QString::fromAscii(".")))
    {
        qDeleteAll(downloadQueue);
        downloadQueue.clear();
        showErrorMessage(tr("Error: Invalid destination folder. The download has been cancelled"));
        return;
    }

    downloader->processDownloadQueue(&downloadQueue, path);
}

void MegaApplication::unityFix()
{
    static QMenu *dummyMenu = NULL;
    if(!dummyMenu)
    {
        dummyMenu = new QMenu();
        connect(this, SIGNAL(unityFixSignal()), dummyMenu, SLOT(close()), Qt::QueuedConnection);
    }

    emit unityFixSignal();
    dummyMenu->exec();
}

void MegaApplication::disableSyncs()
{
    for(int i=0; i<preferences->getNumSyncedFolders(); i++)
    {
       if(!preferences->isFolderActive(i))
       {
           continue;
       }

       Platform::syncFolderRemoved(preferences->getLocalFolder(i), preferences->getSyncName(i));
       Platform::notifyItemChange(preferences->getLocalFolder(i));
       preferences->setSyncState(i, false, true);
       MegaNode *node = megaApi->getNodeByHandle(preferences->getMegaFolderHandle(i));
       megaApi->disableSync(node);
       delete node;
    }
}

void MegaApplication::restoreSyncs()
{
    for(int i=0; i<preferences->getNumSyncedFolders(); i++)
    {
       if(!preferences->isTemporaryInactiveFolder(i) || preferences->isFolderActive(i))
       {
           continue;
       }

       MegaNode *node = megaApi->getNodeByPath(preferences->getMegaFolder(i).toUtf8().constData());
       if(!node)
       {
           preferences->setSyncState(i, false, false);
           continue;
       }

       QFileInfo localFolderInfo(preferences->getLocalFolder(i));
       QString localFolderPath = QDir::toNativeSeparators(localFolderInfo.canonicalFilePath());
       if(!localFolderPath.size() || !localFolderInfo.isDir())
       {
           delete node;
           preferences->setSyncState(i, false, false);
           continue;
       }

       preferences->setMegaFolderHandle(i, node->getHandle());
       preferences->setSyncState(i, true, false);
       megaApi->syncFolder(localFolderPath.toUtf8().constData(), node);
       delete node;
       //Platform::notifyItemChange(preferences->getLocalFolder(i));
    }
}

void MegaApplication::closeDialogs()
{
    delete setupWizard;
    setupWizard = NULL;

    delete settingsDialog;
    settingsDialog = NULL;

    delete uploadFolderSelector;
    uploadFolderSelector = NULL;

    delete downloadFolderSelector;
    downloadFolderSelector = NULL;

    delete multiUploadFileDialog;
    multiUploadFileDialog = NULL;

    delete pasteMegaLinksDialog;
    pasteMegaLinksDialog = NULL;

    delete changeLogDialog;
    changeLogDialog = NULL;

    delete importDialog;
    importDialog = NULL;

    delete downloadNodeSelector;
    downloadNodeSelector = NULL;

    delete infoOverQuota;
    infoOverQuota = NULL;
}

void MegaApplication::rebootApplication(bool update)
{
    reboot = true;
    if(update && (megaApi->getNumPendingDownloads() || megaApi->getNumPendingUploads() || megaApi->isWaiting()))
    {
        if(!updateBlocked)
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
#ifndef __APPLE__
    if(!megaApi->isLoggedIn())
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

    if(!exitDialog)
    {
        exitDialog = new QMessageBox(QMessageBox::Question, tr("MEGAsync"),
                                     tr("Synchronization will stop.\n\nExit anyway?"), QMessageBox::Yes|QMessageBox::No);
        int button = exitDialog->exec();
        if(!exitDialog)
        {
            return;
        }

        exitDialog->deleteLater();
        exitDialog = NULL;
        if(button == QMessageBox::Yes)
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
        exitDialog->raise();
        exitDialog->activateWindow();
    }
}

void MegaApplication::pauseTransfers(bool pause)
{
    megaApi->pauseTransfers(pause);
    megaApiLinks->pauseTransfers(pause);
}

void MegaApplication::refreshTrayIcon()
{
    QList<QNetworkInterface> newNetworkInterfaces;
    QList<QNetworkInterface> configs = QNetworkInterface::allInterfaces();
    //Filter interfaces (QT provides interfaces with loopback IP addresses)
    for(int i=0; i<configs.size(); i++)
    {
        QNetworkInterface networkInterface = configs.at(i);
        if(networkInterface.flags() & QNetworkInterface::IsUp)
        {
            MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, QString::fromUtf8("Active network interface: %1").arg(networkInterface.humanReadableName()).toUtf8().constData());
            int numActiveIPs = 0;
            QList<QNetworkAddressEntry> addresses = networkInterface.addressEntries();
            for(int i=0; i < addresses.size(); i++)
            {
                QHostAddress ip = addresses.at(i).ip();
                switch(ip.protocol())
                {
                case QAbstractSocket::IPv4Protocol:
                    if(!ip.toString().startsWith(QString::fromUtf8("127."), Qt::CaseInsensitive))
                    {
                        MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, QString::fromUtf8("IPv4: %1").arg(ip.toString()).toUtf8().constData());
                        numActiveIPs++;
                    }
                    break;
                case QAbstractSocket::IPv6Protocol:
                    if(!ip.toString().startsWith(QString::fromUtf8("FE80::"), Qt::CaseInsensitive) && !(ip.toString() == QString::fromUtf8("::1")))
                    {
                        MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, QString::fromUtf8("IPv6: %1").arg(ip.toString()).toUtf8().constData());
                        numActiveIPs++;
                    }
                    break;
                default:
                    break;
                }
            }

            if(!numActiveIPs)
            {
                continue;
            }

            lastActiveTime = QDateTime::currentMSecsSinceEpoch();
            newNetworkInterfaces.append(networkInterface);
        }
    }

    bool disconnect = false;
    if(!newNetworkInterfaces.size())
    {
        networkManager.updateConfigurations();
    }
    else if(!activeNetworkInterfaces.size())
    {
        activeNetworkInterfaces = newNetworkInterfaces;
    }
    else if(activeNetworkInterfaces.size() != newNetworkInterfaces.size())
    {
        disconnect = true;
        MegaApi::log(MegaApi::LOG_LEVEL_INFO, "Local network interface change detected");
    }
    else
    {
        for(int i=0; i < newNetworkInterfaces.size(); i++)
        {
            QNetworkInterface networkInterface = newNetworkInterfaces.at(i);
            int j=0;
            for(; j < activeNetworkInterfaces.size(); j++)
            {
                if(activeNetworkInterfaces.at(j).name() == networkInterface.name())
                    break;
            }

            if(j == activeNetworkInterfaces.size())
            {
                //New interface
                MegaApi::log(MegaApi::LOG_LEVEL_INFO, QString::fromUtf8("New working network interface detected (%1)").arg(networkInterface.humanReadableName()).toUtf8().constData());
                disconnect = true;
            }
            else
            {
                QNetworkInterface oldNetworkInterface = activeNetworkInterfaces.at(j);
                QList<QNetworkAddressEntry> addresses = networkInterface.addressEntries();
                if(addresses.size() != oldNetworkInterface.addressEntries().size())
                {
                    MegaApi::log(MegaApi::LOG_LEVEL_INFO, "Local IP change detected");
                    disconnect = true;
                }
                else
                {
                    for(int k=0; k < addresses.size(); k++)
                    {
                        QHostAddress ip = addresses.at(k).ip();
                        switch(ip.protocol())
                        {
                        case QAbstractSocket::IPv4Protocol:
                        case QAbstractSocket::IPv6Protocol:
                        {
                            QList<QNetworkAddressEntry> oldAddresses = oldNetworkInterface.addressEntries();
                            int l=0;
                            for(; l<oldAddresses.size(); l++)
                            {
                                if(oldAddresses.at(l).ip().toString() == ip.toString())
                                {
                                    break;
                                }
                            }
                            if(l == oldAddresses.size())
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

    if(disconnect || (QDateTime::currentMSecsSinceEpoch() - lastActiveTime) > Preferences::MAX_IDLE_TIME_MS)
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

    static int counter = 0;
    if(megaApi)
    {
        if(!(++counter % 6))
        {
            networkManager.updateConfigurations();
            megaApi->update();
        }

        megaApi->updateStats();
        onGlobalSyncStateChanged(megaApi);
        if(isLinux) updateTrayIcon();
    }
    if(trayIcon) trayIcon->show();
}

void MegaApplication::cleanAll()
{
    appfinished = true;

#ifndef DEBUG
    CrashHandler::instance()->Disable();
#endif

    qInstallMsgHandler(0);
#if QT_VERSION >= 0x050000
    qInstallMessageHandler(0);
#endif

    refreshTimer->stop();
    stopUpdateTask();
    Platform::stopShellDispatcher();
    for(int i=0; i<preferences->getNumSyncedFolders(); i++)
        Platform::notifyItemChange(preferences->getLocalFolder(i));

    closeDialogs();
    delete infoDialog;
    delete httpServer;
    delete uploader;
    delete delegateListener;
    delete delegateLinksListener;
    delete megaApi;
    delete megaApiLinks;

    preferences->setLastExit(QDateTime::currentMSecsSinceEpoch());
    trayIcon->deleteLater();

    if(reboot)
    {
    #ifndef __APPLE__
        QString app = MegaApplication::applicationFilePath();
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
    addRecentFile(name, handle);
}

void MegaApplication::onDupplicateUpload(QString localPath, QString name, MegaHandle handle)
{
    addRecentFile(name, handle, localPath);
}

void MegaApplication::onInstallUpdateClicked()
{
    if(updateAvailable)
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
    if(isLinux && showStatusAction && megaApi)
    {
        megaApi->retryPendingConnections();
    }

    if(infoOverQuota)
    {
        if(!infoOverQuota->isVisible())
        {
            int posx, posy;
            calculateInfoDialogCoordinates(infoOverQuota, &posx, &posy);

            if(isLinux) unityFix();

            infoOverQuota->move(posx, posy);
            infoOverQuota->show();
            infoOverQuota->setFocus();
            infoOverQuota->raise();
            infoOverQuota->activateWindow();
        }
        else
        {
            if(trayOverQuotaMenu->isVisible())
                trayOverQuotaMenu->close();
            infoOverQuota->hide();
        }
    }
    else if(infoDialog)
    {
        if(!infoDialog->isVisible())
        {
            int posx, posy;
            calculateInfoDialogCoordinates(infoDialog, &posx, &posy);

            if(isLinux) unityFix();

            infoDialog->move(posx, posy);

            #ifdef __APPLE__
                QPoint positionTrayIcon = trayIcon->getPosition();
                QPoint globalCoordinates(positionTrayIcon.x() + trayIcon->geometry().width()/2, posy);

                //Work-Around to paint the arrow correctly
                infoDialog->show();
                QPixmap px = QPixmap::grabWidget(infoDialog);
                infoDialog->hide();
                QPoint localCoordinates = infoDialog->mapFromGlobal(globalCoordinates);
                infoDialog->moveArrow(localCoordinates);
            #endif

            infoDialog->show();
            infoDialog->setFocus();
            infoDialog->raise();
            infoDialog->activateWindow();
            infoDialog->updateTransfers();
        }
        else
        {
            infoDialog->closeSyncsMenu();
            if(trayMenu->isVisible())
            {
                trayMenu->close();
            }
            if(trayGuestMenu->isVisible())
            {
                trayGuestMenu->close();
            }

            infoDialog->hide();
        }
    }
}

void MegaApplication::calculateInfoDialogCoordinates(QDialog *dialog, int *posx, int *posy)
{
    QPoint position, positionTrayIcon;
    QRect screenGeometry;

    #ifdef __APPLE__
        positionTrayIcon = trayIcon->getPosition();
    #endif

    position = QCursor::pos();
    QDesktopWidget *desktop = QApplication::desktop();
    int screenIndex = desktop->screenNumber(position);
    screenGeometry = desktop->availableGeometry(screenIndex);


    #ifdef __APPLE__
        if(positionTrayIcon.x() || positionTrayIcon.y())
        {
            if((positionTrayIcon.x()+dialog->width()/2) > screenGeometry.right())
            {
                *posx = screenGeometry.right() - dialog->width() - 1;
            }
            else
            {
                *posx = positionTrayIcon.x() + trayIcon->geometry().width()/2 - dialog->width()/2 - 1;
            }

        }
        else
        {
            *posx = screenGeometry.right() - dialog->width() - 1;
        }
        *posy = screenIndex ? screenGeometry.top()+22: screenGeometry.top();
    #else
        #ifdef WIN32
            QRect totalGeometry = QApplication::desktop()->screenGeometry();
            if(totalGeometry == screenGeometry)
            {
                APPBARDATA pabd;
                pabd.cbSize = sizeof(APPBARDATA);
                pabd.hWnd = FindWindow(L"Shell_TrayWnd", NULL);
                if(pabd.hWnd && SHAppBarMessage(ABM_GETTASKBARPOS, &pabd))
                {
                    switch (pabd.uEdge)
                    {
                        case ABE_LEFT:
                            screenGeometry.setLeft(pabd.rc.right+1);
                            break;
                        case ABE_RIGHT:
                            screenGeometry.setRight(pabd.rc.left-1);
                            break;
                        case ABE_TOP:
                            screenGeometry.setTop(pabd.rc.bottom+1);
                            break;
                        case ABE_BOTTOM:
                            screenGeometry.setBottom(pabd.rc.top-1);
                            break;
                    }
                }
            }
        #endif

        if(position.x() > (screenGeometry.right()/2))
            *posx = screenGeometry.right() - dialog->width() - 2;
        else
            *posx = screenGeometry.left() + 2;

        if(position.y() > (screenGeometry.bottom()/2))
            *posy = screenGeometry.bottom() - dialog->height() - 2;
        else
            *posy = screenGeometry.top() + 2;
    #endif

}

bool MegaApplication::anUpdateIsAvailable()
{
    return updateAvailable;
}

void MegaApplication::triggerInstallUpdate()
{
    emit installUpdate();
}

void MegaApplication::scanningAnimationStep()
{
    scanningAnimationIndex = scanningAnimationIndex%4;
    scanningAnimationIndex++;
    trayIcon->setIcon(QIcon(QString::fromAscii("://images/icon_syncing_mac") +
                            QString::number(scanningAnimationIndex) + QString::fromAscii(".png"))
#ifdef __APPLE__
    , QIcon(QString::fromAscii("://images/icon_syncing_mac_white") + QString::number(scanningAnimationIndex) + QString::fromAscii(".png")));
#else
    );
#endif
}

void MegaApplication::runConnectivityCheck()
{
    QNetworkProxy proxy;
    proxy.setType(QNetworkProxy::NoProxy);
    if(preferences->proxyType() == Preferences::PROXY_TYPE_CUSTOM)
    {
        int proxyProtocol = preferences->proxyProtocol();
        switch(proxyProtocol)
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
        if(preferences->proxyRequiresAuth())
        {
            proxy.setUser(preferences->getProxyUsername());
            proxy.setPassword(preferences->getProxyPassword());
        }
    }
    else if(preferences->proxyType() == MegaProxy::PROXY_AUTO)
    {
        MegaProxy* autoProxy = megaApi->getAutoProxySettings();
        if(autoProxy && autoProxy->getProxyType()==MegaProxy::PROXY_CUSTOM)
        {
            string sProxyURL = autoProxy->getProxyURL();
            QString proxyURL = QString::fromUtf8(sProxyURL.data());

            QStringList parts = proxyURL.split(QString::fromAscii("://"));
            if(parts.size() == 2 && parts[0].startsWith(QString::fromUtf8("socks")))
            {
                proxy.setType(QNetworkProxy::Socks5Proxy);
            }
            else
            {
                proxy.setType(QNetworkProxy::HttpProxy);
            }

            QStringList arguments = parts[parts.size()-1].split(QString::fromAscii(":"));
            if(arguments.size() == 2)
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
    MegaApi::log(MegaApi::LOG_LEVEL_INFO, "Connectivity test finished OK");
}

void MegaApplication::onConnectivityCheckError()
{
    showErrorMessage(tr("MEGAsync is unable to connect. Please check your Internet connectivity and local firewall configuration. Note that most antivirus software includes a firewall."));
}

void MegaApplication::setupWizardFinished()
{
    if(setupWizard)
    {
        setupWizard->deleteLater();
        setupWizard = NULL;
    }

    guestModeEnabled = false;
    infoDialog->setGuestMode(false);

    QStringList exclusions = preferences->getExcludedSyncNames();
    vector<string> vExclusions;
    for(int i=0; i<exclusions.size(); i++)
        vExclusions.push_back(exclusions[i].toUtf8().constData());
    megaApi->setExcludedNames(&vExclusions);

    if(preferences->lowerSizeLimit())
    {
        megaApi->setExclusionLowerSizeLimit(preferences->lowerSizeLimitValue() * pow((float)1024, preferences->lowerSizeLimitUnit()));
    }
    else
    {
        megaApi->setExclusionLowerSizeLimit(0);
    }

    if(preferences->upperSizeLimit())
    {
        megaApi->setExclusionUpperSizeLimit(preferences->upperSizeLimitValue() * pow((float)1024, preferences->upperSizeLimitUnit()));
    }
    else
    {
        megaApi->setExclusionUpperSizeLimit(0);
    }

    loggedIn();
    startSyncs();
}

void MegaApplication::unlink()
{
    //Reset fields that will be initialized again upon login
    stopUpdateTask();
    Platform::stopShellDispatcher();
    megaApi->logout();
}

void MegaApplication::showInfoMessage(QString message, QString title)
{
    MegaApi::log(MegaApi::LOG_LEVEL_INFO, message.toUtf8().constData());

    if(notificator)
    {
        #ifdef __APPLE__
            if(infoDialog && infoDialog->isVisible()) infoDialog->hide();
        #endif
        lastTrayMessage = message;
        notificator->notify(Notificator::Information, title, message,
                                    QIcon(QString::fromUtf8("://images/app_128.png")));
    }
    else QMessageBox::information(NULL, title, message);
}

void MegaApplication::showWarningMessage(QString message, QString title)
{
    MegaApi::log(MegaApi::LOG_LEVEL_WARNING, message.toUtf8().constData());

    if(!preferences->showNotifications()) return;
    if(notificator)
    {
        lastTrayMessage = message;
        notificator->notify(Notificator::Warning, title, message,
                                    QIcon(QString::fromUtf8("://images/app_128.png")));
    }
    else QMessageBox::warning(NULL, title, message);
}

void MegaApplication::showErrorMessage(QString message, QString title)
{
    MegaApi::log(MegaApi::LOG_LEVEL_ERROR, message.toUtf8().constData());
    if(notificator)
    {
        #ifdef __APPLE__
            if(infoDialog && infoDialog->isVisible()) infoDialog->hide();
        #endif
        notificator->notify(Notificator::Critical, title, message,
        QIcon(QString::fromUtf8("://images/app_128.png")));
    }
    else QMessageBox::critical(NULL, title, message);
}

void MegaApplication::showNotificationMessage(QString message, QString title)
{
    MegaApi::log(MegaApi::LOG_LEVEL_INFO, message.toUtf8().constData());

    if(!preferences->showNotifications()) return;
    if(notificator)
    {
        lastTrayMessage = message;
        notificator->notify(Notificator::Information, title, message,
                                    QIcon(QString::fromUtf8("://images/app_128.png")));
    }
}

//KB/s
void MegaApplication::setUploadLimit(int limit)
{
    if(limit<0) megaApi->setUploadLimit(-1);
    else megaApi->setUploadLimit(limit*1024);
}

void MegaApplication::startUpdateTask()
{
#if defined(WIN32) || defined(__APPLE__)
    if(!updateThread && preferences->canUpdate())
    {
        updateThread = new QThread();
        updateTask = new UpdateTask(megaApi);
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
    if(updateThread)
    {
        updateThread->quit();
        updateThread = NULL;
        updateTask = NULL;
    }
}

void MegaApplication::applyProxySettings()
{
    QNetworkProxy proxy(QNetworkProxy::NoProxy);
    MegaProxy *proxySettings = new MegaProxy();
    proxySettings->setProxyType(preferences->proxyType());

    if(preferences->proxyType() == MegaProxy::PROXY_CUSTOM)
    {
        int proxyProtocol = preferences->proxyProtocol();
        QString proxyString = preferences->proxyHostAndPort();
        switch(proxyProtocol)
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
        if(preferences->proxyRequiresAuth())
        {
            QString username = preferences->getProxyUsername();
            QString password = preferences->getProxyPassword();
            proxySettings->setCredentials(username.toUtf8().constData(), password.toUtf8().constData());

            proxy.setUser(preferences->getProxyUsername());
            proxy.setPassword(preferences->getProxyPassword());
        }
    }
    else if(preferences->proxyType() == MegaProxy::PROXY_AUTO)
    {
        MegaProxy* autoProxy = megaApi->getAutoProxySettings();
        delete proxySettings;
        proxySettings = autoProxy;

        if(proxySettings->getProxyType()==MegaProxy::PROXY_CUSTOM)
        {
            string sProxyURL = proxySettings->getProxyURL();
            QString proxyURL = QString::fromUtf8(sProxyURL.data());

            QStringList arguments = proxyURL.split(QString::fromAscii(":"));
            if(arguments.size() == 2)
            {
                proxy.setType(QNetworkProxy::HttpProxy);
                proxy.setHostName(arguments[0]);
                proxy.setPort(arguments[1].toInt());
            }
        }
    }

    megaApi->setProxySettings(proxySettings);
    megaApiLinks->setProxySettings(proxySettings);
    delete proxySettings;
    QNetworkProxy::setApplicationProxy(proxy);
}

void MegaApplication::showUpdatedMessage()
{
    updated = true;
}

void MegaApplication::updateUserStats()
{
    long long lastRequest = preferences->lastStatsRequest();
    if((QDateTime::currentMSecsSinceEpoch()-lastRequest) > Preferences::MIN_UPDATE_STATS_INTERVAL)
    {
        preferences->setLastStatsRequest(QDateTime::currentMSecsSinceEpoch());
        megaApi->getAccountDetails();
    }
}

void MegaApplication::addRecentFile(QString fileName, long long fileHandle, QString localPath, QString nodeKey)
{
    if(infoDialog)
        infoDialog->addRecentFile(fileName, fileHandle, localPath, nodeKey);
}

void MegaApplication::checkForUpdates()
{
    this->showInfoMessage(tr("Checking for updates..."));
    emit tryUpdate();
}

void MegaApplication::showTrayMenu(QPoint *point)
{
    if(trayGuestMenu && guestModeEnabled)
    {
        if(trayGuestMenu->isVisible())
            trayGuestMenu->close();
        QPoint p = point ? (*point)-QPoint(trayGuestMenu->sizeHint().width(), 0) : QCursor::pos();
        #ifdef __APPLE__
            trayGuestMenu->exec(p);
        #else
            trayGuestMenu->popup(p);
        #endif
    }
    else if(trayMenu && !infoOverQuota)
    {
        if(trayMenu->isVisible())
            trayMenu->close();
        QPoint p = point ? (*point)-QPoint(trayMenu->sizeHint().width(), 0) : QCursor::pos();
        #ifdef __APPLE__
            trayMenu->exec(p);
        #else
            trayMenu->popup(p);
        #endif
    }
    else if(trayOverQuotaMenu && infoOverQuota)
    {
        if(trayOverQuotaMenu->isVisible())
            trayOverQuotaMenu->close();
        QPoint p = point ? (*point)-QPoint(trayOverQuotaMenu->sizeHint().width(), 0) : QCursor::pos();
        #ifdef __APPLE__
            trayOverQuotaMenu->exec(p);
        #else
            trayOverQuotaMenu->popup(p);
        #endif
    }
}

void MegaApplication::toggleLogging()
{
    if(logger->isLogToFileEnabled())
    {
        logger->sendLogsToFile(false);
        MegaApi::setLogLevel(MegaApi::LOG_LEVEL_WARNING);
        showInfoMessage(tr("DEBUG mode disabled"));
    }
    else
    {
        logger->sendLogsToFile(true);
        MegaApi::setLogLevel(MegaApi::LOG_LEVEL_MAX);
        showInfoMessage(tr("DEBUG mode enabled. A log is being created in your desktop (MEGAsync.log)"));
        megaApi->log(MegaApi::LOG_LEVEL_INFO, QString::fromUtf8("Version string: %1   Version code: %2.%3   User-Agent: %4").arg(Preferences::VERSION_STRING)
                     .arg(Preferences::VERSION_CODE).arg(Preferences::BUILD_ID).arg(QString::fromUtf8(megaApi->getUserAgent())).toUtf8().constData());
    }
}

#if (QT_VERSION == 0x050500) && defined(_WIN32)
bool MegaApplication::eventFilter(QObject *o, QEvent *ev)
{
    QMenu *menu = dynamic_cast<QMenu *>(o);
    if(menu && menu->isVisible() && menu->isEnabled() && ev->type() == QEvent::MouseButtonRelease)
    {
        QMouseEvent * mouseEvent = dynamic_cast<QMouseEvent *>(ev);
        if(mouseEvent)
        {
            QAction *action = menu->actionAt(mouseEvent->pos());
            if(action && action->isEnabled())
            {
                action->trigger();
                menu->close();
                return true;
            }
        }
    }
    return false;
}
#endif

void MegaApplication::pauseSync()
{
    pauseTransfers(true);
}

void MegaApplication::resumeSync()
{
    pauseTransfers(false);
}

//Called when the "Import links" menu item is clicked
void MegaApplication::importLinks()
{
    if(pasteMegaLinksDialog)
    {
        pasteMegaLinksDialog->setVisible(true);
        pasteMegaLinksDialog->activateWindow();
        pasteMegaLinksDialog->raise();
        pasteMegaLinksDialog->setFocus();
        return;
    }

    if(importDialog)
    {
        importDialog->setVisible(true);
        importDialog->activateWindow();
        importDialog->raise();
        importDialog->setFocus();
        return;
    }

    //Show the dialog to paste public links
    pasteMegaLinksDialog = new PasteMegaLinksDialog();
    pasteMegaLinksDialog->exec();
    if(!pasteMegaLinksDialog)
    {
        return;
    }

    //If the dialog isn't accepted, return
    if(pasteMegaLinksDialog->result()!=QDialog::Accepted)
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
    LinkProcessor *linkProcessor = new LinkProcessor(megaApiLinks, linkList);

    //Open the import dialog
    importDialog = new ImportMegaLinksDialog(megaApiLinks, preferences, linkProcessor, guestModeEnabled);
    importDialog->exec();
    if(!importDialog)
    {
        return;
    }

    if(importDialog->result()!=QDialog::Accepted)
    {
        delete importDialog;
        importDialog = NULL;
        return;
    }

    //If the user wants to download some links, do it
    if(importDialog->shouldDownload())
    {
        if(!preferences->hasDefaultDownloadFolder())
        {
            preferences->setDownloadFolder(importDialog->getDownloadPath());
        }
        linkProcessor->downloadLinks(importDialog->getDownloadPath());
    }

    //If the user wants to import some links, do it
    if(!guestModeEnabled && importDialog->shouldImport())
    {
        connect(linkProcessor, SIGNAL(onLinkImportFinish()), this, SLOT(onLinkImportFinished()));
        connect(linkProcessor, SIGNAL(onDupplicateLink(QString, QString, long long)),
                this, SLOT(onDupplicateLink(QString, QString, long long)));
        linkProcessor->importLinks(importDialog->getImportPath());
    }
    //If importing links isn't needed, we can delete the link processor
    //It doesn't track transfers, only the importation of links
    else delete linkProcessor;

    delete importDialog;
    importDialog = NULL;

}

void MegaApplication::showChangeLog()
{
    if(changeLogDialog)
    {
        changeLogDialog->setVisible(true);
        changeLogDialog->activateWindow();
        changeLogDialog->raise();
        changeLogDialog->setFocus();
        return;
    }

    changeLogDialog = new ChangeLogDialog(Preferences::VERSION_STRING, Preferences::SDK_ID, Preferences::CHANGELOG);
    changeLogDialog->activateWindow();
    changeLogDialog->raise();
    changeLogDialog->show();
}

void MegaApplication::uploadActionClicked()
{
    #ifdef __APPLE__
         if(QSysInfo::MacintoshVersion >= QSysInfo::MV_10_7)
         {
                infoDialog->hide();
                QApplication::processEvents();
                QStringList files = MacXPlatform::multipleUpload(QCoreApplication::translate("ShellExtension", "Upload to MEGA"));
                if(files.size())
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

    if(multiUploadFileDialog)
    {
        #ifdef WIN32
            multiUploadFileDialog->showMinimized();
            multiUploadFileDialog->setWindowState(Qt::WindowActive);
            multiUploadFileDialog->showNormal();
        #endif

        multiUploadFileDialog->raise();
        multiUploadFileDialog->activateWindow();
        return;
    }

#if QT_VERSION < 0x050000
    QString defaultFolderPath = QDesktopServices::storageLocation(QDesktopServices::HomeLocation);
#else
    QString defaultFolderPath = QStandardPaths::standardLocations(QStandardPaths::HomeLocation)[0];
#endif

    multiUploadFileDialog = new MultiQFileDialog(NULL,
           QCoreApplication::translate("ShellExtension", "Upload to MEGA"),
           defaultFolderPath);

    int result = multiUploadFileDialog->exec();
    if(!multiUploadFileDialog)
    {
        return;
    }

    if(result == QDialog::Accepted)
    {
        QStringList files = multiUploadFileDialog->selectedFiles();
        if(files.size())
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
    if(appfinished) return;

    if(downloadNodeSelector)
    {
        downloadNodeSelector->setVisible(true);
        downloadNodeSelector->activateWindow();
        downloadNodeSelector->raise();
        downloadNodeSelector->setFocus();
        return;
    }

    downloadNodeSelector = new NodeSelector(megaApi, NodeSelector::DOWNLOAD_SELECT, NULL);
    int result = downloadNodeSelector->exec();
    if(!downloadNodeSelector)
    {
        return;
    }

    if(result != QDialog::Accepted)
    {
        delete downloadNodeSelector;
        downloadNodeSelector = NULL;
        return;
    }

    long long selectedMegaFolderHandle = downloadNodeSelector->getSelectedFolderHandle();
    MegaNode *selectedNode = megaApi->getNodeByHandle(selectedMegaFolderHandle);
    delete downloadNodeSelector;
    downloadNodeSelector = NULL;
    if(!selectedNode)
    {
        selectedMegaFolderHandle = mega::INVALID_HANDLE;
        return;
    }

    if(selectedNode)
    {
        downloadQueue.append(selectedNode);
        processDownloads();
    }
}

void MegaApplication::loginActionClicked()
{
    userAction(GuestWidget::LOGIN_CLICKED);
}

void MegaApplication::userAction(int action)
{
    if(preferences && guestModeEnabled)
    {
        if(setupWizard)
        {
            setupWizard->setVisible(true);
            setupWizard->raise();
            setupWizard->activateWindow();
            setupWizard->setFocus();
            return;
        }
        setupWizard = new SetupWizard(this);
        setupWizard->setModal(false);
        connect(setupWizard, SIGNAL(finished(int)), this, SLOT(setupWizardFinished()));
        setupWizard->goToStep(action);
        setupWizard->show();
    }
}

void MegaApplication::processDownloads()
{
    if(!downloadQueue.size())
    {
        return;
    }

    if(downloadFolderSelector)
    {
        downloadFolderSelector->setVisible(true);
        #ifdef WIN32
            downloadFolderSelector->showMinimized();
            downloadFolderSelector->setWindowState(Qt::WindowActive);
            downloadFolderSelector->showNormal();
        #endif
        downloadFolderSelector->raise();
        downloadFolderSelector->activateWindow();
        downloadFolderSelector->setFocus();
        return;
    }

    QString defaultPath = preferences->downloadFolder();
    if(preferences->hasDefaultDownloadFolder() && QFile(defaultPath).exists())
    {
        processDownloadQueue(defaultPath);
        return;
    }

    downloadFolderSelector = new DownloadFromMegaDialog(preferences->downloadFolder());
    #ifdef WIN32
        downloadFolderSelector->showMinimized();
        downloadFolderSelector->setWindowState(Qt::WindowActive);
        downloadFolderSelector->showNormal();
    #endif
    downloadFolderSelector->raise();
    downloadFolderSelector->activateWindow();
    downloadFolderSelector->setFocus();
    downloadFolderSelector->exec();
    if(!downloadFolderSelector)
    {
        return;
    }

    if(downloadFolderSelector->result()==QDialog::Accepted)
    {
        //If the dialog is accepted, get the destination node
        QString path = downloadFolderSelector->getPath();
        preferences->setHasDefaultDownloadFolder(downloadFolderSelector->isDefaultDownloadOption());
        preferences->setDownloadFolder(path);
        if(settingsDialog)
        {
            settingsDialog->loadSettings();
        }
        processDownloadQueue(path);
    }
    else
    {
        //If the dialog is rejected, cancel uploads
        downloadQueue.clear();
    }

    delete downloadFolderSelector;
    downloadFolderSelector = NULL;
    return;
}

void MegaApplication::logoutActionClicked()
{
    unlink();
}

//Called when the user wants to generate the public link for a node
void MegaApplication::copyFileLink(MegaHandle fileHandle, QString nodeKey)
{
    if(nodeKey.size())
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

    //Launch the creation of the import link, it will be handled in the "onRequestFinish" callback
    if(infoDialog) infoDialog->disableGetLink(true);
	megaApi->exportNode(megaApi->getNodeByHandle(fileHandle));
}

//Called when the user wants to upload a list of files and/or folders from the shell
void MegaApplication::shellUpload(QQueue<QString> newUploadQueue)
{    
    if(appfinished || !megaApi->isLoggedIn())
    {
        return;
    }

    //Append the list of files to the upload queue
	uploadQueue.append(newUploadQueue);

    //If the dialog to select the upload folder is active, return.
    //Files will be uploaded when the user selects the upload folder
    if(uploadFolderSelector)
    {
        uploadFolderSelector->setVisible(true);
        #ifdef WIN32
            uploadFolderSelector->showMinimized();
            uploadFolderSelector->setWindowState(Qt::WindowActive);
            uploadFolderSelector->showNormal();
        #endif
        uploadFolderSelector->raise();
        uploadFolderSelector->activateWindow();
        uploadFolderSelector->setFocus();
        return;
    }

    //If there is a default upload folder in the preferences
    MegaNode *node = megaApi->getNodeByHandle(preferences->uploadFolder());
    if(preferences->hasDefaultUploadFolder() && node)
	{
        //use it to upload the list of files
        processUploadQueue(node->getHandle());
        delete node;
		return;
	}

    uploadFolderSelector = new UploadToMegaDialog(megaApi);
    uploadFolderSelector->setDefaultFolder(preferences->uploadFolder());

    #ifdef WIN32
        uploadFolderSelector->showMinimized();
        uploadFolderSelector->setWindowState(Qt::WindowActive);
        uploadFolderSelector->showNormal();
    #endif
    uploadFolderSelector->raise();
    uploadFolderSelector->activateWindow();
    uploadFolderSelector->setFocus();
    uploadFolderSelector->exec();
    if(!uploadFolderSelector)
    {
        return;
    }

    if(uploadFolderSelector->result()==QDialog::Accepted)
    {
        //If the dialog is accepted, get the destination node
        MegaHandle nodeHandle = uploadFolderSelector->getSelectedHandle();
        preferences->setHasDefaultUploadFolder(uploadFolderSelector->isDefaultFolder());
        preferences->setUploadFolder(nodeHandle);
        if(settingsDialog)
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

void MegaApplication::shellExport(QQueue<QString> newExportQueue)
{
    if(appfinished || !megaApi->isLoggedIn())
    {
        return;
    }

    ExportProcessor *processor = new ExportProcessor(megaApi, newExportQueue);
    connect(processor, SIGNAL(onRequestLinksFinished()), this, SLOT(onRequestLinksFinished()));
    processor->requestLinks();
    exportOps++;
}

void MegaApplication::externalDownload(QQueue<MegaNode *> newDownloadQueue)
{
    downloadQueue.append(newDownloadQueue);
}

void MegaApplication::externalDownload(QString megaLink)
{
    pendingLinks.append(megaLink);
    megaApiLinks->getPublicNode(megaLink.toUtf8().constData());
}

void MegaApplication::internalDownload(long long handle)
{
    MegaNode *node = megaApi->getNodeByHandle(handle);
    if(!node)
    {
        return;
    }

    downloadQueue.append(node);
    processDownloads();
}

void MegaApplication::syncFolder(long long handle)
{
    if(infoDialog)
    {
        infoDialog->addSync(handle);
    }
}

//Called when the link import finishes
void MegaApplication::onLinkImportFinished()
{
	LinkProcessor *linkProcessor = ((LinkProcessor *)QObject::sender());
	preferences->setImportFolder(linkProcessor->getImportParentFolder());
    linkProcessor->deleteLater();
}

void MegaApplication::onRequestLinksFinished()
{
    ExportProcessor *exportProcessor = ((ExportProcessor *)QObject::sender());
    QStringList links = exportProcessor->getValidLinks();
    if(!links.size())
    {
        exportOps--;
        return;
    }
    QString linkForClipboard(links.join(QChar::fromAscii('\n')));
    QApplication::clipboard()->setText(linkForClipboard);
    if(links.size()==1) showInfoMessage(tr("The link has been copied to the clipboard"));
    else showInfoMessage(tr("The links have been copied to the clipboard"));
    exportProcessor->deleteLater();
    exportOps--;
}

void MegaApplication::onUpdateCompleted()
{
    if(trayMenu)
    {
        updateAction->setText(tr("About") + QString::fromUtf8(" ") + QCoreApplication::applicationName());
    }

    if(trayOverQuotaMenu)
    {
        updateActionOverquota->setText(tr("About") + QString::fromUtf8(" ") + QCoreApplication::applicationName());
    }

    updateAvailable = false;
    rebootApplication();
}

void MegaApplication::onUpdateAvailable(bool requested)
{
    updateAvailable = true;

    if(trayMenu)
    {
        updateAction->setText(tr("Install update"));
    }

    if(trayOverQuotaMenu)
    {
        updateActionOverquota->setText(tr("Install update"));
    }

    if(settingsDialog)
        settingsDialog->setUpdateAvailable(true);

    if(requested)
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
    if(requested)
        showInfoMessage(tr("Update available. Downloading..."));
}

void MegaApplication::onUpdateNotFound(bool requested)
{
    if(requested)
    {
        if(!updateAvailable)
            showInfoMessage(tr("No update available at this time"));
        else
            showInfoMessage(tr("There was a problem installing the update. Please try again later or download the last version from:\nhttps://mega.co.nz/#sync")
                            .replace(QString::fromUtf8("mega.co.nz"), QString::fromUtf8("mega.nz")));
    }
}

void MegaApplication::onUpdateError()
{
    showInfoMessage(tr("There was a problem installing the update. Please try again later or download the last version from:\nhttps://mega.co.nz/#sync")
                    .replace(QString::fromUtf8("mega.co.nz"), QString::fromUtf8("mega.nz")));
}

//Called when users click in the tray icon
void MegaApplication::trayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    megaApi->retryPendingConnections();

    if(reason == QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::Context)
    {
        if(!infoDialog)
        {
            if(setupWizard)
            {
                setupWizard->setVisible(true);
                setupWizard->raise();
                setupWizard->activateWindow();
            }
            else if(reason == QSystemTrayIcon::Trigger)
                showInfoMessage(tr("Logging in..."));

            return;
        }

#ifdef _WIN32
        if(reason == QSystemTrayIcon::Context)
        {
            return;
        }
#endif

#ifndef __APPLE__
        if(isLinux)
        {
            if(showStatusAction)
            {
                initialMenu->removeAction(showStatusAction);

                delete showStatusAction;
                showStatusAction = NULL;
            }

            if(trayMenu && trayMenu->isVisible())
                trayMenu->close();

            if(trayOverQuotaMenu && trayOverQuotaMenu->isVisible())
                trayOverQuotaMenu->close();
        }
        infoDialogTimer->start(200);
#else
        showInfoDialog();
#endif
    }
#ifndef __APPLE__
    else if(reason == QSystemTrayIcon::DoubleClick)
    {
        if(!infoDialog)
        {
            if(setupWizard)
            {
                setupWizard->setVisible(true);
                setupWizard->raise();
                setupWizard->activateWindow();
            }
            else showInfoMessage(tr("Logging in..."));

            return;
        }

        int i;
        for(i = 0; i < preferences->getNumSyncedFolders(); i++)
        {
            if(preferences->isFolderActive(i))
                break;
        }
        if(i == preferences->getNumSyncedFolders())
            return;

        infoDialogTimer->stop();
        infoDialog->hide();
        QString localFolderPath = preferences->getLocalFolder(i);
        if(!localFolderPath.isEmpty())
            QDesktopServices::openUrl(QUrl::fromLocalFile(localFolderPath));
    }
    else if(reason == QSystemTrayIcon::MiddleClick)
    {
        showTrayMenu();
    }
#endif
}

void MegaApplication::onMessageClicked()
{
    if(lastTrayMessage == tr("A new version of MEGAsync is available! Click on this message to install it"))
        triggerInstallUpdate();
    else
        trayIconActivated(QSystemTrayIcon::Trigger);
}

//Called when the user wants to open the settings dialog
void MegaApplication::openSettings(int tab)
{
    if(settingsDialog)
    {
        //If the dialog is active
		if(settingsDialog->isVisible())
		{
            //and visible -> show it
            if(infoOverQuota)
            {
                settingsDialog->setOverQuotaMode(true);
            }
            else
            {
                settingsDialog->setOverQuotaMode(false);
                settingsDialog->openSettingsTab(tab);
            }
            settingsDialog->loadSettings();
            settingsDialog->raise();
			settingsDialog->activateWindow();
			return;
		}

        //Otherwise, delete it
        delete settingsDialog;
        settingsDialog = NULL;
	}

    //Show a new settings dialog
    settingsDialog = new SettingsDialog(this);
    if(infoOverQuota)
    {
        settingsDialog->setOverQuotaMode(true);
    }
    else
    {
        if(guestModeEnabled)
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
    settingsDialog->raise();
    settingsDialog->activateWindow();
}

void MegaApplication::changeProxy()
{
    bool proxyOnly = true;

    if(megaApi)
    {
        proxyOnly = !megaApi->isLoggedIn();
        megaApi->retryPendingConnections();
    }

    if(settingsDialog)
    {            
        settingsDialog->setProxyOnly(proxyOnly);

        //If the dialog is active
        if(settingsDialog->isVisible())
        {
            if(isLinux && !proxyOnly)
            {
                if(infoOverQuota)
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
            settingsDialog->raise();
            settingsDialog->activateWindow();
            return;
        }

        //Otherwise, delete it
        delete settingsDialog;
        settingsDialog = NULL;
    }

    //Show a new settings dialog
    settingsDialog = new SettingsDialog(this, proxyOnly);
    if(isLinux && !proxyOnly)
    {
        if(infoOverQuota)
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
void MegaApplication::createTrayIcon()
{
    if(!trayMenu)
    {
        trayMenu = new QMenu();

#if (QT_VERSION == 0x050500) && defined(_WIN32)
        trayMenu->installEventFilter(this);
#endif

        #ifndef __APPLE__
            trayMenu->setStyleSheet(QString::fromAscii(
                    "QMenu {background-color: white; border: 2px solid #B8B8B8; padding: 5px; border-radius: 5px;} "
                    "QMenu::item {background-color: white; color: black;} "
                    "QMenu::item:selected {background-color: rgb(242, 242, 242);}"));
        #endif
    }
    else
    {
        QList<QAction *> actions = trayMenu->actions();
        for(int i=0; i<actions.size(); i++)
        {
            trayMenu->removeAction(actions[i]);
        }
    }

    if(exitAction)
    {
        exitAction->deleteLater();
        exitAction = NULL;
    }

#ifndef __APPLE__
    exitAction = new QAction(tr("Exit"), this);
#else
    exitAction = new QAction(tr("Quit"), this);
#endif
    connect(exitAction, SIGNAL(triggered()), this, SLOT(exitApplication()));

    if(settingsAction)
    {
        settingsAction->deleteLater();
        settingsAction = NULL;
    }

#ifndef __APPLE__
    settingsAction = new QAction(tr("Settings"), this);
#else
    settingsAction = new QAction(tr("Preferences"), this);
#endif
    connect(settingsAction, SIGNAL(triggered()), this, SLOT(openSettings()));

    if(importLinksAction)
    {
        importLinksAction->deleteLater();
        importLinksAction = NULL;
    }

    importLinksAction = new QAction(tr("Import links"), this);
    connect(importLinksAction, SIGNAL(triggered()), this, SLOT(importLinks()));

    if(uploadAction)
    {
        uploadAction->deleteLater();
        uploadAction = NULL;
    }

    uploadAction = new QAction(tr("Upload to MEGA"), this);
    connect(uploadAction, SIGNAL(triggered()), this, SLOT(uploadActionClicked()));

    if(downloadAction)
    {
        downloadAction->deleteLater();
        downloadAction = NULL;
    }

    downloadAction = new QAction(tr("Download from MEGA"), this);
    connect(downloadAction, SIGNAL(triggered()), this, SLOT(downloadActionClicked()));

    if(updateAction)
    {
        updateAction->deleteLater();
        updateAction = NULL;
    }

    if(updateAvailable)
    {
        updateAction = new QAction(tr("Install update"), this);
    }
    else
    {
        updateAction = new QAction(tr("About") + QString::fromUtf8(" ") + QCoreApplication::applicationName(), this);
#ifndef __APPLE__
        updateAction->setIcon(QIcon(QString::fromUtf8("://images/check_mega_version.png")));
        updateAction->setIconVisibleInMenu(true);
#endif
    }
    connect(updateAction, SIGNAL(triggered()), this, SLOT(onInstallUpdateClicked()));

    trayMenu->addAction(updateAction);
    trayMenu->addSeparator();
	trayMenu->addAction(importLinksAction);
    trayMenu->addAction(uploadAction);
    trayMenu->addAction(downloadAction);
    trayMenu->addAction(settingsAction);
    trayMenu->addSeparator();
    trayMenu->addAction(exitAction);

    if (isLinux)
    {
        if(showStatusAction)
        {
            showStatusAction->deleteLater();
            showStatusAction = NULL;
        }

        showStatusAction = new QAction(tr("Show status"), this);
        connect(showStatusAction, SIGNAL(triggered()), this, SLOT(showInfoDialog()));

        initialMenu->insertAction(changeProxyAction, showStatusAction);
    }
    else
    {
        #ifdef _WIN32
            trayIcon->setContextMenu(windowsMenu);
        #else
            trayIcon->setContextMenu(&emptyMenu);
        #endif

        #ifndef __APPLE__
            #ifdef _WIN32
                trayIcon->setIcon(QIcon(QString::fromAscii("://images/tray_sync.ico")));
            #else
                trayIcon->setIcon(QIcon(QString::fromAscii("://images/22_synching.png")));
            #endif
        #else
            trayIcon->setIcon(QIcon(QString::fromAscii("://images/icon_syncing_mac.png")),QIcon(QString::fromAscii("://images/icon_syncing_mac_white.png")));
            if(!scanningTimer->isActive())
            {
                scanningAnimationIndex = 1;
                scanningTimer->start();
            }
        #endif
    }
    trayIcon->setToolTip(QCoreApplication::applicationName() + QString::fromAscii(" ") + Preferences::VERSION_STRING + QString::fromAscii("\n") + tr("Starting"));
}

void MegaApplication::createOverQuotaMenu()
{
    if(!trayOverQuotaMenu)
    {
        trayOverQuotaMenu = new QMenu();

#if (QT_VERSION == 0x050500) && defined(_WIN32)
        trayOverQuotaMenu->installEventFilter(this);
#endif

        #ifndef __APPLE__
            trayOverQuotaMenu->setStyleSheet(QString::fromAscii(
                    "QMenu {background-color: white; border: 2px solid #B8B8B8; padding: 5px; border-radius: 5px;} "
                    "QMenu::item {background-color: white; color: black;} "
                    "QMenu::item:selected {background-color: rgb(242, 242, 242);}"));
        #endif
    }
    else
    {
        QList<QAction *> actions = trayOverQuotaMenu->actions();
        for(int i=0; i<actions.size(); i++)
        {
            trayOverQuotaMenu->removeAction(actions[i]);
        }
    }

    if(exitActionOverquota)
    {
        exitActionOverquota->deleteLater();
        exitActionOverquota = NULL;
    }

#ifndef __APPLE__
    exitActionOverquota = new QAction(tr("Exit"), this);
#else
    exitActionOverquota = new QAction(tr("Quit"), this);
#endif
    connect(exitActionOverquota, SIGNAL(triggered()), this, SLOT(exitApplication()));


    if(logoutActionOverquota)
    {
        logoutActionOverquota->deleteLater();
        logoutActionOverquota = NULL;
    }

    logoutActionOverquota = new QAction(tr("Logout"), this);
    connect(logoutActionOverquota, SIGNAL(triggered()), this, SLOT(logoutActionClicked()));


    if(settingsActionOverquota)
    {
        settingsActionOverquota->deleteLater();
        settingsActionOverquota = NULL;
    }

#ifndef __APPLE__
    settingsActionOverquota = new QAction(tr("Settings"), this);
#else
    settingsActionOverquota = new QAction(tr("Preferences"), this);
#endif
    connect(settingsActionOverquota, SIGNAL(triggered()), this, SLOT(openSettings()));


    if(updateActionOverquota)
    {
        updateActionOverquota->deleteLater();
        updateActionOverquota = NULL;
    }

    if(updateAvailable)
    {
        updateActionOverquota = new QAction(tr("Install update"), this);
    }
    else
    {
        updateActionOverquota = new QAction(tr("About") + QString::fromUtf8(" ") + QCoreApplication::applicationName(), this);

#ifndef __APPLE__
        updateActionOverquota->setIcon(QIcon(QString::fromAscii("://images/check_mega_version.png")));
        updateActionOverquota->setIconVisibleInMenu(true);
#endif
    }
    connect(updateActionOverquota, SIGNAL(triggered()), this, SLOT(onInstallUpdateClicked()));

    trayOverQuotaMenu->addAction(updateActionOverquota);
    trayOverQuotaMenu->addSeparator();
    trayOverQuotaMenu->addAction(settingsActionOverquota);
    trayOverQuotaMenu->addAction(logoutActionOverquota);
    trayOverQuotaMenu->addSeparator();
    trayOverQuotaMenu->addAction(exitActionOverquota);
}

void MegaApplication::createGuestMenu()
{
    if(!trayGuestMenu)
    {
        trayGuestMenu = new QMenu();
        #ifndef __APPLE__
            trayGuestMenu->setStyleSheet(QString::fromAscii(
                    "QMenu {background-color: white; border: 2px solid #B8B8B8; padding: 5px; border-radius: 5px;} "
                    "QMenu::item {background-color: white; color: black;} "
                    "QMenu::item:selected {background-color: rgb(242, 242, 242);}"));
        #endif
    }
    else
    {
        QList<QAction *> actions = trayGuestMenu->actions();
        for(int i=0; i<actions.size(); i++)
        {
            trayGuestMenu->removeAction(actions[i]);
        }
    }

    if(exitGuestAction)
    {
        exitGuestAction->deleteLater();
        exitGuestAction = NULL;
    }

#ifndef __APPLE__
    exitGuestAction = new QAction(tr("Exit"), this);
#else
    exitGuestAction = new QAction(tr("Quit"), this);
#endif
    connect(exitGuestAction, SIGNAL(triggered()), this, SLOT(exitApplication()));

    if(updateGuestAction)
    {
        updateGuestAction->deleteLater();
        updateGuestAction = NULL;
    }

    if(updateAvailable)
    {
        updateGuestAction = new QAction(tr("Install update"), this);
        updateGuestAction->setEnabled(true);
    }
    else
    {
        updateGuestAction = new QAction(QCoreApplication::applicationName() + QString::fromAscii(" ") + Preferences::VERSION_STRING +
                                  QString::fromAscii(" (") + Preferences::SDK_ID + QString::fromAscii(")"), this);
#ifndef __APPLE__
        updateGuestAction->setIcon(QIcon(QString::fromAscii("://images/check_mega_version.png")));
        updateGuestAction->setIconVisibleInMenu(true);
#endif
        updateGuestAction->setEnabled(false);
    }
    connect(updateGuestAction, SIGNAL(triggered()), this, SLOT(onInstallUpdateClicked()));

    if(importLinksGuestAction)
    {
        importLinksGuestAction->deleteLater();
        importLinksGuestAction = NULL;
    }

    importLinksGuestAction = new QAction(tr("Import links"), this);
    connect(importLinksGuestAction, SIGNAL(triggered()), this, SLOT(importLinks()));

    if(settingsActionGuest)
    {
        settingsActionGuest->deleteLater();
        settingsActionGuest = NULL;
    }

    #ifndef __APPLE__
        settingsActionGuest = new QAction(tr("Settings"), this);
    #else
        settingsActionGuest = new QAction(tr("Preferences"), this);
    #endif
    connect(settingsActionGuest, SIGNAL(triggered()), this, SLOT(changeProxy()));

    if(loginAction)
    {
        loginAction->deleteLater();
        loginAction = NULL;
    }

    loginAction = new QAction(tr("Login"), this);
    connect(loginAction, SIGNAL(triggered()), this, SLOT(loginActionClicked()));

    trayGuestMenu->addAction(updateActionOverquota);
    trayGuestMenu->addSeparator();
    trayGuestMenu->addAction(importLinksGuestAction);
    trayGuestMenu->addAction(settingsActionGuest);
    trayGuestMenu->addAction(loginAction);
    trayGuestMenu->addSeparator();
    trayGuestMenu->addAction(exitGuestAction);
}

//Called when a request is about to start
void MegaApplication::onRequestStart(MegaApi* , MegaRequest *request)
{
    int type = request->getType();
    if(type == MegaRequest::TYPE_LOGIN)
    {
        connectivityTimer->start();
    }
}

//Called when a request has finished
void MegaApplication::onRequestFinish(MegaApi*, MegaRequest *request, MegaError* e)
{
    if(e->getErrorCode() == MegaError::API_EOVERQUOTA)
    {
        //Cancel pending uploads and disable syncs
        disableSyncs();
        if(!infoOverQuota)
        {
            infoOverQuota = new InfoOverQuotaDialog(this);

            preferences->setUsedStorage(preferences->totalStorage());
            megaApi->getAccountDetails();

            if(trayMenu && trayMenu->isVisible())
            {
                trayMenu->close();
            }

            if(infoDialog && infoDialog->isVisible())
            {
                infoDialog->hide();
            }

            showInfoDialog();
        }

        if(settingsDialog)
        {
            delete settingsDialog;
            settingsDialog = NULL;
        }

        megaApi->cancelTransfers(MegaTransfer::TYPE_UPLOAD);
        onGlobalSyncStateChanged(megaApi);
    }

    switch (request->getType()) {
	case MegaRequest::TYPE_EXPORT:
	{
        if(!exportOps && e->getErrorCode() == MegaError::API_OK)
		{
            //A public link has been created, put it in the clipboard and inform users
            QString linkForClipboard(QString::fromUtf8(request->getLink()));
            QApplication::clipboard()->setText(linkForClipboard);
            showInfoMessage(tr("The link has been copied to the clipboard"));
		}

        if(e->getErrorCode() != MegaError::API_OK)
            showErrorMessage(tr("Error getting link: ") + QString::fromUtf8(" ") + QCoreApplication::translate("MegaError", e->getErrorString()));

        if(infoDialog) infoDialog->disableGetLink(false);
		break;
	}
	case MegaRequest::TYPE_LOGIN:
	{
		connectivityTimer->stop();

        //This prevents to handle logins in the initial setup wizard
        if(preferences->logged())
		{
            int errorCode = e->getErrorCode();
            if(errorCode == MegaError::API_OK)
			{
                const char *session = megaApi->dumpSession();
                if(session)
                {
                    QString sessionKey = QString::fromUtf8(session);
                    preferences->setSession(sessionKey);
                    delete session;

                    //Successful login, fetch nodes
                    megaApi->fetchNodes();
                    break;
                }
			}
            else if(errorCode == MegaError::API_EBLOCKED)
            {
                QMessageBox::critical(NULL, tr("MEGAsync"), tr("Your account has been blocked. Please contact support@mega.co.nz"));
            }
            else if(errorCode != MegaError::API_ESID && errorCode != MegaError::API_ESSL)
            //Invalid session or public key, already managed in TYPE_LOGOUT
            {
                QMessageBox::warning(NULL, tr("MEGAsync"), tr("Login error: %1").arg(QCoreApplication::translate("MegaError", e->getErrorString())));
            }

            //Wrong login -> logout
            unlink();
		}
		break;
	}
    case MegaRequest::TYPE_LOGOUT:
    {
        int errorCode = e->getErrorCode();
        if(errorCode)
        {
            if(errorCode == MegaError::API_ESID)
            {
                QMessageBox::information(NULL, QString::fromAscii("MEGAsync"), tr("You have been logged out on this computer from another location"));
            }
            else if(errorCode == MegaError::API_ESSL)
            {
                QMessageBox::critical(NULL, QString::fromAscii("MEGAsync"), tr("Our SSL key can't be verified. You could be affected by a man-in-the-middle attack or your antivirus software could be intercepting your communications and causing this problem. Please disable it and try again."));
            }
            else
            {
                QMessageBox::information(NULL, QString::fromAscii("MEGAsync"), tr("You have been logged out because of this error: %1")
                                         .arg(QCoreApplication::translate("MegaError", e->getErrorString())));
            }
            unlink();
        }

        if(preferences && preferences->logged())
        {
            preferences->unlink();
            closeDialogs();
            refreshTrayIcon();
            preferences->setFirstStartDone();
            start();
        }
        break;
    }
	case MegaRequest::TYPE_FETCH_NODES:
	{
        //This prevents to handle node requests in the initial setup wizard
        if(preferences->logged())
		{
			if(e->getErrorCode() == MegaError::API_OK)
			{
                MegaNode *rootNode = megaApi->getRootNode();
                if(rootNode)
                {
                    delete rootNode;

                    //If we have got the filesystem, start the app
                    Preferences *preferences = Preferences::instance();
                    if(preferences->logged() && preferences->wasPaused())
                        pauseTransfers(true);

                    loggedIn();
                    restoreSyncs();
                }
                else
                {
                    QMessageBox::warning(NULL, tr("Error"), tr("Unable to get the filesystem.\n"
                                                               "Please, try again. If the problem persists "
                                                               "please contact bug@mega.co.nz"), QMessageBox::Ok);
                    preferences->setCrashed(true);
                    preferences->unlink();
                    rebootApplication(false);
                }
			}
            else
            {
                MegaApi::log(MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("Error fetching nodes: %1")
                             .arg(QString::fromUtf8(e->getErrorString())).toUtf8().constData());
                QMessageBox::warning(NULL, tr("Error"), QCoreApplication::translate("MegaError", e->getErrorString()), QMessageBox::Ok);
                unlink();
            }
		}

		break;
	}
    case MegaRequest::TYPE_ACCOUNT_DETAILS:
    {
        if(!preferences->logged()) break;

		if(e->getErrorCode() != MegaError::API_OK)
			break;

        //Account details retrieved, update the preferences and the information dialog
        MegaAccountDetails *details = request->getMegaAccountDetails();
        preferences->setAccountType(details->getProLevel());
        preferences->setTotalStorage(details->getStorageMax());
        preferences->setUsedStorage(details->getStorageUsed());
        preferences->setTotalBandwidth(details->getTransferMax());
        preferences->setUsedBandwidth(details->getTransferOwnUsed());

        MegaNode *root = megaApi->getRootNode();
        MegaHandle rootHandle = root->getHandle();
        preferences->setCloudDriveStorage(details->getStorageUsed(rootHandle));
        preferences->setCloudDriveFiles(details->getNumFiles(rootHandle));
        preferences->setCloudDriveFolders(details->getNumFolders(rootHandle));
        delete root;

        MegaNode *inbox = megaApi->getInboxNode();
        MegaHandle inboxHandle = inbox->getHandle();
        preferences->setInboxStorage(details->getStorageUsed(inboxHandle));
        preferences->setInboxFiles(details->getNumFiles(inboxHandle));
        preferences->setInboxFolders(details->getNumFolders(inboxHandle));
        delete inbox;

        MegaNode *rubbish = megaApi->getRubbishNode();
        MegaHandle rubbishHandle = rubbish->getHandle();
        preferences->setRubbishStorage(details->getStorageUsed(rubbishHandle));
        preferences->setRubbishFiles(details->getNumFiles(rubbishHandle));
        preferences->setRubbishFolders(details->getNumFolders(rubbishHandle));
        delete rubbish;

        long long inShareSize = 0, inShareFiles = 0, inShareFolders  = 0;
        MegaNodeList *inShares = megaApi->getInShares();
        for(int i=0; i<inShares->size(); i++)
        {
            MegaNode *node = inShares->get(i);
            if(!node)
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

        if(infoOverQuota && preferences->usedStorage() < preferences->totalStorage())
        {
            if(settingsDialog)
            {
                settingsDialog->setOverQuotaMode(false);
            }

            infoOverQuota->deleteLater();
            infoOverQuota = NULL;

            if(trayOverQuotaMenu && trayOverQuotaMenu->isVisible())
            {
                trayOverQuotaMenu->close();
            }

            restoreSyncs();
            onGlobalSyncStateChanged(megaApi);
        }

        if(infoDialog) infoDialog->setUsage();
        if(infoOverQuota) infoOverQuota->setUsage();
        if(settingsDialog) settingsDialog->refreshAccountDetails();
        delete details;
        break;
    }
    case MegaRequest::TYPE_PAUSE_TRANSFERS:
    {
        paused = request->getFlag();
        preferences->setWasPaused(paused);
        onGlobalSyncStateChanged(megaApi);
        break;
    }
    case MegaRequest::TYPE_ADD_SYNC:
    {
        for(int i=preferences->getNumSyncedFolders()-1; i>=0; i--)
        {
            if((request->getNodeHandle() == preferences->getMegaFolderHandle(i)))
            {
                if(e->getErrorCode() != MegaError::API_OK)
                {
                    QString localFolder = preferences->getLocalFolder(i);
                    MegaNode *node = megaApi->getNodeByHandle(preferences->getMegaFolderHandle(i));
                    const char *nodePath = megaApi->getNodePath(node);
                    delete node;

                    if(!QFileInfo(localFolder).isDir())
                    {
                        showErrorMessage(tr("Your sync \"%1\" has been disabled because the local folder doesn't exist")
                                         .arg(preferences->getSyncName(i)));
                    }
                    else if(nodePath && QString::fromUtf8(nodePath).startsWith(QString::fromUtf8("//bin")))
                    {
                        showErrorMessage(tr("Your sync \"%1\" has been disabled because the remote folder is in the rubbish bin")
                                         .arg(preferences->getSyncName(i)));
                    }
                    else if(!nodePath || preferences->getMegaFolder(i).compare(QString::fromUtf8(nodePath)))
                    {
                        showErrorMessage(tr("Your sync \"%1\" has been disabled because the remote folder doesn't exist")
                                         .arg(preferences->getSyncName(i)));
                    }
                    else if(e->getErrorCode() == MegaError::API_EFAILED)
                    {
#ifdef WIN32
                        WCHAR VBoxSharedFolderFS[] = L"VBoxSharedFolderFS";
                        string path, fsname;
                        path.resize(MAX_PATH * sizeof(WCHAR));
                        fsname.resize(MAX_PATH * sizeof(WCHAR));
                        if(GetVolumePathNameW((LPCWSTR)localFolder.utf16(), (LPWSTR)path.data(), MAX_PATH)
                            && GetVolumeInformationW((LPCWSTR)path.data(), NULL, 0, NULL, NULL, NULL, (LPWSTR)fsname.data(), MAX_PATH)
                            && !memcmp(fsname.data(), VBoxSharedFolderFS, sizeof(VBoxSharedFolderFS)))
                        {
                                    QMessageBox::critical(NULL, tr("MEGAsync"),
                                        tr("Your sync \"%1\" has been disabled because the synchronization of VirtualBox shared folders is not supported due to deficiencies in that filesystem.")
                                        .arg(preferences->getSyncName(i)));


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
                    else if(e->getErrorCode() == MegaError::API_EACCESS)
                    {
                        showErrorMessage(tr("Your sync \"%1\" has been disabled. The remote folder (or part of it) doesn't have full access")
                                         .arg(preferences->getSyncName(i)));
                    }
                    else if(e->getErrorCode() != MegaError::API_ENOENT) // Managed in onNodesUpdate
                    {
                        showErrorMessage(QCoreApplication::translate("MegaError", e->getErrorString()));
                    }

                    delete[] nodePath;

                    MegaApi::log(MegaApi::LOG_LEVEL_ERROR, "Error adding sync");
                    Platform::syncFolderRemoved(localFolder, preferences->getSyncName(i));
                    preferences->setSyncState(i, false);
                    openSettings(SettingsDialog::SYNCS_TAB);
                    if(settingsDialog)
                        settingsDialog->loadSettings();
                }
                else
                {
                    preferences->setLocalFingerprint(i, request->getNumber());
                    if(!isFirstSyncDone && !preferences->isFirstSyncDone())
                    {
                        megaApi->sendEvent(99501, "MEGAsync first sync");
                        isFirstSyncDone = true;
                    }

#ifdef _WIN32
                    QString debrisPath = QDir::toNativeSeparators(preferences->getLocalFolder(i) +
                            QDir::separator() + QString::fromAscii(MEGA_DEBRIS_FOLDER));

                    WIN32_FILE_ATTRIBUTE_DATA fad;
                    if (GetFileAttributesExW((LPCWSTR)debrisPath.utf16(), GetFileExInfoStandard, &fad))
                        SetFileAttributesW((LPCWSTR)debrisPath.utf16(), fad.dwFileAttributes | FILE_ATTRIBUTE_HIDDEN);
#endif

                }
                break;
            }
        }

        if(infoDialog)
        {
            MegaApi::log(MegaApi::LOG_LEVEL_INFO, "Sync added");
            infoDialog->updateSyncsButton();
        }
        break;
    }
    case MegaRequest::TYPE_REMOVE_SYNC:
    {
        if(e->getErrorCode() == MegaError::API_OK)
        {
            QString syncPath = QString::fromUtf8(request->getFile());

            #ifdef WIN32
            if(syncPath.startsWith(QString::fromAscii("\\\\?\\")))
                syncPath = syncPath.mid(4);
            #endif

            Platform::notifyItemChange(syncPath);
        }
        if(infoDialog) infoDialog->updateSyncsButton();
        if(settingsDialog) settingsDialog->loadSettings();
        onGlobalSyncStateChanged(megaApi);
        break;
    }
    case MegaRequest::TYPE_GET_SESSION_TRANSFER_URL:
    {
        QDesktopServices::openUrl(QUrl(QString::fromUtf8(request->getLink())));
        break;
    }
    case MegaRequest::TYPE_GET_PUBLIC_NODE:
    {
        QString link = QString::fromUtf8(request->getLink());
        if(pendingLinks.contains(link))
        {
            pendingLinks.removeOne(link);
            if(e->getErrorCode() == MegaError::API_OK)
            {
                downloadQueue.append(request->getPublicMegaNode());
                processDownloads();
            }
            else
            {
                showErrorMessage(tr("Error getting link information"));
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
void MegaApplication::onTransferStart(MegaApi *, MegaTransfer *transfer)
{
    if(appfinished) return;

    if(infoDialog && !totalUploadSize && !totalDownloadSize)
    {
        infoDialog->setWaiting(true);
        onGlobalSyncStateChanged(megaApi);
    }

    //Update statics
	if(transfer->getType() == MegaTransfer::TYPE_DOWNLOAD)
	{
		downloadSpeed = 0;
		totalDownloadSize += transfer->getTotalBytes();
	}
	else
	{
		uploadSpeed = 0;
		totalUploadSize += transfer->getTotalBytes();
	}

    //Send statics to the information dialog
    if(infoDialog)
    {
        infoDialog->setTotalTransferSize(totalDownloadSize, totalUploadSize);
        infoDialog->setTransferSpeeds(downloadSpeed, uploadSpeed);
        infoDialog->updateTransfers();
    }
}

//Called when there is a temporal problem in a request
void MegaApplication::onRequestTemporaryError(MegaApi *, MegaRequest *, MegaError* )
{
}

//Called when a transfer has finished
void MegaApplication::onTransferFinish(MegaApi* , MegaTransfer *transfer, MegaError* e)
{
	if(appfinished) return;

    if(e->getErrorCode() == MegaError::API_EOVERQUOTA)
    {
        //Cancel pending uploads and disable syncs
        disableSyncs();
        if(!infoOverQuota)
        {
            infoOverQuota = new InfoOverQuotaDialog(this);

            preferences->setUsedStorage(preferences->totalStorage());
            megaApi->getAccountDetails();

            if(trayMenu && trayMenu->isVisible())
            {
                trayMenu->close();
            }

            if(infoDialog && infoDialog->isVisible())
            {
                infoDialog->hide();
            }

            showInfoDialog();
        }

        if(settingsDialog)
        {
            delete settingsDialog;
            settingsDialog = NULL;
        }

        megaApi->cancelTransfers(MegaTransfer::TYPE_UPLOAD);
        onGlobalSyncStateChanged(megaApi);
    }

    if(e->getErrorCode() == MegaError::API_OK
            && transfer->isSyncTransfer()
            && !isFirstFileSynced
            && !preferences->isFirstFileSynced())
    {
        megaApi->sendEvent(99502, "MEGAsync first synced file");
        isFirstFileSynced = true;
    }

	//Update statics
	if(transfer->getType()==MegaTransfer::TYPE_DOWNLOAD)
	{
		totalDownloadedSize += transfer->getDeltaSize();
		downloadSpeed = transfer->getSpeed();

        //Show the transfer in the "recently updated" list
        if(e->getErrorCode() == MegaError::API_OK)
        {
            QString localPath = QString::fromUtf8(transfer->getPath());
            #ifdef WIN32
                if(localPath.startsWith(QString::fromAscii("\\\\?\\"))) localPath = localPath.mid(4);
            #endif

            MegaNode *node = transfer->getPublicMegaNode();
            QString publicKey;
            if(node)
            {
                const char* key = node->getBase64Key();
                publicKey = QString::fromUtf8(key);
                delete [] key;
                delete node;
            }
            addRecentFile(QString::fromUtf8(transfer->getFileName()), transfer->getNodeHandle(), localPath, publicKey);
        }
	}
	else
	{
		totalUploadedSize += transfer->getDeltaSize();
		uploadSpeed = transfer->getSpeed();

        //Here the file isn't added to the "recently updated" list,
        //because the file isn't in the destination folder yet.
        //The SDK still has to put the new node.
        //onNodes update will be called with node->tag == transfer->getTag()
        //so we save the path of the file to show it later
        if((e->getErrorCode() == MegaError::API_OK) && (!transfer->isSyncTransfer()))
        {
            QString localPath = QString::fromUtf8(transfer->getPath());
            #ifdef WIN32
                if(localPath.startsWith(QString::fromAscii("\\\\?\\"))) localPath = localPath.mid(4);
            #endif
            uploadLocalPaths[transfer->getTag()]=localPath;
        }
	}

    //Send updated statics to the information dialog
    if(infoDialog)
    {
        if(e->getErrorCode() == MegaError::API_OK)
        {
            if(((transfer->getType() == MegaTransfer::TYPE_DOWNLOAD) && (transfer->getStartTime()>=lastStartedDownload)) ||
                ((transfer->getType() == MegaTransfer::TYPE_UPLOAD) && (transfer->getStartTime()>=lastStartedUpload)))
                infoDialog->setTransfer(transfer);

            infoDialog->setTransferSpeeds(downloadSpeed, uploadSpeed);
            infoDialog->setTransferredSize(totalDownloadedSize, totalUploadedSize);
        }

        infoDialog->updateTransfers();
        infoDialog->transferFinished(e->getErrorCode());
    }

    if(transfer->getType()==MegaTransfer::TYPE_DOWNLOAD)
    {
        if(lastStartedDownload == transfer->getStartTime())
            lastStartedDownload = 0;
    }
    else
    {
        if(lastStartedUpload == transfer->getStartTime())
            lastStartedUpload = 0;

        if((e->getErrorCode() == MegaError::API_OK))
        {
            preferences->setUsedStorage(preferences->usedStorage()+transfer->getTotalBytes());
            preferences->setCloudDriveStorage(preferences->cloudDriveStorage() + transfer->getTotalBytes());
            preferences->setCloudDriveFiles(preferences->cloudDriveFiles()+1);
            if(settingsDialog) settingsDialog->refreshAccountDetails();

            if(infoDialog)
            {
                bool isShare = false;

                MegaHandle handle = transfer->getParentHandle();
                MegaNode *node = megaApi->getNodeByHandle(handle);

                const char *path = megaApi->getNodePath(node);
                if(path && path[0] != '/')
                {
                    isShare = true;
                }

                infoDialog->increaseUsedStorage(transfer->getTotalBytes(), isShare);

                delete node;
                delete [] path;
            }
        }
    }

    int errorCode = e->getErrorCode();
    if(errorCode != MegaError::API_OK && transfer->getTag() > 0
            && errorCode != MegaError::API_EACCESS
            && errorCode != MegaError::API_ESID
            && errorCode != MegaError::API_ESSL
            && errorCode != MegaError::API_EINCOMPLETE)
    {
        showErrorMessage(tr("Transfer failed:") + QString::fromUtf8(" " ) + QCoreApplication::translate("MegaError", e->getErrorString()), QString::fromUtf8(transfer->getFileName()));
    }

    //If there are no pending transfers, reset the statics and update the state of the tray icon
    if(!megaApi->getNumPendingDownloads() && !megaApi->getNumPendingUploads())
    {
        if(totalUploadSize || totalDownloadSize)
            onGlobalSyncStateChanged(megaApi);

		totalUploadSize = totalDownloadSize = 0;
		totalUploadedSize = totalDownloadedSize = 0;
        uploadSpeed = downloadSpeed = 0;
	}
}

//Called when a transfer has been updated
void MegaApplication::onTransferUpdate(MegaApi *, MegaTransfer *transfer)
{
    if(appfinished) return;

    //Update statics
	if(transfer->getType() == MegaTransfer::TYPE_DOWNLOAD)
	{
        downloadSpeed = transfer->getSpeed();
        if(!lastStartedDownload || !transfer->getTransferredBytes())
            lastStartedDownload = transfer->getStartTime();
		totalDownloadedSize += transfer->getDeltaSize();
	}
	else
	{
        uploadSpeed = transfer->getSpeed();
        if(!lastStartedUpload || !transfer->getTransferredBytes())
            lastStartedUpload = transfer->getStartTime();
		totalUploadedSize += transfer->getDeltaSize();
	}

    //Send updated statics to the information dialog
    if(infoDialog)
    {
        if(((transfer->getType() == MegaTransfer::TYPE_DOWNLOAD) && (transfer->getStartTime()>=lastStartedDownload)) ||
            ((transfer->getType() == MegaTransfer::TYPE_UPLOAD) && (transfer->getStartTime()>=lastStartedUpload)))
        {
            infoDialog->setTransfer(transfer);
            infoDialog->setTransferSpeeds(downloadSpeed, uploadSpeed);
            infoDialog->setTransferredSize(totalDownloadedSize, totalUploadedSize);
            infoDialog->updateTransfers();
        }

    }
}

//Called when there is a temporal problem in a transfer
void MegaApplication::onTransferTemporaryError(MegaApi *, MegaTransfer *transfer, MegaError* e)
{
    //Show information to users
    if(transfer->getNumRetry() == 1)
        showWarningMessage(tr("Temporary transmission error: ") + QCoreApplication::translate("MegaError", e->getErrorString()), QString::fromUtf8(transfer->getFileName()));
    else
        onGlobalSyncStateChanged(megaApi);
}

void MegaApplication::onAccountUpdate(MegaApi *api)
{
    megaApi->getAccountDetails();
}

//Called when contacts have been updated in MEGA
void MegaApplication::onUsersUpdate(MegaApi* , MegaUserList *)
{

}

//Called when nodes have been updated in MEGA
void MegaApplication::onNodesUpdate(MegaApi* , MegaNodeList *nodes)
{
    if(!infoDialog) return;

    bool externalNodes = 0;
    bool nodesRemoved = false;
    long long usedStorage = preferences->usedStorage();

    //If this is a full reload, return
	if(!nodes) return;

	MegaApi::log(MegaApi::LOG_LEVEL_INFO, QString::fromUtf8("%1 updated files/folders").arg(nodes->size()).toUtf8().constData());

    //Check all modified nodes
    QString localPath;
    for(int i=0; i<nodes->size(); i++)
	{
        localPath.clear();
        MegaNode *node = nodes->get(i);

        for(int i=0; i<preferences->getNumSyncedFolders(); i++)
        {
            if(!preferences->isFolderActive(i))
            {
                continue;
            }

            if(node->getType()==MegaNode::TYPE_FOLDER && (node->getHandle() == preferences->getMegaFolderHandle(i)))
            {
                MegaNode *nodeByHandle = megaApi->getNodeByHandle(preferences->getMegaFolderHandle(i));
                const char *nodePath = megaApi->getNodePath(nodeByHandle);

                if(!nodePath || preferences->getMegaFolder(i).compare(QString::fromUtf8(nodePath)))
                {
                    if(nodePath && QString::fromUtf8(nodePath).startsWith(QString::fromUtf8("//bin")))
                    {
                        showErrorMessage(tr("Your sync \"%1\" has been disabled because the remote folder is in the rubbish bin")
                                         .arg(preferences->getSyncName(i)));
                    }
                    else
                    {
                        showErrorMessage(tr("Your sync \"%1\" has been disabled because the remote folder doesn't exist")
                                         .arg(preferences->getSyncName(i)));
                    }
                    Platform::syncFolderRemoved(preferences->getLocalFolder(i), preferences->getSyncName(i));
                    Platform::notifyItemChange(preferences->getLocalFolder(i));
                    MegaNode *node = megaApi->getNodeByHandle(preferences->getMegaFolderHandle(i));
                    megaApi->removeSync(node);
                    delete node;
                    preferences->setSyncState(i, false);
                    openSettings(SettingsDialog::SYNCS_TAB);
                }

                delete nodeByHandle;
                delete [] nodePath;
            }
        }

        if(!node->getTag() && !node->isRemoved() && !node->isSyncDeleted() && ((lastExit/1000)<node->getCreationTime()))
            externalNodes++;

        if(node->isRemoved() && (node->getType() == MegaNode::TYPE_FILE))
        {
            usedStorage -= node->getSize();
            nodesRemoved = true;
        }

        if(!node->isRemoved() && node->getTag() && !node->isSyncDeleted() && (node->getType()==MegaNode::TYPE_FILE))
        {
            //Get the associated local node
            string path = node->getLocalPath();
            if(path.size())
            {
                //If the node has been uploaded by a synced folder
                //the SDK provides its local path
            #ifdef WIN32
                localPath = QString::fromWCharArray((const wchar_t *)path.data());
                if(localPath.startsWith(QString::fromAscii("\\\\?\\"))) localPath = localPath.mid(4);
            #else
                localPath = QString::fromUtf8(path.data());
            #endif
            }
            else if(uploadLocalPaths.contains(node->getTag()))
            {
                //If the node has been uploaded by a regular upload,
                //we recover the path using the tag of the transfer
                localPath = uploadLocalPaths.value(node->getTag());
                uploadLocalPaths.remove(node->getTag());
            }
            else
            {
                MegaApi::log(MegaApi::LOG_LEVEL_WARNING, QString::fromUtf8("Unable to get the local path of the file: %1 Tag: %2")
                             .arg(QString::fromUtf8(node->getName())).arg(node->getTag()).toUtf8().constData());
            }

            addRecentFile(QString::fromUtf8(node->getName()), node->getHandle(), localPath);
            if(node->getAttrString()->size())
            {
                //NO_KEY node created by this client detected
                if(!noKeyDetected)
                {
                    megaApi->fetchNodes();
                }
                else if(noKeyDetected > 20)
                {
                    QMessageBox::critical(NULL, QString::fromUtf8("MEGAsync"),
                        QString::fromUtf8("Something went wrong. MEGAsync will restart now. If the problem persists please contact bug@mega.co.nz"));
                    preferences->setCrashed(true);
                    rebootApplication(false);
                }
                noKeyDetected++;
            }
        }
	}

    if(nodesRemoved)
    {
        preferences->setUsedStorage(usedStorage);
        if(infoOverQuota && (preferences->usedStorage() < preferences->totalStorage()))
        {
            updateUserStats();
        }
    }

    if(externalNodes)
    {
        updateUserStats();

        if(QDateTime::currentMSecsSinceEpoch() - externalNodesTimestamp > Preferences::MIN_EXTERNAL_NODES_WARNING_MS)
        {
            externalNodesTimestamp = QDateTime::currentMSecsSinceEpoch();
            showNotificationMessage(tr("You have new or updated files in your account"));
        }
    }
}

void MegaApplication::onReloadNeeded(MegaApi*)
{
    //Don't reload the filesystem here because it's unsafe
    //and the most probable cause for this callback is a false positive.
    //Simply set the crashed flag to force a filesystem reload in the next execution.
    preferences->setCrashed(true);
    //megaApi->fetchNodes();
}

void MegaApplication::onGlobalSyncStateChanged(MegaApi *)
{
    if(megaApi && megaApiLinks)
    {
        indexing = megaApi->isScanning();
        waiting = megaApi->isWaiting() || megaApiLinks->isWaiting();
    }

    if(infoDialog)
    {
        infoDialog->setIndexing(indexing);
        infoDialog->setWaiting(waiting);
        infoDialog->setPaused(paused);
        infoDialog->updateState();
        infoDialog->transferFinished(MegaError::API_OK);
        infoDialog->updateRecentFiles();
    }

    MegaApi::log(MegaApi::LOG_LEVEL_INFO, QString::fromUtf8("Current state. Paused = %1   Indexing = %2   Waiting = %3")
                 .arg(paused).arg(indexing).arg(waiting).toUtf8().constData());

    int pendingUploads = megaApi->getNumPendingUploads();
    int pendingDownloads = megaApi->getNumPendingDownloads() + megaApiLinks->getNumPendingDownloads();
    if(pendingUploads)
    {
        MegaApi::log(MegaApi::LOG_LEVEL_INFO, QString::fromUtf8("Pending uploads: %1").arg(pendingUploads).toUtf8().constData());
    }

    if(pendingDownloads)
    {
        MegaApi::log(MegaApi::LOG_LEVEL_INFO, QString::fromUtf8("Pending downloads: %1").arg(pendingDownloads).toUtf8().constData());
    }

    if(!isLinux) updateTrayIcon();
}

void MegaApplication::onSyncStateChanged(MegaApi *api, MegaSync *sync)
{
    onGlobalSyncStateChanged(api);
}

void MegaApplication::onSyncFileStateChanged(MegaApi *, MegaSync *, const char *filePath, int)
{
    QString localPath = QString::fromUtf8(filePath);
    Platform::notifyItemChange(localPath);
}

MEGASyncDelegateListener::MEGASyncDelegateListener(MegaApi *megaApi, MegaListener *parent)
    : QTMegaListener(megaApi, parent)
{ }

void MEGASyncDelegateListener::onRequestFinish(MegaApi *api, MegaRequest *request, MegaError *e)
{
    QTMegaListener::onRequestFinish(api, request, e);

    if(request->getType() != MegaRequest::TYPE_FETCH_NODES || e->getErrorCode() != MegaError::API_OK)
        return;

    Preferences *preferences = Preferences::instance();
    if(preferences->logged() && !api->getNumActiveSyncs())
    {
        //Start syncs
        for(int i=0; i<preferences->getNumSyncedFolders(); i++)
        {
            QString tmpPath = preferences->getLocalFolder(i) + QDir::separator() + QString::fromUtf8(mega::MEGA_DEBRIS_FOLDER) + QString::fromUtf8("/tmp");
            QDirIterator di(tmpPath, QDir::Files | QDir::NoDotAndDotDot);
            while (di.hasNext())
            {
                di.next();
                const QFileInfo& fi = di.fileInfo();
                if(fi.fileName().endsWith(QString::fromAscii(".mega")))
                    QFile::remove(di.filePath());
            }

            if(!preferences->isFolderActive(i))
                continue;

            MegaNode *node = api->getNodeByHandle(preferences->getMegaFolderHandle(i));
            if(!node)
            {
                preferences->setSyncState(i, false);
                continue;
            }

            QString localFolder = preferences->getLocalFolder(i);
            api->resumeSync(localFolder.toUtf8().constData(), node, preferences->getLocalFingerprint(i));
            delete node;
        }
    }
}


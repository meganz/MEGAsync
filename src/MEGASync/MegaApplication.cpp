#include "MegaApplication.h"

#include "AccountDetailsManager.h"
#include "AccountStatusController.h"
#include "AppStatsEvents.h"
#include "Avatar.h"
#include "ChangeLogDialog.h"
#include "CommonMessages.h"
#include "CrashHandler.h"
#include "CrashReportDialog.h"
#include "CreateRemoveSyncsManager.h"
#include "DateTimeFormatter.h"
#include "DeviceCentre.h"
#include "DialogOpener.h"
#include "DuplicatedNodeDialog.h"
#include "EmailRequester.h"
#include "EphemeralCredentials.h"
#include "EventUpdater.h"
#include "ExportProcessor.h"
#include "FatalEventHandler.h"
#include "FullName.h"
#include "GuiUtilities.h"
#include "IconTokenizer.h"
#include "ImportMegaLinksDialog.h"
#include "IntervalExecutioner.h"
#include "LoginController.h"
#include "mega/types.h"
#include "MegaMenuItemAction.h"
#include "MegaProxyStyle.h"
#include "MessageDialogOpener.h"
#include "MyBackupsHandle.h"
#include "NodeSelector.h"
#include "NodeSelectorSpecializations.h"
#include "Onboarding.h"
#include "OverQuotaDialog.h"
#include "ParallelConnectionsValues.h"
#include "Platform.h"
#include "PlatformStrings.h"
#include "PowerOptions.h"
#include "ProxyStatsEventHandler.h"
#include "QmlDialogManager.h"
#include "QmlDialogWrapper.h"
#include "QTMegaApiManager.h"
#include "ReloadingEventHandler.h"
#include "RequestListenerManager.h"
#include "ServiceUrls.h"
#include "StalledIssuesDialog.h"
#include "StalledIssuesModel.h"
#include "StatsEventHandler.h"
#include "StreamingFromMegaDialog.h"
#include "SyncController.h"
#include "SyncReminderNotificationManager.h"
#include "SyncsMenu.h"
#include "ThemeManager.h"
#include "TransferMetaData.h"
#include "UploadToMegaDialog.h"
#include "UpsellComponent.h"
#include "UserAttributesManager.h"
#include "UserMessageController.h"
#include "Utilities.h"

#include <QCheckBox>
#include <QClipboard>
#include <QDesktopWidget>
#include <QFontDatabase>
#include <QFuture>
#include <QNetworkProxy>
#include <QScreen>
#include <QSettings>
#include <QtConcurrent/QtConcurrent>
#include <QToolTip>
#include <QTranslator>

#include <cassert>

#ifdef Q_OS_LINUX
#include <condition_variable>
#include <QSvgRenderer>
#include <signal.h>
#endif

#ifdef Q_OS_MACX
#include "platform/macx/PlatformImplementation.h"
#endif

#ifndef WIN32
// sleep
#include <unistd.h>
#else
#include <Windows.h>
#include <Psapi.h>
#include <Strsafe.h>
#include <Shellapi.h>
#endif

using namespace mega;
using namespace std;

QString MegaApplication::appPath = QString();
QString MegaApplication::appDirPath = QString();
QString MegaApplication::dataPath = QString();
QString MegaApplication::lastNotificationError = QString();

constexpr auto openUrlClusterMaxElapsedTime = std::chrono::seconds(5);

static const QString SCHEME_LOCAL_URL = QString::fromUtf8("local");

void MegaApplication::loadDataPath()
{
#ifdef Q_OS_LINUX
    dataPath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)
               + QString::fromUtf8("/data/") // appending "data" is non-standard behavior according to XDG Base Directory specification
               + organizationName() + QString::fromLatin1("/")
               + applicationName();
#else
    QStringList dataPaths = QStandardPaths::standardLocations(QStandardPaths::DataLocation);
    if (dataPaths.size())
    {
        dataPath = dataPaths.at(0);
    }
#endif

    if (dataPath.isEmpty())
    {
        dataPath = QDir::currentPath();
    }

    dataPath = QDir::toNativeSeparators(dataPath);
    QDir currentDir(dataPath);
    if (!currentDir.exists())
    {
        currentDir.mkpath(QString::fromUtf8("."));
    }
}

MegaApplication::MegaApplication(int& argc, char** argv):
    QApplication(argc, argv),
    mSyncs2waysMenu(nullptr),
    mBackupsMenu(nullptr),
    mIsFirstFileTwoWaySynced(false),
    mIsFirstFileBackedUp(false),
    mLoginController(nullptr),
    scanStageController(this),
    mDisableGfx(false),
    mUserMessageController(nullptr),
    mGfxProvider(nullptr),
    mSyncReminderNotificationManager(nullptr),
    misSyncingStateWrongLogged(false)
{
#if defined Q_OS_MACX && !defined QT_DEBUG
    if (!qEnvironmentVariableIsSet("MEGA_DISABLE_RUN_MAC_RESTRICTION"))
    {
        QString path = appBundlePath();
        if (path.compare(QStringLiteral("/Applications/MEGAsync.app")))
        {
            // Use regular QMessageBox with modal behaviour instead of MessageDialogOpener to avoid
            // issues.
            QMessageBox::warning(
                nullptr,
                QString(),
                QCoreApplication::translate("MegaSyncError",
                                            "You can't run MEGA Desktop App from this location. "
                                            "Move it into the Applications folder then run it."),
                QMessageBox::Ok);
            ::exit(0);
        }
    }
#endif

    appfinished = false;

    bool logToStdout = false;

#if defined(LOG_TO_STDOUT)
    logToStdout = true;
#endif

    // Collect program arguments
    QStringList args;
    for (int i=0; i < argc; ++i)
    {
        args += QString::fromUtf8(argv[i]);
    }

#ifdef Q_OS_LINUX

    if (args.contains(QLatin1String("--version")))
    {
        QTextStream(stdout) << getMEGAString() << " v" << Preferences::VERSION_STRING << " ("
                            << Preferences::SDK_ID << ")" << Qt::endl;
        ::exit(0);
    }

    logToStdout |= args.contains(QLatin1String("--debug"));

#endif

    connect(this, SIGNAL(blocked()), this, SLOT(onBlocked()));
    connect(this, SIGNAL(unblocked()), this, SLOT(onUnblocked()));

#ifdef _WIN32
    connect(this, SIGNAL(screenAdded(QScreen *)), this, SLOT(changeDisplay(QScreen *)));
    connect(this, SIGNAL(screenRemoved(QScreen *)), this, SLOT(changeDisplay(QScreen *)));
#endif

    setQuitOnLastWindowClosed(false);

    // For some reason this doesn't work on Windows (done in stylesheet above)
    // TODO: re-try with Qt > 5.12.15
    QPalette palette = QToolTip::palette();
    palette.setColor(QPalette::ToolTipBase, QColor("#333333"));
    palette.setColor(QPalette::ToolTipText, QColor("#FAFAFA"));
    QToolTip::setPalette(palette);

    appPath = QDir::toNativeSeparators(QCoreApplication::applicationFilePath());
    appDirPath = QDir::toNativeSeparators(QCoreApplication::applicationDirPath());

    //Set the working directory
    QDir::setCurrent(MegaApplication::applicationDataPath());

    QString desktopPath;

    QStringList desktopPaths = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation);
    if (desktopPaths.size())
    {
        desktopPath = desktopPaths.at(0);
    }
    else
    {
        desktopPath = Utilities::getDefaultBasePath();
    }

    logger.reset(new MegaSyncLogger(this, dataPath, desktopPath, logToStdout));
#if defined(LOG_TO_FILE)
    logger->setDebug(true);
#endif

    mThreadPool = ThreadPoolSingleton::getInstance();

    updateAvailable = false;
    networkConnectivity = true;
    trayIcon = nullptr;
    infoDialogMenu = nullptr;
    guestMenu = nullptr;
    megaApi = nullptr;
    megaApiFolders = nullptr;
    delegateListener = nullptr;
    httpServer = nullptr;
    exportOps = 0;
    infoDialog = nullptr;
    mSettingsDialog = nullptr;
    reboot = false;
    exitAction = nullptr;
    exitActionGuest = nullptr;
    settingsAction = nullptr;
    settingsActionGuest = nullptr;
    importLinksAction = nullptr;
    initialTrayMenu = nullptr;
    lastHovered = nullptr;
    isPublic = false;
    prevVersion = 0;
    mTransfersModel = nullptr;
    mStalledIssuesModel = nullptr;
    mStatusController = nullptr;
    mStatsEventHandler = nullptr;

#ifdef _WIN32
    windowsMenu = nullptr;
    windowsExitAction = nullptr;
    windowsUpdateAction = nullptr;
    windowsAboutAction = nullptr;
    windowsImportLinksAction = nullptr;
    windowsFilesAction = nullptr;
    windowsUploadAction = nullptr;
    windowsDownloadAction = nullptr;
    windowsStreamAction = nullptr;
    windowsTransferManagerAction = nullptr;
    windowsSettingsAction = nullptr;

    WCHAR commonPath[MAX_PATH + 1];
    if (SHGetSpecialFolderPathW(NULL, commonPath, CSIDL_COMMON_APPDATA, FALSE))
    {
        int len = lstrlen(commonPath);
        if (!memcmp(commonPath, (WCHAR *)appDirPath.utf16(), len * sizeof(WCHAR))
                && appDirPath.size() > len && appDirPath[len] == QLatin1Char('\\'))
        {
            isPublic = true;

            int intVersion = 0;
            QDir dataDir(dataPath);
            QString appVersionPath = dataDir.filePath(QString::fromUtf8("megasync.version"));
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
    guestSettingsAction = nullptr;
    initialExitAction = nullptr;
    uploadAction = nullptr;
    downloadAction = nullptr;
    streamAction = nullptr;
    filesAction = nullptr;
    MEGAWebAction = nullptr;
    deviceCentreAction = nullptr;
    mWaiting = false;
    updated = false;
    mSyncing = false;
    mSyncStalled = false;
    mTransferring = false;
    checkupdate = false;
    updateAction = nullptr;
    updateActionGuest = nullptr;
    showStatusAction = nullptr;
    updateBlocked = false;
    updateThread = nullptr;
    updateTask = nullptr;
    mTransferManager = nullptr;
    cleaningSchedulerExecution = 0;
    lastUserActivityExecution = 0;
    lastTsBusinessWarning = 0;
    lastTsErrorMessageShown = 0;
    mMaxMemoryUsage = 0;
    nodescurrent = false;
    getUserDataRequestReady = false;
    storageState = MegaApi::STORAGE_STATE_UNKNOWN;
    appliedStorageState = MegaApi::STORAGE_STATE_UNKNOWN;
    transferOverQuotaWaitTimeExpiredReceived = false;

#ifdef __APPLE__
    scanningTimer = nullptr;
#endif

    // Passing "--nogfx" or "/nogfx" disables graphics processing for the current run.
    // To always disable graphics processing, create a "megasync.nogfx" file in the user data dir.
    mDisableGfx |=
        args.contains(QLatin1String("--nogfx")) || args.contains(QLatin1String("/nogfx"));

    mFolderTransferListener = std::make_shared<FolderTransferListener>();

    connect(mFolderTransferListener.get(), &FolderTransferListener::folderTransferUpdated,
            this, &MegaApplication::onFolderTransferUpdate);

    connect(&transferProgressController, &BlockingStageProgressController::updateUi,
            &scanStageController, &ScanStageController::onFolderTransferUpdate);

    setAttribute(Qt::AA_DisableWindowContextHelpButton);

    // Don't execute the "onGlobalSyncStateChangedImpl" function too often or the dialog locks up,
    // eg. queueing a folder with 1k items for upload/download
    mIntervalExecutioner =
        std::make_unique<IntervalExecutioner>(Preferences::minSyncStateChangeProcessingIntervalMs);
}

MegaApplication::~MegaApplication()
{
    // Unregister own url schemes
    QDesktopServices::unsetUrlHandler(ServiceUrls::getSessionTransferBaseUrl().scheme());
    QDesktopServices::unsetUrlHandler(SCHEME_LOCAL_URL);

    logger.reset();

    if (!translator.isEmpty())
    {
        removeTranslator(&translator);
    }

    if (mMutexStealerThread)
    {
        mMutexStealerThread->join();
    }
}

void MegaApplication::showInterface(QString)
{
    if (appfinished)
    {
        return;
    }

    bool show = true;

    QDir dataDir(dataPath);
    if (dataDir.exists(QString::fromUtf8("megasync.show")))
    {
        QFile showFile(dataDir.filePath(QString::fromUtf8("megasync.show")));
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
        if (mSettingsDialog && mSettingsDialog->isVisible())
        {
            DialogOpener::showDialog(mSettingsDialog);
            return;
        }
    }
}

bool gCrashableForTesting = false;

void MegaApplication::initStyleAndResources()
{
    ThemeManager::instance()->init();

    setStyle(new MegaProxyStyle());
    QFile file(QLatin1String(":/style/WidgetsComponentsStyleSheetsSizes.css"));
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QString sourceStandardComponentsStyleSheet = QString::fromLatin1(file.readAll());
        file.close();
        setStyleSheet(Utilities::getPlatformProps(sourceStandardComponentsStyleSheet));
    }
    else
    {
        MegaApi::log(MegaApi::LOG_LEVEL_INFO,
                     "Couldn't open WidgetsComponentsStyleSheetsSizes.css");
    }
}

void MegaApplication::initialize()
{
    if (megaApi)
    {
        return;
    }

    paused = false;
    mIndexing = false;

    // Register own url schemes
    QDesktopServices::setUrlHandler(ServiceUrls::getSessionTransferBaseUrl().scheme(),
                                    this,
                                    "handleMEGAurl");
    QDesktopServices::setUrlHandler(SCHEME_LOCAL_URL, this, "handleLocalPath");

    qRegisterMetaTypeStreamOperators<EphemeralCredentials>("EphemeralCredentials");

    preferences = Preferences::instance();
    connect(preferences.get(), SIGNAL(stateChanged()), this, SLOT(changeState()));
    connect(preferences.get(), SIGNAL(updated(int)), this, SLOT(showUpdatedMessage(int)),
            Qt::DirectConnection); // Use direct connection to make sure 'updated' and 'prevVersions' are set as needed
    preferences->initialize(dataPath);

    if (preferences->error())
    {
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("Encountered corrupt prefrences.").toUtf8().constData());

        MessageDialogInfo msgInfo;
        msgInfo.descriptionText = tr("Your config is corrupt, please start over");
        msgInfo.enqueue = true;
        MessageDialogOpener::critical(msgInfo);
    }

    preferences->setLastStatsRequest(0);
    lastExit = preferences->getLastExit();

    // Apply specific rcc files depending on selected theme
    initStyleAndResources();

    installTranslator(&translator);
    QString language = preferences->language();
    changeLanguage(language);

    mOsNotifications = std::make_shared<DesktopNotifications>(applicationName(), trayIcon);

    Qt::KeyboardModifiers modifiers = queryKeyboardModifiers();
    if (modifiers.testFlag(Qt::ControlModifier)
            && modifiers.testFlag(Qt::ShiftModifier))
    {
        toggleLogging();
    }

    const QString basePath = QDir::toNativeSeparators(dataPath + QString::fromUtf8("/"));

    createGfxProvider(basePath);

    // Connect to AppState signals
    auto appState = AppState::instance();
    connect(this, &MegaApplication::requestAppState, appState.get(), &AppState::setAppState);
    connect(appState.get(), &AppState::appStateChanged, this, &MegaApplication::onAppStateChanged);

    // Connect to FatalEventHandler signals
    auto fatalEventHandler = FatalEventHandler::instance();
    connect(fatalEventHandler.get(),
            &FatalEventHandler::requestExitApp,
            this,
            &MegaApplication::tryExitApplication);
    connect(fatalEventHandler.get(),
            &FatalEventHandler::requestUnlink,
            this,
            &MegaApplication::unlink);
    connect(fatalEventHandler.get(),
            &FatalEventHandler::requestRebootApp,
            this,
            &MegaApplication::rebootApplication);

    // Check presence of media processing disabling file
    const QFile nogfxFile(QDir(dataPath).filePath(QLatin1String("megasync.nogfx")));
    mDisableGfx |= nogfxFile.exists();

    QTMegaApiManager::createMegaApi(megaApi,
                                    Preferences::CLIENT_KEY,
                                    mGfxProvider.get(),
                                    basePath.toUtf8().constData(),
                                    Preferences::USER_AGENT.toUtf8().constData(),
                                    !preferences->SSLcertificateException());
    megaApi->disableGfxFeatures(mDisableGfx);

    QTMegaApiManager::createMegaApi(megaApiFolders,
                                    Preferences::CLIENT_KEY,
                                    nullptr,
                                    basePath.toUtf8().constData(),
                                    Preferences::USER_AGENT.toUtf8().constData(),
                                    !preferences->SSLcertificateException());
    megaApiFolders->disableGfxFeatures(true);

    model = SyncInfo::instance();
    connect(model, &SyncInfo::syncStateChanged, this, &MegaApplication::onSyncModelUpdated);
    connect(model, &SyncInfo::syncRemoved, this, &MegaApplication::onSyncModelUpdated);
    connect(model, &SyncInfo::syncDisabledListUpdated, this, &MegaApplication::updateTrayIcon);

    MegaApi::log(MegaApi::LOG_LEVEL_INFO, QString::fromLatin1("Graphics processing %1")
                 .arg(mDisableGfx ? QLatin1String("disabled")
                                  : QLatin1String("enabled"))
                 .toUtf8().constData());

    // Init the Service Urls instance with the newly created API
    ServiceUrls::instance()->reset(megaApi);

    // Set maximum log line size to 10k (same as SDK default)
    // Otherwise network logging can cause large glitches when logging hundreds of MB
    // On Mac it is particularly apparent, causing the beachball to appear often
    size_t newPayLoadLogSize = 10240;
    megaApi->log(MegaApi::LOG_LEVEL_INFO, QString::fromUtf8("Establishing max payload log size: %1").arg(newPayLoadLogSize).toUtf8().constData());
    megaApi->setMaxPayloadLogSize(newPayLoadLogSize);
    megaApiFolders->setMaxPayloadLogSize(newPayLoadLogSize);

    mStatsEventHandler = std::make_unique<ProxyStatsEventHandler>(megaApi);
    QmlManager::instance()->setRootContextProperty(mStatsEventHandler.get());

    QString stagingPath = QDir(dataPath).filePath(QString::fromUtf8("megasync.staging"));
    QFile fstagingPath(stagingPath);
    if (fstagingPath.exists())
    {
        QSettings settings(stagingPath, QSettings::IniFormat);
        QString apiURL = settings
                             .value(QString::fromUtf8("apiurl"),
                                    ServiceUrls::getDefaultStagingApiUrl().toString())
                             .toString();
        const auto disablepkpValue =
            settings.value(QLatin1String("disablepkp"), QLatin1String("0")).toString();
        const bool disablepkp = disablepkpValue == QLatin1String("1");
        megaApi->changeApiUrl(apiURL.toUtf8(), disablepkp);
        megaApiFolders->changeApiUrl(apiURL.toUtf8(), disablepkp);

        MessageDialogInfo msgInfo;
        msgInfo.descriptionText = QString::fromUtf8("API URL changed to ") + apiURL;
        msgInfo.enqueue = true;
        MessageDialogOpener::warning(msgInfo);

        const auto baseUrl = settings.value(QString::fromUtf8("baseurl")).toString();

        if (!baseUrl.isEmpty())
        {
            auto megaUrls = ServiceUrls::instance();
            const auto defaultBaseUrl = megaUrls->getBaseUrl().toString();
            megaUrls->baseUrlOverride(baseUrl);

            if (baseUrl != defaultBaseUrl)
            {
                MessageDialogInfo msgInfo;
                msgInfo.descriptionText = QString::fromUtf8("base URL changed to ") + baseUrl;
                MessageDialogOpener::warning(msgInfo);
            }
        }
        gCrashableForTesting = settings.value(QString::fromUtf8("crashable"), false).toBool();

        Preferences::overridePreferences(settings);
        Preferences::SDK_ID.append(QString::fromUtf8(" - STAGING"));
    }
    trayIcon->show();

    megaApi->log(MegaApi::LOG_LEVEL_INFO,
                 QString::fromUtf8("MEGA Desktop App is starting. Version string: %1   Version "
                                   "code: %2.%3-%4   User-Agent: %5")
                     .arg(Preferences::VERSION_STRING)
                     .arg(Preferences::VERSION_CODE)
                     .arg(Preferences::VERSION_RC)
                     .arg(Preferences::BUILD_ID)
                     .arg(QString::fromUtf8(megaApi->getUserAgent()))
                     .toUtf8()
                     .constData());

    megaApi->setLanguage(currentLanguageCode.toUtf8().constData());
    megaApiFolders->setLanguage(currentLanguageCode.toUtf8().constData());

    // In case the user has logout and closed the app, we set the default values
    setMaxConnections(MegaTransfer::TYPE_UPLOAD, preferences->parallelUploadConnections());
    setMaxConnections(MegaTransfer::TYPE_DOWNLOAD, preferences->parallelDownloadConnections());
    setUseHttpsOnly(preferences->usingHttpsOnly());
    megaApi->setDefaultFilePermissions(preferences->filePermissionsValue());
    megaApi->setDefaultFolderPermissions(preferences->folderPermissionsValue());

    megaApi->retrySSLerrors(true);

    mStatusController = new AccountStatusController(this);
    QmlManager::instance()->setRootContextProperty(mStatusController);
    AccountDetailsManager::instance()->init(megaApi);

    delegateListener = new QTMegaListener(megaApi, this);
    megaApi->addListener(delegateListener);
    uploader = new MegaUploader(megaApi, mFolderTransferListener);
    downloader = new MegaDownloader(megaApi, mFolderTransferListener);
    connect(uploader, &MegaUploader::startingTransfers, this, &MegaApplication::startingUpload);
    connect(downloader, &MegaDownloader::startingTransfers,
            &scanStageController, &ScanStageController::startDelayedScanStage);

#ifdef _WIN32
    if (isPublic && prevVersion <= 3104 && preferences->canUpdate(appPath))
    {
        megaApi->log(MegaApi::LOG_LEVEL_INFO, QString::fromUtf8("Fixing permissions for other users in the computer").toUtf8().constData());
        QDirIterator it (appDirPath, QDir::AllEntries | QDir::Hidden | QDir::System | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
        while (it.hasNext())
        {
            Platform::getInstance()->makePubliclyReadable(QDir::toNativeSeparators(it.next()));
        }
    }
#endif

#ifdef __APPLE__
    MEGA_SET_PERMISSIONS;
#endif

    if (!preferences->isOneTimeActionDone(Preferences::ONE_TIME_ACTION_REGISTER_UPDATE_TASK))
    {
        bool success = Platform::getInstance()->registerUpdateJob();
        if (success)
        {
            preferences->setOneTimeActionDone(Preferences::ONE_TIME_ACTION_REGISTER_UPDATE_TASK, true);
        }
    }
    startCrashReportingDialog();
    mTransferQuota = std::make_shared<TransferQuota>(mOsNotifications);
    connect(mTransferQuota.get(), &TransferQuota::waitTimeIsOver, this, &MegaApplication::updateStatesAfterTransferOverQuotaTimeHasExpired);

    periodicTasksTimer = new QTimer(this);
    connect(periodicTasksTimer, SIGNAL(timeout()), this, SLOT(periodicTasks()));
    periodicTasksTimer->start(Preferences::STATE_REFRESH_INTERVAL_MS);

    networkCheckTimer = new QTimer(this);
    networkCheckTimer->start(Preferences::NETWORK_REFRESH_INTERVAL_MS);
    connect(networkCheckTimer, SIGNAL(timeout()), this, SLOT(checkNetworkInterfaces()));

    // SDK locker code for testing purposes
    if (Preferences::MUTEX_STEALER_MS && Preferences::MUTEX_STEALER_PERIOD_MS)
    {
        mMutexStealerThread.reset(new std::thread([this]() {
            while (!appfinished)
            {
                {
                    std::unique_ptr<MegaApiLock> apiLock {megaApi->getMegaApiLock(true)};
                    Utilities::sleepMilliseconds(Preferences::MUTEX_STEALER_MS);
                }
                if (Preferences::MUTEX_STEALER_PERIOD_ONLY_ONCE)
                {
                    return;
                }
                Utilities::sleepMilliseconds(Preferences::MUTEX_STEALER_PERIOD_MS);
            }
        }));
    }

    infoDialogTimer = new QTimer(this);
    infoDialogTimer->setSingleShot(true);
    connect(infoDialogTimer, SIGNAL(timeout()), this, SLOT(showInfoDialog()));

    connect(this, SIGNAL(aboutToQuit()), this, SLOT(cleanAll()));

    if (preferences->logged() && preferences->getGlobalPaused())
    {
        pauseTransfers(true);
    }

    QDir dataDir(dataPath);
    if (dataDir.exists())
    {
        QString appShowInterfacePath = dataDir.filePath(QString::fromUtf8("megasync.show"));
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

    mTransfersModel = new TransfersModel();
    connect(mTransfersModel.data(), &TransfersModel::transfersCountUpdated, this, &MegaApplication::onTransfersModelUpdate);

    mStalledIssuesModel = new StalledIssuesModel();
    connect(Platform::getInstance()->getShellNotifier().get(), &AbstractShellNotifier::shellNotificationProcessed,
            this, &MegaApplication::onNotificationProcessed);

    mLogoutController = new LogoutController(QmlManager::instance()->getEngine());
    connect(mLogoutController, &LogoutController::logout, this, &MegaApplication::onLogout);
    QmlManager::instance()->setRootContextProperty(mLogoutController);

    //! NOTE! Create a raw pointer, as the lifetime of this object needs to be carefully managed:
    //! mSetManager needs to be manually deleted, as the SDK needs to be destroyed first
    mSetManager = new SetManager(megaApi, megaApiFolders);

    mLinkProcessor = new LinkProcessor(megaApi, megaApiFolders);
    connect(mLinkProcessor,
            &LinkProcessor::linkCopyErrorDetected,
            this,
            &MegaApplication::onCopyLinkError);

    connect(mLinkProcessor, &LinkProcessor::requestFetchSetFromLink, mSetManager, &SetManager::requestFetchSetFromLink);
    connect(mSetManager, &SetManager::onFetchSetFromLink, mLinkProcessor, &LinkProcessor::onFetchSetFromLink);
    connect(mLinkProcessor,
            &LinkProcessor::requestDownloadSet,
            mSetManager,
            &SetManager::requestDownloadSet);
    connect(mLinkProcessor,
            &LinkProcessor::requestImportSet,
            mSetManager,
            &SetManager::requestImportSet);

    createUserMessageController();

    TokenParserWidgetManager::instance();

#ifdef Q_OS_LINUX
    connect(Platform::getInstance(),
            &AbstractPlatform::themeChanged,
            this,
            &MegaApplication::onOperatingSystemThemeChanged);
#endif
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

    QmlManager::instance()->retranslate();

    createTrayIcon();
}

#ifdef Q_OS_LINUX
void MegaApplication::setTrayIconFromTheme(QString icon)
{
    QString name = QString(icon).replace(QString::fromUtf8("://images/"), QString::fromUtf8("mega")).replace(QString::fromUtf8(".svg"),QString::fromUtf8(""));
    const bool needsToBeUpdated{name != trayIcon->icon().name()};
    if(needsToBeUpdated)
    {
        trayIcon->setIcon(QIcon::fromTheme(name, QIcon(icon)));
    }
}
#endif

void MegaApplication::updateTrayIcon()
{
    if (appfinished || !trayIcon)
    {
        return;
    }

    QString tooltipState;
    QString icon;

    static std::map<std::string, QString> icons = {
    #ifndef __APPLE__
        #ifdef _WIN32
            { "warning", QString::fromUtf8("://images/warning_ico.ico") },
            { "synching", QString::fromUtf8("://images/tray_sync.ico") },
            { "uptodate", QString::fromUtf8("://images/app_ico.ico") },
            { "paused", QString::fromUtf8("://images/tray_pause.ico") },
            { "logging", QString::fromUtf8("://images/login_ico.ico") },
            { "alert", QString::fromUtf8("://images/alert_ico.ico") },
            { "someissues", QString::fromUtf8("://images/warning_ico.ico") }

        #else
            { "warning", QString::fromUtf8("://images/warning.svg") },
            { "synching", QString::fromUtf8("://images/synching.svg") },
            { "uptodate", QString::fromUtf8("://images/uptodate.svg") },
            { "paused", QString::fromUtf8("://images/paused.svg") },
            { "logging", QString::fromUtf8("://images/logging.svg") },
            { "alert", QString::fromUtf8("://images/alert.svg") },
            { "someissues", QString::fromUtf8("://images/warning.svg") }
        #endif
    #else
            { "warning", QString::fromUtf8("://images/icon_overquota_mac.png") },
            { "synching", QString::fromUtf8("://images/icon_syncing_mac.png") },
            { "uptodate", QString::fromUtf8("://images/icon_synced_mac.png") },
            { "paused", QString::fromUtf8("://images/icon_paused_mac.png") },
            { "logging", QString::fromUtf8("://images/icon_logging_mac.png") },
            { "alert", QString::fromUtf8("://images/icon_alert_mac.png") },
            { "someissues", QString::fromUtf8("://images/icon_overquota_mac.png") }
    #endif
        };

    const bool isStorageOverQuotaOrPaywall = appliedStorageState == MegaApi::STORAGE_STATE_RED
                                                || appliedStorageState == MegaApi::STORAGE_STATE_PAYWALL;

    if (AppState::instance()->getAppState() == AppState::FATAL_ERROR)
    {
        icon = icons["alert"];
        tooltipState = FatalEventHandler::instance()->getErrorTitle();
    }
    else if (isStorageOverQuotaOrPaywall || mTransferQuota->isOverQuota())
    {
        tooltipState = isStorageOverQuotaOrPaywall ? tr("Storage full") : tr("Transfer quota exceeded");
        icon = icons["warning"];

#ifdef __APPLE__
        if (scanningTimer->isActive())
        {
            scanningTimer->stop();
        }
#endif
    }
    else if (mStatusController->isAccountBlocked())
    {
        tooltipState = tr("Locked account");

        icon = icons["alert"];

#ifdef __APPLE__
        if (scanningTimer->isActive())
        {
            scanningTimer->stop();
        }
#endif
    }
    else if (model->hasUnattendedDisabledSyncs({MegaSync::TYPE_TWOWAY, MegaSync::TYPE_BACKUP}))
    {
        if (model->hasUnattendedDisabledSyncs(MegaSync::TYPE_TWOWAY)
            && model->hasUnattendedDisabledSyncs(MegaSync::TYPE_BACKUP))
        {
            tooltipState = tr("Some syncs and backups have been disabled");
        }
        else if (model->hasUnattendedDisabledSyncs(MegaSync::TYPE_BACKUP))
        {
            tooltipState = tr("One or more backups have been disabled");
        }
        else
        {
            tooltipState = tr("One or more syncs have been disabled");
        }

        icon = icons["alert"];

#ifdef __APPLE__
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
            tooltipState = tr("Logging in");
            icon = icons["synching"];
    #ifdef __APPLE__
            if (!scanningTimer->isActive())
            {
                scanningAnimationIndex = 1;
                scanningTimer->start();
            }
    #endif
        }
        else
        {
            tooltipState = tr("You are not logged in");
            icon = icons["uptodate"];

    #ifdef __APPLE__
            if (scanningTimer->isActive())
            {
                scanningTimer->stop();
            }
    #endif
        }
    }
    else if (!nodescurrent || !getRootNode() ||
             AppState::instance()->getAppState() == AppState::RELOADING)
    {
        tooltipState = tr("Fetching file list...");
        icon = icons["synching"];

#ifdef __APPLE__
        if (!scanningTimer->isActive())
        {
            scanningAnimationIndex = 1;
            scanningTimer->start();
        }
#endif
    }
    else if (mSyncStalled && !mStalledIssuesModel->isEmpty())
    {
        tooltipState = tr("Stalled");
        icon = icons["alert"];

#ifdef __APPLE__
        if (scanningTimer->isActive())
        {
            scanningTimer->stop();
        }
#endif
    }
    else if (paused)
    {
        auto transfersFailed(mTransfersModel ? mTransfersModel->failedTransfers() : 0);

        if(transfersFailed > 0)
        {
            //We won´t never have thousand of millions of failed issues...so overflow is not a problem here
            tooltipState = QCoreApplication::translate("TransferManager","Issue found", "", static_cast<int>(transfersFailed));
            icon = icons["someissues"];
        }
        else
        {
            tooltipState = tr("Paused");
            icon = icons["paused"];
        }

#ifdef __APPLE__
        if (scanningTimer->isActive())
        {
            scanningTimer->stop();
        }
#endif
    }
    else if (mIndexing || mWaiting || mSyncing || mTransferring)
    {
        if (mIndexing)
        {
            tooltipState = tr("Scanning");
        }
        else if (mSyncing)
        {
            tooltipState = tr("Syncing");
        }
        else if (mWaiting)
        {
            tooltipState = tr("Waiting");
        }
        else
        {
            tooltipState = tr("Transferring");
        }

        icon = icons["synching"];

#ifdef __APPLE__
        if (!scanningTimer->isActive())
        {
            scanningAnimationIndex = 1;
            scanningTimer->start();
        }
#endif
    }
    else
    {
        auto transfersFailed(mTransfersModel ? mTransfersModel->failedTransfers() : 0);

        if(transfersFailed > 0)
        {
            tooltipState = QCoreApplication::translate("TransferManager","Issue found", "", static_cast<int>(transfersFailed));
            icon = icons["someissues"];
        }
        else
        {
            tooltipState = tr("Up to date");
            icon = icons["uptodate"];
        }

#ifdef __APPLE__
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
        tooltipState = tr("No Internet connection");
        icon = icons["logging"];
    }

    QString tooltip = QString::fromUtf8("%1 %2\n").arg(QString::fromUtf8("MEGA")).arg(Preferences::VERSION_STRING);

    if (updateAvailable)
    {
        // Only overwrite the tooltipState if it is "Up to date", because
        // in that case it would be conflicting with "Update available!"
        if (tooltipState != tr("Up to date"))
        {
            tooltip += tooltipState + QString::fromUtf8("\n");
        }

        tooltip += tr("Update available!");
    }
    else
    {
        tooltip += tooltipState;
    }

    // Finally apply icon and tooltip
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

    mIndexing = false;
    paused = false;
    nodescurrent = false;
    getUserDataRequestReady = false;
    storageState = MegaApi::STORAGE_STATE_UNKNOWN;
    appliedStorageState = MegaApi::STORAGE_STATE_UNKNOWN;
    receivedStorageSum = 0;

    AccountDetailsManager::instance()->reset();

    if (infoDialog)
    {
        infoDialog->reset();
    }
    mTransferQuota->reset();
    transferOverQuotaWaitTimeExpiredReceived = false;
    updateTrayIconMenu();

#ifndef __APPLE__
    #ifdef _WIN32
        trayIcon->setIcon(QIcon(QString::fromUtf8("://images/tray_sync.ico")));
    #else
        setTrayIconFromTheme(QString::fromUtf8("://images/synching.svg"));
    #endif
#else
    QIcon ic = QIcon(QString::fromUtf8("://images/icon_syncing_mac.png"));
    ic.setIsMask(true);
    trayIcon->setIcon(ic);

    if (!scanningTimer->isActive())
    {
        scanningAnimationIndex = 1;
        scanningTimer->start();
    }
#endif
    trayIcon->setToolTip(QCoreApplication::applicationName() + QString::fromUtf8(" ") + Preferences::VERSION_STRING + QString::fromUtf8("\n") + tr("Logging in"));
    trayIcon->show();

    //In case the previous session did not remove all of them
    Preferences::instance()->clearTempTransfersPath();

    if (!preferences->lastExecutionTime())
    {
        Platform::getInstance()->enableTrayIcon(QFileInfo(MegaApplication::applicationFilePath()).fileName());
    }

    if (updated)
    {
        showInfoMessage(tr("MEGAsync has been updated"));
        preferences->setFirstFileSynced();
        preferences->setFirstBackupDone();
        preferences->setFirstFileBackedUp();
        preferences->setFirstWebDownloadDone();

        if (!preferences->installationTime())
        {
            preferences->setInstallationTime(-1);
        }
        Platform::getInstance()->runPostAutoUpdateStep();
    }

    applyProxySettings();
    Platform::getInstance()->startShellDispatcher(this);
#ifdef Q_OS_MACX
    if (!preferences->isOneTimeActionDone(Preferences::ONE_TIME_ACTION_ACTIVE_FINDER_EXT))
    {
        MegaApi::log(MegaApi::LOG_LEVEL_INFO,
                     "MEGA Finder Sync added to system database and enabled");
        Platform::getInstance()->addFileManagerExtensionToSystem();
        QTimer::singleShot(5000, this, SLOT(enableFinderExt()));
    }
#endif

    //Start the initial setup wizard if needed
    if (preferences->getSession().isEmpty())
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
            mStatsEventHandler->sendEvent(AppStatsEvents::EventType::UPDATE);
            checkupdate = true;
        }
        updated = false;

        checkOperatingSystem();

        if (!infoDialog)
        {
            createInfoDialog();
            checkSystemTray();
            createTrayIcon();
        }

        mLoginController = new LoginController(QmlManager::instance()->getEngine());
        if (!preferences->isFirstStartDone())
        {
            mStatsEventHandler->sendEvent(AppStatsEvents::EventType::FIRST_START);
        }
        else if (!QSystemTrayIcon::isSystemTrayAvailable() &&
                 !qEnvironmentVariableIsSet("START_MEGASYNC_IN_BACKGROUND"))
        {
            showInfoDialog();
        }

        onGlobalSyncStateChanged(megaApi);
    }
    else //Otherwise, login in the account
    {
        mLoginController = new FastLoginController(QmlManager::instance()->getEngine());
        if (mLoginController == nullptr || !static_cast<FastLoginController*>(mLoginController)->fastLogin()) //In case preferences are corrupt with empty session, just unlink and remove associated data.
        {
            MegaApi::log(MegaApi::LOG_LEVEL_ERROR, "MEGAsync preferences logged but empty session. Unlink account and fresh start.");
            unlink();
        }
        if (updated)
        {
            mStatsEventHandler->sendEvent(AppStatsEvents::EventType::UPDATE);
            checkupdate = true;
        }
    }

    // The same name is used for fast login
    QmlManager::instance()->setRootContextProperty(QString::fromUtf8("loginControllerAccess"),
                                                   mLoginController);
    if (preferences->getSession().isEmpty())
    {
        QmlDialogManager::instance()->openOnboardingDialog();
    }

    updateTrayIcon();
}

void MegaApplication::requestUserData()
{
    if (!megaApi)
    {
        return;
    }
    UserAttributes::MyBackupsHandle::requestMyBackupsHandle();
    UserAttributes::FullName::requestFullName();
    UserAttributes::Avatar::requestAvatar();

    megaApi->getFileVersionsOption();
    megaApi->getPSA();
}

void MegaApplication::onboardingFinished(bool fastLogin, bool comesFromOnboarding)
{
    if (appfinished)
    {
        return;
    }

    registerUserActivity();
    pauseTransfers(paused);

    // This must be done before mSettingsDialog->setProxyOnly(false);
    if (megaApi->isBusinessAccount() || megaApi->isProFlexiAccount())
    {
        // Make sure the business/pro flexi nature of the account are set in preferences
        preferences->setAccountType(megaApi->isProFlexiAccount() ? Preferences::ACCOUNT_TYPE_PRO_FLEXI
                                                                 : Preferences::ACCOUNT_TYPE_BUSINESS);
        manageBusinessStatus(megaApi->getBusinessStatus());
    }

    int cachedStorageState = preferences->getStorageState();

    // Ask for storage on first login or when cached value is invalid
    bool checkStorage = !fastLogin || cachedStorageState == MegaApi::STORAGE_STATE_UNKNOWN;
    AccountDetailsManager::instance()->updateUserStats(
        checkStorage ? AccountDetailsManager::Flag::ALL : AccountDetailsManager::Flag::TRANSFER_PRO,
        true,
        !fastLogin ? USERSTATS_LOGGEDIN : USERSTATS_STORAGECACHEUNKNOWN);

    // Apply the "Start on startup" configuration, make sure configuration has the actual value
    // get the requested value
    bool startOnStartup = preferences->startOnStartup();
    // try to enable / disable startup (e.g. copy or delete desktop file)
    if (!Platform::getInstance()->startOnStartup(startOnStartup))
    {
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

preferences->setLastExecutionTime(QDateTime::currentMSecsSinceEpoch());

startUpdateTask();
QString language = preferences->language();
changeLanguage(language);
updated = false;

checkOperatingSystem();

if (!infoDialog)
{
    createInfoDialog();
}
    infoDialog->setUsage();
    infoDialog->setAvatar();
    infoDialog->setAccountType(preferences->accountType());
    infoDialog->createUpsellController();

    if (!QSystemTrayIcon::isSystemTrayAvailable())
    {
        checkSystemTray();
        if (!qEnvironmentVariableIsSet("START_MEGASYNC_IN_BACKGROUND"))
        {
            showInfoDialog();
        }
    }

    model->setUnattendedDisabledSyncs(preferences->getDisabledSyncTags());

    if (preferences->getNotifyDisabledSyncsOnLogin())
    {
        auto settingsTabToOpen = SettingsDialog::SYNCS_TAB;
        QString message;
        QVector<MegaSync::SyncType> syncsTypesToDismiss;

        bool haveSyncs (false);
        bool haveBackups (false);

        unique_ptr<MegaSyncList> syncList(megaApi->getSyncs());
        for (int i = 0; i < syncList->size(); ++i)
        {
            if (syncList->get(i)->getType() == MegaSync::TYPE_BACKUP)
            {
                haveBackups = true;
            }
            else
            {
                haveSyncs = true;
            }
        }

        // Set text according to situation
        if (haveSyncs && haveBackups)
        {
            syncsTypesToDismiss = {MegaSync::TYPE_TWOWAY, MegaSync::TYPE_BACKUP};
            message = tr(
                "Some syncs and backups have been disabled. Go to settings to enable them again.");
        }
        else if (haveBackups)
        {
            syncsTypesToDismiss = {MegaSync::TYPE_BACKUP};
            message =
                tr("One or more backups have been disabled. Go to settings to enable them again.");
        }
        else if (haveSyncs)
        {
            syncsTypesToDismiss = {MegaSync::TYPE_TWOWAY};
            message =
                tr("One or more syncs have been disabled. Go to settings to enable them again.");
        }

        // Display the message if it has been set
        if (!message.isEmpty())
        {
            MessageDialogInfo msgInfo;
            msgInfo.descriptionText = message;
            msgInfo.buttons = QMessageBox::Yes | QMessageBox::No;
            QMap<QMessageBox::Button, QString> textsByButton;
            textsByButton.insert(QMessageBox::Yes, tr("Open settings"));
            textsByButton.insert(QMessageBox::No, tr("Dismiss"));
            msgInfo.buttonsText = textsByButton;
            msgInfo.defaultButton = QMessageBox::No;
            msgInfo.finishFunc = [this, settingsTabToOpen](QPointer<MessageDialogResult> msg)
            {
                if (msg->result() == QMessageBox::Yes)
                {
                    openSettings(settingsTabToOpen);
                }
            };
            MessageDialogOpener::warning(msgInfo);
        }

        preferences->setNotifyDisabledSyncsOnLogin(false);
        model->dismissUnattendedDisabledSyncs(syncsTypesToDismiss);
    }

    // Init first synced and first backed-up file states from preferences
    mIsFirstFileTwoWaySynced = preferences->isFirstFileSynced();
    mIsFirstFileBackedUp = preferences->isFirstFileBackedUp();

    createAppMenus();

    mThreadPool->push([=](){
    setMaxUploadSpeed(preferences->uploadLimitKB());
    setMaxDownloadSpeed(preferences->downloadLimitKB());
    setMaxConnections(MegaTransfer::TYPE_UPLOAD,   preferences->parallelUploadConnections());
    setMaxConnections(MegaTransfer::TYPE_DOWNLOAD, preferences->parallelDownloadConnections());
    setUseHttpsOnly(preferences->usingHttpsOnly());

    megaApi->setDefaultFilePermissions(preferences->filePermissionsValue());
    megaApi->setDefaultFolderPermissions(preferences->folderPermissionsValue());
    });

    // Connect ScanStage signal
    connect(&scanStageController, &ScanStageController::enableTransferActions,
            this, &MegaApplication::enableTransferActions);

    // Process any pending download/upload queued during GuestMode
    processDownloads();
    processUploads();
    for (QMap<QString, QString>::iterator it = pendingLinks.begin(); it != pendingLinks.end(); it++)
    {
        QString link = it.key();
        megaApi->getPublicNode(link.toUtf8().constData());
    }

    updateUsedStorage();
    refreshStorageUIs();

    onGlobalSyncStateChanged(megaApi);

    if (cachedStorageState != MegaApi::STORAGE_STATE_UNKNOWN)
    {
        applyStorageState(cachedStorageState, true);
    }
    mStatusController->loggedIn();
    preferences->monitorUserAttributes();

    checkOverStorageStates(true);

    if (!preferences->isFirstSyncDone())
    {
        mSyncReminderNotificationManager = new SyncReminderNotificationManager(comesFromOnboarding);
        connect(&SyncController::instance(),
                &SyncController::syncAddStatus,
                mSyncReminderNotificationManager,
                &SyncReminderNotificationManager::onSyncAddRequestStatus);
        connect(this,
                &MegaApplication::syncsDialogClosed,
                mSyncReminderNotificationManager,
                &SyncReminderNotificationManager::onSyncsDialogClosed);
    }
}

void MegaApplication::onLoginFinished()
{
    if (mIntervalExecutioner)
    {
        connect(mIntervalExecutioner.get(), &IntervalExecutioner::execute,
                this, &MegaApplication::onScheduledExecution);
    }

    // Init desktop integration
    ThemeManager::instance()->init();
    Platform::getInstance()->disableContextMenu(Preferences::instance()->contextMenuDisabled());
    Platform::getInstance()->unHideTrayIcon();
}

void MegaApplication::onFetchNodesFinished()
{
    onGlobalSyncStateChanged(megaApi);

    if(mSettingsDialog)
    {
        mSettingsDialog->setProxyOnly(false);
    }
    requestUserData();
}

void MegaApplication::onLogout()
{
    if (mIntervalExecutioner)
    {
        disconnect(mIntervalExecutioner.get(), &IntervalExecutioner::execute,
                   this, &MegaApplication::onScheduledExecution);
    }

    if (infoDialog && infoDialog->isVisible())
    {
        infoDialog->hide();
    }
    model->reset();
    mTransfersModel->resetModel();
    mStalledIssuesModel->fullReset();
    mStatusController->reset();
    EmailRequester::instance()->reset();

    // Queue processing of logout cleanup to avoid race conditions
    // due to threadifing processing.
    // Eg: transfers added to data model after a logout
    mThreadPool->push([this]()
    {
        Utilities::queueFunctionInAppThread([this]()
        {
            if (preferences)
            {
                if (preferences->logged())
                {
                    clearUserAttributes();
                    preferences->unlink();
                    preferences->setFirstStartDone();
                }
                else
                {
                    preferences->resetGlobalSettings();
                }
                mLoginController->deleteLater();
                mLoginController = nullptr;
                DialogOpener::closeAllDialogs();
                mGfxProvider.reset();
                mUserMessageController.reset();
                mSyncReminderNotificationManager->deleteLater();
                createUserMessageController();
                infoDialog->deleteLater();
                infoDialog = nullptr;
                removeSyncsAndBackupsMenus();
                start();
                periodicTasks();
                ThemeManager::instance()->init();
            }
        });
    });
}

StatsEventHandler* MegaApplication::getStatsEventHandler() const
{
    return mStatsEventHandler.get();
}

void MegaApplication::checkSystemTray()
{
    if (QSystemTrayIcon::isSystemTrayAvailable() || Platform::getInstance()->validateSystemTrayIntegration())
    {
        return;
    }

    if (preferences->isOneTimeActionDone(Preferences::ONE_TIME_ACTION_NO_SYSTRAY_AVAILABLE))
    {
        return;
    }

    MessageDialogInfo msgInfo;
    msgInfo.descriptionText = tr(
        "Could not find a system tray to place MEGAsync tray icon. "
        "MEGAsync is intended to be used with a system tray icon but it can work fine without it. "
        "If you want to open the interface, just try to open MEGAsync again.");
    msgInfo.finishFunc = [this](QPointer<MessageDialogResult>)
    {
        preferences->setOneTimeActionDone(Preferences::ONE_TIME_ACTION_NO_SYSTRAY_AVAILABLE, true);
    };
    MessageDialogOpener::warning(msgInfo);
}

void MegaApplication::applyStorageState(int state, bool doNotAskForUserStats)
{
    if (state == MegaApi::STORAGE_STATE_CHANGE)
    {
        // this one is requested with force=false so it can't possibly occur to often.
        // It will in turn result in another call of this function with the actual new state (if it changed),
        // which is taken care of below with force=true (so that one does not have to wait further).
        // Also request pro state (low cost) in case the storage status is due to expiration of paid period etc.
        AccountDetailsManager::instance()->updateUserStats(AccountDetailsManager::Flag::STORAGE_PRO,
                                                           true,
                                                           USERSTATS_STORAGESTATECHANGE);
        return;
    }

    storageState = state;
    preferences->setStorageState(storageState);
    if (preferences->logged())
    {
        if (storageState != appliedStorageState)
        {
            {
                AccountDetailsManager::instance()->updateUserStats(
                    AccountDetailsManager::Flag::STORAGE_PRO,
                    true,
                    USERSTATS_TRAFFICLIGHT);
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

                if(mSettingsDialog)
                {
                    mSettingsDialog->close();
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
void MegaApplication::processUploadQueue(MegaHandle nodeHandle, QWidget* caller)
{
    if (appfinished)
    {
        return;
    }

    std::shared_ptr<MegaNode> node(megaApi->getNodeByHandle(nodeHandle));

    //If the destination node doesn't exist in the current filesystem, clear the queue and show an error message
    if (!node || node->isFile())
    {
        uploadQueue.clear();
        showErrorMessage(tr("Error: Invalid destination folder. The upload has been cancelled"));
        return;
    }

    noUploadedStarted = true;

    auto conflicts = CheckDuplicatedNodes::checkUploads(uploadQueue, node);

    if(!conflicts->isEmpty())
    {
        auto checkUploadNameDialog(new DuplicatedNodeDialog(caller));
        checkUploadNameDialog->setConflicts(conflicts);
        DialogOpener::showDialog<DuplicatedNodeDialog>(
            checkUploadNameDialog,
            [this, checkUploadNameDialog]() {
                if (checkUploadNameDialog && checkUploadNameDialog->result() == QDialog::Accepted)
                {
                    onUploadsCheckedAndReady(checkUploadNameDialog->conflicts());
                }
            });
    }
    else
    {
        onUploadsCheckedAndReady(conflicts);
    }
}

void MegaApplication::onUploadsCheckedAndReady(std::shared_ptr<ConflictTypes> conflicts)
{
        auto uploads = conflicts->mResolvedConflicts;

        auto data = TransferMetaDataContainer::createTransferMetaData<UploadTransferMetaData>(conflicts->mTargetNode->getHandle());
        preferences->setOverStorageDismissExecution(0);

        auto batch = std::shared_ptr<TransferBatch>(new TransferBatch(data->getAppId()));
        mBlockingBatch.add(batch);

        EventUpdater updater(uploads.size(),20);

        auto counter = 0;
        data->setInitialTransfers(uploads.size());
        foreach(auto uploadInfo, uploads)
        {
            QString filePath = uploadInfo->getSourceItemPath();
            uploader->upload(filePath, uploadInfo->getNewName(), conflicts->mTargetNode, data->getAppId(), batch);

            //Do not update the last items, leave Qt to do it in its natural way
            //If you update them, the flag mProcessingUploadQueue will be false and the scanning widget
            //will be stuck forever
            if(uploadInfo != uploads.last())
            {
                updater.update(counter);
                counter++;
            }
        }

        if (!batch->isEmpty())
        {
            QString logMessage = QString::fromUtf8("Added batch upload");
            MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, logMessage.toUtf8().constData());
        }
        else
        {
            mBlockingBatch.removeBatch();
            data->remove();
        }
}

void MegaApplication::processDownloadQueue(QString path)
{
    if (appfinished || downloadQueue.isEmpty())
    {
        return;
    }

    QDir dir(path);
    if (!dir.exists() && !dir.mkpath(QString::fromUtf8(".")))
    {
        QQueue<WrappedNode>::iterator it;
        for (it = downloadQueue.begin(); it != downloadQueue.end(); ++it)
        {
            HTTPServer::onTransferDataUpdate((*it).getMegaNode()->getHandle(),
                                             MegaTransfer::STATE_CANCELLED,
                                             0,
                                             0,
                                             0,
                                             QString());
        }

        downloadQueue.clear();
        showErrorMessage(tr("Error: Invalid destination folder. The download has been cancelled"));
        return;
    }

    MegaDownloader::DownloadInfo info;
    info.downloadQueue = downloadQueue;
    downloadQueue.clear();

    info.downloadBatches = &mBlockingBatch;
    info.path = path;

    downloader->processDownloadQueue(info);
}

void MegaApplication::createTransferManagerDialog()
{
    mTransferManager = new TransferManager(megaApi);
    infoDialog->setTransferManager(mTransferManager);

    // Signal/slot to notify the tracking of unseen completed transfers of Transfer Manager. If Completed tab is
    // active, tracking is disabled
    connect(mTransferManager.data() , &TransferManager::userActivity, this, &MegaApplication::registerUserActivity);
    connect(mTransferQuota.get(), &TransferQuota::sendState,
            mTransferManager.data(), &TransferManager::onTransferQuotaStateChanged);
    connect(mTransferManager.data(), SIGNAL(cancelScanning()), this, SLOT(cancelScanningStage()));
    scanStageController.updateReference(mTransferManager);
}

void MegaApplication::rebootApplication(bool update)
{
    if (appfinished)
    {
        return;
    }

    reboot = true;
    auto transferCount = getTransfersModel()->getTransfersCount();
    if (update && (transferCount.pendingDownloads || transferCount.pendingUploads
                   || megaApi->isWaiting() || megaApi->isScanning()
                   || scanStageController.isInScanningState()))
    {
        if (!updateBlocked)
        {
            updateBlocked = true;
            showInfoMessage(tr("An update will be applied during the next application restart"));
        }
        return;
    }

    trayIcon->hide();
    QApplication::exit();
}

int* testCrashPtr = nullptr;

void MegaApplication::tryExitApplication(bool force)
{
    if (appfinished)
    {
        return;
    }

    mStatsEventHandler->sendTrackedEvent(AppStatsEvents::EventType::MENU_EXIT_CLICKED,
                                         sender(), exitAction, true);

    if (dontAskForExitConfirmation(force))
    {
        exitApplication();
    }
    else
    {
        MessageDialogInfo msgInfo;
        msgInfo.descriptionText = tr("There is an active transfer. Exit the app?\n"
                                     "Transfer will automatically resume when you re-open the app.",
                                     "",
                                     mTransfersModel->hasActiveTransfers());
        msgInfo.buttons = QMessageBox::Yes|QMessageBox::No;
        QMap<QMessageBox::Button, QString> textsByButton;
        textsByButton.insert(QMessageBox::Yes, tr("Exit app"));
        textsByButton.insert(QMessageBox::No, tr("Stay in app"));
        msgInfo.buttonsText = textsByButton;
        msgInfo.finishFunc = [this](QPointer<MessageDialogResult> msg)
        {
            if (msg->result() == QMessageBox::Yes)
            {
                exitApplication();
            }
            else if (gCrashableForTesting)
            {
                *testCrashPtr = 0;
            }
        };
        MessageDialogOpener::question(msgInfo);
    }
}

void MegaApplication::pauseTransfers(bool pause)
{
    if (appfinished || !megaApi)
    {
        return;
    }

    megaApi->pauseTransfers(pause);
    if(getTransfersModel())
    {
        getTransfersModel()->pauseResumeAllTransfers(pause);
    }
}

void MegaApplication::checkNetworkInterfaces()
{
    if (appfinished)
    {
        return;
    }

    bool disconnect = false;
    const QList<QNetworkInterface> newNetworkInterfaces = findNewNetworkInterfaces();
    if (!newNetworkInterfaces.empty() && !networkConnectivity)
    {
        disconnect = true;
        networkConnectivity = true;
    }

    if (newNetworkInterfaces.empty())
    {
        MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, "No active network interfaces found");
        networkConnectivity = false;
    }
    else if (activeNetworkInterfaces.empty())
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
        disconnect |= checkNetworkInterfaces(newNetworkInterfaces);
    }

    reconnectIfNecessary(disconnect, newNetworkInterfaces);
}

void MegaApplication::checkMemoryUsage()
{
    auto numNodes = megaApi->getNumNodes();
    auto numLocalNodes = static_cast<unsigned long long>(megaApi->getNumLocalNodes());
    auto totalNodes = numNodes + numLocalNodes;
    auto transferCount = getTransfersModel()->getTransfersCount();
    auto totalTransfers =  transferCount.pendingUploads + transferCount.pendingDownloads;
    unsigned long long procesUsage = 0ULL;

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
    procesUsage =  static_cast<unsigned long long>(pmc.PrivateUsage);
#else
    #ifdef __APPLE__
        struct task_basic_info t_info;
        mach_msg_type_number_t t_info_count = TASK_BASIC_INFO_COUNT;

        if (KERN_SUCCESS == task_info(mach_task_self(),
                                      TASK_BASIC_INFO, (task_info_t)&t_info,
                                      &t_info_count))
        {
            procesUsage = static_cast<unsigned long long>(t_info.resident_size);
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
                 .arg(static_cast<float>(procesUsage) / static_cast<float>(totalNodes))
                 .arg(totalTransfers).toUtf8().constData());

    if (procesUsage > mMaxMemoryUsage)
    {
        mMaxMemoryUsage = procesUsage;
    }

    if (mMaxMemoryUsage > preferences->getMaxMemoryUsage()
            && mMaxMemoryUsage > 268435456 //256MB
            + 2028 * totalNodes // 2KB per node
            + 5120 * totalTransfers) // 5KB per transfer
    {
        long long currentTime = QDateTime::currentMSecsSinceEpoch();
        if (currentTime - preferences->getMaxMemoryReportTime() > 86400000)
        {
            preferences->setMaxMemoryUsage(mMaxMemoryUsage);
            preferences->setMaxMemoryReportTime(currentTime);
            mStatsEventHandler->sendEvent(AppStatsEvents::EventType::MEM_USAGE,
                                          { QString::number(mMaxMemoryUsage),
                                            QString::number(numNodes),
                                            QString::number(numLocalNodes) });
        }
    }
}

void MegaApplication::checkOverStorageStates(bool isOnboardingAboutClosing)
{
    if (!preferences->logged() ||
        ((!infoDialog || !infoDialog->isVisible()) && !Platform::getInstance()->isUserActive()))
    {
        return;
    }

    if (appliedStorageState == MegaApi::STORAGE_STATE_RED)
    {
        bool isOnboardingDialogVisible(false);
        if (auto dialogInfo = DialogOpener::findDialog<QmlDialogWrapper<Onboarding>>())
        {
            isOnboardingDialogVisible = dialogInfo->getDialog()->isVisible();
        }

        if ((!isOnboardingDialogVisible || isOnboardingAboutClosing) &&
            (!preferences->getOverStorageDialogExecution() ||
             ((QDateTime::currentMSecsSinceEpoch() - preferences->getOverStorageDialogExecution()) >
              Preferences::OQ_DIALOG_INTERVAL_MS)))
        {
            preferences->setOverStorageDialogExecution(QDateTime::currentMSecsSinceEpoch());
            mStatsEventHandler->sendEvent(AppStatsEvents::EventType::OVER_STORAGE_DIAL);
            showUpsellDialog(UpsellPlans::ViewMode::STORAGE_FULL);
        }
        else if (((QDateTime::currentMSecsSinceEpoch() - preferences->getOverStorageDialogExecution()) > Preferences::OQ_NOTIFICATION_INTERVAL_MS)
                     && (!preferences->getOverStorageNotificationExecution() || ((QDateTime::currentMSecsSinceEpoch() - preferences->getOverStorageNotificationExecution()) > Preferences::OQ_NOTIFICATION_INTERVAL_MS)))
        {
            preferences->setOverStorageNotificationExecution(QDateTime::currentMSecsSinceEpoch());
            mStatsEventHandler->sendEvent(AppStatsEvents::EventType::OVER_STORAGE_NOTIF);
            mOsNotifications->sendOverStorageNotification(Preferences::STATE_OVER_STORAGE);
        }

        if (infoDialog)
        {
            if (!preferences->getOverStorageDismissExecution()
                    || ((QDateTime::currentMSecsSinceEpoch() - preferences->getOverStorageDismissExecution()) > Preferences::OQ_UI_MESSAGE_INTERVAL_MS))
            {
                if (infoDialog->updateOverStorageState(Preferences::STATE_OVER_STORAGE))
                {
                    mStatsEventHandler->sendEvent(AppStatsEvents::EventType::OVER_STORAGE_MSG);
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
        bool isOnboardingDialogVisible(false);
        if (auto dialogInfo = DialogOpener::findDialog<QmlDialogWrapper<Onboarding>>())
        {
            isOnboardingDialogVisible = dialogInfo->getDialog()->isVisible();
        }
        if ((!isOnboardingDialogVisible || isOnboardingAboutClosing) &&
            (!preferences->getAlmostOverStorageDialogExecution() ||
             ((QDateTime::currentMSecsSinceEpoch() -
               preferences->getAlmostOverStorageDialogExecution()) >
              Preferences::OQ_DIALOG_INTERVAL_MS)))
        {
            preferences->setAlmostOverStorageDialogExecution(QDateTime::currentMSecsSinceEpoch());
            showUpsellDialog(UpsellPlans::ViewMode::STORAGE_ALMOST_FULL);
        }

        if (infoDialog)
        {
            if (((QDateTime::currentMSecsSinceEpoch() - preferences->getOverStorageDismissExecution()) > Preferences::ALMOST_OQ_UI_MESSAGE_INTERVAL_MS)
                         && (!preferences->getAlmostOverStorageDismissExecution() || ((QDateTime::currentMSecsSinceEpoch() - preferences->getAlmostOverStorageDismissExecution()) > Preferences::ALMOST_OQ_UI_MESSAGE_INTERVAL_MS)))
            {
                if (infoDialog->updateOverStorageState(Preferences::STATE_ALMOST_OVER_STORAGE))
                {
                    mStatsEventHandler->sendEvent(AppStatsEvents::EventType::ALMOST_OVER_STORAGE_MSG);
                }
            }
            else
            {
                infoDialog->updateOverStorageState(Preferences::STATE_OVER_STORAGE_DISMISSED);
            }
        }

        auto transferCount = getTransfersModel()->getTransfersCount();
        uint pendingTransfers =  transferCount.pendingUploads || transferCount.pendingDownloads;

        if (!pendingTransfers && ((QDateTime::currentMSecsSinceEpoch() - preferences->getOverStorageNotificationExecution()) > Preferences::ALMOST_OQ_UI_MESSAGE_INTERVAL_MS)
                              && ((QDateTime::currentMSecsSinceEpoch() - preferences->getOverStorageDialogExecution()) > Preferences::ALMOST_OQ_UI_MESSAGE_INTERVAL_MS)
                              && (!preferences->getAlmostOverStorageNotificationExecution() || (QDateTime::currentMSecsSinceEpoch() - preferences->getAlmostOverStorageNotificationExecution()) > Preferences::ALMOST_OQ_UI_MESSAGE_INTERVAL_MS))
        {
            preferences->setAlmostOverStorageNotificationExecution(QDateTime::currentMSecsSinceEpoch());
            mStatsEventHandler->sendEvent(AppStatsEvents::EventType::ALMOST_OVER_STORAGE_NOTIF);
            mOsNotifications->sendOverStorageNotification(Preferences::STATE_ALMOST_OVER_STORAGE);
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
                int64_t remainDaysOut(0);
                Utilities::getDaysToTimestamp(megaApi->getOverquotaDeadlineTs(), remainDaysOut);
                if (remainDaysOut > 0) //Only show notification if at least there is one day left
                {
                    preferences->setPayWallNotificationExecution(QDateTime::currentMSecsSinceEpoch());
                    mStatsEventHandler->sendEvent(AppStatsEvents::EventType::PAYWALL_NOTIF);
                    mOsNotifications->sendOverStorageNotification(Preferences::STATE_PAYWALL);
                }
            }

            closeUpsellStorageDialog();
        }
    }
    else
    {
        if (infoDialog)
        {
            infoDialog->updateOverStorageState(Preferences::STATE_BELOW_OVER_STORAGE);
        }

        closeUpsellStorageDialog();
    }

    if (infoDialog)
    {
        infoDialog->setOverQuotaMode(appliedStorageState == MegaApi::STORAGE_STATE_RED
                                     || appliedStorageState == MegaApi::STORAGE_STATE_PAYWALL);
    }
}

void MegaApplication::checkOverQuotaStates()
{
    mTransferQuota->checkQuotaAndAlerts();
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

    AccountDetailsManager::instance()->periodicUpdate();

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
                mStatsEventHandler->sendEvent(AppStatsEvents::EventType::UPDATE_OK);
            }

            checkMemoryUsage();
            mThreadPool->push(
                [=]()
                {
                    Utilities::queueFunctionInAppThread(
                        [=]()
                        {
                            checkOverStorageStates();
                            checkOverQuotaStates();
                        });
                });
        }

        onGlobalSyncStateChanged(megaApi);
    }

    sendPeriodicStats();

    if (trayIcon)
    {
#ifdef Q_OS_LINUX
        const QString xdgEnvVar = qEnvironmentVariable("XDG_CURRENT_DESKTOP");
        if (counter == 4 && !xdgEnvVar.isEmpty() && xdgEnvVar == QString::fromUtf8("XFCE"))
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

    emit requestAppState(AppState::FINISHED);

    qInstallMessageHandler(0);

    periodicTasksTimer->stop();
    networkCheckTimer->stop();
    stopUpdateTask();
    Platform::getInstance()->stopShellDispatcher();

    for (auto localFolder : model->getLocalFolders(SyncInfo::AllHandledSyncTypes))
    {
        Platform::getInstance()->notifyItemChange(localFolder, MegaApi::STATE_NONE);
    }

    Preferences::instance()->clearTempTransfersPath();
    PowerOptions::appShutdown();

    DialogOpener::closeAllDialogs();
    QmlDialogManager::instance()->forceCloseOnboardingDialog();
    QmlManager::instance()->finish();

    if(mBlockingBatch.isValid())
    {
        mBlockingBatch.cancelTransfer();
    }

    delete mLinkProcessor;
    mLinkProcessor = nullptr;
    delete mSetManager;
    mSetManager = nullptr;
    delete httpServer;
    httpServer = nullptr;
    delete uploader;
    uploader = nullptr;
    delete downloader;
    downloader = nullptr;
    delete delegateListener;
    delegateListener = nullptr;

    mGfxProvider.reset();
    mUserMessageController.reset();
    mSyncReminderNotificationManager->deleteLater();
    infoDialog->deleteLater();

    // Delete menus and menu items
    deleteMenu(initialTrayMenu);
    deleteMenu(infoDialogMenu);
    deleteMenu(guestMenu);
#ifdef _WIN32
    deleteMenu(windowsMenu);
#endif

    removeSyncsAndBackupsMenus();

    preferences->setLastExit(QDateTime::currentMSecsSinceEpoch());

    // Remove models using deleteLater to be sure that they are removed after removing Transfer and
    // Stalled issues dialogs. Otherwise we need to set to null the view models as the views will
    // contain dangling pointers
    mStalledIssuesModel->deleteLater();
    mStalledIssuesModel = nullptr;
    mTransfersModel->deleteLater();
    mTransfersModel = nullptr;

    // Ensure that there aren't objects deleted with deleteLater()
    // that may try to access megaApi after
    // their deletion
    // Besides that, do not set any preference setting after this line, it won´t be persistent.
    QApplication::processEvents();

    QTMegaApiManager::removeMegaApis();

    trayIcon->deleteLater();
    trayIcon = nullptr;

    logger.reset();

    if (reboot)
    {
#ifndef __APPLE__
        QString app = MegaApplication::applicationFilePath();
        QProcess::startDetached(app, {});
#else
        QString app = MegaApplication::applicationDirPath();
        QString launchCommand = QString::fromUtf8("open");
        QStringList args = QStringList();

        QDir appPath(app);
        appPath.cdUp();
        appPath.cdUp();

        args.append(QString::fromUtf8("-n"));
        args.append(appPath.absolutePath());
        QProcess::startDetached(launchCommand, args);
#endif

#ifdef WIN32
        Sleep(2000);
#else
        sleep(2);
#endif
    }
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
}

void MegaApplication::onAboutClicked()
{
    if (appfinished)
    {
        return;
    }

    mStatsEventHandler->sendTrackedEvent(AppStatsEvents::EventType::MENU_ABOUT_CLICKED,
                                         sender(), aboutAction, true);
    showChangeLog();
}

QString MegaApplication::getFormattedDateByCurrentLanguage(const QDateTime &datetime, QLocale::FormatType format) const
{
    return DateTimeFormatter::create(currentLanguageCode, datetime, format);
}

void MegaApplication::raiseInfoDialog()
{
    if (QmlDialogManager::instance()->raiseGuestDialog())
    {
        return;
    }

    if (infoDialog)
    {
        infoDialog->updateDialogState();
        DialogOpener::raiseAllDialogs();

#ifdef __APPLE__
        Platform::getInstance()->raiseFileFolderSelectors();
#endif

        infoDialog->show();
        infoDialog->raise();
        infoDialog->activateWindow();
        infoDialog->highDpiResize.queueRedraw();
    }
}

bool MegaApplication::isShellNotificationProcessingOngoing()
{
    return mProcessingShellNotifications > 0;
}

void MegaApplication::showInfoDialog()
{
    if (appfinished)
    {
        return;
    }

    if (AppState::instance()->getAppState() != AppState::NOMINAL)
    {
        return;
    }

#ifdef Q_OS_LINUX
    if (showStatusAction && megaApi)
    {
        megaApi->retryPendingConnections();
    }
#endif

#ifdef WIN32

    if (QWidget *anyModalWindow = QApplication::activeModalWidget())
    {
        if(anyModalWindow->windowModality() == Qt::ApplicationModal)
        {
            // If the InfoDialog has opened any MessageBox (eg. enter your email),
            // those must be closed first (as we are executing from that dialog's message loop!)
            // Bring that dialog to the front for the user to dismiss.s
            DialogOpener::raiseAllDialogs();
            return;
        }
    }

    if (infoDialog)
    {
        // in case the screens have changed, eg. laptop with 2 monitors attached (200%, main:100%, 150%),
        // lock screen, unplug monitors, wait 30s, plug monitors, unlock screen:
        // infoDialog may be double size and only showing 1/4 or 1/2
        infoDialog->setWindowFlags(Qt::FramelessWindowHint);
        infoDialog->setWindowFlags(Qt::FramelessWindowHint | Qt::Popup | Qt::NoDropShadowWindowHint);
    }
#endif

    const bool transferQuotaWaitTimeExpired{transferOverQuotaWaitTimeExpiredReceived && !mTransferQuota->isOverQuota()};
    const bool loggedAndNotBandwidthOverquota{preferences && preferences->logged()};
    if (loggedAndNotBandwidthOverquota && transferQuotaWaitTimeExpired)
    {
        transferOverQuotaWaitTimeExpiredReceived = false;
        AccountDetailsManager::instance()->updateUserStats(
            AccountDetailsManager::Flag::TRANSFER,
            true,
            USERSTATS_BANDWIDTH_TIMEOUT_SHOWINFODIALOG);
    }

    if (infoDialog)
    {
        if (!infoDialog->isVisible() || ((infoDialog->windowState() & Qt::WindowMinimized)) )
        {
            if (storageState == MegaApi::STORAGE_STATE_RED)
            {
                mStatsEventHandler->sendEvent(AppStatsEvents::EventType::MAIN_DIAL_WHILE_OVER_QUOTA);
            }
            else if (storageState == MegaApi::STORAGE_STATE_ORANGE)
            {
                mStatsEventHandler->sendEvent(AppStatsEvents::EventType::MAIN_DIAL_WHILE_ALMOST_OVER_QUOTA);
            }

            raiseInfoDialog();
        }
        else
        {
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

    if(!mStatusController->isAccountBlocked())
    {
        AccountDetailsManager::instance()->updateUserStats(AccountDetailsManager::Flag::TRANSFER,
                                                           true,
                                                           USERSTATS_SHOWMAINDIALOG);
    }
}

void MegaApplication::showInfoDialogNotifications()
{
    showInfoDialog();
    infoDialog->showNotifications();
}

void MegaApplication::deleteMenu(QMenu *menu)
{
    if (menu)
    {
        clearMenu(menu, true);
        menu->deleteLater();
    }
}

void MegaApplication::clearMenu(QMenu* menu, bool deleteAction)
{
    if (menu)
    {
        QList<QAction *> actions = menu->actions();
        for (int i = 0; i < actions.size(); i++)
        {
            menu->removeAction(actions[i]);
            if (deleteAction)
            {
                delete actions[i];
            }
        }
    }
}

void MegaApplication::startHttpServer()
{
    if (!httpServer)
    {
        httpServer = new HTTPServer(megaApi, Preferences::HTTP_PORT);
        ConnectServerSignals(httpServer);
        MegaApi::log(MegaApi::LOG_LEVEL_INFO, "Local HTTP server started");
    }
}

void MegaApplication::initLocalServer()
{
    if (!httpServer)
    {
        startHttpServer();
    }
}

bool MegaApplication::eventFilter(QObject *obj, QEvent *e)
{
    if (!appfinished && obj == infoDialogMenu)
    {
        if (e->type() == QEvent::Leave)
        {
            if (lastHovered)
            {
                lastHovered->setHighlight(false);
                lastHovered = nullptr;
            }
        }
    }

    if (!appfinished && e->type() == QEvent::LanguageChange)
    {
        emit languageChanged();
    }

    return QApplication::eventFilter(obj, e);
}

void MegaApplication::createInfoDialog()
{
    infoDialog = new InfoDialog(this);
    TokenParserWidgetManager::instance()->applyCurrentTheme(infoDialog);
    TokenParserWidgetManager::instance()->registerWidgetForTheming(infoDialog);
    connect(infoDialog.data(), &InfoDialog::dismissStorageOverquota,
            this, &MegaApplication::onDismissStorageOverquota);
    connect(infoDialog.data(), &InfoDialog::transferOverquotaMsgVisibilityChange,
            mTransferQuota.get(), &TransferQuota::onTransferOverquotaVisibilityChange);
    connect(infoDialog.data(), &InfoDialog::almostTransferOverquotaMsgVisibilityChange,
            mTransferQuota.get(), &TransferQuota::onAlmostTransferOverquotaVisibilityChange);
    connect(infoDialog.data(), &InfoDialog::userActivity,
            this, &MegaApplication::registerUserActivity);
    connect(mTransferQuota.get(), &TransferQuota::sendState,
            infoDialog.data(), &InfoDialog::setBandwidthOverquotaState);
    connect(mTransferQuota.get(), &TransferQuota::overQuotaMessageNeedsToBeShown,
            infoDialog.data(), &InfoDialog::enableTransferOverquotaAlert);
    connect(mTransferQuota.get(), &TransferQuota::almostOverQuotaMessageNeedsToBeShown,
            infoDialog.data(), &InfoDialog::enableTransferAlmostOverquotaAlert);
    connect(infoDialog, SIGNAL(cancelScanning()),
            this, SLOT(cancelScanningStage()));
    connect(this, &MegaApplication::addBackup,
            infoDialog.data(), &InfoDialog::onAddBackup);
    connect(mUserMessageController.get(), &UserMessageController::unseenAlertsChanged,
            infoDialog.data(), &InfoDialog::onUnseenAlertsChanged);
    scanStageController.updateReference(infoDialog);
}

void MegaApplication::updateUsedStorage(const bool sendEvent)
{
    if (storageState == MegaApi::STORAGE_STATE_RED
            && receivedStorageSum < preferences->totalStorage())
    {
        preferences->setUsedStorage(preferences->totalStorage());
        if(sendEvent)
        {
            mStatsEventHandler->sendEvent(AppStatsEvents::EventType::RED_LIGHT_USED_STORAGE_MISMATCH);
        }
    }
    else
    {
        preferences->setUsedStorage(receivedStorageSum);
    }
}

std::shared_ptr<TransferQuota> MegaApplication::getTransferQuota() const
{
    return mTransferQuota;
}

int MegaApplication::getAppliedStorageState() const
{
    return appliedStorageState;
}

bool MegaApplication::isAppliedStorageOverquota() const
{
    return appliedStorageState == MegaApi::STORAGE_STATE_RED || appliedStorageState == MegaApi::STORAGE_STATE_PAYWALL;
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
    QIcon ic = QIcon(QString::fromUtf8("://images/icon_syncing_mac") +
                     QString::number(scanningAnimationIndex) + QString::fromUtf8(".png"));
#ifdef __APPLE__
    ic.setIsMask(true);
#endif
    trayIcon->setIcon(ic);
}

QList<QNetworkInterface> MegaApplication::findNewNetworkInterfaces()
{
    QList<QNetworkInterface> newInterfaces;
    const QList<QNetworkInterface> configs = QNetworkInterface::allInterfaces();
    for (const auto& networkInterface : configs)
    {
        QString interfaceName = networkInterface.humanReadableName();
        QNetworkInterface::InterfaceFlags flags = networkInterface.flags();
        if (isActiveNetworkInterface(interfaceName, flags))
        {
            if (logger->isDebug())
            {
                MegaApi::log(MegaApi::LOG_LEVEL_INFO, QString::fromUtf8("Active network interface: %1").arg(interfaceName).toUtf8().constData());
            }

            const int numActiveIPs = countActiveIps(networkInterface.addressEntries());
            if (numActiveIPs > 0)
            {
                lastActiveTime = QDateTime::currentMSecsSinceEpoch();
                newInterfaces.append(networkInterface);
            }
        }
        else if (logger->isDebug())
        {
            MegaApi::log(MegaApi::LOG_LEVEL_INFO, QString::fromUtf8("Ignored network interface: %1 Flags: %2")
                         .arg(interfaceName)
                         .arg(QString::number(flags)).toUtf8().constData());
        }
    }
    return newInterfaces;
}

bool MegaApplication::checkNetworkInterfaces(const QList<QNetworkInterface> &newNetworkInterfaces) const
{
    bool disconnect = false;
    for (const auto& networkInterface : newNetworkInterfaces)
    {
        disconnect |= checkNetworkInterface(networkInterface);
    }
    return disconnect;
}

bool MegaApplication::checkNetworkInterface(const QNetworkInterface &newNetworkInterface) const
{
    bool disconnect = false;
    auto interfaceIt = std::find_if(activeNetworkInterfaces.begin(), activeNetworkInterfaces.end(), [&newNetworkInterface](const QNetworkInterface& currentInterface){
        return (currentInterface.name() == newNetworkInterface.name());
    });

    if (interfaceIt == activeNetworkInterfaces.end())
    {
        //New interface
        MegaApi::log(MegaApi::LOG_LEVEL_INFO, QString::fromUtf8("New working network interface detected (%1)").arg(newNetworkInterface.humanReadableName()).toUtf8().constData());
        disconnect = true;
    }
    else
    {
        disconnect |= checkNetworkAddresses(*interfaceIt, newNetworkInterface);
    }
    return disconnect;
}

bool MegaApplication::checkNetworkAddresses(const QNetworkInterface& oldNetworkInterface, const QNetworkInterface &newNetworkInterface) const
{
    bool disconnect = false;
    const auto newAddresses = newNetworkInterface.addressEntries();
    if (newAddresses.size() != oldNetworkInterface.addressEntries().size())
    {
        MegaApi::log(MegaApi::LOG_LEVEL_INFO, "Local IP change detected");
        disconnect = true;
    }
    else
    {
        for (const auto& newAddress : newAddresses)
        {
            disconnect |= checkIpAddress(newAddress.ip(), oldNetworkInterface.addressEntries(), newNetworkInterface.name());
        }
    }
    return disconnect;
}

bool MegaApplication::checkIpAddress(const QHostAddress& ip, const QList<QNetworkAddressEntry>& oldAddresses, const QString& newNetworkInterfaceName) const
{
    bool disconnect = false;
    switch (ip.protocol())
    {
        case QAbstractSocket::IPv4Protocol:
        case QAbstractSocket::IPv6Protocol:
        {
            auto addressIt = std::find_if(oldAddresses.begin(), oldAddresses.end(), [&ip](const QNetworkAddressEntry& address){
               return address.ip().toString() == ip.toString();
            });

            if (addressIt == oldAddresses.end())
            {
                //New IP
                const QString addressToLog = obfuscateIfNecessary(ip);
                MegaApi::log(MegaApi::LOG_LEVEL_INFO, QString::fromUtf8("New IP detected (%1) for interface %2").arg(addressToLog).arg(newNetworkInterfaceName).toUtf8().constData());
                disconnect = true;
            }
        }
        default:
            break;
    }
    return disconnect;
}

bool MegaApplication::isActiveNetworkInterface(const QString& interfaceName, const QNetworkInterface::InterfaceFlags flags)
{
    return (flags & (QNetworkInterface::IsUp | QNetworkInterface::IsRunning)) &&
            !(interfaceName == QString::fromUtf8("Teredo Tunneling Pseudo-Interface"));
}

int MegaApplication::countActiveIps(const QList<QNetworkAddressEntry> &addresses) const
{
    int numActiveIPs = 0;
    for (const auto& address : addresses)
    {
        QHostAddress ip = address.ip();
        switch (ip.protocol())
        {
            case QAbstractSocket::IPv4Protocol:
                if (!isLocalIpv4(ip.toString()))
                {
                    logIpAddress("Active IPv4", ip);
                    numActiveIPs++;
                }
                else
                {
                    logIpAddress("Ignored IPv4", ip);
                }
                break;
            case QAbstractSocket::IPv6Protocol:
                if (!isLocalIpv6(ip.toString()))
                {
                    logIpAddress("Active IPv6", ip);
                    numActiveIPs++;
                }
                else
                {
                    logIpAddress("Ignored IPv6", ip);
                }
                break;
            default:
                logIpAddress("Ignored IPv6", ip);
                break;
        }
    }
    return numActiveIPs;
}

bool MegaApplication::isLocalIpv4(const QString& address)
{
    return address.startsWith(QString::fromUtf8("127."), Qt::CaseInsensitive) ||
           address.startsWith(QString::fromUtf8("169.254."), Qt::CaseInsensitive);
}

bool MegaApplication::isLocalIpv6(const QString &address)
{
    return address.startsWith(QString::fromUtf8("FE80:"), Qt::CaseInsensitive) ||
           address.startsWith(QString::fromUtf8("FD00:"), Qt::CaseInsensitive) ||
           address == QString::fromUtf8("::1");
}

void MegaApplication::logIpAddress(const char* message, const QHostAddress &ipAddress) const
{
    if (logger->isDebug())
    {
        // these are quite frequent (30s) and usually there are around 10.
        // so, just log when debug logging has been activated (ie, file to desktop)
        // otherwise, it's quite distracting
        const QString logMessage = QString::fromUtf8(message) + QString::fromUtf8(": %1");
        const QString addressToLog = obfuscateIfNecessary(ipAddress);
        MegaApi::log(MegaApi::LOG_LEVEL_INFO, logMessage.arg(addressToLog).toUtf8().constData());
    }
}

QString MegaApplication::obfuscateIfNecessary(const QHostAddress &ipAddress) const
{
    return (logger->isDebug()) ? ipAddress.toString() : obfuscateAddress(ipAddress);
}

QString MegaApplication::obfuscateAddress(const QHostAddress &ipAddress)
{
    if (ipAddress.protocol() == QAbstractSocket::IPv4Protocol)
    {
        return obfuscateIpv4Address(ipAddress);
    }
    else if (ipAddress.protocol() == QAbstractSocket::IPv6Protocol)
    {
        return obfuscateIpv6Address(ipAddress);
    }
    return ipAddress.toString();
}

QString MegaApplication::obfuscateIpv4Address(const QHostAddress &ipAddress)
{
    const QStringList addressParts = ipAddress.toString().split(QChar::fromLatin1('.'));
    if (addressParts.size() == 4)
    {
        auto itAddressPart = addressParts.begin()+2;
        return QString::fromUtf8("%1.%1.%2.%3").arg(QString::fromUtf8("XXX"))
                                               .arg(*itAddressPart++).arg(*itAddressPart);
    }
    return QString::fromUtf8("XXX.XXX.XXX.XXX");
}

QString MegaApplication::obfuscateIpv6Address(const QHostAddress &ipAddress)
{
    const QStringList addressParts = explodeIpv6(ipAddress);
    if (addressParts.size() == 8)
    {
        auto itAddressPart = addressParts.begin()+4;
        return QString::fromUtf8("%1:%1:%1:%1:%2:%3:%4:%5").arg(QString::fromUtf8("XXXX"))
                                                           .arg(*itAddressPart++).arg(*itAddressPart++)
                                                           .arg(*itAddressPart++).arg(*itAddressPart);
    }
    return QString::fromUtf8("XXXX:XXXX:XXXX:XXXX:XXXX:XXXX");
}

QStringList MegaApplication::explodeIpv6(const QHostAddress &ipAddress)
{
    QStringList addressParts;
    auto ipv6 = ipAddress.toIPv6Address();
    for (int i=0; i<8; ++i) {
        const int baseI = i*2;
        addressParts.push_back(QString::fromUtf8("%1%2").arg(ipv6[baseI], 0, 16, QChar::fromLatin1('0'))
                                                        .arg(ipv6[baseI+1], 0, 16, QChar::fromLatin1('0')));
    }
    return addressParts;
}

void MegaApplication::reconnectIfNecessary(const bool disconnected, const QList<QNetworkInterface> &newNetworkInterfaces)
{
    if (disconnected || isIdleForTooLong())
    {
        MegaApi::log(MegaApi::LOG_LEVEL_INFO, "Reconnecting due to local network changes");
        megaApi->retryPendingConnections(true, true);
        activeNetworkInterfaces = newNetworkInterfaces;
        lastActiveTime = QDateTime::currentMSecsSinceEpoch();
    }
    else
    {
        MegaApi::log(MegaApi::LOG_LEVEL_INFO, "Local network adapters haven't changed");
    }
}

bool MegaApplication::isIdleForTooLong() const
{
    return (QDateTime::currentMSecsSinceEpoch() - lastActiveTime) > Preferences::MAX_IDLE_TIME_MS;
}

void MegaApplication::startUpload(const QString& rawLocalPath, MegaNode* target, MegaCancelToken* cancelToken)
{
    auto localPathArray = QDir::toNativeSeparators(rawLocalPath).toUtf8();
    const char* appData = nullptr;
    const char* fileName = nullptr;
    const bool startFirst = false;
    const bool isSrcTemporary = false;
    int64_t mtime = ::mega::MegaApi::INVALID_CUSTOM_MOD_TIME;
    MegaTransferListener* listener = nullptr;

    megaApi->startUpload(localPathArray.constData(), target, fileName, mtime, appData, isSrcTemporary, startFirst, cancelToken, listener);
}

void MegaApplication::cancelScanningStage()
{
    mBlockingBatch.cancelTransfer();
    transferProgressController.stopUiUpdating();
}

void MegaApplication::transferBatchFinished(unsigned long long appDataId, bool fromCancellation)
{
    if(mBlockingBatch.isValid())
    {
        QString message = QString::fromUtf8("Transferbatch scanning finished");
        MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, message.toUtf8().constData());

        mBlockingBatch.onScanCompleted(appDataId);
        if (mBlockingBatch.isBlockingStageFinished() || mBlockingBatch.isCancelled())
        {
            scanStageController.stopDelayedScanStage(fromCancellation);
            transferProgressController.stopUiUpdating();
            mFolderTransferListener->reset();
        }
    }
}

void MegaApplication::logBatchStatus(const char* tag)
{
#ifdef DEBUG
    QString logMessage = QString::fromLatin1("%1 : %2").arg(QString::fromUtf8(tag)).arg(mBlockingBatch.description());
    MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, logMessage.toUtf8().constData());
#else
    Q_UNUSED(tag)
#endif
}

void MegaApplication::enableTransferActions(bool enable)
{
    if (appfinished)
    {
        return;
    }

#ifdef _WIN32
    if(updateAvailable && windowsUpdateAction)
    {
        windowsUpdateAction->setEnabled(enable);
    }
    windowsSettingsAction->setEnabled(enable);
    windowsImportLinksAction->setEnabled(enable);
    windowsFilesAction->setEnabled(enable);
    windowsUploadAction->setEnabled(enable);
    windowsDownloadAction->setEnabled(enable);
    windowsStreamAction->setEnabled(enable);
#endif

    if(updateAvailable && updateAction)
    {
        updateAction->setEnabled(enable);
    }

    guestSettingsAction->setEnabled(enable);
    importLinksAction->setEnabled(enable);
    uploadAction->setEnabled(enable);
    downloadAction->setEnabled(enable);
    streamAction->setEnabled(enable);
    settingsAction->setEnabled(enable);

    if (mSyncs2waysMenu)
    {
        mSyncs2waysMenu->setEnabled(enable);
    }
    if (mBackupsMenu)
    {
        mBackupsMenu->setEnabled(enable);
    }
}

void MegaApplication::startingUpload()
{
    if (noUploadedStarted && mBlockingBatch.hasNodes())
    {
        noUploadedStarted = false;
        scanStageController.startDelayedScanStage();
    }
}

void MegaApplication::ConnectServerSignals(HTTPServer* server)
{
    connect(server, &HTTPServer::onLinkReceived, this, &MegaApplication::externalLinkDownload, Qt::QueuedConnection);
    connect(server, &HTTPServer::onExternalDownloadRequested, this, &MegaApplication::externalDownload, Qt::QueuedConnection);
    connect(server, &HTTPServer::onExternalDownloadRequestFinished, this, &MegaApplication::processDownloads, Qt::QueuedConnection);
    connect(server, &HTTPServer::onExternalFileUploadRequested, this, &MegaApplication::externalFileUpload, Qt::QueuedConnection);
    connect(server, &HTTPServer::onExternalFolderUploadRequested, this, &MegaApplication::externalFolderUpload, Qt::QueuedConnection);
    connect(server, &HTTPServer::onExternalFolderSyncRequested, this, &MegaApplication::externalFolderSync, Qt::QueuedConnection);
    connect(server, &HTTPServer::onExternalOpenTransferManagerRequested, this, &MegaApplication::externalOpenTransferManager, Qt::QueuedConnection);
    connect(server, &HTTPServer::onExternalShowInFolderRequested, this, &MegaApplication::openFolderPath, Qt::QueuedConnection);
    connect(server, &HTTPServer::onExternalAddBackup, this, &MegaApplication::externalAddBackup, Qt::QueuedConnection);
    connect(server, &HTTPServer::onExternalDownloadSetRequested, this, &MegaApplication::processSetDownload, Qt::QueuedConnection);
}

bool MegaApplication::dontAskForExitConfirmation(bool force)
{
    return force || !megaApi->isLoggedIn() || mTransfersModel->hasActiveTransfers() == 0;
}

void MegaApplication::exitApplication()
{
    reboot = false;
    trayIcon->hide();
    QApplication::exit();
}

QString MegaApplication::getDefaultUploadPath()
{
    QString  defaultFolderPath;
    QStringList paths = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
    if (paths.size())
    {
        defaultFolderPath = paths.at(0);
    }
    return defaultFolderPath;
}

MegaApplication::NodeCount MegaApplication::countFilesAndFolders(const QStringList& paths)
{
    NodeCount count;
    count.files = 0;
    count.folders = 0;

    for (const auto& path : paths)
    {
        count.folders++;
        QDirIterator it (path, QDir::AllEntries | QDir::Hidden | QDir::System | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
        while (it.hasNext())
        {
            it.next();
            if (it.fileInfo().isDir())
            {
                count.folders++;
            }
            else if (it.fileInfo().isFile())
            {
                count.files++;
            }
        }
    }

    return count;
}

void MegaApplication::processUploads(const QStringList &uploads)
{
    uploadQueue.append(uploads);
    processUploadQueue(folderUploadTarget);
}

void MegaApplication::processUpgradeSecurityEvent()
{
    // Get outShares paths, to show them to the user
    QSet<QString> outSharesStrings;
    std::unique_ptr<MegaShareList> outSharesList (megaApi->getOutShares());
    for (int i = 0; i < outSharesList->size(); ++i)
    {
        MegaHandle handle = outSharesList->get(i)->getNodeHandle();
        std::unique_ptr<char[]> path (megaApi->getNodePathByNodeHandle(handle));
        outSharesStrings << QString::fromUtf8(path.get());
    }

    // Prepare the dialog
    QString message = tr("Your account's security is now being upgraded. "
                         "This will happen only once. If you have seen this message for "
                         "this account before, press Cancel.");
    if (!outSharesStrings.isEmpty())
    {
        message.append(QLatin1String("<br><br>"));
        message.append(tr("You are currently sharing the following folder: %1", "", outSharesStrings.size())
                  .arg(outSharesStrings.values().join(QLatin1String(", "))));
    }

    MessageDialogInfo msgInfo;
    msgInfo.titleText = tr("Security upgrade");
    msgInfo.descriptionText = message;
    msgInfo.buttons = QMessageBox::Ok|QMessageBox::Cancel;
    msgInfo.textFormat = Qt::RichText;
    msgInfo.finishFunc = [this](QPointer<MessageDialogResult> msg)
    {
        if (msg->result() == QMessageBox::Ok && !appfinished)
        {
            auto listener = RequestListenerManager::instance().registerAndGetCustomFinishListener(
                this,
                [=](::mega::MegaRequest* request, ::mega::MegaError* e) {
                    if (e->getErrorCode() != MegaError::API_OK)
                    {
                        showErrorMessage(tr("Failed to ugrade security. Error: %1")
                                             .arg(tr(e->getErrorString())));
                        exitApplication();
                    }
            });

            megaApi->upgradeSecurity(listener.get());
        }
        else
        {
            exitApplication();
        }
    };

    MessageDialogOpener::information(msgInfo);
}

QQueue<QString> MegaApplication::createQueue(const QStringList &newUploads) const
{
    QQueue<QString> newUploadQueue;
    foreach(QString file, newUploads)
    {
        newUploadQueue.append(file);
    }
    return newUploadQueue;
}

void MegaApplication::onFolderTransferUpdate(FolderTransferUpdateEvent event)
{
    if (appfinished)
    {
        return;
    }

    //Top Level transfers has finish its scanning
    if(event.stage >= MegaTransfer::STAGE_TRANSFERRING_FILES)
    {
        auto appDataId = TransferMetaDataContainer::appDataToId(event.appData.c_str());
        if(appDataId.first)
        {
            if(auto data = TransferMetaDataContainer::getAppDataById(appDataId.second))
            {
                data->topLevelFolderScanningFinished(event.filecount);
            }
        }
    }

    transferProgressController.update(event);
}

void MegaApplication::onNotificationProcessed()
{
    --mProcessingShellNotifications;
    if (mProcessingShellNotifications <= 0)
    {
        emit shellNotificationsProcessed();
    }
}

void MegaApplication::clearDownloadAndPendingLinks()
{
    if (downloadQueue.size() || pendingLinks.size())
    {
        for (QQueue<WrappedNode>::iterator it = downloadQueue.begin(); it != downloadQueue.end();
             ++it)
        {
            HTTPServer::onTransferDataUpdate((*it).getMegaNode()->getHandle(),
                                             MegaTransfer::STATE_CANCELLED,
                                             0,
                                             0,
                                             0,
                                             QString());
        }

        for (QMap<QString, QString>::iterator it = pendingLinks.begin(); it != pendingLinks.end(); it++)
        {
            QString link = it.key();
            QString handle = link.mid(18, 8);
            HTTPServer::onTransferDataUpdate(megaApi->base64ToHandle(handle.toUtf8().constData()),
                                             MegaTransfer::STATE_CANCELLED, 0, 0, 0, QString());
        }

        downloadQueue.clear();
        pendingLinks.clear();
        showInfoMessage(tr("Transfer canceled"));
    }
}

void MegaApplication::unlink(bool keepLogs)
{
    if (appfinished)
    {
        return;
    }

    // Reset fields that will be initialized again upon login
    downloadQueue.clear();
    mRootNode.reset();
    mRubbishNode.reset();
    mVaultNode.reset();
    if(megaApi->isLoggedIn())
    {
        megaApi->logout(true, nullptr);
    }
    megaApiFolders->setAccountAuth(nullptr);
    DialogOpener::closeAllDialogs();

    // Reset desktop integration
    Platform::getInstance()->notifyAllSyncFoldersRemoved();
    Platform::getInstance()->disableContextMenu(false);

    AccountDetailsManager::instance()->reset();

    if (!keepLogs)
    {
        logger->cleanLogs();
    }

    // When unlinking to solve a sdk fatal issue, change back to nominal after unlink.
    if (AppState::instance()->getAppState() == AppState::FATAL_ERROR)
    {
        emit requestAppState(AppState::NOMINAL);
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
        for (auto syncPath : model->getLocalFolders(SyncInfo::AllHandledSyncTypes))
        {
            if (!syncPath.isEmpty())
            {
                QDir cacheDir(syncPath + QDir::separator() + QString::fromUtf8(MEGA_DEBRIS_FOLDER));
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

                        QDateTime creationTime(cacheFolder.birthTime());
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
    DesktopNotifications::NotificationInfo info;
    info.message = message;
    info.title = title;

    showInfoMessage(info);
}

void MegaApplication::showInfoMessage(DesktopNotifications::NotificationInfo info)
{
    if (appfinished)
    {
        return;
    }

    MegaApi::log(MegaApi::LOG_LEVEL_INFO, info.message.toUtf8().constData());

    if (mOsNotifications)
    {
#ifdef __APPLE__
        // In case this method is called from another thread
        Utilities::queueFunctionInAppThread(
            [this]()
            {
                if (infoDialog && infoDialog->isVisible())
                {
                    infoDialog->hide();
                }
            });
#endif
        lastTrayMessage = info.message;
        mOsNotifications->sendInfoNotification(info);
    }
    else
    {
        MessageDialogInfo msgInfo;
        msgInfo.descriptionText = info.message;
        MessageDialogOpener::information(msgInfo);
    }
}

void MegaApplication::showWarningMessage(QString message, QString title)
{
    if (appfinished)
    {
        return;
    }

    MegaApi::log(MegaApi::LOG_LEVEL_WARNING, message.toUtf8().constData());

    if (mOsNotifications)
    {
        lastTrayMessage = message;
        mOsNotifications->sendWarningNotification(title, message);
    }
    else
    {
        MessageDialogInfo msgInfo;
        msgInfo.descriptionText = message;
        MessageDialogOpener::warning(msgInfo);
    }
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
    if (mOsNotifications)
    {
#ifdef __APPLE__
        if (infoDialog && infoDialog->isVisible())
        {
            infoDialog->hide();
        }
#endif
        mOsNotifications->sendErrorNotification(title, message);
    }
    else
    {
        MessageDialogInfo msgInfo;
        msgInfo.descriptionText = message;
        MessageDialogOpener::critical(msgInfo);
    }
}

void MegaApplication::showNotificationMessage(QString message, QString title)
{
    if (appfinished)
    {
        return;
    }

    MegaApi::log(MegaApi::LOG_LEVEL_INFO, message.toUtf8().constData());

    if (mOsNotifications)
    {
        lastTrayMessage = message;
        mOsNotifications->sendInfoNotification(title, message);
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

    if (connections >= ParallelConnectionsValues::getMinValue() &&
        connections <= ParallelConnectionsValues::getMaxValue())
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
        updateThread = nullptr;
        updateTask = nullptr;
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

            QStringList arguments = proxyURL.split(QString::fromUtf8(":"));
            if (arguments.size() == 2)
            {
                proxy.setType(QNetworkProxy::HttpProxy);
                proxy.setHostName(arguments[0]);
                proxy.setPort(arguments[1].toUShort());
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

    {
        QMutexLocker locker(&mMutexOpenUrls);

        //Remove outdated url refs
        QMutableMapIterator<QUrl, std::chrono::system_clock::time_point> it(mOpenUrlsClusterTs);
        while (it.hasNext())
        {
            it.next();

            const auto elapsedTime = std::chrono::system_clock::now() - it.value();
            if(elapsedTime > openUrlClusterMaxElapsedTime)
            {
                it.remove();
            }
        }

        //Check if URl was notified within last openUrlClusterMaxElapsedTime
        const auto megaUrlIterator = mOpenUrlsClusterTs.find(url);
        const auto itemFound(megaUrlIterator != mOpenUrlsClusterTs.end());
        if(itemFound)
        {
            MegaApi::log(MegaApi::LOG_LEVEL_INFO, "Session transfer to URL already managed");
            return;
        }

        mOpenUrlsClusterTs.insert(url, std::chrono::system_clock::now());
    }

    megaApi->getSessionTransferURL(url.toString(QUrl::RemoveScheme).toUtf8().constData());
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
        Utilities::openUrl(QUrl::fromLocalFile(path));
    }
    else
    {
        #ifdef WIN32
        if (path.startsWith(QString::fromUtf8("\\\\?\\")))
        {
            path = path.mid(4);
        }
        #endif
        Platform::getInstance()->showInFolder(path);
    }
}

void MegaApplication::clearUserAttributes()
{
    if (infoDialog)
    {
        infoDialog->clearUserAttributes();
    }

    Utilities::removeAvatars();

    UserAttributes::UserAttributesManager::instance().reset();
}

void MegaApplication::checkOperatingSystem()
{
#ifdef MEGASYNC_DEPRECATED_OS
    if (!preferences->isOneTimeActionDone(Preferences::ONE_TIME_ACTION_OS_TOO_OLD))
    {
        MessageDialogInfo msgInfo;
        QString message =
            tr("Please consider updating your operating system.") + QString::fromUtf8("\n") +
            tr("MEGAsync will continue to work, however you might not receive new updates.");
        msgInfo.descriptionText = message;
        msgInfo.finishFunc = [this](QPointer<MessageDialogResult>)
        {
            preferences->setOneTimeActionDone(Preferences::ONE_TIME_ACTION_OS_TOO_OLD, true);
        };
        MessageDialogOpener::warning(msgInfo);
    }
#endif
}

void MegaApplication::notifyChangeToAllFolders()
{
    for (const auto& localFolder: model->getLocalFolders(SyncInfo::AllHandledSyncTypes))
    {
        ++mProcessingShellNotifications;
        auto stdLocalFolder = Platform::getInstance()->toLocalEncodedPath(localFolder);
        Platform::getInstance()->notifyItemChange(localFolder, megaApi->syncPathState(&stdLocalFolder));
    }
}

int MegaApplication::getPrevVersion()
{
    return prevVersion;
}

void MegaApplication::showNotificationFinishedTransfers(unsigned long long appDataId)
{
    if (mOsNotifications)
    {
        mOsNotifications->sendFinishedTransferNotification(appDataId);
    }
}

#ifdef __APPLE__
void MegaApplication::enableFinderExt()
{
    // We need to wait from OS X El capitan to reload system db before enable the extension
    Platform::getInstance()->enableFileManagerExtension(true);
    preferences->setOneTimeActionDone(Preferences::ONE_TIME_ACTION_ACTIVE_FINDER_EXT, true);
}
#endif

QSystemTrayIcon *MegaApplication::getTrayIcon()
{
    return trayIcon;
}

LoginController *MegaApplication::getLoginController()
{
    return mLoginController;
}

AccountStatusController* MegaApplication::getAccountStatusController()
{
    return mStatusController;
}

void MegaApplication::openFolderPath(QString localPath)
{
    if (!localPath.isEmpty())
    {
        #ifdef WIN32
        if (localPath.startsWith(QString::fromUtf8("\\\\?\\")))
        {
            localPath = localPath.mid(4);
        }
        #endif
        Platform::getInstance()->showInFolder(localPath);
    }
}

void MegaApplication::updateStatesAfterTransferOverQuotaTimeHasExpired()
{
    transferOverQuotaWaitTimeExpiredReceived = true;
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
        infoDialog->updateHeaderBackground();
    }
}

void MegaApplication::onSyncModelUpdated(std::shared_ptr<SyncSettings>)
{
    if(mLoginController->isFetchNodesFinished())
    {
        createAppMenus();
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

void MegaApplication::onTransfersModelUpdate()
{
    if (appfinished)
    {
        return;
    }
    //Send updated statics to the information dialog
    if (infoDialog)
    {
        infoDialog->updateDialogState();
    }

    auto TransfersStats = mTransfersModel->getTransfersCount();
    //If there are no pending transfers or we have the first ones, reset the statics and update the state of the tray icon
    if ((!TransfersStats.pendingDownloads
         && !TransfersStats.pendingUploads) ||
        !mTransferring)
    {
        onGlobalSyncStateChanged(megaApi);
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

std::shared_ptr<MegaNode> MegaApplication::getVaultNode(bool forceReset)
{
    if (forceReset || !mVaultNode)
    {
        mVaultNode.reset(megaApi->getVaultNode());
    }
    return mVaultNode;
}

std::shared_ptr<MegaNode> MegaApplication::getRubbishNode(bool forceReset)
{
    if (forceReset || !mRubbishNode)
    {
        mRubbishNode.reset(megaApi->getRubbishNode());
    }
    return mRubbishNode;
}

void MegaApplication::resetRootNodes()
{
    getRootNode(true);
    getVaultNode(true);
    getRubbishNode(true);
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
    if (!mLoginController->isFetchNodesFinished() || mStatusController->isAccountBlocked()) // if not logged or blocked account
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
            displayedMenu = guestMenu;
        }
    }
    else // Fetching nodes finished and onboading is not shown
    {
        if (infoDialogMenu)
        {
            if (infoDialogMenu->isVisible())
            {
                infoDialogMenu->close();
            }

            menuWidthInitialPopup = infoDialogMenu->sizeHint().width();

            auto cursorPos = QCursor::pos();

            QPoint p = point ? (*point) - QPoint(infoDialogMenu->sizeHint().width(), 0)
                                     : cursorPos;


            infoDialogMenu->update();
            infoDialogMenu->popup(p);
            displayedMenu = infoDialogMenu;


            MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, QString::fromUtf8("Poping up Info Dialog menu: p = %1, cursor = %2, dialog size hint = %3, displayedMenu = %4, menuWidthInitialPopup = %5")
                         .arg(QString::fromUtf8("[%1,%2]").arg(p.x()).arg(p.y()))
                         .arg(QString::fromUtf8("[%1,%2]").arg(cursorPos.x()).arg(cursorPos.y()))
                         .arg(QString::fromUtf8("[%1,%2]").arg(infoDialogMenu->sizeHint().width()).arg(infoDialogMenu->sizeHint().height()))
                         .arg(QString::fromUtf8("[%1,%2,%3,%4]").arg(displayedMenu->rect().x()).arg(displayedMenu->rect().y()).arg(displayedMenu->rect().width()).arg(displayedMenu->rect().height()))
                         .arg(menuWidthInitialPopup)
                         .toUtf8().constData());
        }
    }

    // Menu width might be incorrect the first time it's shown. This works around that and repositions the menu at the expected position afterwards
    if (point && displayedMenu)
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

                MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, QString::fromUtf8("Poping up Info Dialog workaround: p = %1, pointValue = %2, displayedMenu size hint = %3, displayedMenu = %4, menuWidthInitialPopup = %5")
                             .arg(QString::fromUtf8("[%1,%2]").arg(p.x()).arg(p.y()))
                             .arg(QString::fromUtf8("[%1,%2]").arg(pointValue.x()).arg(pointValue.y()))
                             .arg(QString::fromUtf8("[%1,%2]").arg(displayedMenu->sizeHint().width()).arg(displayedMenu->sizeHint().height()))
                             .arg(QString::fromUtf8("[%1,%2,%3,%4]").arg(displayedMenu->rect().x()).arg(displayedMenu->rect().y()).arg(displayedMenu->rect().width()).arg(displayedMenu->rect().height()))
                             .arg(menuWidthInitialPopup)
                             .toUtf8().constData());

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
        if (megaApi) megaApi->setLogExtraForModules(false, false);
    }
    else
    {
        Preferences::HTTPS_ORIGIN_CHECK_ENABLED = false;
        logger->setDebug(true);
        showInfoMessage(tr("DEBUG mode enabled. A log is being created in your desktop (MEGAsync.log)"));
        if (megaApi)
        {
            megaApi->setLogExtraForModules(true, true);

            MegaApi::log(MegaApi::LOG_LEVEL_INFO, QString::fromUtf8("Version string: %1   Version code: %2.%3   User-Agent: %4").arg(Preferences::VERSION_STRING)
                     .arg(Preferences::VERSION_CODE).arg(Preferences::BUILD_ID).arg(QString::fromUtf8(megaApi->getUserAgent())).toUtf8().constData());
        }
    }
}

void MegaApplication::pauseTransfers()
{
    pauseTransfers(!preferences->getGlobalPaused());
}

void MegaApplication::officialWeb()
{
    Utilities::openUrl(ServiceUrls::instance()->getBaseUrl());
}

void MegaApplication::goToMyCloud()
{
    auto node(getRootNode());
    if (node)
    {
        std::unique_ptr<char[]> handle(node->getBase64Handle());
        const auto url = ServiceUrls::instance()->getNodeUrl(QString::fromUtf8(handle.get()));
        Utilities::openUrl(url);

        mStatsEventHandler->sendTrackedEvent(AppStatsEvents::EventType::MENU_CLOUD_DRIVE_CLICKED,
                                             sender(),
                                             MEGAWebAction,
                                             true);
    }
    // If the root node fails, open, at least, the official web
    else
    {
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR, "Opening User cloud failed. Root node invalid.");
        officialWeb();
    }
}

void MegaApplication::goToFiles()
{
    if (infoDialog)
    {
        CloudDriveNodeSelector* nodeSelector = new CloudDriveNodeSelector();
        nodeSelector->init();
        DialogOpener::showGeometryRetainerDialog<NodeSelector>(nodeSelector);

        mStatsEventHandler->sendTrackedEvent(AppStatsEvents::EventType::CLOUD_DRIVE_OPENED,
                                             sender(),
                                             filesAction,
                                             true);
    }
}

void MegaApplication::openDeviceCentre()
{
    if (appfinished)
    {
        return;
    }
    mStatsEventHandler->sendTrackedEvent(AppStatsEvents::EventType::MENU_DEVICE_CENTRE_CLICKED,
                                         sender(),
                                         deviceCentreAction,
                                         true);
#ifdef Q_OS_MACOS
    if (infoDialog)
    {
        infoDialog->hide();
    }
#endif

    QMLComponent::showDialog<DeviceCentre>();
}

void MegaApplication::importLinks()
{
    if (appfinished)
    {
        return;
    }

    mStatsEventHandler->sendTrackedEvent(AppStatsEvents::EventType::MENU_OPEN_LINKS_CLICKED,
                                         sender(), importLinksAction, true);

    mTransferQuota->checkImportLinksAlertDismissed([this](int result){
        if(result == QDialog::Rejected)
        {
            if (QmlDialogManager::instance()->openOnboardingDialog())
            {
                return;
            }

            //Show the dialog to paste public links
            auto pasteMegaLinksDialog = new PasteMegaLinksDialog();
            DialogOpener::showDialog<PasteMegaLinksDialog>(
                pasteMegaLinksDialog,
                this,
                &MegaApplication::onPasteMegaLinksDialogFinish);
        }
    });
}

void MegaApplication::onPasteMegaLinksDialogFinish(QPointer<PasteMegaLinksDialog> pasteMegaLinksDialog)
{
    if (pasteMegaLinksDialog->result() == QDialog::Accepted)
    {
        //Get the list of links from the dialog
        QStringList linkList = pasteMegaLinksDialog->getLinks();

        mLinkProcessor->resetAndSetLinkList(linkList);

        //Open the import dialog
        auto importDialog = new ImportMegaLinksDialog(linkList);

        connect(mLinkProcessor, &LinkProcessor::onLinkInfoAvailable, importDialog, &ImportMegaLinksDialog::onLinkInfoAvailable);
        connect(mLinkProcessor, &LinkProcessor::onLinkInfoRequestFinish, importDialog, &ImportMegaLinksDialog::onLinkInfoRequestFinish);
        connect(importDialog, &ImportMegaLinksDialog::linkSelected, mLinkProcessor, &LinkProcessor::onLinkSelected);
        connect(importDialog, &ImportMegaLinksDialog::onChangeEvent, mLinkProcessor, &LinkProcessor::refreshLinkInfo);

        mLinkProcessor->requestLinkInfo();

        DialogOpener::showDialog<ImportMegaLinksDialog>(
            importDialog,
            [this, importDialog]()
            {
                if (importDialog->result() == QDialog::Accepted)
                {
                    // If the user wants to download some links, do it
                    if (importDialog->shouldDownload())
                    {
                        if (!preferences->hasDefaultDownloadFolder())
                        {
                            preferences->setDownloadFolder(importDialog->getDownloadPath());
                        }

                        mLinkProcessor->downloadLinks(importDialog->getDownloadPath());
                    }

                    // If the user wants to import some links, do it
                    if (preferences->logged() && importDialog->shouldImport())
                    {
                        preferences->setOverStorageDismissExecution(0);

                        connect(mLinkProcessor,
                                &LinkProcessor::onLinkImportFinish,
                                this,
                                [this]() mutable
                                {
                                    preferences->setImportFolder(
                                        mLinkProcessor->getImportParentFolder());
                                });

                        mLinkProcessor->importLinks(importDialog->getImportPath());
                    }
                }
            });
    }
}

void MegaApplication::showChangeLog()
{
    if (appfinished)
    {
        return;
    }

    auto changeLogDialog = new ChangeLogDialog(Preferences::VERSION_STRING, Preferences::SDK_ID, Preferences::CHANGELOG);
    DialogOpener::showDialog<ChangeLogDialog>(changeLogDialog);
}

void MegaApplication::runUploadActionWithTargetHandle(const MegaHandle &targetFolder, QWidget* parent)
{
    if (appfinished)
    {
        return;
    }
    QString  defaultFolderPath = getDefaultUploadPath();

    auto processUpload = [this, defaultFolderPath, targetFolder, parent]()
    {
        SelectorInfo info;
        info.title = QCoreApplication::translate("ShellExtension", "Upload to MEGA");
        info.defaultDir = defaultFolderPath;
        info.multiSelection = true;
        info.parent = parent;
        info.func = [this, targetFolder, parent](QStringList files)
        {
            if(files.size() >= 1)
            {
                std::unique_ptr<MegaNode> folder(getMegaApi()->getNodeByHandle(targetFolder));
                if(folder)
                {
                    uploadQueue.append(createQueue(files));
                    processUploadQueue(targetFolder, parent);
                }
                else
                {
                    NodeSelector::showNotFoundNodeMessageBox();
                }
            }
        };

        Platform::getInstance()->fileAndFolderSelector(info);
    };

    const bool storageIsOverQuota(storageState == MegaApi::STORAGE_STATE_RED || storageState == MegaApi::STORAGE_STATE_PAYWALL);
    if(storageIsOverQuota)
    {
        auto overQuotaDialog = OverQuotaDialog::showDialog(OverQuotaDialogType::STORAGE_UPLOAD);
        if(overQuotaDialog)
        {
            overQuotaDialog->setParent(parent);
            DialogOpener::showDialog<OverQuotaDialog>(overQuotaDialog, [processUpload]()
            {
                processUpload();
            });
        }
    }

    processUpload();
}

void MegaApplication::uploadActionClicked()
{
    if (appfinished)
    {
        return;
    }

    mStatsEventHandler->sendTrackedEvent(AppStatsEvents::EventType::MENU_UPLOAD_CLICKED,
                                         sender(), uploadAction, true);

    const bool storageIsOverQuota(storageState == MegaApi::STORAGE_STATE_RED || storageState == MegaApi::STORAGE_STATE_PAYWALL);
    if(storageIsOverQuota)
    {
        auto overQuotaDialog = OverQuotaDialog::showDialog(OverQuotaDialogType::STORAGE_UPLOAD);
        if(overQuotaDialog)
        {
            DialogOpener::showDialog<OverQuotaDialog, TransferManager>(overQuotaDialog, false, [this](){
                uploadActionFromWindowAfterOverQuotaCheck();
            });

            return;
        }
    }

    uploadActionFromWindowAfterOverQuotaCheck();
}

void MegaApplication::uploadActionFromWindowAfterOverQuotaCheck()
{
    QString  defaultFolderPath = getDefaultUploadPath();

    infoDialog->hide();
    QApplication::processEvents();
    if (appfinished)
    {
        return;
    }

    QWidget* selectorParent(nullptr);

    if(!selectorParent)
    {
        auto TMDialog = DialogOpener::findDialog<TransferManager>();
        if(TMDialog && (TMDialog->getDialog()->isActiveWindow() || !TMDialog->getDialog()->isMinimized()))
        {
            selectorParent = TMDialog->getDialog();
            DialogOpener::closeDialogsByParentClass<TransferManager>();
        }
    }

    SelectorInfo info;
    info.title = QCoreApplication::translate("ShellExtension", "Upload to MEGA");
    info.defaultDir = defaultFolderPath;
    info.multiSelection = true;
    info.parent = selectorParent;
    info.func = [this/*, blocker*/](QStringList files)
    {
        shellUpload(createQueue(files));
    };

    Platform::getInstance()->fileAndFolderSelector(info);
}

QPointer<OverQuotaDialog> MegaApplication::showSyncOverquotaDialog()
{
    QPointer<OverQuotaDialog> dialog(nullptr);

    if(storageState == MegaApi::STORAGE_STATE_RED)
    {
        dialog = OverQuotaDialog::showDialog(OverQuotaDialogType::STORAGE_SYNCS);
    }
    else if(mTransferQuota->isOverQuota())
    {
        dialog = OverQuotaDialog::showDialog(OverQuotaDialogType::BANDWITH_SYNC);
    }

    return dialog;
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

    mStatsEventHandler->sendTrackedEvent(AppStatsEvents::EventType::MENU_DOWNLOAD_CLICKED,
                                         sender(), downloadAction, true);

    mTransferQuota->checkDownloadAlertDismissed([this](int result)
    {
        if(result == QDialog::Rejected)
        {
            auto downloadNodeSelector = new DownloadNodeSelector();
            downloadNodeSelector->init();
            downloadNodeSelector->setSelectedNodeHandle();

            DialogOpener::showDialog<NodeSelector, TransferManager>(downloadNodeSelector, false, [this, downloadNodeSelector]()
            {
                if (downloadNodeSelector->result() == QDialog::Accepted)
                {
                    QList<MegaHandle> selectedMegaFolderHandles = downloadNodeSelector->getMultiSelectionNodeHandle();

                    foreach(auto& selectedMegaFolderHandle, selectedMegaFolderHandles)
                    {
                        MegaNode *selectedNode = megaApi->getNodeByHandle(selectedMegaFolderHandle);
                        if (selectedNode)
                        {
                            downloadQueue.append(
                                WrappedNode(WrappedNode::TransferOrigin::FROM_APP, selectedNode));
                        }
                    }
                    processDownloads();
                }
            });
        }
    });
}

void MegaApplication::downloadACtionClickedWithHandles(const QList<MegaHandle> &handles)
{
    foreach(auto& selectedMegaFolderHandle, handles)
    {
        MegaNode *selectedNode = megaApi->getNodeByHandle(selectedMegaFolderHandle);
        if (selectedNode)
        {
            downloadQueue.append(WrappedNode(WrappedNode::TransferOrigin::FROM_APP, selectedNode));
        }
    }

    processDownloads();
}

void MegaApplication::streamActionClicked()
{
    if (appfinished)
    {
        return;
    }

    mStatsEventHandler->sendTrackedEvent(AppStatsEvents::EventType::MENU_STREAM_CLICKED,
                                         sender(), streamAction, true);

    mTransferQuota->checkStreamingAlertDismissed([this](int result){
        if(result == QDialog::Rejected)
        {
            auto streamSelector = new StreamingFromMegaDialog(megaApi, megaApiFolders);
            connect(mTransferQuota.get(), &TransferQuota::waitTimeIsOver, streamSelector, &StreamingFromMegaDialog::updateStreamingState);
            DialogOpener::showDialog<StreamingFromMegaDialog>(streamSelector);
        }
    });
}

void MegaApplication::transferManagerActionClicked(int tab)
{
    if (appfinished)
    {
        return;
    }

    if(!mTransferManager)
    {
        createTransferManagerDialog();
    }

    DialogOpener::showGeometryRetainerDialog(mTransferManager);
    mTransferManager->toggleTab(tab);
}

void MegaApplication::changeState()
{
    if (appfinished)
    {
        return;
    }
    updateTrayIconMenu();
}

#ifdef _WIN32
void MegaApplication::changeDisplay(QScreen*)
{
    MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, QString::fromUtf8("DISPLAY CHANGED").toUtf8().constData());

    if (infoDialog)
    {
        infoDialog->setWindowFlags(Qt::FramelessWindowHint);
        infoDialog->setWindowFlags(Qt::FramelessWindowHint | Qt::Popup);
    }
    if (mTransferManager && mTransferManager->isVisible())
    {
        //hack to force qt to reconsider zoom/sizes/etc ...
        //this closes the window
        mTransferManager->setWindowFlags(Qt::Window);
        mTransferManager->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
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
            trayIcon->setContextMenu(initialTrayMenu.data() ? initialTrayMenu.data() : &emptyMenu);
        }
#else

        trayIcon->setContextMenu(nullptr); //prevents duplicated context menu in qt 5.12.8 64 bits

        if (preferences && preferences->logged() && getRootNode() &&
            !mStatusController->isAccountBlocked() &&
            AppState::instance()->getAppState() != AppState::FATAL_ERROR)
        { //regular situation: fully logged and without any blocking status
#ifdef _WIN32
            trayIcon->setContextMenu(windowsMenu.data() ? windowsMenu.data() : &emptyMenu);
#else
            trayIcon->setContextMenu(initialTrayMenu.data() ? initialTrayMenu.data() : &emptyMenu);
#endif
        }
        else
        {
            trayIcon->setContextMenu(initialTrayMenu.data() ? initialTrayMenu.data() : &emptyMenu);
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
        connect(trayIcon, &QSystemTrayIcon::activated,
                this, &MegaApplication::trayIconActivated);

    #ifdef __APPLE__
        scanningTimer = new QTimer();
        scanningTimer->setSingleShot(false);
        scanningTimer->setInterval(500);
        scanningAnimationIndex = 1;
        connect(scanningTimer, SIGNAL(timeout()), this, SLOT(scanningAnimationStep()));
    #endif
    }

    updateTrayIconMenu();

    trayIcon->setToolTip(QCoreApplication::applicationName()
                     + QString::fromUtf8(" ")
                     + Preferences::VERSION_STRING
                     + QString::fromUtf8("\n")
                     + tr("Starting"));

#ifndef __APPLE__
    #ifdef _WIN32
        trayIcon->setIcon(QIcon(QString::fromUtf8("://images/tray_sync.ico")));
    #else
        setTrayIconFromTheme(QString::fromUtf8("://images/synching.svg"));
    #endif
#else
    QIcon ic = QIcon(QString::fromUtf8("://images/icon_syncing_mac.png"));
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
    if (appfinished || !megaApi->isLoggedIn())
    {
        return;
    }

    if (!uploadQueue.size())
    {
        return;
    }

    if (mStatusController->isAccountBlocked())
    {
        if (infoDialog)
        {
            raiseInfoDialog();
        }
        return;
    }

    if (QmlDialogManager::instance()->openOnboardingDialog())
    {
        return;
    }

    //If there is a default upload folder in the preferences
    std::shared_ptr<MegaNode> node(megaApi->getNodeByHandle(preferences->uploadFolder()));
    if (node)
    {
        std::unique_ptr<const char[]> path(megaApi->getNodePath(node.get()));
        if (path && !strncmp(path.get(), "//bin", 5))
        {
            preferences->setHasDefaultUploadFolder(false);
            preferences->setUploadFolder(INVALID_HANDLE);
        }

        if (preferences->hasDefaultUploadFolder())
        {
            //use it to upload the list of files
            processUploadQueue(node->getHandle());
            return;
        }
    }

    auto uploadFolderSelector = new UploadToMegaDialog(megaApi);
    uploadFolderSelector->setDefaultFolder(preferences->uploadFolder());

    DialogOpener::showDialog<UploadToMegaDialog, TransferManager>(uploadFolderSelector, false, [this, uploadFolderSelector]()
    {
        if (uploadFolderSelector->result()==QDialog::Accepted)
        {
            //If the dialog is accepted, get the destination node
            MegaHandle nodeHandle = uploadFolderSelector->getSelectedHandle();
            preferences->setHasDefaultUploadFolder(uploadFolderSelector->isDefaultFolder());
            preferences->setUploadFolder(nodeHandle);
            if (mSettingsDialog)
            {
                mSettingsDialog->updateUploadFolder(); //this could be done via observer
            }

            processUploadQueue(nodeHandle);
        }
        //If the dialog is rejected, cancel uploads
        else
        {
            uploadQueue.clear();
        }
    });
}

void MegaApplication::processDownloads()
{
    if (appfinished || !megaApi->isLoggedIn() || !downloadQueue.size())
    {
        return;
    }

    if (mStatusController->isAccountBlocked())
    {
        if (infoDialog)
        {
            raiseInfoDialog();
        }
        return;
    }

    if (QmlDialogManager::instance()->openOnboardingDialog())
    {
        return;
    }

    if (hasDefaultDownloadFolder())
    {
        showInfoDialogIfHTTPServerSender();
        processDownloadQueue(preferences->downloadFolder());
        return;
    }

    auto downloadFolderSelector = new DownloadFromMegaDialog(preferences->downloadFolder());
    DialogOpener::showDialog<DownloadFromMegaDialog, TransferManager>(downloadFolderSelector, false, this, &MegaApplication::onDownloadFromMegaFinished);
}

bool MegaApplication::hasDefaultDownloadFolder() const
{
    bool hasDefaultDownloadFolder = false;

    QString defaultPath = preferences->downloadFolder();
    if (preferences->hasDefaultDownloadFolder() && QDir(defaultPath).exists())
    {
        // QFile always wants `/` as separator
        QString qFilePath = QDir::fromNativeSeparators(defaultPath);
        QTemporaryFile *test = new QTemporaryFile(qFilePath + QDir::separator());
        if (test->open())
        {
            hasDefaultDownloadFolder = true;
        }
        else
        {
            // There is no default download folder
            preferences->setHasDefaultDownloadFolder(false);
            preferences->setDownloadFolder(QString());
        }

        delete test;
    }

    return hasDefaultDownloadFolder;
}

void MegaApplication::showInfoDialogIfHTTPServerSender()
{
    if (qobject_cast<HTTPServer*>(sender()))
    {
        showInfoDialog();
    }
}

void MegaApplication::sendPeriodicStats() const
{
    auto lastTime = preferences->lastDailyStatTime();
    if(Utilities::dayHasChangedSince(lastTime) && !mStatusController->isAccountBlocked())
    {
        QString accountType = QString::number(preferences->logged() ? preferences->accountType() : -1);
        mStatsEventHandler->sendEvent(AppStatsEvents::EventType::DAILY_ACTIVE_USER, { accountType });
        preferences->setLastDailyStatTime(QDateTime::currentDateTime().toMSecsSinceEpoch());

        if(Utilities::monthHasChangedSince(lastTime))
        {
            mStatsEventHandler->sendEvent(AppStatsEvents::EventType::MONTHLY_ACTIVE_USER, { accountType });
        }
    }

    CloudDriveNodeSelector::sendStats();
}

void MegaApplication::createUserMessageController()
{
    if(!mUserMessageController)
    {
        mUserMessageController = std::make_unique<UserMessageController>(nullptr);
        if(mOsNotifications)
        {
            connect(mUserMessageController.get(), &UserMessageController::userAlertsUpdated,
                    mOsNotifications.get(), &DesktopNotifications::onUserAlertsUpdated);
        }
    }
}

void MegaApplication::createGfxProvider(const QString& basePath)
{
    MegaGfxProvider* provider = nullptr;

#if defined(ENABLE_SDK_ISOLATED_GFX)
    auto prefs(Preferences::instance());
    auto endpoint = prefs->getGfxWorkerEndpointInGeneral();
    if (endpoint.isEmpty())
    {
        endpoint = QUuid::createUuid().toString(QUuid::WithoutBraces);
        prefs->setGfxWorkerEndpointInGeneral(endpoint);
    }
    const auto path = QDir::toNativeSeparators(Platform::getInstance()->getGfxProviderPath());
    auto logdirParam = QString::fromUtf8("%1/%2").arg(basePath, LOGS_FOLDER_LEAFNAME_QSTRING);
    logdirParam = QString::fromLatin1("-d=") + QDir::toNativeSeparators(logdirParam);

    std::unique_ptr<MegaStringList> extraParams(MegaStringList::createInstance());
    extraParams->add(logdirParam.toUtf8().constData());

    provider = MegaGfxProvider::createIsolatedInstance(endpoint.toUtf8().constData(),
                                                       path.toUtf8().constData(),
                                                       Preferences::GFXWORKER_KEEPALIVE_S,
                                                       extraParams.get());
#endif

    mGfxProvider.reset(provider ? provider : MegaGfxProvider::createInternalInstance());
}

void MegaApplication::removeSyncsAndBackupsMenus()
{
    if (mSyncs2waysMenu)
    {
        mSyncs2waysMenu->deleteLater();
        mSyncs2waysMenu = nullptr;
    }

    if (mBackupsMenu)
    {
        mBackupsMenu->deleteLater();
        mBackupsMenu = nullptr;
    }
}

void MegaApplication::closeUpsellStorageDialog()
{
    if (auto dialog = DialogOpener::findDialog<QmlDialogWrapper<UpsellComponent>>())
    {
        auto viewMode(dialog->getDialog()->wrapper()->viewMode());
        if (viewMode == UpsellPlans::ViewMode::STORAGE_FULL ||
            viewMode == UpsellPlans::ViewMode::STORAGE_ALMOST_FULL)
        {
            dialog->close();
        }
    }
}

void MegaApplication::showUpsellDialog(UpsellPlans::ViewMode viewMode)
{
    auto dialogInfo(DialogOpener::findDialog<QmlDialogWrapper<UpsellComponent>>());
    if (dialogInfo)
    {
        dialogInfo->getDialog()->wrapper()->setViewMode(viewMode);
        DialogOpener::showDialog(dialogInfo->getDialog());
    }
    else
    {
        dialogInfo = QMLComponent::addDialog<UpsellComponent>(nullptr, viewMode);
        dialogInfo->getDialog()->setShowWhenCreated();
    }
}

void MegaApplication::processSetDownload(const QString& publicLink,
                                         const QList<MegaHandle>& elementHandleList)
{
    if (appfinished || !megaApi->isLoggedIn() || publicLink.isEmpty())
    {
        return;
    }

    if (mStatusController->isAccountBlocked())
    {
        if (infoDialog)
        {
            raiseInfoDialog();
        }
        return;
    }

    if (QmlDialogManager::instance()->openOnboardingDialog())
    {
        return;
    }

    // -----

    if (hasDefaultDownloadFolder())
    {
        showInfoDialogIfHTTPServerSender();

        // Request to download Set
        if (mSetManager)
        {
            mSetManager->requestDownloadSetFromLink(publicLink,
                                                    preferences->downloadFolder(),
                                                    elementHandleList);
        }

        return;
    }

    mLinkToPublicSet = publicLink;
    mElementHandleList = elementHandleList;

    // There is no default download folder; ask user
    auto downloadFolderSelector = new DownloadFromMegaDialog(preferences->downloadFolder());
    DialogOpener::showDialog<DownloadFromMegaDialog, TransferManager>(
        downloadFolderSelector,
        false,
        this,
        &MegaApplication::onDownloadSetFolderDialogFinished);
}

void MegaApplication::onDownloadFromMegaFinished(QPointer<DownloadFromMegaDialog> dialog)
{
    if(dialog)
    {
        if (dialog->result()==QDialog::Accepted)
        {
            //If the dialog is accepted, get the destination node
            QString path = dialog->getPath();
            preferences->setHasDefaultDownloadFolder(dialog->isDefaultDownloadOption());
            preferences->setDownloadFolder(path);
            if (mSettingsDialog)
            {
                mSettingsDialog->updateDownloadFolder(); // this could use observer pattern
            }

            showInfoDialogIfHTTPServerSender();
            processDownloadQueue(path);
        }
        else
        {
            QQueue<WrappedNode>::iterator it;
            for (it = downloadQueue.begin(); it != downloadQueue.end(); ++it)
            {
                HTTPServer::onTransferDataUpdate((*it).getMegaNode()->getHandle(),
                                                 MegaTransfer::STATE_CANCELLED,
                                                 0,
                                                 0,
                                                 0,
                                                 QString());
            }

            // If the dialog is rejected, cancel uploads
            downloadQueue.clear();
        }
    }
}

void MegaApplication::onDownloadSetFolderDialogFinished(QPointer<DownloadFromMegaDialog> dialog)
{
    if (!dialog) { return; }

    if (dialog->result()==QDialog::Accepted)
    {
        // If the dialog is accepted, get the destination node (download folder)
        QString path = dialog->getPath();
        preferences->setHasDefaultDownloadFolder(dialog->isDefaultDownloadOption());
        preferences->setDownloadFolder(path);
        if (mSettingsDialog)
        {
            mSettingsDialog->updateDownloadFolder(); // this could use observer pattern
        }

        showInfoDialogIfHTTPServerSender();

        // Request to download Set
        if (mSetManager)
        {
            mSetManager->requestDownloadSetFromLink(mLinkToPublicSet,
                                                    preferences->downloadFolder(),
                                                    mElementHandleList);
        }
    }
    else
    {
        // Reset
        mLinkToPublicSet = QString::fromUtf8("");
        mElementHandleList.clear();
    }
}

void MegaApplication::logoutActionClicked()
{
    if (appfinished)
    {
        return;
    }

    unlink();
}

//Called when the user wants to upload a list of files and/or folders from the shell
void MegaApplication::shellUpload(QQueue<QString> newUploadQueue)
{
    if (appfinished)
    {
        return;
    }

    //Append the list of files to the upload queue, but avoid duplicates
    std::for_each(newUploadQueue.begin(), newUploadQueue.end(), [&](const QString &str) {
        if (!uploadQueue.contains(str)) {
            uploadQueue.enqueue(str);
        }
    });
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

void MegaApplication::shellViewOnMega(const QString& path, bool versions)
{
    auto tmpPath = Platform::getInstance()->toLocalEncodedPath(path);

    std::unique_ptr<MegaNode> node(megaApi->getSyncedNode(&tmpPath));
    if (node)
    {
        shellViewOnMegaByHandle(node->getHandle(), versions);
    }
}

void MegaApplication::shellViewOnMegaByHandle(MegaHandle handle, bool versions)
{
    std::unique_ptr<const char[]> nodeHandle(MegaApi::handleToBase64(handle));
    Utilities::openUrl(
        ServiceUrls::instance()->getNodeUrl(QString::fromUtf8(nodeHandle.get()), versions));
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

void MegaApplication::externalDownload(QQueue<WrappedNode> newDownloadQueue)
{
    if (appfinished)
    {
        return;
    }

    downloadQueue.append(newDownloadQueue);
}

void MegaApplication::uploadFilesToNode(const QList<QUrl>& files,
                                        MegaHandle targetNode,
                                        QWidget* caller)
{
    if (appfinished)
    {
        return;
    }

    //Append the list of files to the upload queue, but avoid duplicates
    std::for_each(files.begin(), files.end(), [&](const QUrl &file) {
        if (!uploadQueue.contains(file.toLocalFile()))
        {
            auto item(file.toLocalFile());
            if(item.endsWith(QDir::separator()))
            {
                item = item.left(item.lastIndexOf(QDir::separator()));
            }
            uploadQueue.enqueue(item);
        }
    });
    processUploadQueue(targetNode, caller);
}

void MegaApplication::externalLinkDownload(QString megaLink, QString auth)
{
    if (appfinished)
    {
        return;
    }

    pendingLinks.insert(megaLink, auth);

    if (!QmlDialogManager::instance()->openOnboardingDialog())
    {
        megaApi->getPublicNode(megaLink.toUtf8().constData());
    }
}

void MegaApplication::externalFileUpload(MegaHandle targetFolder)
{
    if (appfinished || QmlDialogManager::instance()->openOnboardingDialog())
    {
        return;
    }

    fileUploadTarget = targetFolder;
    auto processUpload = [this](QStringList selectedFiles){
        if (!selectedFiles.isEmpty())
        {
            uploadQueue.append(createQueue(selectedFiles));
            processUploadQueue(fileUploadTarget);

            HTTPServer::onUploadSelectionAccepted(selectedFiles.size(), 0);
        }
        else
        {
            HTTPServer::onUploadSelectionDiscarded();
        }
    };

    QString  defaultFolderPath = getDefaultUploadPath();

    QWidget* parent(nullptr);

#ifdef Q_OS_WIN
    parent = infoDialog;
#endif
    SelectorInfo info;
    info.title = QCoreApplication::translate("ShellExtension", "Upload to MEGA");
    info.defaultDir = defaultFolderPath;
    info.multiSelection = true;
    info.parent = parent;
    info.func = processUpload;
    Platform::getInstance()->fileSelector(info);
}

void MegaApplication::externalFolderUpload(MegaHandle targetFolder)
{
    if (appfinished || QmlDialogManager::instance()->openOnboardingDialog())
    {
        return;
    }

    folderUploadTarget = targetFolder;

    auto processUpload = [this](const QStringList& foldersSelected){
        if (!foldersSelected.isEmpty())
        {
            QFuture<NodeCount> future;
            QFutureWatcher<NodeCount>* watcher(new QFutureWatcher<NodeCount>());

            connect(watcher,
                    &QFutureWatcher<NodeCount>::finished,
                    this,
                    [this, foldersSelected, watcher]()
                    {
                        const NodeCount nodeCount = watcher->result();
                        processUploads(foldersSelected);
                        HTTPServer::onUploadSelectionAccepted(nodeCount.files, nodeCount.folders);
                        watcher->deleteLater();
                    });

            future = QtConcurrent::run(countFilesAndFolders, foldersSelected);
            watcher->setFuture(future);
        }
        else
        {
            HTTPServer::onUploadSelectionDiscarded();
        }
    };

    QString  defaultFolderPath = getDefaultUploadPath();

    QWidget* parent(nullptr);

#ifdef Q_OS_WIN
    parent = infoDialog;
#endif

    SelectorInfo info;
    info.title = QCoreApplication::translate("ShellExtension", "Upload to MEGA");
    info.defaultDir = defaultFolderPath;
    info.multiSelection = false;
    info.parent = parent;
    info.func = processUpload;
    Platform::getInstance()->folderSelector(info);
}

void MegaApplication::externalFolderSync(MegaHandle targetFolder)
{
    if (appfinished || QmlDialogManager::instance()->openOnboardingDialog())
    {
        return;
    }

    if (infoDialog)
    {
        infoDialog->addSync(SyncInfo::SyncOrigin::EXTERNAL_ORIGIN, targetFolder);
    }
}

void MegaApplication::externalAddBackup()
{
    if (appfinished || QmlDialogManager::instance()->openOnboardingDialog())
    {
        return;
    }

    if(infoDialog)
    {
        infoDialog->addBackup();
    }
}

void MegaApplication::externalOpenTransferManager(int tab)
{
    if (appfinished || !infoDialog || QmlDialogManager::instance()->openOnboardingDialog())
    {
        return;
    }

    transferManagerActionClicked(tab);
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
    QString linkForClipboard(links.join(QLatin1Char('\n')));
    QApplication::clipboard()->setText(linkForClipboard);

    QString message;
    if (links.size() == 1)
    {
        message = tr("The link has been copied to the clipboard");
    }
    else
    {
        message = tr("The links have been copied to the clipboard");
    }

    if (!preferences->getDontShowExportLinkDialog() &&
        (!mOsNotifications ||
         (mOsNotifications &&
          !preferences->isNotificationEnabled(Preferences::NotificationsTypes::INFO_MESSAGES))))
    {
        MessageDialogInfo msgInfo;
        msgInfo.descriptionText = message;
        msgInfo.checkboxText = tr("Don’t show me again");
        msgInfo.finishFunc = [this](QPointer<MessageDialogResult> msgResult)
        {
            if (msgResult->result() == QMessageBox::StandardButton::Ok)
            {
                preferences->setDontShowExportLinkDialog(msgResult->isChecked());
            }
        };

        MessageDialogOpener::success(msgInfo);
    }
    else
    {
        DesktopNotifications::NotificationInfo info;
        info.title = MegaSyncApp->getMEGAString();
        info.message = message;

        showInfoMessage(info);
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

    if (mSettingsDialog)
    {
        mSettingsDialog->setUpdateAvailable(true);
    }

    if (requested)
    {
#ifdef WIN32
        showInfoMessage(tr("A new version of MEGAsync is available. Click on this message to install it"));
#else
        showInfoMessage(tr("A new version of MEGAsync is available"));
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
            // URL handled through translations. TODO use placeholder
            auto msg = tr("There was a problem installing the update. Please try again later or "
                          "download the last version from:\nhttps://mega.co.nz/#sync");
            showInfoMessage(msg);
        }
    }
}

void MegaApplication::onUpdateError()
{
    if (appfinished)
    {
        return;
    }
    // URL handled through translations. TODO use placeholder
    auto msg = tr("There was a problem installing the update. Please try again later or download "
                  "the last version from:\nhttps://mega.co.nz/#sync");
    showInfoMessage(msg);
}

//Called when users click in the tray icon
void MegaApplication::trayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (appfinished)
    {
        return;
    }

    registerUserActivity();

    if (AppState::instance()->getAppState() == AppState::FATAL_ERROR)
    {
        if (reason == QSystemTrayIcon::Trigger)
        {
            QmlDialogManager::instance()->raiseGuestDialog();
        }
        return;
    }

#ifdef Q_OS_LINUX
    const QString desktopEnv = qEnvironmentVariable("XDG_CURRENT_DESKTOP");
    if (!desktopEnv.isEmpty() && (
            desktopEnv == QString::fromUtf8("ubuntu:GNOME") ||
            desktopEnv == QString::fromUtf8("LXDE")))
    {
        QString msg =
            QString::fromUtf8("Ignoring unexpected trayIconActivated detected in ") + desktopEnv;
        MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, msg.toUtf8().constData());
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

    megaApi->retryPendingConnections();

    if (reason == QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::Context)
    {
        if (!infoDialog)
        {
            if (reason == QSystemTrayIcon::Trigger)
            {
                mStatsEventHandler->sendTrackedEvent(
                    AppStatsEvents::EventType::TRAY_ICON_TRIGGERED);
                if (mStatusController->isAccountBlocked())
                {
                    createInfoDialog();
                    checkSystemTray();
                    createTrayIcon();
                    showInfoDialog();
                }
                else
                {
                    QmlDialogManager::instance()->openGuestDialog();
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

#ifdef Q_OS_LINUX
        if (infoDialogMenu && infoDialogMenu->isVisible())
        {
            infoDialogMenu->close();
        }
#endif

        DialogOpener::raiseAllDialogs();
        QmlDialogManager::instance()->raiseOnboardingDialog();
        QmlDialogManager::instance()->raiseOrHideInfoGuestDialog(infoDialogTimer, 200);
    }
#ifdef Q_OS_WINDOWS
    else if (reason == QSystemTrayIcon::DoubleClick)
    {
        openFirstActiveSync();
    }
#endif
#ifndef __APPLE__
    else if (reason == QSystemTrayIcon::MiddleClick)
    {
        showTrayMenu();
    }
#endif
}

void MegaApplication::openFirstActiveSync()
{
    if (appfinished)
    {
        return;
    }

    auto* syncInfo{SyncInfo::instance()};
    if (syncInfo != nullptr)
    {
        const auto syncsSettings(syncInfo->getAllSyncSettings());
        auto firstActiveSyncSettings(std::find_if(syncsSettings.cbegin(),
                                                  syncsSettings.cend(),
                                                  [](auto syncSettings)
                                                  {
                                                      return syncSettings->isActive();
                                                  }));

        if (firstActiveSyncSettings != syncsSettings.cend())
        {
            infoDialogTimer->stop();

            if (infoDialog)
            {
                infoDialog->hide();
            }

            QString localFolderPath = (*firstActiveSyncSettings)->getLocalFolder();
            if (!localFolderPath.isEmpty())
            {
                Utilities::openUrl(QUrl::fromLocalFile(localFolderPath));
            }
        }
    }
}

void MegaApplication::onMessageClicked()
{
    if (appfinished)
    {
        return;
    }

    if (lastTrayMessage == tr("A new version of MEGAsync is available. Click on this message to install it"))
    {
        triggerInstallUpdate();
    }
    else if (lastTrayMessage == tr("MEGAsync is now running. Click here to open the status window."))
    {
        trayIconActivated(QSystemTrayIcon::Trigger);
    }
}

void MegaApplication::openSettings(int tab)
{
    if (appfinished)
    {
        return;
    }

    mStatsEventHandler->sendTrackedEvent(AppStatsEvents::EventType::MENU_SETTINGS_CLICKED,
                                         sender(), settingsAction, true);

    bool proxyOnly = AppState::instance()->getAppState() != AppState::NOMINAL;

    if (megaApi)
    {
        proxyOnly |=
            !mLoginController->isFetchNodesFinished() || mStatusController->isAccountBlocked();
        megaApi->retryPendingConnections();
    }

#ifndef __MACH__
    if (preferences && !proxyOnly)
    {
        AccountDetailsManager::instance()->updateUserStats(AccountDetailsManager::Flag::ALL,
                                                           true,
                                                           USERSTATS_OPENSETTINGSDIALOG);
    }
#endif

    if (mSettingsDialog)
    {
        if (proxyOnly)
        {
            mSettingsDialog->showGuestMode();
        }

        mSettingsDialog->setProxyOnly(proxyOnly);
        mSettingsDialog->openSettingsTab(tab);

        DialogOpener::showDialog(mSettingsDialog);
    }
    else
    {
        //Show a new settings dialog
        mSettingsDialog = new SettingsDialog(this, proxyOnly);
        mSettingsDialog->setUpdateAvailable(updateAvailable);
        connect(mSettingsDialog.data(), &SettingsDialog::userActivity, this, &MegaApplication::registerUserActivity);
        DialogOpener::showDialog(mSettingsDialog);
        if (proxyOnly)
        {
            mSettingsDialog->showGuestMode();
        }
        else
        {
            mSettingsDialog->openSettingsTab(tab);
        }
    }
}

void MegaApplication::openSettingsAddSync(MegaHandle megaFolderHandle)
{
    if (appfinished)
    {
        return;
    }

    if (auto dialog = DialogOpener::findDialog<QmlDialogWrapper<Onboarding>>())
    {
        // The onboarding is shown and the remote folder is set
        // (sync button notification, incoming share with full access)
        DialogOpener::showDialog(dialog->getDialog());
    }
    else
    {
        openSettings(SettingsDialog::SYNCS_TAB);
        if (megaFolderHandle == ::mega::INVALID_HANDLE)
        {
            MegaApi::log(MegaApi::LOG_LEVEL_ERROR,
                         QString::fromUtf8("Invalid Mega handle when trying to add sync")
                             .toUtf8()
                             .constData());
        }
        else
        {
            CreateRemoveSyncsManager::addSync(SyncInfo::SyncOrigin::MAIN_APP_ORIGIN,
                                              megaFolderHandle);
        }
    }
}

void MegaApplication::createAppMenus()
{
    if (appfinished)
    {
        return;
    }

    createTrayIconMenus();

    if (preferences->logged())
    {
        createInfoDialogMenus();
    }

    updateTrayIconMenu();
}

// Create menus for the tray icon.
void MegaApplication::createTrayIconMenus()
{
    lastHovered = nullptr;

    // First, create the initial Menu, shown while not connected

    // Clear menu if it exists
    if (initialTrayMenu)
    {
        const bool deleteActions(true);
        clearMenu(initialTrayMenu, deleteActions);
    }
#ifndef _WIN32 // win32 needs to recreate menu to fix scaling qt issue
    else
#endif
    {
        deleteMenu(initialTrayMenu);
        initialTrayMenu = new QMenu();
        Platform::getInstance()->initMenu(initialTrayMenu, "TrayMenu", false);
    }

#ifdef USE_BREAKPAD
    if (!preferences->isCrashed())
#endif
    {
        guestSettingsAction = new QAction(tr("Settings"), this);

#ifdef __APPLE__
        guestSettingsAction->setIcon(
            QIcon(QString::fromUtf8(":/images/icons/tray/macos/settings.svg")));
#endif

#ifdef Q_OS_LINUX
        guestSettingsAction->setIcon(
            QIcon(QString::fromUtf8(":/images/icons/tray/linux/settings.svg")));
#endif

#ifdef _WIN32
        guestSettingsAction->setIcon(
            QIcon(QString::fromUtf8(":/images/icons/tray/windows/settings.svg")));
#endif

        // When triggered, open "Settings" window. As the user is not logged in, it
        // will only show proxy settings.
        connect(guestSettingsAction, &QAction::triggered, this, &MegaApplication::openSettings);
    }

    initialExitAction = new QAction(PlatformStrings::exit(), this);

#ifdef __APPLE__
    initialExitAction->setIcon(QIcon(QString::fromUtf8(":/images/icons/tray/macos/exit.svg")));
#endif

#ifdef Q_OS_LINUX
    initialExitAction->setIcon(QIcon(QString::fromUtf8(":/images/icons/tray/linux/exit.svg")));
#endif

#ifdef _WIN32
    initialExitAction->setIcon(QIcon(QString::fromUtf8(":/images/icons/tray/windows/exit.svg")));
#endif
    connect(initialExitAction, &QAction::triggered, this, &MegaApplication::tryExitApplication);

    initialTrayMenu->addAction(guestSettingsAction);
    initialTrayMenu->addAction(initialExitAction);

#ifdef Q_OS_LINUX
    showStatusAction = new QAction(tr("Show status"), this);
    showStatusAction->setIcon(QIcon(QString::fromUtf8(":/images/icons/tray/linux/status.svg")));
    connect(showStatusAction,
            &QAction::triggered,
            this,
            [this]()
            {
                DialogOpener::raiseAllDialogs();
                QmlDialogManager::instance()->raiseOnboardingDialog();

                if (infoDialog)
                {
                    infoDialogTimer->start(200);
                }

                if (Preferences::instance()->logged())
                {
                    QmlDialogManager::instance()->raiseGuestDialog();
                }
            });

    initialTrayMenu->insertAction(guestSettingsAction, showStatusAction);
#endif

#ifdef Q_OS_LINUX
    onOperatingSystemThemeChanged(Platform::getInstance()->getCurrentThemeAppearance());
#endif
}

void MegaApplication::createInfoDialogMenus()
{
    if (!infoDialog)
    {
        return;
    }

#ifdef _WIN32
    if (!windowsMenu)
    {
        deleteMenu(windowsMenu);
        windowsMenu = new QMenu();
        Platform::getInstance()->initMenu(windowsMenu, "WindowsMenu", false);
    }
    else
    {
        clearMenu(windowsMenu);
    }

    recreateAction(&windowsExitAction,
                   windowsMenu,
                   PlatformStrings::exit(),
                   &MegaApplication::tryExitApplication,
                   QString::fromLatin1(":/images/icons/tray/windows/exit.svg"));
    recreateAction(&windowsSettingsAction,
                   windowsMenu,
                   tr("Settings"),
                   &MegaApplication::openSettings,
                   QString::fromLatin1(":/images/icons/tray/windows/settings.svg"));
    recreateAction(&windowsImportLinksAction,
                   windowsMenu,
                   tr("Open links"),
                   &MegaApplication::importLinks,
                   QString::fromLatin1(":/images/icons/tray/windows/open_links.svg"));
    recreateAction(&windowsFilesAction,
                   windowsMenu,
                   tr("Files"),
                   &MegaApplication::goToFiles,
                   QString::fromLatin1(":/images/icons/tray/windows/drive.svg"));
    recreateAction(&windowsUploadAction,
                   windowsMenu,
                   tr("Upload"),
                   &MegaApplication::uploadActionClicked,
                   QString::fromLatin1(":/images/icons/tray/windows/upload.svg"));
    recreateAction(&windowsDownloadAction,
                   windowsMenu,
                   tr("Download"),
                   &MegaApplication::downloadActionClicked,
                   QString::fromLatin1(":/images/icons/tray/windows/download.svg"));
    recreateAction(&windowsStreamAction,
                   windowsMenu,
                   tr("Stream"),
                   &MegaApplication::streamActionClicked,
                   QString::fromLatin1(":/images/icons/tray/windows/stream.svg"));
    recreateAction(&windowsTransferManagerAction,
                   windowsMenu,
                   tr("Transfer manager"),
                   &MegaApplication::transferManagerActionClicked,
                   QString::fromLatin1(":/images/icons/tray/windows/transfer_manager.svg"));

    bool windowsUpdateActionEnabled = true;
    if (windowsUpdateAction)
    {
        windowsUpdateActionEnabled = windowsUpdateAction->isEnabled();
        windowsUpdateAction->deleteLater();
        windowsUpdateAction = nullptr;
    }

    if(windowsAboutAction)
    {
        windowsAboutAction->deleteLater();
        windowsAboutAction = nullptr;
    }

    if (updateAvailable)
    {
        windowsUpdateAction = new QAction(tr("Install update"), this);
        windowsUpdateAction->setEnabled(windowsUpdateActionEnabled);

        windowsMenu->addAction(windowsUpdateAction);

        connect(windowsUpdateAction, &QAction::triggered, this, &MegaApplication::onInstallUpdateClicked);
    }
    else
    {
        windowsAboutAction = new QAction(tr("About"), this);
        windowsAboutAction->setIcon(
            QIcon(QString::fromUtf8(":/images/icons/tray/windows/about.svg")));

        windowsMenu->addAction(windowsAboutAction);
        connect(windowsAboutAction, &QAction::triggered, this, &MegaApplication::onAboutClicked);
    }

    windowsMenu->addSeparator();
    windowsMenu->addAction(windowsFilesAction);
    windowsMenu->addAction(windowsImportLinksAction);
    windowsMenu->addAction(windowsUploadAction);
    windowsMenu->addAction(windowsDownloadAction);
    windowsMenu->addAction(windowsStreamAction);
    windowsMenu->addAction(windowsTransferManagerAction);
    windowsMenu->addAction(windowsSettingsAction);
    windowsMenu->addSeparator();
    windowsMenu->addAction(windowsExitAction);

#endif

    // Info Dialog overflow menu
    if (infoDialogMenu)
    {
        clearMenu(infoDialogMenu);
    }
#ifndef _WIN32 // win32 needs to recreate menu to fix scaling qt issue
    else
#endif
    {
        deleteMenu(infoDialogMenu);
        delete mBackupsMenu;
        delete mSyncs2waysMenu;
        infoDialogMenu = new QMenu(infoDialog);
    }

    recreateMegaMenuAction(&exitAction,
                           infoDialogMenu,
                           PlatformStrings::exit(),
                           Utilities::getPixmapName(QLatin1String("log-out-01"),
                                                    Utilities::AttributeType::SMALL |
                                                        Utilities::AttributeType::THIN |
                                                        Utilities::AttributeType::OUTLINE,
                                                    false)
                               .toStdString()
                               .c_str(),
                           &MegaApplication::tryExitApplication);

    recreateMegaMenuAction(&settingsAction,
                           infoDialogMenu,
                           tr("Settings"),
                           Utilities::getPixmapName(QLatin1String("gear_six"),
                                                    Utilities::AttributeType::SMALL |
                                                        Utilities::AttributeType::THIN |
                                                        Utilities::AttributeType::OUTLINE,
                                                    false)
                               .toStdString()
                               .c_str(),
                           &MegaApplication::openSettings);

    connect(settingsAction,
            &QAction::triggered,
            this,
            &MegaApplication::openSettings,
            Qt::QueuedConnection);

    recreateMegaMenuAction(&MEGAWebAction,
                           infoDialogMenu,
                           tr("MEGA web"),
                           Utilities::getPixmapName(QLatin1String("globe-01"),
                                                    Utilities::AttributeType::SMALL |
                                                        Utilities::AttributeType::THIN |
                                                        Utilities::AttributeType::OUTLINE,
                                                    false)
                               .toStdString()
                               .c_str(),
                           &MegaApplication::goToMyCloud);

    recreateMegaMenuAction(&filesAction,
                           infoDialogMenu,
                           tr("Files"),
                           Utilities::getPixmapName(QLatin1String("folder-open"),
                                                    Utilities::AttributeType::SMALL |
                                                        Utilities::AttributeType::THIN |
                                                        Utilities::AttributeType::OUTLINE,
                                                    false)
                               .toStdString()
                               .c_str(),
                           &MegaApplication::goToFiles);

    // recreateMenuAction(&deviceCentreAction,
    //                    infoDialogMenu,
    //                    tr("Device Centre"),
    //                    "://images/ico-device-centre.svg",
    //                    &MegaApplication::openDeviceCentre);

    if (!mSyncs2waysMenu)
    {
        mSyncs2waysMenu = SyncsMenu::newSyncsMenu(MegaSync::TYPE_TWOWAY, infoDialogMenu);
        connect(mSyncs2waysMenu.data(),
                &SyncsMenu::addSync,
                infoDialog.data(),
                &InfoDialog::onAddSync);
    }

    if (!mBackupsMenu)
    {
        mBackupsMenu = SyncsMenu::newSyncsMenu(MegaSync::TYPE_BACKUP, infoDialogMenu);
        connect(mBackupsMenu.data(),
                &SyncsMenu::addSync,
                infoDialog.data(),
                &InfoDialog::onAddSync);
    }

    recreateMegaMenuAction(&importLinksAction,
                           infoDialogMenu,
                           tr("Open links"),
                           Utilities::getPixmapName(QLatin1String("link-01"),
                                                    Utilities::AttributeType::SMALL |
                                                        Utilities::AttributeType::THIN |
                                                        Utilities::AttributeType::OUTLINE,
                                                    false)
                               .toStdString()
                               .c_str(),
                           &MegaApplication::importLinks);
    recreateMegaMenuAction(&uploadAction,
                           infoDialogMenu,
                           tr("Upload"),
                           Utilities::getPixmapName(QLatin1String("arrow-up-circle"),
                                                    Utilities::AttributeType::SMALL |
                                                        Utilities::AttributeType::THIN |
                                                        Utilities::AttributeType::OUTLINE,
                                                    false)
                               .toStdString()
                               .c_str(),
                           &MegaApplication::uploadActionClicked);
    recreateMegaMenuAction(&downloadAction,
                           infoDialogMenu,
                           tr("Download"),
                           Utilities::getPixmapName(QLatin1String("arrow-down-circle"),
                                                    Utilities::AttributeType::SMALL |
                                                        Utilities::AttributeType::THIN |
                                                        Utilities::AttributeType::OUTLINE,
                                                    false)
                               .toStdString()
                               .c_str(),
                           &MegaApplication::downloadActionClicked);
    recreateMegaMenuAction(&streamAction,
                           infoDialogMenu,
                           tr("Stream"),
                           Utilities::getPixmapName(QLatin1String("stream"),
                                                    Utilities::AttributeType::SMALL |
                                                        Utilities::AttributeType::THIN |
                                                        Utilities::AttributeType::OUTLINE,
                                                    false)
                               .toStdString()
                               .c_str(),
                           &MegaApplication::streamActionClicked);

    if (updateAction)
    {
        updateAction->deleteLater();
        updateAction = nullptr;
    }

    if(aboutAction)
    {
        aboutAction->deleteLater();
        aboutAction = nullptr;
    }

    if (updateAvailable)
    {
        updateAction =
            new MegaMenuItemAction(tr("Install update"),
                                   Utilities::getPixmapName(QLatin1String("MEGA"),
                                                            Utilities::AttributeType::SMALL |
                                                                Utilities::AttributeType::THIN |
                                                                Utilities::AttributeType::OUTLINE,
                                                            false),
                                   0);
        connect(updateAction,
                &QAction::triggered,
                this,
                &MegaApplication::onInstallUpdateClicked,
                Qt::QueuedConnection);
        infoDialogMenu->addAction(updateAction);
    }
    else
    {
        aboutAction =
            new MegaMenuItemAction(tr("About"),
                                   Utilities::getPixmapName(QLatin1String("MEGA"),
                                                            Utilities::AttributeType::SMALL |
                                                                Utilities::AttributeType::THIN |
                                                                Utilities::AttributeType::OUTLINE,
                                                            false),
                                   0);
        connect(aboutAction,
                &QAction::triggered,
                this,
                &MegaApplication::onAboutClicked,
                Qt::QueuedConnection);

        infoDialogMenu->addAction(aboutAction);
    }

    infoDialogMenu->addAction(MEGAWebAction);
    infoDialogMenu->addAction(filesAction);
    infoDialogMenu->addSeparator();
    if (mSyncs2waysMenu)
        infoDialogMenu->addAction(mSyncs2waysMenu->getAction());
    if (mBackupsMenu)
        infoDialogMenu->addAction(mBackupsMenu->getAction());
    infoDialogMenu->addAction(importLinksAction);
    infoDialogMenu->addAction(uploadAction);
    infoDialogMenu->addAction(downloadAction);
    infoDialogMenu->addAction(streamAction);

    infoDialogMenu->setProperty("class", QLatin1String("MegaMenu"));
    infoDialogMenu->setProperty("icon-token", QLatin1String("icon-primary"));

    infoDialogMenu->addAction(settingsAction);
    infoDialogMenu->addSeparator();
    infoDialogMenu->addAction(exitAction);
}

void MegaApplication::createGuestMenu()
{
    if (appfinished)
    {
        return;
    }

    if (guestMenu)
    {
        const bool deleteActions(true);
        clearMenu(guestMenu, deleteActions);
    }
#ifndef _WIN32 // win32 needs to recreate menu to fix scaling qt issue
    else
#endif
    {
        deleteMenu(guestMenu);
        guestMenu = new QMenu();
        Platform::getInstance()->initMenu(guestMenu, "GuestMenu");
    }

    exitActionGuest = new MenuItemAction(PlatformStrings::exit(), QLatin1String("://images/ico_quit.png"), guestMenu);

    connect(exitActionGuest, &QAction::triggered, this, &MegaApplication::tryExitApplication);

    if (updateAvailable)
    {
        updateActionGuest = new MenuItemAction(tr("Install update"), QLatin1String("://images/ico_about_MEGA.png"), guestMenu);
        connect(updateActionGuest, &QAction::triggered, this, &MegaApplication::onInstallUpdateClicked);
    }
    else
    {
        updateActionGuest = new MenuItemAction(tr("About"), QLatin1String("://images/ico_about_MEGA.png"), guestMenu);
        connect(updateActionGuest, &QAction::triggered, this, &MegaApplication::onAboutClicked);
    }

    settingsActionGuest = new MenuItemAction(tr("Settings"), QLatin1String("://images/ico_preferences.png"), guestMenu);

    connect(settingsActionGuest, &QAction::triggered, this, &MegaApplication::openSettings);

    guestMenu->addAction(updateActionGuest);
    guestMenu->addSeparator();
    guestMenu->addAction(settingsActionGuest);
    guestMenu->addSeparator();
    guestMenu->addAction(exitActionGuest);
}

void MegaApplication::refreshStorageUIs()
{
    if (infoDialog)
    {
        infoDialog->setUsage();
    }

    // Ideally this should be the only call here
    AccountDetailsManager::instance()->notifyStorageObservers();
}

void MegaApplication::manageBusinessStatus(int64_t event)
{
    switch (event)
    {
        case MegaApi::BUSINESS_STATUS_GRACE_PERIOD:
        {
            if (megaApi->isMasterBusinessAccount())
            {
                const QString message = tr("This month's payment has failed. Please resolve your payment issue as soon as possible "
                                           "to avoid any suspension of your business account.");
                GuiUtilities::showPayNowOrDismiss(tr("Payment Failed"), message);
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
            if (megaApi->isProFlexiAccount())
            {
                const QString title = tr("Pro Flexi Account deactivated");
                const QString message = CommonMessages::getExpiredProFlexiMessage();
                GuiUtilities::showPayReactivateOrDismiss(title, message);
            }
            else if (megaApi->isMasterBusinessAccount())
            {
                const QString title = tr("Account deactivated");
                const QString message = tr("It seems the payment for your business account has failed. "
                                           "Your account is suspended as read only until you proceed with the needed payments.");
                GuiUtilities::showPayNowOrDismiss(title, message);
            }
            else
            {
                MessageDialogInfo msgInfo;
                msgInfo.titleText = tr("Account Suspended");
                msgInfo.textFormat = Qt::RichText;
                msgInfo.descriptionText =
                    tr("Your account is currently [A]suspended[/A]. You can only browse your data.")
                        .replace(QString::fromUtf8("[A]"),
                                 QString::fromUtf8(
                                     "<span style=\"font-weight: bold; text-decoration:none;\">"))
                        .replace(QString::fromUtf8("[/A]"), QString::fromUtf8("</span>")) +
                    QString::fromUtf8("<br>") + QString::fromUtf8("<br>") +
                    tr("[A]Important:[/A] Contact your business account administrator to resolve "
                       "the issue and activate your account.")
                        .replace(QString::fromUtf8("[A]"),
                                 QString::fromUtf8("<span style=\"font-weight: bold; "
                                                   "color:#DF4843; text-decoration:none;\">"))
                        .replace(QString::fromUtf8("[/A]"), QString::fromUtf8("</span>")) +
                    QString::fromUtf8("\n");
                MessageDialogOpener::warning(msgInfo);
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

    businessStatus = static_cast<int>(event);
    if (preferences->logged())
    {
        preferences->setBusinessState(businessStatus);
    }
}

void MegaApplication::onEvent(MegaApi*, MegaEvent* event)
{
    if (event->getType() == MegaEvent::EVENT_CHANGE_TO_HTTPS)
    {
        preferences->setUseHttpsOnly(true);
    }
    else if (event->getType() == MegaEvent::EVENT_NODES_CURRENT)
    {
        nodescurrent = true;
    }
    else if (event->getType() == MegaEvent::EVENT_STORAGE)
    {
        if (mLoginController->isFetchNodesFinished())
        {
            const auto storageState = static_cast<int>(event->getNumber());
            applyStorageState(storageState);
        }
    }
    else if (event->getType() == MegaEvent::EVENT_STORAGE_SUM_CHANGED)
    {
        receivedStorageSum = event->getNumber();
        if (!preferences->logged())
        {
            return;
        }

        updateUsedStorage();
        refreshStorageUIs();
    }
    else if (event->getType() == MegaEvent::EVENT_BUSINESS_STATUS)
    {
        manageBusinessStatus(event->getNumber());
    }
    else if (event->getType() == MegaEvent::EVENT_UPGRADE_SECURITY)
    {
        processUpgradeSecurityEvent();
    }
    else if (event->getType() == MegaEvent::EVENT_RELOADING)
    {
        ReloadingEventHandler::instance()->processEvent(std::unique_ptr<MegaEvent>(event->copy()),
                                                        mOsNotifications);
    }
    else if (event->getType() == MegaEvent::EVENT_FATAL_ERROR)
    {
        FatalEventHandler::instance()->processEvent(std::unique_ptr<MegaEvent>(event->copy()),
                                                    logger.get());
    }
}

//Called when a request has finished
void MegaApplication::onRequestFinish(MegaApi*, MegaRequest *request, MegaError* e)
{
    if (appfinished)
    {
        return;
    }

    if (e->getErrorCode() == MegaError::API_EBUSINESSPASTDUE
            && (!lastTsBusinessWarning || (QDateTime::currentMSecsSinceEpoch() - lastTsBusinessWarning) > 3000))//Notify only once within last five seconds
    {
        lastTsBusinessWarning = QDateTime::currentMSecsSinceEpoch();
        mOsNotifications->sendBusinessWarningNotification(businessStatus);
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
            showErrorMessage(tr("Error getting link: %1").arg(QCoreApplication::translate("MegaError", e->getErrorString())));
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
        QString request_email = QString::fromUtf8(request->getEmail());
        if (!preferences->logged()
            || (!request_email.isEmpty() && request_email != preferences->email()))
        {
            break;
        }

        if (e->getErrorCode() != MegaError::API_OK && e->getErrorCode() != MegaError::API_ENOENT)
        {
            break;
        }

        if (request->getParamType() == MegaApi::USER_ATTR_DISABLE_VERSIONS)
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
                if(mTransfersModel)
                {
                    mTransfersModel->globalPauseStateChanged(paused);
                }
                this->paused = paused;
                break;
        }

        if (infoDialog)
        {
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
            mStatsEventHandler->sendEvent(AppStatsEvents::EventType::SYNC_ADD_FAIL_API_EACCESS);
            //this would enforce a fetchNodes in the past
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
            mStatsEventHandler->sendEvent(AppStatsEvents::EventType::PRO_REDIRECT);
        }

        Utilities::openUrl(QUrl(QString::fromUtf8(request->getLink())));
        break;
    }
    case MegaRequest::TYPE_GET_PUBLIC_NODE:
    {
        MegaNode *node = nullptr;
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

                downloadQueue.append(WrappedNode(WrappedNode::TransferOrigin::FROM_APP, node));
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
                infoDialog->setPSAannouncement(static_cast<int>(request->getNumber()),
                                               QString::fromUtf8(request->getName() ? request->getName() : ""),
                                               QString::fromUtf8(request->getText() ? request->getText() : ""),
                                               QString::fromUtf8(request->getFile() ? request->getFile() : ""),
                                               QString::fromUtf8(request->getPassword() ? request->getPassword() : ""),
                                               QString::fromUtf8(request->getLink() ? request->getLink() : ""));
            }
        }

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
        // MegaApi uses an int to define the max number of events, but request->getNumber returns a
        // unsigned long long
        switch (AppStatsEvents::getEventType(static_cast<int>(request->getNumber())))
        {
            case AppStatsEvents::EventType::FIRST_START:
                preferences->setFirstStartDone();
                break;
            case AppStatsEvents::EventType::FIRST_SYNC:
            // Fallthrough
            case AppStatsEvents::EventType::FIRST_SYNC_FROM_ONBOARDING:
                preferences->setFirstSyncDone();
                break;
            case AppStatsEvents::EventType::FIRST_SYNCED_FILE:
                preferences->setFirstFileSynced();
                break;
            case AppStatsEvents::EventType::FIRST_BACKUP:
            // Fallthrough
            case AppStatsEvents::EventType::FIRST_BACKUP_FROM_ONBOARDING:
                preferences->setFirstBackupDone();
                break;
            case AppStatsEvents::EventType::FIRST_BACKED_UP_FILE:
                preferences->setFirstFileBackedUp();
                break;
            case AppStatsEvents::EventType::FIRST_WEBCLIENT_DL:
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
                                             transfer->getSpeed(),
                                             QString::fromUtf8(transfer->getPath()));
    }


    onTransferUpdate(api, transfer);
}

//Called when a transfer has finished
void MegaApplication::onTransferFinish(MegaApi* , MegaTransfer *transfer, MegaError* e)
{
    if (appfinished || transfer->isStreamingTransfer())
    {
        return;
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

    if (e->getErrorCode() == MegaError::API_EBUSINESSPASTDUE
            && (!lastTsBusinessWarning || (QDateTime::currentMSecsSinceEpoch() - lastTsBusinessWarning) > 3000))//Notify only once within last five seconds
    {
        lastTsBusinessWarning = QDateTime::currentMSecsSinceEpoch();
        mOsNotifications->sendBusinessWarningNotification(businessStatus);
    }

    // Check if we have ot send a FIRST_FILE
    if (e->getErrorCode() == MegaError::API_OK
            && transfer->isSyncTransfer()
            && (!mIsFirstFileTwoWaySynced || !mIsFirstFileBackedUp))
    {
        MegaSync::SyncType type (MegaSync::SyncType::TYPE_UNKNOWN);

        // Check transfer local path against configured syncs to determine the sync type
        QString filePath (QString::fromUtf8(transfer->getPath()));

#ifdef WIN32
        if (filePath.startsWith(QString::fromUtf8("\\\\?\\")))
        {
            filePath = filePath.mid(4);
        }
#endif

        filePath = QDir::fromNativeSeparators(filePath);

        auto typeIt (SyncInfo::AllHandledSyncTypes.constBegin());
        while (type == MegaSync::SyncType::TYPE_UNKNOWN
               && typeIt != SyncInfo::AllHandledSyncTypes.constEnd())
        {
            const auto syncsPaths (model->getLocalFolders(*typeIt));
            auto pathIt (syncsPaths.constBegin());
            while (type == MegaSync::SyncType::TYPE_UNKNOWN
                   && pathIt != syncsPaths.constEnd())
            {
                QString syncDir (QDir::fromNativeSeparators(*pathIt) + QLatin1Char('/'));
                if (filePath.startsWith(syncDir))
                {
                    type = *typeIt;
                }
                pathIt++;
            }
            typeIt++;
        }

        // Now emit an event if necessary
        switch (type)
        {
        case MegaSync::SyncType::TYPE_TWOWAY:
        {
            if (!mIsFirstFileTwoWaySynced && !preferences->isFirstFileSynced())
            {
                mStatsEventHandler->sendEvent(AppStatsEvents::EventType::FIRST_SYNCED_FILE);
            }
            mIsFirstFileTwoWaySynced = true;
            break;
        }
        case MegaSync::SyncType::TYPE_BACKUP:
        {
            if (!mIsFirstFileBackedUp && !preferences->isFirstFileBackedUp())
            {
                mStatsEventHandler->sendEvent(AppStatsEvents::EventType::FIRST_BACKED_UP_FILE);
            }
            mIsFirstFileBackedUp = true;
            break;
        }
        default:
            break;
        }
    }
}

//Called when a transfer has been updated
void MegaApplication::onTransferUpdate(MegaApi*, MegaTransfer* transfer)
{
    if (appfinished)
    {
        return;
    }

    if (transfer->isStreamingTransfer() || transfer->isFolderTransfer())
    {
        return;
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
}

// Called when there is a temporal problem in a transfer
void MegaApplication::onTransferTemporaryError(MegaApi* api, MegaTransfer* transfer, MegaError* e)
{
    if (appfinished)
    {
        return;
    }

    onTransferUpdate(api, transfer);

    if (e->getErrorCode() == MegaError::API_EOVERQUOTA)
    {
        if (transfer->isForeignOverquota())
        {
            MegaUser* contact =
                megaApi->getUserFromInShare(megaApi->getNodeByHandle(transfer->getParentHandle()),
                                            true);
            showErrorMessage(
                tr("Your upload(s) cannot proceed because %1's account is full")
                    .arg(contact ? QString::fromUtf8(contact->getEmail()) : tr("contact")));
        }
        else if (e->getValue() && !mTransferQuota->isOverQuota())
        {
            preferences->clearTemporalBandwidth();
            // Get udpated transfer quota (also pro status in case out of quota is due to account
            // paid period expiry).
            AccountDetailsManager::instance()->updateUserStats(
                AccountDetailsManager::Flag::TRANSFER_PRO,
                true,
                USERSTATS_TRANSFERTEMPERROR);
            mTransferQuota->setOverQuota(std::chrono::seconds(e->getValue()));
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
    AccountDetailsManager::instance()->updateUserStats(AccountDetailsManager::Flag::ALL,
                                                       true,
                                                       USERSTATS_ACCOUNTUPDATE);
}

MegaSyncLogger& MegaApplication::getLogger() const
{
    return *logger;
}

void MegaApplication::pushToThreadPool(std::function<void()> functor)
{
    mThreadPool->push(std::move(functor));
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
    }
}

void MegaApplication::onScheduledExecution()
{
    onGlobalSyncStateChangedImpl();
}

void MegaApplication::onCopyLinkError(const QString& nodeName, const int errorCode)
{
    const QString title = tr("Link import error");

    QString message;
    if (errorCode == MegaError::API_EOVERQUOTA)
    {
        message = tr("Link can't be imported, your storage is full");
    }
    else if (errorCode == MegaError::API_EKEY)
    {
        message = tr("Link can't be imported, invalid key or decryption error");
    }
    else
    {
        const QString errorString = QString::fromUtf8(MegaError::getErrorString(errorCode));
        message = tr("Link can't be imported: %1").arg(errorString);
    }

    showErrorMessage(message, title);
}

void MegaApplication::onGlobalSyncStateChanged(MegaApi* api)
{
    // Don't execute the "onGlobalSyncStateChangedImpl" function too often or the dialog locks up,
    // eg. queueing a folder with 1k items for upload/download
    if (mIntervalExecutioner)
    {
        mIntervalExecutioner->scheduleExecution();
    }
}

void MegaApplication::onGlobalSyncStateChangedImpl()
{
    if (appfinished)
    {
        return;
    }

    if (megaApi && infoDialog && mTransfersModel)
    {
        mIndexing = megaApi->isScanning();
        mWaiting = megaApi->isWaiting() || mSyncStalled;

        mSyncing = megaApi->isSyncing();
        mSyncStalled = megaApi->isSyncStalled();

        auto transferCount = mTransfersModel->getTransfersCount();
        auto pendingSyncTransfers = transferCount.pendingSyncTransfers;

        if (!mSyncStalled && mSyncing && pendingSyncTransfers == 0)
        {
            mSyncing = false;

            if (!misSyncingStateWrongLogged)
            {
                misSyncingStateWrongLogged = true;

                auto listener =
                    RequestListenerManager::instance().registerAndGetCustomFinishListener(
                        this,
                        [](::mega::MegaRequest* request, ::mega::MegaError* e)
                        {
                            if (e->getErrorCode() == MegaError::API_OK)
                            {
                                // If mSyncing == true, mSyncStalled == false and
                                // pendingSyncTransfers == 0 and we don´t have transfers in the
                                // throttling queue, there is an unexpected scenario as mSyncing
                                // should be false
                                if (!request->getFlag())
                                {
                                    MegaApi::log(MegaApi::LOG_LEVEL_WARNING,
                                                 "isSyncing is set but no transfers in "
                                                 "the throttling queue");
                                }
                            }
                        });
                megaApi->checkSyncUploadsThrottled(listener.get());
            }
        }
        else if (misSyncingStateWrongLogged)
        {
            misSyncingStateWrongLogged = false;
        }

        mTransferring = transferCount.pendingUploads || transferCount.pendingDownloads;

        auto pendingUploads = transferCount.pendingUploads;
        auto pendingDownloads = transferCount.pendingDownloads;

        if (pendingUploads)
        {
            MegaApi::log(
                MegaApi::LOG_LEVEL_INFO,
                QString::fromUtf8("Pending uploads: %1").arg(pendingUploads).toUtf8().constData());
        }

        if (pendingDownloads)
        {
            MegaApi::log(MegaApi::LOG_LEVEL_INFO,
                         QString::fromUtf8("Pending downloads: %1")
                             .arg(pendingDownloads)
                             .toUtf8()
                             .constData());
        }

        infoDialog->setIndexing(mIndexing);
        infoDialog->setWaiting(mWaiting);
        infoDialog->setSyncing(mSyncing);
        infoDialog->setTransferring(mTransferring);
        infoDialog->updateDialogState();

        MegaApi::log(MegaApi::LOG_LEVEL_INFO,
                     QString::fromUtf8("Current state. Paused = %1 Indexing = %2 Waiting = %3 "
                                       "Syncing = %4 Stalled = %5")
                         .arg(paused)
                         .arg(mIndexing)
                         .arg(mWaiting)
                         .arg(mSyncing)
                         .arg(mSyncStalled)
                         .toUtf8()
                         .constData());

        updateTrayIcon();
    }
}

void MegaApplication::requestFetchSetFromLink(const QString& link)
{
    if (mSetManager)
    {
        mSetManager->requestFetchSetFromLink(link);
    }
}

void MegaApplication::onAppStateChanged(AppState::AppStates, AppState::AppStates)
{
    updateTrayIcon();
    updateTrayIconMenu();
}

bool MegaApplication::mightBeCaseSensitivityIssue(const QString& folderPath)
{
    QFileInfo info(folderPath);
    return info.exists();
}

void MegaApplication::startCrashReportingDialog()
{
#ifdef USE_BREAKPAD
    if (preferences->isCrashed())
    {
        MegaApi::log(MegaApi::LOG_LEVEL_INFO, "Crash detected, loading crash reports");
        preferences->setCrashed(false);
        QStringList reports = CrashHandler::instance()->getPendingCrashReports();
        if (reports.size())
        {
            MegaApi::log(MegaApi::LOG_LEVEL_INFO,
                         QString::fromUtf8("Crash reports found: %1")
                             .arg(reports.size())
                             .toUtf8()
                             .constData());
            QPointer<CrashReportDialog> crashDialog = new CrashReportDialog();
            crashDialog->setAttribute(Qt::WA_DeleteOnClose);
            crashDialog->setParent(crashDialog->parentWidget(), crashDialog->windowFlags());
            TokenParserWidgetManager::instance()->applyCurrentTheme(crashDialog);
            TokenParserWidgetManager::instance()->registerWidgetForTheming(crashDialog);

            connect(crashDialog,
                    &CrashReportDialog::finished,
                    this,
                    [crashDialog, reports, this]()
                    {
                        if (crashDialog->result() != QDialog::Accepted)
                        {
                            MegaApi::log(MegaApi::LOG_LEVEL_INFO,
                                         "Crash dialog cancelled, deleting crash report");
                            CrashHandler::instance()->deletePendingCrashReports(reports);
                            return;
                        }
                        applyProxySettings();
                        const bool shouldSendLogs = crashDialog->sendLogs();
                        CrashHandler::instance()->sendPendingCrashReports(
                            crashDialog->getUserMessage(),
                            shouldSendLogs);
                    });
            crashDialog->exec();
        }
        else
        {
            MegaApi::log(MegaApi::LOG_LEVEL_WARNING, "No crash reports found.");
        }
    }
#endif
}

// clang-format off
void MegaApplication::onOperatingSystemThemeChanged(const Preferences::SystemColorScheme& theme)
{
#ifdef Q_OS_LINUX
    constexpr QSize iconSize(256, 256);

    auto applyTokenToIconAction = [&](const QPointer<QMenu>& menu)
    {
        if (!menu.isNull() && menu.data() != nullptr)
        {
            foreach(auto action, menu->actions())
            {
                auto coloredPixmap = IconTokenizer::changePixmapColor(
                    action->icon().pixmap(iconSize),
                    TokenParserWidgetManager::instance()->getColor(
                        QStringLiteral("icon-primary"),
                        ThemeManager::instance()->getColorSchemaString(theme.systemScheme)));

                if (coloredPixmap.has_value())
                {
                    action->setIcon(coloredPixmap.value());
                }
            }
        }
    };

    applyTokenToIconAction(initialTrayMenu);
#endif
}

// clang-format on

#include "MegaApplication.h"
#include "CrashReportDialog.h"
#include "MegaProxyStyle.h"
#include "QMegaMessageBox.h"
#include "control/AppStatsEvents.h"
#include "control/Utilities.h"
#include "control/CrashHandler.h"
#include "control/ExportProcessor.h"
#include "EventUpdater.h"
#include "platform/Platform.h"
#include "OverQuotaDialog.h"
#include "ConnectivityChecker.h"
#include "StalledIssuesModel.h"
#include "TransferMetadata.h"
#include "DuplicatedNodeDialogs/DuplicatedNodeDialog.h"
#include "PlatformStrings.h"
#include "UserAttributesManager.h"
#include "UserAttributesRequests/FullName.h"
#include "UserAttributesRequests/Avatar.h"
#include "TransferNotificationMessageBuilder.h"
#include "UserAttributesRequests/DeviceName.h"
#include "UserAttributesRequests/MyBackupsHandle.h"
#include "syncs/gui/SyncsMenu.h"
#include "TextDecorator.h"
#include "platform/PowerOptions.h"

#include "mega/types.h"

#include <QTranslator>
#include <QClipboard>
#include <QDesktopWidget>
#include <QFontDatabase>
#include <QNetworkProxy>
#include <QScreen>
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

using namespace mega;
using namespace std;

QString MegaApplication::appPath = QString();
QString MegaApplication::appDirPath = QString();
QString MegaApplication::dataPath = QString();
QString MegaApplication::lastNotificationError = QString();

constexpr auto openUrlClusterMaxElapsedTime = std::chrono::seconds(5);

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

MegaApplication::MegaApplication(int &argc, char **argv) :
    QApplication(argc, argv),
    mSyncs2waysMenu(nullptr),
    mBackupsMenu(nullptr),
    mIsFirstFileTwoWaySynced(false),
    mIsFirstFileBackedUp(false),
    scanStageController(this),
    mDisableGfx (false)
{

#if defined Q_OS_MACX && !defined QT_DEBUG
    if (!getenv("MEGA_DISABLE_RUN_MAC_RESTRICTION"))
    {
        QString path = appBundlePath();
        if (path.compare(QStringLiteral("/Applications/MEGAsync.app")))
        {
            QMessageBox::warning(nullptr, tr("Error"), QCoreApplication::translate("MegaSyncError", "You can't run MEGA Desktop App from this location. Move it into the Applications folder then run it."), QMessageBox::Ok);
            ::exit(0);
        }
    }
#endif

    appfinished = false;

    bool logToStdout = false;

    // Collect program arguments
    QStringList args;
    for (int i=0; i < argc; ++i)
    {
        args += QString::fromUtf8(argv[i]);
    }

#ifdef Q_OS_LINUX

    if (args.contains(QLatin1String("--version")))
    {
        QTextStream(stdout) << "MEGAsync" << " v" << Preferences::VERSION_STRING << " (" << Preferences::SDK_ID << ")" << endl;
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

#ifdef _WIN32
    setStyleSheet(QString::fromUtf8(
    "QRadioButton::indicator, QCheckBox::indicator, QAbstractItemView::indicator {width: 12px; height: 12px;}"
    "QRadioButton::indicator:unchecked {image: url(:/images/rb_unchecked.svg);}"
    "QRadioButton::indicator:unchecked:hover {image: url(:/images/rb_unchecked_hover.svg);}"
    "QRadioButton::indicator:unchecked:pressed {image: url(:/images/rb_unchecked_pressed.svg);}"
    "QRadioButton::indicator:unchecked:disabled {image: url(:/images/rb_unchecked_disabled.svg);}"
    "QRadioButton::indicator:checked {image: url(:/images/rb_checked.svg);}"
    "QRadioButton::indicator:checked:hover {image: url(:/images/rb_checked_hover.svg);}"
    "QRadioButton::indicator:checked:pressed {image: url(:/images/rb_checked_pressed.svg);}"
    "QRadioButton::indicator:checked:disabled {image: url(:/images/rb_checked_disabled.svg);}"
    "QCheckBox::indicator:checked, QAbstractItemView::indicator:checked {image: url(:/images/cb_checked.svg);}"
    "QCheckBox::indicator:checked:hover, QAbstractItemView::indicator:checked:hover {image: url(:/images/cb_checked_hover.svg);}"
    "QCheckBox::indicator:checked:pressed, QAbstractItemView::indicator:checked:pressed {image: url(:/images/cb_checked_pressed.svg);}"
    "QCheckBox::indicator:checked:disabled, QAbstractItemView::indicator:checked:disabled {image: url(:/images/cb_checked_disabled.svg);}"
    "QCheckBox::indicator:unchecked, QAbstractItemView::indicator:unchecked {image: url(:/images/cb_unchecked.svg);}"
    "QCheckBox::indicator:unchecked:hover, QAbstractItemView::indicator:unchecked:hover {image: url(:/images/cb_unchecked_hover.svg);}"
    "QCheckBox::indicator:unchecked:pressed, QAbstractItemView::indicator:unchecked:pressed {image: url(:/images/cb_unchecked_pressed.svg);}"
    "QCheckBox::indicator::unchecked:disabled, QAbstractItemView:indicator:unchecked:disabled {image: url(:/images/cb_unchecked_disabled.svg);}"
    "QMessageBox QLabel {font-size: 13px;}"
    "QMenu {font-size: 13px;}"
    "QToolTip {color: #FAFAFA; background-color: #333333; border: 0px; margin-bottom: 2px;}"
    "QPushButton {font-size: 12px; padding-right: 12px; padding-left: 12px; min-height: 22px;}"
    "QFileDialog QWidget {font-size: 13px;}"
                      ));
#endif

    // For some reason this doesn't work on Windows (done in stylesheet above)
    // TODO: re-try with Qt > 5.12.12
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
    trayIcon = NULL;
    verifyEmail = nullptr;
    infoDialogMenu = nullptr;
    guestMenu = nullptr;
    megaApi = NULL;
    megaApiFolders = NULL;
    delegateListener = NULL;
    httpServer = NULL;
    httpsServer = NULL;
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
    initialTrayMenu = nullptr;
    lastHovered = NULL;
    isPublic = false;
    prevVersion = 0;
    updatingSSLcert = false;
    lastSSLcertUpdate = 0;
    mTransfersModel = nullptr;
    mStalledIssuesModel = nullptr;

    notificationsModel = NULL;
    notificationsProxyModel = NULL;
    notificationsDelegate = NULL;

    context = new QObject(this);

#ifdef _WIN32
    windowsMenu = nullptr;
    windowsExitAction = NULL;
    windowsUpdateAction = NULL;
    windowsAboutAction = nullptr;
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
    initialExitAction = NULL;
    uploadAction = NULL;
    downloadAction = NULL;
    streamAction = NULL;
    myCloudAction = NULL;
    waiting = false;
    updated = false;
    syncing = false;
    syncStalled = false;
    transferring = false;
    checkupdate = false;
    updateAction = NULL;
    aboutAction = nullptr;
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
    downloadNodeSelector = NULL;
    mPricing.reset();
    mCurrency.reset();
    storageOverquotaDialog = NULL;
    infoWizard = NULL;
    mTransferManager = nullptr;
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

    mDisableGfx = args.contains(QLatin1String("--nogfx")) || args.contains(QLatin1String("/nogfx"));
    folderTransferListener = std::make_shared<FolderTransferListener>(nullptr);

    connect(folderTransferListener.get(), &FolderTransferListener::folderTransferUpdated,
            this, &MegaApplication::onFolderTransferUpdate);

    connect(&scanStageController, &ScanStageController::enableTransferActions,
            this, &MegaApplication::enableTransferActions);

    connect(&transferProgressController, &BlockingStageProgressController::updateUi,
            &scanStageController, &ScanStageController::onFolderTransferUpdate);
}

MegaApplication::~MegaApplication()
{
    PowerOptions::appShutdown();

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
        //If the dialog is active and visible -> show it
        if (settingsDialog && settingsDialog->isVisible())
        {
            settingsDialog->show();
            return;
        }
    }
}

bool gCrashableForTesting = false;

void MegaApplication::initialize()
{
    if (megaApi)
    {
        return;
    }

    paused = false;
    indexing = false;

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
    connect(preferences.get(), SIGNAL(stateChanged()), this, SLOT(changeState()));
    connect(preferences.get(), SIGNAL(updated(int)), this, SLOT(showUpdatedMessage(int)));
    preferences->initialize(dataPath);

    model = SyncInfo::instance();

    connect(model, SIGNAL(syncStateChanged(std::shared_ptr<SyncSettings>)),
            this, SLOT(onSyncStateChanged(std::shared_ptr<SyncSettings>)));
    connect(model, SIGNAL(syncRemoved(std::shared_ptr<SyncSettings>)),
            this, SLOT(onSyncDeleted(std::shared_ptr<SyncSettings>)));

    if (preferences->error())
    {
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("Encountered corrupt prefrences.").toUtf8().constData());
        QMegaMessageBox::critical(nullptr, QString::fromUtf8("MEGAsync"), tr("Your config is corrupt, please start over"));
    }

    preferences->setLastStatsRequest(0);
    lastExit = preferences->getLastExit();

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

    QString basePath = QDir::toNativeSeparators(dataPath + QString::fromUtf8("/"));
    megaApi = new MegaApi(Preferences::CLIENT_KEY, basePath.toUtf8().constData(), Preferences::USER_AGENT);
    megaApi->disableGfxFeatures(mDisableGfx);

    megaApiFolders = new MegaApi(Preferences::CLIENT_KEY, basePath.toUtf8().constData(), Preferences::USER_AGENT);
    megaApiFolders->disableGfxFeatures(mDisableGfx);

    MegaApi::log(MegaApi::LOG_LEVEL_INFO, QString::fromLatin1("Graphics processing %1")
                 .arg(mDisableGfx ? QLatin1String("disabled")
                                  : QLatin1String("enabled"))
                 .toUtf8().constData());

    // Set maximum log line size to 10k (same as SDK default)
    // Otherwise network logging can cause large glitches when logging hundreds of MB
    // On Mac it is particularly apparent, causing the beachball to appear often
    long long newPayLoadLogSize = 10240;
    megaApi->log(MegaApi::LOG_LEVEL_INFO, QString::fromUtf8("Establishing max payload log size: %1").arg(newPayLoadLogSize).toUtf8().constData());
    megaApi->setMaxPayloadLogSize(newPayLoadLogSize);
    megaApiFolders->setMaxPayloadLogSize(newPayLoadLogSize);

    QString stagingPath = QDir(dataPath).filePath(QString::fromUtf8("megasync.staging"));
    QFile fstagingPath(stagingPath);
    if (fstagingPath.exists())
    {
        QSettings settings(stagingPath, QSettings::IniFormat);
        QString apiURL = settings.value(QString::fromUtf8("apiurl"), QString::fromUtf8("https://staging.api.mega.co.nz/")).toString();
        QString disablepkp = settings.value(QString::fromUtf8("disablepkp"), QString::fromUtf8("0")).toString();
        megaApi->changeApiUrl(apiURL.toUtf8(), disablepkp == QString::fromUtf8("1"));
        megaApiFolders->changeApiUrl(apiURL.toUtf8());
        QMegaMessageBox::warning(nullptr, QString::fromUtf8("MEGAsync"), QString::fromUtf8("API URL changed to ")+ apiURL);

        QString baseURL = settings.value(QString::fromUtf8("baseurl"), Preferences::BASE_URL).toString();
        Preferences::setBaseUrl(baseURL);
        if (baseURL.compare(QString::fromUtf8("https://mega.nz")))
        {
            QMegaMessageBox::warning(nullptr, QString::fromUtf8("MEGAsync"), QString::fromUtf8("base URL changed to ") + Preferences::BASE_URL);
        }

        gCrashableForTesting = settings.value(QString::fromUtf8("crashable"), false).toBool();

        Preferences::overridePreferences(settings);
        Preferences::SDK_ID.append(QString::fromUtf8(" - STAGING"));
    }
    trayIcon->show();

    megaApi->log(MegaApi::LOG_LEVEL_INFO, QString::fromUtf8("MEGAsync is starting. Version string: %1   Version code: %2.%3   User-Agent: %4").arg(Preferences::VERSION_STRING)
             .arg(Preferences::VERSION_CODE).arg(Preferences::BUILD_ID).arg(QString::fromUtf8(megaApi->getUserAgent())).toUtf8().constData());

    megaApi->setLanguage(currentLanguageCode.toUtf8().constData());
    megaApiFolders->setLanguage(currentLanguageCode.toUtf8().constData());
    setMaxConnections(MegaTransfer::TYPE_UPLOAD,   preferences->parallelUploadConnections());
    setMaxConnections(MegaTransfer::TYPE_DOWNLOAD, preferences->parallelDownloadConnections());
    setUseHttpsOnly(preferences->usingHttpsOnly());

    megaApi->setDefaultFilePermissions(preferences->filePermissionsValue());
    megaApi->setDefaultFolderPermissions(preferences->folderPermissionsValue());
    megaApi->retrySSLerrors(true);
    megaApi->setPublicKeyPinning(!preferences->SSLcertificateException());

    delegateListener = new MEGASyncDelegateListener(megaApi, this, this);
    megaApi->addListener(delegateListener);
    uploader = new MegaUploader(megaApi, folderTransferListener);
    downloader = new MegaDownloader(megaApi, folderTransferListener);
    connect(uploader, &MegaUploader::startingTransfers, this, &MegaApplication::startingUpload);
    connect(downloader, &MegaDownloader::finishedTransfers, this, &MegaApplication::showNotificationFinishedTransfers, Qt::QueuedConnection);
    connect(downloader, &MegaDownloader::startingTransfers,
            &scanStageController, &ScanStageController::startDelayedScanStage);
    connect(downloader, &MegaDownloader::folderTransferUpdated, this, &MegaApplication::onFolderTransferUpdate);

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
        MegaApi::log(MegaApi::LOG_LEVEL_WARNING, QString::fromUtf8("Force reloading (isCrashed true)").toUtf8().constData());
        preferences->setCrashed(false);
        QDirIterator di(dataPath, QDir::Files | QDir::NoDotAndDotDot);
        while (di.hasNext())
        {
            di.next();
            const QFileInfo& fi = di.fileInfo();
            if (!fi.fileName().contains(QString::fromUtf8("transfers_"))
                && !fi.fileName().contains(QString::fromUtf8("syncconfigsv2_"))
                && (fi.fileName().endsWith(QString::fromUtf8(".db"))
                    || fi.fileName().endsWith(QString::fromUtf8(".db-wal"))
                    || fi.fileName().endsWith(QString::fromUtf8(".db-shm"))))
            {
                MegaApi::log(MegaApi::LOG_LEVEL_WARNING, QString::fromUtf8("Deleting local cache: %1").arg(di.filePath()).toUtf8().constData());
                QFile::remove(di.filePath());
            }
        }

        QStringList reports = CrashHandler::instance()->getPendingCrashReports();
        if (reports.size())
        {
            CrashReportDialog crashDialog(reports.join(QString::fromUtf8("------------------------------\n")));
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

                    connect(logger.get(), &MegaSyncLogger::logReadyForReporting, context, [this, crashTimestamp]()
                    {
                        crashReportFilePath = Utilities::joinLogZipFiles(megaApi, &crashTimestamp, CrashHandler::instance()->getLastCrashHash());
                        if (!crashReportFilePath.isNull()
                                && megaApi && megaApi->isLoggedIn())
                        {
                            megaApi->startUploadForSupport(QDir::toNativeSeparators(crashReportFilePath).toUtf8().constData(), false);
                            crashReportFilePath.clear();
                        }
                        context->deleteLater();
                    });

                    logger->prepareForReporting();
                }

#ifndef __APPLE__
                QMegaMessageBox::information(nullptr, QString::fromUtf8("MEGAsync"), tr("Thank you for your collaboration"));
#endif
            }
        }
    }

    transferQuota = std::make_shared<TransferQuota>(mOsNotifications);
    connect(transferQuota.get(), &TransferQuota::waitTimeIsOver, this, &MegaApplication::updateStatesAfterTransferOverQuotaTimeHasExpired);

    periodicTasksTimer = new QTimer(this);
    periodicTasksTimer->start(Preferences::STATE_REFRESH_INTERVAL_MS);
    connect(periodicTasksTimer, SIGNAL(timeout()), this, SLOT(periodicTasks()));

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

    firstTransferTimer = new QTimer(this);
    firstTransferTimer->setSingleShot(true);
    firstTransferTimer->setInterval(200);
    connect(firstTransferTimer, &QTimer::timeout, this, &MegaApplication::checkFirstTransfer);

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

    mTransfersModel = new TransfersModel(nullptr);
    connect(mTransfersModel.data(), &TransfersModel::transfersCountUpdated, this, &MegaApplication::onTransfersModelUpdate);

    mStalledIssuesModel = new StalledIssuesModel(this);
    connect(Platform::getShellNotifier().get(), &AbstractShellNotifier::shellNotificationProcessed,
            this, &MegaApplication::onNotificationProcessed);
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

    const bool isOverQuotaOrPaywall{appliedStorageState == MegaApi::STORAGE_STATE_RED ||
                transferQuota->isOverQuota() ||
                appliedStorageState == MegaApi::STORAGE_STATE_PAYWALL};
    if (isOverQuotaOrPaywall)
    {
        tooltipState = tr("Over quota");

        icon = icons["warning"];

#ifdef __APPLE__
        if (scanningTimer->isActive())
        {
            scanningTimer->stop();
        }
#endif
    }
    else if (blockState)
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
    else if (!nodescurrent || !getRootNode())
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
    else if (syncStalled)
    {
        tooltipState = tr("Stalled");
        icon = icons["alert"];
    }
    else if (paused)
    {
        long long transfersFailed(mTransfersModel ? mTransfersModel->failedTransfers() : 0);

        if(transfersFailed > 0)
        {
            tooltipState = QCoreApplication::translate("TransferManager","Issue found", "", transfersFailed);
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
    else if (indexing || waiting || syncing || transferring)
    {
        if (indexing)
        {
            tooltipState = tr("Scanning");
        }
        else if (syncing)
        {
            tooltipState = tr("Syncing");
        }
        else if (waiting)
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
        long long transfersFailed(mTransfersModel ? mTransfersModel->failedTransfers() : 0);

        if(transfersFailed > 0)
        {
            tooltipState = QCoreApplication::translate("TransferManager","Issue found", "", transfersFailed);
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

    QString tooltip = QString::fromUtf8("%1 %2\n%3").arg(QCoreApplication::applicationName()).arg(Preferences::VERSION_STRING).arg(tooltipState);

    if (updateAvailable)
    {
        tooltip += QString::fromUtf8("\n") + tr("Update available!");
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
    getUserDataRequestReady = false;
    mFetchingNodes = false;
    mQueringWhyAmIBlocked = false;
    whyamiblockedPeriodicPetition = false;
    storageState = MegaApi::STORAGE_STATE_UNKNOWN;
    appliedStorageState = MegaApi::STORAGE_STATE_UNKNOWN;
    eventsPendingLoggedIn.clear();
    receivedStorageSum = 0;
    finishedBlockedTransfers.clear();
    mSyncController.reset();

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

    if (!preferences->lastExecutionTime())
    {
        Platform::enableTrayIcon(QFileInfo(MegaApplication::applicationFilePath()).fileName());
    }

    if (updated)
    {
        showInfoMessage(tr("MEGAsync has been updated"));
        preferences->setFirstSyncDone();
        preferences->setFirstFileSynced();
        preferences->setFirstBackupDone();
        preferences->setFirstFileBackedUp();
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
    auto current = QOperatingSystemVersion::current();
    if (current > QOperatingSystemVersion::OSXMavericks) //FinderSync API support from 10.10+
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
            megaApi->sendEvent(AppStatsEvents::EVENT_UPDATE, "MEGAsync update");
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
            megaApi->sendEvent(AppStatsEvents::EVENT_1ST_START, "MEGAsync first start");
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
            megaApi->sendEvent(AppStatsEvents::EVENT_UPDATE, "MEGAsync update");
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
    UserAttributes::DeviceName::requestDeviceName();
    UserAttributes::MyBackupsHandle::requestMyBackupsHandle();
    UserAttributes::FullName::requestFullName();
    UserAttributes::Avatar::requestAvatar();

    megaApi->getPricing();
    megaApi->getFileVersionsOption();
    megaApi->getPSA();
}

void MegaApplication::populateUserAlerts(MegaUserAlertList *theList, bool copyRequired)
{
    if (!theList)
    {
        return;
    }

    if (mOsNotifications)
    {
        mOsNotifications->addUserAlertList(theList);
    }

    if (notificationsModel)
    {
        notificationsModel->insertAlerts(theList, copyRequired);
    }
    else
    {
        notificationsModel = new QAlertsModel(theList, copyRequired);
        notificationsProxyModel = new QFilterAlertsModel();
        notificationsProxyModel->setSourceModel(notificationsModel);
        notificationsProxyModel->setSortRole(Qt::UserRole); //Role used to sort the model by date.

        notificationsDelegate = new MegaAlertDelegate(notificationsModel, true, this);

        if (infoDialog)
        {
            infoDialog->updateNotificationsTreeView(notificationsProxyModel, notificationsDelegate);
        }
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
        theList->clear(); //empty the list otherwise they will be deleted
        delete theList;
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

    mSyncController.reset(new SyncController());
    connect(mSyncController.get(), &SyncController::syncAddStatus, this, [](const int errorCode, const QString errorMsg, QString name)
    {
        if (errorCode != MegaError::API_OK)
        {
            QString msg = errorMsg;
            Text::Link link(Utilities::SUPPORT_URL);
            Text::Decorator tc(&link);
            tc.process(msg);
            QMegaMessageBox::warning(nullptr, tr("Error"), tr("Error adding %1:").arg(name)
                                     + QString::fromLatin1("\n")
                                     + msg, QMessageBox::Ok, QMessageBox::NoButton, QMap<QMessageBox::StandardButton, QString>(), Qt::RichText);
        }
    });

    //Check business status in case we need to alert the user
    if (megaApi->isBusinessAccount())
    {
        manageBusinessStatus(megaApi->getBusinessStatus());
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
    if (!Platform::startOnStartup(startOnStartup))
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
    infoDialog->setAvatar();
    infoDialog->setAccountType(preferences->accountType());

    model->setUnattendedDisabledSyncs(preferences->getDisabledSyncTags());

    if (preferences->getNotifyDisabledSyncsOnLogin())
    {
        auto settingsTabToOpen = SettingsDialog::SYNCS_TAB;
        QString message;
        QVector<MegaSync::SyncType> syncsTypesToDismiss;

        bool haveSyncs (false);
        bool haveBackups (false);

        // Check if we have syncs and backups
        if (model)
        {
            haveSyncs = model->hasUnattendedDisabledSyncs(MegaSync::TYPE_TWOWAY);
            haveBackups = model->hasUnattendedDisabledSyncs(MegaSync::TYPE_BACKUP);
        }

        // Set text according to situation
        if (haveSyncs && haveBackups)
        {
            syncsTypesToDismiss = {MegaSync::TYPE_TWOWAY, MegaSync::TYPE_BACKUP};
            message = PlatformStrings::syncsAndBackupsDisableWarning();
        }
        else if (haveBackups)
        {
            settingsTabToOpen = SettingsDialog::BACKUP_TAB;
            syncsTypesToDismiss = {MegaSync::TYPE_BACKUP};
            message = PlatformStrings::backupsDisableWarning();
        }
        else if (haveSyncs)
        {
            syncsTypesToDismiss = {MegaSync::TYPE_TWOWAY};
            message = PlatformStrings::syncsDisableWarning();
        }

        // Display the message if it has been set
        if (!message.isEmpty())
        {
            QMessageBox msgBox (QMessageBox::Warning, QCoreApplication::applicationName(), message);
            QString buttonText (PlatformStrings::openSettings());
            QPushButton *openPreferences = msgBox.addButton(buttonText, QMessageBox::YesRole);

            msgBox.addButton(tr("Dismiss"), QMessageBox::NoRole);
            msgBox.setDefaultButton(openPreferences);
            msgBox.exec();
            if (msgBox.clickedButton() == openPreferences)
            {
                openSettings(settingsTabToOpen);
            }
        }

        preferences->setNotifyDisabledSyncsOnLogin(false);
        model->dismissUnattendedDisabledSyncs(syncsTypesToDismiss);
    }

    // Init first synced and first backed-up file states from preferences
    mIsFirstFileTwoWaySynced = preferences->isFirstFileSynced();
    mIsFirstFileBackedUp = preferences->isFirstFileBackedUp();

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

    mThreadPool->push([=](){
    setMaxUploadSpeed(preferences->uploadLimitKB());
    setMaxDownloadSpeed(preferences->downloadLimitKB());
    setMaxConnections(MegaTransfer::TYPE_UPLOAD,   preferences->parallelUploadConnections());
    setMaxConnections(MegaTransfer::TYPE_DOWNLOAD, preferences->parallelDownloadConnections());
    setUseHttpsOnly(preferences->usingHttpsOnly());

    megaApi->setDefaultFilePermissions(preferences->filePermissionsValue());
    megaApi->setDefaultFolderPermissions(preferences->folderPermissionsValue());
    });

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

    preferences->monitorUserAttributes();
}

void MegaApplication::startSyncs(QList<PreConfiguredSync> syncs)
{
    if (appfinished)
    {
        return;
    }

    // Load default exclusion rules before adding the new syncs from setup wizard.
    // We could not load them before fetch nodes, because default exclusion rules
    // are only created once the local preferences are logged.
    std::unique_ptr<char[]> email(megaApi->getMyEmail());
    loadSyncExclusionRules(QString::fromUtf8(email.get()));

    // add syncs from setupWizard
    for (auto & ps : syncs)
    {
        QString localFolderPath = ps.localFolder();
        MegaApi::log(MegaApi::LOG_LEVEL_INFO, QString::fromLatin1("Adding sync %1 from SetupWizard: ").arg(localFolderPath).toUtf8().constData());
        mSyncController->addSync(localFolderPath, ps.megaFolderHandle(), ps.syncName(), MegaSync::TYPE_TWOWAY);
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
                    settingsDialog = nullptr;
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

    std::shared_ptr<MegaNode> node(megaApi->getNodeByHandle(nodeHandle));

    //If the destination node doesn't exist in the current filesystem, clear the queue and show an error message
    if (!node || node->isFile())
    {
        uploadQueue.clear();
        showErrorMessage(tr("Error: Invalid destination folder. The upload has been cancelled"));
        return;
    }

    noUploadedStarted = true;

    unsigned long long transferId = preferences->transferIdentifier();
    TransferMetaData* data = new TransferMetaData(MegaTransfer::TYPE_UPLOAD, uploadQueue.size(), uploadQueue.size());
    transferAppData.insert(transferId, data);
    preferences->setOverStorageDismissExecution(0);

    DuplicatedNodeDialog checkDialog;
    HighDpiResize hDpiResizer(&checkDialog);

    auto counter(0);
    EventUpdater checkUpdater(uploadQueue.size());
    while (!uploadQueue.isEmpty())
    {
        QString nodePath = uploadQueue.dequeue();
        checkDialog.checkUpload(nodePath, node);

        checkUpdater.update(counter);
        counter++;
    }

    QList<std::shared_ptr<DuplicatedNodeInfo>> uploads = checkDialog.show();

    auto batch = std::shared_ptr<TransferBatch>(new TransferBatch());
    mBlockingBatch.add(batch);

    EventUpdater updater(uploads.size(),20);
    mProcessingUploadQueue = true;

    counter = 0;
    foreach(auto uploadInfo, uploads)
    {
        QString filePath = uploadInfo->getLocalPath();

        updateMetadata(data, filePath);

        uploader->upload(filePath, uploadInfo->getNewName(), node, transferId, batch);

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
    }

    mProcessingUploadQueue = false;
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
        QQueue<WrappedNode *>::iterator it;
        for (it = downloadQueue.begin(); it != downloadQueue.end(); ++it)
        {
            HTTPServer::onTransferDataUpdate((*it)->getMegaNode()->getHandle(),
                                             MegaTransfer::STATE_CANCELLED,
                                             0, 0, 0, QString());
        }

        qDeleteAll(downloadQueue);
        downloadQueue.clear();
        showErrorMessage(tr("Error: Invalid destination folder. The download has been cancelled"));
        return;
    }

    unsigned long long transferId = preferences->transferIdentifier();
    TransferMetaData *transferData =  new TransferMetaData(MegaTransfer::TYPE_DOWNLOAD,
                                                           downloadQueue.size(),
                                                           downloadQueue.size());
    transferAppData.insert(transferId, transferData);
    if (!downloader->processDownloadQueue(&downloadQueue, mBlockingBatch, path, transferId))
    {
        transferAppData.remove(transferId);
        delete transferData;
    }
}

void MegaApplication::closeDialogs(bool/* bwoverquota*/)
{
    delete mTransferManager;
    mTransferManager = nullptr;

    delete setupWizard;
    setupWizard = NULL;

    delete settingsDialog;
    settingsDialog = nullptr;

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

    if(transferQuota)
    {
        transferQuota->closeDialogs();
    }

    delete storageOverquotaDialog;
    storageOverquotaDialog = NULL;

    verifyEmail.reset(nullptr);
}

void MegaApplication::createTransferManagerDialog(TransfersWidget::TM_TAB tab)
{
    mTransferManager = new TransferManager(tab, megaApi);
    infoDialog->setTransferManager(mTransferManager);

    // Signal/slot to notify the tracking of unseen completed transfers of Transfer Manager. If Completed tab is
    // active, tracking is disabled
    connect(mTransferManager.data() , &TransferManager::userActivity, this, &MegaApplication::registerUserActivity);
    connect(transferQuota.get(), &TransferQuota::sendState,
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
    if (update && (transferCount.pendingDownloads || transferCount.pendingUploads || megaApi->isWaiting()))
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

    QApplication::exit();
}

int* testCrashPtr = nullptr;

void MegaApplication::tryExitApplication(bool force)
{
    if (appfinished)
    {
        return;
    }

    if (dontAskForExitConfirmation(force))
    {
        exitApplication();
    }
    else
    {
        QString exitMessage = tr("There is an active transfer. Exit the app?\n"
                                 "Transfer will automatically resume when you re-open the app.",
                                 "",
                                 mTransfersModel->hasActiveTransfers());
        auto exitDialog = new QMessageBox(QMessageBox::Question, tr("MEGAsync"), exitMessage, QMessageBox::Yes|QMessageBox::No);
        exitDialog->setAttribute(Qt::WA_DeleteOnClose);
        exitDialog->button(QMessageBox::Yes)->setText(tr("Exit app"));
        exitDialog->button(QMessageBox::No)->setText(tr("Stay in app"));
        HighDpiResize hDpiResizer(exitDialog);
        int button = exitDialog->exec();

        QPointer<MegaApplication> currentMegaApp(this);
        if (!currentMegaApp)
        {
            return;
        }

        if (button == QMessageBox::Yes)
        {
            exitApplication();
        }
        else if (gCrashableForTesting)
        {
            *testCrashPtr = 0;
        }
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
    long long numNodes = megaApi->getNumNodes();
    long long numLocalNodes = megaApi->getNumLocalNodes();
    long long totalNodes = numNodes + numLocalNodes;
    auto transferCount = getTransfersModel()->getTransfersCount();
    long long totalTransfers =  transferCount.pendingUploads + transferCount.pendingDownloads;
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
                 .arg(static_cast<float>(procesUsage) / static_cast<float>(totalNodes))
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
            megaApi->sendEvent(AppStatsEvents::EVENT_MEM_USAGE, QString::fromUtf8("%1 %2 %3")
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
            megaApi->sendEvent(AppStatsEvents::EVENT_OVER_STORAGE_DIAL,
                               "Overstorage dialog shown");
            if (!storageOverquotaDialog)
            {
                storageOverquotaDialog = new UpgradeOverStorage(megaApi, mPricing, mCurrency);
                connect(storageOverquotaDialog, SIGNAL(finished(int)), this, SLOT(storageOverquotaDialogFinished(int)));
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
            megaApi->sendEvent(AppStatsEvents::EVENT_OVER_STORAGE_NOTIF,
                               "Overstorage notification shown");
            mOsNotifications->sendOverStorageNotification(Preferences::STATE_OVER_STORAGE);
        }

        if (infoDialog)
        {
            if (!preferences->getOverStorageDismissExecution()
                    || ((QDateTime::currentMSecsSinceEpoch() - preferences->getOverStorageDismissExecution()) > Preferences::OQ_UI_MESSAGE_INTERVAL_MS))
            {
                if (infoDialog->updateOverStorageState(Preferences::STATE_OVER_STORAGE))
                {
                    megaApi->sendEvent(AppStatsEvents::EVENT_OVER_STORAGE_MSG,
                                       "Overstorage warning shown");
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
                    megaApi->sendEvent(AppStatsEvents::EVENT_ALMOST_OVER_STORAGE_MSG,
                                       "Almost overstorage warning shown");
                }
            }
            else
            {
                infoDialog->updateOverStorageState(Preferences::STATE_OVER_STORAGE_DISMISSED);
            }
        }

        auto transferCount = getTransfersModel()->getTransfersCount();
        long long pendingTransfers =  transferCount.pendingUploads || transferCount.pendingDownloads;

        if (!pendingTransfers && ((QDateTime::currentMSecsSinceEpoch() - preferences->getOverStorageNotificationExecution()) > Preferences::ALMOST_OQ_UI_MESSAGE_INTERVAL_MS)
                              && ((QDateTime::currentMSecsSinceEpoch() - preferences->getOverStorageDialogExecution()) > Preferences::ALMOST_OQ_UI_MESSAGE_INTERVAL_MS)
                              && (!preferences->getAlmostOverStorageNotificationExecution() || (QDateTime::currentMSecsSinceEpoch() - preferences->getAlmostOverStorageNotificationExecution()) > Preferences::ALMOST_OQ_UI_MESSAGE_INTERVAL_MS))
        {
            preferences->setAlmostOverStorageNotificationExecution(QDateTime::currentMSecsSinceEpoch());
            megaApi->sendEvent(AppStatsEvents::EVENT_ALMOST_OVER_STORAGE_NOTIF,
                               "Almost overstorage notification shown");
            mOsNotifications->sendOverStorageNotification(Preferences::STATE_ALMOST_OVER_STORAGE);
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
                int64_t remainDaysOut(0);
                Utilities::getDaysToTimestamp(megaApi->getOverquotaDeadlineTs(), remainDaysOut);
                if (remainDaysOut > 0) //Only show notification if at least there is one day left
                {
                    preferences->setPayWallNotificationExecution(QDateTime::currentMSecsSinceEpoch());
                    megaApi->sendEvent(AppStatsEvents::EVENT_PAYWALL_NOTIF,
                                       "Paywall notification shown");
                    mOsNotifications->sendOverStorageNotification(Preferences::STATE_PAYWALL);
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

void MegaApplication::checkOverQuotaStates()
{
    transferQuota->checkQuotaAndAlerts();
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
        queuedUserStats[0] = queuedUserStats[1] = queuedUserStats[2] = false;
        updateUserStats(storage, transfer, pro, false, -1);
    }

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
                megaApi->sendEvent(AppStatsEvents::EVENT_UPDATE_OK, "MEGAsync updated OK");
            }

            checkMemoryUsage();
            mThreadPool->push([=]()
            {//thread pool function
                megaApi->update();

                Utilities::queueFunctionInAppThread([=]()
                {//queued function
                    checkOverStorageStates();
                    checkOverQuotaStates();
                });//end of queued function

            });// end of thread pool function
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
    networkCheckTimer->stop();
    stopUpdateTask();
    Platform::stopShellDispatcher();

    for (auto localFolder : model->getLocalFolders(SyncInfo::AllHandledSyncTypes))
    {
        Platform::notifyItemChange(localFolder, MegaApi::STATE_NONE);
    }

    mSyncController.reset();

    UserAttributes::UserAttributesManager::instance().reset();

    closeDialogs();
    removeAllFinishedTransfers();
    clearViewedTransfers();

    if(mBlockingBatch.isValid())
    {
        mBlockingBatch.cancelTransfer();
    }

    delete mTransfersModel;
    mTransfersModel = nullptr;

    delete mStalledIssuesModel;
    mStalledIssuesModel = nullptr;

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
    mPricing.reset();
    mCurrency.reset();

    // Delete notifications stuff
    delete notificationsModel;
    notificationsModel = NULL;
    delete notificationsProxyModel;
    notificationsProxyModel = NULL;
    delete notificationsDelegate;
    notificationsDelegate = NULL;

    // Delete menus and menu items
    deleteMenu(initialTrayMenu);
    deleteMenu(infoDialogMenu);
    deleteMenu(guestMenu);
#ifdef _WIN32
    deleteMenu(windowsMenu);
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
}

void MegaApplication::onAboutClicked()
{
    if (appfinished)
    {
        return;
    }

    showChangeLog();
}

void MegaApplication::repositionInfoDialog()
{
    if (!infoDialog)
    {
        return;
    }

    int posx, posy;
    calculateInfoDialogCoordinates(infoDialog, &posx, &posy);

    fixMultiscreenResizeBug(posx, posy);

    MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, QString::fromUtf8("Moving Info Dialog to posx = %1, posy = %2")
                 .arg(posx)
                 .arg(posy)
                 .toUtf8().constData());

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

void MegaApplication::raiseInfoDialog()
{
    if (infoDialog)
    {
        infoDialog->show();
        infoDialog->updateDialogState();
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

    const bool transferQuotaWaitTimeExpired{transferOverQuotaWaitTimeExpiredReceived && !transferQuota->isOverQuota()};
    const bool loggedAndNotBandwidthOverquota{preferences && preferences->logged()};
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
                megaApi->sendEvent(AppStatsEvents::EVENT_MAIN_DIAL_WHILE_OVER_QUOTA,
                                   "Main dialog shown while overquota");
            }
            else if (storageState == MegaApi::STORAGE_STATE_ORANGE)
            {
                megaApi->sendEvent(AppStatsEvents::EVENT_MAIN_DIAL_WHILE_ALMOST_OVER_QUOTA,
                                   "Main dialog shown while almost overquota");
            }

            repositionInfoDialog();

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

    updateUserStats(false, true, false, true, USERSTATS_SHOWMAINDIALOG);
}

void MegaApplication::showInfoDialogNotifications()
{
    showInfoDialog();
    infoDialog->showNotifications();
}

void MegaApplication::calculateInfoDialogCoordinates(QDialog *dialog, int *posx, int *posy)
{
    if (appfinished)
    {
        return;
    }

    int xSign = 1;
    int ySign = 1;
    QPoint position;
    QRect screenGeometry;

    #ifdef __APPLE__
        QPoint positionTrayIcon;
        positionTrayIcon = trayIcon->geometry().topLeft();
    #endif

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
            otherInfo = QString::fromUtf8("dialog rect = %1").arg(RectToString(dialog->rect()));
            logInfoDialogCoordinates("screenGeometry", screenGeometry, otherInfo);

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

    #ifdef __APPLE__
        MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, QString::fromUtf8("Calculating Info Dialog coordinates. posTrayIcon = %1")
                     .arg(QString::fromUtf8("[%1,%2]").arg(positionTrayIcon.x()).arg(positionTrayIcon.y()))
                     .toUtf8().constData());
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
        *posy = screenGeometry.top();

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


                MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, QString::fromUtf8("Calculating Info Dialog coordinates. pabd.uEdge = %1, pabd.rc = %2")
                             .arg(pabd.uEdge)
                             .arg(QString::fromUtf8("[%1,%2,%3,%4]").arg(pabd.rc.left).arg(pabd.rc.top).arg(pabd.rc.right).arg(pabd.rc.bottom))
                             .toUtf8().constData());

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

    QString otherInfo = QString::fromUtf8("dialog rect = %1, posx = %2, posy = %3").arg(RectToString(dialog->rect())).arg(*posx).arg(*posy);
    logInfoDialogCoordinates("Final", screenGeometry, otherInfo);
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
        menu->deleteLater();
    }
}

void MegaApplication::startHttpServer()
{
    if (!httpServer)
    {
        httpServer = new HTTPServer(megaApi, Preferences::HTTP_PORT, false);
        ConnectServerSignals(httpServer);
        MegaApi::log(MegaApi::LOG_LEVEL_INFO, "Local HTTP server started");
    }
}

void MegaApplication::startHttpsServer()
{
    if (!httpsServer)
    {
        httpsServer = new HTTPServer(megaApi, Preferences::HTTPS_PORT, true);
        ConnectServerSignals(httpsServer);
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

bool MegaApplication::eventFilter(QObject *obj, QEvent *e)
{
    if (!appfinished && obj == infoDialogMenu)
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
    connect(infoDialog.data(), &InfoDialog::dismissStorageOverquota, this, &MegaApplication::onDismissStorageOverquota);
    connect(infoDialog.data(), &InfoDialog::transferOverquotaMsgVisibilityChange, transferQuota.get(), &TransferQuota::onTransferOverquotaVisibilityChange);
    connect(infoDialog.data(), &InfoDialog::almostTransferOverquotaMsgVisibilityChange, transferQuota.get(), &TransferQuota::onAlmostTransferOverquotaVisibilityChange);
    connect(infoDialog.data(), &InfoDialog::userActivity, this, &MegaApplication::registerUserActivity);
    connect(transferQuota.get(), &TransferQuota::sendState, infoDialog.data(), &InfoDialog::setBandwidthOverquotaState);
    connect(transferQuota.get(), &TransferQuota::overQuotaMessageNeedsToBeShown, infoDialog.data(), &InfoDialog::enableTransferOverquotaAlert);
    connect(transferQuota.get(), &TransferQuota::almostOverQuotaMessageNeedsToBeShown, infoDialog.data(), &InfoDialog::enableTransferAlmostOverquotaAlert);
    connect(infoDialog, SIGNAL(cancelScanning()), this, SLOT(cancelScanningStage()));
    connect(this, &MegaApplication::addBackup, infoDialog.data(), &InfoDialog::onAddBackup);
    scanStageController.updateReference(infoDialog);
}

QuotaState MegaApplication::getTransferQuotaState() const
{
     QuotaState quotaState (QuotaState::OK);

     if (transferQuota->isQuotaWarning())
     {
         quotaState = QuotaState::WARNING;
     }
     else if (transferQuota->isQuotaFull())
     {
         quotaState = QuotaState::FULL;
     }
     else if (transferQuota->isOverQuota())
     {
         quotaState = QuotaState::OVERQUOTA;
     }

     return quotaState;
}

std::shared_ptr<TransferQuota> MegaApplication::getTransferQuota() const
{
    return transferQuota;
}

int MegaApplication::getAppliedStorageState() const
{
    return appliedStorageState;
}

bool MegaApplication::isAppliedStorageOverquota() const
{
    return appliedStorageState == MegaApi::STORAGE_STATE_RED || appliedStorageState == MegaApi::STORAGE_STATE_PAYWALL;
}

std::shared_ptr<MegaPricing> MegaApplication::getPricing() const
{
    return mPricing;
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
    QIcon ic = QIcon(QString::fromUtf8("://images/icon_syncing_mac") +
                     QString::number(scanningAnimationIndex) + QString::fromUtf8(".png"));
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
        proxy.setPort(qint16(preferences->proxyPort()));
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

            QStringList parts = proxyURL.split(QString::fromUtf8("://"));
            if (parts.size() == 2 && parts[0].startsWith(QString::fromUtf8("socks")))
            {
                proxy.setType(QNetworkProxy::Socks5Proxy);
            }
            else
            {
                proxy.setType(QNetworkProxy::HttpProxy);
            }

            QStringList arguments = parts[parts.size()-1].split(QString::fromUtf8(":"));
            if (arguments.size() == 2)
            {
                proxy.setHostName(arguments[0]);
                proxy.setPort(quint16(arguments[1].toInt()));
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

void MegaApplication::loadSyncExclusionRules(QString email)
{
    assert(preferences->logged() || !email.isEmpty());

    // if not logged in & email provided, read old syncs from that user and load new-cache sync from prev session
    bool temporarilyLoggedPrefs = false;
    if (!preferences->logged() && !email.isEmpty())
    {
        temporarilyLoggedPrefs = preferences->enterUser(email);
        if (!temporarilyLoggedPrefs) // nothing to load
        {
            return;
        }

        preferences->loadExcludedSyncNames(); //to attend the corner case:
                  // comming from old versions that didn't include some defaults

    }
    assert(preferences->logged()); //At this point preferences should be logged, just because you enterUser() or it was already logged

    if (!preferences->logged())
    {
        return;
    }

    QStringList exclusions = preferences->getExcludedSyncNames();
    vector<string> vExclusions;
    for (int i = 0; i < exclusions.size(); i++)
    {
        vExclusions.push_back(exclusions[i].toUtf8().constData());
    }
    megaApi->setLegacyExcludedNames(&vExclusions);

    QStringList exclusionPaths = preferences->getExcludedSyncPaths();
    vector<string> vExclusionPaths;
    for (int i = 0; i < exclusionPaths.size(); i++)
    {
        vExclusionPaths.push_back(exclusionPaths[i].toUtf8().constData());
    }
    megaApi->setLegacyExcludedPaths(&vExclusionPaths);

    if (preferences->lowerSizeLimit())
    {
        megaApi->setLegacyExclusionLowerSizeLimit(computeExclusionSizeLimit(preferences->lowerSizeLimitValue(), preferences->lowerSizeLimitUnit()));
    }
    else
    {
        megaApi->setLegacyExclusionLowerSizeLimit(0);
    }

    if (preferences->upperSizeLimit())
    {
        megaApi->setLegacyExclusionUpperSizeLimit(computeExclusionSizeLimit(preferences->upperSizeLimitValue(), preferences->upperSizeLimitUnit()));
    }
    else
    {
        megaApi->setLegacyExclusionUpperSizeLimit(0);
    }


    if (temporarilyLoggedPrefs)
    {
        preferences->leaveUser();
    }
}

long long MegaApplication::computeExclusionSizeLimit(const long long sizeLimitValue, const int unit)
{
    const double sizeLimitPower = pow(static_cast<double>(1024), static_cast<double>(unit));
    return sizeLimitValue * static_cast<long long>(sizeLimitPower);
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
    const QStringList addressParts = ipAddress.toString().split(QChar::fromAscii('.'));
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
        addressParts.push_back(QString::fromUtf8("%1%2").arg(ipv6[baseI], 0, 16, QChar::fromAscii('0'))
                                                        .arg(ipv6[baseI+1], 0, 16, QChar::fromAscii('0')));
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

void MegaApplication::updateFileTransferBatchesAndUi(const QString& nodePath, BlockingBatch &batch)
{
    if(batch.isValid())
    {
        QString message = QString::fromUtf8("updateFileTransferBatchesAndUi");
        MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, message.toUtf8().constData());

        batch.onScanCompleted(nodePath);
        updateIfBlockingStageFinished(batch, false);
    }
}

void MegaApplication::updateFolderTransferBatchesAndUi(const QString& nodePath, BlockingBatch &batch, bool fromCancellation)
{
    if(batch.isValid())
    {
        QString message = QString::fromUtf8("updateFolderTransferBatchesAndUi");
        MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, message.toUtf8().constData());

        batch.onScanCompleted(nodePath);
        updateIfBlockingStageFinished(batch, fromCancellation);
    }
}

void MegaApplication::updateIfBlockingStageFinished(BlockingBatch &batch, bool fromCancellation)
{
    if (batch.isBlockingStageFinished() && (batch.isCancelled() || !isQueueProcessingOngoing()))
    {
        scanStageController.stopDelayedScanStage(fromCancellation);
        unblockBatch(batch);
    }
}

void MegaApplication::unblockBatch(BlockingBatch &batch)
{
    if (batch.hasCancelToken())
    {
        mUnblockedCancelTokens.push_back(batch.getCancelToken());
    }
    batch.setAsUnblocked();
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
#ifdef _WIN32
    if(updateAvailable && windowsUpdateAction)
    {
        windowsUpdateAction->setEnabled(enable);
    }
    windowsSettingsAction->setEnabled(enable);
    windowsImportLinksAction->setEnabled(enable);
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

void MegaApplication::updateFreedCancelToken(MegaTransfer* transfer)
{
    auto finder = [transfer](std::shared_ptr<MegaCancelToken> currentToken)
    {
        return (transfer->getCancelToken() == currentToken.get());
    };
    auto itToken = std::find_if(mUnblockedCancelTokens.begin(), mUnblockedCancelTokens.end(), finder);
    if (itToken != mUnblockedCancelTokens.end())
    {
        mUnblockedCancelTokens.erase(itToken);
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
    connect(server, SIGNAL(onLinkReceived(QString, QString)), this, SLOT(externalDownload(QString, QString)), Qt::QueuedConnection);
    connect(server, SIGNAL(onExternalDownloadRequested(QQueue<WrappedNode *>)), this, SLOT(externalDownload(QQueue<WrappedNode *>)));
    connect(server, SIGNAL(onExternalDownloadRequestFinished()), this, SLOT(processDownloads()), Qt::QueuedConnection);
    connect(server, SIGNAL(onExternalFileUploadRequested(qlonglong)), this, SLOT(externalFileUpload(qlonglong)), Qt::QueuedConnection);
    connect(server, SIGNAL(onExternalFolderUploadRequested(qlonglong)), this, SLOT(externalFolderUpload(qlonglong)), Qt::QueuedConnection);
    connect(server, SIGNAL(onExternalFolderSyncRequested(qlonglong)), this, SLOT(externalFolderSync(qlonglong)), Qt::QueuedConnection);
    connect(server, SIGNAL(onExternalOpenTransferManagerRequested(int)), this, SLOT(externalOpenTransferManager(int)), Qt::QueuedConnection);
    connect(server, SIGNAL(onExternalShowInFolderRequested(QString)), this, SLOT(openFolderPath(QString)), Qt::QueuedConnection);
    connect(server, &HTTPServer::onExternalAddBackup, this, &MegaApplication::externalAddBackup, Qt::QueuedConnection);
}

QString MegaApplication::RectToString(const QRect &rect)
{
    return QString::fromUtf8("[%1,%2,%3,%4]").arg(rect.x()).arg(rect.y()).arg(rect.width()).arg(rect.height());
}

void MegaApplication::fixMultiscreenResizeBug(int &posX, int &posY)
{
    // An issue occurred with certain multiscreen setup that caused Qt to missplace the info dialog.
    // This works around that by ensuring infoDialog does not get incorrectly resized. in which case,
    // it is reverted to the correct size.

    infoDialog->ensurePolished();
    auto initialDialogWidth  = infoDialog->width();
    auto initialDialogHeight = infoDialog->height();
    QTimer::singleShot(1, infoDialog, [this, initialDialogWidth, initialDialogHeight, posX, posY](){
        if (infoDialog->width() > initialDialogWidth || infoDialog->height() > initialDialogHeight) //miss scaling detected
        {
            MegaApi::log(MegaApi::LOG_LEVEL_ERROR,
                         QString::fromUtf8("A dialog. New size = %1,%2. should be %3,%4 ")
                         .arg(infoDialog->width()).arg(infoDialog->height()).arg(initialDialogWidth).arg(initialDialogHeight)
                         .toUtf8().constData());

            infoDialog->resize(initialDialogWidth,initialDialogHeight);

            auto iDPos = infoDialog->pos();
            if (iDPos.x() != posX || iDPos.y() != posY )
            {
                MegaApi::log(MegaApi::LOG_LEVEL_ERROR,
                             QString::fromUtf8("Missplaced info dialog. New pos = %1,%2. should be %3,%4 ")
                             .arg(iDPos.x()).arg(iDPos.y()).arg(posX).arg(posY)
                             .toUtf8().constData());
                infoDialog->move(posX, posY);

                QTimer::singleShot(1, this, [this, initialDialogWidth, initialDialogHeight](){
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
}

void MegaApplication::logInfoDialogCoordinates(const char *message, const QRect &screenGeometry, const QString &otherInformation)
{
    MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, QString::fromUtf8("Calculating Info Dialog coordinates. %1: valid = %2, geom = %3, %4")
                 .arg(QString::fromUtf8(message))
                 .arg(screenGeometry.isValid())
                 .arg(RectToString(screenGeometry))
                 .arg(otherInformation)
                 .toUtf8().constData());
}

bool MegaApplication::dontAskForExitConfirmation(bool force)
{
    return force || !megaApi->isLoggedIn() || mTransfersModel->hasActiveTransfers() == 0;
}

void MegaApplication::exitApplication()
{
    reboot = false;
    trayIcon->hide();
    closeDialogs();
    QApplication::exit();
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

void MegaApplication::updateMetadata(TransferMetaData *data, const QString &filePath)
{
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

    QFileInfo filePathInfo(filePath);
    if (filePathInfo.isDir())
    {
        data->totalFolders++;
    }
    else
    {
        data->totalFiles++;
    }
}

bool MegaApplication::isQueueProcessingOngoing()
{
    return mProcessingUploadQueue || downloader->isQueueProcessingOngoing();
}

void MegaApplication::onFolderTransferUpdate(FolderTransferUpdateEvent event)
{
    if (appfinished)
    {
        return;
    }

    transferProgressController.update(event);
    if (event.stage >= MegaTransfer::STAGE_TRANSFERRING_FILES)
    {
        transferProgressController.stopUiUpdating();
        updateFolderTransferBatchesAndUi(event.transferName, mBlockingBatch, false);
        logBatchStatus("onTransferUpdate");
    }
}

void MegaApplication::onNotificationProcessed()
{
    --mProcessingShellNotifications;
    if (mProcessingShellNotifications <= 0)
    {
        emit shellNotificationsProcessed();
    }
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
            for (auto it = downloadQueue.begin(); it != downloadQueue.end(); ++it)
            {
                HTTPServer::onTransferDataUpdate((*it)->getMegaNode()->getHandle(), MegaTransfer::STATE_CANCELLED, 0, 0, 0, QString());
            }

            for (auto it = pendingLinks.begin(); it != pendingLinks.end(); it++)
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
            for (auto it = downloadQueue.begin(); it != downloadQueue.end(); ++it)
            {
                HTTPServer::onTransferDataUpdate((*it)->getMegaNode()->getHandle(), MegaTransfer::STATE_CANCELLED, 0, 0, 0, QString());
            }

            for (auto it = pendingLinks.begin(); it != pendingLinks.end(); it++)
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
    mRubbishNode.reset();
    mVaultNode.reset();
    mFetchingNodes = false;
    mQueringWhyAmIBlocked = false;
    whyamiblockedPeriodicPetition = false;
    megaApi->logout(true, nullptr);
    megaApiFolders->setAccountAuth(nullptr);
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

    if (mOsNotifications)
    {
#ifdef __APPLE__
        if (infoDialog && infoDialog->isVisible())
        {
            infoDialog->hide();
        }
#endif
        lastTrayMessage = message;
        mOsNotifications->sendInfoNotification(title, message);
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

    if (mOsNotifications)
    {
        lastTrayMessage = message;
        mOsNotifications->sendWarningNotification(title, message);
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

    if (mOsNotifications)
    {
        lastTrayMessage = message;
        mOsNotifications->sendInfoNotification(title, message);
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
        proxy.setPort(qint16(preferences->proxyPort()));
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
                proxy.setPort(qint16(arguments[1].toInt()));
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
        QMutableMapIterator<QString, std::chrono::system_clock::time_point> it(mOpenUrlsClusterTs);
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
        const auto megaUrlIterator = mOpenUrlsClusterTs.find(url.fragment());
        const auto itemFound(megaUrlIterator != mOpenUrlsClusterTs.end());
        if(itemFound)
        {
            MegaApi::log(MegaApi::LOG_LEVEL_INFO, "Session transfer to URL already managed");
            return;
        }

        mOpenUrlsClusterTs.insert(url.fragment(), std::chrono::system_clock::now());
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

    UserAttributes::UserAttributesManager::instance().reset();
}

void MegaApplication::clearViewedTransfers()
{
    nUnviewedTransfers = 0;
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

    //Its a single shot timer, delete it forever
    firstTransferTimer->deleteLater();
    firstTransferTimer = nullptr;

    auto TransfersStats = mTransfersModel->getTransfersCount();

    if (TransfersStats.pendingDownloads)
    {
        mThreadPool->push([=]()
        {//thread pool function

            MegaTransfer *nextTransfer = megaApi->getFirstTransfer(MegaTransfer::TYPE_DOWNLOAD);

            Utilities::queueFunctionInAppThread([=]()
            {//queued function

                if (nextTransfer)
                {
                    onTransferUpdate(megaApi, nextTransfer);
                    delete nextTransfer;
                }
            });//end of queued function

        });// end of thread pool function
    }

    if (TransfersStats.pendingUploads)
    {
        mThreadPool->push([=]()
        {//thread pool function

            MegaTransfer *nextTransfer = megaApi->getFirstTransfer(MegaTransfer::TYPE_UPLOAD);
            if (nextTransfer)
            {
                Utilities::queueFunctionInAppThread([=]()
                {//queued function

                    onTransferUpdate(megaApi, nextTransfer);
                    delete nextTransfer;

                });//end of queued function
            }
        });// end of thread pool function
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
                        if((int)majorVersion < 14) // Older versions from 10.10 (yosemite)
                        {
                            isOSdeprecated = true;
                        }
                    }
                }
            }
        }
#endif

#ifdef WIN32
#pragma warning(push)
#pragma warning(disable: 4996) // declared deprecated
        DWORD dwVersion = GetVersion();
#pragma warning(pop)
        DWORD dwMajorVersion = (DWORD)(LOBYTE(LOWORD(dwVersion)));
        DWORD dwMinorVersion = (DWORD) (HIBYTE(LOWORD(dwVersion)));
        isOSdeprecated = (dwMajorVersion < 6) || ((dwMajorVersion == 6) && (dwMinorVersion == 0));
#endif

        if (isOSdeprecated)
        {
            QMegaMessageBox::warning(nullptr, tr("MEGAsync"),
                                 tr("Please consider updating your operating system.") + QString::fromUtf8("\n")
#ifdef __APPLE__
                                 + tr("MEGAsync will continue to work, however updates will no longer be supported for versions prior to OS X Yosemite soon.")
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

void MegaApplication::notifyChangeToAllFolders()
{
    for (auto localFolder : model->getLocalFolders(SyncInfo::AllHandledSyncTypes))
    {
        ++mProcessingShellNotifications;
        Platform::notifyItemChange(localFolder, MegaApi::STATE_NONE);
    }
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
        if (preferences->isNotificationEnabled(Preferences::NotificationsTypes::INFO_MESSAGES))
        {
            TransferNotificationMessageBuilder messageBuilder(data);
            QString message = messageBuilder.buildMessage();
            if (mOsNotifications && !message.isEmpty())
            {
                preferences->setLastTransferNotificationTimestamp();
                mOsNotifications->sendFinishedTransferNotification(messageBuilder.buildTitle(), message, data->localPath);
            }
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
        Platform::showInFolder(localPath);
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
    }
}

void MegaApplication::onSyncStateChanged(std::shared_ptr<SyncSettings>)
{
    createAppMenus();
}

void MegaApplication::onSyncDeleted(std::shared_ptr<SyncSettings>)
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
    std::shared_ptr<int>oldCacheSyncsCount(new int(oldCachedSyncs.size()));
    if (*oldCacheSyncsCount > 0)
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
                                     new MegaListenerFuncExecuter(true, [this, osd, oldCacheSyncsCount, needsMigratingFromOldSession, email](MegaApi*,  MegaRequest* request, MegaError* e)
        {

            if (e->getErrorCode() == MegaError::API_OK)
            {
                //preload the model with the restored configuration: that includes info that the SDK does not handle (e.g: syncID)
                model->pickInfoFromOldSync(osd, request->getParentHandle(), needsMigratingFromOldSession);
                preferences->removeOldCachedSync(osd.mPos, email);
            }
            else
            {
                MegaApi::log(MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("Failed to copy sync %1: %2").arg(osd.mLocalFolder).arg(QString::fromUtf8(e->getErrorString())).toUtf8().constData());
            }

            --*oldCacheSyncsCount;
            if (*oldCacheSyncsCount == 0)//All syncs copied to sdk, proceed with fetchnodes
            {
                megaApi->fetchNodes();
            }
         }));
    }

    if (*oldCacheSyncsCount == 0)//No syncs to be copied to sdk, proceed with fetchnodes
    {
        megaApi->fetchNodes();
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
    //Send updated statics to the information dialog
    if (infoDialog)
    {
        infoDialog->updateDialogState();
    }

    auto TransfersStats = mTransfersModel->getTransfersCount();
    //If there are no pending transfers, reset the statics and update the state of the tray icon
    if (!TransfersStats.pendingDownloads
            && !TransfersStats.pendingUploads)
    {
        onGlobalSyncStateChanged(megaApi);
    }
}

void MegaApplication::fetchNodes(QString email)
{
    assert(!mFetchingNodes);
    mFetchingNodes = true;

    // We need to load exclusions and migrate sync configurations from MEGAsync held cache, to SDK's
    // prior fetching nodes (when the SDK will resume syncing)

    // If we are loging into a new session of an account previously used in MEGAsync,
    // we will use the previous configurations stored in that user preferences
    // However, there is a case in which we are not able to do so at this point:
    // we don't know the user email.
    // That should only happen when trying to resume a session (using the session id stored in general preferences)
    // that didn't complete a fetch nodes (i.e. does not have preferences logged).
    // that can happen for blocked accounts.
    // Fortunately, the SDK can help us get the email of the session
    bool needFindingOutEmail = !preferences->logged() && email.isEmpty();

    auto loadMigrateAndFetchNodes = [this](const QString &email)
    {
        if (!preferences->logged() && email.isEmpty()) // I still couldn't get the the email: won't be able to access user settings
        {
            megaApi->fetchNodes();
        }
        else
        {
            loadSyncExclusionRules(email);
            migrateSyncConfToSdk(email); // this will produce the fetch nodes once done
        }
    };

    if (!needFindingOutEmail)
    {
        loadMigrateAndFetchNodes(email);
    }
    else // we will ask the SDK the email
    {
        megaApi->getUserEmail(megaApi->getMyUserHandleBinary(),new MegaListenerFuncExecuter(true, [loadMigrateAndFetchNodes](MegaApi*,  MegaRequest* request, MegaError* e) {
              QString email;

              if (e->getErrorCode() == API_OK)
              {
                  auto emailFromRequest = request->getEmail();
                  if (emailFromRequest)
                  {
                      email = QString::fromUtf8(emailFromRequest);
                  }
              }

              // in any case, proceed:
              loadMigrateAndFetchNodes(email);
        }));

    }

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

void MegaApplication::addRecentFile(QString/* fileName*/, long long/* fileHandle*/, QString/* localPath*/, QString/* nodeKey*/)
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
            displayedMenu = guestMenu;
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
    Utilities::openUrl(QUrl(webUrl));
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
    importDialog = new ImportMegaLinksDialog(linkProcessor);
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
    uploadActionClickedFromWindow(nullptr);
}

void MegaApplication::uploadActionClickedFromWindow(QWidget* openFrom)
{
    if (appfinished)
    {
        return;
    }

    const bool storageIsOverQuota(storageState == MegaApi::STORAGE_STATE_RED || storageState == MegaApi::STORAGE_STATE_PAYWALL);
    if(storageIsOverQuota)
    {
        if(OverQuotaDialog::showDialog(OverQuotaDialogType::STORAGE_UPLOAD,openFrom))
        {
            return;
        }
    }

    #ifdef __APPLE__
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
    #endif

    if (multiUploadFileDialog)
    {
        multiUploadFileDialog->activateWindow();
        multiUploadFileDialog->raise();
        return;
    }

    QString  defaultFolderPath;
    QStringList paths = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
    if (paths.size())
    {
        defaultFolderPath = paths.at(0);
    }

    multiUploadFileDialog = new MultiQFileDialog(openFrom,
           QCoreApplication::translate("ShellExtension", "Upload to MEGA"),
           defaultFolderPath, true);

    if (!multiUploadFileDialog)
    {
        return;
    }

    if (multiUploadFileDialog->exec() == QDialog::Accepted)
    {
        QStringList files = multiUploadFileDialog->selectedFiles();
        if (files.size() > 0)
        {
            QQueue<QString> qFiles;
            foreach(QString file, files)
            {
                qFiles.append(file);
            }
            shellUpload(qFiles);
        }
    }

    delete multiUploadFileDialog;
    multiUploadFileDialog = nullptr;
}

bool MegaApplication::showSyncOverquotaDialog()
{
    const bool storageFull(storageState == MegaApi::STORAGE_STATE_RED);

    if(storageFull)
    {
        if(OverQuotaDialog::showDialog(OverQuotaDialogType::STORAGE_SYNCS))
            return false;

        return true;
    }

    if(transferQuota->isOverQuota())
    {
        if(OverQuotaDialog::showDialog(OverQuotaDialogType::BANDWITH_SYNC))
            return false;

        return true;
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

    downloadNodeSelector = new NodeSelector(NodeSelectorTreeViewWidget::DOWNLOAD_SELECT, NULL);
    downloadNodeSelector->setSelectedNodeHandle();
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

    QList<MegaHandle> selectedMegaFolderHandles = downloadNodeSelector->getMultiSelectionNodeHandle();
    delete downloadNodeSelector;
    downloadNodeSelector = nullptr;
    foreach(auto& selectedMegaFolderHandle, selectedMegaFolderHandles)
    {
        MegaNode *selectedNode = megaApi->getNodeByHandle(selectedMegaFolderHandle);
        if (selectedNode)
        {
            downloadQueue.append(new WrappedNode(WrappedNode::TransferOrigin::FROM_APP, selectedNode));
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

    streamSelector = new StreamingFromMegaDialog(megaApi, megaApiFolders);
    connect(transferQuota.get(), &TransferQuota::waitTimeIsOver, streamSelector.data(), &StreamingFromMegaDialog::updateStreamingState);
    streamSelector->show();
}

void MegaApplication::transferManagerActionClicked(int tab)
{
    if (appfinished)
    {
        return;
    }

    if(!mTransferManager)
    {
        createTransferManagerDialog(static_cast<TransfersWidget::TM_TAB>(tab));
    }
    else
    {
        mTransferManager->toggleTab(tab);
    }

    mTransferManagerGeometryRetainer.showDialog(mTransferManager);
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

        if (preferences && preferences->logged() && getRootNode() && !blockState)
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
    if (appfinished)
    {
        return;
    }

    if (!uploadQueue.size())
    {
        return;
    }

    if (blockState)
    {
        if (infoDialog)
        {
            raiseInfoDialog();
        }
        else
        {
            // No infodialog available (logged with session locked),
            // shows verifyemaildialog instead
            showVerifyAccountInfo();
        }
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
        Platform::showBackgroundWindow(uploadFolderSelector);
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
    Platform::execBackgroundWindow(uploadFolderSelector);
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

        //Do not use deleteLater to remove the delegate listener at the moment
        delete uploadFolderSelector;

        processUploadQueue(nodeHandle);
    }
    //If the dialog is rejected, cancel uploads
    else
    {
        delete uploadFolderSelector;
        uploadQueue.clear();
    }

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

    if (blockState)
    {
        if (infoDialog)
        {
            raiseInfoDialog();
        }
        else
        {
            // No infodialog available (logged with session locked),
            // shows verifyemaildialog instead
            showVerifyAccountInfo();
        }
        return;
    }

    if (!preferences->logged())
    {
        openInfoWizard();
        return;
    }

    if (downloadFolderSelector)
    {
        Platform::showBackgroundWindow(downloadFolderSelector);
        return;
    }

    QString defaultPath = preferences->downloadFolder();
    if (preferences->hasDefaultDownloadFolder()
            && QDir(defaultPath).exists())
    {
        QString qFilePath = QDir::fromNativeSeparators(defaultPath); // QFile always wants `/` as separator
        QTemporaryFile *test = new QTemporaryFile(qFilePath + QDir::separator());
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
    Platform::execBackgroundWindow(downloadFolderSelector);

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

        delete downloadFolderSelector;
        processDownloadQueue(path);
    }
    else
    {
        delete downloadFolderSelector;

        QQueue<WrappedNode *>::iterator it;
        for (it = downloadQueue.begin(); it != downloadQueue.end(); ++it)
        {
            HTTPServer::onTransferDataUpdate((*it)->getMegaNode()->getHandle(), MegaTransfer::STATE_CANCELLED, 0, 0, 0, QString());
        }

        //If the dialog is rejected, cancel uploads
        qDeleteAll(downloadQueue);
        downloadQueue.clear();
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
    if (!localPath.startsWith(QByteArray((const char *)(void*)L"\\\\", 4)))
    {
        localPath.insert(0, QByteArray((const char *)(void*)L"\\\\?\\", 8));
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
    const char* handleBase64Pointer{MegaApi::handleToBase64(handle)};
    const QString handleArgument{QString::fromUtf8(handleBase64Pointer)};
    delete [] handleBase64Pointer;
    const QString versionsArgument{versions ? QString::fromUtf8("/versions") : QString::fromUtf8("")};
    const QString url{QString::fromUtf8("fm%1/%2").arg(versionsArgument).arg(handleArgument)};
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

void MegaApplication::externalDownload(QQueue<WrappedNode *> newDownloadQueue)
{
    if (appfinished)
    {
        return;
    }

    downloadQueue.append(newDownloadQueue);
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
        Platform::showBackgroundWindow(folderUploadSelector);
        return;
    }

    fileUploadTarget = targetFolder;
    if (fileUploadSelector)
    {
        Platform::showBackgroundWindow(fileUploadSelector);
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
        for (const auto& path : paths)
        {
            files++;
            startUpload(path, target, nullptr);
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
        NodeCount nodeCount = countFilesAndFolders(paths);

        processUploads(paths);
        HTTPServer::onUploadSelectionAccepted(nodeCount.files, nodeCount.folders);
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

    const bool upgradingDissmised{showSyncOverquotaDialog()};
    if (infoDialog && upgradingDissmised)
    {
        infoDialog->addSync(targetFolder);
    }
}

void MegaApplication::externalAddBackup()
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

    emit addBackup();
}

void MegaApplication::externalOpenTransferManager(int tab)
{
    if (appfinished || !infoDialog)
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

    downloadQueue.append(new WrappedNode(WrappedNode::TransferOrigin::FROM_APP, node));
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
    QString linkForClipboard(links.join(QLatin1Char('\n')));
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
        const auto syncSettings (model->getAllSyncSettings());
        auto firstActiveSyncSetting (std::find_if(syncSettings.cbegin(), syncSettings.cend(),
                                                  [](std::shared_ptr<SyncSettings> s)
                                     {return s->getRunState() == MegaSync::RUNSTATE_RUNNING;}));
        if (firstActiveSyncSetting != syncSettings.cend())
        {
            infoDialogTimer->stop();
            infoDialog->hide();
            QString localFolderPath = (*firstActiveSyncSetting)->getLocalFolder();
            if (!localFolderPath.isEmpty())
            {
                Utilities::openUrl(QUrl::fromLocalFile(localFolderPath));
            }
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

    if (lastTrayMessage == tr("A new version of MEGAsync is available. Click on this message to install it"))
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
    infoWizard->activateWindow();
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
        proxyOnly = !getRootNode() || !preferences->logged() || blockState;
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
        //If the dialog is active
        if (settingsDialog->isVisible())
        {
            if (proxyOnly)
            {
                settingsDialog->showGuestMode();
            }
            else
            {
                settingsDialog->openSettingsTab(tab);
            }
            settingsDialog->setProxyOnly(proxyOnly);
            //and visible -> show it
            settingsDialog->show();
            settingsDialog->activateWindow();
            settingsDialog->raise();
            return;
        }

        //Otherwise, delete it
        delete settingsDialog;
        settingsDialog = nullptr;
    }

    //Show a new settings dialog
    settingsDialog = new SettingsDialog(this, proxyOnly);
    settingsDialog->setUpdateAvailable(updateAvailable);
    settingsDialog->setModal(false);
    connect(settingsDialog, SIGNAL(userActivity()), this, SLOT(registerUserActivity()));
    if (proxyOnly)
        settingsDialog->showGuestMode();
    else
        settingsDialog->openSettingsTab(tab);
    settingsDialog->show();
}

void MegaApplication::openSettingsAddSync(MegaHandle megaFolderHandle)
{
    openSettings(SettingsDialog::SYNCS_TAB);
    settingsDialog->addSyncFolder(megaFolderHandle);
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
        QList<QAction *> actions = initialTrayMenu->actions();
        for (int i = 0; i < actions.size(); i++)
        {
            initialTrayMenu->removeAction(actions[i]);
        }
    }
#ifndef _WIN32 // win32 needs to recreate menu to fix scaling qt issue
    else
#endif
    {
        initialTrayMenu->deleteLater();
        initialTrayMenu = new QMenu();
    }

    if (guestSettingsAction)
    {
        guestSettingsAction->deleteLater();
        guestSettingsAction = nullptr;
    }
    guestSettingsAction = new QAction(QCoreApplication::translate("Platform", Platform::settingsString), this);

    // When triggered, open "Settings" window. As the user is not logged in, it
    // will only show proxy settings.
    connect(guestSettingsAction, &QAction::triggered, this, &MegaApplication::openSettings);

    if (initialExitAction)
    {
        initialExitAction->deleteLater();
        initialExitAction = nullptr;
    }
    initialExitAction = new QAction(QCoreApplication::translate("Platform", Platform::exitString), this);
    connect(initialExitAction, &QAction::triggered, this, &MegaApplication::tryExitApplication);

    initialTrayMenu->addAction(guestSettingsAction);
    initialTrayMenu->addAction(initialExitAction);

    // On Linux, add a "Show Status" action, which opens the Info Dialog.
    if (isLinux && infoDialog)
    {
        // Create action
        if (showStatusAction)
        {
            showStatusAction->deleteLater();
            showStatusAction = nullptr;
        }
        showStatusAction = new QAction(tr("Show status"), this);
        connect(showStatusAction, SIGNAL(triggered()), this, SLOT(showInfoDialog()));

        initialTrayMenu->insertAction(guestSettingsAction, showStatusAction);
    }
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
        windowsMenu->deleteLater();
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

    recreateAction(&windowsExitAction, QCoreApplication::translate("Platform", Platform::exitString),
                   &MegaApplication::tryExitApplication);
    recreateAction(&windowsSettingsAction, QCoreApplication::translate("Platform", Platform::settingsString),
                   &MegaApplication::openSettings);
    recreateAction(&windowsImportLinksAction, tr("Open links"), &MegaApplication::importLinks);
    recreateAction(&windowsUploadAction, tr("Upload"), &MegaApplication::uploadActionClicked);
    recreateAction(&windowsDownloadAction, tr("Download"), &MegaApplication::downloadActionClicked);
    recreateAction(&windowsStreamAction, tr("Stream"), &MegaApplication::streamActionClicked);
    recreateAction(&windowsTransferManagerAction, tr("Transfer manager"),
                   &MegaApplication::transferManagerActionClicked);

    bool windowsUpdateActionEnabled = true;
    if (windowsUpdateAction)
    {
        windowsUpdateActionEnabled = windowsUpdateAction->isEnabled();
        windowsUpdateAction->deleteLater();
        windowsUpdateAction = NULL;
    }

    if(windowsAboutAction)
    {
        windowsAboutAction->deleteLater();
        windowsAboutAction = NULL;
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

        windowsMenu->addAction(windowsAboutAction);
        connect(windowsAboutAction, &QAction::triggered, this, &MegaApplication::onAboutClicked);
    }

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

    // Info Dialog overflow menu
    if (infoDialogMenu)
    {
        QList<QAction*> actions = infoDialogMenu->actions();
        for (int i = 0; i < actions.size(); i++)
        {
            infoDialogMenu->removeAction(actions[i]);
        }
    }
#ifndef _WIN32 // win32 needs to recreate menu to fix scaling qt issue
    else
#endif
    {
        infoDialogMenu->deleteLater();
        infoDialogMenu = new QMenu();
        Platform::initMenu(infoDialogMenu);

        //Highlight menu entry on mouse over
        connect(infoDialogMenu, SIGNAL(hovered(QAction*)), this, SLOT(highLightMenuEntry(QAction*)), Qt::QueuedConnection);

        //Hide highlighted menu entry when mouse over
        infoDialogMenu->installEventFilter(this);
    }

    recreateMenuAction(&exitAction, QCoreApplication::translate("Platform", Platform::exitString),
                       "://images/ico_quit.png", &MegaApplication::tryExitApplication);
    recreateMenuAction(&settingsAction, QCoreApplication::translate("Platform", Platform::settingsString),
                       "://images/ico_preferences.png", &MegaApplication::openSettings);
    recreateMenuAction(&myCloudAction, tr("Cloud drive"), "://images/ico-cloud-drive.png", &MegaApplication::goToMyCloud);

    bool previousEnabledState = true;
    if (!mSyncs2waysMenu)
    {
        mSyncs2waysMenu = new SyncsMenu(MegaSync::TYPE_TWOWAY, infoDialog);
        connect(mSyncs2waysMenu.data(), &SyncsMenu::addSync,
                infoDialog.data(), &InfoDialog::onAddSync);
        mSyncs2waysMenu->setEnabled(exitAction->isEnabled());
    }

    if (!mBackupsMenu)
    {
        mBackupsMenu = new SyncsMenu(MegaSync::TYPE_BACKUP, infoDialog);
        connect(mBackupsMenu.data(), &SyncsMenu::addSync,
                infoDialog.data(), &InfoDialog::onAddSync);
        mBackupsMenu->setEnabled(exitAction->isEnabled());
    }

    recreateMenuAction(&importLinksAction, tr("Open links"), "://images/ico_Import_links.png", &MegaApplication::importLinks);
    recreateMenuAction(&uploadAction, tr("Upload"), "://images/ico_upload.png", &MegaApplication::uploadActionClicked);
    recreateMenuAction(&downloadAction, tr("Download"), "://images/ico_download.png", &MegaApplication::downloadActionClicked);
    recreateMenuAction(&streamAction, tr("Stream"), "://images/ico_stream.png", &MegaApplication::streamActionClicked);


    previousEnabledState = true;
    if (updateAction)
    {
        previousEnabledState = updateAction->isEnabled();
        updateAction->deleteLater();
        updateAction = NULL;
    }

    if(aboutAction)
    {
        aboutAction->deleteLater();
        aboutAction = NULL;
    }

    if (updateAvailable)
    {
        updateAction = new MenuItemAction(tr("Install update"), QIcon(QString::fromUtf8("://images/ico_about_MEGA.png")), true);
        updateAction->setEnabled(previousEnabledState);
        connect(updateAction, &QAction::triggered, this, &MegaApplication::onInstallUpdateClicked, Qt::QueuedConnection);

        infoDialogMenu->addAction(updateAction);
    }
    else
    {
        aboutAction = new MenuItemAction(tr("About MEGAsync"), QIcon(QString::fromUtf8("://images/ico_about_MEGA.png")), true);
        connect(aboutAction, &QAction::triggered, this, &MegaApplication::onAboutClicked, Qt::QueuedConnection);

        infoDialogMenu->addAction(aboutAction);
    }


    infoDialogMenu->addAction(myCloudAction);
    infoDialogMenu->addSeparator();
    infoDialogMenu->addAction(mSyncs2waysMenu->getAction().get());
    infoDialogMenu->addAction(mBackupsMenu->getAction().get());
    infoDialogMenu->addAction(importLinksAction);
    infoDialogMenu->addAction(uploadAction);
    infoDialogMenu->addAction(downloadAction);
    infoDialogMenu->addAction(streamAction);
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
        guestMenu->deleteLater();
        guestMenu = new QMenu();
        Platform::initMenu(guestMenu);
    }

    if (exitActionGuest)
    {
        exitActionGuest->deleteLater();
        exitActionGuest = NULL;
    }

    exitActionGuest = new MenuItemAction(QCoreApplication::translate("Platform", Platform::exitString), QIcon(QString::fromUtf8("://images/ico_quit.png")));

    connect(exitActionGuest, &QAction::triggered, this, &MegaApplication::tryExitApplication);

    if (updateActionGuest)
    {
        updateActionGuest->deleteLater();
        updateActionGuest = NULL;
    }

    if (updateAvailable)
    {
        updateActionGuest = new MenuItemAction(tr("Install update"), QIcon(QString::fromUtf8("://images/ico_about_MEGA.png")));
        connect(updateActionGuest, &QAction::triggered, this, &MegaApplication::onInstallUpdateClicked);
    }
    else
    {
        updateActionGuest = new MenuItemAction(tr("About MEGAsync"), QIcon(QString::fromUtf8("://images/ico_about_MEGA.png")));
        connect(updateActionGuest, &QAction::triggered, this, &MegaApplication::onAboutClicked);
    }


    if (settingsActionGuest)
    {
        settingsActionGuest->deleteLater();
        settingsActionGuest = NULL;
    }
    settingsActionGuest = new MenuItemAction(QCoreApplication::translate("Platform", Platform::settingsString), QIcon(QString::fromUtf8("://images/ico_preferences.png")));

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

    notifyStorageObservers(); //Ideally this should be the only call here

    transferQuota->refreshOverQuotaDialogDetails();

    if (storageOverquotaDialog)
    {
        storageOverquotaDialog->refreshStorageDetails();
    }
}

void MegaApplication::manageBusinessStatus(int64_t event)
{
    switch (event)
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
                    Utilities::openUrl(QUrl(url));
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
                    Utilities::openUrl(QUrl(url));
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
                                .replace(QString::fromUtf8("[/A]"), QString::fromUtf8("</span>")) + QString::fromUtf8("\n"));

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

    businessStatus = static_cast<int>(event);
    if (preferences->logged())
    {
        preferences->setBusinessState(businessStatus);
    }
}

void MegaApplication::onEvent(MegaApi*, MegaEvent* event)
{
    DeferPreferencesSyncForScope deferrer(this);
    const int eventNumber = static_cast<int>(event->getNumber());

    if (event->getType() == MegaEvent::EVENT_CHANGE_TO_HTTPS)
    {
        preferences->setUseHttpsOnly(true);
    }
    else if (event->getType() == MegaEvent::EVENT_SYNCS_RESTORED)
    {
        if (SyncInfo::instance()->getNumSyncedFolders(SyncInfo::AllHandledSyncTypes) > 0)
        {
            Platform::notifyAllSyncFoldersAdded();
        }
    }
    else if (event->getType() == MegaEvent::EVENT_SYNCS_DISABLED && event->getNumber() != MegaSync::Error::LOGGED_OUT)
    {
        auto syncsUnattended = model->getUnattendedDisabledSyncs(MegaSync::TYPE_TWOWAY);
        auto backupsUnattended = model->getUnattendedDisabledSyncs(MegaSync::TYPE_BACKUP);

        if((syncsUnattended.size() + backupsUnattended.size()) == 1)
        {
            if(syncsUnattended.size() == 1)
            {
                showSingleSyncDisabledNotification(model->getSyncSettingByTag(*syncsUnattended.begin()));
            }
            else if(backupsUnattended.size() == 1)
            {
                showSingleSyncDisabledNotification(model->getSyncSettingByTag(*backupsUnattended.begin()));
            }
        }
        else if(!syncsUnattended.isEmpty() || !backupsUnattended.isEmpty())
        {
            if (!syncsUnattended.isEmpty()
                    && !backupsUnattended.isEmpty())
            {
                showErrorMessage(tr("Your syncs and backups have been disabled").append(QString::fromUtf8(": "))
                                 .append(QCoreApplication::translate("MegaSyncError", MegaSync::getMegaSyncErrorCode(eventNumber))));
            }
            else if (!backupsUnattended.isEmpty())
            {
                showErrorMessage(tr("Your backups have been disabled").append(QString::fromUtf8(": "))
                                 .append(QCoreApplication::translate("MegaSyncError", MegaSync::getMegaSyncErrorCode(eventNumber))));
            }
            else
            {
                showErrorMessage(tr("Your syncs have been disabled").append(QString::fromUtf8(": "))
                                 .append(QCoreApplication::translate("MegaSyncError", MegaSync::getMegaSyncErrorCode(eventNumber))));
            }
        }
    }
    else if (event->getType() == MegaEvent::EVENT_ACCOUNT_BLOCKED)
    {
        switch (event->getNumber())
        {
            case MegaApi::ACCOUNT_BLOCKED_VERIFICATION_EMAIL:
            case MegaApi::ACCOUNT_BLOCKED_VERIFICATION_SMS:
            {
                blockState = eventNumber;
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
            applyStorageState(eventNumber);
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
        manageBusinessStatus(event->getNumber());
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
            showErrorMessage(tr("Error getting link: ") + QString::fromUtf8(" ") + QCoreApplication::translate("MegaError", e->getErrorString()));
        }

        break;
    }
    case MegaRequest::TYPE_GET_PRICING:
    {
        if (e->getErrorCode() == MegaError::API_OK)
        {
            MegaPricing* pricing (request->getPricing());
            MegaCurrency* currency (request->getCurrency());

            if (pricing && currency)
            {
                mPricing.reset(pricing);
                mCurrency.reset(currency);

                transferQuota->setOverQuotaDialogPricing(mPricing, mCurrency);

                if (storageOverquotaDialog)
                {
                    storageOverquotaDialog->setPricing(mPricing, mCurrency);
                }
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
                //Typical case: Connecting from a public wifi when the wifi sends you to a landing page
                //SDK cannot connect through SSL securely and asks MEGA Desktop to log out

                //In previous versions, the user was asked to continue with a warning about a MITM risk.
                //One of the options was disabling the public key pinning to continue working as usual
                //This option was to risky and the solution taken was silently retry reconnection

                // Retry while enforcing key pinning silently
                megaApi->retryPendingConnections();
                break;
            }

            if (errorCode == MegaError::API_ESID)
            {
                QMegaMessageBox::information(nullptr, QString::fromUtf8("MEGAsync"), tr("You have been logged out on this computer from another location"));
            }
            else if (errorCode == MegaError::API_ESSL)
            {
                QMegaMessageBox::critical(nullptr, QString::fromUtf8("MEGAsync"),
                                      tr("Our SSL key can't be verified. You could be affected by a man-in-the-middle attack or your antivirus software could be intercepting your communications and causing this problem. Please disable it and try again.")
                                       + QString::fromUtf8(" (Issuer: %1)").arg(QString::fromUtf8(request->getText() ? request->getText() : "Unknown")));
            }
            else if (errorCode != MegaError::API_EACCESS)
            {
                QMegaMessageBox::information(nullptr, QString::fromUtf8("MEGAsync"), tr("You have been logged out because of this error: %1")
                                         .arg(QCoreApplication::translate("MegaError", e->getErrorString())));
            }
            unlink();
        }

        //Check for any sync disabled by logout to warn user on next login with user&password
        const auto syncSettings (model->getAllSyncSettings());
        auto isErrorLoggedOut = [](std::shared_ptr<SyncSettings> s) {return s->getError() == MegaSync::LOGGED_OUT;};
        if (std::any_of(syncSettings.cbegin(), syncSettings.cend(), isErrorLoggedOut))
        {
            preferences->setNotifyDisabledSyncsOnLogin(true);
        }

        model->reset();
        mTransfersModel->resetModel();

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
             });
        });
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
                megaApi->sendEvent(AppStatsEvents::EVENT_LOCAL_SSL_CERT_RENEWED,
                                   "Local SSL certificate renewed");
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
            getRootNode(true); //TODO: move this to thread pool, notice that mRootNode is used below
            getVaultNode(true);
            getRubbishNode(true);

            preferences->setAccountStateInGeneral(Preferences::STATE_FETCHNODES_OK);
            preferences->setNeedsFetchNodesInGeneral(false);

            if (!mRootNode)
            {
                QMegaMessageBox::warning(nullptr, tr("Error"), tr("Unable to get the filesystem.\n"
                                                       "Please, try again. If the problem persists "
                                                       "please contact bug@mega.co.nz"), QMessageBox::Ok);

                setupWizardFinished(QDialog::Rejected);
                MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, "Setting isCrashed true: !mRootNode (fetch node callback)");
                preferences->setCrashed(true);
                rebootApplication(false);
                break;
            }

            std::unique_ptr<char[]> email(megaApi->getMyEmail());
            bool logged = preferences->logged();
            bool firstTime = !logged && email && !preferences->hasEmail(QString::fromUtf8(email.get()));
            if (!logged) //session resumed from general storage (or logged in via user/pass)
            {
                if (firstTime)
                {
                    showSetupWizard(SetupWizard::PAGE_MODE);
                }
                else
                {
                    // We will proceed with a new login
                    preferences->setEmailAndGeneralSettings(QString::fromUtf8(email.get()));
                    model->rewriteSyncSettings(); //write sync settings into user's preferences

                    if (infoDialog && infoDialog->isVisible())
                    {
                        infoDialog->hide();
                    }

                    loggedIn(true);
                    emit closeSetupWizard();
                }
            }
            else // session resumed regularly
            {
                loggedIn(false);
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

        // We need to be both logged AND have fetched the nodes to continue
        if (mFetchingNodes || !preferences->logged())
        {
            break;
        }

        if (e->getErrorCode() != MegaError::API_OK)
        {
            break;
        }


        auto root = getRootNode();
        auto vault = getVaultNode();
        auto rubbish = getRubbishNode();

        if (!root || !vault || !rubbish)
        {
            preferences->setCrashed(true);
            MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, "Setting isCrashed true: !root || !inbox || !rubbish (account details callback)");
            break;
        }

        //Account details retrieved, update the preferences and the information dialog
        shared_ptr<MegaAccountDetails> details(request->getMegaAccountDetails());

        mThreadPool->push([=]()
        {//thread pool function
        shared_ptr<MegaNodeList> inShares(megaApi->getInShares());

        if (!inShares)
        {
            MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, "Setting isCrashed true: !inShares (account details callback)");
            preferences->setCrashed(true);
            return;
        }

        Utilities::queueFunctionInAppThread([=]()
        {//queued function

        if (pro)
        {
            preferences->setAccountType(details->getProLevel());
            if (details->getProLevel() != Preferences::ACCOUNT_TYPE_FREE)
            {
                if (details->getProExpiration() && preferences->proExpirityTime() != details->getProExpiration())
                {
                    preferences->setProExpirityTime(details->getProExpiration());
                    proExpirityTimer.stop();
                    const long long interval = qMax(0LL, details->getProExpiration() * 1000 - QDateTime::currentMSecsSinceEpoch());
                    proExpirityTimer.setInterval(static_cast<int>(interval));
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
                megaApi->sendEvent(AppStatsEvents::EVENT_RED_LIGHT_USED_STORAGE_MISMATCH,
                                   "Red light does not match used storage");
                preferences->setUsedStorage(preferences->totalStorage());
            }
            else
            {
                preferences->setUsedStorage(receivedStorageSum);
            }

            MegaHandle rootHandle = root->getHandle();
            MegaHandle vaultHandle = vault->getHandle();
            MegaHandle rubbishHandle = rubbish->getHandle();

            // For versions, match the webclient by only counting the user's own nodes.  Versions in inshares are not cleared by 'clear versions'
            // Also the no-parameter getVersionStorageUsed() double counts the versions in outshares.  Inshare storage count should include versions.
            preferences->setVersionsStorage(details->getVersionStorageUsed(rootHandle)
                                          + details->getVersionStorageUsed(vaultHandle)
                                          + details->getVersionStorageUsed(rubbishHandle));

            preferences->setCloudDriveStorage(details->getStorageUsed(rootHandle));
            preferences->setCloudDriveFiles(details->getNumFiles(rootHandle));
            preferences->setCloudDriveFolders(details->getNumFolders(rootHandle));

            preferences->setVaultStorage(details->getStorageUsed(vaultHandle));
            preferences->setVaultFiles(details->getNumFiles(vaultHandle));
            preferences->setVaultFolders(details->getNumFolders(vaultHandle));

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

        const bool proUserIsOverquota (megaApi->getBandwidthOverquotaDelay() &&
                    preferences->accountType() != Preferences::ACCOUNT_TYPE_FREE);
        if (proUserIsOverquota)
        {
            transferQuota->setOverQuota(std::chrono::seconds(megaApi->getBandwidthOverquotaDelay()));
        }

        preferences->setTotalBandwidth(details->getTransferMax());
        preferences->setBandwidthInterval(details->getTemporalBandwidthInterval());
        preferences->setUsedBandwidth(details->getTransferUsed());

        preferences->setTemporalBandwidthInterval(details->getTemporalBandwidthInterval());
        preferences->setTemporalBandwidth(details->getTemporalBandwidth());
        preferences->setTemporalBandwidthValid(details->isTemporalBandwidthValid());

        notifyBandwidthObservers();

        if (preferences->accountType() != Preferences::ACCOUNT_TYPE_FREE)
        {
            transferQuota->updateQuotaState();
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
            storageOverquotaDialog->refreshStorageDetails();
        }

        });//end of queued function

        });// end of thread pool function
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
            megaApi->sendEvent(AppStatsEvents::EVENT_SYNC_ADD_FAIL_API_EACCESS,
                               "Sync addition fails with API_EACCESS"); //this would enforce a fetchNodes in the past
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
            megaApi->sendEvent(AppStatsEvents::EVENT_PRO_REDIRECT, "Redirection to PRO");
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

                downloadQueue.append(new WrappedNode(WrappedNode::TransferOrigin::FROM_APP, node));
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
            case AppStatsEvents::EVENT_1ST_START:
                preferences->setFirstStartDone();
                break;
            case AppStatsEvents::EVENT_1ST_SYNC:
                preferences->setFirstSyncDone();
                break;
            case AppStatsEvents::EVENT_1ST_SYNCED_FILE:
                preferences->setFirstFileSynced();
                break;
            case AppStatsEvents::EVENT_1ST_BACKUP:
                preferences->setFirstBackupDone();
                break;
            case AppStatsEvents::EVENT_1ST_BACKED_UP_FILE:
                preferences->setFirstFileBackedUp();
                break;
            case AppStatsEvents::EVENT_1ST_WEBCLIENT_DL:
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

    if(!transfer->isSyncTransfer() && !transfer->isBackupTransfer())
    {
        updateFileTransferBatchesAndUi(Utilities::getNodePath(transfer), mBlockingBatch);
        logBatchStatus("onTransferStart");
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


    onTransferUpdate(api, transfer);
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
    bool isFileTransfer = (folderTransferTag == 0);
    bool isFolderTransfer = (folderTransferTag == -1);
    if (isFileTransfer || isFolderTransfer)
    {

        if(!transfer->isSyncTransfer() && !transfer->isBackupTransfer())
        {
            if(mBlockingBatch.isValid())
            {
                mBlockingBatch.onTransferFinished(Utilities::getNodePath(transfer));
                updateIfBlockingStageFinished(mBlockingBatch, mBlockingBatch.hasCancelToken());
                updateFreedCancelToken(transfer);
            }

            logBatchStatus("onTransferFinish");
        }

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
                        isFileTransfer ? data->transfersFileOK++ : data->transfersFolderOK++;
                    }
                }

                // update the path before showing the notification, in case the destination file was renamed
                data->localPath = QString::fromUtf8(transfer->getPath());

#ifdef WIN32 // this should really be done in a function, not all over the code...
                if (data->localPath.startsWith(QString::fromUtf8("\\\\?\\")))
                {
                    data->localPath = data->localPath.mid(4);
                }
#endif

                data->pendingTransfers--;
                showNotificationFinishedTransfers(notificationId);
            }
        }
    }

    if (transfer->isFolderTransfer() && !mBlockingBatch.hasCancelToken())
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
            megaApi->sendEvent(AppStatsEvents::EVENT_DUP_FINISHED_TRSF,
                               QString::fromUtf8("Duplicated finished transfer: %1").arg(QString::number(transfer->getTag())).toUtf8().constData());
            removeFinishedTransfer(transfer->getTag());
        }

        finishedTransfers.insert(transfer->getTag(), t);
        finishedTransferOrder.push_back(t);

        if (!mTransferManager)
        {
            completedTabActive = false;
        }

        if (!completedTabActive)
        {
            ++nUnviewedTransfers;
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

    if (finishedTransferOrder.size() > (int)Preferences::MAX_COMPLETED_ITEMS)
    {
        removeFinishedTransfer(finishedTransferOrder.first()->getTag());
    }

    if (e->getErrorCode() == MegaError::API_EBUSINESSPASTDUE
            && (!lastTsBusinessWarning || (QDateTime::currentMSecsSinceEpoch() - lastTsBusinessWarning) > 3000))//Notify only once within last five seconds
    {
        lastTsBusinessWarning = QDateTime::currentMSecsSinceEpoch();
        mOsNotifications->sendBusinessWarningNotification(businessStatus);
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
        if (localPath.startsWith(QString::fromUtf8("\\\\?\\")))
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

    // Check if we have ot send a EVENT_1ST_***_FILE
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
                megaApi->sendEvent(AppStatsEvents::EVENT_1ST_SYNCED_FILE,
                                   "MEGAsync first synced file");
            }
            mIsFirstFileTwoWaySynced = true;
            break;
        }
        case MegaSync::SyncType::TYPE_BACKUP:
        {
            if (!mIsFirstFileBackedUp && !preferences->isFirstFileBackedUp())
            {
                megaApi->sendEvent(AppStatsEvents::EVENT_1ST_BACKED_UP_FILE,
                                   "MEGAsync first backed-up file");
            }
            mIsFirstFileBackedUp = true;
            break;
        }
        }
    }

    if (firstTransferTimer && !firstTransferTimer->isActive())
    {
        firstTransferTimer->start();
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

    DeferPreferencesSyncForScope deferrer(this);

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

    if (firstTransferTimer && !firstTransferTimer->isActive())
    {
        firstTransferTimer->start();
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

    onTransferUpdate(api, transfer);

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
            const auto waitTime = std::chrono::seconds(e->getValue());
            preferences->clearTemporalBandwidth();
            megaApi->getPricing();
            updateUserStats(false, true, true, true, USERSTATS_TRANSFERTEMPERROR);  // get udpated transfer quota (also pro status in case out of quota is due to account paid period expiry)
            transferQuota->setOverQuota(waitTime);
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

void MegaApplication::showAddSyncError(int errorCode, QString localpath, QString /*remotePath*/)
{
    if (errorCode != MegaError::API_OK)
    {
        QMegaMessageBox::critical(nullptr, tr("Error adding sync"),
                                  tr("This sync can't be added: %1. Reason: %2").arg(localpath)
                                  .arg( errorCode > 0 ? QCoreApplication::translate("MegaSyncError", MegaSync::getMegaSyncErrorCode(errorCode))
                                                      : QCoreApplication::translate("MegaError", MegaError::getErrorString(errorCode))));
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

void MegaApplication::pushToThreadPool(std::function<void()> functor)
{
    mThreadPool->push(std::move(functor));
}

void MegaApplication::onUserAlertsUpdate(MegaApi *api, MegaUserAlertList *list)
{
    Q_UNUSED(api)

    if (appfinished)
    {
        return;
    }

    // if we have a list, we don't need to query megaApi for it and block the sdk mutex, we do this
    // synchronously, since we are not copying the list, and we need to process it before it goes out of scope.
    bool doSynchronously{list != NULL};

    if (doSynchronously)
    {
        populateUserAlerts(list, true);
    }
    else
    {
        auto funcToThreadPool = [this]()
        { //thread pool function
            MegaUserAlertList *theList;
            theList = megaApi->getUserAlerts();
            //queued function
            Utilities::queueFunctionInAppThread([this, theList]() { populateUserAlerts(theList, false); });
        }; // end of thread pool function
        mThreadPool->push(funcToThreadPool);
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
    MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, "Setting isCrashed true: onReloadNeeded");
    preferences->setCrashed(true);
}

void MegaApplication::onGlobalSyncStateChangedTimeout()
{
    onGlobalSyncStateChangedImpl(NULL, true);
}

void MegaApplication::onGlobalSyncStateChanged(MegaApi* api)
{
    onGlobalSyncStateChangedImpl(api, false);
}

void MegaApplication::onGlobalSyncStateChangedImpl(MegaApi *, bool timeout)
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
        mThreadPool->push([this]() {

        auto model = getTransfersModel();
        if (!megaApi || !model)
        {
            return;
        }

        indexing = megaApi->isScanning();
        waiting = megaApi->isWaiting() || megaApi->isSyncStalled();
        syncing = megaApi->isSyncing();
        syncStalled = megaApi->isSyncStalled();
        auto transferCount = model->getTransfersCount();
        transferring = transferCount.pendingUploads || transferCount.pendingDownloads;

        Utilities::queueFunctionInAppThread([=](){

            if (!infoDialog)
            {
                return;
            }

            int pendingUploads = transferCount.pendingUploads;
            int pendingDownloads = transferCount.pendingDownloads;

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
            });

       });
    }

    MegaApi::log(MegaApi::LOG_LEVEL_INFO, QString::fromUtf8("Current state. Paused = %1 Indexing = %2 Waiting = %3 Syncing = %4 Stalled = %5")
                 .arg(paused).arg(indexing).arg(waiting).arg(syncing).arg(syncStalled).toUtf8().constData());

    updateTrayIcon();
}

static std::map<MegaHandle, unique_ptr<MegaSync>> knownSyncStates;


void MegaApplication::onSyncStateChanged(MegaApi *api, MegaSync *sync)
{
    if (appfinished)
    {
        return;
    }

    if (sync->getRunState() == MegaSync::RUNSTATE_DISABLED)
    {
        if (sync->getError())
        {
            model->addUnattendedDisabledSync(sync->getBackupId(),
                                             static_cast<MegaSync::SyncType>(sync->getType()));
        }

        showSingleSyncDisabledNotification(model->getSyncSettingByTag(sync->getBackupId()));
    }

    model->updateSyncSettings(sync); //Note, we are not updating the remote sync path
    // we asume that cannot change for existing syncs.

    onGlobalSyncStateChanged(api);
}

void MegaApplication::onSyncStatsUpdated(MegaApi *api, MegaSyncStats* stats)
{
    if (appfinished)
    {
        return;
    }

    model->updateSyncStats(stats); //Note, we are not updating the remote sync path
    // we asume that cannot change for existing syncs.

    onGlobalSyncStateChanged(api);
}

void MegaApplication::onSyncFileStateChanged(MegaApi *, MegaSync *, string *localPath, int newState)
{
    if (appfinished)
    {
        return;
    }

    Platform::notifySyncFileChange(localPath, newState);
}

void MegaApplication::showSingleSyncDisabledNotification(std::shared_ptr<SyncSettings> syncSetting)
{
    if(syncSetting)
    {
        auto errorCode (syncSetting->getError());
        auto syncType (syncSetting->getType());
        QString syncName (syncSetting->name());

        if (syncType == MegaSync::TYPE_TWOWAY)
        {
            MegaApi::log(MegaApi::LOG_LEVEL_ERROR,
                         QString::fromUtf8("Sync \"%1\" Path: %2 disabled: %3")
                         .arg(syncName, syncSetting->getLocalFolder(), QString::number(errorCode)).toUtf8().constData());

            if (errorCode != MegaSync::NO_SYNC_ERROR
                     && errorCode != MegaSync::Error::LOGGED_OUT)
            {
                switch(errorCode)
                {
                    case MegaSync::Error::NO_SYNC_ERROR:
                    {
                        assert(false && "unexpected no error after onSyncAdded failed");
                        return;
                    }
                    case MegaSync::Error::LOCAL_PATH_UNAVAILABLE:
                    {
                        showErrorMessage(tr("Your sync \"%1\" has been disabled because the local folder doesn't exist")
                                         .arg(syncName));
                        break;
                    }
                    case MegaSync::Error::REMOTE_NODE_NOT_FOUND:
                    {
                        showErrorMessage(tr("Your sync \"%1\" has been disabled because the remote folder doesn't exist")
                                         .arg(syncName));
                        break;
                    }
                    case MegaSync::Error::VBOXSHAREDFOLDER_UNSUPPORTED:
                    {
                        showErrorMessage(tr("Your sync \"%1\" has been disabled because the synchronization of VirtualBox shared folders is not supported due to deficiencies in that filesystem.")
                                         .arg(syncName));
                        break;
                    }
                    case MegaSync::Error::REMOTE_NODE_MOVED_TO_RUBBISH:
                    {
                        showErrorMessage(tr("Your sync \"%1\" has been disabled because the remote folder is in the rubbish bin")
                                         .arg(syncName));
                        break;
                    }
                    case MegaSync::Error::SHARE_NON_FULL_ACCESS:
                    {
                        showErrorMessage(tr("Your sync \"%1\" has been disabled. The remote folder (or part of it) doesn't have full access")
                                         .arg(syncName));
                        break;
                    }
                    case MegaSync::Error::LOCAL_FILESYSTEM_MISMATCH:
                    {
                        showErrorMessage(tr("Your sync \"%1\" has been disabled because the local folder has changed")
                                         .arg(syncName));
                        break;
                    }
                    case MegaSync::Error::PUT_NODES_ERROR:
                    default:
                    {
                        showErrorMessage(tr("Your sync \"%1\" has been disabled. Reason: %2").arg(syncName,
                                                                                                  QCoreApplication::translate("MegaSyncError", MegaSync::getMegaSyncErrorCode(errorCode))));
                        break;
                    }
                }
            }
        }
        else if (syncType == MegaSync::TYPE_BACKUP)
        {
            MegaApi::log(MegaApi::LOG_LEVEL_ERROR,
                         QString::fromUtf8("Backup \"%1\" Path: %2 disabled: %3")
                         .arg(syncName, syncSetting->getLocalFolder(), QString::number(errorCode)).toUtf8().constData());
            if (errorCode != MegaSync::NO_SYNC_ERROR
                     && errorCode != MegaSync::Error::LOGGED_OUT)
            {
                switch(errorCode)
                {
                    case MegaSync::Error::NO_SYNC_ERROR:
                    {
                        assert(false && "unexpected no error after onSyncAdded failed");
                        return;
                    }
                    case MegaSync::Error::LOCAL_PATH_UNAVAILABLE:
                    {
                        showErrorMessage(tr("Your backup \"%1\" has been disabled because the local folder doesn't exist")
                                         .arg(syncName));
                        break;
                    }
                    case MegaSync::Error::REMOTE_NODE_NOT_FOUND:
                    {
                        // We don't want to show a notification here because the removal of the remote
                        // folder means that the backup has been deleted from the Backups Center
                        break;
                    }
                    case MegaSync::Error::VBOXSHAREDFOLDER_UNSUPPORTED:
                    {
                        showErrorMessage(tr("Your backup \"%1\" has been disabled because the synchronization of VirtualBox shared folders is not supported due to deficiencies in that filesystem.")
                                         .arg(syncName));
                        break;
                    }
                    case MegaSync::Error::REMOTE_NODE_MOVED_TO_RUBBISH:
                    {
                        showErrorMessage(tr("Your backup \"%1\" has been disabled because the remote folder is in the rubbish bin")
                                         .arg(syncName));
                        break;
                    }
                    case MegaSync::Error::SHARE_NON_FULL_ACCESS:
                    {
                        showErrorMessage(tr("Your backup \"%1\" has been disabled. The remote folder (or part of it) doesn't have full access")
                                         .arg(syncName));
                        break;
                    }
                    case MegaSync::Error::LOCAL_FILESYSTEM_MISMATCH:
                    {
                        showErrorMessage(tr("Your backup \"%1\" has been disabled because the local folder has changed")
                                         .arg(syncName));
                        break;
                    }
                    case MegaSync::Error::BACKUP_MODIFIED:
                    {
                        showErrorMessage(tr("Your backup \"%1\" has been disabled because the remote folder has changed")
                                         .arg(syncName));
                        break;
                    }
                    case MegaSync::Error::PUT_NODES_ERROR:
                    default:
                    {
                        showErrorMessage(tr("Your backup \"%1\" has been disabled. Reason: %2").arg(syncName,
                                                                                                    QCoreApplication::translate("MegaSyncError", MegaSync::getMegaSyncErrorCode(errorCode))));
                        break;
                    }
                }
            }
        }
        else
        {
            MegaApi::log(MegaApi::LOG_LEVEL_ERROR,
                         QString::fromLatin1("Unknown type of sync: %1")
                         .arg(syncType).toUtf8().constData());
        }
    }
}

void MegaApplication::onSyncAdded(MegaApi *api, MegaSync *sync)
{
    auto syncSetting = model->updateSyncSettings(sync);

    onGlobalSyncStateChanged(api);
}

void MegaApplication::onSyncDeleted(MegaApi *api, MegaSync *sync)
{
    if (appfinished || !sync)
    {
        return;
    }

    model->removeSyncedFolderByBackupId(sync->getBackupId());

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
    QTMegaListener::onEvent(api, e);
}

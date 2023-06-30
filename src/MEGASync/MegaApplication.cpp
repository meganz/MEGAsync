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
#include "TransferMetaData.h"
#include "DuplicatedNodeDialogs/DuplicatedNodeDialog.h"
#include "PlatformStrings.h"
#include "UserAttributesManager.h"
#include "UserAttributesRequests/FullName.h"
#include "UserAttributesRequests/Avatar.h"
#include "UserAttributesRequests/DeviceName.h"
#include "UserAttributesRequests/MyBackupsHandle.h"
#include "syncs/gui/SyncsMenu.h"
#include "TextDecorator.h"

#include "qml/QmlDialog.h"
#include "qml/QmlDialogWrapper.h"
#include "qml/QmlClipboard.h"
#include "onboarding/ChooseFolder.h"
#include "onboarding/Onboarding.h"
#include "onboarding/AccountInfoData.h"
#include "onboarding/BackupsModel.h"

#include <QQmlApplicationEngine>
#include "DialogOpener.h"
#include "PowerOptions.h"
#include "DateTimeFormatter.h"

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

#ifdef Q_OS_MACX
    #include "platform/macx/PlatformImplementation.h"
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
    mDisableGfx (false),
    mEngine(new QQmlEngine(this))
{

#if defined Q_OS_MACX && !defined QT_DEBUG
    if (!getenv("MEGA_DISABLE_RUN_MAC_RESTRICTION"))
    {
        QString path = appBundlePath();
        if (path.compare(QStringLiteral("/Applications/MEGAsync.app")))
        {
            QMegaMessageBox::MessageBoxInfo msgInfo;
            msgInfo.title = QMegaMessageBox::errorTitle();
            msgInfo.text = QCoreApplication::translate("MegaSyncError", "You can't run MEGA Desktop App from this location. Move it into the Applications folder then run it.");
            msgInfo.buttons = QMessageBox::Ok;
            msgInfo.finishFunc = [this](QPointer<QMessageBox>)
            {
                ::exit(0);
            };
            QMegaMessageBox::information(msgInfo);
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
        QTextStream(stdout) << getMEGAString() << " v" << Preferences::VERSION_STRING << " (" << Preferences::SDK_ID << ")" << endl;
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
    httpsServer = nullptr;
    exportOps = 0;
    infoDialog = nullptr;
    blockState = MegaApi::ACCOUNT_NOT_BLOCKED;
    blockStateSet = false;
    mSetupWizard = nullptr;
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
    updatingSSLcert = false;
    lastSSLcertUpdate = 0;
    mTransfersModel = nullptr;

    notificationsModel = nullptr;
    notificationsProxyModel = nullptr;
    notificationsDelegate = nullptr;

    context = new QObject(this);

#ifdef _WIN32
    windowsMenu = nullptr;
    windowsExitAction = nullptr;
    windowsUpdateAction = nullptr;
    windowsAboutAction = nullptr;
    windowsImportLinksAction = nullptr;
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
    myCloudAction = nullptr;
    waiting = false;
    updated = false;
    syncing = false;
    transferring = false;
    checkupdate = false;
    updateAction = nullptr;
    aboutAction = nullptr;
    updateActionGuest = nullptr;
    showStatusAction = nullptr;
    updateBlocked = false;
    updateThread = nullptr;
    updateTask = nullptr;
    mPricing.reset();
    mCurrency.reset();
    mStorageOverquotaDialog = nullptr;
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
    scanningTimer = nullptr;
#endif

    mDisableGfx = args.contains(QLatin1String("--nogfx")) || args.contains(QLatin1String("/nogfx"));
    mFolderTransferListener = std::make_shared<FolderTransferListener>();

    connect(mFolderTransferListener.get(), &FolderTransferListener::folderTransferUpdated,
            this, &MegaApplication::onFolderTransferUpdate);

    connect(&scanStageController, &ScanStageController::enableTransferActions,
            this, &MegaApplication::enableTransferActions);

    connect(&transferProgressController, &BlockingStageProgressController::updateUi,
            &scanStageController, &ScanStageController::onFolderTransferUpdate);

    setAttribute(Qt::AA_DisableWindowContextHelpButton);
}

MegaApplication::~MegaApplication()
{
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

    registerCommonQMLElements();

    preferences = Preferences::instance();
    connect(preferences.get(), SIGNAL(stateChanged()), this, SLOT(changeState()));
    connect(preferences.get(), SIGNAL(updated(int)), this, SLOT(showUpdatedMessage(int)),
            Qt::DirectConnection); // Use direct connection to make sure 'updated' and 'prevVersions' are set as needed
    preferences->initialize(dataPath);

    model = SyncInfo::instance();

    connect(model, SIGNAL(syncStateChanged(std::shared_ptr<SyncSettings>)),
            this, SLOT(onSyncStateChanged(std::shared_ptr<SyncSettings>)));
    connect(model, SIGNAL(syncRemoved(std::shared_ptr<SyncSettings>)),
            this, SLOT(onSyncDeleted(std::shared_ptr<SyncSettings>)));

    if (preferences->error())
    {
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("Encountered corrupt prefrences.").toUtf8().constData());

        QMegaMessageBox::MessageBoxInfo msgInfo;
        msgInfo.title = getMEGAString();
        msgInfo.text = tr("Your config is corrupt, please start over");
        msgInfo.enqueue = true;
        QMegaMessageBox::critical(msgInfo);
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

    // TODO: This is legacy behavior and should be deleted when SRW is merged
    // We also handle here the case where the user changed the exclusions with a
    // version not using the mustDeleteSdkCacheAtStartup flag, did not restart
    // from the settings dialog to activate the new exclusions, and the app got (auto) updated.
    if (preferences->mustDeleteSdkCacheAtStartup()
        || (prevVersion <= Preferences::LAST_VERSION_WITHOUT_deleteSdkCacheAtStartup_FLAG
            && preferences->isCrashed()))
    {
        preferences->setDeleteSdkCacheAtStartup(false);
        MegaApi::log(MegaApi::LOG_LEVEL_WARNING, "deleteSdkCacheAtStartup is true: force reload");
        deleteSdkCache();
    }

    QString basePath = QDir::toNativeSeparators(dataPath + QString::fromUtf8("/"));
    megaApi = new MegaApi(Preferences::CLIENT_KEY, basePath.toUtf8().constData(), Preferences::USER_AGENT.toUtf8().constData());
    megaApi->disableGfxFeatures(mDisableGfx);

    megaApiFolders = new MegaApi(Preferences::CLIENT_KEY, basePath.toUtf8().constData(), Preferences::USER_AGENT.toUtf8().constData());
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

        QMegaMessageBox::MessageBoxInfo msgInfo;
        msgInfo.title = MegaSyncApp->getMEGAString();
        msgInfo.text = QString::fromUtf8("API URL changed to ")+ apiURL;
        msgInfo.enqueue = true;
        QMegaMessageBox::warning(msgInfo);

        QString baseURL = settings.value(QString::fromUtf8("baseurl"), Preferences::BASE_URL).toString();
        Preferences::setBaseUrl(baseURL);
        if (baseURL.compare(QString::fromUtf8("https://mega.nz")))
        {
            msgInfo.text = QString::fromUtf8("base URL changed to ") + Preferences::BASE_URL;
            QMegaMessageBox::warning(msgInfo);
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
    uploader = new MegaUploader(megaApi, mFolderTransferListener);
    downloader = new MegaDownloader(megaApi, mFolderTransferListener);
    connect(uploader, &MegaUploader::startingTransfers, this, &MegaApplication::startingUpload);
    connect(downloader, &MegaDownloader::startingTransfers,
            &scanStageController, &ScanStageController::startDelayedScanStage);

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

    if (preferences->isCrashed())
    {       
        preferences->setCrashed(false);
        QStringList reports = CrashHandler::instance()->getPendingCrashReports();
        if (reports.size())
        {
            QPointer<CrashReportDialog> crashDialog = new CrashReportDialog(reports.join(QString::fromUtf8("------------------------------\n")));
            DialogOpener::showDialog(crashDialog, [reports, crashDialog, this]
            {
                if (crashDialog->result() == QDialog::Accepted)
                {
                    applyProxySettings();
                    CrashHandler::instance()->sendPendingCrashReports(crashDialog->getUserMessage());
                    if (crashDialog->sendLogs())
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
                    QMegaMessageBox::MessageBoxInfo msgInfo;
                    msgInfo.title = MegaSyncApp->getMEGAString();
                    msgInfo.text = tr("Thank you for your collaboration");
                    msgInfo.enqueue = true;
                    QMegaMessageBox::information(msgInfo);
#endif
                }
            });
        }
    }

    mTransferQuota = std::make_shared<TransferQuota>(mOsNotifications);
    connect(mTransferQuota.get(), &TransferQuota::waitTimeIsOver, this, &MegaApplication::updateStatesAfterTransferOverQuotaTimeHasExpired);

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

    connect(Platform::getInstance()->getShellNotifier().get(), &AbstractShellNotifier::shellNotificationProcessed,
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
                mTransferQuota->isOverQuota() ||
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

    QString tooltip = QString::fromUtf8("%1 %2\n%3").arg(QString::fromUtf8("MEGA")).arg(Preferences::VERSION_STRING).arg(tooltipState);

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
    mTransferQuota->reset();
    transferOverQuotaWaitTimeExpiredReceived = false;
    updateTrayIconMenu();

    if(notificationsModel) notificationsModel->deleteLater();
    notificationsModel = nullptr;
    if (notificationsProxyModel) notificationsProxyModel->deleteLater();
    notificationsProxyModel = nullptr;
    if (notificationsDelegate) notificationsDelegate->deleteLater();
    notificationsDelegate = nullptr;

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
        Platform::getInstance()->enableTrayIcon(QFileInfo(MegaApplication::applicationFilePath()).fileName());
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
        Platform::getInstance()->reloadFileManagerExtension();
    }

    applyProxySettings();
    Platform::getInstance()->startShellDispatcher(this);
#ifdef Q_OS_MACX
    auto current = QOperatingSystemVersion::current();
    if (current > QOperatingSystemVersion::OSXMavericks) //FinderSync API support from 10.10+
    {
        if (!preferences->isOneTimeActionDone(Preferences::ONE_TIME_ACTION_ACTIVE_FINDER_EXT))
        {
            MegaApi::log(MegaApi::LOG_LEVEL_INFO, "MEGA Finder Sync added to system database and enabled");
            Platform::getInstance()->addFileManagerExtensionToSystem();
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
            megaApi->sendEvent(AppStatsEvents::EVENT_UPDATE, "MEGAsync update", false, nullptr);
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


        if (!preferences->isFirstStartDone())
        {
            megaApi->sendEvent(AppStatsEvents::EVENT_1ST_START, "MEGAsync first start", false, nullptr);
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
            megaApi->sendEvent(AppStatsEvents::EVENT_UPDATE, "MEGAsync update", false, nullptr);
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

    //DialogOpener::removeDialogByClass<QmlDialogWrapper<Onboarding>>();

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

            QMegaMessageBox::MessageBoxInfo msgInfo;
            msgInfo.title = QMegaMessageBox::errorTitle();
            msgInfo.text = tr("Error adding %1:").arg(name)
                                     + QString::fromLatin1("\n")
                    + msg;
            QMegaMessageBox::warning(msgInfo);
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

    if (mSettingsDialog)
    {
        mSettingsDialog->setProxyOnly(false);
    }

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
            checkSystemTray();

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
            message = tr("Some syncs and backups have been disabled. Go to settings to enable them again.");
        }
        else if (haveBackups)
        {
            settingsTabToOpen = SettingsDialog::BACKUP_TAB;
            syncsTypesToDismiss = {MegaSync::TYPE_BACKUP};
            message = tr("One or more backups have been disabled. Go to settings to enable them again.");
        }
        else if (haveSyncs)
        {
            syncsTypesToDismiss = {MegaSync::TYPE_TWOWAY};
            message = tr("One or more syncs have been disabled. Go to settings to enable them again.");
        }

        // Display the message if it has been set
        if (!message.isEmpty())
        {
            QMegaMessageBox::MessageBoxInfo msgInfo;
            msgInfo.title = QCoreApplication::applicationName();
            msgInfo.text = message;
            msgInfo.buttons = QMessageBox::Yes | QMessageBox::No;
            QMap<QMessageBox::Button, QString> textsByButton;
            textsByButton.insert(QMessageBox::Yes, tr("Open settings"));
            textsByButton.insert(QMessageBox::No, tr("Dismiss"));
            msgInfo.buttonsText = textsByButton;
            msgInfo.defaultButton = QMessageBox::No;
            msgInfo.finishFunc = [this, settingsTabToOpen](QPointer<QMessageBox> msg){
                if(msg->result() == QMessageBox::Yes)
            {
                openSettings(settingsTabToOpen);
            }
            };
            QMegaMessageBox::warning(msgInfo);
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

void MegaApplication::checkSystemTray()
{
    if (!QSystemTrayIcon::isSystemTrayAvailable())
    {
        if (!preferences->isOneTimeActionDone(Preferences::ONE_TIME_ACTION_NO_SYSTRAY_AVAILABLE))
        {
            QMegaMessageBox::MessageBoxInfo msgInfo;
            msgInfo.title = getMEGAString();
            msgInfo.text = tr("Could not find a system tray to place MEGAsync tray icon. "
                              "MEGAsync is intended to be used with a system tray icon but it can work fine without it. "
                              "If you want to open the interface, just try to open MEGAsync again.");
            msgInfo.finishFunc = [this](QPointer<QMessageBox>){
                preferences->setOneTimeActionDone(Preferences::ONE_TIME_ACTION_NO_SYSTRAY_AVAILABLE, true);
            };
            QMegaMessageBox::warning(msgInfo);
        }
    }
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

    auto checkUploadNameDialog = new DuplicatedNodeDialog(node);
    
    auto counter(0);
    EventUpdater checkUpdater(uploadQueue.size());
    while (!uploadQueue.isEmpty())
    {
        QString nodePath = uploadQueue.dequeue();
        checkUploadNameDialog->checkUpload(nodePath, node);

        checkUpdater.update(counter);
        counter++;
    }

    if(!checkUploadNameDialog->isEmpty())
    {
        DialogOpener::showDialog<DuplicatedNodeDialog>(checkUploadNameDialog, this, &MegaApplication::onUploadsCheckedAndReady);
    }
    else
    {
        checkUploadNameDialog->accept();
        onUploadsCheckedAndReady(checkUploadNameDialog);
        checkUploadNameDialog->close();
        checkUploadNameDialog->deleteLater();
    }
}

void MegaApplication::onUploadsCheckedAndReady(QPointer<DuplicatedNodeDialog> checkDialog)
{
    if(checkDialog && checkDialog->result() == QDialog::Accepted)
    {
        auto uploads = checkDialog->getResolvedConflicts();

        auto data = TransferMetaDataContainer::createTransferMetaData<UploadTransferMetaData>(checkDialog->getNode()->getHandle());
        preferences->setOverStorageDismissExecution(0);

        auto batch = std::shared_ptr<TransferBatch>(new TransferBatch(data->getAppId()));
        mBlockingBatch.add(batch);

        EventUpdater updater(uploads.size(),20);

        auto counter = 0;
        data->setInitialTransfers(uploads.size());
        foreach(auto uploadInfo, uploads)
        {
            QString filePath = uploadInfo->getLocalPath();
            uploader->upload(filePath, uploadInfo->getNewName(), checkDialog->getNode(), data->getAppId(), batch);
            
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

    downloader->processDownloadQueue(&downloadQueue, mBlockingBatch, path);
}

void MegaApplication::createTransferManagerDialog(TransfersWidget::TM_TAB tab)
{
    mTransferManager = new TransferManager(tab, megaApi);
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
    QApplication::exit();
}

// TODO: This is legacy behavior and should be deleted when SRW is merged
void MegaApplication::deleteSdkCache()
{
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
        QMegaMessageBox::MessageBoxInfo msgInfo;
        msgInfo.title = getMEGAString();
        msgInfo.text = tr("There is an active transfer. Exit the app?\n"
                                 "Transfer will automatically resume when you re-open the app.",
                                 "",
                                 mTransfersModel->hasActiveTransfers());
        msgInfo.buttons = QMessageBox::Yes|QMessageBox::No;
        QMap<QMessageBox::Button, QString> textsByButton;
        textsByButton.insert(QMessageBox::Yes, tr("Exit app"));
        textsByButton.insert(QMessageBox::No, tr("Stay in app"));
        msgInfo.buttonsText = textsByButton;
        msgInfo.finishFunc = [this](QPointer<QMessageBox> msg)
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
        QMegaMessageBox::question(msgInfo);
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
                               .arg(numLocalNodes).toUtf8().constData(), false, nullptr);
        }
    }
}

void MegaApplication::checkOverStorageStates()
{
    if (!preferences->logged() || ((!infoDialog || !infoDialog->isVisible()) && !mStorageOverquotaDialog && !Platform::getInstance()->isUserActive()))
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
                               "Overstorage dialog shown", false, nullptr);
            if (!mStorageOverquotaDialog)
            {
                mStorageOverquotaDialog = new UpgradeOverStorage(megaApi, mPricing, mCurrency);
                DialogOpener::showDialog(mStorageOverquotaDialog);
            }
        }
        else if (((QDateTime::currentMSecsSinceEpoch() - preferences->getOverStorageDialogExecution()) > Preferences::OQ_NOTIFICATION_INTERVAL_MS)
                     && (!preferences->getOverStorageNotificationExecution() || ((QDateTime::currentMSecsSinceEpoch() - preferences->getOverStorageNotificationExecution()) > Preferences::OQ_NOTIFICATION_INTERVAL_MS)))
        {
            preferences->setOverStorageNotificationExecution(QDateTime::currentMSecsSinceEpoch());
            megaApi->sendEvent(AppStatsEvents::EVENT_OVER_STORAGE_NOTIF,
                               "Overstorage notification shown", false, nullptr);
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
                                       "Overstorage warning shown", false, nullptr);
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
                                       "Almost overstorage warning shown", false, nullptr);
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
                               "Almost overstorage notification shown", false, nullptr);
            mOsNotifications->sendOverStorageNotification(Preferences::STATE_ALMOST_OVER_STORAGE);
        }

        if(mStorageOverquotaDialog)
        {
            mStorageOverquotaDialog->close();
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
                                       "Paywall notification shown", false, nullptr);
                    mOsNotifications->sendOverStorageNotification(Preferences::STATE_PAYWALL);
                }
            }

            if(mStorageOverquotaDialog)
            {
                mStorageOverquotaDialog->close();
            }
        }
    }
    else
    {
        if (infoDialog)
        {
            infoDialog->updateOverStorageState(Preferences::STATE_BELOW_OVER_STORAGE);
        }

        if(mStorageOverquotaDialog)
        {
            mStorageOverquotaDialog->close();
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
    mTransferQuota->checkQuotaAndAlerts();
}

void MegaApplication::periodicTasks()
{
    if (appfinished)
    {
        return;
    }
    return;

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
                megaApi->sendEvent(AppStatsEvents::EVENT_UPDATE_OK, "MEGAsync updated OK", false, nullptr);
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
    Platform::getInstance()->stopShellDispatcher();

    for (auto localFolder : model->getLocalFolders(SyncInfo::AllHandledSyncTypes))
    {
        Platform::getInstance()->notifyItemChange(localFolder, MegaApi::STATE_NONE);
    }

    PowerOptions::appShutdown();
    mSyncController.reset();
    UserAttributes::UserAttributesManager::instance().reset();

    removeAllFinishedTransfers();
    clearViewedTransfers();
    DialogOpener::closeAllDialogs();

    if(mBlockingBatch.isValid())
    {
        mBlockingBatch.cancelTransfer();
    }

    delete httpServer;
    httpServer = nullptr;
    delete httpsServer;
    httpsServer = nullptr;
    delete uploader;
    uploader = nullptr;
    delete downloader;
    downloader = nullptr;
    delete delegateListener;
    delegateListener = nullptr;
    mPricing.reset();
    mCurrency.reset();

    // Delete notifications stuff
    delete notificationsModel;
    notificationsModel = nullptr;
    delete notificationsProxyModel;
    notificationsProxyModel = nullptr;
    delete notificationsDelegate;
    notificationsDelegate = nullptr;

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
    megaApi = nullptr;

    delete megaApiFolders;
    megaApiFolders = nullptr;

    preferences->setLastExit(QDateTime::currentMSecsSinceEpoch());
    trayIcon->deleteLater();
    trayIcon = nullptr;

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

QString MegaApplication::getFormattedDateByCurrentLanguage(const QDateTime &datetime, QLocale::FormatType format) const
{
    return DateTimeFormatter::create(currentLanguageCode, datetime, format);
}

void MegaApplication::raiseInfoDialog()
{
    if(preferences && !preferences->logged())
    {
        openInfoWizard();
    }

    if (infoDialog)
    {
        infoDialog->updateDialogState();
        infoDialog->show();
        DialogOpener::raiseAllDialogs();

#ifdef __APPLE__
        Platform::getInstance()->raiseFileFolderSelectors();
#endif

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
        if(anyModalWindow->windowModality() == Qt::ApplicationModal)
        {
            // If the InfoDialog has opened any MessageBox (eg. enter your email), those must be closed first (as we are executing from that dialog's message loop!)
            // Bring that dialog to the front for the user to dismiss.s
            DialogOpener::raiseAllDialogs();
            return;
        }
    }

    if (infoDialog)
    {
        // in case the screens have changed, eg. laptop with 2 monitors attached (200%, main:100%, 150%), lock screen, unplug monitors, wait 30s, plug monitors, unlock screen:  infoDialog may be double size and only showing 1/4 or 1/2
        infoDialog->setWindowFlags(Qt::FramelessWindowHint);
        infoDialog->setWindowFlags(Qt::FramelessWindowHint | Qt::Popup);
    }
#endif

    const bool transferQuotaWaitTimeExpired{transferOverQuotaWaitTimeExpiredReceived && !mTransferQuota->isOverQuota()};
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
                                   "Main dialog shown while overquota", false, nullptr);
            }
            else if (storageState == MegaApi::STORAGE_STATE_ORANGE)
            {
                megaApi->sendEvent(AppStatsEvents::EVENT_MAIN_DIAL_WHILE_ALMOST_OVER_QUOTA,
                                   "Main dialog shown while almost overquota", false, nullptr);
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
    if (!httpServer) // && Platform::getInstance()->shouldRunHttpServer())
    {
        startHttpServer();
    }

    if (!updatingSSLcert) // && (httpsServer || Platform::getInstance()->shouldRunHttpsServer()))
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
                lastHovered = nullptr;
            }
        }
    }

    return QApplication::eventFilter(obj, e);
}

void MegaApplication::createInfoDialog()
{
    infoDialog = new InfoDialog(this);
    connect(infoDialog.data(), &InfoDialog::dismissStorageOverquota, this, &MegaApplication::onDismissStorageOverquota);
    connect(infoDialog.data(), &InfoDialog::transferOverquotaMsgVisibilityChange, mTransferQuota.get(), &TransferQuota::onTransferOverquotaVisibilityChange);
    connect(infoDialog.data(), &InfoDialog::almostTransferOverquotaMsgVisibilityChange, mTransferQuota.get(), &TransferQuota::onAlmostTransferOverquotaVisibilityChange);
    connect(infoDialog.data(), &InfoDialog::userActivity, this, &MegaApplication::registerUserActivity);
    connect(mTransferQuota.get(), &TransferQuota::sendState, infoDialog.data(), &InfoDialog::setBandwidthOverquotaState);
    connect(mTransferQuota.get(), &TransferQuota::overQuotaMessageNeedsToBeShown, infoDialog.data(), &InfoDialog::enableTransferOverquotaAlert);
    connect(mTransferQuota.get(), &TransferQuota::almostOverQuotaMessageNeedsToBeShown, infoDialog.data(), &InfoDialog::enableTransferAlmostOverquotaAlert);
    connect(infoDialog, SIGNAL(cancelScanning()), this, SLOT(cancelScanningStage()));
    connect(this, &MegaApplication::addBackup, infoDialog.data(), &InfoDialog::onAddBackup);
    scanStageController.updateReference(infoDialog);
}

QuotaState MegaApplication::getTransferQuotaState() const
{
     QuotaState quotaState (QuotaState::OK);

     if (mTransferQuota->isQuotaWarning())
     {
         quotaState = QuotaState::WARNING;
     }
     else if (mTransferQuota->isQuotaFull())
     {
         quotaState = QuotaState::FULL;
     }
     else if (mTransferQuota->isOverQuota())
     {
         quotaState = QuotaState::OVERQUOTA;
     }

     return quotaState;
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

std::shared_ptr<MegaPricing> MegaApplication::getPricing() const
{
    return mPricing;
}

int MegaApplication::getBlockState() const
{
    return blockState;
}

QPointer<SetupWizard> MegaApplication::getSetupWizard() const
{
    return mSetupWizard;
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
        megaApi->setExclusionLowerSizeLimit(computeExclusionSizeLimit(preferences->lowerSizeLimitValue(), preferences->lowerSizeLimitUnit()));
    }
    else
    {
        megaApi->setExclusionLowerSizeLimit(0);
    }

    if (preferences->upperSizeLimit())
    {
        megaApi->setExclusionUpperSizeLimit(computeExclusionSizeLimit(preferences->upperSizeLimitValue(), preferences->upperSizeLimitUnit()));
    }
    else
    {
        megaApi->setExclusionUpperSizeLimit(0);
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
            MegaApi::log(MegaApi::LOG_LEVEL_INFO, QString::fromUtf8("Active network interface: %1").arg(interfaceName).toUtf8().constData());

            const int numActiveIPs = countActiveIps(networkInterface.addressEntries());
            if (numActiveIPs > 0)
            {
                lastActiveTime = QDateTime::currentMSecsSinceEpoch();
                newInterfaces.append(networkInterface);
            }
        }
        else
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
    const QString logMessage = QString::fromUtf8(message) + QString::fromUtf8(": %1");
    const QString addressToLog = obfuscateIfNecessary(ipAddress);
    MegaApi::log(MegaApi::LOG_LEVEL_INFO, logMessage.arg(addressToLog).toUtf8().constData());
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
                  .arg(outSharesStrings.toList().join(QLatin1String(", "))));
    }

    QMegaMessageBox::MessageBoxInfo msgInfo;
    msgInfo.title = tr("Security upgrade");
    msgInfo.text = message;
    msgInfo.buttons = QMessageBox::Ok|QMessageBox::Cancel;
    msgInfo.finishFunc = [this](QPointer<QMessageBox> msg)
        {
        if (msg->result() == QMessageBox::Ok)
        {
            megaApi->upgradeSecurity(new OnFinishOneShot(megaApi, [=](const MegaError& e){
                if (e.getErrorCode() != MegaError::API_OK)
                {
                    QString errorMessage = tr("Failed to ugrade security. Error: %1")
                                           .arg(tr(e.getErrorString()));
                    showErrorMessage(errorMessage, QMegaMessageBox::errorTitle());
                    exitApplication();
                }
            }));
        }
        else
        {
            exitApplication();
        }
    };

    QMegaMessageBox::information(msgInfo);
}

void MegaApplication::registerCommonQMLElements()
{
    //Register metatypes to use them in signals/slots
    qRegisterMetaType<QQueue<QString> >("QQueueQString");
    qRegisterMetaTypeStreamOperators<QQueue<QString> >("QQueueQString");

    qmlRegisterType<AccountInfoData>("AccountInfoData", 1, 0, "AccountInfoData");
    qmlRegisterType<ChooseLocalFolder>("ChooseLocalFolder", 1, 0, "ChooseLocalFolder");
    qmlRegisterType<ChooseRemoteFolder>("ChooseRemoteFolder", 1, 0, "ChooseRemoteFolder");

    qmlRegisterType<BackupsProxyModel>("BackupsProxyModel", 1, 0, "BackupsProxyModel");

    qmlRegisterModule("Components.BusyIndicator", 1, 0);
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/components/busyIndicator/BusyIndicator.qml")), "Components.BusyIndicator", 1, 0, "BusyIndicator");

    qmlRegisterModule("Components.Buttons", 1, 0);
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/components/buttons/Button.qml")), "Components.Buttons", 1, 0, "Button");
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/components/buttons/CardButton.qml")), "Components.Buttons", 1, 0, "CardButton");
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/components/buttons/CardHorizontalButton.qml")), "Components.Buttons", 1, 0, "CardHorizontalButton");
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/components/buttons/CardVerticalButton.qml")), "Components.Buttons", 1, 0, "CardVerticalButton");
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/components/buttons/HelpButton.qml")), "Components.Buttons", 1, 0, "HelpButton");
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/components/buttons/IconButton.qml")), "Components.Buttons", 1, 0, "IconButton");
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/components/buttons/OutlineButton.qml")), "Components.Buttons", 1, 0, "OutlineButton");
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/components/buttons/PrimaryButton.qml")), "Components.Buttons", 1, 0, "PrimaryButton");
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/components/buttons/SecondaryButton.qml")), "Components.Buttons", 1, 0, "SecondaryButton");
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/components/buttons/Colors.qml")), "Components.Buttons", 1, 0, "Colors");
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/components/buttons/Icon.qml")), "Components.Buttons", 1, 0, "Icon");
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/components/buttons/Progress.qml")), "Components.Buttons", 1, 0, "Progress");
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/components/buttons/Sizes.qml")), "Components.Buttons", 1, 0, "Sizes");
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/components/buttons/SmallSizes.qml")), "Components.Buttons", 1, 0, "SmallSizes");
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/components/buttons/LargeSizes.qml")), "Components.Buttons", 1, 0, "LargeSizes");

    qmlRegisterModule("Components.CheckBoxes", 1, 0);
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/components/checkBoxes/CheckBox.qml")), "Components.CheckBoxes", 1, 0, "CheckBox");
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/components/checkBoxes/Colors.qml")), "Components.CheckBoxes", 1, 0, "Colors");
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/components/checkBoxes/Icons.qml")), "Components.CheckBoxes", 1, 0, "Icons");
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/components/checkBoxes/Sizes.qml")), "Components.CheckBoxes", 1, 0, "Sizes");

    qmlRegisterModule("Components.Images", 1, 0);
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/components/images/SvgImage.qml")), "Components.Images", 1, 0, "SvgImage");
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/components/images/Image.qml")), "Components.Images", 1, 0, "Image");

    qmlRegisterModule("Components.ScrollBars", 1, 0);
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/components/scrollBars/ScrollBar.qml")), "Components.ScrollBars", 1, 0, "ScrollBar");

    qmlRegisterModule("Components.TextFields", 1, 0);
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/components/textFields/EmailTextField.qml")), "Components.TextFields", 1, 0, "EmailTextField");
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/components/textFields/Hint.qml")), "Components.TextFields", 1, 0, "Hint");
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/components/textFields/IconTextField.qml")), "Components.TextFields", 1, 0, "IconTextField");
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/components/textFields/LeftIcon.qml")), "Components.TextFields", 1, 0, "LeftIcon");
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/components/textFields/PasswordTextField.qml")), "Components.TextFields", 1, 0, "PasswordTextField");
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/components/textFields/RightIcon.qml")), "Components.TextFields", 1, 0, "RightIcon");
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/components/textFields/TextField.qml")), "Components.TextFields", 1, 0, "TextField");
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/components/textFields/TwoFA.qml")), "Components.TextFields", 1, 0, "TwoFA");
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/components/textFields/TwoFADigit.qml")), "Components.TextFields", 1, 0, "TwoFADigit");
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/components/textFields/Sizes.qml")), "Components.TextFields", 1, 0, "Sizes");
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/components/textFields/SmallSizes.qml")), "Components.TextFields", 1, 0, "SmallSizes");
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/components/textFields/LargeSizes.qml")), "Components.TextFields", 1, 0, "LargeSizes");

    qmlRegisterModule("Components.Texts", 1, 0);
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/components/texts/HintStyle.qml")), "Components.Texts", 1, 0, "HintStyle");
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/components/texts/HintText.qml")), "Components.Texts", 1, 0, "HintText");
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/components/texts/NotificationInfo.qml")), "Components.Texts", 1, 0, "NotificationInfo");
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/components/texts/NotificationText.qml")), "Components.Texts", 1, 0, "NotificationText");
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/components/texts/RichText.qml")), "Components.Texts", 1, 0, "RichText");
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/components/texts/SecondaryText.qml")), "Components.Texts", 1, 0, "SecondaryText");
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/components/texts/Text.qml")), "Components.Texts", 1, 0, "Text");

    qmlRegisterModule("Components.ToolTips", 1, 0);
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/components/toolTips/ToolTip.qml")), "Components.ToolTips", 1, 0, "ToolTip");

    qmlRegisterModule("Components.Views", 1, 0);
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/components/views/ScrollPanel.qml")), "Components.Views", 1, 0, "ScrollPanel");

    qmlRegisterModule("Common", 1, 0);
    qmlRegisterSingletonType(QUrl(QString::fromUtf8("qrc:/common/Constants.qml")), "Common", 1, 0, "Constants");
    qmlRegisterSingletonType(QUrl(QString::fromUtf8("qrc:/common/Images.qml")), "Common", 1, 0, "Images");
    qmlRegisterSingletonType(QUrl(QString::fromUtf8("qrc:/common/Links.qml")), "Common", 1, 0, "Links");
    qmlRegisterSingletonType(QUrl(QString::fromUtf8("qrc:/common/RegexExpressions.qml")), "Common", 1, 0, "RegexExpressions");
    qmlRegisterSingletonType(QUrl(QString::fromUtf8("qrc:/common/Styles.qml")), "Common", 1, 0, "Styles");

    qmlRegisterType<QmlDialog>("com.qmldialog", 1, 0, "QmlDialog");
    qmlRegisterSingletonType<QmlClipboard>("QmlClipboard", 1, 0, "QmlClipboard", &QmlClipboard::qmlInstance);
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

void MegaApplication::setupWizardFinished(QPointer<SetupWizard> dialog)
{
    if(dialog)
    {
        auto result = dialog->result();

        if (appfinished)
        {
            return;
        }

        if (result == QDialog::Rejected)
        {
            auto onboarding = DialogOpener::findDialog<QmlDialogWrapper<Onboarding>>();
            if(!onboarding)
            {
                //clearDownloadAndPendingLinks(); // TODO: ONBOARDING
            }
        }
        else
        {
            QList<PreConfiguredSync> syncs = dialog->preconfiguredSyncs();

            if (infoDialog && infoDialog->isVisible())
            {
                infoDialog->hide();
            }

            loggedIn(true);
            startSyncs(syncs);
        }
    }
}

void MegaApplication::clearDownloadAndPendingLinks()
{
    if (downloadQueue.size() || pendingLinks.size())
    {
        for (QQueue<WrappedNode *>::iterator it = downloadQueue.begin(); it != downloadQueue.end(); ++it)
        {
            HTTPServer::onTransferDataUpdate((*it)->getMegaNode()->getHandle(), MegaTransfer::STATE_CANCELLED, 0, 0, 0, QString());
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

//void MegaApplication::infoWizardDialogFinished(QPointer<QmlDialogWrapper<Onboarding>> dialog)
//{
//    if (appfinished)
//    {
//        return;
//    }
//
//    if (dialog->result() != QDialog::Accepted)
//    {
//        auto setupWizard = DialogOpener::findDialog<SetupWizard>();
//        if(!setupWizard)
//        {
//            clearDownloadAndPendingLinks();
//        }
//    }
//}


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
    DialogOpener::closeAllDialogs();
    Platform::getInstance()->notifyAllSyncFoldersRemoved();

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
        QMegaMessageBox::MessageBoxInfo msgInfo;
        msgInfo.title = title;
        msgInfo.text = message;
        QMegaMessageBox::information(msgInfo);
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
        QMegaMessageBox::MessageBoxInfo msgInfo;
        msgInfo.title = title;
        msgInfo.text = message;
        QMegaMessageBox::warning(msgInfo);
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
        QMegaMessageBox::MessageBoxInfo msgInfo;
        msgInfo.title = title;
        msgInfo.text = message;
        QMegaMessageBox::critical(msgInfo);
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
        Platform::getInstance()->showInFolder(path);
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
                    char *endPtr = nullptr;
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
            QMegaMessageBox::MessageBoxInfo msgInfo;
            msgInfo.title = getMEGAString();
            QString message = tr("Please consider updating your operating system.") + QString::fromUtf8("\n")
#ifdef __APPLE__
                    + tr("MEGAsync will continue to work, however updates will no longer be supported for versions prior to OS X Yosemite soon.");
#elif defined(_WIN32)
                    + tr("MEGAsync will continue to work, however, updates will no longer be supported for Windows Vista and older operating systems soon.");
#else
                    + tr("MEGAsync will continue to work, however you might not receive new updates.");
#endif

            msgInfo.text = message;
            msgInfo.finishFunc = [this](QPointer<QMessageBox>)
            {
            preferences->setOneTimeActionDone(Preferences::ONE_TIME_ACTION_OS_TOO_OLD, true);
            };
            QMegaMessageBox::warning(msgInfo);
        }
    }
}

void MegaApplication::notifyChangeToAllFolders()
{
    for (auto localFolder : model->getLocalFolders(SyncInfo::AllHandledSyncTypes))
    {
        ++mProcessingShellNotifications;
        std::string stdLocalFolder = localFolder.toStdString();
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

void MegaApplication::showVerifyAccountInfo(std::function<void()> func)
{
    QPointer<VerifyLockMessage> verifyEmail = new VerifyLockMessage(blockState, infoDialog ? true : false);
    connect(verifyEmail.data(), SIGNAL(logout()), this, SLOT(unlink()));

    DialogOpener::showDialog(verifyEmail, func);
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
        return nullptr;
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

void MegaApplication::importLinks()
{
    if (appfinished)
    {
        return;
    }

    mTransferQuota->checkImportLinksAlertDismissed([this](int result){
        if(result == QDialog::Rejected)
        {
            if (!preferences->logged())
            {
                openInfoWizard();
                return;
            }

            //Show the dialog to paste public links
            auto pasteMegaLinksDialog = new PasteMegaLinksDialog();
            DialogOpener::showDialog<PasteMegaLinksDialog, TransferManager>(pasteMegaLinksDialog, true, this, &MegaApplication::onPasteMegaLinksDialogFinish);
        }
    });
}

void MegaApplication::onPasteMegaLinksDialogFinish(QPointer<PasteMegaLinksDialog> pasteMegaLinksDialog)
{
    if (pasteMegaLinksDialog->result() == QDialog::Accepted)
    {
        //Get the list of links from the dialog
        QStringList linkList = pasteMegaLinksDialog->getLinks();

        //We prefer to use a raw pointer to avoid crashes if the app is closed while the link is still being imported
        //If the app is closed while the link is being imported, there is a memory leak but nothing else
        auto linkProcessor = new LinkProcessor(linkList, MegaSyncApp->getMegaApi(), MegaSyncApp->getMegaApiFolders());

        //Open the import dialog
        auto importDialog = new ImportMegaLinksDialog(linkProcessor);
        DialogOpener::showDialog<ImportMegaLinksDialog, TransferManager>(importDialog, true, [this, linkProcessor, importDialog]()
        {
            if (importDialog->result() == QDialog::Accepted)
            {
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

                    connect(linkProcessor, &LinkProcessor::onLinkImportFinish, this, [this, linkProcessor]() mutable
                    {
                        preferences->setImportFolder(linkProcessor->getImportParentFolder());
                    });

                    linkProcessor->importLinks(importDialog->getImportPath());
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

void MegaApplication::uploadActionClicked()
{
    if (appfinished)
    {
        return;
    }

    const bool storageIsOverQuota(storageState == MegaApi::STORAGE_STATE_RED || storageState == MegaApi::STORAGE_STATE_PAYWALL);
    if(storageIsOverQuota)
    {
        auto overQuotaDialog = OverQuotaDialog::showDialog(OverQuotaDialogType::STORAGE_UPLOAD);
        if(overQuotaDialog)
        {
            DialogOpener::showDialog<OverQuotaDialog, TransferManager>(overQuotaDialog, false, [this](){
                uploadActionClickedFromWindowAfterOverQuotaCheck();
            });

            return;
        }
    }

    uploadActionClickedFromWindowAfterOverQuotaCheck();
}

void MegaApplication::uploadActionClickedFromWindowAfterOverQuotaCheck()
{
    QString  defaultFolderPath = getDefaultUploadPath();

    infoDialog->hide();
    QApplication::processEvents();
    if (appfinished)
    {
        return;
    }

    QWidget* parent(nullptr);

    auto TMDialog = DialogOpener::findDialog<TransferManager>();
    if(TMDialog && (TMDialog->getDialog()->isActiveWindow() || !TMDialog->getDialog()->isMinimized()))
    {
        parent = TMDialog->getDialog();
        DialogOpener::closeDialogsByParentClass<TransferManager>();
    }

    Platform::getInstance()->fileAndFolderSelector(QCoreApplication::translate("ShellExtension", "Upload to MEGA"), defaultFolderPath, true,
                                 parent,
                                 [this/*, blocker*/](QStringList files)
    {
        QQueue<QString> qFiles;
        foreach(QString file, files)
        {
            qFiles.append(file);
        }

        shellUpload(qFiles);
    });
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

    mTransferQuota->checkDownloadAlertDismissed([this](int result)
    {
        if(result == QDialog::Rejected)
        {
            auto downloadNodeSelector = new DownloadNodeSelector();
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
                            downloadQueue.append(new WrappedNode(WrappedNode::TransferOrigin::FROM_APP, selectedNode));
                        }
                    }
                    processDownloads();
                }
            });
        }
    });
}

void MegaApplication::streamActionClicked()
{
    if (appfinished)
    {
        return;
    }

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
        createTransferManagerDialog(static_cast<TransfersWidget::TM_TAB>(tab));
    }
    else
    {
        mTransferManager->toggleTab(tab);
    }

    DialogOpener::showGeometryRetainerDialog(mTransferManager);
}

void MegaApplication::loginActionClicked()
{
    if (appfinished)
    {
        return;
    }

   // userAction(SetupWizard::PAGE_LOGIN); // TODO ONBOARDING
}

void MegaApplication::showSetupWizard(int action)
{
    mSetupWizard = new SetupWizard(this);
    emit setupWizardCreated();
    mSetupWizard->goToStep(action);
    DialogOpener::showDialog(mSetupWizard, this, &MegaApplication::setupWizardFinished);
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

    auto downloadFolderSelector = new DownloadFromMegaDialog(preferences->downloadFolder());
    DialogOpener::showDialog<DownloadFromMegaDialog, TransferManager>(downloadFolderSelector, false, this, &MegaApplication::onDownloadFromMegaFinished);
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

            HTTPServer *webCom = qobject_cast<HTTPServer *>(sender());
            if (webCom)
            {
                showInfoDialog();
            }

            processDownloadQueue(path);
        }
        else
        {
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
    MegaNode *node = nullptr;

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

void MegaApplication::externalLinkDownload(QString megaLink, QString auth)
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

    fileUploadTarget = targetFolder;
    auto processUpload = [this](QStringList selectedFiles){
        if (!selectedFiles.isEmpty())
        {
            std::unique_ptr<MegaNode> target(megaApi->getNodeByHandle(fileUploadTarget));
            int files = 0;
            for (const auto& path : selectedFiles)
            {
                files++;
                startUpload(path, target.get(), nullptr);
            }

            HTTPServer::onUploadSelectionAccepted(files, 0);
        }
        else
        {
            HTTPServer::onUploadSelectionDiscarded();
        }
    };

    QString  defaultFolderPath = getDefaultUploadPath();

    QWidget* parent(nullptr);

#ifdef Q_OS_WIN
    repositionInfoDialog();
    parent = infoDialog;
#endif

    Platform::getInstance()->fileSelector(QCoreApplication::translate("ShellExtension", "Upload to MEGA"), defaultFolderPath,
                                 true, parent, processUpload);
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

    folderUploadTarget = targetFolder;

    auto processUpload = [this](QStringList foldersSelected){
        if (!foldersSelected.isEmpty())
        {
            NodeCount nodeCount = countFilesAndFolders(foldersSelected);

            processUploads(foldersSelected);
            HTTPServer::onUploadSelectionAccepted(nodeCount.files, nodeCount.folders);
        }
        else
        {
            HTTPServer::onUploadSelectionDiscarded();
        }
    };

    QString  defaultFolderPath = getDefaultUploadPath();

    QWidget* parent(nullptr);

#ifdef Q_OS_WIN
    repositionInfoDialog();
    parent = infoDialog;
#endif

    Platform::getInstance()->folderSelector(QCoreApplication::translate("ShellExtension", "Upload to MEGA"), defaultFolderPath,
                                 false, parent, processUpload);
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

    if(infoDialog)
    {
        infoDialog->addBackup();
    }
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
            if (reason == QSystemTrayIcon::Trigger)
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
#else
        DialogOpener::raiseAllDialogs();
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
            if (mSetupWizard)
            {
                DialogOpener::showDialog(mSetupWizard);
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
                                     {return s->isActive();}));
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

    if(auto dialog = DialogOpener::findDialog<QmlDialogWrapper<Onboarding>>())
    {
        DialogOpener::showDialog(dialog->getDialog());
        return;
    }

    QPointer<QmlDialogWrapper<Onboarding>> onboarding = new QmlDialogWrapper<Onboarding>();
    DialogOpener::showDialog(onboarding, [onboarding, this]
    {
       if(preferences && preferences->logged())
       {
            loggedIn(true);
       }
    });
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

    if (mSettingsDialog)
    {
        if (proxyOnly)
        {
            mSettingsDialog->showGuestMode();
        }

        mSettingsDialog->setProxyOnly(proxyOnly);

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
    openSettings(SettingsDialog::SYNCS_TAB);
    mSettingsDialog->addSyncFolder(megaFolderHandle);
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
        Platform::getInstance()->initMenu(initialTrayMenu, "TrayMenu", false);
    }

    if (guestSettingsAction)
    {
        guestSettingsAction->deleteLater();
        guestSettingsAction = nullptr;
    }
    guestSettingsAction = new QAction(tr("Settings"), this);

    // When triggered, open "Settings" window. As the user is not logged in, it
    // will only show proxy settings.
    connect(guestSettingsAction, &QAction::triggered, this, &MegaApplication::openSettings);

    if (initialExitAction)
    {
        initialExitAction->deleteLater();
        initialExitAction = nullptr;
    }
    initialExitAction = new QAction(PlatformStrings::exit(), this);
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
        Platform::getInstance()->initMenu(windowsMenu, "WindowsMenu", false);
    }
    else
    {
        QList<QAction *> actions = windowsMenu->actions();
        for (int i = 0; i < actions.size(); i++)
        {
            windowsMenu->removeAction(actions[i]);
        }
    }

    recreateAction(&windowsExitAction, PlatformStrings::exit(), &MegaApplication::tryExitApplication);
    recreateAction(&windowsSettingsAction, tr("Settings"), &MegaApplication::openSettings);
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
        Platform::getInstance()->initMenu(infoDialogMenu, "InfoDialogMenu");

        //Highlight menu entry on mouse over
        connect(infoDialogMenu, SIGNAL(hovered(QAction*)), this, SLOT(highLightMenuEntry(QAction*)), Qt::QueuedConnection);

        //Hide highlighted menu entry when mouse over
        infoDialogMenu->installEventFilter(this);
    }

    recreateMenuAction(&exitAction, PlatformStrings::exit(),
                       "://images/ico_quit.png", &MegaApplication::tryExitApplication);
    recreateMenuAction(&settingsAction, tr("Settings"),
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
        updateAction = nullptr;
    }

    if(aboutAction)
    {
        aboutAction->deleteLater();
        aboutAction = nullptr;
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
        Platform::getInstance()->initMenu(guestMenu, "GuestMenu");
    }

    if (exitActionGuest)
    {
        exitActionGuest->deleteLater();
        exitActionGuest = nullptr;
    }

    exitActionGuest = new MenuItemAction(PlatformStrings::exit(), QIcon(QString::fromUtf8("://images/ico_quit.png")));

    connect(exitActionGuest, &QAction::triggered, this, &MegaApplication::tryExitApplication);

    if (updateActionGuest)
    {
        updateActionGuest->deleteLater();
        updateActionGuest = nullptr;
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
        settingsActionGuest = nullptr;
    }
    settingsActionGuest = new MenuItemAction(tr("Settings"), QIcon(QString::fromUtf8("://images/ico_preferences.png")));

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

    if (mStorageOverquotaDialog)
    {
        mStorageOverquotaDialog->refreshStorageDetails();
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
                QMegaMessageBox::MessageBoxInfo msgInfo;
                msgInfo.title = getMEGAString();
                msgInfo.text = tr("Payment Failed");
                msgInfo.informativeText = tr("This month's payment has failed. Please resolve your payment issue as soon as possible to avoid any suspension of your business account.");
                msgInfo.buttons = QMessageBox::Yes|QMessageBox::No;
                msgInfo.defaultButton = QMessageBox::Yes;
                QMap<QMessageBox::StandardButton, QString> buttonsText;
                buttonsText.insert(QMessageBox::Yes, tr("Pay Now"));
                buttonsText.insert(QMessageBox::No, tr("Dismiss"));
                msgInfo.buttonsText = buttonsText;
                msgInfo.finishFunc = [this](QPointer<QMessageBox> msg)
                {
                    if(msg->result() == QMessageBox::Yes)
                    {
                        QString url = QString::fromUtf8("mega://#repay");
                        Utilities::getPROurlWithParameters(url);
                        Utilities::openUrl(QUrl(url));
                    }
                };

                QMegaMessageBox::warning(msgInfo);
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
            QMegaMessageBox::MessageBoxInfo msgInfo;
            msgInfo.title = getMEGAString();
            QMap<QMessageBox::StandardButton, QString> buttonsText;
            buttonsText.insert(QMessageBox::No, tr("Dismiss"));

            if (megaApi->isMasterBusinessAccount())
            {
                msgInfo.text = tr("Your Business account is expired");
                msgInfo.informativeText = tr("It seems the payment for your business account has failed. Your account is suspended as read only until you proceed with the needed payments.");
                msgInfo.buttons = QMessageBox::Yes|QMessageBox::No;
                buttonsText.insert(QMessageBox::Yes, tr("Pay Now"));
                msgInfo.defaultButton = QMessageBox::Yes;
                msgInfo.finishFunc = [this](QPointer<QMessageBox> msg)
                {
                    if(msg->result() == QMessageBox::Yes)
                    {
                        QString url = QString::fromUtf8("mega://#repay");
                        Utilities::getPROurlWithParameters(url);
                        Utilities::openUrl(QUrl(url));
                    }
                };
            }
            else
            {
                msgInfo.textFormat = Qt::RichText;
                msgInfo.text = tr("Account Suspended");
                msgInfo.informativeText = tr("Your account is currently [A]suspended[/A]. You can only browse your data.")
                                .replace(QString::fromUtf8("[A]"), QString::fromUtf8("<span style=\"font-weight: bold; text-decoration:none;\">"))
                                .replace(QString::fromUtf8("[/A]"), QString::fromUtf8("</span>"))
                            + QString::fromUtf8("<br>") + QString::fromUtf8("<br>") +
                            tr("[A]Important:[/A] Contact your business account administrator to resolve the issue and activate your account.")
                                .replace(QString::fromUtf8("[A]"), QString::fromUtf8("<span style=\"font-weight: bold; color:#DF4843; text-decoration:none;\">"))
                        .replace(QString::fromUtf8("[/A]"), QString::fromUtf8("</span>")) + QString::fromUtf8("\n");
                msgInfo.buttons = QMessageBox::No;
            }

            msgInfo.buttonsText = buttonsText;
            QMegaMessageBox::warning(msgInfo);

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
            Platform::getInstance()->notifyAllSyncFoldersAdded();
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
            QString megaSyncError (QCoreApplication::translate("MegaSyncError", MegaSync::getMegaSyncErrorCode(eventNumber)));

            if (!syncsUnattended.isEmpty()
                    && !backupsUnattended.isEmpty())
            {
                showErrorMessage(tr("Your syncs and backups have been disabled: %1").arg(megaSyncError));
            }
            else if (!backupsUnattended.isEmpty())
            {
                showErrorMessage(tr("Your backups have been disabled: %1").arg(megaSyncError));
            }
            else
            {
                showErrorMessage(tr("Your syncs have been disabled: %1").arg(megaSyncError));
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

                showVerifyAccountInfo([this]()
                {
                    if (infoDialog)
                    {
                        if (infoDialog->getLoggedInMode() != blockState)
                        {
                            infoDialog->regenerateLayout(blockState);
                            DialogOpener::closeAllDialogs();
                        }
                    }
                    else if (!whyamiblockedPeriodicPetition) //Do not force show on periodic whyamiblocked call
                    {
                        showVerifyAccountInfo();
                    }

                    whyamiblockedPeriodicPetition = false;
                });
                break;
            }
            case MegaApi::ACCOUNT_BLOCKED_SUBUSER_DISABLED:
            {
                QMegaMessageBox::MessageBoxInfo msgInfo;
                msgInfo.title = getMEGAString();
                msgInfo.text = tr("Your account has been disabled by your administrator. Please contact your business account administrator for further details.");
                QMegaMessageBox::warning(msgInfo);
                break;
            }
            default:
            {
                QMegaMessageBox::MessageBoxInfo msgInfo;
                msgInfo.title = getMEGAString();
                msgInfo.text = QCoreApplication::translate("MegaError", event->getText());
                QMegaMessageBox::critical(msgInfo);
                break;
        }
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
//    else if (event->getType() == MegaEvent::EVENT_UPGRADE_SECURITY)
//    {
//        processUpgradeSecurityEvent();
//    }
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
            showErrorMessage(tr("Error getting link: %1").arg(QCoreApplication::translate("MegaError", e->getErrorString())));
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

                mTransferQuota->setOverQuotaDialogPricing(mPricing, mCurrency);

                if (mStorageOverquotaDialog)
                {
                    mStorageOverquotaDialog->setPricing(mPricing, mCurrency);
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
            Platform::getInstance()->prepareForSync();
            int errorCode = e->getErrorCode();
            if (errorCode == MegaError::API_OK)
            {
                if (!preferences->getSession().isEmpty())
                {
                    //Successful login, fetch nodes
                    fetchNodes();
                    break;
                }

                unlink(true);
            }
            else
            {
                QMegaMessageBox::MessageBoxInfo msgInfo;
                msgInfo.title = getMEGAString();
                msgInfo.finishFunc = [this](QPointer<QMessageBox>)
                {
                    unlink(true);
                };

                if (errorCode == MegaError::API_EBLOCKED)
                {
                    msgInfo.text = tr("Your account has been blocked. Please contact support@mega.co.nz");
                    QMegaMessageBox::critical(msgInfo);
                }
                else if (errorCode != MegaError::API_ESID && errorCode != MegaError::API_ESSL)
                    //Invalid session or public key, already managed in TYPE_LOGOUT
                {
                    msgInfo.text = tr("Login error: %1").arg(QCoreApplication::translate("MegaError", e->getErrorString()));
                    QMegaMessageBox::warning(msgInfo);
                }
                else
                {
                    //Wrong login -> logout
                    unlink(true);
                }
            }
        }
        onGlobalSyncStateChanged(megaApi);
        break;
    }
    case MegaRequest::TYPE_LOGOUT:
    {
        int errorCode = e->getErrorCode();
        if (errorCode)
        {
            mLogoutWithError = true;

            QMegaMessageBox::MessageBoxInfo msgInfo;
            msgInfo.title =  getMEGAString();
            msgInfo.finishFunc = [this](QPointer<QMessageBox>){
                mLogoutWithError = false;
                unlink();};

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
                msgInfo.text = tr("You have been logged out on this computer from another location");
                QMegaMessageBox::information(msgInfo);
            }
            else if (errorCode == MegaError::API_ESSL)
            {
                msgInfo.text = tr("Our SSL key can't be verified. You could be affected by a man-in-the-middle attack or your antivirus software could be intercepting your communications and causing this problem. Please disable it and try again.")
                        + QString::fromUtf8(" (Issuer: %1)").arg(QString::fromUtf8(request->getText() ? request->getText() : "Unknown"));
                QMegaMessageBox::critical(msgInfo);
            }
            else if (errorCode != MegaError::API_EACCESS)
            {
                msgInfo.text = tr("You have been logged out because of this error: %1")
                        .arg(QCoreApplication::translate("MegaError", e->getErrorString()));
                QMegaMessageBox::information(msgInfo);
            }
            else
            {
                unlink();
            }
        }
        else if(!mLogoutWithError)
        {
            DialogOpener::closeAllDialogs();
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

                     DialogOpener::closeAllDialogs();
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
                                   "Local SSL certificate renewed", false, nullptr);
                delete httpsServer;
                httpsServer = nullptr;
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

            // TODO: check with sdk team if this case is possible
            if (!mRootNode)
            {
                QMegaMessageBox::MessageBoxInfo msgInfo;
                msgInfo.title =  QMegaMessageBox::errorTitle();
                msgInfo.text =   tr("Unable to get the filesystem.\n"
                                                       "Please, try again. If the problem persists "
                                                       "please contact bug@mega.co.nz");

//                auto onboarding = DialogOpener::findDialogByClass<Onboarding>();
//                if(onboarding)
//                {
//                   // onboarding->close(); TODO ONBOARDING
//                }
//                msgInfo.finishFunc = [this](QPointer<QMessageBox>)
//                {
//                    auto setupWizard = DialogOpener::findDialog<SetupWizard>();
//                    if(setupWizard)
//                    {
//                        setupWizard->close();
//                    }

//                    rebootApplication(false);
//                };

//                QMegaMessageBox::warning(msgInfo);

                break;
            }

            std::unique_ptr<char[]> email(megaApi->getMyEmail());
            bool logged = preferences->logged();
            bool firstTime = !logged && email && !preferences->hasEmail(QString::fromUtf8(email.get()));
            if (!logged) //session resumed from general storage (or logged in via user/pass)
            {
                if (firstTime)
                {
                    //showSetupWizard(SetupWizard::PAGE_MODE);
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

                    if(mSetupWizard)
                    {
                        mSetupWizard->close();
                    }
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
            QMegaMessageBox::MessageBoxInfo msgInfo;
            msgInfo.title =  tr("Password changed");
            msgInfo.text =   tr("Your password has been changed.");
            QMegaMessageBox::information(msgInfo);
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

        // TODO: investigate: is this case possible and what should we do? Restart the app?
        if (!root || !vault || !rubbish)
        {
            break;
        }

        //Account details retrieved, update the preferences and the information dialog
        shared_ptr<MegaAccountDetails> details(request->getMegaAccountDetails());

        mThreadPool->push([=]()
        {//thread pool function
        shared_ptr<MegaNodeList> inShares(megaApi->getInShares());

        if (!inShares)
        {
            // TODO: investigate: is this case possible and what should we do? Restart the app?
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
                                   "Red light does not match used storage", false, nullptr);
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
            if (mSettingsDialog)
            {
                mSettingsDialog->storageChanged();
            }

            notifyStorageObservers();
        }

        const bool proUserIsOverquota (megaApi->getBandwidthOverquotaDelay() &&
                    preferences->accountType() != Preferences::ACCOUNT_TYPE_FREE);
        if (proUserIsOverquota)
        {
            mTransferQuota->setOverQuota(std::chrono::seconds(megaApi->getBandwidthOverquotaDelay()));
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
            mTransferQuota->updateQuotaState();
        }

        preferences->sync();

        if (infoDialog)
        {
            infoDialog->setUsage();
            infoDialog->setAccountType(preferences->accountType());
        }

        if (mStorageOverquotaDialog)
        {
            mStorageOverquotaDialog->refreshStorageDetails();
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
            megaApi->sendEvent(AppStatsEvents::EVENT_SYNC_ADD_FAIL_API_EACCESS,
                               "Sync addition fails with API_EACCESS", false, nullptr); //this would enforce a fetchNodes in the past
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
            megaApi->sendEvent(AppStatsEvents::EVENT_PRO_REDIRECT, "Redirection to PRO", false, nullptr);
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

            if (mSettingsDialog)
            {
                mSettingsDialog->setProxyOnly(false);
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

    if (transfer->getState() == MegaTransfer::STATE_COMPLETED || transfer->getState() == MegaTransfer::STATE_FAILED)
    {
        MegaTransfer *t = transfer->copy();
        if (finishedTransfers.count(transfer->getTag()))
        {
            assert(false);
            megaApi->sendEvent(AppStatsEvents::EVENT_DUP_FINISHED_TRSF,
                               QString::fromUtf8("Duplicated finished transfer: %1").arg(QString::number(transfer->getTag())).toUtf8().constData(), false, nullptr);
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
                                   "MEGAsync first synced file", false, nullptr);
            }
            mIsFirstFileTwoWaySynced = true;
            break;
        }
        case MegaSync::SyncType::TYPE_BACKUP:
        {
            if (!mIsFirstFileBackedUp && !preferences->isFirstFileBackedUp())
            {
                megaApi->sendEvent(AppStatsEvents::EVENT_1ST_BACKED_UP_FILE,
                                   "MEGAsync first backed-up file", false, nullptr);
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
        else if (e->getValue() && !mTransferQuota->isOverQuota())
        {
            const auto waitTime = std::chrono::seconds(e->getValue());
            preferences->clearTemporalBandwidth();
            megaApi->getPricing();
            updateUserStats(false, true, true, true, USERSTATS_TRANSFERTEMPERROR);  // get udpated transfer quota (also pro status in case out of quota is due to account paid period expiry)
            mTransferQuota->setOverQuota(waitTime);
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
        QMegaMessageBox::MessageBoxInfo msgInfo;
        msgInfo.title =  tr("Error adding sync");
        msgInfo.text = tr("This sync can't be added: %1. Reason: %2").arg(localpath)
                                  .arg( errorCode > 0 ? QCoreApplication::translate("MegaSyncError", MegaSync::getMegaSyncErrorCode(errorCode))
                                   : QCoreApplication::translate("MegaError", MegaError::getErrorString(errorCode)));
        QMegaMessageBox::critical(msgInfo);
    }
}

void MegaApplication::onAccountUpdate(MegaApi *)
{
    if (appfinished || !preferences->logged())
    {
        return;
    }

    preferences->clearTemporalBandwidth();
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

    // TODO: investigate this. Could a restart of the app be enough?

    //Don't reload the filesystem here because it's unsafe
    //and the most probable cause for this callback is a false positive.
    //Simply set the crashed flag to force a filesystem reload in the next execution.
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
        waiting = megaApi->isWaiting();
        syncing = megaApi->isSyncing();

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

    if (sync->getRunState() == MegaSync::RUNSTATE_DISABLED)
    {
        if (sync->getError())
        {
            model->addUnattendedDisabledSync(sync->getBackupId(),
                                             static_cast<MegaSync::SyncType>(sync->getType()));
        }
        onSyncDisabled(model->getSyncSettingByTag(sync->getBackupId()));
    }

    model->updateSyncSettings(sync); //Note, we are not updating the remote sync path
    // we asume that cannot change for existing syncs.

    onGlobalSyncStateChanged(api);
}

void MegaApplication::onSyncFileStateChanged(MegaApi *, MegaSync *, string *localPath, int newState)
{
    if (appfinished || !localPath || localPath->empty())
    {
        return;
    }

    Platform::getInstance()->notifySyncFileChange(localPath, newState);
}

void MegaApplication::onSyncDisabled(std::shared_ptr<SyncSettings> syncSetting)
{
    if (!syncSetting)
    {
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR,
                     QString::fromUtf8("onSyncDisabled for non existing sync").toUtf8().constData());
        return;
    }

    auto errorCode (syncSetting->getError());
    auto syncType (syncSetting->getType());

    if (errorCode != MegaError::API_OK)
    {
        model->addUnattendedDisabledSync(syncSetting->backupId(),
                                         static_cast<MegaSync::SyncType>(syncType));
    }
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

            if (!syncSetting->isEnabled()
                    && errorCode != MegaSync::Error::LOGGED_OUT)
            {
                QString errMsg (tr("Your sync \"%1\" has been temporarily disabled: %2")
                                .arg(syncName, QCoreApplication::translate("MegaSyncError", MegaSync::getMegaSyncErrorCode(errorCode))));
                showErrorMessage(errMsg);
            }
            else if (errorCode != MegaSync::NO_SYNC_ERROR
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
            if (!syncSetting->isEnabled()
                    && errorCode != MegaSync::Error::LOGGED_OUT)
            {
                QString errMsg (tr("Your backup \"%1\" has been temporarily disabled: %2")
                                .arg(syncName, QCoreApplication::translate("MegaSyncError", MegaSync::getMegaSyncErrorCode(errorCode))));
                showErrorMessage(errMsg);
            }
            else if (errorCode != MegaSync::NO_SYNC_ERROR
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

void MegaApplication::onSyncEnabled(std::shared_ptr<SyncSettings> syncSetting)
{
    if (!syncSetting)
    {
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("onSyncEnabled for non existing sync").toUtf8().constData());
        return;
    }

    MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, QString::fromUtf8("Your sync \"%1\" has been re-enabled. Error = %2")
                 .arg(syncSetting->name()).arg(syncSetting->getError()).toUtf8().constData());


    showErrorMessage(tr("Your sync \"%1\" has been enabled")
                     .arg(syncSetting->name()));

    model->removeUnattendedDisabledSync(syncSetting->backupId(), syncSetting->getType());
}

void MegaApplication::onSyncAdded(MegaApi *api, MegaSync *sync)
{
    if (appfinished || !sync)
    {
        return;
    }

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

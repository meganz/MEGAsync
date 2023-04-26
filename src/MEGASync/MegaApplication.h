#ifndef MEGAAPPLICATION_H
#define MEGAAPPLICATION_H

#include <QApplication>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QDir>
#include <QLocalServer>
#include <QLocalSocket>
#include <QDataStream>
#include <QQueue>
#include <QNetworkInterface>
#include <memory>

#include "gui/TransferManager.h"
#include "gui/InfoDialog.h"
#include "gui/UpgradeOverStorage.h"
#include "gui/SetupWizard.h"
#include "gui/SettingsDialog.h"
#include "gui/UploadToMegaDialog.h"
#include "gui/DownloadFromMegaDialog.h"
#include "gui/StreamingFromMegaDialog.h"
#include "gui/ImportMegaLinksDialog.h"
#include "gui/MultiQFileDialog.h"
#include "gui/PasteMegaLinksDialog.h"
#include "gui/ChangeLogDialog.h"
#include "gui/InfoWizard.h"
#include "control/Preferences.h"
#include "control/HTTPServer.h"
#include "control/MegaUploader.h"
#include "control/MegaDownloader.h"
#include "control/UpdateTask.h"
#include "control/MegaSyncLogger.h"
#include "control/ThreadPool.h"
#include "control/Utilities.h"
#include "syncs/control/SyncInfo.h"
#include "syncs/control/SyncController.h"
#include "megaapi.h"
#include "QTMegaListener.h"
#include "gui/QFilterAlertsModel.h"
#include "gui/MegaAlertDelegate.h"
#include "gui/VerifyLockMessage.h"
#include "notifications/DesktopNotifications.h"
#include "ScanStageController.h"
#include "TransferQuota.h"
#include "BlockingStageProgressController.h"

class TransfersModel;
class StalledIssuesModel;

#ifdef __APPLE__
    #include "gui/MegaSystemTrayIcon.h"
    #include <mach/mach.h>
    #include <sys/sysctl.h>
    #include <errno.h>
#endif

Q_DECLARE_METATYPE(QQueue<QString>)

class NotificatorBase;
class MEGASyncDelegateListener;
class ShellNotifier;
class TransferMetadata;
class DuplicatedNodeDialog;
class LinkProcessor;

enum GetUserStatsReason {
    USERSTATS_LOGGEDIN,
    USERSTATS_STORAGESTATECHANGE,
    USERSTATS_TRAFFICLIGHT,
    USERSTATS_SHOWDIALOG,
    USERSTATS_CHANGEPROXY,
    USERSTATS_TRANSFERTEMPERROR,
    USERSTATS_ACCOUNTUPDATE,
    USERSTATS_STORAGECLICKED,
    USERSTATS_BANDWIDTH_TIMEOUT_SHOWINFODIALOG,
    USERSTATS_PRO_EXPIRED,
    USERSTATS_OPENSETTINGSDIALOG,
    USERSTATS_STORAGECACHEUNKNOWN,
    USERSTATS_SHOWMAINDIALOG,
    USERSTATS_REMOVEVERSIONS,
};

class MegaApplication : public QApplication, public mega::MegaListener, public StorageDetailsObserved, public BandwidthDetailsObserved, public AccountDetailsObserved
{
    Q_OBJECT

#ifdef Q_OS_LINUX
    void setTrayIconFromTheme(QString icon);
#endif

    static void loadDataPath();

public:

    explicit MegaApplication(int &argc, char **argv);
    ~MegaApplication();

    void initialize();
    static QString applicationFilePath();
    static QString applicationDirPath();
    static QString applicationDataPath();
    QString getCurrentLanguageCode();
    void changeLanguage(QString languageCode);
    void updateTrayIcon();
    void repositionInfoDialog();

    QString getFormattedDateByCurrentLanguage(const QDateTime& datetime, QLocale::FormatType format = QLocale::FormatType::LongFormat) const;

    void onEvent(mega::MegaApi *api, mega::MegaEvent *event) override;
    void onRequestStart(mega::MegaApi* api, mega::MegaRequest *request) override;
    void onRequestFinish(mega::MegaApi* api, mega::MegaRequest *request, mega::MegaError* e) override;
    void onTransferStart(mega::MegaApi *api, mega::MegaTransfer *transfer) override;
    void onTransferFinish(mega::MegaApi* api, mega::MegaTransfer *transfer, mega::MegaError* e) override;
    void onTransferUpdate(mega::MegaApi *api, mega::MegaTransfer *transfer) override;
    void onTransferTemporaryError(mega::MegaApi *api, mega::MegaTransfer *transfer, mega::MegaError* e) override;
    void onAccountUpdate(mega::MegaApi *api) override;
    void onUserAlertsUpdate(mega::MegaApi *api, mega::MegaUserAlertList *list) override;
    void onUsersUpdate(mega::MegaApi* api, mega::MegaUserList *users) override;
    void onNodesUpdate(mega::MegaApi* api, mega::MegaNodeList *nodes) override;
    void onReloadNeeded(mega::MegaApi* api) override;
    void onGlobalSyncStateChanged(mega::MegaApi *api) override;
    void onSyncStateChanged(mega::MegaApi *api,  mega::MegaSync *sync) override;
    void onSyncStatsUpdated(mega::MegaApi *api,  mega::MegaSyncStats *stats) override;
    void onSyncFileStateChanged(mega::MegaApi *api, mega::MegaSync *sync, std::string *localPath, int newState) override;

    void onSyncAdded(mega::MegaApi *api, mega::MegaSync *sync) override;
    void onSyncDeleted(mega::MegaApi *api, mega::MegaSync *sync) override;

    virtual void onCheckDeferredPreferencesSync(bool timeout);
    void onGlobalSyncStateChangedImpl(mega::MegaApi* api, bool timeout);

    void showAddSyncError(mega::MegaRequest *request, mega::MegaError* e, QString localpath, QString remotePath = QString());
    void showAddSyncError(int errorCode, QString localpath, QString remotePath = QString());

    /**
     * @brief Migrate sync configuration to sdk cache
     * @param email of sync configuration to migrate from previous sessions
     */
    void migrateSyncConfToSdk(QString email = QString());

    mega::MegaApi *getMegaApi() { return megaApi; }
    mega::MegaApi *getMegaApiFolders() { return megaApiFolders; }
    std::unique_ptr<mega::MegaApiLock> megaApiLock;

    void cleanLocalCaches(bool all = false);
    void showInfoMessage(QString message, QString title = tr("MEGAsync"));
    void showWarningMessage(QString message, QString title = tr("MEGAsync"));
    void showErrorMessage(QString message, QString title = tr("MEGAsync"));
    void showNotificationMessage(QString message, QString title = tr("MEGAsync"));
    void setUploadLimit(int limit);
    void setMaxUploadSpeed(int limit);
    void setMaxDownloadSpeed(int limit);
    void setMaxConnections(int direction, int connections);
    void setUseHttpsOnly(bool httpsOnly);
    void startUpdateTask();
    void stopUpdateTask();
    void applyProxySettings();
    void updateUserStats(bool storage, bool transfer, bool pro, bool force, int source);
    void checkForUpdates();
    // Actually show InfoDialog view, not tray menu.
    void showTrayMenu(QPoint *point = NULL);
    // Create menus used in the app.
    void createAppMenus();
    // Create menus for the tray icon.
    void createTrayIconMenus();
    // Create menus for the "..." menu in InfoDialog view.
    void createInfoDialogMenus();
    void toggleLogging();
    QList<mega::MegaTransfer* > getFinishedTransfers();
    int getNumUnviewedTransfers();
    void removeFinishedTransfer(int transferTag);
    void removeAllFinishedTransfers();
    void showVerifyAccountInfo(std::function<void ()> func = nullptr);

    void removeFinishedBlockedTransfer(int transferTag);
    bool finishedTransfersWhileBlocked(int transferTag);

    mega::MegaTransfer* getFinishedTransferByTag(int tag);
    bool notificationsAreFiltered();
    bool hasNotifications();
    bool hasNotificationsOfType(int type);
    std::shared_ptr<mega::MegaNode> getRootNode(bool forceReset = false);
    std::shared_ptr<mega::MegaNode> getVaultNode(bool forceReset = false);
    std::shared_ptr<mega::MegaNode> getRubbishNode(bool forceReset = false);

    MegaSyncLogger& getLogger() const;
    void pushToThreadPool(std::function<void()> functor);
    QPointer<SetupWizard> getSetupWizard() const;

    TransfersModel* getTransfersModel(){return mTransfersModel;}
    StalledIssuesModel* getStalledIssuesModel(){return mStalledIssuesModel;}

    /**
     * @brief migrates sync configuration and fetches nodes
     * @param email of sync configuration to migrate from previous sessions. If present
     * syncs configured in previous sessions will be loaded.
     */
    void fetchNodes(QString email = QString());
    void whyAmIBlocked(bool periodicCall = false);
    QPointer<OverQuotaDialog> showSyncOverquotaDialog();
    bool finished() const;
    bool isInfoDialogVisible() const;

    int getBlockState() const;

    void updateTrayIconMenu();

    std::shared_ptr<mega::MegaPricing> getPricing() const;

    QuotaState getTransferQuotaState() const;
    std::shared_ptr<TransferQuota> getTransferQuota() const;

    int getAppliedStorageState() const;
    bool isAppliedStorageOverquota() const;
    void reloadSyncsInSettings();

    void raiseInfoDialog();
    bool isShellNotificationProcessingOngoing();

signals:
    void startUpdaterThread();
    void tryUpdate();
    void installUpdate();
    void clearAllFinishedTransfers();
    void clearFinishedTransfer(int transferTag);
    void fetchNodesAfterBlock();
    void setupWizardCreated();
    void unblocked();
    void nodeMoved(mega::MegaHandle handle);
    void nodeAttributesChanged(mega::MegaHandle handle);
    void blocked();
    void storageStateChanged(int);
    void pauseStateChanged();
    void addBackup();
    void shellNotificationsProcessed();

public slots:
    void unlink(bool keepLogs = false);
    void showInterface(QString);
    void trayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void onMessageClicked();
    void start();
    void openSettings(int tab = -1);
    void openSettingsAddSync(mega::MegaHandle megaFolderHandle);
    void openInfoWizard();
    void importLinksFromWidget(QWidget* parent);
    void importLinks();
    void officialWeb();
    void goToMyCloud();
    void pauseTransfers();
    void showChangeLog();
    void uploadActionClicked();
    void uploadActionClickedFromWidget(QWidget *openFrom);
    void uploadActionClickedFromWindowAfterOverQuotaCheck(QWidget *openFrom);
    void loginActionClicked();
    void downloadActionClicked();
    void downloadActionClickedFromWidget(QWidget *openFrom);
    void streamActionClicked();
    void transferManagerActionClicked(int tab = 0);
    void logoutActionClicked();
    void processDownloads();
    void processUploads();
    void shellUpload(QQueue<QString> newUploadQueue);
    void shellExport(QQueue<QString> newExportQueue);
    void shellViewOnMega(QByteArray localPath, bool versions);
    void shellViewOnMega(mega::MegaHandle handle, bool versions);
    void exportNodes(QList<mega::MegaHandle> exportList, QStringList extraLinks = QStringList());
    void externalDownload(QQueue<WrappedNode *> newDownloadQueue);
    void externalLinkDownload(QString megaLink, QString auth);
    void externalFileUpload(qlonglong targetFolder);
    void externalFolderUpload(qlonglong targetFolder);
    void externalFolderSync(qlonglong targetFolder);
    void externalAddBackup();
    void externalOpenTransferManager(int tab);
    void internalDownload(long long handle);
    void onLinkImportFinished();
    void onRequestLinksFinished();
    void onUpdateCompleted();
    void onUpdateAvailable(bool requested);
    void onInstallingUpdate(bool requested);
    void onUpdateNotFound(bool requested);
    void onUpdateError();
    void rebootApplication(bool update = true);
    void deleteSdkCache();
    void tryExitApplication(bool force = false);
    void highLightMenuEntry(QAction* action);
    void pauseTransfers(bool pause);
    void checkNetworkInterfaces();
    void checkMemoryUsage();
    void checkOverStorageStates();
    void checkOverQuotaStates();
    void periodicTasks();
    void cleanAll();
    void onInstallUpdateClicked();
    void onAboutClicked();
    void showInfoDialog();
    void showInfoDialogNotifications();
    void triggerInstallUpdate();
    void scanningAnimationStep();
    void setupWizardFinished(QPointer<SetupWizard> dialog);
    void clearDownloadAndPendingLinks();
    void infoWizardDialogFinished(QPointer<InfoWizard> dialog);
    void runConnectivityCheck();
    void onConnectivityCheckSuccess();
    void onConnectivityCheckError();
    void proExpirityTimedOut();
    void userAction(int action);
    void showSetupWizard(int action);
    void applyNotificationFilter(int opt);
    void changeState();
#ifdef _WIN32
    void changeDisplay(QScreen *disp);
#endif
    void showUpdatedMessage(int lastVersion);
    void handleMEGAurl(const QUrl &url);
    void handleLocalPath(const QUrl &url);
    void clearUserAttributes();
    void clearViewedTransfers();
    void onCompletedTransfersTabActive(bool active);
    void checkFirstTransfer();
    void checkOperatingSystem();
    void notifyChangeToAllFolders();
    int getPrevVersion();
    void onDismissStorageOverquota(bool overStorage);
    void showNotificationFinishedTransfers(unsigned long long appDataId);
    void renewLocalSSLcert();
    void onHttpServerConnectionError();
    void onGlobalSyncStateChangedTimeout();
    void onCheckDeferredPreferencesSyncTimeout();
    void updateStatesAfterTransferOverQuotaTimeHasExpired();
#ifdef __APPLE__
    void enableFinderExt();
#endif
private slots:
    void openFolderPath(QString path);
    void registerUserActivity();
    void PSAseen(int id);
    void onSyncStateChanged(std::shared_ptr<SyncSettings> syncSettings);
    void onSyncDeleted(std::shared_ptr<SyncSettings> syncSettings);
    void showSingleSyncDisabledNotification(std::shared_ptr<SyncSettings> syncSetting);
    void onBlocked();
    void onUnblocked();
    void onTransfersModelUpdate();

    void startingUpload();
    void cancelScanningStage();

protected slots:
    void onUploadsCheckedAndReady(QPointer<DuplicatedNodeDialog> checkDialog);
    void onPasteMegaLinksDialogFinish(QPointer<PasteMegaLinksDialog>);
    void onImportDialogFinish(QPointer<ImportMegaLinksDialog>);
    void onDownloadFromMegaFinished(QPointer<DownloadFromMegaDialog> dialog);

protected:
    void createTrayIcon();
    void createGuestMenu();
    bool showTrayIconAlwaysNEW();
    void loggedIn(bool fromWizard);
    void startSyncs(QList<PreConfiguredSync> syncs); //initializes syncs configured in the setup wizard
    void applyStorageState(int state, bool doNotAskForUserStats = false);
    void processUploadQueue(mega::MegaHandle nodeHandle);
    void processDownloadQueue(QString path);
    void disableSyncs();
    void restoreSyncs();
    void createTransferManagerDialog(TransfersWidget::TM_TAB tab);
    void calculateInfoDialogCoordinates(QDialog *dialog, int *posx, int *posy);
    void deleteMenu(QMenu *menu);
    void startHttpServer();
    void startHttpsServer();
    void initLocalServer();
    void refreshStorageUIs();
    void manageBusinessStatus(int64_t event);
    void requestUserData(); //groups user attributes retrieving, getting PSA, ... to be retrieved after login in
    void populateUserAlerts(mega::MegaUserAlertList *list, bool copyRequired);

    std::vector<std::unique_ptr<mega::MegaEvent>> eventsPendingLoggedIn;

    bool eventFilter(QObject *obj, QEvent *e) override;
    void createInfoDialog();

    QSystemTrayIcon *trayIcon;

    QAction *guestSettingsAction;
    QAction *initialExitAction;
    QPointer<QMenu> initialTrayMenu;

#ifdef _WIN32
    QPointer<QMenu> windowsMenu;
    QAction *windowsExitAction;
    QAction *windowsUpdateAction;
    QAction *windowsAboutAction;
    QAction *windowsImportLinksAction;
    QAction *windowsUploadAction;
    QAction *windowsDownloadAction;
    QAction *windowsStreamAction;
    QAction *windowsTransferManagerAction;
    QAction *windowsSettingsAction;
#endif

    QPointer<QMenu> infoDialogMenu;
    QPointer<QMenu> guestMenu;
    QMenu emptyMenu;

    MenuItemAction *exitAction;
    MenuItemAction *settingsAction;
    MenuItemAction *importLinksAction;
    MenuItemAction *uploadAction;
    MenuItemAction *downloadAction;
    MenuItemAction *streamAction;
    MenuItemAction *myCloudAction;
    MenuItemAction *updateAction;
    MenuItemAction *aboutAction;
    QAction *showStatusAction;
    QPointer<SyncsMenu> mSyncs2waysMenu;
    QPointer<SyncsMenu> mBackupsMenu;

    MenuItemAction *exitActionGuest;
    MenuItemAction *settingsActionGuest;
    MenuItemAction *updateActionGuest;
    MenuItemAction* lastHovered;

#ifdef __APPLE__
    QTimer *scanningTimer;
#endif

    QTimer *connectivityTimer;
    std::unique_ptr<QTimer> onGlobalSyncStateChangedTimer;
    std::unique_ptr<QTimer> onDeferredPreferencesSyncTimer;
    QTimer proExpirityTimer;
    int scanningAnimationIndex;
    QPointer<SetupWizard> mSetupWizard;
    QPointer<SettingsDialog> mSettingsDialog;
    QPointer<InfoDialog> infoDialog;
    std::shared_ptr<Preferences> preferences;
    SyncInfo *model;
    mega::MegaApi *megaApi;
    mega::MegaApi *megaApiFolders;
    QFilterAlertsModel *notificationsProxyModel;
    QAlertsModel *notificationsModel;
    MegaAlertDelegate *notificationsDelegate;
    QObject *context;
    QString crashReportFilePath;

    HTTPServer *httpServer;
    HTTPServer *httpsServer;
    long long lastTsConnectionError = 0;
    mega::MegaHandle fileUploadTarget;
    mega::MegaHandle folderUploadTarget;

    QQueue<QString> uploadQueue;
    QQueue<WrappedNode *> downloadQueue;
    BlockingBatch mBlockingBatch;
    std::vector<std::shared_ptr<mega::MegaCancelToken>> mUnblockedCancelTokens;

    ThreadPool* mThreadPool;
    std::shared_ptr<mega::MegaNode> mRootNode;
    std::shared_ptr<mega::MegaNode> mVaultNode;
    std::shared_ptr<mega::MegaNode> mRubbishNode;
    bool mFetchingNodes = false;
    bool mQueringWhyAmIBlocked = false;
    bool queuedUserStats[3];
    int queuedStorageUserStatsReason;
    long long userStatsLastRequest[3];
    bool inflightUserStats[3];
    long long cleaningSchedulerExecution;
    long long lastUserActivityExecution;
    long long lastTsBusinessWarning;
    long long lastTsErrorMessageShown;
    int storageState;
    int appliedStorageState;
    bool getUserDataRequestReady;
    long long receivedStorageSum;
    long long maxMemoryUsage;
    int exportOps;
    int syncState;
    std::shared_ptr<mega::MegaPricing> mPricing;
    std::shared_ptr<mega::MegaCurrency> mCurrency;
    QPointer<UpgradeOverStorage> mStorageOverquotaDialog;
    mega::QTMegaListener *delegateListener;
    MegaUploader *uploader;
    MegaDownloader *downloader;
    QTimer *periodicTasksTimer;
    QTimer *networkCheckTimer;
    QTimer *infoDialogTimer;
    QTimer *firstTransferTimer;
    std::unique_ptr<std::thread> mMutexStealerThread;

    QTranslator translator;
    std::shared_ptr<LinkProcessor> mLinkProcessor;
    QString lastTrayMessage;
    QStringList extraLinks;
    QString currentLanguageCode;

    static QString appPath;
    static QString appDirPath;
    static QString dataPath;
    static QString lastNotificationError;

    QThread *updateThread;
    UpdateTask *updateTask;
    long long lastActiveTime;
    QList<QNetworkInterface> activeNetworkInterfaces;
    QMap<QString, QString> pendingLinks;
    std::unique_ptr<MegaSyncLogger> logger;
    QPointer<TransferManager> mTransferManager;
    bool mTransferManagerFullScreen;
    QMap<int, mega::MegaTransfer*> finishedTransfers;
    QList<mega::MegaTransfer*> finishedTransferOrder;
    QSet<int> finishedBlockedTransfers;

    bool reboot;
    bool syncActive;
    bool paused;
    bool indexing;
    bool waiting;
    bool syncing; //if any sync is in syncing state
    bool syncStalled = false;
    bool updated;
    bool transferring; //if there is any regular transfer in progress
    bool checkupdate;
    bool updateBlocked;
    long long lastExit;
    bool appfinished;
    bool updateAvailable;
    bool isLinux;
    bool mIsFirstFileTwoWaySynced;
    bool mIsFirstFileBackedUp;
    bool networkConnectivity;
    int nUnviewedTransfers;
    bool completedTabActive;
    int prevVersion;
    bool isPublic;
    bool updatingSSLcert;
    long long lastSSLcertUpdate;
    bool nodescurrent;
    int businessStatus = -2;
    int blockState;
    bool blockStateSet = false;
    bool whyamiblockedPeriodicPetition = false;
    friend class DeferPreferencesSyncForScope;
    std::shared_ptr<TransferQuota> mTransferQuota;
    bool transferOverQuotaWaitTimeExpiredReceived;
    std::shared_ptr<DesktopNotifications> mOsNotifications;
    QMutex mMutexOpenUrls;
    QMap<QString, std::chrono::system_clock::time_point> mOpenUrlsClusterTs;

    std::unique_ptr<SyncController> mSyncController;

    QPointer<TransfersModel> mTransfersModel;

    ScanStageController scanStageController;
    std::shared_ptr<FolderTransferListener> mFolderTransferListener;

    bool mDisableGfx;
    StalledIssuesModel* mStalledIssuesModel;

private:
    void loadSyncExclusionRules(QString email = QString());

    static long long computeExclusionSizeLimit(const long long sizeLimitValue, const int unit);

    QList<QNetworkInterface> findNewNetworkInterfaces();
    bool checkNetworkInterfaces(const QList<QNetworkInterface>& newNetworkInterfaces) const;
    bool checkNetworkInterface(const QNetworkInterface& newNetworkInterface) const;
    bool checkNetworkAddresses(const QNetworkInterface& oldNetworkInterface, const QNetworkInterface &newNetworkInterface) const;
    bool checkIpAddress(const QHostAddress& ip, const QList<QNetworkAddressEntry>& oldAddresses, const QString& newNetworkInterfaceName) const;
    static bool isActiveNetworkInterface(const QString& interfaceName, const QNetworkInterface::InterfaceFlags flags);
    int countActiveIps(const QList<QNetworkAddressEntry>& addresses) const;
    static bool isLocalIpv4(const QString& address);
    static bool isLocalIpv6(const QString& address);
    void logIpAddress(const char *message, const QHostAddress &ipAddress) const;

    QString obfuscateIfNecessary(const QHostAddress& ipAddress) const;
    static QString obfuscateAddress(const QHostAddress& ipAddress);
    static QString obfuscateIpv4Address(const QHostAddress& ipAddress);
    static QString obfuscateIpv6Address(const QHostAddress& ipAddress);
    static QStringList explodeIpv6(const QHostAddress &ipAddress);

    void reconnectIfNecessary(const bool disconnected, const QList<QNetworkInterface>& newNetworkInterfaces);
    bool isIdleForTooLong() const;

    void startUpload(const QString& rawLocalPath, mega::MegaNode* target, mega::MegaCancelToken *cancelToken);

    void updateTransferNodesStage(mega::MegaTransfer* transfer);

    void updateFileTransferBatchesAndUi(const QString &nodePath, BlockingBatch& batch);
    void updateFolderTransferBatchesAndUi(const QString &nodePath, BlockingBatch& batch, bool fromCancellation);
    void updateIfBlockingStageFinished(BlockingBatch &batch, bool fromCancellation);
    void unblockBatch(BlockingBatch &batch);

    void logBatchStatus(const char* tag);

    void enableTransferActions(bool enable);

    void updateFreedCancelToken(mega::MegaTransfer* transfer);

    bool noUploadedStarted = true;
    bool mProcessingUploadQueue = false;
    int mProcessingShellNotifications = 0;

    void ConnectServerSignals(HTTPServer* server);

    static QString RectToString(const QRect& rect);

    void fixMultiscreenResizeBug(int& posX, int& posY);

    static void logInfoDialogCoordinates(const char* message, const QRect& screenGeometry, const QString& otherInformation);

    bool dontAskForExitConfirmation(bool force);
    void exitApplication();

    QString getDefaultUploadPath();

    struct NodeCount
    {
        int files;
        int folders;
    };

    BlockingStageProgressController transferProgressController;

    static NodeCount countFilesAndFolders(const QStringList& paths);

    void processUploads(const QStringList& uploads);

    void updateMetadata(TransferMetaData* data, const QString& filePath);

    bool isQueueProcessingOngoing();

    template <class Func>
    void recreateMenuAction(MenuItemAction** action, const QString& actionName,
                            const char* iconPath, Func slotFunc)
    {
        bool previousEnabledState = true;
        if (*action)
        {
            previousEnabledState = (*action)->isEnabled();
            (*action)->deleteLater();
            *action = nullptr;
        }

        *action = new MenuItemAction(actionName, QIcon(QString::fromUtf8(iconPath)), true);
        connect(*action, &QAction::triggered, this, slotFunc, Qt::QueuedConnection);
        (*action)->setEnabled(previousEnabledState);
    }

    template <class Func>
    void recreateAction(QAction** action, const QString& actionName, Func slotFunc)
    {
        bool previousEnabledState = true;
        if (*action)
        {
            previousEnabledState = (*action)->isEnabled();
            (*action)->deleteLater();
            *action = nullptr;
        }

        *action = new QAction(actionName, this);
        connect(*action, &QAction::triggered, this, slotFunc);
        (*action)->setEnabled(previousEnabledState);
    }

    void processUpgradeSecurityEvent();

private slots:
    void onFolderTransferUpdate(FolderTransferUpdateEvent event);
    void onNotificationProcessed();
};

class DeferPreferencesSyncForScope
{
    // This class is provided as an easy way to avoid updating the preferences file so often that it becomes a performance issue
    // eg. when 1000 transfers all have a temporary error callback at once.
    // It causes sync() to set a flag instead of actually rewriting the file, and the app will start a timer
    // to do the actual sync() in 100ms instead.   Any other sync() calls (that are also protected by this class) in the meantime are effectively skipped.
    MegaApplication* app;

public:
    DeferPreferencesSyncForScope(MegaApplication* a) : app(a)
    {
        app->preferences->deferSyncs(true);
    }

    ~DeferPreferencesSyncForScope()
    {
        app->preferences->deferSyncs(false);
        app->onCheckDeferredPreferencesSync(false);
    }
};

class MEGASyncDelegateListener: public mega::QTMegaListener
{
public:
    MEGASyncDelegateListener(mega::MegaApi *megaApi, mega::MegaListener *parent = NULL, MegaApplication *app = NULL);
    void onRequestFinish(mega::MegaApi* api, mega::MegaRequest *request, mega::MegaError* e) override;
    void onEvent(mega::MegaApi *api, mega::MegaEvent *e) override;

protected:
    MegaApplication *app;
};

#endif // MEGAAPPLICATION_H

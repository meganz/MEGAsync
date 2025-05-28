#ifndef MEGAAPPLICATION_H
#define MEGAAPPLICATION_H

#include "AppState.h"
#include "BlockingStageProgressController.h"
#include "DesktopNotifications.h"
#include "DownloadFromMegaDialog.h"
#include "DuplicatedNodeInfo.h"
#include "HTTPServer.h"
#include "InfoDialog.h"
#include "LinkProcessor.h"
#include "megaapi.h"
#include "MegaDownloader.h"
#include "MegaSyncLogger.h"
#include "MegaUploader.h"
#include "PasteMegaLinksDialog.h"
#include "Preferences.h"
#include "QTMegaListener.h"
#include "ScanStageController.h"
#include "SetManager.h"
#include "SettingsDialog.h"
#include "SyncInfo.h"
#include "ThreadPool.h"
#include "TransferManager.h"
#include "TransferQuota.h"
#include "UpdateTask.h"
#include "UpsellPlans.h"
#include "Utilities.h"

#include <QAction>
#include <QApplication>
#include <QDataStream>
#include <QDir>
#include <QFutureWatcher>
#include <QLocalServer>
#include <QLocalSocket>
#include <QMenu>
#include <QNetworkInterface>
#include <QQueue>
#include <QSystemTrayIcon>

#include <memory>

class IntervalExecutioner;
class TransfersModel;
class StalledIssuesModel;

#ifdef __APPLE__
    #include <mach/mach.h>
    #include <sys/sysctl.h>
#endif

Q_DECLARE_METATYPE(QQueue<QString>)

class LogoutController;
class TransferMetadata;
class DuplicatedNodeDialog;
class LoginController;
class AccountStatusController;
class StatsEventHandler;
class UserMessageController;
class SyncReminderNotificationManager;

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

class MegaApplication : public QApplication, public mega::MegaListener
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

    QString getFormattedDateByCurrentLanguage(const QDateTime& datetime, QLocale::FormatType format = QLocale::FormatType::LongFormat) const;

    void onEvent(mega::MegaApi *api, mega::MegaEvent *event) override;
    void onRequestFinish(mega::MegaApi* api, mega::MegaRequest *request, mega::MegaError* e) override;
    void onTransferStart(mega::MegaApi *api, mega::MegaTransfer *transfer) override;
    void onTransferFinish(mega::MegaApi* api, mega::MegaTransfer *transfer, mega::MegaError* e) override;
    void onTransferUpdate(mega::MegaApi *api, mega::MegaTransfer *transfer) override;
    void onTransferTemporaryError(mega::MegaApi *api, mega::MegaTransfer *transfer, mega::MegaError* e) override;
    void onAccountUpdate(mega::MegaApi *api) override;
    void onUsersUpdate(mega::MegaApi* api, mega::MegaUserList *users) override;
    void onNodesUpdate(mega::MegaApi* api, mega::MegaNodeList* nodes) override;
    void onGlobalSyncStateChanged(mega::MegaApi *api) override;

    void onGlobalSyncStateChangedImpl();

    void showAddSyncError(mega::MegaRequest *request, mega::MegaError* e, QString localpath, QString remotePath = QString());
    void showAddSyncError(int errorCode, QString localpath, QString remotePath = QString());

    /**
     * @brief Migrate sync configuration to sdk cache
     * @param email of sync configuration to migrate from sprevious sessions
     */
    void migrateSyncConfToSdk(QString email = QString());

    mega::MegaApi *getMegaApi() { return megaApi; }
    mega::MegaApi *getMegaApiFolders() { return megaApiFolders; }
    std::unique_ptr<mega::MegaApiLock> megaApiLock;

    QString getMEGAString(){return QLatin1String("MEGA");}

    void cleanLocalCaches(bool all = false);
    void showInfoMessage(QString message, QString title = MegaSyncApp->getMEGAString());
    void showInfoMessage(DesktopNotifications::NotificationInfo info);
    void showWarningMessage(QString message, QString title = MegaSyncApp->getMEGAString());
    void showErrorMessage(QString message, QString title = MegaSyncApp->getMEGAString());
    void showNotificationMessage(QString message, QString title = MegaSyncApp->getMEGAString());
    void setMaxUploadSpeed(int limit);
    void setMaxDownloadSpeed(int limit);
    void setMaxConnections(int direction, int connections);
    void setUseHttpsOnly(bool httpsOnly);
    void startUpdateTask();
    void stopUpdateTask();
    void applyProxySettings();
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

    std::shared_ptr<mega::MegaNode> getRootNode(bool forceReset = false);
    std::shared_ptr<mega::MegaNode> getVaultNode(bool forceReset = false);
    std::shared_ptr<mega::MegaNode> getRubbishNode(bool forceReset = false);
    void resetRootNodes();
    void initLocalServer();
    void onboardingFinished(bool fastLogin);
    void onLoginFinished();
    void onFetchNodesFinished();
    void onLogout();

    StatsEventHandler* getStatsEventHandler() const;

    MegaSyncLogger& getLogger() const;
    void pushToThreadPool(std::function<void()> functor);

    TransfersModel* getTransfersModel(){return mTransfersModel;}
    StalledIssuesModel* getStalledIssuesModel(){return mStalledIssuesModel;}
    UserMessageController* getNotificationController() { return mUserMessageController.get(); }

    /**
     * @brief migrates sync configuration and fetches nodes
     * @param email of sync configuration to migrate from previous sessions. If present
     * syncs configured in previous sessions will be loaded.
     */
    QPointer<OverQuotaDialog> showSyncOverquotaDialog();
    bool finished() const;
    bool isInfoDialogVisible() const;

    void requestUserData(); //groups user attributes retrieving, getting PSA, ... to be retrieved after login in

    void updateTrayIconMenu();

    std::shared_ptr<TransferQuota> getTransferQuota() const;

    int getAppliedStorageState() const;
    bool isAppliedStorageOverquota() const;
    void reloadSyncsInSettings();

    void raiseInfoDialog();
    bool isShellNotificationProcessingOngoing();

    QSystemTrayIcon* getTrayIcon();
    LoginController* getLoginController();
    AccountStatusController* getAccountStatusController();

    void updateUsedStorage(const bool sendEvent = false);
    void showUpsellDialog(UpsellPlans::ViewMode viewMode);

signals:
    void startUpdaterThread();
    void tryUpdate();
    void installUpdate();
    void clearAllFinishedTransfers();
    void fetchNodesAfterBlock();
    void unblocked();
    void nodeMoved(mega::MegaHandle handle);
    void nodeAttributesChanged(mega::MegaHandle handle);
    void blocked();
    void storageStateChanged(int);
    void pauseStateChanged();
    void addBackup();
    void shellNotificationsProcessed();
    void updateUserInterface();
    void requestAppState(AppState::AppStates newAppState);
    void syncsDialogClosed();
    void languageChanged();

public slots:
    void updateTrayIcon();
    void unlink(bool keepLogs = false);
    void showInterface(QString);
    void trayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void onMessageClicked();
    void start();
    void openSettings(int tab = -1);
    void openSettingsAddSync(mega::MegaHandle megaFolderHandle);
    void importLinks();
    void officialWeb();
    void goToMyCloud();
    void goToFiles();
    void openDeviceCentre();
    void pauseTransfers();
    void showChangeLog();
    void uploadActionClicked();
    void uploadActionFromWindowAfterOverQuotaCheck();
    void runUploadActionWithTargetHandle(const mega::MegaHandle &targetFolder, QWidget *parent);
    void downloadActionClicked();
    void downloadACtionClickedWithHandles(const QList<mega::MegaHandle>& handles);
    void streamActionClicked();
    void transferManagerActionClicked(int tab = 0);
    void logoutActionClicked();
    void processDownloads();
    void processSetDownload(const QString& publicLink, const QList<mega::MegaHandle>& elementHandleList);
    void processUploads();
    void shellUpload(QQueue<QString> newUploadQueue);
    void shellExport(QQueue<QString> newExportQueue);
    void shellViewOnMega(QByteArray localPath, bool versions);
    void shellViewOnMega(mega::MegaHandle handle, bool versions);
    void exportNodes(QList<mega::MegaHandle> exportList, QStringList extraLinks = QStringList());
    void uploadFilesToNode(const QList<QUrl>& files, mega::MegaHandle targetNode, QWidget* caller);
    void externalDownload(QQueue<WrappedNode> newDownloadQueue);
    void externalLinkDownload(QString megaLink, QString auth);
    void externalFileUpload(mega::MegaHandle targetFolder);
    void externalFolderUpload(mega::MegaHandle targetFolder);
    void externalFolderSync(mega::MegaHandle targetFolder);
    void externalAddBackup();
    void externalOpenTransferManager(int tab);
    void onRequestLinksFinished();
    void onUpdateCompleted();
    void onUpdateAvailable(bool requested);
    void onInstallingUpdate(bool requested);
    void onUpdateNotFound(bool requested);
    void onUpdateError();
    void rebootApplication(bool update = true);
    void tryExitApplication(bool force = false);
    void highLightMenuEntry(QAction* action);
    void pauseTransfers(bool pause);
    void checkNetworkInterfaces();
    void checkMemoryUsage();
    void checkOverStorageStates(bool isOnboardingAboutClosing = false);
    void checkOverQuotaStates();
    void periodicTasks();
    void cleanAll();
    void onInstallUpdateClicked();
    void onAboutClicked();
    void showInfoDialog();
    void showInfoDialogNotifications();
    void triggerInstallUpdate();
    void scanningAnimationStep();
    void clearDownloadAndPendingLinks();
    void changeState();

#ifdef _WIN32
    void changeDisplay(QScreen *disp);
#endif
    void showUpdatedMessage(int lastVersion);
    void handleMEGAurl(const QUrl &url);
    void handleLocalPath(const QUrl &url);
    void clearUserAttributes();
    void checkOperatingSystem();
    void notifyChangeToAllFolders();
    int getPrevVersion();
    void onDismissStorageOverquota(bool overStorage);
    void showNotificationFinishedTransfers(unsigned long long appDataId);
    void transferBatchFinished(unsigned long long appDataId, bool fromCancellation);
    void updateStatesAfterTransferOverQuotaTimeHasExpired();
#ifdef __APPLE__
    void enableFinderExt();
#endif
    void requestFetchSetFromLink(const QString& link);
    void onAppStateChanged(AppState::AppStates, AppState::AppStates);

private slots:
    void openFolderPath(QString path);
    void registerUserActivity();
    void PSAseen(int id);
    void onSyncModelUpdated(std::shared_ptr<SyncSettings> syncSettings);
    void onBlocked();
    void onUnblocked();
    void onTransfersModelUpdate();

    void startingUpload();
    void cancelScanningStage();

protected slots:
    void onUploadsCheckedAndReady(std::shared_ptr<ConflictTypes> conflicts);
    void onPasteMegaLinksDialogFinish(QPointer<PasteMegaLinksDialog>);
    void onDownloadFromMegaFinished(QPointer<DownloadFromMegaDialog> dialog);
    void onDownloadSetFolderDialogFinished(QPointer<DownloadFromMegaDialog> dialog);

protected:
    void createTrayIcon();
    void createGuestMenu();
    bool showTrayIconAlwaysNEW();
    void applyStorageState(int state, bool doNotAskForUserStats = false);
    void processUploadQueue(mega::MegaHandle nodeHandle, QWidget* caller = nullptr);
    void processDownloadQueue(QString path);
    void disableSyncs();
    void restoreSyncs();
    void createTransferManagerDialog(TransfersWidget::TM_TAB tab);
    void deleteMenu(QMenu* menu);
    void clearMenu(QMenu* menu, bool deleteAction = false);
    void startHttpServer();
    void startHttpsServer();
    void refreshStorageUIs();
    void manageBusinessStatus(int64_t event);

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
    QAction *windowsFilesAction;
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
    MenuItemAction* filesAction;
    MenuItemAction* MEGAWebAction;
    MenuItemAction* deviceCentreAction;
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

    std::unique_ptr<QTimer> onGlobalSyncStateChangedTimer;
    int scanningAnimationIndex;
    QPointer<SettingsDialog> mSettingsDialog;
    QPointer<InfoDialog> infoDialog;
    std::shared_ptr<Preferences> preferences;
    SyncInfo *model;
    mega::MegaApi *megaApi;
    mega::MegaApi* megaApiFolders;

    HTTPServer *httpServer;
    mega::MegaHandle fileUploadTarget;
    mega::MegaHandle folderUploadTarget;

    QQueue<QString> uploadQueue;
    QQueue<WrappedNode> downloadQueue;
    BlockingBatch mBlockingBatch;

    ThreadPool* mThreadPool;
    std::shared_ptr<mega::MegaNode> mRootNode;
    std::shared_ptr<mega::MegaNode> mVaultNode;
    std::shared_ptr<mega::MegaNode> mRubbishNode;
    long long cleaningSchedulerExecution;
    long long lastUserActivityExecution;
    long long lastTsBusinessWarning;
    long long lastTsErrorMessageShown;
    int storageState;
    int appliedStorageState;
    bool getUserDataRequestReady;
    long long receivedStorageSum;
    unsigned long long mMaxMemoryUsage;
    int exportOps;
    mega::QTMegaListener *delegateListener;
    MegaUploader *uploader;
    MegaDownloader *downloader;
    QTimer *periodicTasksTimer;
    QTimer *networkCheckTimer;
    QTimer *infoDialogTimer;
    std::unique_ptr<std::thread> mMutexStealerThread;

    QTranslator translator;
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

    bool reboot;
    bool paused;
    bool mIndexing;
    bool mWaiting;
    bool mSyncing; //if any sync is in syncing state
    bool mSyncStalled = false;
    bool updated;
    bool mTransferring; //if there is any regular transfer in progress
    bool checkupdate;
    bool updateBlocked;
    long long lastExit;
    bool appfinished;
    bool updateAvailable;
    bool isLinux;
    bool mIsFirstFileTwoWaySynced;
    bool mIsFirstFileBackedUp;
    bool networkConnectivity;
    int prevVersion;
    bool isPublic;
    bool nodescurrent;
    int businessStatus = -2;
    bool whyamiblockedPeriodicPetition = false;
    LoginController* mLoginController;
    friend class DeferPreferencesSyncForScope;
    std::shared_ptr<TransferQuota> mTransferQuota;
    bool transferOverQuotaWaitTimeExpiredReceived;
    std::shared_ptr<DesktopNotifications> mOsNotifications;
    AccountStatusController* mStatusController;
    QMutex mMutexOpenUrls;
    QMap<QString, std::chrono::system_clock::time_point> mOpenUrlsClusterTs;

    LogoutController* mLogoutController;

    QPointer<TransfersModel> mTransfersModel;

    ScanStageController scanStageController;
    std::shared_ptr<FolderTransferListener> mFolderTransferListener;

    StalledIssuesModel* mStalledIssuesModel;
    std::unique_ptr<StatsEventHandler> mStatsEventHandler;

    SetManager* mSetManager;
    LinkProcessor* mLinkProcessor;

    QString mLinkToPublicSet;
    QList<mega::MegaHandle> mElementHandleList;
    std::unique_ptr<IntervalExecutioner> mIntervalExecutioner;
    bool mDisableGfx;

    std::unique_ptr<UserMessageController> mUserMessageController;

    std::unique_ptr<mega::MegaGfxProvider> mGfxProvider;

    QPointer<SyncReminderNotificationManager> mSyncReminderNotificationManager;

    bool misSyncingStateWrongLogged;

private:
    void loadSyncExclusionRules(QString email = QString());

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

    static bool mightBeCaseSensitivityIssue(const QString& folderPath);

    void reconnectIfNecessary(const bool disconnected, const QList<QNetworkInterface>& newNetworkInterfaces);
    bool isIdleForTooLong() const;

    void startUpload(const QString& rawLocalPath, mega::MegaNode* target, mega::MegaCancelToken *cancelToken);

    void updateTransferNodesStage(mega::MegaTransfer* transfer);

    void logBatchStatus(const char* tag);

    void enableTransferActions(bool enable);

    bool noUploadedStarted = true;
    int mProcessingShellNotifications = 0;

    void ConnectServerSignals(HTTPServer* server);

    bool dontAskForExitConfirmation(bool force);
    void exitApplication();
    void openFirstActiveSync();

    QString getDefaultUploadPath();

    void checkSystemTray();

    struct NodeCount
    {
        int files;
        int folders;
    };

    BlockingStageProgressController transferProgressController;

    static NodeCount countFilesAndFolders(const QStringList& paths);

    void processUploads(const QStringList& uploads);

    void updateMetadata(TransferMetaData* data, const QString& filePath);

    template <class Func>
    void recreateMenuAction(MenuItemAction** action, QMenu* menu, const QString& actionName,
                            const char* iconPath, Func slotFunc)
    {
        bool previousEnabledState = true;
        if (*action)
        {
            previousEnabledState = (*action)->isEnabled();
            (*action)->deleteLater();
            *action = nullptr;
        }

        *action = new MenuItemAction(actionName, QLatin1String(iconPath), menu);
        (*action)->setManagesHoverStates(true);
        connect(*action, &QAction::triggered, this, slotFunc, Qt::QueuedConnection);
        (*action)->setEnabled(previousEnabledState);
    }

    template<class Func>
    void recreateAction(QAction** action,
                        QMenu* menu,
                        const QString& actionName,
                        Func slotFunc,
                        const QString& iconPath = QString())
    {
        bool previousEnabledState = true;
        if (*action)
        {
            previousEnabledState = (*action)->isEnabled();
            (*action)->deleteLater();
            *action = nullptr;
        }

        *action = new QAction(actionName, menu);
        connect(*action, &QAction::triggered, this, slotFunc);
        (*action)->setEnabled(previousEnabledState);

        if (!iconPath.isEmpty())
        {
            (*action)->setIcon(QIcon(iconPath));
        }
    }

    void processUpgradeSecurityEvent();
    QQueue<QString> createQueue(const QStringList& newUploads) const;

    bool hasDefaultDownloadFolder() const;
    void showInfoDialogIfHTTPServerSender();

    void sendPeriodicStats() const;

    void createUserMessageController();
    void closeUpsellStorageDialog();

    void createGfxProvider(const QString& basePath);
    void startCrashReportingDialog();

    void removeSyncsAndBackupsMenus();

private slots:
    void onFolderTransferUpdate(FolderTransferUpdateEvent event);
    void onNotificationProcessed();
    void onScheduledExecution();
    void onCopyLinkError(const QString& nodeName, const int errorCode);
};

#endif // MEGAAPPLICATION_H

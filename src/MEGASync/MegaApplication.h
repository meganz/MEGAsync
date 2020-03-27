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
#include <QNetworkConfigurationManager>
#include <QNetworkInterface>
#include <memory>

#include "gui/TransferManager.h"
#include "gui/NodeSelector.h"
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
#include "gui/UpgradeDialog.h"
#include "gui/InfoWizard.h"
#include "control/Preferences.h"
#include "control/HTTPServer.h"
#include "control/MegaUploader.h"
#include "control/MegaDownloader.h"
#include "control/UpdateTask.h"
#include "control/MegaSyncLogger.h"
#include "megaapi.h"
#include "QTMegaListener.h"
#include "QFilterAlertsModel.h"
#include "gui/MegaAlertDelegate.h"
#include "gui/VerifyEmailMessage.h"

#ifdef __APPLE__
    #include "gui/MegaSystemTrayIcon.h"
    #include <mach/mach.h>
    #include <sys/sysctl.h>
    #include <errno.h>
#endif

Q_DECLARE_METATYPE(QQueue<QString>)

class TransferMetaData
{
public:
    TransferMetaData(int direction, int total = 0, int pending = 0, QString path = QString())
                    : transferDirection(direction), totalTransfers(total), pendingTransfers(pending),
                      localPath(path), totalFiles(0), totalFolders(0),
                      transfersFileOK(0), transfersFolderOK(0),
                      transfersFailed(0), transfersCancelled(0) {}

    int totalTransfers;
    int pendingTransfers;
    int totalFiles;
    int totalFolders;
    int transfersFileOK;
    int transfersFolderOK;
    int transfersFailed;
    int transfersCancelled;
    int transferDirection;
    QString localPath;
};

class Notificator;
class MEGASyncDelegateListener;

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

    virtual void onEvent(mega::MegaApi *api, mega::MegaEvent *event);
    virtual void onRequestStart(mega::MegaApi* api, mega::MegaRequest *request);
    virtual void onRequestFinish(mega::MegaApi* api, mega::MegaRequest *request, mega::MegaError* e);
    virtual void onRequestTemporaryError(mega::MegaApi *api, mega::MegaRequest *request, mega::MegaError* e);
    virtual void onTransferStart(mega::MegaApi *api, mega::MegaTransfer *transfer);
    virtual void onTransferFinish(mega::MegaApi* api, mega::MegaTransfer *transfer, mega::MegaError* e);
    virtual void onTransferUpdate(mega::MegaApi *api, mega::MegaTransfer *transfer);
    virtual void onTransferTemporaryError(mega::MegaApi *api, mega::MegaTransfer *transfer, mega::MegaError* e);
    virtual void onAccountUpdate(mega::MegaApi *api);
    virtual void onUserAlertsUpdate(mega::MegaApi *api, mega::MegaUserAlertList *list);
    virtual void onUsersUpdate(mega::MegaApi* api, mega::MegaUserList *users);
    virtual void onNodesUpdate(mega::MegaApi* api, mega::MegaNodeList *nodes);
    virtual void onReloadNeeded(mega::MegaApi* api);
    virtual void onGlobalSyncStateChanged(mega::MegaApi *api, bool timeout = false);
    virtual void onSyncStateChanged(mega::MegaApi *api,  mega::MegaSync *sync);
    virtual void onSyncFileStateChanged(mega::MegaApi *api, mega::MegaSync *sync, std::string *localPath, int newState);
    virtual void onCheckDeferredPreferencesSync(bool timeout);

    mega::MegaApi *getMegaApi() { return megaApi; }

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
    void addRecentFile(QString fileName, long long fileHandle, QString localPath = QString(), QString nodeKey = QString());
    void checkForUpdates();
    void showTrayMenu(QPoint *point = NULL);
    void createAppMenus();
    void toggleLogging();
    QList<mega::MegaTransfer* > getFinishedTransfers();
    int getNumUnviewedTransfers();
    void removeFinishedTransfer(int transferTag);
    void removeAllFinishedTransfers();
    void showVerifyAccountInfo();
    mega::MegaTransfer* getFinishedTransferByTag(int tag);

    TransferMetaData* getTransferAppData(unsigned long long appDataID);

    bool notificationsAreFiltered();
    bool hasNotifications();
    bool hasNotificationsOfType(int type);
    std::shared_ptr<mega::MegaNode> getRootNode(bool forceReset = false);

    MegaSyncLogger& getLogger() const;
    void fetchNodes();


signals:
    void startUpdaterThread();
    void tryUpdate();
    void installUpdate();
    void unityFixSignal();
    void clearAllFinishedTransfers();
    void clearFinishedTransfer(int transferTag);
    void fetchNodesAfterBlock();

public slots:
    void unlink();
    void showInterface(QString);
    void trayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void onMessageClicked();
    void start();
    void openSettings(int tab = -1);
    void openInfoWizard();
    void openBwOverquotaDialog();
    void importLinks();
    void officialWeb();
    void goToMyCloud();
    void pauseTransfers();
    void showChangeLog();
    void uploadActionClicked();
    void loginActionClicked();
    void copyFileLink(mega::MegaHandle fileHandle, QString nodeKey = QString());
    void downloadActionClicked();
    void streamActionClicked();
    void transferManagerActionClicked(int tab = 0);
    void logoutActionClicked();
    void processDownloads();
    void processUploads();
    void shellUpload(QQueue<QString> newUploadQueue);
    void shellExport(QQueue<QString> newExportQueue);
    void shellViewOnMega(QByteArray localPath, bool versions);
    void exportNodes(QList<mega::MegaHandle> exportList, QStringList extraLinks = QStringList());
    void externalDownload(QQueue<mega::MegaNode *> newDownloadQueue);
    void externalDownload(QString megaLink, QString auth);
    void externalFileUpload(qlonglong targetFolder);
    void externalFolderUpload(qlonglong targetFolder);
    void externalFolderSync(qlonglong targetFolder);
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
    void exitApplication(bool force = false);
    void highLightMenuEntry(QAction* action);
    void pauseTransfers(bool pause);
    void checkNetworkInterfaces();
    void checkMemoryUsage();
    void checkOverStorageStates();
    void periodicTasks();
    void cleanAll();
    void onDupplicateLink(QString link, QString name, mega::MegaHandle handle);
    void onInstallUpdateClicked();
    void showInfoDialog();
    void triggerInstallUpdate();
    void scanningAnimationStep();
    void setupWizardFinished(int result);
    void overquotaDialogFinished(int result);
    void infoWizardDialogFinished(int result);
    void runConnectivityCheck();
    void onConnectivityCheckSuccess();
    void onConnectivityCheckError();
    void proExpirityTimedOut();
    void userAction(int action);
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
    void notifyItemChange(QString path, int newState);
    int getPrevVersion();
    void onDismissOQ(bool overStorage);
    void showNotificationFinishedTransfers(unsigned long long appDataId);
    void renewLocalSSLcert();
    void onHttpServerConnectionError();
    void onGlobalSyncStateChangedTimeout();
    void onCheckDeferredPreferencesSyncTimeout();
#ifdef __APPLE__
    void enableFinderExt();
#endif
private slots:
    void showInFolder(int activationButton);
    void openFolderPath(QString path);
    void redirectToUpgrade(int activationButton);
    void redirectToPayBusiness(int activationButton);
    void registerUserActivity();
    void PSAseen(int id);

protected:
    bool checkOverquotaBandwidth();
    void createTrayIcon();
    void createGuestMenu();
    bool showTrayIconAlwaysNEW();
    void loggedIn(bool fromWizard);
    void startSyncs();
    void applyStorageState(int state, bool doNotAskForUserStats = false);
    void processUploadQueue(mega::MegaHandle nodeHandle);
    void processDownloadQueue(QString path);
    void unityFix();
    void disableSyncs();
    void restoreSyncs();
    void closeDialogs(bool bwoverquota = false);
    void calculateInfoDialogCoordinates(QDialog *dialog, int *posx, int *posy);
    void deleteMenu(QMenu *menu);
    void startHttpServer();
    void startHttpsServer();
    void initLocalServer();
    void refreshStorageUIs();

    void sendOverStorageNotification(int state);
    void sendBusinessWarningNotification();

    bool eventFilter(QObject *obj, QEvent *e);

    QSystemTrayIcon *trayIcon;

    QAction *changeProxyAction;
    QAction *initialExitAction;
    std::unique_ptr<QMenu> initialMenu;

#ifdef _WIN32
    std::unique_ptr<QMenu> windowsMenu;
    QAction *windowsExitAction;
    QAction *windowsUpdateAction;
    QAction *windowsImportLinksAction;
    QAction *windowsUploadAction;
    QAction *windowsDownloadAction;
    QAction *windowsStreamAction;
    QAction *windowsTransferManagerAction;
    QAction *windowsSettingsAction;
#endif

    std::unique_ptr<VerifyEmailMessage> verifyEmail;
    std::unique_ptr<QMenu> infoDialogMenu;
    std::unique_ptr<QMenu> guestMenu;
    QMenu emptyMenu;
    std::unique_ptr<QMenu> syncsMenu;
    QSignalMapper *menuSignalMapper;

    MenuItemAction *exitAction;
    MenuItemAction *settingsAction;
    MenuItemAction *importLinksAction;
    MenuItemAction *uploadAction;
    MenuItemAction *downloadAction;
    MenuItemAction *streamAction;
    MenuItemAction *myCloudAction;
    MenuItemAction *addSyncAction;

    MenuItemAction *updateAction;
    QAction *showStatusAction;

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
    SetupWizard *setupWizard;
    SettingsDialog *settingsDialog;
    InfoDialog *infoDialog;
#ifdef _WIN32
    QMap<QString, double> lastCheckedScreens;
#endif
    bool infoOverQuota;
    Preferences *preferences;
    mega::MegaApi *megaApi;
    mega::MegaApi *megaApiFolders;
    QFilterAlertsModel *notificationsProxyModel;
    QAlertsModel *notificationsModel;
    MegaAlertDelegate *notificationsDelegate;
    std::unique_ptr<QObject> context{new QObject};
    QString crashReportFilePath;

    HTTPServer *httpServer;
    HTTPServer *httpsServer;
    long long lastTsConnectionError = 0;
    UploadToMegaDialog *uploadFolderSelector;
    DownloadFromMegaDialog *downloadFolderSelector;
    mega::MegaHandle fileUploadTarget;
    QFileDialog *fileUploadSelector;
    mega::MegaHandle folderUploadTarget;
    QFileDialog *folderUploadSelector;
    QPointer<StreamingFromMegaDialog> streamSelector;
    MultiQFileDialog *multiUploadFileDialog;
    QQueue<QString> uploadQueue;
    QQueue<mega::MegaNode *> downloadQueue;
    std::shared_ptr<mega::MegaNode> mRootNode;
    bool mFetchingNodes = false;
    int numTransfers[2];
    int activeTransferTag[2];
    unsigned long long activeTransferPriority[2];
    unsigned int activeTransferState[2];
    bool queuedUserStats[3];
    int queuedStorageUserStatsReason;
    long long userStatsLastRequest[3];
    bool inflightUserStats[3];
    long long cleaningSchedulerExecution;
    long long lastUserActivityExecution;
    long long lastTsBusinessWarning;
    long long lastTsErrorMessageShown;
    bool almostOQ;
    int storageState;
    int appliedStorageState;
    long long receivedStorageSum;
    long long maxMemoryUsage;
    int exportOps;
    int syncState;
    mega::MegaPricing *pricing;
    long long bwOverquotaTimestamp;
    UpgradeDialog *bwOverquotaDialog;
    UpgradeOverStorage *storageOverquotaDialog;
    bool bwOverquotaEvent;
    InfoWizard *infoWizard;
    mega::QTMegaListener *delegateListener;
    MegaUploader *uploader;
    MegaDownloader *downloader;
    QTimer *periodicTasksTimer;
    QTimer *infoDialogTimer;
    QTimer *firstTransferTimer;
    QTranslator translator;
    PasteMegaLinksDialog *pasteMegaLinksDialog;
    ChangeLogDialog *changeLogDialog;
    ImportMegaLinksDialog *importDialog;
    QMessageBox *exitDialog;
    QMessageBox *sslKeyPinningError;
    NodeSelector *downloadNodeSelector;
    QString lastTrayMessage;
    QStringList extraLinks;
    QString currentLanguageCode;

    static QString appPath;
    static QString appDirPath;
    static QString dataPath;
    static QString lastNotificationError;

    QThread *updateThread;
    UpdateTask *updateTask;
    Notificator *notificator;
    long long lastActiveTime;
    QNetworkConfigurationManager networkConfigurationManager;
    QList<QNetworkInterface> activeNetworkInterfaces;
    QMap<QString, QString> pendingLinks;
    std::unique_ptr<MegaSyncLogger> logger;
    QPointer<TransferManager> transferManager;
    QMap<int, mega::MegaTransfer*> finishedTransfers;
    QList<mega::MegaTransfer*> finishedTransferOrder;

    QHash<unsigned long long, TransferMetaData*> transferAppData;

    bool reboot;
    bool syncActive;
    bool paused;
    bool indexing;
    bool waiting;
    bool syncing; //if any sync is in syncing state
    bool updated;
    bool checkupdate;
    bool updateBlocked;
    long long lastExit;
    bool appfinished;
    bool updateAvailable;
    bool isLinux;
    int noKeyDetected;
    bool isFirstSyncDone;
    bool isFirstFileSynced;
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
    friend class DeferPreferencesSyncForScope;
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
    virtual void onRequestFinish(mega::MegaApi* api, mega::MegaRequest *request, mega::MegaError* e);

protected:
    MegaApplication *app;
};

#endif // MEGAAPPLICATION_H

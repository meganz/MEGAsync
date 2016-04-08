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

#include "gui/NodeSelector.h"
#include "gui/InfoDialog.h"
#include "gui/InfoOverQuotaDialog.h"
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
#include "control/Preferences.h"
#include "control/HTTPServer.h"
#include "control/MegaUploader.h"
#include "control/MegaDownloader.h"
#include "control/UpdateTask.h"
#include "control/MegaSyncLogger.h"
#include "megaapi.h"
#include "QTMegaListener.h"

#ifdef __APPLE__
    #include "gui/MegaSystemTrayIcon.h"
#endif

Q_DECLARE_METATYPE(QQueue<QString>)

class Notificator;
class MEGASyncDelegateListener;

class MegaApplication : public QApplication, public mega::MegaListener
{
    Q_OBJECT

public:

    explicit MegaApplication(int &argc, char **argv);
    ~MegaApplication();

    void initialize();
    static QString applicationFilePath();
    static QString applicationDirPath();
    static QString applicationDataPath();
    void changeLanguage(QString languageCode);
    void updateTrayIcon();

    virtual void onRequestStart(mega::MegaApi* api, mega::MegaRequest *request);
    virtual void onRequestFinish(mega::MegaApi* api, mega::MegaRequest *request, mega::MegaError* e);
    virtual void onRequestTemporaryError(mega::MegaApi *api, mega::MegaRequest *request, mega::MegaError* e);
    virtual void onTransferStart(mega::MegaApi *api, mega::MegaTransfer *transfer);
    virtual void onTransferFinish(mega::MegaApi* api, mega::MegaTransfer *transfer, mega::MegaError* e);
    virtual void onTransferUpdate(mega::MegaApi *api, mega::MegaTransfer *transfer);
    virtual void onTransferTemporaryError(mega::MegaApi *api, mega::MegaTransfer *transfer, mega::MegaError* e);
    virtual void onAccountUpdate(mega::MegaApi *api);
    virtual void onUsersUpdate(mega::MegaApi* api, mega::MegaUserList *users);
    virtual void onNodesUpdate(mega::MegaApi* api, mega::MegaNodeList *nodes);
    virtual void onReloadNeeded(mega::MegaApi* api);
    virtual void onGlobalSyncStateChanged(mega::MegaApi *api);
    virtual void onSyncStateChanged(mega::MegaApi *api,  mega::MegaSync *sync);
    virtual void onSyncFileStateChanged(mega::MegaApi *api, mega::MegaSync *sync, const char *filePath, int newState);


    mega::MegaApi *getMegaApi() { return megaApi; }

    void unlink();
    void showInfoMessage(QString message, QString title = tr("MEGAsync"));
    void showWarningMessage(QString message, QString title = tr("MEGAsync"));
    void showErrorMessage(QString message, QString title = tr("MEGAsync"));
    void showNotificationMessage(QString message, QString title = tr("MEGAsync"));
    void setUploadLimit(int limit);
    void setUseHttpsOnly(bool httpsOnly);
    void startUpdateTask();
    void stopUpdateTask();
    void applyProxySettings();
    void updateUserStats();
    void addRecentFile(QString fileName, long long fileHandle, QString localPath = QString(), QString nodeKey = QString());
    void checkForUpdates();
    void showTrayMenu(QPoint *point = NULL);
    void toggleLogging();

#if (QT_VERSION == 0x050500) && defined(_WIN32)
    bool eventFilter(QObject *o, QEvent * ev);
#endif

signals:
    void startUpdaterThread();
    void tryUpdate();
    void installUpdate();
    void unityFixSignal();

public slots:
    void trayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void onMessageClicked();
    void start();
    void openSettings(int tab = SettingsDialog::ACCOUNT_TAB);
    void openBwOverquotaDialog();
    void changeProxy();
    void importLinks();
    void showChangeLog();
    void uploadActionClicked();
    void loginActionClicked();
    void copyFileLink(mega::MegaHandle fileHandle, QString nodeKey = QString());
    void downloadActionClicked();
    void streamActionClicked();
    void logoutActionClicked();
    void processDownloads();
    void shellUpload(QQueue<QString> newUploadQueue);
    void shellExport(QQueue<QString> newExportQueue);
    void externalDownload(QQueue<mega::MegaNode *> newDownloadQueue);
    void externalDownload(QString megaLink, QString auth);
    void internalDownload(long long handle);
    void syncFolder(long long handle);
    void onLinkImportFinished();
    void onRequestLinksFinished();
    void onUpdateCompleted();
    void onUpdateAvailable(bool requested);
    void onInstallingUpdate(bool requested);
    void onUpdateNotFound(bool requested);
    void onUpdateError();
    void rebootApplication(bool update = true);
    void exitApplication();
    void pauseTransfers(bool pause);
    void checkNetworkInterfaces();
    void periodicTasks();
    void cleanAll();
    void onDupplicateLink(QString link, QString name, mega::MegaHandle handle);
    void onDupplicateTransfer(QString localPath, QString name, mega::MegaHandle handle, QString nodeKey = QString());
    void onInstallUpdateClicked();
    void showInfoDialog();
    bool anUpdateIsAvailable();
    void triggerInstallUpdate();
    void scanningAnimationStep();
    void setupWizardFinished(int result);
    void overquotaDialogFinished(int result);
    void runConnectivityCheck();
    void onConnectivityCheckSuccess();
    void onConnectivityCheckError();
    void userAction(int action);
    void changeState();
    void showUpdatedMessage();
    void handleMEGAurl(const QUrl &url);
    void handleLocalPath(const QUrl &url);

protected:
    void createTrayIcon();
    void createTrayMenu();
    void createOverQuotaMenu();
    void createGuestMenu();
    bool showTrayIconAlwaysNEW();
    void loggedIn();
    void startSyncs();
    void processUploadQueue(mega::MegaHandle nodeHandle);
    void processDownloadQueue(QString path);
    void unityFix();
    void disableSyncs();
    void restoreSyncs();
    void closeDialogs();
    void calculateInfoDialogCoordinates(QDialog *dialog, int *posx, int *posy);

#ifdef __APPLE__
    MegaSystemTrayIcon *trayIcon;
#else
    QSystemTrayIcon *trayIcon;
#endif

    QAction *changeProxyAction;
    QAction *initialExitAction;
    QMenu *initialMenu;

#ifdef _WIN32
    QMenu *windowsMenu;
    QAction *windowsExitAction;
#endif

    QMenu *trayMenu;
    QMenu *trayOverQuotaMenu;
    QMenu *trayGuestMenu;
    QMenu emptyMenu;
    QAction *exitAction;
    QAction *settingsAction;
    QAction *importLinksAction;
    QAction *uploadAction;
    QAction *downloadAction;
    QAction *streamAction;

    QAction *updateAction;
    QAction *showStatusAction;

    QAction *logoutActionOverquota;
    QAction *settingsActionOverquota;
    QAction *exitActionOverquota;
    QAction *updateActionOverquota;

    QAction *importLinksActionGuest;
    QAction *exitActionGuest;
    QAction *settingsActionGuest;
    QAction *updateActionGuest;
    QAction *loginActionGuest;

    QTimer *scanningTimer;
    QTimer *connectivityTimer;
    int scanningAnimationIndex;
    SetupWizard *setupWizard;
    SettingsDialog *settingsDialog;
    InfoDialog *infoDialog;
    InfoOverQuotaDialog *infoOverQuota;
    Preferences *preferences;
    mega::MegaApi *megaApi;
    HTTPServer *httpServer;
    UploadToMegaDialog *uploadFolderSelector;
    DownloadFromMegaDialog *downloadFolderSelector;
    QPointer<StreamingFromMegaDialog> streamSelector;
    MultiQFileDialog *multiUploadFileDialog;
    QQueue<QString> uploadQueue;
    QQueue<mega::MegaNode *> downloadQueue;
    long long totalDownloadSize, totalUploadSize;
    long long totalDownloadedSize, totalUploadedSize;
    long long uploadSpeed, downloadSpeed;
    long long lastStartedDownload;
    long long lastStartedUpload;
    int exportOps;
    int syncState;
    mega::MegaPricing *pricing;
    long long bwOverquotaTimestamp;
    UpgradeDialog *bwOverquotaDialog;
    mega::QTMegaListener *delegateListener;
    QMap<int, QString> uploadLocalPaths;
    MegaUploader *uploader;
    MegaDownloader *downloader;
    QTimer *periodicTasksTimer;
    QTimer *infoDialogTimer;
    QTranslator *translator;
    PasteMegaLinksDialog *pasteMegaLinksDialog;
    ChangeLogDialog *changeLogDialog;
    ImportMegaLinksDialog *importDialog;
    QMessageBox *exitDialog;
    QMessageBox *sslKeyPinningError;
    NodeSelector *downloadNodeSelector;
    QString lastTrayMessage;

    static QString appPath;
    static QString appDirPath;
    static QString dataPath;

    QThread *updateThread;
    UpdateTask *updateTask;
    Notificator *notificator;
    long long lastActiveTime;
    QNetworkConfigurationManager networkConfigurationManager;
    QList<QNetworkInterface> activeNetworkInterfaces;
    QMap<QString, QString> pendingLinks;
    MegaSyncLogger *logger;

    bool reboot;
    bool syncActive;
    bool paused;
    bool indexing;
    bool waiting;
    bool updated;
    bool updateBlocked;
    long long lastExit;
    bool appfinished;
    bool updateAvailable;
    bool isLinux;
    long long externalNodesTimestamp;
    bool enableDebug;
    bool overquotaCheck;
    int noKeyDetected;
    bool isFirstSyncDone;
    bool isFirstFileSynced;
    bool networkConnectivity;
};

class MEGASyncDelegateListener: public mega::QTMegaListener
{
public:
    MEGASyncDelegateListener(mega::MegaApi *megaApi, mega::MegaListener *parent=NULL);
    virtual void onRequestFinish(mega::MegaApi* api, mega::MegaRequest *request, mega::MegaError* e);
};

#endif // MEGAAPPLICATION_H

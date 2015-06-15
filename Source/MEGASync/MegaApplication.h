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
#include "gui/ImportMegaLinksDialog.h"
#include "gui/MultiQFileDialog.h"
#include "gui/PasteMegaLinksDialog.h"
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
    void startUpdateTask();
    void stopUpdateTask();
    void applyProxySettings();
    void showUpdatedMessage();
    void updateUserStats();
    void addRecentFile(QString fileName, long long fileHandle, QString localPath = QString(), QString nodeKey = QString());
    void checkForUpdates();
    void showTrayMenu(QPoint *point = NULL);
    void toggleLogging();

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
    void changeProxy();
    void pauseSync();
    void resumeSync();
	void importLinks();
    void uploadActionClicked();
    void copyFileLink(mega::MegaHandle fileHandle, QString nodeKey = QString());
    void downloadActionClicked();
    void logoutActionClicked();
    void processDownload(mega::MegaNode *node);
    void shellUpload(QQueue<QString> newUploadQueue);
    void shellExport(QQueue<QString> newExportQueue);
    void externalDownload(QString megaLink);
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
    void aboutDialog();
    void refreshTrayIcon();
    void cleanAll();
    void onDupplicateLink(QString link, QString name, mega::MegaHandle handle);
    void onDupplicateUpload(QString localPath, QString name, mega::MegaHandle handle);
    void onInstallUpdateClicked();
    void showInfoDialog();
    bool anUpdateIsAvailable();
    void triggerInstallUpdate();
    void scanningAnimationStep();
    void setupWizardFinished();
    void runConnectivityCheck();
    void onConnectivityCheckSuccess();
    void onConnectivityCheckError();

protected:
    void createTrayIcon();
    void createOverQuotaMenu();
    bool showTrayIconAlwaysNEW();
    void loggedIn();
    void startSyncs();
    void processUploadQueue(mega::MegaHandle nodeHandle);
    void processDownloadQueue(QString path);
    void unityFix();
    void disableSyncs();
    void closeDialogs();
    void calculateInfoDialogCoordinates(QDialog *dialog, int *posx, int *posy);

#ifdef __APPLE__
    MegaSystemTrayIcon *trayIcon;
#else
    QSystemTrayIcon *trayIcon;
#endif

#ifdef _WIN32
    QMenu *windowsMenu;
    QAction *windowsExitAction;
#endif

    QMenu *initialMenu;
    QMenu *trayMenu;
    QMenu *trayOverQuotaMenu;
    QMenu emptyMenu;
    QAction *exitAction;
    QAction *settingsAction;
    QAction *importLinksAction;
    QAction *uploadAction;
    QAction *downloadAction;
    QAction *aboutAction;
    QAction *changeProxyAction;
    QAction *initialExitAction;
    QAction *updateAction;
    QAction *showStatusAction;

    QAction *showStatusActionOverquota;
    QAction *logoutActionOverquota;
    QAction *settingsActionOverquota;
    QAction *exitActionOverquota;
    QAction *updateActionOverquota;

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
    mega::QTMegaListener *delegateListener;
	QMap<int, QString> uploadLocalPaths;
    MegaUploader *uploader;
    MegaDownloader *downloader;
    QTimer *refreshTimer;
    QTimer *infoDialogTimer;
    QTranslator *translator;
    PasteMegaLinksDialog *pasteMegaLinksDialog;
    ImportMegaLinksDialog *importDialog;
    QMessageBox *exitDialog;
    NodeSelector *downloadNodeSelector;
    QString lastTrayMessage;

    static QString appPath;
    static QString appDirPath;
    static QString dataPath;

    QThread *updateThread;
    UpdateTask *updateTask;
    Notificator *notificator;
    long long lastActiveTime;
    QNetworkConfigurationManager networkManager;
    QList<QNetworkInterface> activeNetworkInterfaces;
    QList<QString> pendingLinks;
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
};

class MEGASyncDelegateListener: public mega::QTMegaListener
{
public:
    MEGASyncDelegateListener(mega::MegaApi *megaApi, mega::MegaListener *parent=NULL);
    virtual void onRequestFinish(mega::MegaApi* api, mega::MegaRequest *request, mega::MegaError* e);
};

#endif // MEGAAPPLICATION_H

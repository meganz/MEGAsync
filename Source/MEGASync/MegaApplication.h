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

#include "gui/InfoDialog.h"
#include "gui/SetupWizard.h"
#include "gui/SettingsDialog.h"
#include "gui/UploadToMegaDialog.h"
#include "utils/Preferences.h"
#include "utils/HTTPServer.h"
#include "utils/ShellDispatcherListener.h"
#include "utils/MegaUploader.h"
#include "utils/UpdateTask.h"
#include "sdk/megaapi.h"
#include "sdk/qt/QTMegaListener.h"

Q_DECLARE_METATYPE(QQueue<QString>)

class MegaApplication : public QApplication, public MegaListener, public ShellDispatcherListener
{
    Q_OBJECT

public:
    explicit MegaApplication(int &argc, char **argv);

    virtual void onRequestStart(MegaApi* api, MegaRequest *request);
    virtual void onRequestFinish(MegaApi* api, MegaRequest *request, MegaError* e);
    virtual void onRequestTemporaryError(MegaApi *api, MegaRequest *request, MegaError* e);
	virtual void onTransferStart(MegaApi *api, MegaTransfer *transfer);
	virtual void onTransferFinish(MegaApi* api, MegaTransfer *transfer, MegaError* e);
    virtual void onTransferUpdate(MegaApi *api, MegaTransfer *transfer);
    virtual void onTransferTemporaryError(MegaApi *api, MegaTransfer *transfer, MegaError* e);
    virtual void onUsersUpdate(MegaApi* api, UserList *users);
    virtual void onNodesUpdate(MegaApi* api, NodeList *nodes);
    virtual void onReloadNeeded(MegaApi* api);

	/*
    virtual void onSyncStateChanged(Sync*, syncstate);
    virtual void onSyncRemoteCopy(Sync*, const char*);
    virtual void onSyncGet(Sync*, const char*);
    virtual void onSyncPut(Sync*, const char*);
	*/

    MegaApi *getMegaApi() { return megaApi; }
    Preferences *getPreferences() { return preferences; }

    void showSyncedIcon();
    void showSyncingIcon();
	void reloadSyncs();
    void unlink();
    void showInfoMessage(QString message, QString title = tr("MEGAsync"));
    void showWarningMessage(QString message, QString title = tr("MEGAsync"));
    void showErrorMessage(QString message, QString title = tr("MEGAsync"));
    void showNotificationMessage(QString message, QString title = tr("MEGAsync"));
    void setUploadLimit(int limit);
    void startUpdateTask();
    void stopUpdateTask();

signals:
    void startUpdaterThread();

public slots:
    void trayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void openSettings();
    void pauseSync();
    void resumeSync();
	void importLinks();
	void copyFileLink(handle fileHandle);
    void shellUpload(QQueue<QString> newUploadQueue);
	void showUploadDialog();
	void onLinkImportFinished();
    void onUpdateCompleted();
    void rebootApplication();
    void exitApplication();
    void pauseTransfers(bool pause);
    void aboutDialog();

protected:
    void createActions();
    void createTrayIcon();
    bool showTrayIconAlwaysNEW();
    void init();
    void loggedIn();
	void startSyncs();
	void stopSyncs();
	void processUploadQueue(handle nodeHandle);

    QSystemTrayIcon *trayIcon;
    QMenu *trayMenu;
    QAction *exitAction;
    QAction *settingsAction;
    QAction *pauseAction;
    QAction *resumeAction;
	QAction *importLinksAction;
    QAction *aboutAction;

	SetupWizard *setupWizard;
    SettingsDialog *settingsDialog;
    InfoDialog *infoDialog;
    Preferences *preferences;
    MegaApi *megaApi;
    HTTPServer *httpServer;
	UploadToMegaDialog *uploadFolderSelector;
	QQueue<QString> uploadQueue;
    int queuedUploads, queuedDownloads;
	int totalUploads, totalDownloads;
	long long totalDownloadSize, totalUploadSize;
	long long totalDownloadedSize, totalUploadedSize;
	long long uploadSpeed, downloadSpeed;
    syncstate syncState;
	QTMegaListener *delegateListener;
	QMap<int, QString> uploadLocalPaths;
    MegaUploader *uploader;

    QThread updateThread;
    UpdateTask updateTask;
    bool reboot;
    bool syncActive;

    static const int VERSION_CODE;
};

#endif // MEGAAPPLICATION_H

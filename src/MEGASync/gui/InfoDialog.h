#ifndef INFODIALOG_H
#define INFODIALOG_H

#include <QDialog>
#include <QTimer>
#include <QProcess>
#include <QDateTime>
#include <QPainter>
#include "GuestWidget.h"
#include "SettingsDialog.h"
#include "DataUsageMenu.h"
#include "MenuItemAction.h"
#include "RecentlyUpdated.h"

namespace Ui {
class InfoDialog;
}

class MegaApplication;
class InfoDialog : public QDialog, public mega::MegaTransferListener
{
    Q_OBJECT

    enum {
        STATE_STARTING,
        STATE_PAUSED,
        STATE_WAITING,
        STATE_INDEXING,
        STATE_UPDATED
    };

    enum {
        ALL_TRANSFERS_TAB = 0,
        DOWNLOADS_TAB   = 1,
        UPLOADS_TAB = 2,
        COMPLETED_TAB = 3
    };

public:
    explicit InfoDialog(MegaApplication *app, QWidget *parent = 0);
    ~InfoDialog();

    void setUsage();
    void setUserName();
    void setAvatar();
    void setTransfer(mega::MegaTransfer *transfer);
    void updateTransfers();
    void transferFinished(int error);
    void updateSyncsButton();
    void setIndexing(bool indexing);
    void setWaiting(bool waiting);
    void increaseUsedStorage(long long bytes, bool isInShare);
    void setOverQuotaMode(bool state);
    void updateState();
    void closeSyncsMenu();
    void addSync(mega::MegaHandle h);
    void clearUserAttributes();

    virtual void onTransferFinish(mega::MegaApi* api, mega::MegaTransfer *transfer, mega::MegaError* e);

#ifdef __APPLE__
    void moveArrow(QPoint p);
#endif

    void regenerateLayout();

private:
    void drawAvatar(QString email);
    void createQuotaUsedMenu();

public slots:
   void addSync();
   void onContextDownloadMenu(QPoint pos, bool regular);
   void onContextUploadMenu(QPoint pos, bool regular);
   void globalDownloadState();
   void downloadState();
   void globalUploadState();
   void uploadState();
   void cancelAllUploads();
   void cancelAllDownloads();
   void cancelCurrentUpload();
   void cancelCurrentDownload();
   void onAllUploadsFinished();
   void onAllDownloadsFinished();
   void onAllTransfersFinished();

private slots:
    void on_bSettings_clicked();
    void on_bSyncFolder_clicked();
    void on_bUpgrade_clicked();
    void openFolder(QString path);
    void on_bChats_clicked();
    void on_bTransferManager_clicked();
    void onOverlayClicked();
    void scanningAnimationStep();
    void onUserAction(int action);

    void on_bDotUsedStorage_clicked();
    void on_bDotUsedQuota_clicked();

    void hideUsageBalloon();


#ifdef __APPLE__
    void recentlyUpdatedStateChanged(int mode);
    void onAnimationFinished();
    void onValueChanged(QVariant);
#endif

signals:
    void openTransferManager(int tab);

private:
    Ui::InfoDialog *ui;
    QPushButton *overlay;
#ifdef __APPLE__
    QPushButton *arrow;

    QPropertyAnimation *minHeightAnimation;
    QPropertyAnimation *maxHeightAnimation;
    QParallelAnimationGroup *animationGroup;
#endif

    QMenu *syncsMenu;
    QMenu *transferMenu;
    DataUsageMenu *storageUsedMenu;

    MenuItemAction *cloudItem;
    MenuItemAction *inboxItem;
    MenuItemAction *sharesItem;
    MenuItemAction *rubbishItem;

    long long downloadSpeed;
    long long uploadSpeed;
    int currentUpload;
    int currentDownload;
    int totalUploads;
    int totalDownloads;
    int activeDownloadState, activeUploadState;
    long long remainingDownloadBytes, remainingUploadBytes;
    long long meanDownloadSpeed, meanUploadSpeed;
    int remainingUploads, remainingDownloads;
    bool indexing;
    bool waiting;
    GuestWidget *gWidget;
    int state;
    bool overQuotaState;

protected:
    void changeEvent(QEvent * event);
    bool eventFilter(QObject *obj, QEvent *e);
#ifdef __APPLE__
    void paintEvent( QPaintEvent * e);
    void hideEvent(QHideEvent *event);
#endif

protected:
    QDateTime lastPopupUpdate;
    QTimer scanningTimer;
    QTimer downloadsFinishedTimer;
    QTimer uploadsFinishedTimer;
    QTimer transfersFinishedTimer;
    int scanningAnimationIndex;
    MegaApplication *app;
    Preferences *preferences;
    mega::MegaApi *megaApi;
    mega::MegaTransfer *activeDownload;
    mega::MegaTransfer *activeUpload;
};

#endif // INFODIALOG_H

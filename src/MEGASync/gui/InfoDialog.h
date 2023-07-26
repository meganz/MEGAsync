#ifndef INFODIALOG_H
#define INFODIALOG_H

#include <QDialog>
#include <QTimer>
#include <QProcess>
#include <QDateTime>
#include <QPainter>
#include "GuestWidget.h"
#include "SettingsDialog.h"
#include "MenuItemAction.h"
#include "control/Preferences.h"
#include "syncs/control/SyncInfo.h"
#include <QGraphicsOpacityEffect>
#include "TransferScanCancelUi.h"
#include "HighDpiResize.h"
#include "Utilities.h"
#include "FilterAlertWidget.h"
#include "QtPositioningBugFixer.h"
#include "TransferQuota.h"
#include "StatusInfo.h"
#include "syncs/gui/SyncsMenu.h"
#include "syncs/control/SyncController.h"
#include "syncs/gui/Backups/AddBackupDialog.h"
#include "syncs/gui/Backups/BackupsWizard.h"

#include <memory>
#ifdef _WIN32
#include <chrono>
#endif

namespace Ui {
class InfoDialog;
}

class MegaApplication;
class TransferManager;
class BindFolderDialog;
class InfoDialog : public QDialog, public mega::MegaTransferListener, ::mega::MegaRequestListener
{
    Q_OBJECT

    enum {
        STATE_STARTING,
        STATE_PAUSED,
        STATE_WAITING,
        STATE_INDEXING,
        STATE_UPDATED,
        STATE_SYNCING,
        STATE_TRANSFERRING,
    };

public:

    enum {
        STATE_NONE = -1,
        STATE_LOGOUT = 0,
        STATE_LOGGEDIN = 1,
        STATE_LOCKED_EMAIL = mega::MegaApi::ACCOUNT_BLOCKED_VERIFICATION_EMAIL,
        STATE_LOCKED_SMS = mega::MegaApi::ACCOUNT_BLOCKED_VERIFICATION_SMS
    };

    explicit InfoDialog(MegaApplication *app, QWidget *parent = 0, InfoDialog* olddialog = nullptr);
    ~InfoDialog();

    PSA_info* getPSAdata();
    void setUsage();
    void setAvatar();
    void setIndexing(bool indexing);
    void setWaiting(bool waiting);
    void setSyncing(bool syncing);
    void setTransferring(bool transferring);
    void setOverQuotaMode(bool state);
    void setAccountType(int accType);
    void setDisabledSyncTags(QSet<int> tags);
    void addBackup();
    void clearUserAttributes();
    void setPSAannouncement(int id, QString title, QString text, QString urlImage, QString textButton, QString linkButton);
    bool updateOverStorageState(int state);
    void updateNotificationsTreeView(QAbstractItemModel *model, QAbstractItemDelegate *delegate);

    void reset();

    void enterBlockingState();
    void leaveBlockingState(bool fromCancellation);
    void disableCancelling();
    void setUiInCancellingStage();
    void updateUiOnFolderTransferUpdate(const FolderTransferUpdateEvent& event);

#ifdef __APPLE__
    void moveArrow(QPoint p);
#endif

    void on_bStorageDetails_clicked();
    void regenerateLayout(int blockState = mega::MegaApi::ACCOUNT_NOT_BLOCKED, InfoDialog* olddialog = nullptr);
    HighDpiResize highDpiResize;
#ifdef _WIN32
    std::chrono::steady_clock::time_point lastWindowHideTime;
#endif

    void setUnseenNotifications(long long value);
    void setUnseenTypeNotifications(long long all, long long contacts, long long shares, long long payment);
    long long getUnseenNotifications() const;
    int getLoggedInMode() const;
    void showNotifications();

    void move(int x, int y);

    void setTransferManager(TransferManager *transferManager);

private:
    InfoDialog() = default;
    void animateStates(bool opt);
    void hideEvent(QHideEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void moveEvent(QMoveEvent *) override;

public slots:
    void showSyncProblems(QString s);

    void pauseResumeClicked();
    void generalAreaClicked();
    void dlAreaClicked();
    void upAreaClicked();

    void pauseResumeHovered(QMouseEvent *event);
    void generalAreaHovered(QMouseEvent *event);
    void dlAreaHovered(QMouseEvent *event);
    void upAreaHovered(QMouseEvent *event);

    void addSync(mega::MegaHandle h = mega::INVALID_HANDLE);
    void onAddSync(mega::MegaSync::SyncType type = mega::MegaSync::TYPE_TWOWAY);
    void onAddBackup();
    void updateDialogState();

   void enableTransferOverquotaAlert();
   void enableTransferAlmostOverquotaAlert();
   void setBandwidthOverquotaState(QuotaState state);

private slots:
    void on_bSettings_clicked();
    void on_bUpgrade_clicked();
    void on_bUpgradeOverDiskQuota_clicked();
    void openFolder(QString path);
    void onOverlayClicked();
    void on_bTransferManager_clicked();
    void on_bAddSync_clicked();
    void on_bAddBackup_clicked();
    void on_bUpload_clicked();
    void onUserAction(int action);
    void resetLoggedInMode();

    void on_tTransfers_clicked();
    void on_tNotifications_clicked();
    void onActualFilterClicked();
    void applyFilterOption(int opt);
    void on_bNotificationsSettings_clicked();

    void on_bDiscard_clicked();
    void on_bBuyQuota_clicked();

    void onAnimationFinished();

    void sTabsChanged(int tab);

    void on_bDismissSyncSettings_clicked();
    void on_bOpenSyncSettings_clicked();
    void on_bDismissBackupsSettings_clicked();
    void on_bOpenBackupsSettings_clicked();
    void on_bDismissAllSyncsSettings_clicked();
    void on_bOpenAllSyncsSettings_clicked();

    void updateTransfersCount();

    void onResetTransfersSummaryWidget();
    void onTransfersStateChanged();

    void onStalledIssuesReceived();

signals:
    void triggerShowSyncProblems(QString s);

    void openTransferManager(int tab);
    void dismissStorageOverquota(bool oq);
    // signal emitted when showing or dismissing the overquota message.
    // parameter messageShown is true when alert is enabled, false when dismissed
    void transferOverquotaMsgVisibilityChange(bool messageShown);
    // signal emitted when showing or dismissing the almost overquota message.
    // parameter messageShown is true when alert is enabled, false when dismissed
    void almostTransferOverquotaMsgVisibilityChange(bool messageShown);
    void userActivity();
    void cancelScanning();

private:
    Ui::InfoDialog *ui;
    QPushButton *overlay;
#ifdef __APPLE__
    QPushButton *arrow;
    QWidget *dummy; // Patch to let text input on line edits of GuestWidget
#endif

    FilterAlertWidget *filterMenu;

    MenuItemAction *cloudItem;
    MenuItemAction *sharesItem;
    MenuItemAction *rubbishItem;

    int activeDownloadState, activeUploadState;
    bool pendingUploadsTimerRunning = false;
    bool pendingDownloadsTimerRunning = false;
    bool circlesShowAllActiveTransfersProgress;
    void showSyncsMenu(QPushButton* b, mega::MegaSync::SyncType type);
    SyncsMenu* createSyncMenu(mega::MegaSync::SyncType type, bool isEnabled);


    bool mIndexing; //scanning
    bool mWaiting;
    bool mSyncing; //if any sync is in syncing state
    bool mTransferring; // if there are ongoing regular transfers
    GuestWidget *gWidget;
    StatusInfo::TRANSFERS_STATES mState;
    bool overQuotaState;
    bool transferOverquotaAlertEnabled;
    bool transferAlmostOverquotaAlertEnabled;
    int storageState;
    QuotaState transferQuotaState;
    int actualAccountType;
    int loggedInMode = STATE_NONE;
    bool notificationsReady = false;
    bool isShown = false;
    long long unseenNotifications = 0;

    QPointer<TransferManager> mTransferManager;

    QPointer<AddBackupDialog> mAddBackupDialog;
    QPointer<BindFolderDialog> mAddSyncDialog;

#ifdef Q_OS_LINUX
    bool doNotActAsPopup;
#endif

    QPropertyAnimation *animation;
    QGraphicsOpacityEffect *opacityEffect;

    bool shownSomeIssuesOccurred = false;
    QPropertyAnimation *minHeightAnimationSomeIssues;
    QPropertyAnimation *maxHeightAnimationSomeIssues;
    QParallelAnimationGroup animationGroupSomeIssues;
    void hideSomeIssues();
    void showSomeIssues();
    QHash<QPushButton*, SyncsMenu*> mSyncsMenus;

protected:
    void updateBlockedState();
    void updateState();
    bool checkFailedState();
    void changeEvent(QEvent * event) override;
    bool eventFilter(QObject *obj, QEvent *e) override;
    void paintEvent( QPaintEvent * e) override;

protected:
    QDateTime lastPopupUpdate;
    QTimer downloadsFinishedTimer;
    QTimer uploadsFinishedTimer;
    QTimer transfersFinishedTimer;
    QTimer mResetTransferSummaryWidget;
    MegaApplication *app;
    std::shared_ptr<Preferences> mPreferences;
    SyncInfo *mSyncInfo;
    mega::MegaApi *megaApi;
    mega::MegaTransfer *activeDownload;
    mega::MegaTransfer *activeUpload;
    std::shared_ptr<SyncController> mSyncController;

 private:
    void onAddSyncDialogFinished(QPointer<BindFolderDialog> dialog);
    static double computeRatio(long long completed, long long remaining);
    void enableUserActions(bool value);
    void changeStatusState(StatusInfo::TRANSFERS_STATES newState,
                           bool animate = true);
    void setupSyncController();

    TransferScanCancelUi* mTransferScanCancelUi = nullptr;
    QtPositioningBugFixer qtBugFixer;
};

#endif // INFODIALOG_H

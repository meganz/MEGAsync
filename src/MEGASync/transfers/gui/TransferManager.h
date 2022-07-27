#ifndef TRANSFERMANAGER_H
#define TRANSFERMANAGER_H

#include "TransferScanCancelUi.h"
#include "megaapi.h"
#include "Preferences.h"
#include "MenuItemAction.h"
#include "Utilities.h"
#include "TransferItem.h"
#include "TransfersModel.h"
#include "TransferQuota.h"
#include "TransfersWidget.h"
#include "StatusInfo.h"
#include "ButtonIconManager.h"

#include <QGraphicsEffect>
#include <QTimer>
#include <QDialog>
#include <QMenu>

namespace Ui {
class TransferManager;
}

namespace Ui {
class TransferManagerDragBackDrop;
}

#include <QObject>
#include <QPushButton>
#include <QEvent>

class TransferManager : public QDialog
{
    Q_OBJECT

    static const QString TRANSFER_QUOTA_WARNING;
    static const QString TRANSFER_QUOTA_MORE_ABOUT;

public:
    explicit TransferManager(mega::MegaApi *megaApi);
    void setActiveTab(int t);
    ~TransferManager();

    void pauseModel(bool state);
    void enterBlockingState();
    void leaveBlockingState(bool fromCancellation);
    void disableCancelling();

    void setTransferState(const StatusInfo::TRANSFERS_STATES &transferState);

public slots:
    void onTransferQuotaStateChanged(QuotaState transferQuotaState);
    void onStorageStateChanged(int storageState);

signals:
    void viewedCompletedTransfers();
    void completedTransfersTabActive(bool);
    void userActivity();
    void showCompleted(bool showCompleted);
    void cancelScanning();
    void retryAllTransfers();
    void aboutToClose();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    void closeEvent(QCloseEvent* event) override;
    void changeEvent(QEvent *event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragLeaveEvent(QDragLeaveEvent* event) override;
    void dropEvent(QDropEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    static const int SPEED_REFRESH_PERIOD_MS = 700;
    static const int STATS_REFRESH_PERIOD_MS = 1000;

    Ui::TransferManager* mUi;
    mega::MegaApi* mMegaApi;

    QTimer mScanningTimer;
    int mScanningAnimationIndex;

    QTimer mTransferQuotaTimer;

    std::shared_ptr<Preferences> mPreferences;
    QPoint mDragPosition;
    QMap<TransfersWidget::TM_TAB, QFrame*> mTabFramesToggleGroup;
    QMap<TransfersWidget::TM_TAB, QLabel*> mNumberLabelsGroup;
    QMap<TransfersWidget::TM_TAB, QWidget*> mTabNoItem;

    TransfersModel* mModel;
    TransfersCount mTransfersCount;

    bool mSearchFieldReturnPressed;

    QGraphicsDropShadowEffect* mShadowTab;
    QSet<Utilities::FileType> mFileTypesFilter;
    QTimer* mSpeedRefreshTimer;
    QTimer* mStatsRefreshTimer;

    Ui::TransferManagerDragBackDrop* mUiDragBackDrop;
    QWidget* mDragBackDrop;
    TransferScanCancelUi* mTransferScanCancelUi = nullptr;

    int mStorageQuotaState;
    QuotaState mTransferQuotaState;

    ButtonIconManager mButtonIconManager;

    void toggleTab(TransfersWidget::TM_TAB newTab);
    void refreshStateStats();
    void refreshTypeStats();
    void refreshFileTypesStats();
    void applyTextSearch(const QString& text);
    void enableUserActions(bool enabled);
    void checkActionAndMediaVisibility();
    void onFileTypeButtonClicked(TransfersWidget::TM_TAB tab, Utilities::FileType fileType, const QString& tabLabel);
    void checkPauseButtonVisibilityIfPossible();
    void showTransferQuotaBanner(bool state);

    void showAllResults();
    void showDownloadResults();
    void showUploadResults();

private slots:
    void on_tCompleted_clicked();
    void on_tDownloads_clicked();
    void on_tUploads_clicked();
    void on_tAllTransfers_clicked();
    void on_tFailed_clicked();
    void on_tActionButton_clicked();
    void on_tSeePlans_clicked();
    void on_bSearch_clicked();
    void on_leSearchField_editingFinished();
    void on_tSearchIcon_clicked();
    void on_bSearchString_clicked();
    void on_tSearchCancel_clicked();
    void on_tClearSearchResult_clicked();
    void on_bPause_toggled();
    void pauseResumeTransfers(bool isPaused);

    void on_bOpenLinks_clicked();
    void on_tCogWheel_clicked();
    void on_bDownload_clicked();
    void on_bUpload_clicked();
    void on_leSearchField_returnPressed();

    void on_bArchives_clicked();
    void on_bDocuments_clicked();
    void on_bImages_clicked();
    void on_bAudio_clicked();
    void on_bVideos_clicked();
    void on_bOther_clicked();

    void onUpdatePauseState(bool isPaused);
    void onPauseStateChangedByTransferResume();
    void onPauseResumeVisibleRows(bool isPaused);
    void showQuotaStorageDialogs(bool isPaused);

    void onTransfersDataUpdated();
    void refreshSearchStats();

    void onVerticalScrollBarVisibilityChanged(bool state);

    void refreshSpeed();
    void refreshView();

    void updateTransferWidget(QWidget* widgetToShow);
    void onScanningAnimationUpdate();

    void onTransferQuotaExceededUpdate();
};

#endif // TRANSFERMANAGER_H

#ifndef TRANSFERMANAGER_H
#define TRANSFERMANAGER_H

#include "ButtonIconManager.h"
#include "megaapi.h"
#include "Preferences.h"
#include "StatusInfo.h"
#include "TabSelector.h"
#include "TransferQuota.h"
#include "TransferScanCancelUi.h"
#include "TransfersModel.h"
#include "TransfersWidget.h"
#include "Utilities.h"

#include <QDialog>
#include <QGraphicsEffect>
#include <QMenu>
#include <QTimer>

namespace Ui {
class TransferManager;
}

namespace Ui {
class TransferManagerDragBackDrop;
}

class TransferManager : public QDialog
{
    Q_OBJECT

public:
    explicit TransferManager(mega::MegaApi* megaApi);
    ~TransferManager();

    void pauseModel(bool state);
    void enterBlockingState();
    void leaveBlockingState(bool fromCancellation);
    void disableCancelling();
    void setUiInCancellingStage();
    void onFolderTransferUpdate(const FolderTransferUpdateEvent& event);

    void setTransferState(const StatusInfo::TRANSFERS_STATES &transferState);

    void toggleTab(TransfersWidget::TM_TAB newTab);
    void toggleTab(int newTab);

    static QString getResumeAllTransfersTooltip();
    static QString getPauseAllTransfersTooltip();

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
    void closeEvent(QCloseEvent* event) override;
    bool event(QEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragLeaveEvent(QDragLeaveEvent* event) override;
    void dropEvent(QDropEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    static const int SPEED_REFRESH_PERIOD_MS = 700;
    static const int STATS_REFRESH_PERIOD_MS = 1000;

    Ui::TransferManager* mUi;
    mega::MegaApi* mMegaApi;

    std::shared_ptr<Preferences> mPreferences;
    QPoint mDragPosition;
    QMap<TransfersWidget::TM_TAB, TabSelector*> mTabSelectorsToggleGroup;
    QMap<TransfersWidget::TM_TAB, QWidget*> mTabNoItem;
    QMap<TransfersWidget::TM_TAB, QPair<int, Qt::SortOrder>> mTabSortCriterion;

    TransfersModel* mModel;
    TransfersCount mTransfersCount;

    QSet<Utilities::FileType> mFileTypesFilter;
    QTimer* mSpeedRefreshTimer;
    QTimer* mStatsRefreshTimer;

    Ui::TransferManagerDragBackDrop* mUiDragBackDrop;
    QWidget* mDragBackDrop;
    TransferScanCancelUi* mTransferScanCancelUi = nullptr;

    int mStorageQuotaState;
    QuotaState mTransferQuotaState;
    bool hasOverQuotaErrors();

    bool mFoundStalledIssues;
    ButtonIconManager mButtonIconManager;
    QTimer* mTaskbarPinningRequestTimer;

    void refreshStateStats();
    void refreshTypeStats();
    void refreshFileTypesStats();
    void applyTextSearch(const QString& text);
    void enableUserActions(bool enabled);
    void checkActionAndMediaVisibility();
    void onFileTypeButtonClicked(TransfersWidget::TM_TAB tab, Utilities::FileType fileType);
    void checkPauseButtonVisibilityIfPossible();
    void showTransferQuotaBanner(bool state);

    void updateCurrentCategoryTitle();

    void filterByTab(TransfersWidget::TM_TAB tab);
    void startRequestTaskbarPinningTimer();
    void createSearchChips();
    QString getIssuesBannerButtonText();
    QString getIssuesBannerText();

private slots:
    void on_tActionButton_clicked();
    void on_bPause_toggled();
    void onCompletedClicked();
    void onDownloadsClicked();
    void onUploadsClicked();
    void onAllTransfersClicked();
    void onFailedClicked();
    void pauseResumeTransfers(bool isPaused);

    void onClearSearchResultClicked();
    void onSearchStringClicked();
    void onSearch(const QString& text);

    void onStalledIssuesStateChanged();
    void checkContentInfo();
    void on_bOpenLinks_clicked();
    void on_tCogWheel_clicked();
    void on_bDownload_clicked();
    void on_bUpload_clicked();

    void onArchivesClicked();
    void onDocumentsClicked();
    void onImagesClicked();
    void onAudioClicked();
    void onVideosClicked();
    void onOtherClicked();
    void onMediaTabSelectorClicked();

    void onUpdatePauseState(bool isPaused);
    void onPauseStateChangedByTransferResume();
    void onPauseResumeVisibleRows(bool isPaused);
    void showQuotaStorageDialogs(bool isPaused);

    void onTransfersDataUpdated();
    void refreshSearchStats();

    void refreshSpeed();
    void refreshView();
    void disableTransferManager(bool state);

    void updateTransferWidget(QWidget* widgetToShow);

    void onSortCriterionChanged(int sortBy, Qt::SortOrder order);
    void onRequestTaskbarPinningTimeout();

    void showAllResults();
    void showDownloadResults();
    void showUploadResults();

    void setStalledIssuesBannerText();
};

#endif // TRANSFERMANAGER_H

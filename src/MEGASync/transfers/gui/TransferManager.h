#ifndef TRANSFERMANAGER_H
#define TRANSFERMANAGER_H

#include "megaapi.h"
#include "Preferences.h"
#include "MenuItemAction.h"
#include "Utilities.h"
#include "TransferItem.h"
#include "TransfersModel.h"
#include "TransferQuota.h"
#include "StatusInfo.h"

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

class StalledIssuesDialog;

class TransferManager : public QDialog
{
    Q_OBJECT

    static const char* ITS_ON;

public:
    enum TM_TAB
    {
        NO_TAB            = -1,
        ALL_TRANSFERS_TAB = 0,
        DOWNLOADS_TAB     = 1,
        UPLOADS_TAB       = 2,
        COMPLETED_TAB     = 3,
        FAILED_TAB        = 4,
        SEARCH_TAB        = 5,
        TYPES_TAB_BASE    = 6,
        TYPE_OTHER_TAB    = TYPES_TAB_BASE + toInt(Utilities::FileType::TYPE_OTHER),
        TYPE_AUDIO_TAB    = TYPES_TAB_BASE + toInt(Utilities::FileType::TYPE_AUDIO),
        TYPE_VIDEO_TAB    = TYPES_TAB_BASE + toInt(Utilities::FileType::TYPE_VIDEO),
        TYPE_ARCHIVE_TAB  = TYPES_TAB_BASE + toInt(Utilities::FileType::TYPE_ARCHIVE),
        TYPE_DOCUMENT_TAB = TYPES_TAB_BASE + toInt(Utilities::FileType::TYPE_DOCUMENT),
        TYPE_IMAGE_TAB    = TYPES_TAB_BASE + toInt(Utilities::FileType::TYPE_IMAGE),
        TYPE_TEXT_TAB     = TYPES_TAB_BASE + toInt(Utilities::FileType::TYPE_TEXT),
        TYPES_LAST
    };
    Q_ENUM(TM_TAB)

    explicit TransferManager(mega::MegaApi *megaApi, QWidget *parent = 0);
    void setActiveTab(int t);
    void disableGetLink(bool disable);
    ~TransferManager();

    void setTransferState(const StatusInfo::TRANSFERS_STATES &transferState);

public slots:
    void onTransferQuotaStateChanged(QuotaState transferQuotaState);
    void onStorageStateChanged(int storageState);

signals:
    void viewedCompletedTransfers();
    void completedTransfersTabActive(bool);
    void userActivity();
    void showCompleted(bool showCompleted);
    void clearCompletedTransfers();
    void retryAllTransfers();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    void closeEvent(QCloseEvent* event) override;
    void changeEvent(QEvent *event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragLeaveEvent(QDragLeaveEvent* event) override;
    void dropEvent(QDropEvent *event) override;

private:
    static constexpr int SPEED_REFRESH_PERIOD_MS = 700;
    static constexpr int STATS_REFRESH_PERIOD_MS = 1000;

    Ui::TransferManager* mUi;
    mega::MegaApi* mMegaApi;

    QTimer mScanningTimer;
    int mScanningAnimationIndex;

    std::shared_ptr<Preferences> mPreferences;
    QPoint mDragPosition;
    QMap<TM_TAB, QFrame*> mTabFramesToggleGroup;
    QMap<TM_TAB, QLabel*> mNumberLabelsGroup;
    QMap<TM_TAB, QWidget*> mTabNoItem;
    QMap<TM_TAB, QString> mTooltipNameByTab;

    TransfersModel* mModel;
    TransfersCount mTransfersCount;

    bool mSearchFieldReturnPressed;

    TM_TAB mCurrentTab;
    QGraphicsDropShadowEffect* mShadowTab;
    QSet<Utilities::FileType> mFileTypesFilter;
    QTimer* mSpeedRefreshTimer;
    QTimer* mStatsRefreshTimer;

    Ui::TransferManagerDragBackDrop* mUiDragBackDrop;
    QWidget* mDragBackDrop;

    int mStorageQuotaState;
    QuotaState mTransferQuotaState;

    bool mFoundStalledIssues;

    void toggleTab(TM_TAB newTab);
    void refreshStateStats();
    void refreshTypeStats();
    void refreshFileTypesStats();
    void applyTextSearch(const QString& text);
    void checkActionAndMediaVisibility();
    void onFileTypeButtonClicked(TM_TAB tab, Utilities::FileType fileType, const QString& tabLabel);
    void checkPauseButtonVisibilityIfPossible();

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
    void on_tAllResults_clicked();
    void on_tDlResults_clicked();
    void on_tUlResults_clicked();
    void on_bPause_clicked();

    void onStalledIssuesStateChanged(bool state);
    void showStalledIssuesInfo();
    void on_bOpenLinks_clicked();
    void on_tCogWheel_clicked();
    void on_bDownload_clicked();
    void on_bUpload_clicked();
    void on_bCancelClearAll_clicked();
    void onCancelAllClicked();
    void on_leSearchField_returnPressed();

    void on_bArchives_clicked();
    void on_bDocuments_clicked();
    void on_bImages_clicked();
    void on_bAudio_clicked();
    void on_bVideos_clicked();
    void on_bOther_clicked();
    void on_bText_clicked();

    void onUpdatePauseState(bool isPaused);
    void onPauseStateChangedByTransferResume();
    void checkCancelAllButtonVisibility();
    void onPauseResumeVisibleRows(bool isPaused);
    void onCancelVisibleRows();

    void onTransfersDataUpdated();
    void refreshSearchStats();

    void onVerticalScrollBarVisibilityChanged(bool state);

    void refreshSpeed();
    void refreshView();

    void onScanningAnimationUpdate();
};

#endif // TRANSFERMANAGER_H

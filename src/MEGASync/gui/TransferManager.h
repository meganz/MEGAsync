#ifndef TRANSFERMANAGER_H
#define TRANSFERMANAGER_H

#include "BlockingGui.h"
#include "megaapi.h"
#include "Preferences.h"
#include "MenuItemAction.h"
#include "Utilities.h"
#include "TransferItem.h"
#include "TransfersModel.h"
#include "TransferQuota.h"

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

class TransferManager : public QDialog
{
    Q_OBJECT

public:
    enum TM_TAB
    {
        NO_TAB            = -1,
        ALL_TRANSFERS_TAB = 0,
        DOWNLOADS_TAB     = 1,
        UPLOADS_TAB       = 2,
        COMPLETED_TAB     = 3,
        SEARCH_TAB        = 4,
        TYPES_TAB_BASE    = 5,
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

    void pauseModel(bool state);

public slots:
    void onTransferQuotaStateChanged(QuotaState transferQuotaState);

signals:
    void viewedCompletedTransfers();
    void completedTransfersTabActive(bool);
    void userActivity();
    void showCompleted(bool showCompleted);
    void clearCompletedTransfers();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    void changeEvent(QEvent *event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragLeaveEvent(QDragLeaveEvent* event) override;
    void dropEvent(QDropEvent *event) override;
    void cancelAllUploads();
    void cancelAllDownloads();

private:
    static constexpr int SPEED_REFRESH_PERIOD_MS = 700;
    static constexpr int STATS_REFRESH_PERIOD_MS = 1000;

    Ui::TransferManager* mUi;
    mega::MegaApi* mMegaApi;

    Preferences* mPreferences;
    QPoint mDragPosition;
    QMap<TM_TAB, QFrame*> mTabFramesToggleGroup;
    QMap<TM_TAB, QLabel*> mNumberLabelsGroup;
    QMap<TM_TAB, QWidget*> mTabNoItem;

    TransfersModel* mModel;
    TransfersCount mTransfersCount;

    TM_TAB mCurrentTab;
    QGraphicsDropShadowEffect* mShadowTab;
    QSet<Utilities::FileType> mFileTypesFilter;
    QTimer* mSpeedRefreshTimer;
    QTimer* mStatsRefreshTimer;

    Ui::TransferManagerDragBackDrop* mUiDragBackDrop;
    QWidget* mDragBackDrop;
    BlockingUi* blockingUi = nullptr;

    void toggleTab(TM_TAB newTab);
    void refreshStateStats();
    void refreshTypeStats();
    void refreshFileTypesStats();
    void applyTextSearch(const QString& text);

private slots:
    void on_tCompleted_clicked();
    void on_tDownloads_clicked();
    void on_tUploads_clicked();
    void on_tAllTransfers_clicked();
    void on_tClearCompleted_clicked();
    void on_tSeePlans_clicked();
    void on_bSearch_clicked();
    void on_tSearchIcon_clicked();
    void on_bSearchString_clicked();
    void on_tSearchCancel_clicked();
    void on_tClearSearchResult_clicked();
    void on_tAllResults_clicked();
    void on_tDlResults_clicked();
    void on_tUlResults_clicked();
    void on_bPause_clicked();

    void on_bImportLinks_clicked();
    void on_tCogWheel_clicked();
    void on_bDownload_clicked();
    void on_bUpload_clicked();
    void on_bCancelClearAll_clicked();
    void on_leSearchField_returnPressed();

    void on_bArchives_clicked();
    void on_bDocuments_clicked();
    void on_bImages_clicked();
    void on_bMusic_clicked();
    void on_bVideos_clicked();
    void on_bOther_clicked();
    void on_bText_clicked();

    void onUpdatePauseState(bool isPaused);
    void onPauseStateChangedByTransferResume();
    void checkCancelAllButtonVisibility();

    void onTransfersDataUpdated();
    void refreshSearchStats();

    void onStorageStateChanged(int storageState);

    void refreshSpeed();
    void refreshView();
};

#endif // TRANSFERMANAGER_H

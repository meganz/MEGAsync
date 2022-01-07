#ifndef TRANSFERMANAGER_H
#define TRANSFERMANAGER_H

#include "megaapi.h"
#include "Preferences.h"
#include "MenuItemAction.h"
#include "Utilities.h"
#include "TransferItem2.h"
#include "QTransfersModel2.h"
#include "TransferQuota.h"

#include <QGraphicsEffect>
#include <QTimer>
#include <QDialog>
#include <QMenu>

namespace Ui {
class TransferManager;
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
        TYPE_OTHER_TAB    = TYPES_TAB_BASE + TransferData::FileType::TYPE_OTHER,
        TYPE_AUDIO_TAB    = TYPES_TAB_BASE + TransferData::FileType::TYPE_AUDIO,
        TYPE_VIDEO_TAB    = TYPES_TAB_BASE + TransferData::FileType::TYPE_VIDEO,
        TYPE_ARCHIVE_TAB  = TYPES_TAB_BASE + TransferData::FileType::TYPE_ARCHIVE,
        TYPE_DOCUMENT_TAB = TYPES_TAB_BASE + TransferData::FileType::TYPE_DOCUMENT,
        TYPE_IMAGE_TAB    = TYPES_TAB_BASE + TransferData::FileType::TYPE_IMAGE,
        TYPE_TEXT_TAB     = TYPES_TAB_BASE + TransferData::FileType::TYPE_TEXT,
    };

    explicit TransferManager(mega::MegaApi *megaApi, QWidget *parent = 0);
    void setActiveTab(int t);
    void disableGetLink(bool disable);
    ~TransferManager();

signals:
    void viewedCompletedTransfers();
    void completedTransfersTabActive(bool);
    void userActivity();
    void showCompleted(bool showCompleted);
    void cancelClearAllRows(bool cancel, bool clear);
    void cancelClearAllTransfers();

private:
    static constexpr int SPEED_REFRESH_PERIOD_MS = 700;
    static constexpr int STATS_REFRESH_PERIOD_MS = 1000;

    Ui::TransferManager* mUi;
    mega::MegaApi* mMegaApi;

    Preferences* mPreferences;
    QPoint mDragPosition;
    ThreadPool* mThreadPool;
    QMap<TM_TAB, QFrame*> mTabFramesToggleGroup;
    QMap<TransferData::FileType, QLabel*> mMediaNumberLabelsGroup;
    QMap<TM_TAB, QWidget*> mTabNoItem;

    QTransfersModel2* mModel;

    TM_TAB mCurrentTab;
    QGraphicsDropShadowEffect* mShadowTab;
    QSet<TransferData::FileType> mFileTypesFilter;
    QTimer* mSpeedRefreshTimer;
    QTimer* mStatsRefreshTimer;

    QMap<TM_TAB, long long> mNumberOfTransfersPerTab;
    QMap<TransferData::TransferTypes, long long> mNumberOfSearchResultsPerTypes;

    void toggleTab(TM_TAB newTab);
    bool refreshStateStats();
    void refreshTypeStats();
    void refreshFileTypesStats();
    void refreshSearchStats();

public slots:
    void onTransferQuotaStateChanged(QuotaState transferQuotaState);

private slots:
    void on_tCompleted_clicked();
    void on_tDownloads_clicked();
    void on_tUploads_clicked();
    void on_tAllTransfers_clicked();
    void on_bClearAll_clicked();
    void on_tClearCompleted_clicked();
    void on_tSeePlans_clicked();
    void on_bSearch_clicked();
    void on_tSearchIcon_clicked();
    void on_tSearchCancel_clicked();
    void on_tClearSearchResult_clicked();
    void on_tAllResults_clicked();
    void on_tDlResults_clicked();
    void on_tUlResults_clicked();

    void on_bImportLinks_clicked();
    void on_tCogWheel_clicked();
    void on_bDownload_clicked();
    void on_bUpload_clicked();
    void on_leSearchField_returnPressed();

    void on_bArchives_clicked();
    void on_bDocuments_clicked();
    void on_bImages_clicked();
    void on_bMusic_clicked();
    void on_bVideos_clicked();
    void on_bOther_clicked();
    void on_bText_clicked();

    void onUpdatePauseState(bool isPaused);
    void onTransfersInModelChanged(bool weHaveTransfers);

    void onStorageStateChanged(int storageState);

    void refreshSpeed();
    void refreshStats();

    void refreshView();

protected:
    bool eventFilter(QObject *obj, QEvent *event);
    void changeEvent(QEvent *event);
};

#endif // TRANSFERMANAGER_H

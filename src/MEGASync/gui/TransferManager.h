#ifndef TRANSFERMANAGER_H
#define TRANSFERMANAGER_H

#include "megaapi.h"
#include "Preferences.h"
#include "MenuItemAction.h"
#include "Utilities.h"
#include "TransferItem2.h"
#include "QTransfersModel2.h"

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
    enum TM_TABS
    {
        NO_TAB            = -1,
        ALL_TRANSFERS_TAB = 0,
        DOWNLOADS_TAB     = 1,
        UPLOADS_TAB       = 2,
        COMPLETED_TAB     = 3,
        SEARCH_TAB        = 4,
        TYPES_TAB_BASE    = 5,
        TYPE_OTHER_TAB    = TYPES_TAB_BASE + TransferData::FileTypes::TYPE_OTHER,
        TYPE_AUDIO_TAB    = TYPES_TAB_BASE + TransferData::FileTypes::TYPE_AUDIO,
        TYPE_VIDEO_TAB    = TYPES_TAB_BASE + TransferData::FileTypes::TYPE_VIDEO,
        TYPE_ARCHIVE_TAB  = TYPES_TAB_BASE + TransferData::FileTypes::TYPE_ARCHIVE,
        TYPE_DOCUMENT_TAB = TYPES_TAB_BASE + TransferData::FileTypes::TYPE_DOCUMENT,
        TYPE_IMAGE_TAB    = TYPES_TAB_BASE + TransferData::FileTypes::TYPE_IMAGE,
        TYPE_TEXT_TAB     = TYPES_TAB_BASE + TransferData::FileTypes::TYPE_TEXT,
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
    void pauseStateChanged(bool pauseState);

private:
    static constexpr int SPEED_REFRESH_PERIOD_MS = 500;
    static constexpr int STATS_REFRESH_PERIOD_MS = 200;
    static const QSet<int> ACTIVE_STATES;
    static const QSet<int> FINISHED_STATES;

    Ui::TransferManager* mUi;
    mega::MegaApi* mMegaApi;
    Preferences* mPreferences;
    QPoint mDragPosition;
    ThreadPool* mThreadPool;
    QMap<int, QFrame*> mTabFramesToggleGroup;
    QMap<TransferData::FileTypes, QLabel*> mMediaNumberLabelsGroup;
    QMap<TM_TABS, QWidget*> mTabNoItem;

    QTransfersModel2* mModel;

    TM_TABS mCurrentTab;
    QGraphicsDropShadowEffect* mShadowTab;
    QGraphicsDropShadowEffect* mShadowSearch;
    QSet<TransferData::FileTypes> mFileTypesFilter;
    QTimer* mSpeedRefreshTimer;
    QTimer* mStatsRefreshTimer;

    QVector<long long> mNumberOfTransfersPerTab;

    void toggleTab(TM_TABS tab);
//    void updateFileTypeFilter(TransferData::FileTypes fileType);
    bool refreshStateStats();
    void refreshTypeStats();
    void refreshFileTypesStats();

public slots:
    void updateState();

private slots:
    void on_tCompleted_clicked();
    void on_tDownloads_clicked();
    void on_tUploads_clicked();
    void on_tAllTransfers_clicked();
//    void on_bPause_clicked();
    void on_bClearAll_clicked();
    void on_bSearch_clicked();
    void on_tSearchIcon_clicked();
    void on_tSearchCancel_clicked();
    void on_tClearSearchResult_clicked();

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

    void refreshSpeed();
    void refreshStats();

protected:
    bool eventFilter(QObject *obj, QEvent *event);
    void changeEvent(QEvent *event);
//    void paintEvent(QPaintEvent* event);
//    void mouseMoveEvent(QMouseEvent *event);
//    void mousePressEvent(QMouseEvent *event);
//    void mouseReleaseEvent(QMouseEvent *event);
};

#endif // TRANSFERMANAGER_H

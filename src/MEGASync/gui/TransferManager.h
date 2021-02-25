#ifndef TRANSFERMANAGER_H
#define TRANSFERMANAGER_H


#include "megaapi.h"
#include "Preferences.h"
#include "MenuItemAction.h"
#include "Utilities.h"
#include "TransferItem2.h"

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
        ALL_TRANSFERS_TAB = 0,
        DOWNLOADS_TAB     = 1,
        UPLOADS_TAB       = 2,
        COMPLETED_TAB     = 3,
    };

    explicit TransferManager(mega::MegaApi *megaApi, QWidget *parent = 0);
    void setActiveTab(int t);
    void updatePauseState(bool isPaused, QString toolTipText);
    void disableGetLink(bool disable);
    void updateNumberOfCompletedTransfers(int num);
    ~TransferManager();

signals:
    void viewedCompletedTransfers();
    void completedTransfersTabActive(bool);
    void userActivity();

private:
    static constexpr int COMPLETED_ITEMS_LIMIT = 999;

    Ui::TransferManager* mUi;
    mega::MegaApi* mMegaApi;
    Preferences* mPreferences;
    QPoint mDragPosition;
    QTimer* mRefreshTransferTime;
    ThreadPool* mThreadPool;
    std::array<QFrame*, 4> mTabFramesToggleGroup;
    TM_TABS mCurrentTab;
    QSet<TransferData::FileTypes> mFileTypesFilter;
    QSet<int> mTransferStatesFilter;
    QSet<int> mTransferTypesFilter;


    void onTransfersActive(bool exists);
    void toggleTab(TM_TABS tab);

public slots:
    void updateState();

private slots:
    void on_tCompleted_clicked();
    void on_tDownloads_clicked();
    void on_tUploads_clicked();
    void on_tAllTransfers_clicked();
    void on_bPause_clicked();
    void on_bClearAll_clicked();
    void on_bSearch_clicked();
    void on_tSearchIcon_clicked();
    void on_tSearchCancel_clicked();
    void on_tClearSearchResult_clicked();
    void on_tPauseResumeAll_clicked();
    void on_tCancelAll_clicked();
    void on_bArchives_clicked();
    void on_bDocuments_clicked();
    void on_bImages_clicked();
    void on_bMusic_clicked();
    void on_bVideos_clicked();
    void on_bOther_clicked();

    void refreshFinishedTime();

protected:
    void changeEvent(QEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
};

#endif // TRANSFERMANAGER_H

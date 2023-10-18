#ifndef TRANSFERSWIDGET_H
#define TRANSFERSWIDGET_H

#include <QWidget>
#include "MegaTransferDelegate.h"
#include "TransfersManagerSortFilterProxyModel.h"
#include "MegaDelegateHoverManager.h"
#include "TransferManagerLoadingItem.h"
#include "ViewLoadingScene.h"
#include "ButtonIconManager.h"

#include <QToolButton>
#include <QStandardItemModel>
#include <QMessageBox>

namespace Ui {
class TransfersWidget;
}

class MegaApplication;
class TransfersWidget : public QWidget
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
        FAILED_TAB        = 4,
        SEARCH_TAB        = 5,
        TYPES_TAB_BASE    = 6,
        TYPE_OTHER_TAB    = TYPES_TAB_BASE + toInt(Utilities::FileType::TYPE_OTHER),
        TYPE_AUDIO_TAB    = TYPES_TAB_BASE + toInt(Utilities::FileType::TYPE_AUDIO),
        TYPE_VIDEO_TAB    = TYPES_TAB_BASE + toInt(Utilities::FileType::TYPE_VIDEO),
        TYPE_ARCHIVE_TAB  = TYPES_TAB_BASE + toInt(Utilities::FileType::TYPE_ARCHIVE),
        TYPE_DOCUMENT_TAB = TYPES_TAB_BASE + toInt(Utilities::FileType::TYPE_DOCUMENT),
        TYPE_IMAGE_TAB    = TYPES_TAB_BASE + toInt(Utilities::FileType::TYPE_IMAGE),
        TYPES_LAST
    };
    Q_ENUM(TM_TAB)

    explicit TransfersWidget(QWidget *parent = 0);

    void setupTransfers();

    void textFilterChanged(const QString& pattern);
    void textFilterTypeChanged(const TransferData::TransferTypes transferTypes);
    void filtersChanged(const TransferData::TransferTypes transferTypes,
                        const TransferData::TransferStates transferStates,
                        const Utilities::FileTypes fileTypes);
    void transferFilterReset();
    void mouseRelease(const QPoint& point);
    void setCurrentTab(TM_TAB);
    TM_TAB getCurrentTab();
    void setScanningWidgetVisible(bool state);

    bool isLoadingViewSet();
    void setSortCriterion(int sortBy, Qt::SortOrder order);

    struct CancelClearButtonInfo
    {
         bool    visible;
         bool    clearAction;
         QString cancelClearTooltip;

         CancelClearButtonInfo():visible(true),clearAction(false){}

         bool isInit(){return !cancelClearTooltip.isEmpty();}
    };
    struct HeaderInfo
    {
        QString headerTime;

        QString headerSpeed;
    };
    void updateHeaders();

    TransfersModel* getModel();
    TransfersManagerSortFilterProxyModel* getProxyModel() {return mProxyModel;}
    ~TransfersWidget();

    void setTopParent(QWidget* parent);

public slots:
    void onHeaderItemClicked(int sortBy, Qt::SortOrder order);
    void on_tPauseResumeVisible_toggled(bool state);
    void on_tCancelClearVisible_clicked();
    void onUiBlockedRequested();
    void onUiUnblockedRequested();

protected:
    void changeEvent(QEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void onUiLoadingViewVisibilityChanged(bool state);
    void onUiUnblockedAndFilter();
    void onModelChanged();
    void onModelAboutToBeChanged();
    void onRowsAboutToBeMoved(int scrollTo);
    void onPauseResumeTransfer(bool pause);
    void onCancelClearButtonPressedOnDelegate();
    void onRetryButtonPressedOnDelegate();
    void onVerticalScrollBarVisibilityChanged(bool state);
    void onCheckPauseResumeButton();
    void togglePauseResumeButton(bool state);
    void onCheckCancelClearButton();
    void updateCancelClearButtonTooltip();
    void updatePauseResumeButtonTooltip();

private:
    Ui::TransfersWidget *ui;
    TransfersModel *mModel;
    TransfersManagerSortFilterProxyModel *mProxyModel;
    MegaTransferDelegate *tDelegate;
    MegaDelegateHoverManager mDelegateHoverManager;
    bool mClearMode;
    MegaApplication *app;
    TM_TAB mCurrentTab;
    bool mScanningIsActive;
    QList<int> mScrollToAfterMovingRow;

    HeaderInfo mHeaderInfo;
    CancelClearButtonInfo mCancelClearInfo;

    ButtonIconManager mButtonIconManager;

    void configureTransferView();
    void clearOrCancel(const QList<QExplicitlySharedDataPointer<TransferData>>& pool, int state, int firstRow);
    void updateHeaderItems();
    void selectAndScrollToMovedTransfer(QAbstractItemView::ScrollHint scrollHint = QAbstractItemView::PositionAtCenter);

    QString getClearTooltip(TM_TAB tab);
    QString getCancelTooltip(TM_TAB tab);
    QString getCancelAndClearTooltip(TM_TAB tab);
    QString getResumeTooltip(TM_TAB tab);
    QString getPauseTooltip(TM_TAB tab);

signals:
    void clearTransfers(int firstRow, int amount);
    void updateSearchFilter(const QString& pattern);
    void pauseResumeVisibleRows(bool state);
    void transferPauseResumeStateChanged(bool isPaused);
    void cancelClearVisibleRows();
    void changeToAllTransfersTab();

    void loadingViewVisibilityChanged(bool);
    void disableTransferManager(bool);
    void sortCriterionChanged(int sortBy, Qt::SortOrder order);

};

#endif // TRANSFERSWIDGET_H

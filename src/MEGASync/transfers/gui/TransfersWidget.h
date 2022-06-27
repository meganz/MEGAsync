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

    void onDialogHidden();
    void onDialogShown();

    struct CancelClearButtonInfo
    {
         bool    visible;
         bool    clearAction;
         QString cancelClearTooltip;

         CancelClearButtonInfo():clearAction(false), visible(true){}

         bool isInit(){return !cancelClearTooltip.isEmpty();}
    };

    struct HeaderInfo
    {
        QString headerTime;

        QString headerSpeed;
        QString pauseTooltip;
        QString resumeTooltip;
    };

    void updateHeaders();

    TransfersModel* getModel();
    TransfersManagerSortFilterProxyModel* getProxyModel() {return mProxyModel;}
    ~TransfersWidget();

public slots:
    void onHeaderItemClicked(int sortBy, Qt::SortOrder order);
    void on_tPauseResumeVisible_toggled(bool state);
    void on_tCancelClearVisible_clicked();

protected:
    void changeEvent(QEvent *event) override;

private slots:
    void onUiBlocked();
    void onUiUnblocked();
    void onUiUnblockedAndFilter();
    void onModelChanged();
    void onModelAboutToBeChanged();
    void onPauseResumeTransfer(bool pause);
    void onCancelClearButtonPressedOnDelegate();
    void onRetryButtonPressedOnDelegate();
    void onVerticalScrollBarVisibilityChanged(bool state);
    void onCheckPauseResumeButton();
    void onCheckCancelClearButton();

private:
    Ui::TransfersWidget *ui;
    TransfersModel *model;
    TransfersManagerSortFilterProxyModel *mProxyModel;
    MegaTransferDelegate *tDelegate;
    ViewLoadingScene<TransferManagerLoadingItem> mLoadingScene;
    MegaDelegateHoverManager mDelegateHoverManager;
    bool mClearMode;
    MegaApplication *app;
    TM_TAB mCurrentTab;
    QMap<TransfersWidget::TM_TAB, QString> mTooltipNameByTab;

    HeaderInfo mHeaderInfo;
    CancelClearButtonInfo mCancelClearInfo;

    ButtonIconManager mButtonIconManager;

    void configureTransferView();
    void clearOrCancel(const QList<QExplicitlySharedDataPointer<TransferData>>& pool, int state, int firstRow);
    void updateTimersState();
    void updateHeaderItems();

    QTimer mCheckPauseResumeButtonTimer;
    QTimer mCheckCancelClearButtonTimer;

signals:
    void clearTransfers(int firstRow, int amount);
    void updateSearchFilter(const QString& pattern);
    void applyFilter();
    void pauseResumeVisibleRows(bool state);
    void transferPauseResumeStateChanged(bool isPaused);
    void cancelClearVisibleRows();
    void changeToAllTransfersTab();

    void disableTransferManager(bool);

};

#endif // TRANSFERSWIDGET_H

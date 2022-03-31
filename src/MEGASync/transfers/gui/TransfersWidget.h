#ifndef TRANSFERSWIDGET_H
#define TRANSFERSWIDGET_H

#include <QWidget>
#include "MegaTransferDelegate.h"
#include "TransfersStateInfoWidget.h"
#include "TransfersManagerSortFilterProxyModel.h"
#include "MegaDelegateHoverManager.h"
#include "TransferManagerLoadingItem.h"
#include "ViewLoadingScene.h"

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
    explicit TransfersWidget(QWidget *parent = 0);

    void setupTransfers();
    void refreshTransferItems();
    void pausedTransfers(bool paused);
    void disableGetLink(bool disable);

    void textFilterChanged(const QString& pattern);
    void textFilterTypeChanged(const TransferData::TransferTypes transferTypes);
    void filtersChanged(const TransferData::TransferTypes transferTypes,
                        const TransferData::TransferStates transferStates,
                        const Utilities::FileTypes fileTypes);
    void transferFilterReset();
    void updateHeaderItems(const QString& headerTime,
                           const QString& cancelClearTooltip,
                           const QString& headerSpeed);

    TransfersModel* getModel();
    TransfersManagerSortFilterProxyModel* getProxyModel() {return mProxyModel;}
    ~TransfersWidget();

public slots:
    void on_pHeaderSize_clicked();
    void onHeaderItemClicked(int sortBy, Qt::SortOrder order);
    void on_tPauseResumeVisible_toggled(bool state);
    void on_tCancelClearVisible_clicked();
    void onPauseStateChanged(bool pauseState);

protected:
    void changeEvent(QEvent *event) override;

private slots:
    void onUiBlocked();
    void onUiUnblocked();
    void onModelChanged();
    void onModelAboutToBeChanged();
    void onPauseResumeButtonCheckedOnDelegate(bool pause);
    void onCancelClearButtonPressedOnDelegate();
    void onRetryButtonPressedOnDelegate();
    void onCheckCancelButtonVisibility(bool state);
    void onActiveTransferCounterChanged(bool state);
    void onPausedTransferCounterChanged(bool state);
    void onVerticalScrollBarVisibilityChanged(bool state);

private:
    Ui::TransfersWidget *ui;
    TransfersModel *model;
    TransfersManagerSortFilterProxyModel *mProxyModel;
    MegaTransferDelegate *tDelegate;
    ViewLoadingScene<TransferManagerLoadingItem> mLoadingScene;
    MegaDelegateHoverManager mDelegateHoverManager;
    bool mClearMode;
    MegaApplication *app;

    void configureTransferView();
    void clearOrCancel(const QList<QExplicitlySharedDataPointer<TransferData>>& pool, int state, int firstRow);

signals:
    void clearTransfers(int firstRow, int amount);
    void updateSearchFilter(const QString& pattern);
    void applyFilter();
    void pauseResumeVisibleRows(bool state);
    void cancelClearVisibleRows();

    void disableTransferManager(bool);

};

#endif // TRANSFERSWIDGET_H

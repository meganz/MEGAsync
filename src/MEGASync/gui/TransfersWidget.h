#ifndef TRANSFERSWIDGET_H
#define TRANSFERSWIDGET_H

#include <QWidget>
#include "MegaTransferDelegate.h"
#include "TransfersStateInfoWidget.h"
#include "TransfersSortFilterProxyModel.h"
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

    void cancelClearAll();

    int rowCount();

    TransfersModel* getModel();
    TransfersSortFilterProxyModel* getProxyModel() {return mProxyModel;}
    ~TransfersWidget();

public slots:
    void on_pHeaderName_clicked();
    void on_pHeaderSize_clicked();
    void on_tPauseResumeVisible_toggled(bool state);
    void on_tCancelClearVisible_clicked();
    void onTransferAdded();
    void onShowCompleted(bool showCompleted);
    void onPauseStateChanged(bool pauseState);

protected:
    void changeEvent(QEvent *event);

private slots:
    void onModelChanged();
    void onModelAboutToBeChanged();
    void onPauseResumeButtonCheckedOnDelegate(bool pause);

private:
    enum HeaderState
    {
        HS_SORT_ASCENDING = 0,
        HS_SORT_DESCENDING,
        HS_SORT_PRIORITY,
        HS_NB_STATES,
    };

    Ui::TransfersWidget *ui;
    TransfersModel *model;
    TransfersSortFilterProxyModel *mProxyModel;
    MegaTransferDelegate *tDelegate;
    ViewLoadingScene<TransferManagerLoadingItem> mLoadingScene;
    MegaDelegateHoverManager mDelegateHoverManager;
    bool mClearMode;
    MegaApplication *app;
    HeaderState mHeaderNameState;
    HeaderState mHeaderSizeState;

    void configureTransferView();
    void clearOrCancel(const QList<QExplicitlySharedDataPointer<TransferData>>& pool, int state, int firstRow);

    void setHeaderState(QPushButton* header, HeaderState state);

signals:
    void clearTransfers(int firstRow, int amount);
    void updateSearchFilter(const QString& pattern);
    void applyFilter();
    void pauseResumeVisibleRows(bool state);
    void cancelClearVisibleRows();
    void pauseResumeAllRows(bool pauseState);
    void cancelClearAllRows();

    void disableTransferManager(bool);

};

#endif // TRANSFERSWIDGET_H

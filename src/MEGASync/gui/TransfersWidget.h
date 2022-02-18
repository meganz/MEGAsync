#ifndef TRANSFERSWIDGET_H
#define TRANSFERSWIDGET_H

#include <QWidget>
#include "MegaTransferDelegate.h"
#include "TransfersStateInfoWidget.h"
#include "TransfersSortFilterProxyModel.h"
#include "MegaDelegateHoverManager.h"

#include <QToolButton>
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
    void filtersChanged(const TransferData::TransferTypes transferTypes,
                        const TransferData::TransferStates transferStates,
                        const TransferData::FileTypes fileTypes);
    void transferFilterReset();

    void cancelClearAll();

    int rowCount();

    QTransfersModel* getModel();
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
    void onProxyActivityLaunchTimeout();
    void onProxyActivityCloseTimeout();
    void onPauseResumeButtonCheckedOnDelegate(bool pause);

private:
    static constexpr int PROXY_ACTIVITY_CLOSE_TIMEOUT_MS = 1000;
    static constexpr int PROXY_ACTIVITY_LAUNCH_TIMEOUT_MS = 300;

    enum HeaderState
    {
        HS_SORT_ASCENDING = 0,
        HS_SORT_DESCENDING,
        HS_SORT_PRIORITY,
        HS_NB_STATES,
    };

    Ui::TransfersWidget *ui;
    QTransfersModel *model2;
    TransfersSortFilterProxyModel *mProxyModel;
    MegaTransferDelegate *tDelegate;
    MegaDelegateHoverManager mDelegateHoverManager;
    bool mClearMode;
    MegaApplication *app;
    HeaderState mHeaderNameState;
    HeaderState mHeaderSizeState;
    ThreadPool* mThreadPool;
    bool mModelIsChanging;

    QTimer* mProxyActivityLaunchTimer;
    QTimer* mProxyActivityCloseTimer;
    QMessageBox* mProxyActivityMessage;

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

};

#endif // TRANSFERSWIDGET_H

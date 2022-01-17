#ifndef TRANSFERSWIDGET_H
#define TRANSFERSWIDGET_H

#include <QWidget>
#include "MegaTransferDelegate.h"
#include "TransfersStateInfoWidget.h"
#include "TransfersSortFilterProxyModel.h"

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
    void transferFilterReset(bool invalidate = false);
    void transferFilterApply(bool invalidate = true);

    int rowCount();

    QTransfersModel* getModel2();
    TransfersSortFilterProxyModel* getProxyModel() {return mProxyModel;}
    ~TransfersWidget();

    bool areTransfersActive();

signals:
    void clearTransfers(int firstRow, int amount);
    void updateSearchFilter(const QString& pattern);
    void applyFilter();
    void pauseResumeAllRows(bool pauseState);
    void cancelClearAllRows(bool cancel, bool clear);

private:
    static constexpr int PROXY_ACTIVITY_TIMEOUT_MS = 100;

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
    MegaTransferDelegate *tDelegate2;
    bool mIsPaused;
    MegaApplication *app;
    HeaderState mHeaderNameState;
    HeaderState mHeaderSizeState;
    ThreadPool* mThreadPool;

    QTimer* mProxyActivityTimer;
    QMessageBox* mProxyActivityMessage;

    void configureTransferView();
    void clearOrCancel(const QList<QExplicitlySharedDataPointer<TransferData>>& pool, int state, int firstRow);

    void setHeaderState(QPushButton* header, HeaderState state);

public slots:
    void on_pHeaderName_clicked();
    void on_pHeaderSize_clicked();
    void on_tPauseResumeAll_clicked();
    void on_tCancelAll_clicked();
    void onTransferAdded();
    void onShowCompleted(bool showCompleted);
    void onPauseStateChanged(bool pauseState);

protected:
    void changeEvent(QEvent *event);
};

#endif // TRANSFERSWIDGET_H

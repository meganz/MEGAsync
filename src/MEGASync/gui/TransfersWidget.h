#ifndef TRANSFERSWIDGET_H
#define TRANSFERSWIDGET_H

#include <QWidget>
#include "TransferItem.h"
#include "QTransfersModel.h"
#include "QActiveTransfersModel.h"
#include "QFinishedTransfersModel.h"
#include "MegaTransferDelegate.h"
#include "MegaTransferDelegate2.h"
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
    void setupFinishedTransfers(QList<mega::MegaTransfer* > transferData, QTransfersModel::ModelType modelType = QTransfersModel::TYPE_FINISHED);
    void setupTransfers(std::shared_ptr<mega::MegaTransferData> transferData, QTransfersModel::ModelType type);
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

    QTransfersModel *getModel();
    QTransfersModel2* getModel2();
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
        SORT_DESCENDING = 0,
        SORT_ASCENDING,
        SORT_DEFAULT,
        NB_STATES,
    };

    Ui::TransfersWidget *ui;
    QTransfersModel *model;
    QTransfersModel2 *model2;
    TransfersSortFilterProxyModel *mProxyModel;
    MegaTransferDelegate *tDelegate;
    MegaTransferDelegate2 *tDelegate2;
    QTransfersModel::ModelType mType;
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

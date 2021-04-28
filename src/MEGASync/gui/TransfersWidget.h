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
    void fileTypeFilterChanged(const TransferData::FileTypes fileTypes);
    void transferStateFilterChanged(const TransferData::TransferStates transferStates);
    void transferTypeFilterChanged(const TransferData::TransferTypes transferTypes);
    void transferFilterReset();
    void transferFilterApply();

    int rowCount();

    QTransfersModel *getModel();
    QTransfersModel2* getModel2();
    TransfersSortFilterProxyModel* getProxyModel() {return mProxyModel;}
    ~TransfersWidget();

    bool areTransfersActive();

signals:
    void clearTransfers(int firstRow, int amount);
    //    void updateSearchFilter(const QRegularExpression& pattern);
    void updateSearchFilter(const QString& pattern);
    void applyFilter();
    void pauseResumeAllRows(bool pauseState);
    void cancelClearAllRows(bool cancel, bool clear);

private:
    Ui::TransfersWidget *ui;
    QTransfersModel *model;
    QTransfersModel2 *model2;
    TransfersSortFilterProxyModel *mProxyModel;
    MegaTransferDelegate *tDelegate;
    MegaTransferDelegate2 *tDelegate2;
    QTransfersModel::ModelType mType;
    bool mIsPaused;
    MegaApplication *app;
    int mHeaderNameState;
    int mHeaderSizeState;

    void configureTransferView();
    void clearOrCancel(const QList<QExplicitlySharedDataPointer<TransferData>>& pool, int state, int firstRow);

    void setHeaderState(QPushButton* header, int state);

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

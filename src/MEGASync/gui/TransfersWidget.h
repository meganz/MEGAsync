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
    void clearTransfers();
    void pausedTransfers(bool paused);
    void disableGetLink(bool disable);


    void textFilterChanged(QRegExp regExp);
    void fileTypeFilterChanged(QSet<TransferData::FileTypes> fileType);
    void transferStateFilterChanged(QSet<int> transferStates);
    void transferTypeFilterChanged(QSet<int> transferTypes);

    QTransfersModel *getModel();
    ~TransfersWidget();

    bool areTransfersActive();

private:
    Ui::TransfersWidget *ui;
    QTransfersModel *model;
    QTransfersModel2 *model2;
    TransfersSortFilterProxyModel *mProxyModel;
    MegaTransferDelegate *tDelegate;
    MegaTransferDelegate2 *tDelegate2;
    QTransfersModel::ModelType mType;
    int isPaused;
    MegaApplication *app;

private:
    void configureTransferView();

private slots:
    void noTransfers();
    void onTransferAdded();
    void on_tPauseResumeAll_clicked();
    void on_tCancelAll_clicked();


protected:
    void changeEvent(QEvent *event);
};

#endif // TRANSFERSWIDGET_H

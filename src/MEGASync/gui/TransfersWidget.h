#ifndef TRANSFERSWIDGET_H
#define TRANSFERSWIDGET_H

#include <QWidget>
#include "TransferItem.h"
#include "QTransfersModel.h"
#include "QActiveTransfersModel.h"
#include "QFinishedTransfersModel.h"
#include "MegaTransferDelegate.h"
#include "TransfersStateInfoWidget.h"
#include <QToolButton>

namespace Ui {
class TransfersWidget;
}

class TransfersWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TransfersWidget(QWidget *parent = 0);
    void setupFinishedTransfers(QList<mega::MegaTransfer* > transferData);
    void setupTransfers(mega::MegaTransferData *transferData, int type);
    void refreshTransferItems();
    void clearTransfers();
    void pausedTransfers(bool paused);
    void disableGetLink(bool disable);
    QTransfersModel *getModel();
    ~TransfersWidget();

    bool areTransfersActive();

private:
    Ui::TransfersWidget *ui;
    QList<TransferItem *> activeTransfers;
    QTransfersModel *model;
    MegaTransferDelegate *tDelegate;
    int type;
    int isPaused;

private:
    void configureTransferView();

private slots:
    void noTransfers();
    void onTransferAdded();

protected:
    void changeEvent(QEvent *event);
};

#endif // TRANSFERSWIDGET_H

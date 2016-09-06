#ifndef TRANSFERSWIDGET_H
#define TRANSFERSWIDGET_H

#include <QWidget>
#include "TransferItem.h"
#include "QTransfersModel.h"
#include "MegaTransferDelegate.h"

namespace Ui {
class TransfersWidget;
}

class TransfersWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TransfersWidget(QWidget *parent = 0);
    void setupTransfers(mega::MegaTransferData *transferData, int type);
    void clearTransfers();
    void pausedTransfers(bool paused);
    QTransfersModel *getModel();
    ~TransfersWidget();

private:
    Ui::TransfersWidget *ui;
    QList<TransferItem *> activeTransfers;
    QTransfersModel *model;
    MegaTransferDelegate *tDelegate;
    int type;

private slots:
    void noTransfers();
    void onTransferAdded();

protected:
    void changeEvent(QEvent *event);
};

#endif // TRANSFERSWIDGET_H

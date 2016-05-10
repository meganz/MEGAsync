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
    void setupTransfers(mega::MegaTransferList *tList, int type);
    ~TransfersWidget();

private:
    Ui::TransfersWidget *ui;
    QList<TransferItem *> activeTransfers;
    QTransfersModel *model;
    MegaTransferDelegate *tDelegate;

};

#endif // TRANSFERSWIDGET_H

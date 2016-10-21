#ifndef TRANSFERSWIDGET_H
#define TRANSFERSWIDGET_H

#include <QWidget>
#include "TransferItem.h"
#include "QTransfersModel.h"
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
    void setupTransfers(QList<mega::MegaTransfer* > transferData, int type);
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
    TransfersStateInfoWidget *overlay;
    QList<TransferItem *> activeTransfers;
    QTransfersModel *model;
    MegaTransferDelegate *tDelegate;
    int type;
    int isPaused;
    bool transfersActive;

private:
    void configureTransferView();

private slots:
    void noTransfers();
    void onTransferAdded();

protected:
    void changeEvent(QEvent *event);
};

#endif // TRANSFERSWIDGET_H

#ifndef QFINISHEDTRANSFERSMODEL_H
#define QFINISHEDTRANSFERSMODEL_H

#include <QAbstractItemModel>
#include <QCache>
#include "TransferItem.h"
#include <megaapi.h>
#include "QTMegaTransferListener.h"
#include <deque>
#include "QTransfersModel.h"

class QFinishedTransfersModel : public QTransfersModel
{
    Q_OBJECT

public:
    explicit QFinishedTransfersModel(QList<mega::MegaTransfer *>finishedTransfers, int type = QTransfersModel::TYPE_FINISHED, QObject *parent = 0);
    void setupModelTransfers();

    virtual mega::MegaTransfer *getTransferByTag(int tag);

    virtual void onTransferFinish(mega::MegaApi* api, mega::MegaTransfer *transfer, mega::MegaError* e);

protected:
    void insertTransfer(mega::MegaTransfer *t);

private slots:
    void refreshTransferItem(int tag);
    void retryAllFailedTransfers(); 

public slots:
    void removeAllTransfers();
    void removeTransferByTag(int transferTag);
    void retryAllTransfers();
};

#endif // QFINISHEDTRANSFERSMODEL_H

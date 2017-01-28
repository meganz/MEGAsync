#ifndef QACTIVETRANSFERSMODEL_H
#define QACTIVETRANSFERSMODEL_H

#include <QAbstractItemModel>
#include <QCache>
#include "TransferItem.h"
#include <megaapi.h>
#include "QTMegaTransferListener.h"
#include <deque>
#include "QTransfersModel.h"

typedef bool (*comparator_function)(TransferItemData* i, TransferItemData *j);
typedef std::deque<TransferItemData*>::iterator transfer_it;

class QActiveTransfersModel : public QTransfersModel
{
    Q_OBJECT

public:
    explicit QActiveTransfersModel(int type, mega::MegaTransferData *transferData, QObject *parent = 0);
    void setupModelTransfers();
    void removeTransferByTag(int transferTag);
    void removeAllTransfers();

    // Drag & drop
    QMimeData *mimeData(const QModelIndexList & indexes) const;
    virtual Qt::ItemFlags flags(const QModelIndex&index) const;
    virtual Qt::DropActions supportedDropActions() const;
    virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);

    virtual mega::MegaTransfer *getTransferByTag(int tag);

    // MegaApi callbacks
    virtual void onTransferStart(mega::MegaApi *api, mega::MegaTransfer *transfer);
    virtual void onTransferFinish(mega::MegaApi* api, mega::MegaTransfer *transfer, mega::MegaError* e);
    virtual void onTransferUpdate(mega::MegaApi *api, mega::MegaTransfer *transfer);
    virtual void onTransferTemporaryError(mega::MegaApi *api, mega::MegaTransfer *transfer, mega::MegaError* e);

protected:
    void updateTransferInfo(mega::MegaTransfer *transfer);

private slots:
    void refreshTransferItem(int tag);
};

#endif // QACTIVETRANSFERSMODEL_H

#include "QFinishedTransfersModel.h"
#include "MegaApplication.h"
#include <assert.h>

using namespace mega;

QFinishedTransfersModel::QFinishedTransfersModel(QList<MegaTransfer *> finishedTransfers, int type, QObject *parent) :
    QTransfersModel(type, parent)
{
    int numTransfers = finishedTransfers.size();
    if (numTransfers)
    {
        for (int i = 0; i < numTransfers; i++)
        {
            MegaTransfer *transfer = finishedTransfers.at(i);
            insertTransfer(transfer);
        }
    }
}

void QFinishedTransfersModel::insertTransfer(MegaTransfer *t)
{
    QPointer<QFinishedTransfersModel> model = this;
    MegaTransfer *transfer = t->copy(); //Copy transfer to avoid deletions by finishedtransfers if QMap reaches limit.

    mThreadPool->push([this, model, transfer]()
    {//thread pool function
        if (!model)
        {

            delete transfer;
            return;
        }

        bool isPublicNode = false;
        int access = MegaShare::ACCESS_UNKNOWN;
        MegaNode *ownNode = ((MegaApplication*)qApp)->getMegaApi()->getNodeByHandle(transfer->getNodeHandle());
        if (ownNode)
        {
           access = ((MegaApplication*)qApp)->getMegaApi()->getAccess(ownNode);
           if (access == MegaShare::ACCESS_OWNER)
           {
               isPublicNode = true;
           }
           delete ownNode;
        }

        Utilities::queueFunctionInAppThread([this, model, isPublicNode, access, transfer]()
        {//queued function
            if (model)
            {
                TransferItemData *item = new TransferItemData(transfer);
                item->data.publicNode = isPublicNode;
                item->data.nodeAccess = access;

                if ((int)transfers.size() == (int)Preferences::MAX_COMPLETED_ITEMS)
                {
                    TransferItemData *t = transferOrder.back();
                    int row = int(transferOrder.size()) - 1;
                    beginRemoveRows(QModelIndex(), row, row);
                    transfers.remove(t->data.tag);
                    transferOrder.pop_back();
                    transferItems.remove(t->data.tag);
                    endRemoveRows();
                    delete t;
                }

                transfer_it it = transferOrder.begin();
                int row = 0;

                beginInsertRows(QModelIndex(), row, row);
                transfers.insert(item->data.tag, item);
                transferOrder.insert(it, item);
                endInsertRows();

                if (transferOrder.size() == 1)
                {
                    emit onTransferAdded();
                }                
            }

            delete transfer;
        });//end of queued function

    });// end of thread pool function;
}

void QFinishedTransfersModel::removeTransferByTag(int transferTag)
{
    TransferItemData *item =  transfers.value(transferTag);
    if (!item)
    {
        return;
    }

    int row = 0;
    transfer_it it;
    for (it = transferOrder.begin(); it != transferOrder.end() && (*it)->data.tag != transferTag; ++it)
    {
        ++row;
    }
    assert(static_cast<size_t>(row) < transferOrder.size());

    beginRemoveRows(QModelIndex(), row, row);
    transfers.remove(transferTag);
    transferOrder.erase(it);
    ((MegaApplication *)qApp)->removeFinishedTransfer(transferTag);
    transferItems.remove(transferTag);
    endRemoveRows();
    delete item;

    if (transfers.isEmpty())
    {
        emit noTransfers();
    }
}

void QFinishedTransfersModel::removeAllTransfers()
{
    if (transfers.size())
    {
        beginRemoveRows(QModelIndex(), 0, transfers.size() - 1);
        qDeleteAll(transfers);
        transfers.clear();
        transferOrder.clear();
        transferItems.clear();
        endRemoveRows();

        ((MegaApplication *)qApp)->removeAllFinishedTransfers();
    }

    emit noTransfers();
}

MegaTransfer *QFinishedTransfersModel::getTransferByTag(int tag)
{
    MegaTransfer *transfer = ((MegaApplication *)qApp)->getFinishedTransferByTag(tag);
    if (transfer)
    {
        return transfer->copy();
    }
    return NULL;
}

void QFinishedTransfersModel::onTransferFinish(MegaApi *, MegaTransfer *transfer, MegaError *)
{
    if (transfer->getState() == MegaTransfer::STATE_COMPLETED || transfer->getState() == MegaTransfer::STATE_FAILED)
    {
        insertTransfer(transfer);
    }
}

void QFinishedTransfersModel::refreshTransferItem(int tag)
{
    int row = 0;
    for (transfer_it it = transferOrder.begin(); it != transferOrder.end() && (*it)->data.tag != tag; ++it)
    {
        ++row;
    }

    auto rowAsSizeT = static_cast<size_t>(row);
    assert(rowAsSizeT < transferOrder.size());
    if (rowAsSizeT >= transferOrder.size())
    {
        return;
    }

    emit dataChanged(index(row, 0, QModelIndex()), index(row, 0, QModelIndex()));
}

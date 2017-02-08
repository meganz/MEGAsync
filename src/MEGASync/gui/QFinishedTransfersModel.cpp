#include "QFinishedTransfersModel.h"
#include "MegaApplication.h"
#include <assert.h>

using namespace mega;

QFinishedTransfersModel::QFinishedTransfersModel(QList<MegaTransfer *> finishedTransfers, QObject *parent) :
    QTransfersModel(QTransfersModel::TYPE_FINISHED, parent)
{
    int numTransfers = finishedTransfers.size();
    if (numTransfers)
    {
        beginInsertRows(QModelIndex(), 0, numTransfers - 1);
        for (int i = 0; i < numTransfers; i++)
        {
            MegaTransfer *transfer = finishedTransfers.at(i);
            TransferItemData *item = new TransferItemData();
            item->tag = transfer->getTag();
            item->priority = transfer->getPriority();
            transfers.insert(item->tag, item);
            transferOrder.push_front(item);
        }
        endInsertRows();
    }
}

void QFinishedTransfersModel::insertTransfer(MegaTransfer *transfer)
{
    TransferItemData *item = new TransferItemData();
    item->tag = transfer->getTag();
    item->priority = transfer->getPriority();

    if (transfers.size() == Preferences::MAX_COMPLETED_ITEMS)
    {
        TransferItemData *t = transferOrder.back();
        int row = transferOrder.size() - 1;
        beginRemoveRows(QModelIndex(), row, row);
        transfers.remove(t->tag);
        transferOrder.pop_back();
        transferItems.remove(t->tag);
        endRemoveRows();
        delete t;
    }

    transfer_it it = transferOrder.begin();
    int row = 0;

    beginInsertRows(QModelIndex(), row, row);
    transfers.insert(item->tag, item);
    transferOrder.insert(it, item);
    endInsertRows();

    if (transferOrder.size() == 1)
    {
        emit onTransferAdded();
    }
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
    for (it = transferOrder.begin(); it != transferOrder.end() && (*it)->tag != transferTag; ++it)
    {
        ++row;
    }
    assert(row < transferOrder.size());

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
    }

    ((MegaApplication *)qApp)->removeAllFinishedTransfers();
    emit noTransfers();
}

MegaTransfer *QFinishedTransfersModel::getTransferByTag(int tag)
{
    return ((MegaApplication *)qApp)->getFinishedTransferByTag(tag);
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
    for (transfer_it it = transferOrder.begin(); it != transferOrder.end() && (*it)->tag != tag; ++it)
    {
        ++row;
    }

    assert(row < transferOrder.size());
    if (row >= transferOrder.size())
    {
        return;
    }

    emit dataChanged(index(row, 0, QModelIndex()), index(row, 0, QModelIndex()));
}

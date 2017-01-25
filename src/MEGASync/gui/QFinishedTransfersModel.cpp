#include "QFinishedTransfersModel.h"
#include "MegaApplication.h"
#include <assert.h>

using namespace mega;

QFinishedTransfersModel::QFinishedTransfersModel(QList<MegaTransfer *> transfers, QObject *parent) :
    QTransfersModel(QTransfersModel::TYPE_FINISHED, parent)
{
    for (int i = 0; i < transfers.size(); i++)
    {
        MegaTransfer *transfer = transfers.at(i);
        insertTransfer(transfer);
        updateTransferInfo(transfer);
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
        ((MegaApplication *)qApp)->removeFinishedTransfer(t->tag);
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
    beginRemoveRows(QModelIndex(), 0, transfers.size());
    qDeleteAll(transfers);
    transfers.clear();
    transferOrder.clear();
    transferItems.clear();
    endRemoveRows();

    ((MegaApplication *)qApp)->removeAllFinishedTransfers();

    emit noTransfers();
}

MegaTransfer *QFinishedTransfersModel::getTransferByTag(int tag)
{
    return ((MegaApplication *)qApp)->getFinishedTransferByTag(tag);
}

MegaTransfer* QFinishedTransfersModel::getFinishedTransferByTag(int tag)
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

void QFinishedTransfersModel::updateTransferInfo(MegaTransfer *transfer)
{
    TransferItemData *itemData = transfers.value(transfer->getTag());
    if (!itemData)
    {
        return;
    }

    unsigned long long newPriority = transfer->getPriority();
    TransferItem *item = transferItems[transfer->getTag()];
    if (item)
    {
        if (item->getType() < 0)
        {
            item->setType(transfer->getType(), transfer->isSyncTransfer());
            item->setFileName(QString::fromUtf8(transfer->getFileName()));
            item->setTotalSize(transfer->getTotalBytes());
        }

        item->setSpeed(transfer->getSpeed(), transfer->getMeanSpeed());
        item->setTransferredBytes(transfer->getTransferredBytes(), !transfer->isSyncTransfer());
        item->setTransferState(transfer->getState());
        item->setPriority(newPriority);

        int row = 0;
        int tag = item->getTransferTag();
        for (transfer_it it = transferOrder.begin(); it != transferOrder.end() && (*it)->tag != tag; ++it)
        {
            ++row;
        }

        assert(row < transferOrder.size());
        emit dataChanged(index(row, 0, QModelIndex()), index(row, 0, QModelIndex()));
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

#include "QCustomTransfersModel.h"
#include <assert.h>
#include "MegaApplication.h"

using namespace mega;

QCustomTransfersModel::QCustomTransfersModel(int type, QObject *parent)
    : QTransfersModel(type, parent)
{
    modelState = IDLE;
    activeUploadTag   = -1;
    activeDownloadTag = -1;
}

void QCustomTransfersModel::refreshTransfers()
{
    if (transferOrder.size())
    {
        emit dataChanged(index(0, 0, QModelIndex()), index(transferOrder.size() - 1, 0, QModelIndex()));
    }
}

MegaTransfer *QCustomTransfersModel::getTransferByTag(int tag)
{
    MegaTransfer *transfer = ((MegaApplication *)qApp)->getFinishedTransferByTag(tag);
    if (transfer)
    {
        return transfer;
    }

    return megaApi->getTransferByTag(tag);
}

void QCustomTransfersModel::onTransferStart(MegaApi *api, MegaTransfer *transfer)
{
    TransferItemData *ti =  transfers.value(transfer->getTag());
    if (ti)
    {
        return;
    }

    TransferItemData *item = new TransferItemData();
    item->tag = transfer->getTag();
    item->priority = transfer->getPriority();

    int row = 0;
    beginInsertRows(QModelIndex(), row, row);
    transfer_it it = getInsertPosition(transfer);
    transfers.insert(item->tag, item);
    transferOrder.insert(it, item);
    endInsertRows();

    // Update model state
    if (transfer->getType() == MegaTransfer::TYPE_DOWNLOAD)
    {
        modelState |= DOWNLOAD;
        activeDownloadTag = transfer->getTag();
    }
    else if (transfer->getType() == MegaTransfer::TYPE_UPLOAD)
    {
        modelState |= UPLOAD;
        activeUploadTag = transfer->getTag();
    }

    if (transferOrder.size() == 1)
    {
        emit onTransferAdded();
    }
}

void QCustomTransfersModel::onTransferFinish(MegaApi *api, MegaTransfer *transfer, MegaError *e)
{
    int actualTag = transfer->getTag();
    if (activeDownloadTag == actualTag && transfer->getType() == MegaTransfer::TYPE_DOWNLOAD)
    {
        modelState &= ~DOWNLOAD;
        activeDownloadTag = -1;
    }
    else if (activeUploadTag == actualTag && transfer->getType() == MegaTransfer::TYPE_UPLOAD)
    {
        modelState &= ~UPLOAD;
        activeUploadTag = -1;
    }

    removeTransferByTag(transfer->getTag());

    if (transfer->getState() == MegaTransfer::STATE_COMPLETED || transfer->getState() == MegaTransfer::STATE_FAILED)
    {
        TransferItemData *item = new TransferItemData();
        item->tag = transfer->getTag();
        item->priority = transfer->getPriority();

        //FIXME SET A THRESHOLD FOR MAXIMUM NUMBER OF COMPLETED TRANSFERS

        int row = 0;

        beginInsertRows(QModelIndex(), row, row);
        transfer_it it = getInsertPosition(transfer);
        transfers.insert(item->tag, item);
        transferOrder.insert(it, item);
        endInsertRows();
    }

    if (transfers.isEmpty())
    {
        emit noTransfers();
    }
}

void QCustomTransfersModel::onTransferUpdate(MegaApi *api, MegaTransfer *transfer)
{
    updateTransferInfo(transfer);
}

void QCustomTransfersModel::onTransferTemporaryError(MegaApi *api, MegaTransfer *transfer, MegaError *e)
{
    updateTransferInfo(transfer);
}

void QCustomTransfersModel::refreshTransferItem(int tag)
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

void QCustomTransfersModel::updateTransferInfo(MegaTransfer *transfer)
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
    }

    //Update modified item
    refreshTransferItem(transfer->getTag());
}

void QCustomTransfersModel::replaceWithTransfer(MegaTransfer *transfer)
{
    int transferToReplaced;
    int type = transfer->getType();
    if (type == MegaTransfer::TYPE_DOWNLOAD)
    {
        transferToReplaced = activeDownloadTag;
        activeDownloadTag = transfer->getTag();
    }
    else
    {
        transferToReplaced = activeUploadTag;
        activeUploadTag = transfer->getTag();
    }

    TransferItemData *item =  transfers.value(transferToReplaced);
    if (!item)
    {
        return;
    }

    int row = 0;
    transfer_it it;
    for (it = transferOrder.begin(); it != transferOrder.end() && (*it)->tag != transferToReplaced; ++it)
    {
        ++row;
    }
    assert(row < transferOrder.size());

    //Generate new element and place it
    TransferItemData *newItem = new TransferItemData();
    newItem->tag = transfer->getTag();
    newItem->priority = transfer->getPriority();

    beginRemoveRows(QModelIndex(), row, row);

    transfers.remove(transferToReplaced);
    transfers.insert(newItem->tag, newItem);
    transferItems.remove(transferToReplaced);
    transferOrder[it - transferOrder.begin()] = newItem;

    endRemoveRows();
    delete item;
}

void QCustomTransfersModel::removeAllTransfers()
{

}

void QCustomTransfersModel::removeTransferByTag(int transferTag)
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
//    ((MegaApplication *)qApp)->removeFinishedTransfer(transferTag);
    transferItems.remove(transferTag);
    endRemoveRows();
    delete item;
}

void QCustomTransfersModel::updateActiveTransfer(MegaApi *api, MegaTransfer *newtransfer)
{
    int type = newtransfer->getType();
    if (type == MegaTransfer::TYPE_DOWNLOAD)
    {
        if (activeDownloadTag == -1)
        {
            onTransferStart(api, newtransfer);
            modelState |= DOWNLOAD;
            activeDownloadTag = newtransfer->getTag();
        }
        else if (activeDownloadTag != newtransfer->getTag())
        {
            replaceWithTransfer(newtransfer);
        }
    }
    else
    {
        if (activeUploadTag == -1)
        {
            onTransferStart(api, newtransfer);
            modelState |= UPLOAD;
            activeUploadTag = newtransfer->getTag();
        }
        else if (activeUploadTag != newtransfer->getTag())
        {
            replaceWithTransfer(newtransfer);
        }
    }
}

transfer_it QCustomTransfersModel::getInsertPosition(MegaTransfer *transfer)
{
    transfer_it it = transferOrder.begin();

    switch (transfer->getState())
    {
        case MegaTransfer::STATE_COMPLETED:
        case MegaTransfer::STATE_FAILED:
            if (modelState & DOWNLOAD)
            {
                std::advance(it, 1);
            }

            if (modelState & UPLOAD)
            {
                std::advance(it, 1);
            }

            break;
        case MegaTransfer::STATE_CANCELLED:
        default:
            if (modelState & DOWNLOAD)
            {
                std::advance(it, 1);
            }
            break;
    }

    return it;
}

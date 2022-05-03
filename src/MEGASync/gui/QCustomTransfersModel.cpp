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
        emit dataChanged(index(0, 0, QModelIndex()), index(int(transferOrder.size()) - 1, 0, QModelIndex()));
    }
}

MegaTransfer *QCustomTransfersModel::getTransferByTag(int tag)
{
    MegaTransfer *transfer = ((MegaApplication *)qApp)->getFinishedTransferByTag(tag);
    if (transfer)
    {
        return transfer->copy();
    }

    return megaApi->getTransferByTag(tag);
}

void QCustomTransfersModel::onTransferStart(MegaApi*, MegaTransfer* transfer)
{
    TransferItemData *ti =  transfers.value(transfer->getTag());
    if (ti)
    {
        return;
    }

    TransferItemData *item = new TransferItemData(transfer);

    transfer_it it = getInsertPosition(transfer);
    const int row = static_cast<int>(it - transferOrder.begin());
    beginInsertRows(QModelIndex(), row, row);
    transfers.insert(item->data.tag, item);
    transferOrder.insert(it, item);

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
    endInsertRows();

    if (transferOrder.size() == 1)
    {
        emit onTransferAdded();
    }
}

void QCustomTransfersModel::onTransferFinish(MegaApi*, MegaTransfer* t, MegaError*)
{
    QPointer<QCustomTransfersModel> model = this;
    MegaTransfer *transfer = t->copy();

    removeTransferByTag(transfer->getTag());

    if (transfer->getState() == MegaTransfer::STATE_COMPLETED || transfer->getState() == MegaTransfer::STATE_FAILED)
    {       
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
                        TransferItemData *itemData = transferOrder.back();
                        int row = int(transferOrder.size()) - 1;
                        beginRemoveRows(QModelIndex(), row, row);
                        transfers.remove(itemData->data.tag);
                        transferOrder.pop_back();
                        transferItems.remove(itemData->data.tag);
                        endRemoveRows();
                        delete itemData;
                    }

                    transfer_it it = getInsertPosition(transfer);
                    const int row = static_cast<int>(it - transferOrder.begin());
                    beginInsertRows(QModelIndex(), row, row);
                    transfers.insert(item->data.tag, item);
                    transferOrder.insert(it, item);
                    endInsertRows();

                    if (transfers.isEmpty())
                    {
                        emit noTransfers();
                    }
                    else
                    {
                        if (transferOrder.size() == 1)
                        {
                            emit onTransferAdded();
                        }
                    }
                }

                delete transfer;

            });//end of queued function

        });// end of thread pool function;
    }
}

void QCustomTransfersModel::onTransferUpdate(MegaApi*, MegaTransfer* transfer)
{
    updateTransferInfo(transfer);
}

void QCustomTransfersModel::onTransferTemporaryError(MegaApi*, MegaTransfer* transfer, MegaError*)
{
    updateTransferInfo(transfer);
}

void QCustomTransfersModel::refreshTransferItem(int tag)
{
    int row = 0;
    for (transfer_it it = transferOrder.begin(); it != transferOrder.end() && (*it)->data.tag != tag; ++it)
    {
        ++row;
    }

    const int transferOrderSize = static_cast<int>(transferOrder.size());
    assert(row < transferOrderSize);
    if (row >= transferOrderSize)
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

        // Get http speed, which reports speed changes faster than the transfer.
        long long httpSpeed;
        if (item->getType() == MegaTransfer::TYPE_DOWNLOAD)
        {
            httpSpeed = static_cast<MegaApplication*>(qApp)->getMegaApi()->getCurrentDownloadSpeed();
        }
        else
        {
            httpSpeed = static_cast<MegaApplication*>(qApp)->getMegaApi()->getCurrentUploadSpeed();
        }

        item->setSpeed(std::min(transfer->getSpeed(), httpSpeed), transfer->getMeanSpeed());
        item->setTransferredBytes(transfer->getTransferredBytes(), !transfer->isSyncTransfer());
        item->setTransferState(transfer->getState());
        item->setPriority(newPriority);

        int tError = transfer->getLastError().getErrorCode();
        if (tError != MegaError::API_OK)
        {
            assert(transfer->getLastErrorExtended());
            item->setTransferError(tError, transfer->getLastErrorExtended()->getValue());
        }
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

    int row = 0;
    transfer_it it;
    for (it = transferOrder.begin(); it != transferOrder.end() && (*it)->data.tag != transferToReplaced; ++it)
    {
        ++row;
    }

    TransferItemData *item =  transfers.value(transferToReplaced);
    assert(item && row < static_cast<int>(transferOrder.size()));

    //Generate new element and place it
    TransferItemData *newItem = new TransferItemData(transfer);

    beginResetModel();
    transfers.insert(newItem->data.tag, newItem);
    transferOrder[row] = newItem;
    transfers.remove(transferToReplaced);
    transferItems.remove(transferToReplaced);
    endResetModel();
    delete item;
}

void QCustomTransfersModel::removeAllTransfers()
{
    removeAllCompletedTransfers();
}

void QCustomTransfersModel::removeAllCompletedTransfers()
{
    if (transfers.size())
    {
        int initialDelPos = 0;
        if (activeDownloadTag > -1)
        {
            initialDelPos++;
        }

        if (activeUploadTag > -1)
        {
            initialDelPos++;
        }

        if (initialDelPos <= (transfers.size() - 1))
        {
            beginRemoveRows(QModelIndex(), initialDelPos, transfers.size() - 1);
            for (QMap<int, TransferItemData*>::iterator it = transfers.begin(); it != transfers.end();)
            {
                int tag = it.key();
                if (tag != activeDownloadTag && tag != activeUploadTag)
                {
                    TransferItemData *item = it.value();
                    transferItems.remove(tag);
                    it = transfers.erase(it);
                    delete item;
                }
                else
                {
                    it++;
                }
            }
            transferOrder.erase(transferOrder.begin() + initialDelPos, transferOrder.end());
            endRemoveRows();
        }
    }

    if (transfers.isEmpty())
    {
        emit noTransfers();
    }
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
    for (it = transferOrder.begin(); it != transferOrder.end() && (*it)->data.tag != transferTag; ++it)
    {
        ++row;
    }
    assert(row < static_cast<int>(transferOrder.size()));

    beginRemoveRows(QModelIndex(), row, row);
    if (activeDownloadTag == transferTag)
    {
        modelState &= static_cast<unsigned char>(~DOWNLOAD);
        activeDownloadTag = -1;
    }
    else if (activeUploadTag == transferTag)
    {
        modelState &= static_cast<unsigned char>(~UPLOAD);
        activeUploadTag = -1;
    }

    transfers.remove(transferTag);
    transferOrder.erase(it);
    transferItems.remove(transferTag);
    endRemoveRows();
    delete item;

    if (transfers.isEmpty())
    {
        emit noTransfers();
    }
}

void QCustomTransfersModel::updateActiveTransfer(MegaApi *api, MegaTransfer *newtransfer)
{
    if (newtransfer->isFinished())
    {
        return;
    }

    int type = newtransfer->getType();
    if (type == MegaTransfer::TYPE_DOWNLOAD)
    {
        if (activeDownloadTag == -1)
        {
            onTransferStart(api, newtransfer);
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
    if (transfer->isFinished())
    {
        if (modelState & DOWNLOAD)
        {
            std::advance(it, 1);
        }

        if (modelState & UPLOAD)
        {
            std::advance(it, 1);
        }
    }
    else
    {
        if (modelState & DOWNLOAD)
        {
            std::advance(it, 1);
        }
    }
    return it;
}

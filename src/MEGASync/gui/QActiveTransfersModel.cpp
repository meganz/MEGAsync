#include "QActiveTransfersModel.h"
#include "MegaApplication.h"

#include "control/AppStatsEvents.h"

#include <assert.h>


using namespace mega;

bool priority_comparator(TransferItemData* i, TransferItemData *j)
{
    if (i->data.priority < j->data.priority)
        return true;

    if (i->data.priority > j->data.priority)
        return false;

    return i->data.tag < j->data.tag;
}

QActiveTransfersModel::QActiveTransfersModel(int type, std::shared_ptr<MegaTransferData> transferData, QObject *parent) :
    QTransfersModel(type, parent)
{
    QPointer<QActiveTransfersModel> model = this;

    if (!transferData)
    {
        return;
    }

    if (type == TYPE_DOWNLOAD)
    {
        int numDownloads = transferData->getNumDownloads();
        if (numDownloads)
        {
            mThreadPool->push([this, model, transferData, numDownloads]()
            {//thread pool function
                for (int i = 0; i < numDownloads; i++)
                {
                    if (!model)
                    {
                        return;
                    }

                    MegaTransfer *nextTransfer = nullptr;
                    nextTransfer = ((MegaApplication *)qApp)->getMegaApi()->getTransferByTag(transferData->getDownloadTag(i));
                    if (nextTransfer)
                    {
                        Utilities::queueFunctionInAppThread([this, model, nextTransfer]()
                        {//queued function

                            if (model)
                            {
                                onTransferStart(nullptr, nextTransfer);
                            }

                            delete nextTransfer;

                        });//end of queued function
                    }
                }
            });// end of thread pool function;

        }
    }
    else if (type == TYPE_UPLOAD)
    {
        int numUploads = transferData->getNumUploads();
        if (numUploads)
        {
            mThreadPool->push([this, model, transferData, numUploads]()
            {//thread pool function
                for (int i = 0; i < numUploads; i++)
                {
                    if (!model)
                    {
                        return;
                    }

                    MegaTransfer *nextTransfer = nullptr;
                    nextTransfer = ((MegaApplication *)qApp)->getMegaApi()->getTransferByTag(transferData->getUploadTag(i));
                    if (nextTransfer)
                    {
                        Utilities::queueFunctionInAppThread([this, model, nextTransfer]()
                        {//queued function

                            if (model)
                            {
                                onTransferStart(nullptr, nextTransfer);
                            }

                            delete nextTransfer;

                        });//end of queued function
                    }
                }
            });// end of thread pool function;

        }
    }

    if (transferOrder.size() != static_cast<size_t>(transfers.size()))
    {
        assert(false);
        megaApi->sendEvent(AppStatsEvents::EVENT_DUP_ACTIVE_TRSF_DURING_INIT,
                           "Duplicated active transfer during initialization");
    }
}

void QActiveTransfersModel::removeTransferByTag(int transferTag)
{
    TransferItemData *item =  transfers.value(transferTag);
    if (!item)
    {
        return;
    }

    transfer_it it = std::lower_bound(transferOrder.begin(), transferOrder.end(), item, priority_comparator);
    const int row = static_cast<int>(std::distance(transferOrder.begin(), it));
    assert(it != transferOrder.end() && (*it)->data.tag == transferTag);
    if (it == transferOrder.end() || (*it)->data.tag != transferTag)
    {
        return;
    }

    beginRemoveRows(QModelIndex(), row, row);
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

void QActiveTransfersModel::removeAllTransfers()
{
}

QMimeData *QActiveTransfersModel::mimeData(const QModelIndexList &indexes) const
{
    QByteArray byteArray;
    QDataStream stream(&byteArray, QIODevice::WriteOnly);
    QList<quintptr> selectedTags;
    for (int i = 0; i < indexes.size(); i++)
    {
        selectedTags.append(indexes.at(i).internalId());
    }
    stream << selectedTags;

    QMimeData *data = new QMimeData();
    data->setData(QString::fromUtf8("application/x-qabstractitemmodeldatalist"), byteArray);
    return data;
}

Qt::ItemFlags QActiveTransfersModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);
    if (index.isValid())
    {
        return Qt::ItemIsDragEnabled | defaultFlags;
    }
    return Qt::ItemIsDropEnabled | defaultFlags;
}

Qt::DropActions QActiveTransfersModel::supportedDropActions() const
{
    return Qt::MoveAction;
}

bool QActiveTransfersModel::dropMimeData(const QMimeData *data, Qt::DropAction, int row, int, const QModelIndex &)
{
    QByteArray byteArray = data->data(QString::fromUtf8("application/x-qabstractitemmodeldatalist"));
    QDataStream stream(&byteArray, QIODevice::ReadWrite);
    QList<quintptr> selectedTags;
    stream >> selectedTags;

    const int transferOrderSize = static_cast<int>(transferOrder.size());
    if (row < 0 || row > transferOrderSize || !selectedTags.size())
    {
        return false;
    }

    TransferItemData *item = NULL;
    if (row != transferOrderSize)
    {
        item = transferOrder[row];
        const int firstSelectedTag = static_cast<int>(selectedTags[0]);
        if (item->data.tag == firstSelectedTag)
        {
            return false;
        }

        TransferItemData* itemData = transfers.value(firstSelectedTag);
        if (!itemData)
        {
            return false;
        }

        std::deque<TransferItemData*>::iterator srcit = std::lower_bound(transferOrder.begin(), transferOrder.end(), itemData, priority_comparator);
        const int srcrow = static_cast<int>(std::distance(transferOrder.begin(), srcit));
        if (srcrow + selectedTags.size() == row)
        {
            return false;
        }
    }

    for (int i = 0; i< selectedTags.size(); i++)
    {
        const int currentTag = static_cast<int>(selectedTags[i]);
        if (item)
        {
            megaApi->moveTransferBeforeByTag(currentTag, item->data.tag);
        }
        else
        {
            megaApi->moveTransferToLastByTag(currentTag);
        }
    }
    return true;
}

MegaTransfer *QActiveTransfersModel::getTransferByTag(int tag)
{
    return megaApi->getTransferByTag(tag);
}

void QActiveTransfersModel::onTransferStart(MegaApi *, MegaTransfer *transfer)
{
    if (transfer->getType() == type)
    {
        TransferItemData *item = new TransferItemData(transfer);

        transfer_it it = std::lower_bound(transferOrder.begin(), transferOrder.end(), item, priority_comparator);
        const int row = static_cast<int>(std::distance(transferOrder.begin(), it));
        if (transfers.count(item->data.tag))
        {
            assert(false);
            megaApi->sendEvent(AppStatsEvents::EVENT_DUP_ACTIVE_TRSF_DURING_INSERT,
                               QString::fromUtf8("Duplicated active transfer during insertion: %1")
                               .arg(QString::number(item->data.tag)).toUtf8().constData());
            delete item;
            return;
        }

        beginInsertRows(QModelIndex(), row, row);
        transfers.insert(item->data.tag, item);
        transferOrder.insert(it, item);
        endInsertRows();

        if (transferOrder.size() == 1)
        {
            emit onTransferAdded();
        }
    }
}

void QActiveTransfersModel::onTransferFinish(MegaApi *, MegaTransfer *transfer, MegaError *)
{
    if (transfer->getType() == type)
    {
        removeTransferByTag(transfer->getTag());
    }
}

void QActiveTransfersModel::onTransferUpdate(MegaApi *, MegaTransfer *transfer)
{
    if (transfer->getType() == type)
    {
        updateTransferInfo(transfer);
    }
}

void QActiveTransfersModel::onTransferTemporaryError(MegaApi *, MegaTransfer *transfer, MegaError *)
{
    if (transfer->getType() == type)
    {
        updateTransferInfo(transfer);
    }
}

void QActiveTransfersModel::updateTransferInfo(MegaTransfer *transfer)
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
    else
    {
        //Update values in case itemdata is not created yet by delegate
        itemData->data.speed = transfer->getSpeed();
        itemData->data.meanSpeed = transfer->getMeanSpeed();
        itemData->data.transferredBytes = transfer->getTransferredBytes();
    }

    if (newPriority == itemData->data.priority)
    {
        //Update modified item
        std::deque<TransferItemData*>::iterator it = std::lower_bound(transferOrder.begin(), transferOrder.end(), itemData, priority_comparator);
        const int row = static_cast<int>(std::distance(transferOrder.begin(), it));
        assert(it != transferOrder.end() && (*it)->data.tag == itemData->data.tag);
        if (row >= static_cast<int>(transferOrder.size()))
        {
            return;
        }
        emit dataChanged(index(row, 0, QModelIndex()), index(row, 0, QModelIndex()));
    }
    else
    {
        //Move item to its new position
        std::deque<TransferItemData*>::iterator it = std::lower_bound(transferOrder.begin(), transferOrder.end(), itemData, priority_comparator);
        const int row = static_cast<int>(std::distance(transferOrder.begin(), it));
        assert(it != transferOrder.end() && (*it)->data.tag == itemData->data.tag);
        if (it == transferOrder.end() || (*it)->data.tag != itemData->data.tag)
        {
            return;
        }

        TransferItemData testItem;
        testItem.data.tag = itemData->data.tag;
        testItem.data.priority = newPriority;
        std::deque<TransferItemData*>::iterator newit = std::lower_bound(transferOrder.begin(), transferOrder.end(), &testItem, priority_comparator);
        const int newrow = static_cast<int>(std::distance(transferOrder.begin(), newit));

        if (row == newrow || (row + 1) == newrow)
        {
            //Priorities are being adjusted, but there isn't an actual move operation
            itemData->data.priority = newPriority;
            emit dataChanged(index(row, 0, QModelIndex()), index(row, 0, QModelIndex()));
        }
        else
        {
            beginMoveRows(QModelIndex(), row, row, QModelIndex(), newrow);
            transferOrder.erase(it);
            itemData->data.priority = newPriority;
            std::deque<TransferItemData*>::iterator finalit = std::lower_bound(transferOrder.begin(), transferOrder.end(), itemData, priority_comparator);
            transferOrder.insert(finalit, itemData);
            endMoveRows();
        }
    }
}

void QActiveTransfersModel::refreshTransferItem(int tag)
{
    TransferItemData *itemData = transfers.value(tag);
    if (!itemData)
    {
        return;
    }

    std::deque<TransferItemData*>::iterator it = std::lower_bound(transferOrder.begin(), transferOrder.end(), itemData, priority_comparator);
    const int row = static_cast<int>(std::distance(transferOrder.begin(), it));
    assert(it != transferOrder.end() && (*it)->data.tag == itemData->data.tag);
    if (row >= static_cast<int>(transferOrder.size()))
    {
        return;
    }

    emit dataChanged(index(row, 0, QModelIndex()), index(row, 0, QModelIndex()));
}

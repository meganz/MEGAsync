#include "QActiveTransfersModel.h"
#include "MegaApplication.h"
#include <assert.h>

using namespace mega;

bool priority_comparator(TransferItemData* i, TransferItemData *j)
{
    if (i->priority < j->priority)
        return true;

    if (i->priority > j->priority)
        return false;

    return i->tag < j->tag;
}

QActiveTransfersModel::QActiveTransfersModel(int type, MegaTransferData *transferData, QObject *parent) :
    QTransfersModel(type, parent)
{
    if (!transferData)
    {
        return;
    }

    TransferItemData *itemData = NULL;
    if (type == TYPE_DOWNLOAD)
    {
        int numDownloads = transferData->getNumDownloads();
        if (numDownloads)
        {
            beginInsertRows(QModelIndex(), 0, numDownloads - 1);
            transferOrder.resize(numDownloads);
            for (int i = 0; i < numDownloads; i++)
            {
                itemData = new TransferItemData();
                itemData->tag = transferData->getDownloadTag(i);
                itemData->priority = transferData->getDownloadPriority(i);
                transfers.insert(itemData->tag, itemData);
                transferOrder[i] = itemData;
            }
            endInsertRows();
        }
    }
    else if (type == TYPE_UPLOAD)
    {
        int numUploads = transferData->getNumUploads();
        if (numUploads)
        {
            beginInsertRows(QModelIndex(), 0, numUploads - 1);
            transferOrder.resize(numUploads);
            for (int i = 0; i < numUploads; i++)
            {
                itemData = new TransferItemData();
                itemData->tag = transferData->getUploadTag(i);
                itemData->priority = transferData->getUploadPriority(i);
                transfers.insert(itemData->tag, itemData);
                transferOrder[i] = itemData;
            }
            endInsertRows();
        }
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
    int row = std::distance(transferOrder.begin(), it);
    assert(it != transferOrder.end() && (*it)->tag == transferTag);
    if (it == transferOrder.end() || (*it)->tag != transferTag)
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
    beginRemoveRows(QModelIndex(), 0, transfers.size());
    qDeleteAll(transfers);
    transfers.clear();
    transferOrder.clear();
    transferItems.clear();
    endRemoveRows();

    emit noTransfers();
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

    if (row < 0 || row > transferOrder.size() || !selectedTags.size())
    {
        return false;
    }

    TransferItemData *item = NULL;
    if (row != transferOrder.size())
    {
        item = transferOrder[row];
        if (item->tag == selectedTags[0])
        {
            return false;
        }

        TransferItemData* itemData = transfers.value(selectedTags[0]);
        if (!itemData)
        {
            return false;
        }

        std::deque<TransferItemData*>::iterator srcit = std::lower_bound(transferOrder.begin(), transferOrder.end(), itemData, priority_comparator);
        int srcrow = std::distance(transferOrder.begin(), srcit);
        if (srcrow + selectedTags.size() == row)
        {
            return false;
        }
    }

    for (int i = 0; i< selectedTags.size(); i++)
    {
        if (item)
        {
            megaApi->moveTransferBeforeByTag(selectedTags[i], item->tag);
        }
        else
        {
            megaApi->moveTransferToLastByTag(selectedTags[i]);
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
        TransferItemData *item = new TransferItemData();
        item->tag = transfer->getTag();
        item->priority = transfer->getPriority();

        transfer_it it = std::lower_bound(transferOrder.begin(), transferOrder.end(), item, priority_comparator);
        int row = std::distance(transferOrder.begin(), it);
        beginInsertRows(QModelIndex(), row, row);
        transfers.insert(item->tag, item);
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

        item->setSpeed(transfer->getSpeed(), transfer->getMeanSpeed());
        item->setTransferredBytes(transfer->getTransferredBytes(), !transfer->isSyncTransfer());
        item->setTransferState(transfer->getState());
        item->setPriority(newPriority);
    }

    if (newPriority == itemData->priority)
    {
        //Update modified item
        std::deque<TransferItemData*>::iterator it = std::lower_bound(transferOrder.begin(), transferOrder.end(), itemData, priority_comparator);
        int row = std::distance(transferOrder.begin(), it);
        assert(it != transferOrder.end() && (*it)->tag == itemData->tag);
        if (row >= transferOrder.size())
        {
            return;
        }
        emit dataChanged(index(row, 0, QModelIndex()), index(row, 0, QModelIndex()));
    }
    else
    {
        //Move item to its new position
        std::deque<TransferItemData*>::iterator it = std::lower_bound(transferOrder.begin(), transferOrder.end(), itemData, priority_comparator);
        int row = std::distance(transferOrder.begin(), it);
        assert(it != transferOrder.end() && (*it)->tag == itemData->tag);
        if (it == transferOrder.end() || (*it)->tag != itemData->tag)
        {
            return;
        }

        TransferItemData testItem;
        testItem.priority = newPriority;
        std::deque<TransferItemData*>::iterator newit = std::lower_bound(transferOrder.begin(), transferOrder.end(), &testItem, priority_comparator);
        int newrow = std::distance(transferOrder.begin(), newit);

        if (row == newrow || (row + 1) == newrow)
        {
            //Priorities are being adjusted, but there isn't an actual move operation
            itemData->priority = newPriority;
            emit dataChanged(index(row, 0, QModelIndex()), index(row, 0, QModelIndex()));
        }
        else
        {
            beginMoveRows(QModelIndex(), row, row, QModelIndex(), newrow);
            transferOrder.erase(it);
            itemData->priority = newPriority;
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
    int row = std::distance(transferOrder.begin(), it);
    assert(it != transferOrder.end() && (*it)->tag == itemData->tag);
    if (row >= transferOrder.size())
    {
        return;
    }

    emit dataChanged(index(row, 0, QModelIndex()), index(row, 0, QModelIndex()));
}

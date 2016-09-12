#include "QTransfersModel.h"
#include "MegaApplication.h"

using namespace mega;

bool priority_comparator(TransferItemData* i, TransferItemData *j)
{
    if (i->priority < j->priority)
        return true;

    if (i->priority > j->priority)
        return false;

    return i->tag < j->tag;
}

QTransfersModel::QTransfersModel(int type, QObject *parent) :
    QAbstractItemModel(parent)
{
    this->type = type;
    this->megaApi = ((MegaApplication *)qApp)->getMegaApi();
    this->transferItems.setMaxCost(16);
}

int QTransfersModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}

QVariant QTransfersModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || (index.row() < 0 || transfers.count() <= index.row()))
    {
        return QVariant();
    }

    if (role == Qt::DisplayRole)
    {
        return QVariant::fromValue(index.internalId());
    }

    return QVariant();
}

QModelIndex QTransfersModel::parent(const QModelIndex &) const
{
    return QModelIndex();
}

QModelIndex QTransfersModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
    {
        return QModelIndex();
    }

    return createIndex(row, column, transferOrder[row]->tag);
}

void QTransfersModel::insertTransfer(MegaTransfer *transfer)
{
    TransferItemData *item = new TransferItemData();
    item->tag = transfer->getTag();
    item->priority = transfer->getPriority();

    auto it = std::lower_bound(transferOrder.begin(), transferOrder.end(), item, priority_comparator);
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

void QTransfersModel::removeTransfer(MegaTransfer *transfer)
{
    removeTransferByTag(transfer->getTag());
}

void QTransfersModel::removeTransferByTag(int transferTag)
{
    TransferItemData *item =  transfers.value(transferTag);
    if (!item)
    {
        return;
    }

    auto it = std::lower_bound(transferOrder.begin(), transferOrder.end(), item, priority_comparator);
    int row = std::distance(transferOrder.begin(), it);

    beginRemoveRows(QModelIndex(), row, row);
    transfers.remove(transferTag);
    transferOrder.erase(it);
    endRemoveRows();
    delete item;

    if (type == TYPE_FINISHED)
    {
        ((MegaApplication *)qApp)->removeFinishedTransfer(transferTag);
    }

    if (transfers.isEmpty())
    {
        emit noTransfers();
    }
}

void QTransfersModel::removeAllTransfers()
{
    beginRemoveRows(QModelIndex(), 0, transfers.size());
    qDeleteAll(transfers);
    transfers.clear();
    transferOrder.clear();
    endRemoveRows();

    if (type == TYPE_FINISHED)
    {
        ((MegaApplication *)qApp)->removeAllFinishedTransfers();
    }

    emit noTransfers();
}

int QTransfersModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return transferOrder.size();
}

QMimeData *QTransfersModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData *data = new QMimeData();
    QModelIndex index = indexes.at(0);
    data->setData(QString::fromUtf8("application/x-qabstractitemmodeldatalist"), QString::number(index.internalId()).toUtf8());
    return data;
}

Qt::ItemFlags QTransfersModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);
    if (type == TYPE_ALL || type == TYPE_FINISHED)
    {
        return defaultFlags;
    }

    if (index.isValid())
    {
        return Qt::ItemIsDragEnabled | defaultFlags;
    }
    return Qt::ItemIsDropEnabled | defaultFlags;
}

Qt::DropActions QTransfersModel::supportedDropActions() const
{
    return Qt::MoveAction;
}

bool QTransfersModel::dropMimeData(const QMimeData *data, Qt::DropAction, int row, int, const QModelIndex &)
{
    TransferItemData *item = NULL;
    int transferTag = QString::fromUtf8(data->data(QString::fromUtf8("application/x-qabstractitemmodeldatalist"))).toInt();
    if (row >= 0 && row < (int)transferOrder.size())
    {
        TransferItemData *srcItem = transfers.value(transferTag);
        if (!srcItem)
        {
            return false;
        }

        auto srcit = std::lower_bound(transferOrder.begin(), transferOrder.end(), srcItem, priority_comparator);
        int srcrow = std::distance(transferOrder.begin(), srcit);
        if (row == srcrow || row == (srcrow + 1))
        {
            return false;
        }

        if (row < srcrow)
        {
            item = transferOrder[row];
            megaApi->moveTransferBeforeByTag(transferTag, item->tag);
        }
        else
        {
            item = transferOrder[row - 1];
            megaApi->moveTransferBeforeByTag(transferTag, item->tag);
        }
    }
    else
    {
        megaApi->moveTransferToLastByTag(transferTag);
    }
    return true;
}

QTransfersModel::~QTransfersModel()
{
    qDeleteAll(transfers);
}

int QTransfersModel::getModelType()
{
    return type;
}

MegaTransfer* QTransfersModel::getFinishedTransferByTag(int tag)
{
    return ((MegaApplication *)qApp)->getFinishedTransferByTag(tag);
}

void QTransfersModel::onTransferPaused(int transferTag, bool pause)
{
    megaApi->pauseTransferByTag(transferTag, pause);
}

void QTransfersModel::onTransferCancel(int transferTag)
{
    megaApi->cancelTransferByTag(transferTag);
}

void QTransfersModel::onMoveTransferToFirst(int transferTag)
{
    megaApi->moveTransferToFirstByTag(transferTag);
}

void QTransfersModel::onMoveTransferUp(int transferTag)
{
    megaApi->moveTransferUpByTag(transferTag);
}

void QTransfersModel::onMoveTransferDown(int transferTag)
{
    megaApi->moveTransferDownByTag(transferTag);
}

void QTransfersModel::onMoveTransferToLast(int transferTag)
{
    megaApi->moveTransferToLastByTag(transferTag);
}

void QTransfersModel::onTransferStart(MegaApi *, MegaTransfer *transfer)
{
    if (type == TYPE_ALL || transfer->getType() == type)
    {
        insertTransfer(transfer);
    }
}

void QTransfersModel::onTransferFinish(MegaApi *, MegaTransfer *transfer, MegaError *)
{
    if (type == TYPE_ALL || transfer->getType() == type)
    {
        removeTransfer(transfer);
    }
    else if (type == TYPE_FINISHED && (transfer->getState() == MegaTransfer::STATE_COMPLETED || transfer->getState() == MegaTransfer::STATE_FAILED))
    {
        insertTransfer(transfer);
    }
}

void QTransfersModel::onTransferUpdate(MegaApi *, MegaTransfer *transfer)
{
    if (type == TYPE_ALL || transfer->getType() == type)
    {
        updateTransferInfo(transfer);
    }
}

void QTransfersModel::onTransferTemporaryError(MegaApi *, MegaTransfer *transfer, MegaError *e)
{

}

void QTransfersModel::setupModelTransfers(QList<MegaTransfer*> tList)
{
    if (tList.isEmpty())
    {
        return;
    }

    for (int i = 0; i < tList.size(); i++)
    {
        insertTransfer(tList.at(i));
        updateTransferInfo(tList.at(i));
    }
 }
void QTransfersModel::setupModelTransfers(MegaTransferData *transferData)
{
    if (!transferData)
    {
        return;
    }

    TransferItemData *itemData = NULL;
    if (type == TYPE_DOWNLOAD)
    {
        int numDownloads = transferData->getNumDownloads();
        transferOrder.resize(numDownloads);
        for (int i = 0; i < numDownloads; i++)
        {
            itemData = new TransferItemData();
            itemData->tag = transferData->getDownloadTag(i);
            itemData->priority = transferData->getDownloadPriority(i);
            transfers.insert(itemData->tag, itemData);
            transferOrder[i] = itemData;
        }
    }
    else if (type == TYPE_UPLOAD)
    {
        int numUploads = transferData->getNumUploads();
        transferOrder.resize(numUploads);
        for (int i = 0; i < numUploads; i++)
        {
            itemData = new TransferItemData();
            itemData->tag = transferData->getUploadTag(i);
            itemData->priority = transferData->getUploadPriority(i);
            transfers.insert(itemData->tag, itemData);
            transferOrder[i] = itemData;
        }
    }
    else if (type == TYPE_ALL)
    {
        int numDownloads = transferData->getNumDownloads();
        transferOrder.resize(numDownloads);
        for (int i = 0; i < numDownloads; i++)
        {
            itemData = new TransferItemData();
            itemData->tag = transferData->getDownloadTag(i);
            itemData->priority = transferData->getDownloadPriority(i);
            transfers.insert(itemData->tag, itemData);
            transferOrder[i] = itemData;
        }

        int numUploads = transferData->getNumUploads();
        for (int i = 0; i < numUploads; i++)
        {
            itemData = new TransferItemData();
            itemData->tag = transferData->getUploadTag(i);
            itemData->priority = transferData->getUploadPriority(i);
            transfers.insert(itemData->tag, itemData);
            auto it = std::lower_bound(transferOrder.begin(), transferOrder.end(), itemData, priority_comparator);
            transferOrder.insert(it, itemData);
        }
    }
}

void QTransfersModel::updateTransferInfo(MegaTransfer *transfer)
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

        item->setSpeed(transfer->getSpeed());
        item->setTransferredBytes(transfer->getTransferredBytes(), !transfer->isSyncTransfer());
        item->setTransferState(transfer->getState());
        item->setPriority(newPriority);
    }

    if (newPriority == itemData->priority)
    {
        //Update modified item
        auto it = std::lower_bound(transferOrder.begin(), transferOrder.end(), itemData, priority_comparator);
        int row = std::distance(transferOrder.begin(), it);
        emit dataChanged(index(row, 0, QModelIndex()), index(row, 0, QModelIndex()));
    }
    else
    {
        //Move item to its new position
        auto it = std::lower_bound(transferOrder.begin(), transferOrder.end(), itemData, priority_comparator);
        int row = std::distance(transferOrder.begin(), it);

        TransferItemData testItem;
        testItem.priority = newPriority;
        auto newit = std::lower_bound(transferOrder.begin(), transferOrder.end(), &testItem, priority_comparator);
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
            auto finalit = std::lower_bound(transferOrder.begin(), transferOrder.end(), itemData, priority_comparator);
            transferOrder.insert(finalit, itemData);
            endMoveRows();
        }
    }
}

void QTransfersModel::animationChanged(int tag)
{
    TransferItemData *itemData = transfers.value(tag);
    if (!itemData)
    {
        return;
    }

    auto it = std::lower_bound(transferOrder.begin(), transferOrder.end(), itemData, priority_comparator);
    int row = std::distance(transferOrder.begin(), it);
    emit dataChanged(index(row, 0, QModelIndex()), index(row, 0, QModelIndex()));
}

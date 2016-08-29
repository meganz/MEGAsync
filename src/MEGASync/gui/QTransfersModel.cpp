#include "QTransfersModel.h"
#include "MegaApplication.h"

using namespace mega;

bool priority_comparator(TransferItem* i, TransferItem *j)
{
    return (i->getPriority() < j->getPriority());
}

QTransfersModel::QTransfersModel(int type, QObject *parent) :
    QAbstractItemModel(parent)
{
    this->type = type;

    MegaApi *api =  ((MegaApplication *)qApp)->getMegaApi();
    delegateListener = new QTMegaTransferListener(api, this);
    api->addTransferListener(delegateListener);
}

TransferItem *QTransfersModel::transferFromIndex(const QModelIndex &index) const
{
    if (index.isValid())
    {
        return static_cast<TransferItem *>(index.internalPointer());
    }
    else
    {
        return NULL;
    }
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
        return QVariant::fromValue(static_cast<TransferItem *>(index.internalPointer()));
    }

    if (role == TransferStatusRole)
    {
        return QVariant::fromValue(static_cast<TransferItem *>(index.internalPointer())->getTransferState());
    }

    if (role == TagRole)
    {
        return QVariant::fromValue(static_cast<TransferItem *>(index.internalPointer())->getTransferTag());
    }

    if (role == IsRegularTransferRole)
    {
        return QVariant::fromValue(static_cast<TransferItem *>(index.internalPointer())->getRegular());
    }

    return QVariant();
}

QModelIndex QTransfersModel::parent(const QModelIndex &index) const
{
    return QModelIndex();
}

QModelIndex QTransfersModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
    {
        return QModelIndex();
    }

    return createIndex(row, column, transferOrder[row]);
}

void QTransfersModel::insertTransfer(MegaTransfer *transfer)
{
    TransferItem *item = new TransferItem();
    item->setPriority(transfer->getPriority());

    auto it = std::lower_bound(transferOrder.begin(), transferOrder.end(), item, priority_comparator);
    int row = std::distance(transferOrder.begin(), it);

    beginInsertRows(QModelIndex(), row, row);
    transfers.insert(transfer->getTag(), item);
    transferOrder.insert(it, item);
    endInsertRows();

    updateInitialTransferInfo(transfer);
    updateTransferInfo(transfer);

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
    TransferItem *item =  transfers.value(transferTag);
    auto it = std::lower_bound(transferOrder.begin(), transferOrder.end(), item, priority_comparator);
    int row = std::distance(transferOrder.begin(), it);

    beginRemoveRows(QModelIndex(), row, row);
    transfers.remove(transferTag);
    transferOrder.erase(it);
    endRemoveRows();

    if (transfers.isEmpty())
    {
        emit noTransfers(type);
    }
}

void QTransfersModel::removeAllTransfers()
{
    beginRemoveRows(QModelIndex(), 0, transfers.size());
    transfers.clear();
    transferOrder.clear();
    endRemoveRows();

    if (transfers.isEmpty())
    {
        emit noTransfers(type);
    }
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
    TransferItem *item = transferFromIndex(index);
    if (item)
    {
        data->setData(QString::fromUtf8("application/x-qabstractitemmodeldatalist"), QString::number(item->getTransferTag()).toUtf8());
    }
    return data;
}

Qt::ItemFlags QTransfersModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);
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
    TransferItem *item = NULL;
    int transferTag = QString::fromUtf8(data->data(QString::fromUtf8("application/x-qabstractitemmodeldatalist"))).toInt();
    if (row >= 0 && row < (int)transferOrder.size())
    {
        item = transferOrder[row];
        ((MegaApplication *)qApp)->getMegaApi()->moveTransferBeforeByTag(transferTag, item->getTransferTag());
    }
    else
    {
        ((MegaApplication *)qApp)->getMegaApi()->moveTransferToLastByTag(transferTag);
    }
    return true;
}

QTransfersModel::~QTransfersModel()
{
    qDeleteAll(transfers);
    delete delegateListener;
}

int QTransfersModel::getModelType()
{
    return type;
}

void QTransfersModel::onTransferPaused(int transferTag, bool pause)
{
    ((MegaApplication *)qApp)->getMegaApi()->pauseTransferByTag(transferTag, pause);
}

void QTransfersModel::onTransferCancel(int transferTag)
{
    ((MegaApplication *)qApp)->getMegaApi()->cancelTransferByTag(transferTag);
}

void QTransfersModel::onMoveTransferToFirst(int transferTag)
{
    ((MegaApplication *)qApp)->getMegaApi()->moveTransferToFirstByTag(transferTag);
}

void QTransfersModel::onMoveTransferUp(int transferTag)
{
    ((MegaApplication *)qApp)->getMegaApi()->moveTransferUpByTag(transferTag);
}

void QTransfersModel::onMoveTransferDown(int transferTag)
{
    ((MegaApplication *)qApp)->getMegaApi()->moveTransferDownByTag(transferTag);
}

void QTransfersModel::onMoveTransferToLast(int transferTag)
{
    ((MegaApplication *)qApp)->getMegaApi()->moveTransferToLastByTag(transferTag);
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
        if (transfers.contains(transfer->getTag()))
        {
            TransferItem *item = transfers.value(transfer->getTag());
            item->finishTransfer();
            removeTransfer(transfer);
        }
    }
    else if (type == TYPE_FINISHED && transfer->getState() == MegaTransfer::STATE_COMPLETED)
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

void QTransfersModel::setupModelTransfers(MegaTransferList *transfers)
{
    if (!transfers)
    {
        return;
    }

    for (int i = 0; i < transfers->size(); i++)
    {
        MegaTransfer *t = transfers->get(i);
        insertTransfer(t);
    }
    delete transfers;
}

void QTransfersModel::updateInitialTransferInfo(MegaTransfer *transfer)
{
    TransferItem *item = transfers.value(transfer->getTag());
    if (!item)
    {
        return;
    }

    item->setTransferTag(transfer->getTag());
    item->setFileName(QString::fromUtf8(transfer->getFileName()));
    item->setType(transfer->getType(), transfer->isSyncTransfer());
    item->setTotalSize(transfer->getTotalBytes());

    //Update modified item
    auto it = std::lower_bound(transferOrder.begin(), transferOrder.end(), item, priority_comparator);
    int row = std::distance(transferOrder.begin(), it);
    emit dataChanged(index(row, 0, QModelIndex()), index(row, 0, QModelIndex()));

}
void QTransfersModel::updateTransferInfo(MegaTransfer *transfer)
{
    TransferItem *item = transfers.value(transfer->getTag());
    if (!item)
    {
        return;
    }

    item->setSpeed(transfer->getSpeed());
    item->setTransferredBytes(transfer->getTransferredBytes(), !transfer->isSyncTransfer());
    item->setTransferState(transfer->getState());

    if (transfer->getPriority() == item->getPriority())
    {
        //Update modified item
        auto it = std::lower_bound(transferOrder.begin(), transferOrder.end(), item, priority_comparator);
        int row = std::distance(transferOrder.begin(), it);
        emit dataChanged(index(row, 0, QModelIndex()), index(row, 0, QModelIndex()));
    }
    else
    {
        //Move item to its new position
        auto it = std::lower_bound(transferOrder.begin(), transferOrder.end(), item, priority_comparator);
        int row = std::distance(transferOrder.begin(), it);

        TransferItem testItem;
        testItem.setPriority(transfer->getPriority());
        auto newit = std::lower_bound(transferOrder.begin(), transferOrder.end(), &testItem, priority_comparator);
        int newrow = std::distance(transferOrder.begin(), newit);

        beginMoveRows(QModelIndex(), row, row, QModelIndex(), newrow);
        transferOrder.erase(it);
        item->setPriority(transfer->getPriority());
        auto finalit = std::lower_bound(transferOrder.begin(), transferOrder.end(), item, priority_comparator);
        transferOrder.insert(finalit, item);
        endMoveRows();
    }
}

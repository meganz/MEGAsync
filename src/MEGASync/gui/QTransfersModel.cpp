#include "QTransfersModel.h"
#include "MegaApplication.h"

using namespace mega;

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

    return createIndex(row, column, transfers.value(transfersOrder.at(row)));
}

void QTransfersModel::insertTransfer(MegaTransfer *transfer)
{
    beginInsertRows(QModelIndex(), transfers.size(), transfers.size());
    transfers.insert(transfer->getTag(), new TransferItem());
    transfersOrder.append(transfer->getTag());
    endInsertRows();

    updateInitialTransferInfo(transfer);
    updateTransferInfo(transfer);

    if (transfersOrder.size() == 1)
    {
        emit onTransferAdded();
    }
}

void QTransfersModel::removeTransfer(MegaTransfer *transfer)
{
    int row = transfersOrder.indexOf(transfer->getTag());
    beginRemoveRows(QModelIndex(), row, row);
    transfers.remove(transfer->getTag());
    transfersOrder.removeAt(row);
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
    return transfersOrder.size();
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
    else if (type == TYPE_FINISHED)
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

    item->setFileName(QString::fromUtf8(transfer->getFileName()));
    item->setType(transfer->getType(), transfer->isSyncTransfer());
    item->setTotalSize(transfer->getTotalBytes());

    //Update modified item
    int row = transfersOrder.indexOf(transfer->getTag());
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
    type == TYPE_FINISHED ? item->finishTransfer() : item->updateTransfer();

    //Update modified item
    int row = transfersOrder.indexOf(transfer->getTag());
    emit dataChanged(index(row, 0, QModelIndex()), index(row, 0, QModelIndex()));
}

#include "QTransfersModel.h"
#include "MegaApplication.h"

using namespace mega;

QTransfersModel::QTransfersModel(int type, QObject *parent) :
    QAbstractItemModel(parent)
{
    MegaApi *api =  ((MegaApplication *)qApp)->getMegaApi();
    this->type = type;

    delegateListener = new QTMegaTransferListener(api, this);
    api->addTransferListener(delegateListener);
}

TransferItem *QTransfersModel::transferFromIndex(const QModelIndex &index) const
{
    if(index.isValid())
    {
        return static_cast<TransferItem *>(index.internalPointer());
    }else
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

void QTransfersModel::insertTransfer(MegaTransfer *transfer, const QModelIndex &parent)
{
    if (transfers.isEmpty())
    {
        emit onTransferAdded();
    }

    beginInsertRows(QModelIndex(), transfers.size(), transfers.size());
    transfers.insert(transfer->getTag(), new TransferItem());
    transfersOrder.append(transfer->getTag());
    endInsertRows();
}

void QTransfersModel::removeTransfer(MegaTransfer *transfer, const QModelIndex &parent)
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

void QTransfersModel::onTransferStart(MegaApi *api, MegaTransfer *transfer)
{
    if (type == TYPE_ALL || transfer->getType() == type)
    {
        insertTransfer(transfer, QModelIndex());
        emit onTransferAdded();
        updateTransferInfo(transfer);
    }

}

void QTransfersModel::onTransferFinish(MegaApi *api, MegaTransfer *transfer, MegaError *e)
{
    if (type == TYPE_ALL || transfer->getType() == type)
    {
        if (transfers.contains(transfer->getTag()))
        {
            TransferItem *item = transfers.value(transfer->getTag());
            item->finishTransfer();
            removeTransfer(transfer, QModelIndex());

            //Update modified item
            int row = transfersOrder.indexOf(transfer->getTag());
            emit dataChanged(index(row, 0, QModelIndex()), index(row, 0, QModelIndex()));
        }
    }
}

void QTransfersModel::onTransferUpdate(MegaApi *api, MegaTransfer *transfer)
{
    if (type == TYPE_ALL || transfer->getType() == type)
    {
        if (transfers.contains(transfer->getTag()))
        {
            updateTransferInfo(transfer);
        }
    }
}

void QTransfersModel::onTransferTemporaryError(MegaApi *api, MegaTransfer *transfer, MegaError *e)
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
        insertTransfer(t, QModelIndex());
    }
    delete transfers;
}

void QTransfersModel::updateTransferInfo(MegaTransfer *transfer)
{
    TransferItem *item = transfers.value(transfer->getTag());
    item->setFileName(QString::fromUtf8(transfer->getFileName()));
    item->setType(transfer->getType(), transfer->isSyncTransfer());
    item->setSpeed(transfer->getSpeed());
    item->setTotalSize(transfer->getTotalBytes());
    item->setTransferredBytes(transfer->getTransferredBytes(), !transfer->isSyncTransfer());
    item->updateTransfer();

    //Update modified item
    int row = transfersOrder.indexOf(transfer->getTag());
    emit dataChanged(index(row, 0, QModelIndex()), index(row, 0, QModelIndex()));
}

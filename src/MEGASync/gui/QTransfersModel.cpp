#include "QTransfersModel.h"

using namespace mega;

QTransfersModel::QTransfersModel(QObject *parent) :
    QAbstractItemModel(parent)
{
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

    return createIndex(row, column, transfers.at(row));
}

void QTransfersModel::insertTransfer(TransferItem *transfer, const QModelIndex &parent)
{
    beginInsertRows(QModelIndex(), transfers.size(), transfers.size());
    transfers.append(transfer);
    endInsertRows();
}

int QTransfersModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return transfers.size();
}

QTransfersModel::~QTransfersModel()
{
    qDeleteAll(transfers);
}

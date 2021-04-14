#include "QTransfersModel.h"
#include "MegaApplication.h"

using namespace mega;

QTransfersModel::QTransfersModel(QTransfersModel::ModelType type, QObject *parent) :
    QAbstractItemModel(parent),
    mType(type),
    mMegaApi(((MegaApplication *)qApp)->getMegaApi()),
    mThreadPool(ThreadPoolSingleton::getInstance())
{
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
    else if (role == Qt::UserRole)
    {
        return QVariant::fromValue(transfers.value(index.internalId()));
    }

    return QVariant();
}

QModelIndex QTransfersModel::parent(const QModelIndex &) const
{
    return QModelIndex();
}

QModelIndex QTransfersModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))//Check out
    {
        return QModelIndex();
    }

    return createIndex(row, column, transferOrder[row]->data.tag);
}

void QTransfersModel::refreshTransfers()
{
    if (transferOrder.size())
    {
        emit dataChanged(index(0, 0, QModelIndex()), index(int(transferOrder.size()) - 1, 0, QModelIndex()));
    }
}

int QTransfersModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return int(transferOrder.size());
}

QTransfersModel::ModelType QTransfersModel::getModelType()
{
    return mType;
}

QTransfersModel::~QTransfersModel()
{
    qDeleteAll(transfers);
}

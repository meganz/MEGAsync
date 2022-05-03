#include "QTransfersModel.h"
#include "MegaApplication.h"

using namespace mega;

QTransfersModel::QTransfersModel(int type, QObject *parent) :
    QAbstractItemModel(parent)
{
    this->type = type;
    this->megaApi = ((MegaApplication *)qApp)->getMegaApi();
    this->transferItems.setMaxCost(16);

    mThreadPool = ThreadPoolSingleton::getInstance();
}

int QTransfersModel::columnCount(const QModelIndex&) const
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
        const int id = static_cast<int>(index.internalId());
        return QVariant::fromValue(transfers.value(id));
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
        // We can safely cast transferOrder.size() to int, because an int should enough
        // to represent that value.
        emit dataChanged(index(0, 0, QModelIndex()),
                         index(static_cast<int>(transferOrder.size()) - 1, 0, QModelIndex()));
    }
}

int QTransfersModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return static_cast<int>(transferOrder.size());
}

int QTransfersModel::getModelType()
{
    return type;
}

QTransfersModel::~QTransfersModel()
{
    qDeleteAll(transfers);
}

void QTransfersModel::retryAllTransfers()
{
  // Interface stub
}

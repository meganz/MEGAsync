#include "QAlertsModel.h"
#include <QDateTime>

using namespace mega;

QAlertsModel::QAlertsModel(MegaUserAlertList *alerts, QObject *parent)
    : QAbstractItemModel(parent)
{
    alertItems.setMaxCost(16);

    int numAlerts = alerts->size();
    if (numAlerts)
    {
        beginInsertRows(QModelIndex(), 0, numAlerts - 1);
        for (int i = 0; i < numAlerts; i++)
        {
            MegaUserAlert *alert = alerts->get(i);
            alertOrder.push_front(alert->getId());
            alertsMap.insert(alert->getId(), alert);
        }
        endInsertRows();
    }
}

QAlertsModel::~QAlertsModel()
{
    qDeleteAll(alertsMap);
}

QModelIndex QAlertsModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
    {
        return QModelIndex();
    }

    return createIndex(row, column, alertsMap.value(alertOrder[row]));
}

QModelIndex QAlertsModel::parent(const QModelIndex &index) const
{
    return QModelIndex();
}

int QAlertsModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}

int QAlertsModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return alertOrder.size();
}

QVariant QAlertsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || (index.row() < 0 || alertOrder.size() <= index.row()))
    {
        return QVariant();
    }

    if (role == Qt::DisplayRole)
    {
        return QVariant::fromValue(index.internalId());
    }

    if (role == Qt::UserRole) //Role used to sort by date
    {
        MegaUserAlert *item = (MegaUserAlert *)index.internalPointer();
        QDateTime date;
        date.setMSecsSinceEpoch(item->getTimestamp(0) * 1000);

        return date;
    }

    return QVariant();
}

void QAlertsModel::refreshAlerts()
{
    if (alertOrder.size())
    {
        emit dataChanged(index(0, 0, QModelIndex()), index(alertOrder.size() - 1, 0, QModelIndex()));
    }
}

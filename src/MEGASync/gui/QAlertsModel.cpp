#include "QAlertsModel.h"
#include <QDateTime>

using namespace mega;

QAlertsModel::QAlertsModel(MegaUserAlertList *alerts, bool copy, QObject *parent)
    : QAbstractItemModel(parent)
{
    alertItems.setMaxCost(16);
    insertAlerts(alerts, copy);
}

void QAlertsModel::insertAlerts(MegaUserAlertList *alerts, bool copy)
{
    int numAlerts = alerts->size();
    int actualnumberofalertstoinsert = 0;
    if (numAlerts)
    {
//        for (int i = 0; i < numAlerts; i++)
//        {
//            if (alertsMap.find(alerts->get(i)->getId()) != alertsMap.end())
//            {
//                actualnumberofalertstoinsert ++;
//            }
//        }
//        beginInsertRows(QModelIndex(), 0, actualnumberofalertstoinsert - 1);
//        actualnumberofalertstoinsert = 0;

        for (int i = 0; i < numAlerts; i++)
        {
            if (!copy) //first time, alertsMap should be empty
            {
                MegaUserAlert *alert = alerts->get(i);
                alertOrder.push_front(alert->getId());
                alertsMap.insert(alert->getId(), alert);
                actualnumberofalertstoinsert++;
            }
            else
            {
                MegaUserAlert *alert = alerts->get(i)->copy();
                QMap<int, mega::MegaUserAlert*>::iterator existing = alertsMap.find(alert->getId());
                if (existing != alertsMap.end())
                {
                    MegaUserAlert *old = existing.value();
                    alertsMap.erase(existing);
                    delete old;
                    alertsMap.insert(alert->getId(), alert);
                }
                else
                {
                    alertOrder.push_front(alert->getId());
                    alertsMap.insert(alert->getId(), alert);
                    actualnumberofalertstoinsert++;
                }
            }
        }

        // this might actually be problematic, but since the change on the underlying data is done synchrnously
        // it seems we don't need to call beginInsert before actually doing it
        beginInsertRows(QModelIndex(), 0, actualnumberofalertstoinsert - 1);
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

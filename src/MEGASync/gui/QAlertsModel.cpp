#include "QAlertsModel.h"
#include "QFilterAlertsModel.h"
#include <QDateTime>
#include "Preferences.h"
#include <assert.h>

using namespace mega;

QAlertsModel::QAlertsModel(MegaUserAlertList *alerts, bool copy, QObject *parent)
    : QAbstractItemModel(parent)
{

    for(int i = 0; i < ALERT_ALL; i++)
    {
        hasNotificationsOfType[i] = false;
        unSeenNotifications[i] = 0;
    }

    alertItems.setMaxCost(16);
    insertAlerts(alerts, copy);
}

void QAlertsModel::insertAlerts(MegaUserAlertList *alerts, bool copy)
{
    int numAlerts = alerts->size();
    int actualnumberofalertstoinsert = 0;
    if (numAlerts)
    {
        for (int i = 0; i < numAlerts; i++)
        {
            MegaUserAlert *alert = alerts->get(i);
            if (alertsMap.find(alert->getId()) == alertsMap.end())
            {
                actualnumberofalertstoinsert++;
                if (checkAlertType(alert->getType()) != -1)
                {
                    hasNotificationsOfType[checkAlertType(alert->getType())] = true;
                }
            }
        }

        int deleted = 0;
        while (copy && actualnumberofalertstoinsert - deleted + (int)alertsMap.size() >= (int)Preferences::MAX_COMPLETED_ITEMS)
        {
            MegaUserAlert *alertToDelete = alertsMap[alertOrder.back()];
            assert(alertToDelete && "something went wrong: no alert to delete");
            if (alertToDelete)
            {
                if (!alertToDelete->getSeen())
                {
                    if (checkAlertType(alertToDelete->getType()) != -1)
                    {
                        unSeenNotifications[checkAlertType(alertToDelete->getType())]--;
                    }
                }

                int row = int(alertOrder.size()) - 1;
                beginRemoveRows(QModelIndex(), row, row);
                alertsMap.remove(alertToDelete->getId());
                alertOrder.pop_back();
                endRemoveRows();
                delete alertToDelete;
                deleted++;
            }
        }

        if (actualnumberofalertstoinsert > 0)
        {
            beginInsertRows(QModelIndex(), 0, actualnumberofalertstoinsert - 1);
        }

        for (int i = qMax(0, numAlerts - (int)Preferences::MAX_COMPLETED_ITEMS) ; i < numAlerts; i++)
        {
            if (!copy) //first time, alertsMap should be empty
            {
                MegaUserAlert *alert = alerts->get(i);
                if (!alert->isRemoved())
                {
                    alertOrder.push_front(alert->getId());
                    alertsMap.insert(alert->getId(), alert);
                    if (!alert->getSeen())
                    {
                        if (checkAlertType(alert->getType()) != -1)
                        {
                            unSeenNotifications[checkAlertType(alert->getType())]++;
                        }
                    }
                }
            }
            else
            {
                MegaUserAlert *alert = alerts->get(i)->copy();
                if (!alert->isRemoved())
                {
                    QMap<int, mega::MegaUserAlert*>::iterator existing = alertsMap.find(alert->getId());
                    if (existing != alertsMap.end())
                    {
                        MegaUserAlert *old = existing.value();
                        alertsMap[alert->getId()] = alert;
                        if (alert->getSeen() != old->getSeen())
                        {
                            if (checkAlertType(alert->getType()) != -1)
                            {
                                unSeenNotifications[checkAlertType(alert->getType())] += alert->getSeen() ? -1 : 1;
                            }
                        }

                        AlertItem *udpatedAlertItem = alertItems[alert->getId()];
                        if (udpatedAlertItem)
                        {
                            udpatedAlertItem->setAlertData(alert);
                        }

                        delete old;

                        //update row element
                        std::deque<unsigned int>::iterator orderIter = std::find(alertOrder.begin(), alertOrder.end(),alert->getId());
                        assert(orderIter != alertOrder.end() && (*orderIter) == alert->getId());
                        if (orderIter != alertOrder.end() && (*orderIter) == alert->getId())
                        {
                            const int row = static_cast<int>(std::distance(alertOrder.begin(),orderIter));
                            if (row < static_cast<int>(alertOrder.size()))
                            {
                                emit dataChanged(index(row, 0, QModelIndex()), index(row, 0, QModelIndex()));
                            }
                            else
                            {
                                assert(false || "unexpected row to update");
                            }
                        }
                    }
                    else
                    {
                        alertOrder.push_front(alert->getId());
                        alertsMap.insert(alert->getId(), alert);
                        if (!alert->getSeen())
                        {
                            if (checkAlertType(alert->getType()) != -1)
                            {
                                unSeenNotifications[checkAlertType(alert->getType())]++;
                            }
                        }
                    }
                }
            }
        }

        if (actualnumberofalertstoinsert > 0)
        {
            endInsertRows();
        }
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

QModelIndex QAlertsModel::parent(const QModelIndex&) const
{
    return QModelIndex();
}

int QAlertsModel::columnCount(const QModelIndex&) const
{
    return 1;
}

int QAlertsModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return int(alertOrder.size());
}

QVariant QAlertsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || (index.row() < 0 || (int)alertOrder.size() <= index.row()))
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
        emit dataChanged(index(0, 0, QModelIndex()), index(int(alertOrder.size()) - 1, 0, QModelIndex()));
    }
}

long long QAlertsModel::getUnseenNotifications(int type) const
{
    return type == ALERT_ALL ? std::accumulate(unSeenNotifications.begin(), unSeenNotifications.end(), 0) : unSeenNotifications[type];
}

bool QAlertsModel::existsNotifications(int type) const
{
    return hasNotificationsOfType[type];
}

int QAlertsModel::checkAlertType(int alertType) const
{
    switch (alertType)
    {
            case MegaUserAlert::TYPE_INCOMINGPENDINGCONTACT_REQUEST:
            case MegaUserAlert::TYPE_INCOMINGPENDINGCONTACT_CANCELLED:
            case MegaUserAlert::TYPE_INCOMINGPENDINGCONTACT_REMINDER:
            case MegaUserAlert::TYPE_CONTACTCHANGE_DELETEDYOU:
            case MegaUserAlert::TYPE_CONTACTCHANGE_CONTACTESTABLISHED:
            case MegaUserAlert::TYPE_CONTACTCHANGE_ACCOUNTDELETED:
            case MegaUserAlert::TYPE_CONTACTCHANGE_BLOCKEDYOU:
            case MegaUserAlert::TYPE_UPDATEDPENDINGCONTACTINCOMING_IGNORED:
            case MegaUserAlert::TYPE_UPDATEDPENDINGCONTACTINCOMING_ACCEPTED:
            case MegaUserAlert::TYPE_UPDATEDPENDINGCONTACTINCOMING_DENIED:
            case MegaUserAlert::TYPE_UPDATEDPENDINGCONTACTOUTGOING_ACCEPTED:
            case MegaUserAlert::TYPE_UPDATEDPENDINGCONTACTOUTGOING_DENIED:
                return ALERT_CONTACTS;

            case MegaUserAlert::TYPE_NEWSHARE:
            case MegaUserAlert::TYPE_DELETEDSHARE:
            case MegaUserAlert::TYPE_NEWSHAREDNODES:
            case MegaUserAlert::TYPE_REMOVEDSHAREDNODES:
                return ALERT_SHARES;

            case MegaUserAlert::TYPE_PAYMENT_SUCCEEDED:
            case MegaUserAlert::TYPE_PAYMENT_FAILED:
            case MegaUserAlert::TYPE_PAYMENTREMINDER:
                return ALERT_PAYMENT;

            case MegaUserAlert::TYPE_TAKEDOWN:
            case MegaUserAlert::TYPE_TAKEDOWN_REINSTATED:
                return ALERT_TAKEDOWNS;

            default:
                return ALERT_UNKNOWN;
    }

#ifndef WIN32
    return -1; // warning C4702: unreachable code
#endif
}

void QAlertsModel::refreshAlertItem(unsigned id)
{
    int row = 0;
    for (auto it = alertOrder.begin(); it != alertOrder.end() && *it != id; ++it)
    {
        ++row;
    }

    const int alertOrderSize = static_cast<int>(alertOrder.size());
    assert(row < alertOrderSize);
    if (row >= alertOrderSize)
    {
        return;
    }

    emit dataChanged(index(row, 0, QModelIndex()), index(row, 0, QModelIndex()));

}

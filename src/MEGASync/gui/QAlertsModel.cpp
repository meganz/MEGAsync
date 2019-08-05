#include "QAlertsModel.h"
#include "QFilterAlertsModel.h"
#include <QDateTime>
#include <assert.h>

using namespace mega;

QAlertsModel::QAlertsModel(MegaUserAlertList *alerts, bool copy, QObject *parent)
    : QAbstractItemModel(parent)
{

    for(int i = 0; i < ALERT_TYPES; i++)
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
                hasNotificationsOfType[checkAlertType(alert->getType())] = true;
            }
        }
        if (actualnumberofalertstoinsert > 0)
        {
            beginInsertRows(QModelIndex(), 0, actualnumberofalertstoinsert - 1);
        }
        actualnumberofalertstoinsert = 0;

        for (int i = 0; i < numAlerts; i++)
        {
            if (!copy) //first time, alertsMap should be empty
            {
                MegaUserAlert *alert = alerts->get(i);
                alertOrder.push_front(alert->getId());
                alertsMap.insert(alert->getId(), alert);
                if (!alert->getSeen())
                {
                    if (checkAlertType(alert->getType()) != -1)
                    {
                        unSeenNotifications[checkAlertType(alert->getType())]++;
                    }
                }
                actualnumberofalertstoinsert++;
            }
            else
            {
                MegaUserAlert *alert = alerts->get(i)->copy();
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
                    delete old;

                    //update row element
                    std::deque<int>::iterator orderIter = std::find(alertOrder.begin(), alertOrder.end(),alert->getId());
                    assert(orderIter != alertOrder.end() && (*orderIter) == alert->getId());
                    if (orderIter != alertOrder.end() && (*orderIter) == alert->getId())
                    {
                        int row = std::distance(alertOrder.begin(),orderIter);
                        if (row < alertOrder.size())
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
                    actualnumberofalertstoinsert++;
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

        if (actualnumberofalertstoinsert > 0)
        {
            // this might actually be problematic, but since the change on the underlying data seems to be done synchronously
            // it seems we might not need to call beginInsert before actually doing it
//            beginInsertRows(QModelIndex(), 0, actualnumberofalertstoinsert - 1);
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
                break;

            case MegaUserAlert::TYPE_NEWSHARE:
            case MegaUserAlert::TYPE_DELETEDSHARE:
            case MegaUserAlert::TYPE_NEWSHAREDNODES:
            case MegaUserAlert::TYPE_REMOVEDSHAREDNODES:
                return ALERT_SHARES;
                break;

            case MegaUserAlert::TYPE_PAYMENT_SUCCEEDED:
            case MegaUserAlert::TYPE_PAYMENT_FAILED:
            case MegaUserAlert::TYPE_PAYMENTREMINDER:
                return ALERT_PAYMENT;
                break;

            case MegaUserAlert::TYPE_TAKEDOWN:
            case MegaUserAlert::TYPE_TAKEDOWN_REINSTATED:
                return ALERT_TAKEDOWNS;
                break;

            default:
                break;
    }

    return -1;
}

#include "AlertModel.h"

#include "Preferences.h"

#include <QDateTime>

#include <assert.h>

namespace
{
constexpr int MAX_COST = 16;
}

using namespace mega;

AlertModel::AlertModel(MegaUserAlertList* alerts, QObject* parent)
    : QAbstractItemModel(parent)
{
    for(int i = 0; i < ALERT_ALL; i++)
    {
        mHasNotificationsOfType[i] = false;
        mUnSeenNotifications[i] = 0;
    }

    alertItems.setMaxCost(MAX_COST);
    insertAlerts(alerts);
}

void AlertModel::insertAlerts(MegaUserAlertList* alerts, bool copy)
{
    int numAlerts = alerts ? alerts->size() : 0;
    int actualNumberOfAlertsToInsert = 0;
    if (numAlerts)
    {
        for (int i = 0; i < numAlerts; i++)
        {
            MegaUserAlert* alert = alerts->get(i);
            if (mAlertsMap.find(alert->getId()) == mAlertsMap.end())
            {
                ++actualNumberOfAlertsToInsert;
                if (checkAlertType(alert->getType()) != AlertModel::ALERT_UNKNOWN)
                {
                    mHasNotificationsOfType[checkAlertType(alert->getType())] = true;
                }
            }
        }

        int deleted = 0;
        while (copy && !mAlertOrder.empty()
               && actualNumberOfAlertsToInsert - deleted + (int)mAlertsMap.size() >= (int)Preferences::MAX_COMPLETED_ITEMS)
        {
            MegaUserAlertExt* alertToDelete = mAlertsMap[mAlertOrder.back()];
            assert(alertToDelete && "something went wrong: no alert to delete");
            if (alertToDelete)
            {
                if (!alertToDelete->getSeen())
                {
                    if (checkAlertType(alertToDelete->getType()) != AlertModel::ALERT_UNKNOWN)
                    {
                        mUnSeenNotifications[checkAlertType(alertToDelete->getType())]--;
                    }
                }

                int row = int(mAlertOrder.size()) - 1;
                beginRemoveRows(QModelIndex(), row, row);
                mAlertsMap.remove(alertToDelete->getId());
                mAlertOrder.pop_back();
                endRemoveRows();
                delete alertToDelete;
                deleted++;
            }
        }

        if (actualNumberOfAlertsToInsert > 0)
        {
            beginInsertRows(QModelIndex(), 0, actualNumberOfAlertsToInsert - 1);
        }

        for (int i = qMax(0, numAlerts - (int)Preferences::MAX_COMPLETED_ITEMS) ; i < numAlerts; i++)
        {
            if (!copy) //first time, alertsMap should be empty
            {
                MegaUserAlert* alert = alerts->get(i);
                if (!alert->isRemoved())
                {
                    mAlertOrder.push_front(alert->getId());
                    mAlertsMap.insert(alert->getId(), new MegaUserAlertExt(alert));
                    if (!alert->getSeen())
                    {
                        if (checkAlertType(alert->getType()) != AlertModel::ALERT_UNKNOWN)
                        {
                            mUnSeenNotifications[checkAlertType(alert->getType())]++;
                        }
                    }
                }
            }
            else
            {
                MegaUserAlert* alert = alerts->get(i)->copy();
                if (!alert->isRemoved())
                {
                    auto existing = mAlertsMap.find(alert->getId());
                    if (existing != mAlertsMap.end())
                    {
                        std::unique_ptr<MegaUserAlertExt> oldAlert{existing.value()};
                        mAlertsMap[alert->getId()] = new MegaUserAlertExt(alert);
                        if (alert->getSeen() != oldAlert->getSeen())
                        {
                            if (checkAlertType(alert->getType()) != AlertModel::ALERT_UNKNOWN)
                            {
                                mUnSeenNotifications[checkAlertType(alert->getType())] += alert->getSeen() ? -1 : 1;
                            }
                        }

                        AlertItem* udpatedAlertItem = alertItems[alert->getId()];
                        if (udpatedAlertItem)
                        {
                            udpatedAlertItem->setAlertData(mAlertsMap[alert->getId()]);
                        }

                        //update row element
                        std::deque<unsigned int>::iterator orderIter = std::find(mAlertOrder.begin(), mAlertOrder.end(),alert->getId());
                        assert(orderIter != mAlertOrder.end() && (*orderIter) == alert->getId());
                        if (orderIter != mAlertOrder.end() && (*orderIter) == alert->getId())
                        {
                            const int row = static_cast<int>(std::distance(mAlertOrder.begin(),orderIter));
                            if (row >= 0 && row < static_cast<int>(mAlertOrder.size()))
                            {
                                emit dataChanged(index(row, 0, QModelIndex()), index(row, 0, QModelIndex()));
                            }
                            else
                            {
                                Q_ASSERT_X(false, "AlertModel::insertAlerts()", "unexpected row to update");
                            }
                        }
                    }
                    else
                    {
                        mAlertOrder.push_front(alert->getId());
                        mAlertsMap.insert(alert->getId(), new MegaUserAlertExt(alert));
                        if (!alert->getSeen())
                        {
                            if (checkAlertType(alert->getType()) != AlertModel::ALERT_UNKNOWN)
                            {
                                mUnSeenNotifications[checkAlertType(alert->getType())]++;
                            }
                        }
                    }
                }
            }
        }

        if (actualNumberOfAlertsToInsert > 0)
        {
            endInsertRows();
        }
    }
}

AlertModel::~AlertModel()
{
    qDeleteAll(mAlertsMap);
}

QModelIndex AlertModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
    {
        return QModelIndex();
    }

    return createIndex(row, column, mAlertsMap.value(mAlertOrder[row]));
}

QModelIndex AlertModel::parent(const QModelIndex&) const
{
    return QModelIndex();
}

int AlertModel::columnCount(const QModelIndex&) const
{
    return 1;
}

int AlertModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return int(mAlertOrder.size());
}

QVariant AlertModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || (index.row() < 0 || (int)mAlertOrder.size() <= index.row()))
    {
        return QVariant();
    }

    if (role == Qt::DisplayRole)
    {
        return QVariant::fromValue(index.internalId());
    }

    if (role == Qt::UserRole) //Role used to sort by date
    {
        MegaUserAlertExt* item = (MegaUserAlertExt *)index.internalPointer();
        QDateTime date;
        date.setMSecsSinceEpoch(item->getTimestamp(0) * 1000);
        return date;
    }

    return QVariant();
}

void AlertModel::refreshAlerts()
{
    if (mAlertOrder.size())
    {
        emit dataChanged(index(0, 0, QModelIndex()), index(int(mAlertOrder.size()) - 1, 0, QModelIndex()));
    }
}

QMap<AlertModel::AlertType, long long> AlertModel::getUnseenNotifications() const
{
    QMap<AlertModel::AlertType, long long> unseen;
    for (int i = 0; i < ALERT_ALL; i++)
    {
        unseen[static_cast<AlertType>(i)] = mUnSeenNotifications[i];
    }
    unseen[ALERT_ALL] = std::accumulate(mUnSeenNotifications.begin(), mUnSeenNotifications.end(), 0);
    return unseen;
}

bool AlertModel::existsNotifications(int type) const
{
    return mHasNotificationsOfType[type];
}

int AlertModel::checkAlertType(int alertType) const
{
    int type = ALERT_UNKNOWN;
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
        {
            type = ALERT_CONTACTS;
            break;
        }
        case MegaUserAlert::TYPE_NEWSHARE:
        case MegaUserAlert::TYPE_DELETEDSHARE:
        case MegaUserAlert::TYPE_NEWSHAREDNODES:
        case MegaUserAlert::TYPE_REMOVEDSHAREDNODES:
        case MegaUserAlert::TYPE_UPDATEDSHAREDNODES:
        {
            type = ALERT_SHARES;
            break;
        }
        case MegaUserAlert::TYPE_PAYMENT_SUCCEEDED:
        case MegaUserAlert::TYPE_PAYMENT_FAILED:
        case MegaUserAlert::TYPE_PAYMENTREMINDER:
        {
            type = ALERT_PAYMENT;
            break;
        }
        case MegaUserAlert::TYPE_TAKEDOWN:
        case MegaUserAlert::TYPE_TAKEDOWN_REINSTATED:
        {
            type = ALERT_TAKEDOWNS;
            break;
        }
        default:
        {
            break;
        }
    }
    return type;
}

void AlertModel::refreshAlertItem(unsigned id)
{
    int row = 0;
    for (auto it = mAlertOrder.begin(); it != mAlertOrder.end() && *it != id; ++it)
    {
        ++row;
    }

    const int alertOrderSize = static_cast<int>(mAlertOrder.size());
    assert(row < alertOrderSize);
    if (row >= alertOrderSize)
    {
        return;
    }

    emit dataChanged(index(row, 0, QModelIndex()), index(row, 0, QModelIndex()));
}

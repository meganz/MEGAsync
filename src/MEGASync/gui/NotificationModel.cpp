#include "NotificationModel.h"

#include <QDateTime>

namespace
{
constexpr int MAX_COST = 16;
}

NotificationModel::NotificationModel(QObject* parent)
    : QAbstractItemModel(parent)
    , mLastSeenNotification(0)
{
    notificationItems.setMaxCost(MAX_COST);
}

NotificationModel::~NotificationModel()
{
    qDeleteAll(mNotificationsMap);
}

QModelIndex NotificationModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
    {
        return QModelIndex();
    }

    return createIndex(row, column, mNotificationsMap.value(mNotificationsOrder[row]));
}

QModelIndex NotificationModel::parent(const QModelIndex&) const
{
    return QModelIndex();
}

int NotificationModel::columnCount(const QModelIndex&) const
{
    return 1;
}

int NotificationModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return mNotificationsMap.size();
}

QVariant NotificationModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0)
    {
        return QVariant();
    }

    if (role == Qt::DisplayRole)
    {
        return QVariant::fromValue(index.internalId());
    }

    return QVariant();
}

void NotificationModel::insert(const mega::MegaNotificationList* notifications)
{
    if (!notifications)
    {
        return;
    }

    removeNotifications(notifications);
    updateNotifications(notifications);
    insertNewNotifications(notifications);
}

long long NotificationModel::getNumUnseenNotifications() const
{
    long long unseenCount = 0;
    for (const auto& notification : mNotificationsMap)
    {
        if (notification->getID() > mLastSeenNotification)
        {
            ++unseenCount;
        }
    }
    return unseenCount;
}

uint32_t NotificationModel::getLastSeenNotification() const
{
    uint32_t lastSeen = 0;
    if(!mLastSeenNotification)
    {
        for (const auto& notification : mNotificationsMap)
        {
            if (notification->getID() > lastSeen)
            {
                lastSeen = notification->getID();
            }
        }
    }
    return lastSeen;
}

void NotificationModel::setLastSeenNotification(uint32_t id)
{
    if(id > mLastSeenNotification)
    {
        mLastSeenNotification = id;
    }
}

int NotificationModel::countNewNotifications(const mega::MegaNotificationList* notifications) const
{
    int newItemsCount = 0;
    for (int i = 0; i < notifications->size(); ++i)
    {
        const mega::MegaNotification* notification = notifications->get(i);
        if (notification && !mNotificationsMap.contains(notification->getID()))
        {
            ++newItemsCount;
        }
    }
    return newItemsCount;
}

void NotificationModel::insertNewNotifications(const mega::MegaNotificationList* notifications)
{
    int newItemsCount = countNewNotifications(notifications);
    if (newItemsCount == 0)
    {
        return;
    }

    beginInsertRows(QModelIndex(), 0, newItemsCount - 1);

    for (int i = 0; i < notifications->size(); ++i)
    {
        const mega::MegaNotification* notification = notifications->get(i);
        if (notification && !mNotificationsMap.contains(notification->getID()))
        {
            MegaNotificationExt* notificationExt = new MegaNotificationExt(notification->copy());
            mNotificationsMap.insert(notification->getID(), notificationExt);
            mNotificationsOrder.push_front(notification->getID());
        }
    }
    endInsertRows();
}

void NotificationModel::removeNotifications(const mega::MegaNotificationList* notifications)
{
    QSet<int> notificationIDsInList = createNotificationIDSet(notifications);
    QList<int> rowsToRemove = findRowsToRemove(notificationIDsInList);
    removeRows(rowsToRemove);
}

QSet<int> NotificationModel::createNotificationIDSet(const mega::MegaNotificationList* notifications) const
{
    QSet<int> notificationIDsInList;
    for (int i = 0; i < notifications->size(); ++i)
    {
        const mega::MegaNotification* notification = notifications->get(i);
        if (notification)
        {
            notificationIDsInList.insert(notification->getID());
        }
    }
    return notificationIDsInList;
}

QList<int> NotificationModel::findRowsToRemove(const QSet<int>& notificationIDsInList) const
{
    QList<int> rowsToRemove;
    for (auto it = mNotificationsOrder.begin(); it != mNotificationsOrder.end(); ++it)
    {
        int notificationID = *it;
        if (!notificationIDsInList.contains(notificationID))
        {
            rowsToRemove.append(std::distance(mNotificationsOrder.begin(), it));
        }
    }
    return rowsToRemove;
}

void NotificationModel::removeRows(const QList<int>& rowsToRemove)
{
    if (rowsToRemove.isEmpty())
    {
        return;
    }

    beginRemoveRows(QModelIndex(), 0, rowsToRemove.size() - 1);

    for (int i = rowsToRemove.size() - 1; i >= 0; --i)
    {
        int row = rowsToRemove.at(i);
        int notificationID = mNotificationsOrder.at(row);
        delete mNotificationsMap.take(notificationID);
        mNotificationsOrder.erase(mNotificationsOrder.begin() + row);
    }

    endRemoveRows();
}

void NotificationModel::updateNotifications(const mega::MegaNotificationList* notifications)
{
    if (!notifications)
    {
        return;
    }

    for (int i = 0; i < notifications->size(); ++i)
    {
        const mega::MegaNotification* notification = notifications->get(i);
        if (!notification)
        {
            continue;
        }

        int notificationID = notification->getID();
        if (mNotificationsMap.contains(notificationID))
        {
            MegaNotificationExt* notificationExt = mNotificationsMap.value(notificationID);
            notificationExt->reset(notification->copy());

            auto orderIter = std::find(mNotificationsOrder.begin(), mNotificationsOrder.end(), notificationID);
            if (orderIter != mNotificationsOrder.end())
            {
                int row = static_cast<int>(std::distance(mNotificationsOrder.begin(), orderIter));
                if (row >= 0 && row < static_cast<int>(mNotificationsOrder.size()))
                {
                    emit dataChanged(index(row, 0, QModelIndex()), index(row, 0, QModelIndex()));
                }
            }
        }
    }
}

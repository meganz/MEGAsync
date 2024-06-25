#include "NotificationModel.h"

#include <QDateTime>

namespace
{
constexpr int MAX_COST = 16;
}

NotificationModel::NotificationModel(const mega::MegaNotificationList* notifications, QObject* parent)
    : QAbstractItemModel(parent)
{
    notificationItems.setMaxCost(MAX_COST);
    insert(notifications);
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

    beginInsertRows(QModelIndex(), 0, notifications->size() - 1);

    for (int i = 0; i < notifications->size(); ++i)
    {
        const mega::MegaNotification* notification = notifications->get(i)->copy();
        if (notification)
        {
            MegaNotificationExt* notificationExt = new MegaNotificationExt(notification);
            mNotificationsMap.insert(notificationExt->getID(), notificationExt);
            mNotificationsOrder.push_front(notificationExt->getID());
        }
    }

    endInsertRows();
}

#include "UserMessageModel.h"

#include "megaapi.h"
#include "Preferences.h"
#include "UserAlert.h"
#include "UserMessageTypes.h"
#include "UserNotification.h"

#include <QDateTime>

UserMessageModel::~UserMessageModel()
{
    qDeleteAll(mUserMessages);
}

QModelIndex UserMessageModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
    {
        return QModelIndex();
    }

    return createIndex(row, column, mUserMessages.at(row));
}

QModelIndex UserMessageModel::parent(const QModelIndex& index) const
{
    Q_UNUSED(index)

    return QModelIndex();
}

int UserMessageModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : mUserMessages.size();
}

int UserMessageModel::columnCount(const QModelIndex& parent) const
{
    return 1;
}

QVariant UserMessageModel::data(const QModelIndex& index, int role) const
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

Qt::ItemFlags UserMessageModel::flags(const QModelIndex& index) const
{
    return QAbstractItemModel::flags(index) | Qt::ItemIsEnabled | Qt::ItemIsEditable;
}

auto UserMessageModel::findById(unsigned id, UserMessage::Type type)
{
    auto it = std::find_if(mUserMessages.begin(),
                           mUserMessages.end(),
                           [id, type](const UserMessage* current) {
                               return current->isOfType(type) && current->hasSameId(id);
                           });
    return it;
}

void UserMessageModel::processAlerts(mega::MegaUserAlertList* alerts)
{
    int numAlerts = alerts ? alerts->size() : 0;
    if (numAlerts)
    {
        QList<mega::MegaUserAlert*> newAlerts;
        QList<mega::MegaUserAlert*> updatedAlerts;
        QList<mega::MegaUserAlert*> removedAlerts;
        for (int i = 0; i < numAlerts; i++)
        {
            mega::MegaUserAlert* alert = alerts->get(i);
            auto it = findById(alert->getId(), UserMessage::Type::ALERT);
            if (it == mUserMessages.end())
            {
                if (alert->isRemoved())
                {
                    removedAlerts.append(alert);
                }
                else
                {
                    newAlerts.append(alert->copy());
                }
            }
            else if (!alert->isRemoved())
            {
                updatedAlerts.append(alert->copy());
            }
        }

        removeAlerts(removedAlerts);
        insertAlerts(newAlerts);
        updateAlerts(updatedAlerts);
    }
}

void UserMessageModel::insertAlerts(const QList<mega::MegaUserAlert*>& alerts)
{
    if (alerts.size() <= 0)
    {
        return;
    }

    beginInsertRows(QModelIndex(), mUserMessages.size(), mUserMessages.size() + alerts.size() - 1);
    for (auto& alert: alerts)
    {
        auto alertItem = new UserAlert(alert);
        mUserMessages.push_back(alertItem);

        if (!alertItem->isSeen())
        {
            mSeenStatusManager.markAsUnseen(alertItem->getMessageType());
        }
    }
    endInsertRows();
}

void UserMessageModel::updateAlerts(const QList<mega::MegaUserAlert*>& alerts)
{
    if (alerts.size() <= 0)
    {
        return;
    }

    for (auto& alert: alerts)
    {
        auto it = findById(alert->getId(), UserMessage::Type::ALERT);
        if (it != mUserMessages.end())
        {
            int row = static_cast<int>(std::distance(mUserMessages.begin(), it));
            auto alertItem = qobject_cast<UserAlert*>(mUserMessages[row]);

            if (alertItem->isSeen() && !alert->getSeen())
            {
                mSeenStatusManager.markAsUnseen(alertItem->getMessageType());
            }
            else if (!alertItem->isSeen() && alert->getSeen())
            {
                mSeenStatusManager.markAsSeen(alertItem->getMessageType());
            }

            alertItem->reset(alert);
            emit dataChanged(index(row, 0, QModelIndex()), index(row, 0, QModelIndex()));
        }
    }
}

void UserMessageModel::removeAlerts(const QList<mega::MegaUserAlert*>& alerts)
{
    if (alerts.size() <= 0)
    {
        return;
    }

    // Remove alerts that are not in the list of removed alerts
    for (auto& alert: alerts)
    {
        auto it = findById(alert->getId(), UserMessage::Type::ALERT);
        if (it != mUserMessages.end())
        {
            int row = static_cast<int>(std::distance(mUserMessages.begin(), it));
            auto alertItem = qobject_cast<UserAlert*>(mUserMessages[row]);
            if (!alertItem->isSeen())
            {
                mSeenStatusManager.markAsSeen(alertItem->getMessageType());
            }

            beginRemoveRows(QModelIndex(), row, row);
            delete mUserMessages[row];
            mUserMessages.erase(it);
            endRemoveRows();
        }
    }

    // Remove the oldest items if the list is too long
    if (static_cast<unsigned>(mUserMessages.size()) > Preferences::MAX_COMPLETED_ITEMS)
    {
        int row = mUserMessages.size() - 1;
        while (row >= 0 &&
               static_cast<unsigned>(mUserMessages.size()) > Preferences::MAX_COMPLETED_ITEMS)
        {
            if (mUserMessages[row]->isOfType(UserMessage::Type::ALERT))
            {
                beginRemoveRows(QModelIndex(), row, row);
                delete mUserMessages[row];
                mUserMessages.removeAt(row);
                endRemoveRows();
            }
            --row;
        }
    }
}

bool UserMessageModel::hasAlertsOfType(MessageType type)
{
    return std::any_of(mUserMessages.begin(),
                       mUserMessages.end(),
                       [type](const UserMessage* current) {
                           if (current->isOfType(UserMessage::Type::ALERT))
                           {
                               auto alertItem = qobject_cast<const UserAlert*>(current);
                               return alertItem && alertItem->getMessageType() == type;
                           }
                           return false;
                       });
}

void UserMessageModel::processNotifications(const mega::MegaNotificationList* notifications)
{
    int numNotifications = notifications ? static_cast<int>(notifications->size()) : 0;
    if (numNotifications > 0)
    {
        QList<mega::MegaNotification*> newNotifications;
        for (int i = 0; i < numNotifications; i++)
        {
            const mega::MegaNotification* notification =
                notifications->get(static_cast<unsigned>(i));

            auto currentTime = QDateTime::currentSecsSinceEpoch();
            if (currentTime < notification->getStart() || currentTime >= notification->getEnd())
            {
                continue;
            }

            auto it = findById(static_cast<unsigned>(notification->getID()),
                               UserMessage::Type::NOTIFICATION);
            if (it == mUserMessages.end())
            {
                newNotifications.append(notification->copy());
            }
            else
            {
                updateNotification(static_cast<int>(std::distance(mUserMessages.begin(), it)),
                                   notification);
            }
        }

        removeNotifications(notifications);
        insertNotifications(newNotifications);
    }
    else
    {
        removeNotifications(nullptr);
    }
}

void UserMessageModel::insertNotifications(const QList<mega::MegaNotification*>& notifications)
{
    if (notifications.size() <= 0)
    {
        return;
    }

    beginInsertRows(QModelIndex(),
                    mUserMessages.size(),
                    mUserMessages.size() + notifications.size() - 1);
    for (auto& notification: notifications)
    {
        auto item = new UserNotification(notification);
        mUserMessages.push_back(item);

        if (!mSeenStatusManager.markNotificationAsUnseen(item->id()))
        {
            item->markAsSeen();
        }

        connect(item, &UserMessage::expired, this, &UserMessageModel::onExpired);
    }
    endInsertRows();
}

void UserMessageModel::updateNotification(int row, const mega::MegaNotification* notification)
{
    auto item = qobject_cast<UserNotification*>(mUserMessages[row]);
    if (!item->equals(notification))
    {
        item->reset(notification->copy());
        emit dataChanged(index(row, 0, QModelIndex()), index(row, 0, QModelIndex()));
    }
}

void UserMessageModel::removeNotifications(const mega::MegaNotificationList* notifications)
{
    for (int row = mUserMessages.size() - 1; row >= 0; --row)
    {
        auto item = mUserMessages.at(row);
        if (!item->isOfType(UserMessage::Type::NOTIFICATION))
        {
            continue;
        }

        bool found = false;
        auto notification = qobject_cast<UserNotification*>(item);
        if (notifications)
        {
            unsigned i = 0;
            while (i < notifications->size() && !found)
            {
                found =
                    notification->hasSameId(static_cast<unsigned>(notifications->get(i)->getID()));
                ++i;
            }
        }

        if (!found)
        {
            if (!notification->isSeen())
            {
                mSeenStatusManager.markAsSeen(MessageType::NOTIFICATIONS);
            }

            beginRemoveRows(QModelIndex(), row, row);
            delete mUserMessages[row];
            mUserMessages.removeAt(row);
            endRemoveRows();
        }
    }
}

UnseenUserMessagesMap UserMessageModel::getUnseenNotifications() const
{
    return mSeenStatusManager.getUnseenUserMessages();
}

uint32_t UserMessageModel::checkLocalLastSeenNotification()
{
    for (auto& item: mUserMessages)
    {
        if (!item->isOfType(UserMessage::Type::NOTIFICATION))
        {
            continue;
        }

        auto currentLastSeen = mSeenStatusManager.getLastSeenNotification();
        auto notification = qobject_cast<UserNotification*>(item);
        auto id = notification->id();
        if (!notification->isSeen() && id > currentLastSeen)
        {
            mSeenStatusManager.setLocalLastSeenNotification(id);
        }
    }
    return mSeenStatusManager.getLocalLastSeenNotification();
}

void UserMessageModel::setLastSeenNotification(uint32_t id)
{
    if (mSeenStatusManager.getLastSeenNotification() < id)
    {
        mSeenStatusManager.setLastSeenNotification(id);
        mSeenStatusManager.setLocalLastSeenNotification(id);
        for (auto& item: mUserMessages)
        {
            if (!item->isOfType(UserMessage::Type::NOTIFICATION))
            {
                continue;
            }

            auto notification = qobject_cast<UserNotification*>(item);
            if (notification->id() <= id && !notification->isSeen())
            {
                notification->markAsSeen();
                mSeenStatusManager.markAsSeen(MessageType::NOTIFICATIONS);
            }
        }
    }
}

void UserMessageModel::onExpired(unsigned id)
{
    // For now, only notifications can expire
    auto it = findById(id, UserMessage::Type::NOTIFICATION);
    if (it != mUserMessages.end())
    {
        int row = static_cast<int>(std::distance(mUserMessages.begin(), it));
        beginRemoveRows(QModelIndex(), row, row);
        delete mUserMessages[row];
        mUserMessages.erase(it);
        endRemoveRows();
    }
}

// ----------------------------------------------------------------------------
//      UserMessageSeenStatusManager
// ----------------------------------------------------------------------------

void UserMessageModel::SeenStatusManager::markAsUnseen(MessageType type)
{
    if (type == MessageType::UNKNOWN || type == MessageType::ALL)
    {
        return;
    }

    mUnseenNotifications[type]++;
    mUnseenNotifications[MessageType::ALL]++;
}

bool UserMessageModel::SeenStatusManager::markNotificationAsUnseen(uint32_t id)
{
    bool success = false;
    if (id > mLastSeenNotification)
    {
        markAsUnseen(MessageType::NOTIFICATIONS);
        success = true;
    }
    return success;
}

void UserMessageModel::SeenStatusManager::markAsSeen(MessageType type)
{
    if (type == MessageType::UNKNOWN || type == MessageType::ALL)
    {
        return;
    }

    mUnseenNotifications[type]--;
    mUnseenNotifications[MessageType::ALL]--;
}

UnseenUserMessagesMap UserMessageModel::SeenStatusManager::getUnseenUserMessages() const
{
    return mUnseenNotifications;
}

void UserMessageModel::SeenStatusManager::setLastSeenNotification(uint32_t id)
{
    mLastSeenNotification = id;
}

uint32_t UserMessageModel::SeenStatusManager::getLastSeenNotification() const
{
    return mLastSeenNotification;
}

void UserMessageModel::SeenStatusManager::setLocalLastSeenNotification(uint32_t id)
{
    mLocalLastSeenNotification = id;
}

uint32_t UserMessageModel::SeenStatusManager::getLocalLastSeenNotification() const
{
    return mLocalLastSeenNotification;
}

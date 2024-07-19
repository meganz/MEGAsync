#include "UserMessageModel.h"

#include "UserMessageTypes.h"
#include "UserNotification.h"
#include "UserAlert.h"
#include "Preferences.h"

#include "megaapi.h"

#include <QDateTime>

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

auto UserMessageModel::findAlertById(unsigned int id)
{
    auto it = std::find_if(mUserMessages.begin(), mUserMessages.end(),
                           [id](const UserMessage* current)
                           {
                               if(current->getType() == UserMessage::Type::ALERT)
                               {
                                   auto alertItem = qobject_cast<const UserAlert*>(current);
                                   return alertItem && alertItem->getId() == id;
                               }
                               return false;
                           });
    return it;
}

auto UserMessageModel::findNotificationById(int64_t id)
{
    auto it = std::find_if(mUserMessages.begin(), mUserMessages.end(),
                           [id](const UserMessage* current)
                           {
                               if(current->getType() == UserMessage::Type::NOTIFICATION)
                               {
                                   auto notificationItem = qobject_cast<const UserNotification*>(current);
                                   return notificationItem && notificationItem->getID() == id;
                               }
                               return false;
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
            auto it = findAlertById(alert->getId());
            if (it == mUserMessages.end() && !alert->isRemoved())
            {
                if(alert->isRemoved())
                {
                    removedAlerts.append(alert);
                }
                else
                {
                    newAlerts.append(alert->copy());
                }
            }
            else if (it != mUserMessages.end())
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
    if(alerts.size() <= 0)
    {
        return;
    }

    beginInsertRows(QModelIndex(),
                    mUserMessages.size(),
                    mUserMessages.size() + alerts.size() - 1);
    for (auto& alert : alerts)
    {
        auto alertItem = new UserAlert(alert);
        mUserMessages.push_back(alertItem);

        if(!alertItem->isSeen())
        {
            mSeenStatusManager.markAsUnseen(alertItem->getMessageType());
        }
    }
    endInsertRows();
}

void UserMessageModel::updateAlerts(const QList<mega::MegaUserAlert*>& alerts)
{
    if(alerts.size() <= 0)
    {
        return;
    }

    for (auto& alert : alerts)
    {
        auto it = findAlertById(alert->getId());
        if (it != mUserMessages.end())
        {
            int row = std::distance(mUserMessages.begin(), it);
            auto alertItem = qobject_cast<UserAlert*>(mUserMessages[row]);

            if(alertItem->isSeen() && !alert->getSeen())
            {
                mSeenStatusManager.markAsUnseen(alertItem->getMessageType());
            }
            else if(!alertItem->isSeen() && alert->getSeen())
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
    if(alerts.size() <= 0)
    {
        return;
    }

    // Remove alerts that are not in the list of removed alerts
    for (auto& alert : alerts)
    {
        auto it = findAlertById(alert->getId());
        if (it != mUserMessages.end())
        {
            int row = std::distance(mUserMessages.begin(), it);
            auto alertItem = qobject_cast<UserAlert*>(mUserMessages[row]);
            if(!alertItem->isSeen())
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
    if(mUserMessages.size() > Preferences::MAX_COMPLETED_ITEMS)
    {
        int row = mUserMessages.size() - 1;
        while(row >= 0 && mUserMessages.size() > Preferences::MAX_COMPLETED_ITEMS)
        {
            if(mUserMessages[row]->getType() == UserMessage::Type::ALERT)
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
    return std::any_of(mUserMessages.begin(), mUserMessages.end(),
                       [type](const UserMessage* current)
                       {
                           if(current->getType() == UserMessage::Type::ALERT)
                           {
                               auto alertItem = qobject_cast<const UserAlert*>(current);
                               return alertItem && alertItem->getMessageType() == type;
                           }
                           return false;
                       });
}

void UserMessageModel::processNotifications(const mega::MegaNotificationList* notifications)
{
    int numNotifications = notifications ? notifications->size() : 0;
    if (numNotifications)
    {
        removeNotifications(notifications);
        insertNotifications(notifications);
    }
}

void UserMessageModel::insertNotifications(const mega::MegaNotificationList* notifications)
{
    QList<const mega::MegaNotification*> newNotifications;
    for (int i = 0; i < notifications->size(); i++)
    {
        const mega::MegaNotification* notification = notifications->get(i);
        auto it = findNotificationById(notification->getID());
        if (it == mUserMessages.end())
        {
            newNotifications.append(notification->copy());
        }
    }

    if(newNotifications.size() <= 0)
    {
        return;
    }

    beginInsertRows(QModelIndex(),
                    mUserMessages.size(),
                    mUserMessages.size() + newNotifications.size() - 1);
    for (auto& notification : newNotifications)
    {
        auto item = new UserNotification(notification);
        mUserMessages.push_back(item);

        if(!item->isSeen())
        {
            if(!mSeenStatusManager.markNotificationAsUnseen(item->getID()))
            {
                item->markAsSeen();
            }
        }
    }
    endInsertRows();
}

void UserMessageModel::removeNotifications(const mega::MegaNotificationList* notifications)
{
    for (int row = 0; row < mUserMessages.size(); ++row)
    {
        auto item = mUserMessages[row];
        if(item->getType() != UserMessage::Type::NOTIFICATION)
        {
            continue;
        }

        unsigned i = 0;
        bool found = false;
        auto notification = qobject_cast<UserNotification*>(item);
        auto id = notification->getID();
        while (i < notifications->size() && !found)
        {
            found = notifications->get(i)->getID() == id;
            ++i;
        }

        if (!found)
        {
            if(!notification->isSeen())
            {
                mSeenStatusManager.markAsSeen(MessageType::NOTIFICATIONS);
            }

            beginRemoveRows(QModelIndex(), row, row);
            delete mUserMessages[row];
            mUserMessages.removeAt(row);
            endRemoveRows();

            --row;
        }
    }
}

UnseenUserMessagesMap UserMessageModel::getUnseenNotifications() const
{
    return mSeenStatusManager.getUnseenUserMessages();
}

uint32_t UserMessageModel::checkLocalLastSeenNotification()
{
    for (auto& item : mUserMessages)
    {
        if(item->getType() != UserMessage::Type::NOTIFICATION)
        {
            continue;
        }

        auto currentLastSeen = mSeenStatusManager.getLastSeenNotification();
        auto notification = qobject_cast<UserNotification*>(item);
        auto id = notification->getID();
        if(!notification->isSeen() && id > currentLastSeen)
        {
            mSeenStatusManager.setLocalLastSeenNotification(id);
        }
    }
    return mSeenStatusManager.getLocalLastSeenNotification();
}

void UserMessageModel::setLastSeenNotification(uint32_t id)
{
    if(mSeenStatusManager.getLastSeenNotification() < id)
    {
        mSeenStatusManager.setLastSeenNotification(id);
        mSeenStatusManager.setLocalLastSeenNotification(id);
        for (auto& item : mUserMessages)
        {
            if(item->getType() != UserMessage::Type::NOTIFICATION)
            {
                continue;
            }

            auto notification = qobject_cast<UserNotification*>(item);
            if(notification->getID() <= id && !notification->isSeen())
            {
                notification->markAsSeen();
                mSeenStatusManager.markAsSeen(MessageType::NOTIFICATIONS);
            }
        }
    }
}

// ----------------------------------------------------------------------------
//      UserMessageSeenStatusManager
// ----------------------------------------------------------------------------

void UserMessageModel::SeenStatusManager::markAsUnseen(MessageType type)
{
    if(type == MessageType::UNKNOWN || type == MessageType::ALL)
    {
        return;
    }

    mUnseenNotifications[type]++;
    mUnseenNotifications[MessageType::ALL]++;
}

bool UserMessageModel::SeenStatusManager::markNotificationAsUnseen(uint32_t id)
{
    bool success = false;
    if(id > mLastSeenNotification)
    {
        markAsUnseen(MessageType::NOTIFICATIONS);
        success = true;
    }
    return success;
}

void UserMessageModel::SeenStatusManager::markAsSeen(MessageType type)
{
    if(type == MessageType::UNKNOWN || type == MessageType::ALL)
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

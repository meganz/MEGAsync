#ifndef USER_MESSAGE_MODEL_H
#define USER_MESSAGE_MODEL_H

#include "UserMessage.h"
#include "UserMessageTypes.h"

#include <QAbstractItemModel>

namespace mega
{
class MegaUserAlert;
class MegaUserAlertList;
class MegaNotificationList;
class MegaNotification;
}

class NotificationSeenStatusManager
{
public:
    NotificationSeenStatusManager() = default;
    virtual ~NotificationSeenStatusManager() = default;

    void markAsUnseen(MessageType type)
    {
        if(type == MessageType::UNKNOWN || type == MessageType::ALL)
        {
            return;
        }

        mUnseenNotifications[type]++;
        mUnseenNotifications[MessageType::ALL]++;
    }

    void markAsSeen(MessageType type)
    {
        if(type == MessageType::UNKNOWN || type == MessageType::ALL)
        {
            return;
        }

        mUnseenNotifications[type]--;
        mUnseenNotifications[MessageType::ALL]--;
    }

    UnseenUserMessagesMap getUnseenNotifications() const
    {
        return mUnseenNotifications;
    }

    void setLastSeenNotification(uint32_t id)
    {
        mLastSeenNotification = id;
    }

    uint32_t getLastSeenNotification() const
    {
        return mLastSeenNotification;
    }

    void setLocalLastSeenNotification(uint32_t id)
    {
        mLocalLastSeenNotification = id;
    }

    uint32_t getLocalLastSeenNotification() const
    {
        return mLocalLastSeenNotification;
    }

private:
    UnseenUserMessagesMap mUnseenNotifications;
    uint32_t mLastSeenNotification = 0;
    uint32_t mLocalLastSeenNotification = 0;

};

class UserMessageModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    using QAbstractItemModel::QAbstractItemModel;
    virtual ~UserMessageModel() = default;

    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    void processAlerts(mega::MegaUserAlertList* alerts);
    void processNotifications(const mega::MegaNotificationList* notifications);
    bool hasAlertsOfType(MessageType type);
    UnseenUserMessagesMap getUnseenNotifications() const;
    uint32_t checkLocalLastSeenNotification();
    void setLastSeenNotification(uint32_t id);

private:
    QList<UserMessage*> mNotifications;
    NotificationSeenStatusManager mSeenStatusManager;

    void insertAlerts(const QList<mega::MegaUserAlert*>& alerts);
    void updateAlerts(const QList<mega::MegaUserAlert*>& alerts);
    void removeAlerts(const QList<mega::MegaUserAlert*>& alerts);

    void insertNotifications(const mega::MegaNotificationList* notifications);
    void removeNotifications(const mega::MegaNotificationList* notifications);

    auto findAlertById(unsigned id);
    auto findNotificationById(int64_t id);

};

#endif // USER_MESSAGE_MODEL_H

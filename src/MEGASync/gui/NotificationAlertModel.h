#ifndef NOTIFICATION_ALERT_MODEL_H
#define NOTIFICATION_ALERT_MODEL_H

#include "NotificationExtBase.h"
#include "NotificationAlertTypes.h"

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

    void markAsUnseen(AlertType type)
    {
        if(type == AlertType::UNKNOWN || type == AlertType::ALL)
        {
            return;
        }

        mUnseenNotifications[type]++;
        mUnseenNotifications[AlertType::ALL]++;
    }

    void markAsSeen(AlertType type)
    {
        if(type == AlertType::UNKNOWN || type == AlertType::ALL)
        {
            return;
        }

        mUnseenNotifications[type]--;
        mUnseenNotifications[AlertType::ALL]--;
    }

    UnseenNotificationsMap getUnseenNotifications() const
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
    UnseenNotificationsMap mUnseenNotifications;
    uint32_t mLastSeenNotification = 0;
    uint32_t mLocalLastSeenNotification = 0;

};

class NotificationAlertModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    using QAbstractItemModel::QAbstractItemModel;
    virtual ~NotificationAlertModel() = default;

    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    void processAlerts(mega::MegaUserAlertList* alerts);
    void processNotifications(const mega::MegaNotificationList* notifications);
    bool hasAlertsOfType(AlertType type);
    UnseenNotificationsMap getUnseenNotifications() const;
    uint32_t checkLocalLastSeenNotification();
    void setLastSeenNotification(uint32_t id);

private:
    QList<NotificationExtBase*> mNotifications;
    NotificationSeenStatusManager mSeenStatusManager;

    void insertAlerts(const QList<mega::MegaUserAlert*>& alerts);
    void updateAlerts(const QList<mega::MegaUserAlert*>& alerts);
    void removeAlerts(const QList<mega::MegaUserAlert*>& alerts);

    void insertNotifications(const mega::MegaNotificationList* notifications);
    void removeNotifications(const mega::MegaNotificationList* notifications);

    auto findAlertById(unsigned id);
    auto findNotificationById(int64_t id);

};

#endif // NOTIFICATION_ALERT_MODEL_H

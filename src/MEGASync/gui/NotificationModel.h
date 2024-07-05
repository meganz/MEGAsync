#ifndef NOTIFICATION_MODEL_H
#define NOTIFICATION_MODEL_H

#include "NotificationItem.h"
#include "MegaNotificationExt.h"

#include <QAbstractItemModel>
#include <QCache>

#include <deque>

class NotificationModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit NotificationModel(QObject* parent = 0);
    virtual ~NotificationModel();

    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    void insert(const mega::MegaNotificationList* notifications);
    long long getNumUnseenNotifications() const;
    uint32_t getLastSeenNotification() const;
    void setLastSeenNotification(uint32_t id);

    QCache<int, NotificationItem> notificationItems;

private:
    QMap<int, MegaNotificationExt*> mNotificationsMap;
    std::deque<unsigned int> mNotificationsOrder;
    uint32_t mLastSeenNotification;

    int countNewNotifications(const mega::MegaNotificationList* notifications) const;
    void insertNewNotifications(const mega::MegaNotificationList* notifications);

    void updateNotifications(const mega::MegaNotificationList* notifications);

    void removeNotifications(const mega::MegaNotificationList* notifications);
    QSet<int> createNotificationIDSet(const mega::MegaNotificationList* notifications) const;
    QList<int> findRowsToRemove(const QSet<int>& notificationIDsInList) const;

};

#endif // NOTIFICATION_MODEL_H

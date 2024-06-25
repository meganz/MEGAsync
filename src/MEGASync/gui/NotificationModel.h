#ifndef QNOTIFICATIONSMODEL_H
#define QNOTIFICATIONSMODEL_H

#include "NotificationItem.h"
#include "MegaNotificationExt.h"

#include <QAbstractItemModel>
#include <QCache>

#include <deque>

class NotificationModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit NotificationModel(const mega::MegaNotificationList* notifications, QObject* parent = 0);
    virtual ~NotificationModel();

    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    void insert(const mega::MegaNotificationList* notifications);

    QCache<int, NotificationItem> notificationItems;

private:
    QMap<int, MegaNotificationExt*> mNotificationsMap;
    std::deque<unsigned int> mNotificationsOrder;

};

#endif // QNOTIFICATIONSMODEL_H

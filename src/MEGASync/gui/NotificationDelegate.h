#ifndef NOTIFICATION_DELEGATE_H
#define NOTIFICATION_DELEGATE_H

#include "NotificationModel.h"
#include "NotificationAlertTypes.h"

#include <QStyledItemDelegate>

class NotificationDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    NotificationDelegate(NotificationModel* notificationModel, QObject* parent = 0);

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;

private:
    NotificationModel* mNotificationModel;

    void handleNotificationItem(MegaNotificationExt* notification, const QRect& rect, QPainter* painter) const;
    NotificationAlertModelItem* getModelItem(const QModelIndex& index) const;

};

#endif // NOTIFICATION_DELEGATE_H

#include "MegaNotificationDelegate.h"

#include "MegaApplication.h"

#include <QPainter>
#include <QEvent>
#include <QAbstractItemModel>

MegaNotificationDelegate::MegaNotificationDelegate(NotificationModel* notificationModel, QObject *parent)
    : QStyledItemDelegate(parent), mNotificationModel(notificationModel)
{
}

void MegaNotificationDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.isValid())
    {
        QModelIndex filteredIndex = ((QSortFilterProxyModel*)index.model())->mapToSource(index);
        AlertNotificationModelItem* item = static_cast<AlertNotificationModelItem*>(filteredIndex.internalPointer());
        if (item->type != AlertNotificationModelItem::NOTIFICATION || !item->pointer)
        {
            QStyledItemDelegate::paint(painter, option, index);
            return;
        }

        NotifTest* notification = static_cast<NotifTest*>(item->pointer);

        if (!notification)
        {
            assert(false || "No notif found");
            QStyledItemDelegate::paint(painter, option, index);
            return;
        }

        NotificationItem* notifItem = mNotificationModel->notificationItems[notification->id];
        if (!notifItem)
        {
            notifItem = new NotificationItem();
            mNotificationModel->notificationItems.insert(notification->id, notifItem);
            notifItem->setNotificationData(notification);
        }

        painter->save();
        painter->translate(option.rect.topLeft());

        notifItem->resize(option.rect.width(), option.rect.height());

        notifItem->render(painter, QPoint(0, 0), QRegion(0, 0, option.rect.width(), option.rect.height()));
        painter->restore();
    }
    else
    {
        QStyledItemDelegate::paint(painter, option, index);
    }
}

QSize MegaNotificationDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!index.isValid())
    {
        return QStyledItemDelegate::sizeHint(option, index);
    }

    QModelIndex filteredIndex = ((QSortFilterProxyModel*)index.model())->mapToSource(index);
    AlertNotificationModelItem* item = static_cast<AlertNotificationModelItem*>(filteredIndex.internalPointer());
    if (item->type != AlertNotificationModelItem::NOTIFICATION || !item->pointer)
    {
        return QStyledItemDelegate::sizeHint(option, index);
    }

    NotifTest* notif = static_cast<NotifTest*>(item->pointer);
    int height = notif->imageName.empty() ? 219 : 346;

    return QSize(400, height);
}

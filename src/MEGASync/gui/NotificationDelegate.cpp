#include "NotificationDelegate.h"

#include "NotificationAlertModel.h"
#include "NotificationAlertTypes.h"

#include <QPainter>
#include <QSortFilterProxyModel>

namespace
{
constexpr int DEFAULT_WIDTH = 400;
constexpr int HEIGHT_WITHOUT_IMAGE = 219;
constexpr int HEIGHT_WITH_IMAGE = 346;
}

NotificationDelegate::NotificationDelegate(NotificationModel* notificationModel, QObject* parent)
    : QStyledItemDelegate(parent)
    , mNotificationModel(notificationModel)
{
}

void NotificationDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    if (index.isValid())
    {
        QModelIndex filteredIndex = ((QSortFilterProxyModel*)index.model())->mapToSource(index);
        NotificationAlertModelItem* item = static_cast<NotificationAlertModelItem*>(filteredIndex.internalPointer());
        if (item->type != NotificationAlertModelItem::NOTIFICATION || !item->pointer)
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

QSize NotificationDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    if (!index.isValid())
    {
        return QStyledItemDelegate::sizeHint(option, index);
    }

    QModelIndex filteredIndex = ((QSortFilterProxyModel*)index.model())->mapToSource(index);
    NotificationAlertModelItem* item = static_cast<NotificationAlertModelItem*>(filteredIndex.internalPointer());
    if (item->type != NotificationAlertModelItem::NOTIFICATION || !item->pointer)
    {
        return QStyledItemDelegate::sizeHint(option, index);
    }

    NotifTest* notif = static_cast<NotifTest*>(item->pointer);
    return QSize(DEFAULT_WIDTH, notif->imageName.empty() ? HEIGHT_WITHOUT_IMAGE : HEIGHT_WITH_IMAGE);
}

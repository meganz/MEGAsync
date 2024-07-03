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
    installEventFilter(this);
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

        MegaNotificationExt* notification = static_cast<MegaNotificationExt*>(item->pointer);

        if (!notification)
        {
            assert(false || "No notif found");
            QStyledItemDelegate::paint(painter, option, index);
            return;
        }

        NotificationItem* notificationItem = mNotificationModel->notificationItems[notification->getID()];
        if (!notificationItem)
        {
            notificationItem = new NotificationItem();
            mNotificationModel->notificationItems.insert(notification->getID(), notificationItem);
            notificationItem->setNotificationData(notification);
        }

        painter->save();
        painter->translate(option.rect.topLeft());
        notificationItem->resize(option.rect.width(), option.rect.height());
        notificationItem->render(painter, QPoint(0, 0), QRegion(0, 0, option.rect.width(), option.rect.height()));
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

    MegaNotificationExt* notification = static_cast<MegaNotificationExt*>(item->pointer);
    return QSize(DEFAULT_WIDTH, notification->showImage() ? HEIGHT_WITH_IMAGE : HEIGHT_WITHOUT_IMAGE);
}

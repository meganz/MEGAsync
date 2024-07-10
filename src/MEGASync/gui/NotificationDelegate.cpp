#include "NotificationDelegate.h"

#include "NotificationAlertModel.h"

#include <QPainter>
#include <QSortFilterProxyModel>

namespace
{
constexpr int DefaultWidth = 400;
constexpr int HeightWithoutImage = 219;
constexpr int HeightWithImage = 346;
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
        /*
        NotificationAlertModelItem* item = getModelItem(index);
        if (!item)
        {
            QStyledItemDelegate::paint(painter, option, index);
            return;
        }*/

        MegaNotificationExt* notification = nullptr;//static_cast<MegaNotificationExt*>(item->pointer);
        if (!notification)
        {
            assert(false || "No notif found");
            QStyledItemDelegate::paint(painter, option, index);
            return;
        }

        painter->save();
        painter->translate(option.rect.topLeft());

        //handleNotificationItem(notification, option.rect, painter);

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

    /*NotificationAlertModelItem* item = getModelItem(index);
    if (!item)
    {
        return QStyledItemDelegate::sizeHint(option, index);
    }*/

    MegaNotificationExt* notification = static_cast<MegaNotificationExt*>(nullptr/*item->pointer*/);
    return QSize(DefaultWidth, notification->showImage() ? HeightWithImage : HeightWithoutImage);
}
/*
void NotificationDelegate::handleNotificationItem(MegaNotificationExt* notification, const QRect& rect, QPainter* painter) const
{
    int id = notification->getID();
    NotificationItem* notificationItem = mNotificationModel->notificationItems[id];
    bool isNew = !notificationItem;
    if (isNew)
    {
        notificationItem = new NotificationItem();
        notificationItem->setNotificationData(notification);
    }

    notificationItem->resize(rect.width(), rect.height());
    notificationItem->render(painter, QPoint(0, 0), QRegion(0, 0, rect.width(), rect.height()));

    if(isNew)
    {
        mNotificationModel->notificationItems.insert(id, notificationItem);
    }
}

NotificationAlertModelItem* NotificationDelegate::getModelItem(const QModelIndex &index) const
{
    NotificationAlertModelItem* item = nullptr;
    const QSortFilterProxyModel* proxyModel = static_cast<const QSortFilterProxyModel*>(index.model());
    QModelIndex filteredIndex = proxyModel->mapToSource(index);
    if (filteredIndex.isValid())
    {
        item = static_cast<NotificationAlertModelItem*>(filteredIndex.internalPointer());
        if (item && (item->type != NotificationAlertModelItem::NOTIFICATION || !item->pointer))
        {
            item = nullptr;
        }
    }
    return item;
}
*/

#include "NotificationAlertDelegate.h"

#include "NotificationAlertModel.h"

NotificationAlertDelegate::NotificationAlertDelegate(MegaNotificationDelegate* notificationsDelegate,
                                                     AlertDelegate* alertsDelegate,
                                                     QObject* parent)
    : QStyledItemDelegate(parent)
    , mNotificationsDelegate(notificationsDelegate)
    , mAlertsDelegate(alertsDelegate)
{
}

void NotificationAlertDelegate::paint(QPainter* painter,
                                      const QStyleOptionViewItem& option,
                                      const QModelIndex& index) const
{
    QModelIndex filteredIndex = ((QSortFilterProxyModel*)index.model())->mapToSource(index);
    AlertNotificationModelItem* item = static_cast<AlertNotificationModelItem*>(filteredIndex.internalPointer());
    switch (item->type)
    {
        case AlertNotificationModelItem::ALERT:
        {
            mAlertsDelegate->paint(painter, option, index);
            break;
        }
        case AlertNotificationModelItem::NOTIFICATION:
        {
            mNotificationsDelegate->paint(painter, option, index);
            break;
        }
        default:
        {
            mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_ERROR, "Invalid notification item type in delegate (paint).");
            break;
        }
    }
}

QSize NotificationAlertDelegate::sizeHint(const QStyleOptionViewItem& option,
                                          const QModelIndex& index) const
{
    QModelIndex filteredIndex = ((QSortFilterProxyModel*)index.model())->mapToSource(index);
    AlertNotificationModelItem* item = static_cast<AlertNotificationModelItem*>(filteredIndex.internalPointer());
    auto result = QStyledItemDelegate::sizeHint(option, index);
    switch (item->type)
    {
        case AlertNotificationModelItem::ALERT:
        {
            result = mAlertsDelegate->sizeHint(option, index);
            break;
        }
        case AlertNotificationModelItem::NOTIFICATION:
        {
            result = mNotificationsDelegate->sizeHint(option, index);
            break;
        }
        default:
        {
            mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_ERROR, "Invalid notification item type in delegate (sizeHint).");
            break;
        }
    }
    return result;
}

bool NotificationAlertDelegate::editorEvent(QEvent* event,
                                            QAbstractItemModel* model,
                                            const QStyleOptionViewItem& option,
                                            const QModelIndex& index)
{
    QModelIndex filteredIndex = ((QSortFilterProxyModel*)index.model())->mapToSource(index);
    AlertNotificationModelItem* item = static_cast<AlertNotificationModelItem*>(filteredIndex.internalPointer());
    auto result = QStyledItemDelegate::editorEvent(event, model, option, index);
    switch (item->type)
    {
        case AlertNotificationModelItem::ALERT:
        {
            result = mAlertsDelegate->editorEvent(event, model, option, index);
            break;
        }
        case AlertNotificationModelItem::NOTIFICATION:
        {
            break;
        }
        default:
        {
            mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_ERROR, "Invalid notification item type in delegate (editorEvent).");
            break;
        }
    }
    return result;
}

bool NotificationAlertDelegate::helpEvent(QHelpEvent* event,
                                          QAbstractItemView* view,
                                          const QStyleOptionViewItem& option,
                                          const QModelIndex& index)
{
    QModelIndex filteredIndex = ((QSortFilterProxyModel*)index.model())->mapToSource(index);
    AlertNotificationModelItem* item = static_cast<AlertNotificationModelItem*>(filteredIndex.internalPointer());
    auto result = QStyledItemDelegate::helpEvent(event, view, option, index);
    switch (item->type)
    {
        case AlertNotificationModelItem::ALERT:
        {
            result = mAlertsDelegate->helpEvent(event, view, option, index);
            break;
        }
        case AlertNotificationModelItem::NOTIFICATION:
        {
            break;
        }
        default:
        {
            mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_ERROR, "Invalid notification item type in delegate (helpEvent).");
            break;
        }
    }
    return result;
}

#include "NotificationAlertDelegate.h"

#include "NotificationAlertModel.h"

#include <QSortFilterProxyModel>

void NotificationAlertDelegate::paint(QPainter* painter,
                                      const QStyleOptionViewItem& option,
                                      const QModelIndex& index) const
{
    switch (getModelType(index))
    {
        case NotificationAlertModelItem::ALERT:
        {
            mAlertsDelegate->paint(painter, option, index);
            break;
        }
        case NotificationAlertModelItem::NOTIFICATION:
        {
            mNotificationsDelegate->paint(painter, option, index);
            break;
        }
        case NotificationAlertModelItem::NONE:
        default:
        {
            mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_ERROR,
                               "Invalid notification item type in delegate (paint).");
            break;
        }
    }
}

QSize NotificationAlertDelegate::sizeHint(const QStyleOptionViewItem& option,
                                          const QModelIndex& index) const
{
    QSize result;
    switch (getModelType(index))
    {
        case NotificationAlertModelItem::ALERT:
        {
            result = mAlertsDelegate->sizeHint(option, index);
            break;
        }
        case NotificationAlertModelItem::NOTIFICATION:
        {
            result = mNotificationsDelegate->sizeHint(option, index);
            break;
        }
        case NotificationAlertModelItem::NONE:
        default:
        {
            result = QStyledItemDelegate::sizeHint(option, index);
            mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_ERROR,
                               "Invalid notification item type in delegate (sizeHint).");
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
    bool result = false;
    switch (getModelType(index))
    {
        case NotificationAlertModelItem::ALERT:
        {
            result = mAlertsDelegate->editorEvent(event, model, option, index);
            break;
        }
        case NotificationAlertModelItem::NOTIFICATION:
        {
            result = QStyledItemDelegate::editorEvent(event, model, option, index);
            break;
        }
        case NotificationAlertModelItem::NONE:
        default:
        {
            result = QStyledItemDelegate::editorEvent(event, model, option, index);
            mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_ERROR,
                               "Invalid notification item type in delegate (editorEvent).");
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
    bool result = false;
    switch (getModelType(index))
    {
        case NotificationAlertModelItem::ALERT:
        {
            result = mAlertsDelegate->helpEvent(event, view, option, index);
            break;
        }
        case NotificationAlertModelItem::NOTIFICATION:
        {
            result = QStyledItemDelegate::helpEvent(event, view, option, index);
            break;
        }
        case NotificationAlertModelItem::NONE:
        default:
        {
            result = QStyledItemDelegate::helpEvent(event, view, option, index);
            mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_ERROR,
                               "Invalid notification item type in delegate (helpEvent).");
            break;
        }
    }
    return result;
}

void NotificationAlertDelegate::createNotificationDelegate(NotificationModel* model)
{
    if(!mNotificationsDelegate)
    {
        mNotificationsDelegate = std::make_unique<NotificationDelegate>(model, this);
    }
}

void NotificationAlertDelegate::createAlertDelegate(AlertModel* model)
{
    if(!mAlertsDelegate)
    {
        mAlertsDelegate = std::make_unique<AlertDelegate>(model, this);
    }
}

NotificationAlertModelItem::ModelType NotificationAlertDelegate::getModelType(const QModelIndex &index) const
{
    NotificationAlertModelItem::ModelType type = NotificationAlertModelItem::NONE;
    if (index.isValid() && index.row() >= 0)
    {
        QModelIndex filteredIndex = ((QSortFilterProxyModel*)index.model())->mapToSource(index);
        if (filteredIndex.isValid() && filteredIndex.row() >= 0)
        {
            NotificationAlertModelItem* item = static_cast<NotificationAlertModelItem*>(filteredIndex.internalPointer());
            if(item)
            {
                type = item->type;
            }
        }
    }
    return type;
}

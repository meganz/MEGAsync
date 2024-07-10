#include "NotificationAlertDelegate.h"

#include "NotificationAlertModel.h"

#include <QSortFilterProxyModel>
#include <QPainter>

NotificationAlertDelegate::NotificationAlertDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
    , mAlertsDelegate(std::make_unique<AlertDelegate>())
{
}

void NotificationAlertDelegate::paint(QPainter* painter,
                                      const QStyleOptionViewItem& option,
                                      const QModelIndex& index) const
{
    if (index.isValid())
    {
        painter->save();
        painter->translate(option.rect.topLeft());

        QWidget* item = getWidget(index);
        if(!item)
        {
            return;
        }

        item->resize(option.rect.width(), option.rect.height());
        item->render(painter, QPoint(0, 0), QRegion(0, 0, option.rect.width(), option.rect.height()));

        painter->restore();
    }
    else
    {
        QStyledItemDelegate::paint(painter, option, index);
    }
}

QSize NotificationAlertDelegate::sizeHint(const QStyleOptionViewItem& option,
                                          const QModelIndex& index) const
{
    QSize result;
    if (index.isValid())
    {
        QWidget* item = getWidget(index);
        if(!item)
        {
            return QSize();
        }
        result = item->sizeHint();
    }
    else
    {
        result = QStyledItemDelegate::sizeHint(option, index);
    }
    return result;
}

QWidget* NotificationAlertDelegate::getWidget(const QModelIndex& index) const
{
    QWidget* widget = nullptr;
    if (index.isValid() && index.row() >= 0)
    {
        QModelIndex filteredIndex = ((QSortFilterProxyModel*)index.model())->mapToSource(index);
        if (filteredIndex.isValid() && filteredIndex.row() >= 0)
        {
            NotificationExtBase* item = static_cast<NotificationExtBase*>(filteredIndex.internalPointer());
            if(item)
            {
                switch (item->getType())
                {
                case NotificationExtBase::Type::ALERT:
                {
                    MegaUserAlertExt* alert = dynamic_cast<MegaUserAlertExt*>(item);
                    widget = mAlertsDelegate->getWidget(alert);
                    break;
                }
                default:
                {
                    break;
                }
                }
            }
        }
    }
    return widget;
}

/*
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
        mNotificationsDelegate = std::make_unique<NotificationDelegate>(model);
    }
}

void NotificationAlertDelegate::createAlertDelegate(AlertModel* model)
{
    if(!mAlertsDelegate)
    {
        mAlertsDelegate = std::make_unique<AlertDelegate>(model);
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
*/



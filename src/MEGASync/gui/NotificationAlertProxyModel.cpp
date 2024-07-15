#include "NotificationAlertProxyModel.h"

#include "UserAlert.h"

#include "megaapi.h"

#include <QDateTime>

NotificationAlertProxyModel::NotificationAlertProxyModel(QObject* parent)
    : QSortFilterProxyModel(parent)
    , mActualFilter(AlertType::ALL)
{
}

AlertType NotificationAlertProxyModel::filterAlertType()
{
    return mActualFilter;
}

void NotificationAlertProxyModel::setFilterAlertType(AlertType filterType)
{
    mActualFilter = filterType;
    invalidateFilter();
}

bool NotificationAlertProxyModel::filterAcceptsRow(int row, const QModelIndex& sourceParent) const
{
    if(mActualFilter == AlertType::ALL)
    {
        return true;
    }

    bool filter = false;
    QModelIndex index = sourceModel()->index(row, 0, sourceParent);
    UserMessage* item = static_cast<UserMessage*>(index.internalPointer());
    if(item)
    {
        switch (item->getType())
        {
            case UserMessage::Type::ALERT:
            {
                UserAlert* alert = qobject_cast<UserAlert*>(item);
                filter = mActualFilter == alert->getAlertType();
                break;
            }
            case UserMessage::Type::NOTIFICATION:
            {
                filter = mActualFilter == AlertType::ALL;
                break;
            }
            default:
            {
                mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_ERROR,
                                   "Invalid notification item type (filterAcceptsRow).");
                break;
            }
        }
    }
    return filter;
}

bool NotificationAlertProxyModel::lessThan(const QModelIndex& left, const QModelIndex& right) const
{
    UserMessage* leftItem = static_cast<UserMessage*>(left.internalPointer());
    UserMessage* rightItem = static_cast<UserMessage*>(right.internalPointer());

    if(!leftItem || !rightItem)
    {
        mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_ERROR, "Invalid items (lessThan).");
        return false;
    }

    bool isLess;
    // If the types are different, prioritise notifications over alerts
    if (leftItem->getType() == UserMessage::Type::NOTIFICATION
        && rightItem->getType() == UserMessage::Type::ALERT)
    {
        isLess = true;
    }
    else if (leftItem->getType() == UserMessage::Type::ALERT
               && rightItem->getType() == UserMessage::Type::NOTIFICATION)
    {
        isLess = false;
    }
    else if (leftItem->getType() == UserMessage::Type::ALERT
               && rightItem->getType() == UserMessage::Type::ALERT)
    {
        // If both are of type alert, order by date
        QDateTime leftDate = sourceModel()->data(left, Qt::UserRole).toDateTime();
        QDateTime rightDate = sourceModel()->data(right, Qt::UserRole).toDateTime();
        isLess = leftDate > rightDate;
    }
    else
    {
        isLess = QSortFilterProxyModel::lessThan(left, right);
    }
    return isLess;
}

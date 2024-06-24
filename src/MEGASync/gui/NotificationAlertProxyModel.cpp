#include "NotificationAlertProxyModel.h"

#include "MegaUserAlertExt.h"

#include "megaapi.h"

#include <QDateTime>

namespace
{
const std::map<AlertType, std::vector<int>> MEGAUSER_ALERTS_BY_TYPE
{
    {
        AlertType::CONTACTS,
        {
            mega::MegaUserAlert::TYPE_INCOMINGPENDINGCONTACT_REQUEST,
            mega::MegaUserAlert::TYPE_INCOMINGPENDINGCONTACT_CANCELLED,
            mega::MegaUserAlert::TYPE_INCOMINGPENDINGCONTACT_REMINDER,
            mega::MegaUserAlert::TYPE_CONTACTCHANGE_DELETEDYOU,
            mega::MegaUserAlert::TYPE_CONTACTCHANGE_CONTACTESTABLISHED,
            mega::MegaUserAlert::TYPE_CONTACTCHANGE_ACCOUNTDELETED,
            mega::MegaUserAlert::TYPE_CONTACTCHANGE_BLOCKEDYOU,
            mega::MegaUserAlert::TYPE_UPDATEDPENDINGCONTACTINCOMING_IGNORED,
            mega::MegaUserAlert::TYPE_UPDATEDPENDINGCONTACTINCOMING_ACCEPTED,
            mega::MegaUserAlert::TYPE_UPDATEDPENDINGCONTACTINCOMING_DENIED,
            mega::MegaUserAlert::TYPE_UPDATEDPENDINGCONTACTOUTGOING_ACCEPTED,
            mega::MegaUserAlert::TYPE_UPDATEDPENDINGCONTACTOUTGOING_DENIED
        }
    },
    {
        AlertType::SHARES,
        {
            mega::MegaUserAlert::TYPE_NEWSHARE,
            mega::MegaUserAlert::TYPE_DELETEDSHARE,
            mega::MegaUserAlert::TYPE_NEWSHAREDNODES,
            mega::MegaUserAlert::TYPE_REMOVEDSHAREDNODES,
            mega::MegaUserAlert::TYPE_UPDATEDSHAREDNODES
        }
    },
    {
        AlertType::PAYMENTS,
        {
            mega::MegaUserAlert::TYPE_PAYMENT_SUCCEEDED,
            mega::MegaUserAlert::TYPE_PAYMENT_FAILED,
            mega::MegaUserAlert::TYPE_PAYMENTREMINDER
        }
    },
    {
        AlertType::TAKEDOWNS,
        {
            mega::MegaUserAlert::TYPE_TAKEDOWN,
            mega::MegaUserAlert::TYPE_TAKEDOWN_REINSTATED
        }
    }
};
}

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

bool NotificationAlertProxyModel::checkFilterType(int sdkType) const
{
    bool success = false;
    if(mActualFilter == AlertType::ALL)
    {
        success = true;
    }
    else
    {
        auto it = MEGAUSER_ALERTS_BY_TYPE.find(mActualFilter);
        if (it != MEGAUSER_ALERTS_BY_TYPE.end())
        {
            success = std::find(it->second.begin(), it->second.end(), sdkType) != it->second.end();
        }
    }
    return success;
}

bool NotificationAlertProxyModel::filterAcceptsRow(int row, const QModelIndex& sourceParent) const
{
    bool filter = false;
    QModelIndex index = sourceModel()->index(row, 0, sourceParent);
    NotificationAlertModelItem* item = static_cast<NotificationAlertModelItem*>(index.internalPointer());
    if(item)
    {
        switch (item->type)
        {
            case NotificationAlertModelItem::ALERT:
            {
                MegaUserAlertExt* alert = static_cast<MegaUserAlertExt*>(item->pointer);
                filter = checkFilterType(alert->getType());
                break;
            }
            case NotificationAlertModelItem::NOTIFICATION:
            {
                filter = true;
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
    NotificationAlertModelItem* leftItem = static_cast<NotificationAlertModelItem*>(left.internalPointer());
    NotificationAlertModelItem* rightItem = static_cast<NotificationAlertModelItem*>(right.internalPointer());

    bool isLess;
    // If the types are different, prioritise notifications over alerts
    if (leftItem->type == NotificationAlertModelItem::NOTIFICATION
        && rightItem->type == NotificationAlertModelItem::ALERT)
    {
        isLess = true;
    }
    else if (leftItem->type == NotificationAlertModelItem::ALERT
               && rightItem->type == NotificationAlertModelItem::NOTIFICATION)
    {
        isLess = false;
    }
    else if (leftItem->type == NotificationAlertModelItem::ALERT
               && rightItem->type == NotificationAlertModelItem::ALERT)
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

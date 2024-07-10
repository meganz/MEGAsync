#include "NotificationAlertProxyModel.h"

#include "MegaUserAlertExt.h"

#include "megaapi.h"

#include <QDateTime>

namespace
{
const std::map<AlertType, std::vector<int>> MegaUserAlertsByType
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

bool NotificationAlertProxyModel::checkAlertFilterType(int sdkType) const
{
    bool success = false;
    if(mActualFilter == AlertType::ALL)
    {
        success = true;
    }
    else
    {
        auto it = MegaUserAlertsByType.find(mActualFilter);
        if (it != MegaUserAlertsByType.end())
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
    NotificationExtBase* item = static_cast<NotificationExtBase*>(index.internalPointer());
    if(item)
    {
        switch (item->getType())
        {
            case NotificationExtBase::Type::ALERT:
            {
                MegaUserAlertExt* alert = static_cast<MegaUserAlertExt*>(item);
                filter = checkAlertFilterType(alert->getType());
                break;
            }
            case NotificationExtBase::Type::NOTIFICATION:
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
    NotificationExtBase* leftItem = static_cast<NotificationExtBase*>(left.internalPointer());
    NotificationExtBase* rightItem = static_cast<NotificationExtBase*>(right.internalPointer());

    if(!leftItem || !rightItem)
    {
        mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_ERROR, "Invalid items (lessThan).");
        return false;
    }

    bool isLess;
    // If the types are different, prioritise notifications over alerts
    if (leftItem->getType() == NotificationExtBase::Type::NOTIFICATION
        && rightItem->getType() == NotificationExtBase::Type::ALERT)
    {
        isLess = true;
    }
    else if (leftItem->getType() == NotificationExtBase::Type::ALERT
               && rightItem->getType() == NotificationExtBase::Type::NOTIFICATION)
    {
        isLess = false;
    }
    else if (leftItem->getType() == NotificationExtBase::Type::ALERT
               && rightItem->getType() == NotificationExtBase::Type::ALERT)
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

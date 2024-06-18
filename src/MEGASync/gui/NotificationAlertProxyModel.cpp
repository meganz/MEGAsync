#include "NotificationAlertProxyModel.h"

#include "NotificationAlertModel.h"

#include <QDateTime>

using namespace mega;

NotificationAlertProxyModel::NotificationAlertProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
    , actualFilter(FilterType::ALL)
{
}

NotificationAlertProxyModel::FilterType NotificationAlertProxyModel::filterAlertType()
{
    return actualFilter;
}

void NotificationAlertProxyModel::setFilterAlertType(FilterType filterType)
{
    actualFilter = filterType;
    invalidateFilter();
}

bool NotificationAlertProxyModel::checkFilterType(int sdkType) const
{
    bool success = false;
    if (actualFilter == FilterType::ALL)
    {
        success = true;
    }
    else
    {
        switch (sdkType)
        {
            case MegaUserAlert::TYPE_INCOMINGPENDINGCONTACT_REQUEST:
            case MegaUserAlert::TYPE_INCOMINGPENDINGCONTACT_CANCELLED:
            case MegaUserAlert::TYPE_INCOMINGPENDINGCONTACT_REMINDER:
            case MegaUserAlert::TYPE_CONTACTCHANGE_DELETEDYOU:
            case MegaUserAlert::TYPE_CONTACTCHANGE_CONTACTESTABLISHED:
            case MegaUserAlert::TYPE_CONTACTCHANGE_ACCOUNTDELETED:
            case MegaUserAlert::TYPE_CONTACTCHANGE_BLOCKEDYOU:
            case MegaUserAlert::TYPE_UPDATEDPENDINGCONTACTINCOMING_IGNORED:
            case MegaUserAlert::TYPE_UPDATEDPENDINGCONTACTINCOMING_ACCEPTED:
            case MegaUserAlert::TYPE_UPDATEDPENDINGCONTACTINCOMING_DENIED:
            case MegaUserAlert::TYPE_UPDATEDPENDINGCONTACTOUTGOING_ACCEPTED:
            case MegaUserAlert::TYPE_UPDATEDPENDINGCONTACTOUTGOING_DENIED:
            {
                success = actualFilter == FilterType::CONTACTS;
                break;
            }
            case MegaUserAlert::TYPE_NEWSHARE:
            case MegaUserAlert::TYPE_DELETEDSHARE:
            case MegaUserAlert::TYPE_NEWSHAREDNODES:
            case MegaUserAlert::TYPE_REMOVEDSHAREDNODES:
            case MegaUserAlert::TYPE_UPDATEDSHAREDNODES:
            {
                success = actualFilter == FilterType::SHARES;
                break;
            }
            case MegaUserAlert::TYPE_PAYMENT_SUCCEEDED:
            case MegaUserAlert::TYPE_PAYMENT_FAILED:
            case MegaUserAlert::TYPE_PAYMENTREMINDER:
            {
                success = actualFilter == FilterType::PAYMENTS;
                break;
            }
            case MegaUserAlert::TYPE_TAKEDOWN:
            case MegaUserAlert::TYPE_TAKEDOWN_REINSTATED:
            {
                success = actualFilter == FilterType::TAKEDOWNS;
                break;
            }
            default:
            {
                break;
            }
        }
    }

    return success;
}

bool NotificationAlertProxyModel::filterAcceptsRow(int row, const QModelIndex &sourceParent) const
{
    bool filter = false;
    QModelIndex index = sourceModel()->index(row, 0, sourceParent);
    AlertNotificationModelItem* item = static_cast<AlertNotificationModelItem*>(index.internalPointer());
    if(item)
    {
        switch (item->type)
        {
            case AlertNotificationModelItem::ALERT:
            {
                MegaUserAlertExt* alert = static_cast<MegaUserAlertExt*>(item->pointer);
                filter = checkFilterType(alert->getType());
                break;
            }
            case AlertNotificationModelItem::NOTIFICATION:
            {
                filter = true;
                break;
            }
            default:
            {
                MegaApi::log(MegaApi::LOG_LEVEL_ERROR, "Invalid notification item type (filterAcceptsRow).");
                break;
            }
        }
    }
    return filter;
}

bool NotificationAlertProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    AlertNotificationModelItem* leftItem = static_cast<AlertNotificationModelItem*>(left.internalPointer());
    AlertNotificationModelItem* rightItem = static_cast<AlertNotificationModelItem*>(right.internalPointer());

    bool isLess;
    // If the types are different, prioritise notifications over alerts
    if (leftItem->type == AlertNotificationModelItem::NOTIFICATION && rightItem->type == AlertNotificationModelItem::ALERT)
    {
        isLess = true;
    }
    else if (leftItem->type == AlertNotificationModelItem::ALERT && rightItem->type == AlertNotificationModelItem::NOTIFICATION)
    {
        isLess = false;
    }
    else if (leftItem->type == AlertNotificationModelItem::ALERT && rightItem->type == AlertNotificationModelItem::ALERT)
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

#include "QFilterAlertsModel.h"

#include "NotificationAlertModel.h"

using namespace mega;

QFilterAlertsModel::QFilterAlertsModel(QObject *parent)
    :  QSortFilterProxyModel(parent)
{
    actualFilter = NO_FILTER;
}

QFilterAlertsModel::~QFilterAlertsModel()
{

}

int QFilterAlertsModel::filterAlertType()
{
    return actualFilter;
}

void QFilterAlertsModel::setFilterAlertType(int filterType)
{
    actualFilter = filterType;
    invalidateFilter();
}

bool QFilterAlertsModel::checkFilterType(int typeToCheck) const
{
    if (actualFilter == NO_FILTER)
    {
        return true;
    }

    switch (typeToCheck)
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
                return actualFilter == FILTER_CONTACTS;
                break;

            case MegaUserAlert::TYPE_NEWSHARE:
            case MegaUserAlert::TYPE_DELETEDSHARE:
            case MegaUserAlert::TYPE_NEWSHAREDNODES:
            case MegaUserAlert::TYPE_REMOVEDSHAREDNODES:
            case MegaUserAlert::TYPE_UPDATEDSHAREDNODES:
                return actualFilter == FILTER_SHARES;
                break;

            case MegaUserAlert::TYPE_PAYMENT_SUCCEEDED:
            case MegaUserAlert::TYPE_PAYMENT_FAILED:
            case MegaUserAlert::TYPE_PAYMENTREMINDER:
                return actualFilter == FILTER_PAYMENT;
                break;

            case MegaUserAlert::TYPE_TAKEDOWN:
            case MegaUserAlert::TYPE_TAKEDOWN_REINSTATED:
                return actualFilter == FILTER_TAKEDOWNS;
                break;

            default:
                break;
    }

    return false;
}

bool QFilterAlertsModel::filterAcceptsRow(int row, const QModelIndex &sourceParent) const
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
                mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_ERROR, "Invalid notification item type.");
                break;
            }
        }
    }
    return filter;
}

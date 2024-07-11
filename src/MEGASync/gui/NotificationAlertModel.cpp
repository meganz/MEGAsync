#include "NotificationAlertModel.h"

#include "NotificationAlertTypes.h"
#include "MegaNotificationExt.h"
#include "MegaUserAlertExt.h"

#include "megaapi.h"

QModelIndex NotificationAlertModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
    {
        return QModelIndex();
    }

    return createIndex(row, column, mNotifications.at(row));
}

QModelIndex NotificationAlertModel::parent(const QModelIndex& index) const
{
    Q_UNUSED(index)

    return QModelIndex();
}

int NotificationAlertModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : mNotifications.size();
}

int NotificationAlertModel::columnCount(const QModelIndex& parent) const
{
    return 1;
}

QVariant NotificationAlertModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0)
    {
        return QVariant();
    }

    if (role == Qt::DisplayRole)
    {
        return QVariant::fromValue(index.internalId());
    }
    else if (role == Qt::UserRole && mNotifications.at(index.row())->getType() == NotificationExtBase::Type::ALERT)
    {
        auto alert = qobject_cast<const MegaUserAlertExt*>(mNotifications.at(index.row()));
        return QDateTime::fromMSecsSinceEpoch(alert->getTimestamp(0) * 1000);
    }

    return QVariant();
}

Qt::ItemFlags NotificationAlertModel::flags(const QModelIndex& index) const
{
    return QAbstractItemModel::flags(index) | Qt::ItemIsEnabled | Qt::ItemIsEditable;
}

auto NotificationAlertModel::findAlertById(unsigned int id)
{
    auto it = std::find_if(mNotifications.begin(), mNotifications.end(),
                           [id](const NotificationExtBase* current)
                           {
                               if(current->getType() == NotificationExtBase::Type::ALERT)
                               {
                                   auto alertItem = qobject_cast<const MegaUserAlertExt*>(current);
                                   return alertItem && alertItem->getId() == id;
                               }
                               return false;
                           });

    return it;
}

auto NotificationAlertModel::findNotificationById(int64_t id)
{
    auto it = std::find_if(mNotifications.begin(), mNotifications.end(),
                           [id](const NotificationExtBase* current)
                           {
                               if(current->getType() == NotificationExtBase::Type::NOTIFICATION)
                               {
                                   auto notificationItem = qobject_cast<const MegaNotificationExt*>(current);
                                   return notificationItem && notificationItem->getID() == id;
                               }
                               return false;
                           });

    return it;
}

void NotificationAlertModel::updateAlerts(mega::MegaUserAlertList* alerts)
{
    int numAlerts = alerts ? alerts->size() : 0;
    if (numAlerts)
    {
        QList<mega::MegaUserAlert*> newAlerts;
        QList<mega::MegaUserAlert*> updatedAlerts;
        QList<mega::MegaUserAlert*> removedAlerts;
        for (int i = 0; i < numAlerts; i++)
        {
            mega::MegaUserAlert* alert = alerts->get(i);
            //unsigned id = alert->getId();

            /*
            auto it = std::find_if(mNotifications.begin(), mNotifications.end(),
                                   [id](const NotificationExtBase* current)
                                   {
                                       if(current->getType() == NotificationExtBase::Type::ALERT)
                                       {
                                           auto alertItem = qobject_cast<const MegaUserAlertExt*>(current);
                                           return alertItem && alertItem->getId() == id;
                                       }
                                       return false;
                                   });
            */
            auto it = findAlertById(alert->getId());
            if (it == mNotifications.end() && !alert->isRemoved())
            {
                if(alert->isRemoved())
                {
                    removedAlerts.append(alert);
                }
                else
                {
                    newAlerts.append(alert->copy());
                }
            }
            else if (it != mNotifications.end())
            {
                updatedAlerts.append(alert);
            }
        }

        removeAlerts(removedAlerts);
        insertAlerts(newAlerts);
        updateAlerts(updatedAlerts);
    }
}

void NotificationAlertModel::insertAlerts(const QList<mega::MegaUserAlert*>& alerts)
{
    if(alerts.size() <= 0)
    {
        return;
    }

    beginInsertRows(QModelIndex(), 0, alerts.size() - 1);
    for (auto& notification : alerts)
    {
        mNotifications.push_back(new MegaUserAlertExt(notification));
    }
    endInsertRows();
}

void NotificationAlertModel::updateAlerts(const QList<mega::MegaUserAlert*>& alerts)
{
    if(alerts.size() <= 0)
    {
        return;
    }

    for (auto& alert : alerts)
    {
        /*
        unsigned id = alert->getId();
        auto it = std::find_if(mNotifications.begin(), mNotifications.end(),
                               [id](const NotificationExtBase* current)
                               {
                                   if(current->getType() == NotificationExtBase::Type::ALERT)
                                   {
                                       auto alertItem = qobject_cast<const MegaUserAlertExt*>(current);
                                       return alertItem && alertItem->getId() == id;
                                   }
                                   return false;
                               });
*/
        auto it = findAlertById(alert->getId());
        if (it != mNotifications.end())
        {
            int row = std::distance(mNotifications.begin(), it);
            auto alertItem = qobject_cast<MegaUserAlertExt*>(mNotifications[row]);
            alertItem->reset(alert);
            emit dataChanged(index(row, 0), index(row, 0));
        }
    }
}

void NotificationAlertModel::removeAlerts(const QList<mega::MegaUserAlert*>& alerts)
{
    if(alerts.size() <= 0)
    {
        return;
    }

    for (auto& alert : alerts)
    {
        /*
        unsigned id = alert->getId();
        auto it = std::find_if(mNotifications.begin(), mNotifications.end(),
                               [id](const NotificationExtBase* current)
                               {
                                   if(current->getType() == NotificationExtBase::Type::ALERT)
                                   {
                                       auto alertItem = qobject_cast<const MegaUserAlertExt*>(current);
                                       return alertItem && alertItem->getId() == id;
                                   }
                                   return false;
                               });
*/
        auto it = findAlertById(alert->getId());
        if (it != mNotifications.end())
        {
            int row = std::distance(mNotifications.begin(), it);
            beginRemoveRows(QModelIndex(), row, row);
            delete mNotifications[row];
            mNotifications.erase(it);
            endRemoveRows();
        }
    }
}

bool NotificationAlertModel::hasAlertsOfType(AlertType type)
{
    return std::any_of(mNotifications.begin(), mNotifications.end(),
                       [type](const NotificationExtBase* current)
                       {
                           if(current->getType() == NotificationExtBase::Type::ALERT)
                           {
                               auto alertItem = qobject_cast<const MegaUserAlertExt*>(current);
                               return alertItem && alertItem->getAlertType() == type;
                           }
                           return false;
                       });
}

void NotificationAlertModel::processNotifications(const mega::MegaNotificationList* notifications)
{
    int numNotifications = notifications ? notifications->size() : 0;
    if (numNotifications)
    {
        removeNotifications(notifications);
        insertNotifications(notifications);
    }
}

void NotificationAlertModel::insertNotifications(const mega::MegaNotificationList* notifications)
{
    QList<const mega::MegaNotification*> newNotifications;
    for (int i = 0; i < notifications->size(); i++)
    {
        const mega::MegaNotification* notification = notifications->get(i);
        auto it = findNotificationById(notification->getID());
        if (it == mNotifications.end())
        {
            newNotifications.append(notification->copy());
        }
    }

    if(newNotifications.size() <= 0)
    {
        return;
    }

    beginInsertRows(QModelIndex(), 0, newNotifications.size() - 1);
    for (auto& notification : newNotifications)
    {
        mNotifications.push_back(new MegaNotificationExt(notification));
    }
    endInsertRows();
}

void NotificationAlertModel::removeNotifications(const mega::MegaNotificationList* notifications)
{
    QList<int64_t> removedItems;
    for (auto& item : mNotifications)
    {
        unsigned i = 0;
        bool found = false;

        if(item->getType() != NotificationExtBase::Type::NOTIFICATION)
        {
            continue;
        }

        auto notification = qobject_cast<MegaNotificationExt*>(item);
        auto id = notification->getID();
        while (i < notifications->size() && !found)
        {
            found = notifications->get(i)->getID() == id;
            ++i;
        }

        if (!found)
        {
            removedItems.append(id);
        }
    }

    if(removedItems.size() <= 0)
    {
        return;
    }

    for (auto& id : removedItems)
    {
        auto it = findNotificationById(id);
        if (it == mNotifications.end())
        {
            continue;
        }

        int row = std::distance(mNotifications.begin(), it);
        beginRemoveRows(QModelIndex(), row, row);
        delete mNotifications[row];
        mNotifications.erase(it);
        endRemoveRows();
    }
}

/*
int NotificationAlertModel::getAlertType(int alertType) const
{
    int type = AlertType::UNKNOWN;
    switch (alertType)
    {
        case mega::MegaUserAlert::TYPE_INCOMINGPENDINGCONTACT_REQUEST:
        case mega::MegaUserAlert::TYPE_INCOMINGPENDINGCONTACT_CANCELLED:
        case mega::MegaUserAlert::TYPE_INCOMINGPENDINGCONTACT_REMINDER:
        case mega::MegaUserAlert::TYPE_CONTACTCHANGE_DELETEDYOU:
        case mega::MegaUserAlert::TYPE_CONTACTCHANGE_CONTACTESTABLISHED:
        case mega::MegaUserAlert::TYPE_CONTACTCHANGE_ACCOUNTDELETED:
        case mega::MegaUserAlert::TYPE_CONTACTCHANGE_BLOCKEDYOU:
        case mega::MegaUserAlert::TYPE_UPDATEDPENDINGCONTACTINCOMING_IGNORED:
        case mega::MegaUserAlert::TYPE_UPDATEDPENDINGCONTACTINCOMING_ACCEPTED:
        case mega::MegaUserAlert::TYPE_UPDATEDPENDINGCONTACTINCOMING_DENIED:
        case mega::MegaUserAlert::TYPE_UPDATEDPENDINGCONTACTOUTGOING_ACCEPTED:
        case mega::MegaUserAlert::TYPE_UPDATEDPENDINGCONTACTOUTGOING_DENIED:
        {
            type = AlertType::CONTACTS;
            break;
        }
        case mega::MegaUserAlert::TYPE_NEWSHARE:
        case mega::MegaUserAlert::TYPE_DELETEDSHARE:
        case mega::MegaUserAlert::TYPE_NEWSHAREDNODES:
        case mega::MegaUserAlert::TYPE_REMOVEDSHAREDNODES:
        case mega::MegaUserAlert::TYPE_UPDATEDSHAREDNODES:
        {
            type = AlertType::SHARES;
            break;
        }
        case mega::MegaUserAlert::TYPE_PAYMENT_SUCCEEDED:
        case mega::MegaUserAlert::TYPE_PAYMENT_FAILED:
        case mega::MegaUserAlert::TYPE_PAYMENTREMINDER:
        {
            type = AlertType::PAYMENTS;
            break;
        }
        case mega::MegaUserAlert::TYPE_TAKEDOWN:
        case mega::MegaUserAlert::TYPE_TAKEDOWN_REINSTATED:
        {
            type = AlertType::TAKEDOWNS;
            break;
        }
        default:
        {
            break;
        }
    }
    return type;
}

void NotificationAlertModel::insertNotifications(const mega::MegaNotificationList* notificationList)
{

}
*/

/*
QModelIndex NotificationAlertModel::index(int row, int column, const QModelIndex& parent) const
{
    QModelIndex result;
    if (!parent.isValid())
    {
        int notificationRowCount = getNotificationRowCount();
        if (notificationRowCount > 0 && row < notificationRowCount)
        {
            MegaNotificationExt* notification = static_cast<MegaNotificationExt*>(mNotificationsModel->index(row, column).internalPointer());
            NotificationAlertModelItem* item = new NotificationAlertModelItem { NotificationAlertModelItem::NOTIFICATION, notification };
            result = createIndex(row, column, item);
        }
        else if (row < (notificationRowCount + getAlertRowCount()))
        {
            int alertRow = getAlertRow(row);
            MegaUserAlertExt* alert = static_cast<MegaUserAlertExt*>(mAlertsModel->index(alertRow, column).internalPointer());
            NotificationAlertModelItem* item = new NotificationAlertModelItem { NotificationAlertModelItem::ALERT, alert };
            result = createIndex(row, column, item);
        }
    }
    return result;
}

QModelIndex NotificationAlertModel::parent(const QModelIndex& index) const
{
    return QModelIndex();
}

int NotificationAlertModel::rowCount(const QModelIndex& parent) const
{
    int totalRowCount = 0;
    if (!parent.isValid())
    {
        totalRowCount = getNotificationRowCount() + getAlertRowCount();
    }
    return totalRowCount;
}

int NotificationAlertModel::columnCount(const QModelIndex& parent) const
{
    int maxColumnCount = 0;
    if (!parent.isValid())
    {
        maxColumnCount = std::max(mNotificationsModel ? mNotificationsModel->columnCount() : 0,
                                  mAlertsModel ? mAlertsModel->columnCount() : 0);
    }
    return maxColumnCount;
}

QVariant NotificationAlertModel::data(const QModelIndex& index, int role) const
{
    QVariant result;
    if (index.isValid())
    {
        NotificationAlertModelItem* item = static_cast<NotificationAlertModelItem*>(index.internalPointer());
        switch (item->type)
        {
            case NotificationAlertModelItem::ALERT:
            {
                result = mAlertsModel->data(mAlertsModel->index(getAlertRow(index.row()), index.column()), role);
                break;
            }
            case NotificationAlertModelItem::NOTIFICATION:
            {
                result = mNotificationsModel->data(mNotificationsModel->index(index.row(), index.column()), role);
                break;
            }
            default:
            {
                mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_ERROR, "Invalid notification item type.");
                break;
            }
        }
    }
    return result;
}

void NotificationAlertModel::createNotificationModel(const mega::MegaNotificationList* notifications)
{
    if(!mNotificationsModel)
    {
        mNotificationsModel = std::make_unique<NotificationModel>(nullptr);
        connect(mNotificationsModel.get(), &QAbstractItemModel::rowsInserted,
                this, &NotificationAlertModel::onNotificationRowsInserted);
        connect(mNotificationsModel.get(), &QAbstractItemModel::rowsRemoved,
                this, &NotificationAlertModel::onNotificationRowsRemoved);
        connect(mNotificationsModel.get(), &QAbstractItemModel::dataChanged,
                this, &NotificationAlertModel::onNotificationDataChanged);
        mNotificationsModel->insert(notifications);
    }
}

void NotificationAlertModel::createAlertModel(mega::MegaUserAlertList* alerts)
{
    if(!mAlertsModel)
    {
        mAlertsModel = std::make_unique<AlertModel>(nullptr);
        connect(mAlertsModel.get(), &QAbstractItemModel::rowsInserted,
                this, &NotificationAlertModel::onAlertRowsInserted);
        connect(mAlertsModel.get(), &QAbstractItemModel::rowsRemoved,
                this, &NotificationAlertModel::onAlertRowsRemoved);
        connect(mAlertsModel.get(), &QAbstractItemModel::dataChanged,
                this, &NotificationAlertModel::onAlertDataChanged);
        mAlertsModel->insertAlerts(alerts);
    }
}

void NotificationAlertModel::onAlertRowsInserted(const QModelIndex& parent, int first, int last)
{
    if (!parent.isValid())
    {
        int startRow = first + getNotificationRowCount();
        int endRow = last + getNotificationRowCount();
        beginInsertRows(QModelIndex(), startRow, endRow);
        endInsertRows();
    }
}

void NotificationAlertModel::onAlertRowsRemoved(const QModelIndex& parent, int first, int last)
{
    if (!parent.isValid())
    {
        int startRow = first + getNotificationRowCount();
        int endRow = last + getNotificationRowCount();
        beginRemoveRows(QModelIndex(), startRow, endRow);
        endRemoveRows();
    }
}

void NotificationAlertModel::onAlertDataChanged(const QModelIndex& topLeft,
                                                const QModelIndex& bottomRight,
                                                const QVector<int>& roles)
{
    int startRow = topLeft.row() + getNotificationRowCount();
    int endRow = bottomRight.row() + getNotificationRowCount();
    emit dataChanged(index(startRow, topLeft.column()),
                     index(endRow, bottomRight.column()), roles);
}

void NotificationAlertModel::onNotificationRowsInserted(const QModelIndex& parent, int first, int last)
{
    if (!parent.isValid())
    {
        beginInsertRows(QModelIndex(), first, last);
        endInsertRows();
    }
}

void NotificationAlertModel::onNotificationRowsRemoved(const QModelIndex& parent, int first, int last)
{
    if (!parent.isValid())
    {
        beginRemoveRows(QModelIndex(), first, last);
        endRemoveRows();
    }
}

void NotificationAlertModel::onNotificationDataChanged(const QModelIndex& topLeft,
                                                const QModelIndex& bottomRight,
                                                const QVector<int>& roles)
{
    emit dataChanged(index(topLeft.row(), topLeft.column()),
                     index(bottomRight.row(), bottomRight.column()), roles);
}

bool NotificationAlertModel::hasNotificationsOrAlerts()
{
    return rowCount() > 0;
}

bool NotificationAlertModel::hasAlertsOfType(int type)
{
    return mAlertsModel && mAlertsModel->existsNotifications(type);
}

QMap<AlertModel::AlertType, long long> NotificationAlertModel::getUnseenNotifications() const
{
    QMap<AlertModel::AlertType, long long> unseenNotifications;
    if (mAlertsModel)
    {
        unseenNotifications = mAlertsModel->getUnseenAlerts();
    }
    if(mNotificationsModel)
    {
        unseenNotifications[AlertModel::AlertType::ALERT_ALL] += mNotificationsModel->getNumUnseenNotifications();
    }
    return unseenNotifications;
}

AlertModel* NotificationAlertModel::alertModel() const
{
    return mAlertsModel.get();
}

NotificationModel* NotificationAlertModel::notificationModel() const
{
    return mNotificationsModel.get();
}

uint32_t NotificationAlertModel::getLastSeenNotification() const
{
    return mNotificationsModel ? mNotificationsModel->getLastSeenNotification() : 0;
}

void NotificationAlertModel::setLastSeenNotification(uint32_t id)
{
    if (mNotificationsModel)
    {
        mNotificationsModel->setLastSeenNotification(id);
    }
}

void NotificationAlertModel::insertAlerts(mega::MegaUserAlertList* alerts)
{
    if (mAlertsModel)
    {
        mAlertsModel->insertAlerts(alerts, true);
    }
}

void NotificationAlertModel::insertNotifications(const mega::MegaNotificationList* notificationList)
{
    if (mNotificationsModel)
    {
        mNotificationsModel->insert(notificationList);
    }
}

int NotificationAlertModel::getNotificationRowCount() const
{
    return mNotificationsModel ? mNotificationsModel->rowCount() : 0;
}

int NotificationAlertModel::getAlertRowCount() const
{
    return mAlertsModel ? mAlertsModel->rowCount() : 0;
}

int NotificationAlertModel::getAlertRow(int row) const
{
    return row - getNotificationRowCount();
}
*/



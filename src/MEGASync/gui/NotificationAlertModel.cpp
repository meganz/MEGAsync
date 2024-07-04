#include "NotificationAlertModel.h"

#include "NotificationAlertTypes.h"

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
        mNotificationsModel = std::make_unique<NotificationModel>(this);
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
        mAlertsModel = std::make_unique<AlertModel>(this);
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

void NotificationAlertModel::onDataChanged(const QModelIndex& topLeft,
                                           const QModelIndex& bottomRight,
                                           const QVector<int>& roles)
{
    emit dataChanged(index(topLeft.row(), topLeft.column()), index(bottomRight.row(), bottomRight.column()), roles);
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

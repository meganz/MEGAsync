#include "NotificationAlertModel.h"

#include "NotificationAlertTypes.h"

NotificationAlertModel::NotificationAlertModel(NotificationModel* notificationsModel,
                                               AlertModel* alertsModel,
                                               QObject* parent)
    : QAbstractItemModel(parent)
    , mNotificationsModel(std::unique_ptr<NotificationModel>(notificationsModel))
    , mAlertsModel(std::unique_ptr<AlertModel>(alertsModel))
{
    connect(mNotificationsModel.get(), &QAbstractItemModel::dataChanged, this, &NotificationAlertModel::onDataChanged);
    connect(mAlertsModel.get(), &QAbstractItemModel::dataChanged, this, &NotificationAlertModel::onDataChanged);
}

QModelIndex NotificationAlertModel::index(int row, int column, const QModelIndex& parent) const
{
    QModelIndex result;
    if (!parent.isValid())
    {
        if (row < mNotificationsModel->rowCount())
        {
            NotificationAlertModelItem* item = new NotificationAlertModelItem { NotificationAlertModelItem::NOTIFICATION,
                                                                                mNotificationsModel->index(row, column).internalPointer() };
            result = createIndex(row, column, item);
        }
        else if (row < mNotificationsModel->rowCount() + mAlertsModel->rowCount())
        {
            int alertRow = row - mNotificationsModel->rowCount();
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
        totalRowCount = mNotificationsModel->rowCount() + mAlertsModel->rowCount();
    }
    return totalRowCount;
}

int NotificationAlertModel::columnCount(const QModelIndex& parent) const
{
    int maxColumnCount = 0;
    if (!parent.isValid())
    {
        maxColumnCount = std::max(mNotificationsModel->columnCount(), mAlertsModel->columnCount());
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
                int alertRow = index.row() - mNotificationsModel->rowCount();
                result = mAlertsModel->data(mAlertsModel->index(alertRow, index.column()), role);
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

bool NotificationAlertModel::hasAlerts()
{
    return (mAlertsModel && mAlertsModel->rowCount(QModelIndex()))
           || (mNotificationsModel && mNotificationsModel->rowCount(QModelIndex()));
}

bool NotificationAlertModel::hasAlertsOfType(int type)
{
    return (mAlertsModel && mAlertsModel->existsNotifications(type))
            || (mNotificationsModel && mNotificationsModel->rowCount(QModelIndex()));
}

QMap<AlertModel::AlertType, long long> NotificationAlertModel::getUnseenNotifications() const
{
    QMap<AlertModel::AlertType, long long> unseenNotifications;
    if (mAlertsModel)
    {
        unseenNotifications = mAlertsModel->getUnseenNotifications();
    }
    return unseenNotifications;
}

void NotificationAlertModel::insertAlerts(mega::MegaUserAlertList* alerts)
{
    if (mAlertsModel)
    {
        mAlertsModel->insertAlerts(alerts, true);
    }
}

void NotificationAlertModel::onDataChanged(const QModelIndex& topLeft,
                                           const QModelIndex& bottomRight,
                                           const QVector<int>& roles)
{
    emit dataChanged(index(topLeft.row(), topLeft.column()), index(bottomRight.row(), bottomRight.column()), roles);
}

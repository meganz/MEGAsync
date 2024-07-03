#ifndef NOTIFICATIONALERTMODEL_H
#define NOTIFICATIONALERTMODEL_H

#include "NotificationModel.h"
#include "AlertModel.h"

class NotificationAlertModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    using QAbstractItemModel::QAbstractItemModel;
    virtual ~NotificationAlertModel() = default;

    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;

    void createNotificationModel(const mega::MegaNotificationList* notifications);
    void createAlertModel(mega::MegaUserAlertList* alerts);

    bool hasNotificationsOrAlerts();
    bool hasAlertsOfType(int type);
    void insertAlerts(mega::MegaUserAlertList* alerts);
    void insertNotifications(const mega::MegaNotificationList* notificationList);
    QMap<AlertModel::AlertType, long long> getUnseenNotifications() const;
    AlertModel* alertModel() const;
    NotificationModel* notificationModel() const;
    uint32_t getLastSeenNotification() const;
    void setLastSeenNotification(uint32_t id);

private slots:
    void onDataChanged(const QModelIndex& topLeft,
                       const QModelIndex& bottomRight,
                       const QVector<int>& roles = QVector<int>());

private:
    std::unique_ptr<NotificationModel> mNotificationsModel = nullptr;
    std::unique_ptr<AlertModel> mAlertsModel = nullptr;

    int getNotificationRowCount() const;
    int getAlertRowCount() const;
    int getAlertRow(int row) const;

};

#endif // NOTIFICATIONALERTMODEL_H

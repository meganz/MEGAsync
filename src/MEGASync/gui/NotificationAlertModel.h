#ifndef NOTIFICATION_ALERT_MODEL_H
#define NOTIFICATION_ALERT_MODEL_H

#include "NotificationExtBase.h"
#include "NotificationAlertTypes.h"

#include <QAbstractItemModel>

namespace mega
{
class MegaUserAlert;
class MegaUserAlertList;
class MegaNotificationList;
class MegaNotification;
}

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
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    void updateAlerts(mega::MegaUserAlertList* alerts);
    bool hasAlertsOfType(AlertType type);
    void processNotifications(const mega::MegaNotificationList* notifications);


    //void insertNotifications(const mega::MegaNotificationList* notificationList);

    /*
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
    void onAlertDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles);
    void onAlertRowsInserted(const QModelIndex& parent, int first, int last);
    void onAlertRowsRemoved(const QModelIndex& parent, int first, int last);
    void onNotificationDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles);
    void onNotificationRowsInserted(const QModelIndex& parent, int first, int last);
    void onNotificationRowsRemoved(const QModelIndex& parent, int first, int last);
*/

private:
    QList<NotificationExtBase*> mNotifications;

    void insertAlerts(const QList<mega::MegaUserAlert*>& alerts);
    void updateAlerts(const QList<mega::MegaUserAlert*>& alerts);
    void removeAlerts(const QList<mega::MegaUserAlert*>& alerts);

    void insertNotifications(const mega::MegaNotificationList* notifications);
    void removeNotifications(const mega::MegaNotificationList* notifications);


   // int getNotificationRowCount() const;
   // int getAlertRowCount() const;
  // int getAlertRow(int row) const;

    auto findAlertById(unsigned id);
    auto findNotificationById(int64_t id);

};

#endif // NOTIFICATION_ALERT_MODEL_H

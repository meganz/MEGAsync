#ifndef NOTIFICATIONALERTMODEL_H
#define NOTIFICATIONALERTMODEL_H

#include "QNotificationsModel.h"
#include "QAlertsModel.h"
#include "NotificationAlertProxyModel.h"

struct AlertNotificationModelItem
{
    enum ModelType
    {
        NONE = -1,
        ALERT = 0,
        NOTIFICATION = 1
    };

    ModelType type;
    void* pointer;
};

class NotificationAlertModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    NotificationAlertModel(QNotificationsModel* notificationsModel,
                           QAlertsModel* alertsModel,
                           QObject *parent = nullptr);

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;

    QAlertsModel* alertsModel() const;
    bool hasAlerts();
    bool hasAlertsOfType(int type);
    QMap<QAlertsModel::AlertType, long long> getUnseenNotifications() const;
    void insertAlerts(mega::MegaUserAlertList* alerts, bool copy = false);

private slots:
    void onDataChanged(const QModelIndex &topLeft,
                       const QModelIndex &bottomRight,
                       const QVector<int> &roles = QVector<int>());

private:
    QNotificationsModel* mNotificationsModel;
    QAlertsModel* mAlertsModel;

};

#endif // NOTIFICATIONALERTMODEL_H

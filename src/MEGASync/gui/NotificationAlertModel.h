#ifndef NOTIFICATIONALERTMODEL_H
#define NOTIFICATIONALERTMODEL_H

#include "NotificationModel.h"
#include "AlertModel.h"

class NotificationAlertModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    NotificationAlertModel(NotificationModel* notificationsModel,
                           AlertModel* alertsModel,
                           QObject* parent = nullptr);

    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;

    AlertModel* alertsModel() const;
    bool hasAlerts();
    bool hasAlertsOfType(int type);
    QMap<AlertModel::AlertType, long long> getUnseenNotifications() const;
    void insertAlerts(mega::MegaUserAlertList* alerts, bool copy = false);

private slots:
    void onDataChanged(const QModelIndex& topLeft,
                       const QModelIndex& bottomRight,
                       const QVector<int>& roles = QVector<int>());

private:
    NotificationModel* mNotificationsModel;
    AlertModel* mAlertsModel;

};

#endif // NOTIFICATIONALERTMODEL_H

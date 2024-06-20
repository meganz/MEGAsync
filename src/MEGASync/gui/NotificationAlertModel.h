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

    bool hasAlerts();
    bool hasAlertsOfType(int type);
    void insertAlerts(mega::MegaUserAlertList* alerts, bool copy = false);
    QMap<AlertModel::AlertType, long long> getUnseenNotifications() const;

private slots:
    void onDataChanged(const QModelIndex& topLeft,
                       const QModelIndex& bottomRight,
                       const QVector<int>& roles = QVector<int>());

private:
    std::unique_ptr<NotificationModel> mNotificationsModel;
    std::unique_ptr<AlertModel> mAlertsModel;

};

#endif // NOTIFICATIONALERTMODEL_H

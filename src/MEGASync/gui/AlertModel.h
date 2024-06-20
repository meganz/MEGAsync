#ifndef ALERTMODEL_H
#define ALERTMODEL_H

#include "AlertItem.h"
#include "MegaUserAlertExt.h"

#include <QCache>
#include <QAbstractItemModel>

#include <deque>
#include <array>

class AlertModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    enum AlertType
    {
        ALERT_CONTACTS = 0,
        ALERT_SHARES,
        ALERT_PAYMENT,
        ALERT_TAKEDOWNS,
        ALERT_UNKNOWN,
        ALERT_ALL //this must be the last on the enum
    };

    explicit AlertModel(mega::MegaUserAlertList* alerts, QObject* parent = 0);
    virtual ~AlertModel();

    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    void refreshAlerts();
    void insertAlerts(mega::MegaUserAlertList* alerts, bool copy = false);
    void refreshAlertItem(unsigned id);
    bool existsNotifications(int type) const;
    QMap<AlertType, long long> getUnseenNotifications() const;

    QCache<int, AlertItem> alertItems;

private:
    QMap<int, MegaUserAlertExt*> mAlertsMap;
    std::deque<unsigned int> mAlertOrder;
    std::array<int, ALERT_ALL> mUnSeenNotifications;
    std::array<bool, ALERT_ALL> mHasNotificationsOfType;

    int checkAlertType(int alertType) const;

};

#endif // ALERTMODEL_H

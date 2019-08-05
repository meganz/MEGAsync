#ifndef QALERTSMODEL_H
#define QALERTSMODEL_H

#include <QCache>
#include <megaapi.h>
#include <deque>
#include "AlertItem.h"
#include <QAbstractItemModel>
#include <array>

class QAlertsModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    enum {
        ALERT_CONTACTS = 0,
        ALERT_SHARES,
        ALERT_PAYMENT,
        ALERT_TAKEDOWNS,
        ALERT_ALL,
    };

    static const int ALERT_TYPES = 4;

    explicit QAlertsModel(mega::MegaUserAlertList* alerts, bool copy = false, QObject *parent = 0);
    virtual ~QAlertsModel();

    void refreshAlerts();

    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    int rowCount(const QModelIndex &parent) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    QCache<int, AlertItem> alertItems;

    void insertAlerts(mega::MegaUserAlertList *alerts, bool copy = false);

    long long getUnseenNotifications(int type) const;
    bool existsNotifications(int type) const;

private:
    int checkAlertType(int alertType) const;

private:
    QMap<int, mega::MegaUserAlert*> alertsMap;
    std::deque<int> alertOrder;
    std::array<int, ALERT_TYPES> unSeenNotifications;
    std::array<bool, ALERT_TYPES> hasNotificationsOfType;

};

#endif // QALERTSMODEL_H

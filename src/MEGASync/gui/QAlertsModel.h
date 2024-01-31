#ifndef QALERTSMODEL_H
#define QALERTSMODEL_H

#include <deque>
#include <array>

#include <QCache>
#include <QAbstractItemModel>

#include <megaapi.h>

#include "AlertItem.h"
#include "MegaUserAlertExt.h"

class QAlertsModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    enum {
        ALERT_CONTACTS = 0,
        ALERT_SHARES,
        ALERT_PAYMENT,
        ALERT_TAKEDOWNS,
        ALERT_UNKNOWN,
        ALERT_ALL, //this must be the last on the enum
    };

    explicit QAlertsModel(mega::MegaUserAlertList* alerts, bool copy = false, QObject *parent = 0);
    virtual ~QAlertsModel();

    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    int rowCount(const QModelIndex &parent) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    QCache<int, AlertItem> alertItems;

    void refreshAlerts();
    void insertAlerts(mega::MegaUserAlertList *alerts, bool copy = false);

    long long getUnseenNotifications(int type) const;
    bool existsNotifications(int type) const;
    void updateContacts(mega::MegaUserList *userList);
    void refreshAlertItem(unsigned item);

private:
    int checkAlertType(int alertType) const;

    QMap<int, mega::MegaUserAlert*> alertsMap;
    std::deque<unsigned int> alertOrder;
    std::array<int, ALERT_ALL> unSeenNotifications;
    std::array<bool, ALERT_ALL> hasNotificationsOfType;
};

#endif // QALERTSMODEL_H

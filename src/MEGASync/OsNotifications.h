#pragma once
#include "notificator.h"
#include <QObject>

namespace mega {
class MegaUserAlertList;
class MegaUserAlert;
}

class OsNotifications: public QObject
{
    Q_OBJECT
public:
    OsNotifications(Notificator *notificator);
    void addUserAlertList(mega::MegaUserAlertList *alertList);
    void sendOverStorageNotification(int state);
    void sendOverTransferNotification(const QString &title);
    void sendBusinessWarningNotification(int businessStatus);

public slots:
    void incomingPendingRequest(MegaNotification::Action action);
    void viewContactOnWebClient();
    void redirectToUpgrade(MegaNotification::Action activationButton);
    void redirectToPayBusiness(MegaNotification::Action activationButton);

private:
    Notificator *mNotificator;
};

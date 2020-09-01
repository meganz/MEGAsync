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

public slots:
    void incomingPendingRequest(int activationButton);

private:
    Notificator *mNotificator;
};

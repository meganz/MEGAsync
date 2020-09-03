#pragma once
#include "notificator.h"
#include <QObject>
#include <memory>

namespace mega {
class MegaUserAlertList;
class MegaUserAlert;
}

class OsNotifications: public QObject
{
    Q_OBJECT
public:
    OsNotifications(const QString& appName, QSystemTrayIcon* trayIcon);
    void addUserAlertList(mega::MegaUserAlertList *alertList);
    void sendOverStorageNotification(int state);
    void sendOverTransferNotification(const QString& title);
    void sendFinishedTransferNotification(const QString& title, const QString& message, const QString& extraData);
    void sendBusinessWarningNotification(int businessStatus);
    void sendInfoNotification(const QString& title, const QString& message);
    void sendWarningNotification(const QString& title, const QString& message);
    void sendErrorNotification(const QString& title, const QString& message);

public slots:
    void incomingPendingRequest(MegaNotification::Action action);
    void viewContactOnWebClient();
    void redirectToUpgrade(MegaNotification::Action activationButton);
    void redirectToPayBusiness(MegaNotification::Action activationButton);
    void showInFolder(MegaNotification::Action action);

private:
    Notificator* mNotificator;
    QIcon mAppIcon;
};

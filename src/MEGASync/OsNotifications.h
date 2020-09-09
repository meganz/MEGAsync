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
    void addUserAlertList(mega::MegaUserAlertList *alertList) const;
    void sendOverStorageNotification(int state) const;
    void sendOverTransferNotification(const QString& title) const;
    void sendFinishedTransferNotification(const QString& title, const QString& message, const QString& extraData) const;
    void sendBusinessWarningNotification(int businessStatus) const;
    void sendInfoNotification(const QString& title, const QString& message) const;
    void sendWarningNotification(const QString& title, const QString& message) const;
    void sendErrorNotification(const QString& title, const QString& message) const;

public slots:
    void replayIncomingPendingRequest(MegaNotification::Action action) const;
    void viewContactOnWebClient() const;
    void redirectToUpgrade(MegaNotification::Action activationButton) const;
    void redirectToPayBusiness(MegaNotification::Action activationButton) const;
    void showInFolder(MegaNotification::Action action) const;
    void viewShareOnWebClient(MegaNotification::Action action) const;

private:
    void notifyTakeDownReinstated(mega::MegaUserAlert* alert) const;
    void notifySharedUpdate(mega::MegaUserAlert* alert, const QString& message) const;

    Notificator* mNotificator;
    QIcon mAppIcon;
};

#pragma once
#include "notificator.h"
#include "RemovedSharesNotificator.h"
#include "Preferences.h"
#include <QObject>
#include <memory>

namespace mega {
class MegaUserAlertList;
class MegaUserAlert;
}

class DesktopNotifications: public QObject
{
    Q_OBJECT
public:
    DesktopNotifications(const QString& appName, QSystemTrayIcon* trayIcon, Preferences *preferences);
    void addUserAlertList(mega::MegaUserAlertList *alertList);
    void sendOverStorageNotification(int state) const;
    void sendOverTransferNotification(const QString& title) const;
    void sendFinishedTransferNotification(const QString& title, const QString& message, const QString& extraData) const;
    void sendBusinessWarningNotification(int businessStatus) const;
    void sendInfoNotification(const QString& title, const QString& message) const;
    void sendWarningNotification(const QString& title, const QString& message) const;
    void sendErrorNotification(const QString& title, const QString& message) const;

public slots:
    void replayIncomingPendingRequest(MegaNotification::Action action) const;
    void viewContactOnWebClient(MegaNotification::Action activationButton) const;
    void redirectToUpgrade(MegaNotification::Action activationButton) const;
    void redirectToPayBusiness(MegaNotification::Action activationButton) const;
    void showInFolder(MegaNotification::Action action) const;
    void viewShareOnWebClient(MegaNotification::Action action) const;
    void receiveClusteredAlert(mega::MegaUserAlert* alert, const QString& message) const;
    void replayNewShareReceived(MegaNotification::Action action) const;
    void viewOnInfoDialogNotifications(MegaNotification::Action action) const;

private:
    void notifyTakeDown(mega::MegaUserAlert* alert, bool isReinstated = false) const;
    void notifySharedUpdate(mega::MegaUserAlert* alert, const QString& message, bool isNewShare = false) const;
    void notifyUnreadNotifications() const;

    QString getItemsAddedText(mega::MegaUserAlert* alert);
    QString getItemsRemovedMessage(mega::MegaUserAlert* alert);
    QString createPaymentReminderText(int64_t expirationTimeStamp);
    QString createDeletedShareMessage(mega::MegaUserAlert* alert);
    QString createTakeDownMessage(mega::MegaUserAlert* alert, bool isReinstated = false) const;
    int countUnseenAlerts(mega::MegaUserAlertList *alertList);

    Notificator* mNotificator;
    QIcon mAppIcon;
    QString mNewContactIconPath, mStorageQuotaFullIconPath, mStorageQuotaWarningIconPath;
    QString mFolderIconPath, mFileDownloadSucceedIconPath;
    RemovedSharesNotificator mRemovedSharedNotificator;
    Preferences *mPreferences;
};

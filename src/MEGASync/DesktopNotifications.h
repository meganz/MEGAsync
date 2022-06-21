#pragma once
#include "notificator.h"
#include "RemovedSharesNotificator.h"
#include "Preferences.h"
#include "QTMegaRequestListener.h"

#include <QObject>

namespace mega {
class MegaUserAlertList;
class MegaUserAlert;
}

namespace UserAttributes{
class FullNameAttributeRequest;
}

class DesktopNotifications: public QObject
{
    Q_OBJECT
public:
    enum  {
        NEW_SHARE = 0,
        DELETE_SHARE = 1,
        NEW_SHARED_NODES = 2,
        REMOVED_SHARED_NODES = 3
    };
    DesktopNotifications(const QString& appName, QSystemTrayIcon* trayIcon);
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

private slots:
    void OnUserAttributesReady();

private:
    void notifyTakeDown(mega::MegaUserAlert* alert, bool isReinstated = false) const;
    void notifySharedUpdate(mega::MegaUserAlert* alert, const QString& message, int type) const;
    void notifyUnreadNotifications() const;

    QString getItemsAddedText(mega::MegaUserAlert* info);
    QString createDeletedShareMessage(mega::MegaUserAlert* info);
    QString createTakeDownMessage(mega::MegaUserAlert* alert, bool isReinstated = false) const;
    int countUnseenAlerts(mega::MegaUserAlertList *alertList);

    void processAlert(mega::MegaUserAlert* alert);
    MegaNotification* CreateContacNotification(const QString& title,
                                               const QString& message,
                                               const QString& email,
                                               const QStringList& actions);

    Notificator* mNotificator;
    QIcon mAppIcon;
    QString mNewContactIconPath, mStorageQuotaFullIconPath, mStorageQuotaWarningIconPath;
    QString mFolderIconPath, mFileDownloadSucceedIconPath;
    RemovedSharesNotificator mRemovedSharedNotificator;
    std::shared_ptr<Preferences> mPreferences;
    bool mIsFirstTime;//Check first time alerts are added to show unified message of unread.

    QMultiMap<QString, mega::MegaUserAlert*> mPendingUserAlerts;
    QMap<QString, std::shared_ptr<const UserAttributes::FullNameAttributeRequest>> mUserAttributes;
};

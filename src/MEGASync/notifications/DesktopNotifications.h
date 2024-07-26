#pragma once
#include "Notificator.h"
#include "NotificationDelayer.h"
#include "Preferences.h"

#include <QObject>

namespace mega {
class MegaUserAlertList;
class MegaUserAlert;
}

namespace UserAttributes{
class FullName;
}

class TransferMetaData;

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

    struct NotificationInfo
    {
        QString title;
        QString message;
        QString imagePath;
        QStringList actions;
        std::function<void(DesktopAppNotificationBase::Action)> activatedFunction = nullptr;
        NotificationInfo();

        bool isValid(){return !title.isEmpty() && !message.isEmpty();}
    };

    DesktopNotifications(const QString& appName, QSystemTrayIcon* trayIcon);

    void addUserAlertList(mega::MegaUserAlertList *alertList);
    void sendAlert(mega::MegaUserAlert* alert);
    void requestFullName(mega::MegaUserAlert* alert, QString email);
    void requestEmail(mega::MegaUserAlert* alert);
    void sendOverStorageNotification(int state) const;
    void sendOverTransferNotification(const QString& title) const;
    void sendFinishedTransferNotification(unsigned long long appDataId) const;
    void sendFinishedSetDownloadNotification(const QString& setName,
                                             const QStringList& succeededDownloadedElements,
                                             const QStringList& failedDownloadedElements,
                                             const QString& destinationPath);
    void sendBusinessWarningNotification(int businessStatus) const;
    void sendInfoNotification(const QString& title, const QString& message) const;
    void sendInfoNotification(const NotificationInfo& info) const;
    void sendWarningNotification(const QString& title, const QString& message) const;
    void sendErrorNotification(const QString& title, const QString& message) const;

public slots:
    void replyIncomingPendingRequest(DesktopAppNotification::Action action);
    void viewContactOnWebClient(DesktopAppNotification::Action activationButton) const;
    void redirectToUpgrade(DesktopAppNotification::Action activationButton) const;
    void redirectToPayBusiness(DesktopAppNotification::Action activationButton) const;
    void actionPressedOnDownloadFinishedTransferNotification(DesktopAppNotification::Action action) const;
    void actionPressedOnUploadFinishedTransferNotification(DesktopAppNotification::Action action) const;
    void actionPressedOnDownloadSetFinished(DesktopAppNotification::Action action);
    void viewShareOnWebClient() const;
    void viewShareOnWebClientByHandle(const QString &nodeBase64Handle) const;
    void getRemoteNodeLink(const QList<std::shared_ptr<mega::MegaNode> > &nodes) const;
    void receiveClusteredAlert(mega::MegaUserAlert* alert, const QString &message) const;
    void replyNewShareReceived(DesktopAppNotification::Action action) const;
    void viewOnInfoDialogNotifications(DesktopAppNotification::Action action) const;

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

    void processAlert(mega::MegaUserAlert* alert, const QString& email = QString());
    DesktopAppNotification* CreateContactNotification(const QString& title,
                                               const QString& message,
                                               const QString& email, const QStringList &actions = QStringList());

    std::unique_ptr<Notificator> mNotificator;
    QString mNewContactIconPath, mStorageQuotaFullIconPath, mStorageQuotaWarningIconPath;
    QString mFolderIconPath, mFileDownloadSucceedIconPath;
    NotificationDelayer mDelayedNotificator;
    std::shared_ptr<Preferences> mPreferences;
    bool mIsFirstTime;//Check first time alerts are added to show unified message of unread.

    QMultiMap<QString, mega::MegaUserAlert*> mPendingUserAlerts;
    QString mSetDownloadPath;
};

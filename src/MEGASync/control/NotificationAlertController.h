#ifndef NOTIFICATION_ALERT_CONTROLLER_H
#define NOTIFICATION_ALERT_CONTROLLER_H

#include "NotificationAlertModel.h"
#include "NotificationAlertDelegate.h"
#include "NotificationAlertProxyModel.h"

#include "megaapi.h"
#include "QTMegaRequestListener.h"
#include "QTMegaGlobalListener.h"

#include <QMap>

class NotificationAlertController : public QObject, public mega::MegaRequestListener, public mega::MegaGlobalListener
{
    Q_OBJECT

public:
    explicit NotificationAlertController(QObject* parent = nullptr);
    virtual ~NotificationAlertController() = default;

    void onRequestFinish(mega::MegaApi* api, mega::MegaRequest* request, mega::MegaError* e) override;
    void onUserAlertsUpdate(mega::MegaApi* api, mega::MegaUserAlertList* list) override;

    void reset();
    bool hasNotificationsOrAlerts();
    bool hasAlertsOfType(int type);
    void applyNotificationFilter(AlertType opt);
    void requestNotifications() const;
    void ackSeenAlertsAndNotifications();

signals:
    void userAlertsUpdated(mega::MegaUserAlertList* list);
    void notificationAlertCreated(NotificationAlertProxyModel* model, NotificationAlertDelegate* delegate);
    void unseenAlertsChanged(const QMap<AlertModel::AlertType, long long>& alerts);

private:
    mega::MegaApi* mMegaApi;
    std::unique_ptr<mega::QTMegaRequestListener> mDelegateListener;
    std::unique_ptr<mega::QTMegaGlobalListener> mGlobalListener;
    std::unique_ptr<NotificationAlertModel> mNotificationAlertModel;
    std::unique_ptr<NotificationAlertProxyModel> mAlertsProxyModel;
    std::unique_ptr<NotificationAlertDelegate> mNotificationAlertDelegate;
    long long mAllUnseenAlerts;

    void populateUserAlerts(mega::MegaUserAlertList* alertList);
    void populateNotifications(const mega::MegaNotificationList* notificationList);
    bool createModelAndDelegate();
    void checkUseenNotifications();
    bool areAlertsFiltered();

};

#endif // NOTIFICATION_ALERT_CONTROLLER_H

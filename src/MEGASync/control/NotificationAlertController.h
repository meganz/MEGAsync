#ifndef NOTIFICATION_ALERT_CONTROLLER_H
#define NOTIFICATION_ALERT_CONTROLLER_H

#include "NotificationAlertTypes.h"
//#include "NotificationAlertModel.h"
//#include "NotificationAlertProxyModel.h"

#include "megaapi.h"
#include "QTMegaRequestListener.h"
#include "QTMegaGlobalListener.h"

//#include <QMap>
#include <QAbstractItemModel>

class NotificationAlertModel;
class NotificationAlertProxyModel;

class NotificationAlertController : public QObject, public mega::MegaRequestListener, public mega::MegaGlobalListener
{
    Q_OBJECT

public:
    explicit NotificationAlertController(QObject* parent = nullptr);
    virtual ~NotificationAlertController() = default;

    void onRequestFinish(mega::MegaApi* api, mega::MegaRequest* request, mega::MegaError* e) override;
    void onUserAlertsUpdate(mega::MegaApi* api, mega::MegaUserAlertList* list) override;

    void reset();
    bool hasNotifications();
    bool hasElementsOfType(AlertType type);
    void applyFilter(AlertType type);
    void requestNotifications() const;
    void ackSeenAlertsAndNotifications();
    QAbstractItemModel* getModel() const;

signals:
    void userAlertsUpdated(mega::MegaUserAlertList* list);
    //void unseenAlertsChanged(const QMap<AlertModel::AlertType, long long>& alerts);

private:
    mega::MegaApi* mMegaApi;
    std::unique_ptr<mega::QTMegaRequestListener> mDelegateListener;
    std::unique_ptr<mega::QTMegaGlobalListener> mGlobalListener;
    std::unique_ptr<NotificationAlertModel> mNotificationAlertModel;
    std::unique_ptr<NotificationAlertProxyModel> mAlertsProxyModel;
    long long mAllUnseenAlerts;

    void populateUserAlerts(mega::MegaUserAlertList* alertList);
    void checkUseenNotifications();

};

#endif // NOTIFICATION_ALERT_CONTROLLER_H

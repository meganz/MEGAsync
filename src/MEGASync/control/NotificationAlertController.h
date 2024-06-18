#ifndef NOTIFICATIONALERTCONTROLLER_H
#define NOTIFICATIONALERTCONTROLLER_H

#include "NotificationDelegate.h"
#include "NotificationAlertModel.h"
#include "NotificationAlertDelegate.h"

#include "megaapi.h"
#include "mega/bindings/qt/QTMegaGlobalListener.h"

#include <QMap>

class NotificationAlertController : public QObject, public mega::MegaGlobalListener
{
    Q_OBJECT

public:
    explicit NotificationAlertController(QObject *parent = nullptr);
    virtual ~NotificationAlertController() = default;

    void onUserAlertsUpdate(mega::MegaApi *api, mega::MegaUserAlertList *list) override;

    void reset();
    bool alertsAreFiltered();
    bool hasAlerts();
    bool hasAlertsOfType(int type);
    void applyNotificationFilter(NotificationAlertProxyModel::FilterType opt);

signals:
    void userAlertsUpdated(mega::MegaUserAlertList* list);
    void notificationAlertCreated(NotificationAlertProxyModel* model, NotificationAlertDelegate* delegate);
    void unseenAlertsChanged(const QMap<AlertModel::AlertType, long long>& alerts);

private:
    mega::MegaApi* mMegaApi;
    std::unique_ptr<mega::QTMegaGlobalListener> mGlobalListener;

    std::unique_ptr<NotificationAlertModel> mNotificationAlertModel;
    std::unique_ptr<NotificationAlertProxyModel> mAlertsProxyModel;
    std::unique_ptr<NotificationAlertDelegate> mNotificationAlertDelegate;

    void populateUserAlerts(mega::MegaUserAlertList* alertList, bool copyRequired);

};

#endif // NOTIFICATIONALERTCONTROLLER_H

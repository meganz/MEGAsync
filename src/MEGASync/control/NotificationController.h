#ifndef NOTIFICATIONCONTROLLER_H
#define NOTIFICATIONCONTROLLER_H

#include "MegaNotificationDelegate.h"
#include "NotificationAlertModel.h"
#include "NotificationAlertDelegate.h"

#include "megaapi.h"
#include "mega/bindings/qt/QTMegaGlobalListener.h"

#include <QObject>

class NotificationController : public QObject, public mega::MegaGlobalListener
{
    Q_OBJECT

public:
    explicit NotificationController(QObject *parent = nullptr);
    virtual ~NotificationController() = default;

    void onUserAlertsUpdate(mega::MegaApi *api, mega::MegaUserAlertList *list) override;

    void populateUserAlerts(mega::MegaUserAlertList* alertList, bool copyRequired);
    bool alertsAreFiltered();
    bool hasAlerts();
    bool hasAlertsOfType(int type);

public slots:
    void applyNotificationFilter(QFilterAlertsModel::FilterType opt);

signals:
    void userAlertsUpdated(mega::MegaUserAlertList* list);
    void notificationAlertCreated(QFilterAlertsModel* model, NotificationAlertDelegate* delegate);
    void unseenAlertsChanged(const QMap<QAlertsModel::AlertType, long long>& alerts);

private:
    mega::MegaApi * mMegaApi;
    std::unique_ptr<mega::QTMegaGlobalListener> mGlobalListener;

    NotificationAlertModel* mNotificationAlertModel;
    QFilterAlertsModel* mAlertsProxyModel;
    NotificationAlertDelegate* mNotificationAlertDelegate;

};

#endif // NOTIFICATIONCONTROLLER_H

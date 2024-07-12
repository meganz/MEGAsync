#ifndef NOTIFICATION_ALERT_TYPES_H
#define NOTIFICATION_ALERT_TYPES_H

#include <QMap>

enum class AlertType
{
    UNKNOWN = 0,
    ALL,
    ALERT_CONTACTS,
    ALERT_SHARES,
    ALERT_PAYMENTS,
    ALERT_TAKEDOWNS,
    NOTIFICATIONS
};

typedef QMap<AlertType, long long> UnseenNotificationsMap;

#endif // NOTIFICATION_ALERT_TYPES_H

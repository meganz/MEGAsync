#ifndef NOTIFICATION_ALERT_TYPES_H
#define NOTIFICATION_ALERT_TYPES_H

#include <QMap>

enum class UserMessageType
{
    UNKNOWN = 0,
    ALL,
    ALERT_CONTACTS,
    ALERT_SHARES,
    ALERT_PAYMENTS,
    ALERT_TAKEDOWNS,
    NOTIFICATIONS
};

typedef QMap<UserMessageType, long long> UnseenUserMessagesMap;

#endif // NOTIFICATION_ALERT_TYPES_H

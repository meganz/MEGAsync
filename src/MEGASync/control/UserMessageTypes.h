#ifndef USER_MESSAGE_TYPES_H
#define USER_MESSAGE_TYPES_H

#include <QMap>

enum class MessageType
{
    UNKNOWN = 0,
    ALL,
    ALERT_CONTACTS,
    ALERT_SHARES,
    ALERT_PAYMENTS,
    ALERT_TAKEDOWNS,
    NOTIFICATIONS
};

typedef QMap<MessageType, long long> UnseenUserMessagesMap;

#endif // USER_MESSAGE_TYPES_H

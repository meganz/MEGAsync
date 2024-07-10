#ifndef NOTIFICATION_ALERT_TYPES_H
#define NOTIFICATION_ALERT_TYPES_H

enum class AlertType
{
    ALL = 0,
    CONTACTS,
    SHARES,
    PAYMENTS,
    TAKEDOWNS
};

struct NotificationAlertModelItem
{
    enum ModelType
    {
        NONE = 0,
        ALERT,
        NOTIFICATION
    };

    ModelType type;
    void* pointer;
};

#endif // NOTIFICATION_ALERT_TYPES_H

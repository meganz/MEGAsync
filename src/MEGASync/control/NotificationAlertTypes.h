#ifndef NOTIFICATIONALERTTYPES_H
#define NOTIFICATIONALERTTYPES_H

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

#endif // NOTIFICATIONALERTTYPES_H

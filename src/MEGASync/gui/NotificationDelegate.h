#ifndef NOTIFICATION_DELEGATE_H
#define NOTIFICATION_DELEGATE_H

#include <QWidget>
#include <QCache>

class NotificationItem;
class UserNotification;

class NotificationDelegate
{

public:
    NotificationDelegate();

    QWidget* getWidget(UserNotification* notification, QWidget* parent);

private:
    QCache<int64_t, NotificationItem> mItems;

};

#endif // NOTIFICATION_DELEGATE_H

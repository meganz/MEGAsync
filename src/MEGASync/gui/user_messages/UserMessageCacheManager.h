#ifndef USER_MESSAGE_CACHE_MANAGER_H
#define USER_MESSAGE_CACHE_MANAGER_H

#include <QWidget>
#include <QCache>

class UserMessage;
class AlertItem;
class UserAlert;
class NotificationItem;
class UserNotification;

class UserMessageCacheManager
{

public:
    UserMessageCacheManager();
    QWidget* getWidget(UserMessage* item, QWidget* parent);

private:
    QCache<unsigned, AlertItem> mAlertItems;
    QCache<int64_t, NotificationItem> mNotificationItems;

    template<class UserMessageChild, class ItemType, typename IdType>
    QWidget* getWidget(UserMessageChild* itemData, IdType id, QCache<IdType, ItemType>* target, QWidget* parent);

};

#endif // USER_MESSAGE_CACHE_MANAGER_H

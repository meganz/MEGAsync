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
    QWidget* getWidget(int row, UserMessage* item, QWidget* parent);

private:
    QCache<int, QWidget> mUserMessageItems;


    QWidget* getWidgetFromCache(int cacheIndex);
};

#endif // USER_MESSAGE_CACHE_MANAGER_H

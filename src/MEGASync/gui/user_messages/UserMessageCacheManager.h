#ifndef USER_MESSAGE_CACHE_MANAGER_H
#define USER_MESSAGE_CACHE_MANAGER_H

#include <QWidget>
#include <QCache>

class UserMessage;
class UserMessageWidget;

class UserMessageCacheManager
{

public:
    UserMessageCacheManager();
    virtual ~UserMessageCacheManager() = default;

    QWidget* getWidget(int row, UserMessage* data, QWidget* parent);

private:
    QCache<int, UserMessageWidget> mUserMessageItems;

    template<class Item>
    QWidget* createOrGetWidget(int cacheIndex, UserMessage* data, QWidget* parent);
    UserMessageWidget* getWidgetFromCache(int cacheIndex);

};

#endif // USER_MESSAGE_CACHE_MANAGER_H

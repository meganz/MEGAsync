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

    UserMessageWidget* createOrGetWidget(int row,
                                         UserMessage* data,
                                         QWidget* parent,
                                         bool& isNew);

private:
    QCache<int, UserMessageWidget> mUserMessageItems;

    template<class Item>
    UserMessageWidget* createOrGetWidget(int cacheIndex,
                                         UserMessage* data,
                                         QWidget* parent,
                                         bool& isNew);

    UserMessageWidget* getWidgetFromCache(int cacheIndex);

};

#endif // USER_MESSAGE_CACHE_MANAGER_H

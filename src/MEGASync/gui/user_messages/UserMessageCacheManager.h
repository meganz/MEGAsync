#ifndef USER_MESSAGE_CACHE_MANAGER_H
#define USER_MESSAGE_CACHE_MANAGER_H

#include <QCache>
#include <QWidget>

class UserMessage;
class UserMessageWidget;

class UserMessageCacheManager
{

public:
    UserMessageCacheManager();
    virtual ~UserMessageCacheManager() = default;

    UserMessageWidget* createOrGetWidget(qsizetype row, UserMessage* data, QWidget* parent);

private:
    using WidgetCache = QCache<qsizetype, UserMessageWidget>;

    // One cache per type of user message. A widget can only display the type of
    // message it was built for, so with a single cache shared by every type, two
    // rows of different types competing for the same entry keep replacing each
    // other's widget, and replacing an entry destroys the widget stored in it.
    WidgetCache mAlertItems;
    WidgetCache mNotificationItems;

    template<class Item>
    UserMessageWidget*
        createOrGetWidget(WidgetCache& cache, qsizetype row, UserMessage* data, QWidget* parent);
};

#endif // USER_MESSAGE_CACHE_MANAGER_H

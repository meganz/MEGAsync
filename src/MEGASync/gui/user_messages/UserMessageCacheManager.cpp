#include "UserMessageCacheManager.h"

#include "AlertItem.h"
#include "UserAlert.h"
#include "NotificationItem.h"
#include "UserNotification.h"

namespace
{
constexpr int MaxCost = 16;
}

using namespace mega;

UserMessageCacheManager::UserMessageCacheManager()
    : mUserMessageItems(MaxCost)
{
}

QWidget *UserMessageCacheManager::getWidget(int row, UserMessage* data, QWidget* parent)
{
    if(!data)
    {
        return nullptr;
    }

    QWidget* widget(nullptr);
    auto cacheIndex(row % mUserMessageItems.maxCost());

    switch (data->getType())
    {
        case UserMessage::Type::ALERT:
        {
            UserAlert* alert = dynamic_cast<UserAlert*>(data);
            AlertItem* item = dynamic_cast<AlertItem*>(getWidgetFromCache(cacheIndex));
            if(item)
            {
                if(item->getData()->id() != alert->id())
                {
                    item->setAlertData(alert);
                }
            }
            else
            {
                item = new AlertItem(alert, parent);
                mUserMessageItems.insert(cacheIndex, item);
            }

            widget = item;

            break;
        }
        case UserMessage::Type::NOTIFICATION:
        {
            UserNotification* notification = dynamic_cast<UserNotification*>(data);
            NotificationItem* item = dynamic_cast<NotificationItem*>(getWidgetFromCache(cacheIndex));
            if(item)
            {
                if(item->getData()->id() != notification->id())
                {
                    item->setNotificationData(notification);
                }
            }
            else
            {
                item = new NotificationItem(notification, parent);
                mUserMessageItems.insert(cacheIndex, item);
            }

            widget = item;

            break;
        }
        default:
        {
            break;
        }
    }

    if(widget)
    {
        widget->show();
        widget->hide();
        data->setSizeHint(widget->sizeHint());
    }

    return widget;
}

QWidget* UserMessageCacheManager::getWidgetFromCache(int cacheIndex)
{
    QWidget* item(nullptr);

    if(cacheIndex < mUserMessageItems.size())
    {
        item = mUserMessageItems.object(cacheIndex);
    }

    return item;
}

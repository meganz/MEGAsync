#include "UserMessageCacheManager.h"

#include "AlertItem.h"
#include "UserAlert.h"
#include "NotificationItem.h"
#include "UserNotification.h"

namespace
{
constexpr int MAX_COST = 16;
}

using namespace mega;

UserMessageCacheManager::UserMessageCacheManager()
    : mUserMessageItems(MAX_COST)
{
}

UserMessageWidget* UserMessageCacheManager::createOrGetWidget(int row,
                                                              UserMessage* data,
                                                              QWidget* parent)
{
    if(!data)
    {
        return nullptr;
    }

    UserMessageWidget* widget(nullptr);
    auto cacheIndex(row % mUserMessageItems.maxCost());

    switch (data->getType())
    {
        case UserMessage::Type::ALERT:
        {
            widget = createOrGetWidget<AlertItem>(cacheIndex, data, parent);
            break;
        }
        case UserMessage::Type::NOTIFICATION:
        {
            widget = createOrGetWidget<NotificationItem>(cacheIndex, data, parent);
            break;
        }
        default:
        {
            break;
        }
    }

    return widget;
}

template<class Item>
UserMessageWidget* UserMessageCacheManager::createOrGetWidget(int cacheIndex,
                                                              UserMessage* data,
                                                              QWidget* parent)
{
    bool newDataInWidget(false);

    UserMessageWidget* widget = getWidgetFromCache(cacheIndex);
    if(dynamic_cast<Item*>(widget))
    {
        if(!widget->getData() || !data->hasSameId(widget->getData()->id()))
        {
            widget->setData(data);
            newDataInWidget = true;
        }
    }
    else
    {
        widget = new Item(parent);
        widget->setData(data);
        mUserMessageItems.insert(cacheIndex, widget);
        newDataInWidget = true;
    }

    if (widget && newDataInWidget)
    {
        widget->show();
        widget->hide();
        data->setSizeHint(widget->sizeHint());
    }

    return widget;
}

UserMessageWidget* UserMessageCacheManager::getWidgetFromCache(int cacheIndex)
{
    UserMessageWidget* item(nullptr);

    if(cacheIndex < mUserMessageItems.size())
    {
        item = mUserMessageItems.object(cacheIndex);
    }

    return item;
}

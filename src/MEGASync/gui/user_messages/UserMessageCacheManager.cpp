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

UserMessageWidget* UserMessageCacheManager::createOrGetWidget(int row,
                                                              UserMessage* data,
                                                              QWidget* parent,
                                                              bool& isNew)
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
            widget = createOrGetWidget<AlertItem>(cacheIndex, data, parent, isNew);
            break;
        }
        case UserMessage::Type::NOTIFICATION:
        {
            widget = createOrGetWidget<NotificationItem>(cacheIndex, data, parent, isNew);
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
                                                              QWidget* parent,
                                                              bool& isNew)
{
    UserMessageWidget* widget = getWidgetFromCache(cacheIndex);
    if(dynamic_cast<Item*>(widget))
    {
        if(!data->hasSameId(widget->getData()->id()))
        {
            widget->setData(data);
        }
        isNew = false;
    }
    else
    {
        widget = new Item(parent);
        widget->setData(data);
        mUserMessageItems.insert(cacheIndex, widget);
        isNew = true;
    }

    if(widget)
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

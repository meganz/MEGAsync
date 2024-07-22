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
QWidget* UserMessageCacheManager::createOrGetWidget(int cacheIndex, UserMessage* data, QWidget* parent)
{
    UserMessageWidget* widget = getWidgetFromCache(cacheIndex);
    if(dynamic_cast<Item*>(widget))
    {
        auto id = widget->getData()->id();
        if(!data->hasSameId(widget->getData()->id()))
        {
            widget->setData(data);
        }
    }
    else
    {
        widget = new Item(parent);
        widget->setData(data);
        mUserMessageItems.insert(cacheIndex, widget);
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

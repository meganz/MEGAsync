#include "UserMessageCacheManager.h"

#include "AlertItem.h"
#include "NotificationItem.h"
#include "TokenParserWidgetManager.h"

namespace
{
constexpr int MAX_COST = 16;
}

using namespace mega;

UserMessageCacheManager::UserMessageCacheManager()
    : mUserMessageItems(MAX_COST)
{
}

UserMessageWidget* UserMessageCacheManager::createOrGetWidget(qsizetype row,
                                                              UserMessage* data,
                                                              QWidget* parent)
{
    if(!data)
    {
        return nullptr;
    }

    UserMessageWidget* widget(nullptr);
    qsizetype cacheIndex(row % mUserMessageItems.maxCost());

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
UserMessageWidget* UserMessageCacheManager::createOrGetWidget(qsizetype cacheIndex,
                                                              UserMessage* data,
                                                              QWidget* parent)
{
    bool newDataInWidget(false);

    UserMessageWidget* widget = getWidgetFromCache(cacheIndex);
    if(dynamic_cast<Item*>(widget))
    {
        if(!widget->getData() || !data->hasSameId(widget->getData()->id()))
        {
            data->clearSizeHint();
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
        TokenParserWidgetManager::instance()->polish(widget);
        widget->hide();
    }

    return widget;
}

UserMessageWidget* UserMessageCacheManager::getWidgetFromCache(qsizetype cacheIndex)
{
    // QCache::size() is the number of stored objects, not the size of the key
    // space, so comparing the index against it discards valid entries. No guard
    // is needed: QCache::object() already returns null for a key not stored.
    return mUserMessageItems.object(cacheIndex);
}

#include "UserMessageCacheManager.h"

#include "AlertItem.h"
#include "NotificationItem.h"
#include "TokenParserWidgetManager.h"

namespace
{
constexpr int MAX_COST = 16;
}

using namespace mega;

UserMessageCacheManager::UserMessageCacheManager():
    mAlertItems(MAX_COST),
    mNotificationItems(MAX_COST)
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

    switch (data->getType())
    {
        case UserMessage::Type::ALERT:
        {
            widget = createOrGetWidget<AlertItem>(mAlertItems, row, data, parent);
            break;
        }
        case UserMessage::Type::NOTIFICATION:
        {
            widget = createOrGetWidget<NotificationItem>(mNotificationItems, row, data, parent);
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
UserMessageWidget* UserMessageCacheManager::createOrGetWidget(WidgetCache& cache,
                                                              qsizetype row,
                                                              UserMessage* data,
                                                              QWidget* parent)
{
    const qsizetype cacheIndex(row % cache.maxCost());

    bool newDataInWidget(false);

    // QCache::object() returns null when nothing is stored for the key. The key
    // must not be compared against QCache::size(), which is the number of stored
    // objects and not the size of the key space.
    UserMessageWidget* widget = cache.object(cacheIndex);
    if (widget)
    {
        // Each cache only ever stores one type of widget, so an entry always fits
        // the data it is asked to display.
        Q_ASSERT(dynamic_cast<Item*>(widget) != nullptr);

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
        cache.insert(cacheIndex, widget);
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

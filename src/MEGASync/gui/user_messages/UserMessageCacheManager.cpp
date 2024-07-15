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
    : mAlertItems(MaxCost)
    , mNotificationItems(MaxCost)
{
}

QWidget *UserMessageCacheManager::getWidget(UserMessage* item, QWidget* parent)
{
    if(!item)
    {
        return nullptr;
    }

    QWidget* widget = nullptr;
    switch (item->getType())
    {
        case UserMessage::Type::ALERT:
        {
            UserAlert* alert = dynamic_cast<UserAlert*>(item);
            widget = getWidget<UserAlert, AlertItem, unsigned>(alert,
                                                               alert->getId(),
                                                               &mAlertItems,
                                                               parent);
            break;
        }
        case UserMessage::Type::NOTIFICATION:
        {
            UserNotification* notification = dynamic_cast<UserNotification*>(item);
            widget = getWidget<UserNotification, NotificationItem, int64_t>(notification,
                                                                            notification->getID(),
                                                                            &mNotificationItems,
                                                                            parent);
            break;
        }
        default:
        {
            break;
        }
    }

    return widget;
}

template<class UserMessageChild, class ItemType, typename IdType>
QWidget* UserMessageCacheManager::getWidget(UserMessageChild* itemData,
                                            IdType id,
                                            QCache<IdType, ItemType>* target,
                                            QWidget* parent)
{
    if(!itemData)
    {
        return nullptr;
    }

    ItemType* item = target->object(id);
    if(item)
    {
        return item;
    }

    item = new ItemType(itemData, parent);
    item->show();
    item->hide();
    target->insert(id, item);
    return target->object(id);
}

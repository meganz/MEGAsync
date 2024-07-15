#include "NotificationDelegate.h"

#include "NotificationItem.h"
#include "UserNotification.h"

namespace
{
constexpr int MaxCost = 16;
}

using namespace mega;

NotificationDelegate::NotificationDelegate()
    : mItems(MaxCost)
{
}

QWidget* NotificationDelegate::getWidget(UserNotification* notification, QWidget* parent)
{
    if(!notification)
    {
        return nullptr;
    }

    auto notificationItem = mItems[notification->getID()];
    if(notificationItem)
    {
        return notificationItem;
    }
    else
    {
        NotificationItem* item = new NotificationItem(notification, parent);
        item->show();
        item->hide();
        mItems.insert(notification->getID(), item);
        return mItems[notification->getID()];
    }
}

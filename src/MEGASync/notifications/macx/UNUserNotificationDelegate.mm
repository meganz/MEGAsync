#include "NotificationDelegate.h"
#include "notificator.h"

@implementation UNUserNotificationDelegate

- (void)userNotificationCenter:(UNUserNotificationCenter *)center
  didReceiveNotificationResponse:(UNNotificationResponse *)response
  withCompletionHandler:(void (^)(void))completionHandler
{
    int64_t notificationKey = [response.notification.request.content.categoryIdentifier longLongValue];
    int64_t  activationType = [response.actionIdentifier integerValue];

    QHash<int64_t, MegaNotification*>::iterator it
           = Notificator::notifications.find(notificationKey);
    if (it == Notificator::notifications.end())
    {
        return;
    }

    MegaNotification* n = it.value();
    Notificator::notifications.erase(it);

    if (![response.actionIdentifier isEqualToString:UNNotificationDismissActionIdentifier])
    {
        emit n->activated((MegaNotification::Action) activationType);
    }
    else
    {
        emit n->closed(MegaNotification::CloseReason::Unknown);
    }

    completionHandler();
}

@end

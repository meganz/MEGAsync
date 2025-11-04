#include "NotificationDelegate.h"
#include "Notificator.h"

@implementation UNUserNotificationDelegate

- (void)userNotificationCenter:(UNUserNotificationCenter *)center
  didReceiveNotificationResponse:(UNNotificationResponse *)response
  withCompletionHandler:(void (^)(void))completionHandler
{
    int64_t notificationKey = [response.notification.request.content.categoryIdentifier longLongValue];
    int64_t  activationType = [response.actionIdentifier integerValue];

    QHash<int64_t, DesktopAppNotification*>::iterator it
           = Notificator::notifications.find(notificationKey);
    if (it == Notificator::notifications.end())
    {
        return;
    }

    DesktopAppNotification* n = it.value();
    Notificator::notifications.erase(it);

    if (![response.actionIdentifier isEqualToString:UNNotificationDismissActionIdentifier])
    {
        emit n->activated((DesktopAppNotification::Action) activationType);
    }
    else
    {
        emit n->closed(DesktopAppNotification::CloseReason::Unknown);
    }

    completionHandler();
}


- (void)userNotificationCenter:(UNUserNotificationCenter *)center
       willPresentNotification:(UNNotification *)notification
         withCompletionHandler:(void (^)(UNNotificationPresentationOptions options))completionHandler
{

    // Show notificatoin even when foreground.
    // On macOS 12+ prefer Banner/List; fall back to Alert on older systems.
    UNNotificationPresentationOptions opts = 0;
    if (@available(macOS 12.0, *)) {
        opts = UNNotificationPresentationOptionBanner |
               UNNotificationPresentationOptionList   |
               UNNotificationPresentationOptionSound;
    } else {
        // Older macOS that still uses "Alert"
        opts = UNNotificationPresentationOptionAlert |
               UNNotificationPresentationOptionSound;
    }

    completionHandler(opts);
}


@end

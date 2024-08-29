#include "NotificationDelegate.h"
#include "Notificator.h"
#include <Cocoa/Cocoa.h>

@implementation NSUserNotificationDelegate

- (BOOL)userNotificationCenter:(NSUserNotificationCenter *)center shouldPresentNotification:(NSUserNotification *)notification{
    return YES;
}

- (void)userNotificationCenter:(NSUserNotificationCenter *)center
      didActivateNotification:(NSUserNotification *)notification {

    int64_t notificationKey = [[notification identifier] longLongValue];
    QHash<int64_t, DesktopAppNotification*>::iterator it
           = Notificator::notifications.find(notificationKey);
    if (it == Notificator::notifications.end())
    {
        return;
    }

    DesktopAppNotification* n = it.value();
    Notificator::notifications.erase(it);
    switch (notification.activationType)
    {
        case NSUserNotificationActivationTypeContentsClicked:
            emit n->activated(DesktopAppNotification::Action::content);
            break;

        case NSUserNotificationActivationTypeActionButtonClicked:
            if (n->getActions().size() > 0)
            {
                emit n->activated(DesktopAppNotification::Action::firstButton);
            }
            else
            {
                emit n->closed(DesktopAppNotification::CloseReason::Unknown);
            }
            break;

        case NSUserNotificationActivationTypeAdditionalActionClicked:
            if (n->getActions().size() > 1)
            {
                emit n->activated(DesktopAppNotification::Action::secondButton);
            }
            else
            {
                emit n->closed(DesktopAppNotification::CloseReason::Unknown);
            }
            break;

       default:
            emit n->closed(DesktopAppNotification::CloseReason::Unknown);
            break;
    }

    NSUserNotificationCenter *notificationCenterInstance = [NSUserNotificationCenter defaultUserNotificationCenter];
    [notificationCenterInstance removeDeliveredNotification:notification];
}

- (void)userNotificationCenter:(NSUserNotificationCenter *)center didDismissAlert:(NSUserNotification *)notification
{
    [self closeAlert:notification];
}

- (void)closeAlert:(NSUserNotification *)notification
{
    int64_t notificationKey = [[notification identifier] longLongValue];
    QHash<int64_t, DesktopAppNotification*>::iterator it
           = Notificator::notifications.find(notificationKey);
    if (it == Notificator::notifications.end())
    {
        return;
    }

    DesktopAppNotification* n = it.value();
    Notificator::notifications.erase(it);

    emit n->closed(DesktopAppNotification::CloseReason::Unknown);
    NSUserNotificationCenter *notificationCenterInstance = [NSUserNotificationCenter defaultUserNotificationCenter];
    [notificationCenterInstance removeDeliveredNotification:notification];
}

@end

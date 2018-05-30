#include "NotificationDelegate.h"
#include "notificator.h"
#include <Cocoa/Cocoa.h>

@implementation NotificationDelegate

- (BOOL)userNotificationCenter:(NSUserNotificationCenter *)center shouldPresentNotification:(NSUserNotification *)notification{
    return YES;
}

- (void)userNotificationCenter:(NSUserNotificationCenter *)center
      didActivateNotification:(NSUserNotification *)notification {

    int64_t notificationKey = [[notification identifier] longLongValue];
    QHash<int64_t, MegaNotification *>::iterator it
           = Notificator::notifications.find(notificationKey);
    if (it == Notificator::notifications.end())
    {
        return;
    }

    MegaNotification *n = it.value();
    Notificator::notifications.erase(it);
    switch (notification.activationType)
    {
        case NSUserNotificationActivationTypeContentsClicked:
            emit n->activated(MegaNotification::ActivationContentClicked);
            break;

        case NSUserNotificationActivationTypeActionButtonClicked:
            if (n->getActions().size() > 0)
            {
                emit n->activated(MegaNotification::ActivationActionButtonClicked);
            }
            else
            {
                emit n->closed(MegaNotification::ActivationActionButtonClicked);
            }
            break;

        case NSUserNotificationActivationTypeAdditionalActionClicked:
            if (n->getActions().size() > 1)
            {
                emit n->activated(MegaNotification::ActivationAdditionalButtonClicked);
            }
            else
            {
                emit n->closed(MegaNotification::ActivationActionButtonClicked);
            }
            break;

       default:
            emit n->closed(MegaNotification::ActivationActionButtonClicked);
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
    QHash<int64_t, MegaNotification *>::iterator it
           = Notificator::notifications.find(notificationKey);
    if (it == Notificator::notifications.end())
    {
        return;
    }

    MegaNotification *n = it.value();
    Notificator::notifications.erase(it);

    emit n->closed(0);
    NSUserNotificationCenter *notificationCenterInstance = [NSUserNotificationCenter defaultUserNotificationCenter];
    [notificationCenterInstance removeDeliveredNotification:notification];
}

@end

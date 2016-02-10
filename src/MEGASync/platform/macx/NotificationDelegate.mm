#include "NotificationDelegate.h"
#include <Cocoa/Cocoa.h>

@implementation NotificationDelegate

- (BOOL)userNotificationCenter:(NSUserNotificationCenter *)center shouldPresentNotification:(NSUserNotification *)notification{
    return YES;
}

@end

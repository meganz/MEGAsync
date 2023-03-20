#ifndef NOTIFICATIONDELEGATE_H
#define NOTIFICATIONDELEGATE_H

#endif // NOTIFICATIONDELEGATE_H

#import <Cocoa/Cocoa.h>
#import <UserNotifications/UserNotifications.h>

@interface NSUserNotificationDelegate : NSObject <NSUserNotificationCenterDelegate>
@end

@interface UNUserNotificationDelegate : NSObject <UNUserNotificationCenterDelegate>
@end

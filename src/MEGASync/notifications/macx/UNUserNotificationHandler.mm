// Copyright (c) 2011-2013 The Bitcoin Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "UNUserNotificationHandler.h"
#include "NotificationDelegate.h"

#undef slots
#include <UserNotifications/UserNotifications.h>

UNUserNotificationHandler::UNUserNotificationHandler()
{
    mNotificationDelegate = [[UNUserNotificationDelegate alloc] init];
}

void UNUserNotificationHandler::showNotification(MegaNotification *notification)
{
    static int64_t currentNotificationId = 1;
    
    UNUserNotificationCenter* center = [UNUserNotificationCenter currentNotificationCenter];
    [center setDelegate:(UNUserNotificationDelegate*) mNotificationDelegate];
    UNAuthorizationOptions options =  UNAuthorizationOptionSound;
    [center requestAuthorizationWithOptions:options completionHandler:^(BOOL granted, NSError * _Nullable error) {
        NSLog(@"%@", error);
    }];

    NSMutableArray<UNNotificationAction*>* actionButtons = [[NSMutableArray<UNNotificationAction*> alloc] init];

    QStringList actions = notification->getActions();
    for(int actionIndex = 0; actionIndex < actions.size(); ++actionIndex)
    {
        NSString* buttonText = [NSString stringWithUTF8String:actions.at(actionIndex).toUtf8().constData()];
        NSString* buttonIndex = [NSString stringWithFormat:@"%i", actionIndex];
        UNNotificationAction *ActionBtn1 = [UNNotificationAction actionWithIdentifier: buttonIndex  title: buttonText options:UNNotificationCategoryOptionCustomDismissAction];
        [actionButtons addObject:ActionBtn1];
    }

    NSString* categoryText = @(currentNotificationId).stringValue;
    UNNotificationCategory *category = [UNNotificationCategory categoryWithIdentifier:categoryText actions: actionButtons intentIdentifiers:@[] options:UNNotificationCategoryOptionCustomDismissAction];

    NSSet *categories = [NSSet setWithObject:category];
    [center setNotificationCategories:categories];

    UNMutableNotificationContent *content = [[UNMutableNotificationContent alloc] init];
    [content setTitle:[NSString stringWithUTF8String:notification->getTitle().toUtf8().constData()]];
    [content setBody:[NSString stringWithUTF8String:notification->getText().toUtf8().constData()]];
    [content setSound: [UNNotificationSound defaultSound]];
    [content setCategoryIdentifier:(NSString *) categoryText];

    Notificator::notifications[currentNotificationId] = notification;
    notification->setId(currentNotificationId);
    currentNotificationId++;

    UNTimeIntervalNotificationTrigger *trigger = [UNTimeIntervalNotificationTrigger
            triggerWithTimeInterval:1
            repeats:NO];
    
    UNNotificationRequest *request = [UNNotificationRequest requestWithIdentifier:categoryText
            content:content
            trigger:trigger];

    
    [center addNotificationRequest:request withCompletionHandler:^(NSError * _Nullable error) {
        if (error)
        {
            NSLog(@"%@", error);
        }
    }];
}

void UNUserNotificationHandler::hideNotification(MegaNotification *notification)
{
    NSString *idString = [[NSNumber numberWithLongLong:notification->getId()] stringValue];
    NSArray<NSString*>* arrayOfIds = [NSArray arrayWithObject: idString];
    UNUserNotificationCenter* center = [UNUserNotificationCenter currentNotificationCenter];
    [center removeDeliveredNotificationsWithIdentifiers:arrayOfIds];
    [center removePendingNotificationRequestsWithIdentifiers:arrayOfIds];
}

bool UNUserNotificationHandler::acceptsMultipleSelection()
{
    return true;
}

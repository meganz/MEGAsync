// Copyright (c) 2011-2013 The Bitcoin Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "macnotificationhandler.h"
#include "NotificationDelegate.h"

#undef slots
#include <Cocoa/Cocoa.h>

MacNotificationHandler::MacNotificationHandler()
{
    notificationDelegate = [[NotificationDelegate alloc] init];
}

void MacNotificationHandler::showNotification(MegaNotification *notification)
{
    static int64_t currentNotificationId = 1;

    NSUserNotification *userNotification = [[NSUserNotification alloc] init];
    [userNotification setTitle:[NSString stringWithUTF8String:notification->getTitle().toUtf8().constData()]];
    [userNotification setInformativeText:[NSString stringWithUTF8String:notification->getText().toUtf8().constData()]];

    QStringList actions = notification->getActions();
    [userNotification setHasActionButton:actions.size() != 0];
    if (actions.size())
    {
        [userNotification setActionButtonTitle:[NSString stringWithUTF8String:actions.at(0).toUtf8().constData()]];
        if (actions.size() > 1)
        {
            // Only two actions supported for now
            [userNotification setOtherButtonTitle:[NSString stringWithUTF8String:actions.at(1).toUtf8().constData()]];
        }
    }

    [userNotification setIdentifier:@(currentNotificationId).stringValue];
    notification->setId(currentNotificationId);
    Notificator::notifications[currentNotificationId] = notification;
    currentNotificationId++;

    //TODO: Migrate to UNUserNotificationCenter 10.14+
    NSUserNotificationCenter *notificationCenterInstance = [NSUserNotificationCenter defaultUserNotificationCenter];
    [notificationCenterInstance setDelegate:(NotificationDelegate *)notificationDelegate];
    [notificationCenterInstance deliverNotification:userNotification];

    int expirationTime = notification->getExpirationTime();
    if (expirationTime > 0)
    {
        [notificationDelegate performSelector:@selector(closeAlert:)
            withObject:(NotificationDelegate *)userNotification
            afterDelay: expirationTime / 1000.0];
    }

    [userNotification release];
}

// sendAppleScript just take a QString and executes it as apple script
void MacNotificationHandler::sendAppleScript(const QString &script)
{
    QByteArray utf8 = script.toUtf8();
    char* cString = (char *)utf8.constData();
    NSString *scriptApple = [[NSString alloc] initWithUTF8String:cString];

    NSAppleScript *as = [[NSAppleScript alloc] initWithSource:scriptApple];
    NSDictionary *err = nil;
    [as executeAndReturnError:&err];
    [as release];
    [scriptApple release];
}

MacNotificationHandler *MacNotificationHandler::instance()
{
    static MacNotificationHandler *s_instance = NULL;
    if (!s_instance)
        s_instance = new MacNotificationHandler();
    return s_instance;
}

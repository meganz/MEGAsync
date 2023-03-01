// Copyright (c) 2011-2013 The Bitcoin Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "NSUserNotificationHandler.h"
#include "NotificationDelegate.h"

#include <QOperatingSystemVersion>

#undef slots
#include <Cocoa/Cocoa.h>
#include <UserNotifications/UserNotifications.h>

NSUserNotificationHandler::NSUserNotificationHandler()
{
    mNotificationDelegate = [[NSUserNotificationDelegate alloc] init];
}

void NSUserNotificationHandler::showNotification(MegaNotification *notification)
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
    [notificationCenterInstance setDelegate:(NSUserNotificationDelegate *)mNotificationDelegate];
    [notificationCenterInstance deliverNotification:userNotification];

    [userNotification release];
}

bool NSUserNotificationHandler::acceptsMultipleSelection()
{
    return false;
}

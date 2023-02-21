// Copyright (c) 2011-2013 The Bitcoin Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "NotificationHandler.h"
#include "NotificationDelegate.h"

#include "UNUserNotificationHandler.h"
#include "NSUserNotificationHandler.h"

#include <QOperatingSystemVersion>

#undef slots
#include <Cocoa/Cocoa.h>
#include <UserNotifications/UserNotifications.h>

NotificationHandler::NotificationHandler()
{
}

// sendAppleScript just take a QString and executes it as apple script
void NotificationHandler::sendAppleScript(const QString &script)
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

NotificationHandler *NotificationHandler::instance()
{
    auto current = QOperatingSystemVersion::current();
    if (current > QOperatingSystemVersion::MacOSHighSierra) //New Notification API support from 10.14+
    {
        static UNUserNotificationHandler instance;
        return &instance;
    }
    else
    {
        static NSUserNotificationHandler instance;
        return &instance;
    }
}

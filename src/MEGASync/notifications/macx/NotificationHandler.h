// Copyright (c) 2011-2013 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef NOTIFICATIONHANDLER_H
#define NOTIFICATIONHANDLER_H

#include <QObject>
#include "notificator.h"

/** Macintosh-specific notification handler (supports UserNotificationCenter and Growl).
 */
class NotificationHandler : public QObject
{
    Q_OBJECT

public:
    NotificationHandler();

    virtual void showNotification(MegaNotification *notification) = 0;
    virtual void hideNotification(MegaNotification *notification) = 0;
    virtual bool acceptsMultipleSelection() = 0;

    /** executes AppleScript */
    void sendAppleScript(const QString &script);

    static NotificationHandler *instance();

protected:
    void *mNotificationDelegate;
};

#endif // NOTIFICATIONHANDLER_H

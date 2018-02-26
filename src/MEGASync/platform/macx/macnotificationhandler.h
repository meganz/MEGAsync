// Copyright (c) 2011-2013 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MACNOTIFICATIONHANDLER_H
#define MACNOTIFICATIONHANDLER_H

#include <QObject>
#include "notificator.h"

/** Macintosh-specific notification handler (supports UserNotificationCenter and Growl).
 */
class MacNotificationHandler : public QObject
{
    Q_OBJECT

public:
    MacNotificationHandler();

    void showNotification(MegaNotification *notification);

    /** executes AppleScript */
    void sendAppleScript(const QString &script);

    static MacNotificationHandler *instance();

private:
    void *notificationDelegate;
};




#endif // MACNOTIFICATIONHANDLER_H

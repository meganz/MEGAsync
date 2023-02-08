// Copyright (c) 2011-2013 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef NSUSERNOTIFICATIONHANDLER_H
#define NSUSERNOTIFICATIONHANDLER_H

#include <QObject>
#include "NotificationHandler.h"

/** Macintosh-specific notification handler (supports UserNotificationCenter and Growl).
 */
class NSUserNotificationHandler : public NotificationHandler
{
    Q_OBJECT

public:
    NSUserNotificationHandler();

    void showNotification(MegaNotification *notification) override;
    bool acceptsMultipleSelection() override;
};

#endif // NSUSERNOTIFICATIONHANDLER_H

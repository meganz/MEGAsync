// Copyright (c) 2011-2013 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef UNUSERNOTIFICATIONHANDLER_H
#define UNUSERNOTIFICATIONHANDLER_H

#include <QObject>
#include "NotificationHandler.h"

/** Macintosh-specific notification handler (supports UserNotificationCenter and Growl).
 */
class UNUserNotificationHandler : public NotificationHandler
{
    Q_OBJECT

public:
    UNUserNotificationHandler();

    void showNotification(MegaNotification *notification) override;
    bool acceptsMultipleSelection() override;
};

#endif // UNUSERNOTIFICATIONHANDLER_H

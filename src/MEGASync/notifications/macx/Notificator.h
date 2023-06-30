// Copyright (c) 2011-2013 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef NOTIFICATOR_H
#define NOTIFICATOR_H

#include "NotificatorBase.h"

class MegaNotification : public MegaNotificationBase
{
    Q_OBJECT

public:
    MegaNotification();
    ~MegaNotification();

    QStringList getActions() const override;
};

/** Cross-platform desktop notification client. */
class Notificator: public NotificatorBase
{
    Q_OBJECT

public:    
    static QHash<int64_t, MegaNotification*> notifications;

    /** Create a new notificator.
       @note Ownership of trayIcon is not transferred to this object.
    */
    Notificator(const QString &programName, QSystemTrayIcon *trayIcon, QObject *parent);
    ~Notificator() = default;

    void notify(Class cls, const QString &title, const QString &text, int millisTimeout = 10000);
    void notify(MegaNotification *notification);
};

#endif // NOTIFICATOR_H

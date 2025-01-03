// Copyright (c) 2011-2013 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef NOTIFICATOR_H
#define NOTIFICATOR_H

#include "NotificatorBase.h"

#include <QHash>
#include <QIcon>
#include <QMutex>
#include <QObject>
#include <QPointer>
#include <QVariant>

/** Cross-platform desktop notification client. */
class Notificator: public NotificatorBase
{
    Q_OBJECT

public:
    /** Create a new notificator.
       @note Ownership of trayIcon is not transferred to this object.
    */
    Notificator(const QString &programName, QSystemTrayIcon *trayIcon, QObject *parent);
    ~Notificator() = default;

    void notifySystray(Class cls, const QString &title, const QString &text, int millisTimeout, bool forceQt) override;
    void notifySystray(DesktopAppNotificationBase *notification) override;

protected slots:
    void onModernNotificationFailed();
};

class DesktopAppNotification : public DesktopAppNotificationBase
{
    Q_OBJECT

public:
    DesktopAppNotification();
    ~DesktopAppNotification();
};

#include "platform/win/wintoastlib.h"

class WinToastNotification : public WinToastLib::IWinToastHandler
{
private:
    static QMutex mMutex;
    QPointer<DesktopAppNotificationBase> notification;

public:
    WinToastNotification(QPointer<DesktopAppNotificationBase> megaNotification);
    virtual ~WinToastNotification();

    void toastActivated();
    void toastActivated(int actionIndex);
    void toastDismissed(WinToastDismissalReason state);
    void toastFailed();
};

#endif // NOTIFICATOR_H

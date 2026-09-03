// Copyright (c) 2011-2013 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef NOTIFICATOR_H
#define NOTIFICATOR_H

#include "NotificatorBase.h"

#ifdef USE_DBUS
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QHash>
#endif

class DesktopAppNotification : public DesktopAppNotificationBase
{
    Q_OBJECT

public:
    DesktopAppNotification();
    ~DesktopAppNotification() = default;

    QIcon getImage() const;
    void setImagePath(const QString &value) override;

#ifdef USE_DBUS
public slots:
    void dBusNotificationSentCallback(QDBusMessage dbusMssage);
    void dbusNotificationSentErrorCallback(QDBusError error);
    void dBusNotificationCallback(QDBusMessage dbusMssage);
#endif

protected:
    QIcon image;
};


/** Cross-platform desktop notification client. */
class Notificator: public NotificatorBase
{
    Q_OBJECT

public:
    /** Create a new notificator.
       @note Ownership of trayIcon is not transferred to this object.
    */
    Notificator(const QString &programName, QSystemTrayIcon *trayIcon, QObject *parent);
    ~Notificator();

    void notify(Class cls, const QString &title, const QString &text, int millisTimeout = 10000);
    void notify(DesktopAppNotification *notification);

#ifdef USE_DBUS
private slots:
    void onDBusNotificationSignal(QDBusMessage dbusMessage);

private:
    // Dedicated session-bus connection used for both sending Notify and receiving the
    // ActionInvoked/NotificationClosed signals. A named connection (unlike sessionBus()) is
    // created with delivery enabled, so incoming signals are dispatched immediately instead of
    // being queued behind sessionBus()'s suspended-delivery re-enable, which never fires on some
    // desktops (e.g. dbus-broker on Arch). Both must share the connection so the daemon's
    // reply/signals reach the same bus name that issued Notify.
    QDBusConnection mNotificationBus;
    QPointer<QDBusInterface> interface;
    bool dbussSupportsActions;
    QHash<const QObject*, quint32> mNotificationIds;
    QHash<quint32, QPointer<DesktopAppNotification>> mNotificationsById;

    void notifyDBus(Class cls, const QString &title, const QString &text, const QIcon &icon, int millisTimeout, const QStringList &actions = QStringList(), DesktopAppNotification *notification = nullptr);
    void onNotificationDestroyed(QObject* notification);
    bool subscribeToDBusSignals();
    void forgetNotification(const QObject* notification);
#endif
};

#endif // NOTIFICATOR_H

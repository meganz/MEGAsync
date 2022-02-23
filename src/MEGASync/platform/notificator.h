// Copyright (c) 2011-2013 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef NOTIFICATOR_H
#define NOTIFICATOR_H

#include <QIcon>
#include <QObject>
#include <QPointer>
#include <QMutex>
#include <QHash>

QT_BEGIN_NAMESPACE
class QSystemTrayIcon;

#ifdef USE_DBUS
#include <QDBusMessage>
class QDBusInterface;
#endif
QT_END_NAMESPACE

class MegaNotification : public QObject
{
    Q_OBJECT

public:
    enum class CloseReason {
        Unknown = 0,
        UserAction,
        AppHidden,
        TimedOut
    };

    enum class Action {
        content = -1,
        firstButton = 0,
        secondButton = 1,
        legacy = 2
    };

    MegaNotification();
    virtual ~MegaNotification();

    QString getTitle() const;
    void setTitle(const QString &value);
    QString getText() const;
    void setText(const QString &value);
    QString getSource() const;
    void setSource(const QString &value);
    int getExpirationTime() const;
    void setExpirationTime(int value);
    QString getImagePath() const;
    void setImagePath(const QString &value);
    int getStyle() const;
    void setStyle(int value);
    QStringList getActions() const;
    void setActions(const QStringList &value);
    QIcon getImage() const;
    void setImage(const QIcon &value);
    int getType() const;
    void setType(int value);
    int64_t getId() const;
    void setId(const int64_t &value);
    QString getData() const;
    void setData(const QString &value);

    void emitLegacyNotificationActivated();

protected:
    QString title;
    QString text;
    QString source;
    int expirationTime;
    QString imagePath;
    QIcon image;
    int style;
    QStringList actions;
    int type;
    int64_t id;
    QString data;

#ifdef USE_DBUS
    int dbusId;
#endif
signals:
    void activated(Action action);
    void closed(CloseReason reason);
    void failed();

#ifdef USE_DBUS
public slots:
    void dBusNotificationSentCallback(QDBusMessage dbusMssage);
    void dBusNotificationSentErrorCallback(QDBusError error);
    void dBusNotificationCallback(QDBusMessage dbusMssage);
#endif
};

#ifdef _WIN32
#include "platform/win/wintoastlib.h"

class WinToastNotification : public WinToastLib::IWinToastHandler
{
private:
    static QMutex mutex;
    QPointer<MegaNotification> notification;

public:
    WinToastNotification(QPointer<MegaNotification> megaNotification);
    virtual ~WinToastNotification();

    void toastActivated();
    void toastActivated(int actionIndex);
    void toastDismissed(WinToastDismissalReason state);
    void toastFailed();
};
#endif


/** Cross-platform desktop notification client. */
class Notificator: public QObject
{
    Q_OBJECT

public:
    /** Create a new notificator.
       @note Ownership of trayIcon is not transferred to this object.
    */
    Notificator(const QString &programName, QSystemTrayIcon *trayIcon, QObject *parent);
    ~Notificator();

    // Message class
    enum Class
    {
        Information,    /**< Informational message */
        Warning,        /**< Notify user of potential problem */
        Critical        /**< An error occurred */
    };

    static QHash<int64_t, MegaNotification *> notifications;

public slots:
    /** Show notification message.
       @param[in] cls    general message class
       @param[in] title  title shown with message
       @param[in] text   message content
       @param[in] icon   optional icon to show with message
       @param[in] millisTimeout notification timeout in milliseconds (defaults to 10 seconds)
       @note Platform implementations are free to ignore any of the provided fields except for \a text.
     */
    void notify(Class cls, const QString &title, const QString &text,
                const QIcon &icon = QIcon(), int millisTimeout = 10000);
    void notify(MegaNotification *notification);

private:

    enum Mode {
        None,                       /**< Ignore informational notifications, and show a modal pop-up dialog for Critical notifications. */
        Freedesktop,                /**< Use DBus org.freedesktop.Notifications */
        QSystemTray,                /**< Use QSystemTray::showMessage */
        UserNotificationCenter      /**< Use the 10.8+ User Notification Center (Mac only) */
    };

    QString programName;
    Mode mode;
    QSystemTrayIcon *trayIcon;
    QString defaultIconPath;
    MegaNotification* currentNotification;

#ifdef USE_DBUS
    QDBusInterface *interface;
    bool dbussSupportsActions;

    void notifyDBus(Class cls, const QString &title, const QString &text, const QIcon &icon, int millisTimeout, QStringList actions = QStringList(), MegaNotification *notification = NULL);
#endif
    void notifySystray(Class cls, const QString &title, const QString &text, const QIcon &icon, int millisTimeout, bool forceQt = false);
    void notifySystray(MegaNotification *notification);

protected slots:
    void onModernNotificationFailed();
    void onMessageClicked();
};

#endif // NOTIFICATOR_H

// Copyright (c) 2011-2013 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef NOTIFICATOR_H
#define NOTIFICATOR_H

#include <QIcon>
#include <QObject>
#include <QPointer>
#include <QMutex>

QT_BEGIN_NAMESPACE
class QSystemTrayIcon;

#ifdef USE_DBUS
class QDBusInterface;
#endif
QT_END_NAMESPACE

class MegaNotification : public QObject
{
    Q_OBJECT

public:
    enum CloseReason {
        Unknown,
        UserAction,
        AppHidden,
        TimedOut
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

signals:
    void activated(int action);
    void closed(int reason);
    void failed();
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

private:

    enum Mode {
        None,                       /**< Ignore informational notifications, and show a modal pop-up dialog for Critical notifications. */
        Freedesktop,                /**< Use DBus org.freedesktop.Notifications */
        QSystemTray,                /**< Use QSystemTray::showMessage */
        Growl12,                    /**< Use the Growl 1.2 notification system (Mac only) */
        Growl13,                    /**< Use the Growl 1.3 notification system (Mac only) */
        UserNotificationCenter      /**< Use the 10.8+ User Notification Center (Mac only) */
    };

    QString programName;
    Mode mode;
    QSystemTrayIcon *trayIcon;
    QString defaultIconPath;

#ifdef USE_DBUS
    QDBusInterface *interface;

    void notifyDBus(Class cls, const QString &title, const QString &text, const QIcon &icon, int millisTimeout);
#endif
    void notifySystray(Class cls, const QString &title, const QString &text, const QIcon &icon, int millisTimeout);
    void notifySystray(MegaNotification *notification);

#ifdef Q_OS_MAC
    void notifyGrowl(Class cls, const QString &title, const QString &text, const QIcon &icon);
    void notifyMacUserNotificationCenter(Class cls, const QString &title, const QString &text, const QIcon &icon);
#endif
};

#endif // NOTIFICATOR_H

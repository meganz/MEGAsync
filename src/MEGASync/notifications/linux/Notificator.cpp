// Copyright (c) 2011-2013 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#include "Notificator.h"

#include "megaapi.h"

#include <QApplication>
#include <QByteArray>
#include <QDebug>
#include <QIcon>
#include <QImageWriter>
#include <QMetaType>
#include <QStyle>
#include <QSystemTrayIcon>
#include <QTemporaryFile>
#include <QVariant>

#include <cassert>

#ifdef USE_DBUS
#include <QtDBus/QtDBus>
#endif

// Include ApplicationServices.h after QtDbus to avoid redefinition of check().
// This affects at least OSX 10.6. See /usr/include/AssertMacros.h for details.
// Note: This could also be worked around using:
// #define __ASSERT_MACROS_DEFINE_VERSIONS_WITHOUT_UNDERSCORES 0

using namespace mega;

// https://wiki.ubuntu.com/NotificationDevelopmentGuidelines recommends at least 128
const int FREEDESKTOP_NOTIFICATION_ICON_SIZE = 128;
const QString& DesktopAppNotificationBase::defaultImage = QString::fromUtf8("://images/app_128.png");

#ifdef USE_DBUS
namespace
{
const QString NOTIFICATION_BUS_NAME = QString::fromUtf8("megasync_notifications");
}
#endif

Notificator::Notificator(const QString& programName, QSystemTrayIcon* trayicon, QObject* parent):
    NotificatorBase(programName, trayicon, parent)
#ifdef USE_DBUS
    ,
    mNotificationBus(
        QDBusConnection::connectToBus(QDBusConnection::BusType::SessionBus, NOTIFICATION_BUS_NAME)),
    interface(0),
    dbussSupportsActions(false)
#endif
{
    if (trayicon && trayicon->supportsMessages())
    {
        mMode = QSystemTray;
    }

#ifdef USE_DBUS
    interface = new QDBusInterface(QString::fromUtf8("org.freedesktop.Notifications"),
                                   QString::fromUtf8("/org/freedesktop/Notifications"),
                                   QString::fromUtf8("org.freedesktop.Notifications"),
                                   mNotificationBus,
                                   this);
    if (interface->isValid())
    {
        mMode = Freedesktop;
        QString xdgCurrentDesktop = qEnvironmentVariable("XDG_CURRENT_DESKTOP");
        //unity shows notification with actions as a popup
        if (xdgCurrentDesktop.isEmpty() || xdgCurrentDesktop != QString::fromUtf8("Unity"))
        {
            dbussSupportsActions = true;
        }
        else
        {
            QString logMessage =
                QString::fromUtf8("Disabling actions for DBUS notifications: not supported for "
                                  "your desktop environment. XDG_CURRENT_DESKTOP=");
            logMessage +=
                xdgCurrentDesktop.isEmpty() ? QString::fromUtf8("unset") : xdgCurrentDesktop;

            MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, logMessage.toUtf8().constData());
            dbussSupportsActions = false;
        }

        if (dbussSupportsActions)
        {
            // Without the signal subscriptions, action buttons would render but never deliver,
            // and the tracked notifications would never receive closed/activated -> leak.
            dbussSupportsActions = subscribeToDBusSignals();
        }
    }
#endif
}

Notificator::~Notificator() {}

#ifdef USE_DBUS
// Loosely based on http://www.qtcentre.org/archive/index.php/t-25879.html
class FreedesktopImage
{
public:
    FreedesktopImage() {}
    FreedesktopImage(const QImage &img);

    static int metaType();

    // Image to variant that can be marshalled over DBus
    static QVariant toVariant(const QImage &img);

private:
    int width, height, stride;
    bool hasAlpha;
    int channels;
    int bitsPerSample;
    QByteArray image;

    friend QDBusArgument &operator<<(QDBusArgument &a, const FreedesktopImage &i);
    friend const QDBusArgument &operator>>(const QDBusArgument &a, FreedesktopImage &i);
};

Q_DECLARE_METATYPE(FreedesktopImage);

// Image configuration settings
const int CHANNELS = 4;
const int BYTES_PER_PIXEL = 4;
const int BITS_PER_SAMPLE = 8;

FreedesktopImage::FreedesktopImage(const QImage &img):
    width(img.width()),
    height(img.height()),
    stride(img.width() * BYTES_PER_PIXEL),
    hasAlpha(true),
    channels(CHANNELS),
    bitsPerSample(BITS_PER_SAMPLE)
{
    // Convert 00xAARRGGBB to RGBA bytewise (endian-independent) format
    QImage tmp = img.convertToFormat(QImage::Format_ARGB32);
    const uint32_t *data = reinterpret_cast<const uint32_t*>(tmp.bits());

    unsigned int num_pixels = width * height;
    image.resize(num_pixels * BYTES_PER_PIXEL);

    for (unsigned int ptr = 0; ptr < num_pixels; ++ptr)
    {
        image[ptr*BYTES_PER_PIXEL+0] = static_cast<char>(data[ptr] >> 16); // R
        image[ptr*BYTES_PER_PIXEL+1] = static_cast<char>(data[ptr] >> 8);  // G
        image[ptr*BYTES_PER_PIXEL+2] = static_cast<char>(data[ptr]);       // B
        image[ptr*BYTES_PER_PIXEL+3] = static_cast<char>(data[ptr] >> 24); // A
    }
}

QDBusArgument &operator<<(QDBusArgument &a, const FreedesktopImage &i)
{
    a.beginStructure();
    a << i.width << i.height << i.stride << i.hasAlpha << i.bitsPerSample << i.channels << i.image;
    a.endStructure();
    return a;
}

const QDBusArgument &operator>>(const QDBusArgument &a, FreedesktopImage &i)
{
    a.beginStructure();
    a >> i.width >> i.height >> i.stride >> i.hasAlpha >> i.bitsPerSample >> i.channels >> i.image;
    a.endStructure();
    return a;
}

int FreedesktopImage::metaType()
{
    return qDBusRegisterMetaType<FreedesktopImage>();
}

QVariant FreedesktopImage::toVariant(const QImage &img)
{
    FreedesktopImage fimg(img);
    return QVariant(FreedesktopImage::metaType(), &fimg);
}

void Notificator::notifyDBus(Class cls, const QString &title, const QString &text, const QIcon &icon, int millisTimeout, const QStringList& actions, DesktopAppNotification *notification)
{
    Q_UNUSED(cls);
    // Arguments for DBus call:
    QList<QVariant> args;

    // Program Name:
    args.append(mProgramName);

    // Unique ID of this notification type:
    args.append(0U);

    // Application Icon, empty string
    args.append(QString());

    // Summary
    args.append(title);

    // Body
    args.append(text);

    // Actions
    args.append(actions);

    // Hints
    QVariantMap hints;

    // If no icon specified, set icon based on class
    QIcon tmpicon;
    if (icon.isNull())
    {
        QStyle::StandardPixmap sicon = QStyle::SP_MessageBoxQuestion;
        switch(cls)
        {
        case Information:
            sicon = QStyle::SP_MessageBoxInformation;
            break;

        case Warning:
            sicon = QStyle::SP_MessageBoxWarning;
            break;

        case Critical:
            sicon = QStyle::SP_MessageBoxCritical;
            break;

        default:
            break;
        }
        tmpicon = QApplication::style()->standardIcon(sicon);
    }
    else
    {
        tmpicon = icon;
    }
    hints[QString::fromUtf8("icon_data")] = FreedesktopImage::toVariant(tmpicon.pixmap(FREEDESKTOP_NOTIFICATION_ICON_SIZE).toImage());
    args.append(hints);

    // Timeout (in msec)
    args.append(millisTimeout);

    if(dbussSupportsActions && notification)
    {
        connect(notification, &QObject::destroyed, this, &Notificator::onNotificationDestroyed);
        connect(notification,
                &DesktopAppNotificationBase::closed,
                this,
                [this, notification](DesktopAppNotificationBase::CloseReason)
                {
                    forgetNotification(notification);
                });
        connect(notification,
                &DesktopAppNotificationBase::failed,
                this,
                [this, notification]()
                {
                    forgetNotification(notification);
                });

        auto* watcher = new QDBusPendingCallWatcher(
            interface->asyncCallWithArgumentList(QString::fromUtf8("Notify"), args),
            this);
        const QPointer<DesktopAppNotification> guardedNotification(notification);

        connect(watcher,
                &QDBusPendingCallWatcher::finished,
                this,
                [this, guardedNotification, watcher]()
                {
                    watcher->deleteLater();

                    if (!guardedNotification)
                    {
                        return;
                    }

                    const QDBusPendingReply<quint32> reply = *watcher;
                    if (reply.isError())
                    {
                        guardedNotification->dbusNotificationSentErrorCallback(reply.error());
                        return;
                    }

                    guardedNotification->dBusNotificationSentCallback(watcher->reply());
                    const auto notificationId = static_cast<quint32>(guardedNotification->getId());
                    if (notificationId != 0U)
                    {
                        mNotificationIds.insert(guardedNotification.data(), notificationId);
                        mNotificationsById.insert(notificationId, guardedNotification);
                    }
                });
    }
    else
    {
        // "Fire and forget"
        interface->callWithArgumentList(QDBus::NoBlock, QString::fromUtf8("Notify"), args);
    }
}

#endif

void Notificator::notify(Class cls, const QString &title, const QString &text, int millisTimeout)
{
    switch(mMode)
    {
#ifdef USE_DBUS
    case Freedesktop:
        static QIcon icon(DesktopAppNotification::defaultImage);
        notifyDBus(cls, title, text, icon, millisTimeout);
        break;
#endif
    default:
        NotificatorBase::notify(cls, title, text, millisTimeout);
    }
}

void Notificator::notify(DesktopAppNotification *notification)
{
#ifdef USE_DBUS
    if (mMode == Freedesktop && dbussSupportsActions)
    {
        QStringList actions;
        for (auto a : notification->getActions())
        {
            //Dbus likes pairs (Text and argument for the callback)
            actions.append(a);
            actions.append(a);
        }

        // The session bus signals (ActionInvoked/NotificationClosed) are subscribed once, in the
        // constructor, with this long-lived Notificator as the receiver. Deliveries are routed to
        // the matching notification by id in onDBusNotificationSignal(), so no queued D-Bus
        // delivery can ever target a short-lived notification object that is being destroyed.
        notifyDBus((Class)notification->getType(),
                   notification->getTitle(),
                   notification->getText(),
                   notification->getImage(),
                   notification->getExpirationTime(),
                   actions,
                   notification);
    }
    else
#endif
    {
        NotificatorBase::notify(notification);
    }
}

////MEGANOTIFICATION
///
DesktopAppNotification::DesktopAppNotification()
    : DesktopAppNotificationBase()
{
    image = QIcon(defaultImage);
}

#ifdef USE_DBUS
void DesktopAppNotification::dBusNotificationSentCallback(QDBusMessage dbusMssage)
{
    if (dbusMssage.arguments().size())
    {
        mId = dbusMssage.arguments().at(0).toUInt();
        MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, QString::fromUtf8("Notification sent to DBUS. Id = %1").arg(mId).toUtf8().constData());
    }
    else
    {
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("Notification sent to DBUS: missing id").toUtf8().constData());
        assert(false && "QDBusMessage missing id");
    }
}

void DesktopAppNotification::dbusNotificationSentErrorCallback(QDBusError error)
{
    if(error.isValid())
    {
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("Notification to DBUS failed %1:\n%2").arg(error.name()).arg(error.message()).toUtf8().constData());
    }

    emit failed();
    deleteLater();
}

void DesktopAppNotification::dBusNotificationCallback(QDBusMessage dbusMssage)
{
    if (dbusMssage.arguments().size() && mId != dbusMssage.arguments().at(0).toUInt())
    {
        MegaApi::log(MegaApi::LOG_LEVEL_DEBUG,
                     QString::fromUtf8("Received notification corresponding to another "
                                       "notification. current = %1. Received = %2")
                         .arg(mId)
                         .arg(dbusMssage.member())
                         .toUtf8()
                         .constData());
        return;
    }
    else if (!dbusMssage.arguments().size())
    {
        assert(false && "QDBusMessage missing id");
    }

    if (dbusMssage.member() == QString::fromUtf8("ActionInvoked"))
    {
        if(dbusMssage.arguments().size() > 1)
        {
            const QString actionText{dbusMssage.arguments().at(1).toString()};
            const auto actionIndex = getActions().indexOf(actionText);
            if(actionIndex == 0)
            {
                emit activated(Action::firstButton);
            }
            else if(actionIndex == 1)
            {
                emit activated(Action::secondButton);
            }
        }
        else
        {
            emit activated(Action::firstButton);
        }
    }
    else if (dbusMssage.member() == QString::fromUtf8("NotificationClosed"))
    {
        emit closed(CloseReason::Unknown);
    }
}

void Notificator::onNotificationDestroyed(QObject* notification)
{
    const auto notificationId = mNotificationIds.take(notification);
    mNotificationsById.remove(notificationId);
    if (notificationId != 0U && mMode == Freedesktop && dbussSupportsActions && interface)
    {
        interface->call(QDBus::NoBlock, QString::fromUtf8("CloseNotification"), notificationId);
    }
}

bool Notificator::subscribeToDBusSignals()
{
    auto sessionbus = mNotificationBus;
    const QString service = QString::fromUtf8("org.freedesktop.Notifications");
    const QString path = QString::fromUtf8("/org/freedesktop/Notifications");
    const char* slot = SLOT(onDBusNotificationSignal(QDBusMessage));

    // Match any sender: on GNOME the ActionInvoked/NotificationClosed signals are emitted by
    // helper connections that are NOT the owner of the org.freedesktop.Notifications well-known
    // name, so pinning the sender makes Qt reject every delivery. Only the notification daemon
    // emits these signals on this path/interface, and deliveries are filtered by id anyway.
    const bool actionInvoked = sessionbus.connect(QString(),
                                                  path,
                                                  service,
                                                  QString::fromUtf8("ActionInvoked"),
                                                  this,
                                                  slot);
    const bool notificationClosed = sessionbus.connect(QString(),
                                                       path,
                                                       service,
                                                       QString::fromUtf8("NotificationClosed"),
                                                       this,
                                                       slot);
    if (!actionInvoked || !notificationClosed)
    {
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR,
                     QString::fromUtf8("Couldn't subscribe to DBus notification signals "
                                       "(ActionInvoked=%1, NotificationClosed=%2): %3")
                         .arg(actionInvoked)
                         .arg(notificationClosed)
                         .arg(mNotificationBus.lastError().message())
                         .toUtf8()
                         .constData());
    }

    return actionInvoked && notificationClosed;
}

void Notificator::onDBusNotificationSignal(QDBusMessage dbusMessage)
{
    if (dbusMessage.arguments().isEmpty())
    {
        return;
    }

    const auto notificationId = dbusMessage.arguments().at(0).toUInt();
    if (auto* notification = mNotificationsById.value(notificationId).data())
    {
        notification->dBusNotificationCallback(dbusMessage);
    }
}

void Notificator::forgetNotification(const QObject* notification)
{
    const auto notificationId = mNotificationIds.take(notification);
    mNotificationsById.remove(notificationId);
}
#endif

void DesktopAppNotification::setImagePath(const QString &value)
{
    DesktopAppNotificationBase::setImagePath(value);
    image = QIcon(value);
}

QIcon DesktopAppNotification::getImage() const
{
    return image;
}

// Copyright (c) 2011-2013 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "Notificator.h"

#include <QApplication>
#include <QByteArray>
#include <QIcon>
#include <QImageWriter>
#include "gui/QMegaMessageBox.h"
#include "Utilities.h"
#include <QMetaType>
#include <QStyle>
#include <QSystemTrayIcon>
#include <QTemporaryFile>
#include <QVariant>
#include <memory>
#include <QDebug>

#include "MegaApplication.h"

#include <stdint.h>
#include <assert.h>

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
const QString& MegaNotification::defaultImage = QString::fromUtf8("://images/app_128.png");


Notificator::Notificator(const QString &programName, QSystemTrayIcon *trayicon, QObject *parent) :
    NotificatorBase(programName, trayicon, parent)
#ifdef USE_DBUS
    ,interface(0)
    ,dbussSupportsActions(false)
#endif
{
    if (trayicon && trayicon->supportsMessages())
    {
        mMode = QSystemTray;
    }

#ifdef USE_DBUS
    interface = new QDBusInterface(QString::fromUtf8("org.freedesktop.Notifications"),
        QString::fromUtf8("/org/freedesktop/Notifications"), QString::fromUtf8("org.freedesktop.Notifications"));
    if (interface->isValid())
    {
        mMode = Freedesktop;
        if (!getenv("XDG_CURRENT_DESKTOP") || strcmp(getenv("XDG_CURRENT_DESKTOP"), "Unity") ) //unity shows notification with actions as a popup
        {
            dbussSupportsActions = true;
        }
        else
        {
            MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, QString::fromUtf8("Disabling actions for DBUS notifications: not supported for your desktop environment. XDG_CURRENT_DESKTOP=%1")
                         .arg(QString::fromUtf8(getenv("XDG_CURRENT_DESKTOP")?getenv("XDG_CURRENT_DESKTOP"):"unset")).toUtf8().constData());
            dbussSupportsActions = false;
        }

    }
#endif
}

Notificator::~Notificator()
{
#ifdef USE_DBUS
    delete interface;
#endif
}

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

void Notificator::notifyDBus(Class cls, const QString &title, const QString &text, const QIcon &icon, int millisTimeout, const QStringList& actions, MegaNotification *notification)
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

    if(dbussSupportsActions)
    {
        // fire with callback to gather ID
        interface->callWithCallback(QString::fromUtf8("Notify"), args, notification,
                                    SLOT(dBusNotificationSentCallback(QDBusMessage)), SLOT(dbusNotificationSentErrorCallback()));
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
        static QIcon icon(MegaNotification::defaultImage);
        notifyDBus(cls, title, text, icon, millisTimeout);
        break;
#endif
    default:
        NotificatorBase::notify(cls, title, text, millisTimeout);
    }
}

void Notificator::notify(MegaNotification *notification)
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

        QDBusConnection::sessionBus().connect(QString::fromUtf8("org.freedesktop.Notifications"),QString::fromUtf8("/org/freedesktop/Notifications"),
                                              QString::fromUtf8("org.freedesktop.Notifications"),QString::fromUtf8(""), notification, SLOT(dBusNotificationCallback(QDBusMessage)));
        notifyDBus((Class)notification->getType(), notification->getTitle(), notification->getText(),
                   notification->getImage(), notification->getExpirationTime(), actions, notification);
    }
    else
#endif
    {
        NotificatorBase::notify(notification);
    }
}

////MEGANOTIFICATION
///
MegaNotification::MegaNotification()
    : MegaNotificationBase()
#ifdef USE_DBUS
    , dbusId(-1)
#endif
{
    image = QIcon(defaultImage);
}

#ifdef USE_DBUS
void MegaNotification::dBusNotificationSentCallback(QDBusMessage dbusMssage)
{
    if (dbusMssage.arguments().size())
    {
        dbusId = dbusMssage.arguments().at(0).toInt();
        MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, QString::fromUtf8("Notification sent to DBUS. Id = %1").arg(dbusId).toUtf8().constData());
    }
    else
    {
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("Notification sent to DBUS: missing id").arg(dbusId).toUtf8().constData());
        assert(false && "QDBusMessage missing id");
    }
}

void MegaNotification::dbusNotificationSentErrorCallback()
{
    MegaApi::log(MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("Notification to DBUS failed").toUtf8().constData());
    deleteLater();
}

void MegaNotification::dBusNotificationCallback(QDBusMessage dbusMssage)
{
    if (dbusMssage.arguments().size() && dbusId != dbusMssage.arguments().at(0).toInt() )
    {
        MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, QString::fromUtf8("Received notification corresponding to another notification. current = %1. Received = %1").arg(dbusId).arg(dbusMssage.member()).toUtf8().constData());
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
#endif

void MegaNotification::setImagePath(const QString &value)
{
    MegaNotificationBase::setImagePath(value);
    image = QIcon(value);
}

QIcon MegaNotification::getImage() const
{
    return image;
}

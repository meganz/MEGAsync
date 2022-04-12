// Copyright (c) 2011-2013 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "notificator.h"

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

#include "MegaApplication.h"

#ifdef USE_DBUS
#include <stdint.h>
#include <QtDBus/QtDBus>
#include <assert.h>
#endif
// Include ApplicationServices.h after QtDbus to avoid redefinition of check().
// This affects at least OSX 10.6. See /usr/include/AssertMacros.h for details.
// Note: This could also be worked around using:
// #define __ASSERT_MACROS_DEFINE_VERSIONS_WITHOUT_UNDERSCORES 0
#ifdef Q_OS_MAC
#include <ApplicationServices/ApplicationServices.h>
#include "macx/macnotificationhandler.h"
#endif

#ifdef _WIN32
#include "platform/win/wintoastlib.h"
using namespace WinToastLib;
#endif

using namespace mega;

// https://wiki.ubuntu.com/NotificationDevelopmentGuidelines recommends at least 128
const int FREEDESKTOP_NOTIFICATION_ICON_SIZE = 128;

QHash<int64_t, MegaNotification *> Notificator::notifications;

Notificator::Notificator(const QString &programName, QSystemTrayIcon *trayicon, QObject *parent) :
    QObject(parent),
    programName(programName),
    mode(None),
    trayIcon(trayicon)
#ifdef USE_DBUS
    ,interface(0)
#endif
{
#ifndef Q_OS_MAC
    if (trayicon && trayicon->supportsMessages())
    {
        mode = QSystemTray;
    }
#endif

#ifdef USE_DBUS
    interface = new QDBusInterface(QString::fromUtf8("org.freedesktop.Notifications"),
        QString::fromUtf8("/org/freedesktop/Notifications"), QString::fromUtf8("org.freedesktop.Notifications"));
    if (interface->isValid())
    {
        mode = Freedesktop;
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
#ifdef Q_OS_MAC
    mode = UserNotificationCenter;
#endif

    QFile icon(QString::fromUtf8("://images/app_ico.ico"));
    defaultIconPath = MegaApplication::applicationDataPath() + QString::fromUtf8("\\MEGAsync.ico");
    if (!QFile(defaultIconPath).exists())
    {
        icon.copy(defaultIconPath);
    }
    currentNotification = NULL;

#ifdef _WIN32
    WinToast::instance()->setAppName(L"MEGAsync");
    WinToast::instance()->setAppUserModelId(L"MegaLimited.MEGAsync");
    WinToast::instance()->initialize();
    connect(trayIcon, SIGNAL(messageClicked()), this, SLOT(onMessageClicked()));
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

void Notificator::notifyDBus(Class cls, const QString &title, const QString &text, const QIcon &icon, int millisTimeout, QStringList actions, MegaNotification *notification)
{
    Q_UNUSED(cls);
    // Arguments for DBus call:
    QList<QVariant> args;

    // Program Name:
    args.append(programName);

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

void Notificator::notifySystray(Class cls, const QString &title, const QString &text, const QIcon &icon, int millisTimeout, bool forceQt)
{
#ifdef _WIN32
    if (!forceQt && WinToast::instance()->isCompatible())
    {
        MegaNotification *n = new MegaNotification();
        if (title == tr("MEGAsync"))
        {
            n->setTitle(QString::fromUtf8("MEGA"));
        }
        else
        {
            n->setTitle(title);
        }
        n->setText(text);
        n->setExpirationTime(millisTimeout);
        n->setImagePath(defaultIconPath);
        n->setType(cls);
        n->setStyle(WinToastTemplate::ImageAndText01);
        notifySystray(n);
        return;
    }
#endif

    if (!forceQt)
    {
        if (currentNotification)
        {
            currentNotification->deleteLater();
        }
        currentNotification = NULL;
    }

    Q_UNUSED(icon);
    QSystemTrayIcon::MessageIcon sicon = QSystemTrayIcon::NoIcon;
    switch(cls) // Set icon based on class
    {
    case Information:
        sicon = QSystemTrayIcon::Information;
        break;

    case Warning:
        sicon = QSystemTrayIcon::Warning;
        break;

    case Critical:
        sicon = QSystemTrayIcon::Critical;
        break;

    default:
        break;
    }
    trayIcon->showMessage(title, text, sicon, millisTimeout);
}

void Notificator::notifySystray(MegaNotification *notification)
{
    if (!notification)
    {
        return;
    }

#ifdef _WIN32
    if (!WinToast::instance()->isCompatible())
    {
        if (currentNotification)
        {
            currentNotification->deleteLater();
        }
        currentNotification = notification;
        notifySystray((Notificator::Class)notification->getType(), notification->getTitle(),
                      notification->getText(), notification->getImage(), notification->getExpirationTime(),
                      true);
        return;
    }

    connect(notification, SIGNAL(failed()), this, SLOT(onModernNotificationFailed()), Qt::QueuedConnection);

    WinToastTemplate templ(WinToastTemplate::ImageAndText02);
    templ.setTextField((LPCWSTR)notification->getTitle().utf16(), WinToastTemplate::FirstLine);
    templ.setTextField((LPCWSTR)notification->getText().utf16(), WinToastTemplate::SecondLine);
    templ.setAttributionText((LPCWSTR)notification->getSource().utf16());
    templ.setExpiration(notification->getExpirationTime());
    if (!notification->getImagePath().isEmpty())
    {
        templ.setImagePath((LPCWSTR)notification->getImagePath().utf16());
    }
    else
    {
        templ.setImagePath((LPCWSTR)defaultIconPath.utf16());
    }

    QStringList userActions = notification->getActions();
    for (int i = 0; i < userActions.size(); i++)
    {
        templ.addAction((LPCWSTR)userActions.at(i).utf16());
    }

    notification->setId(WinToast::instance()->showToast(templ, std::shared_ptr<IWinToastHandler>(new WinToastNotification(notification))));
    return;
#else

    notifySystray((Notificator::Class)notification->getType(), notification->getText(),
                  notification->getSource(), notification->getImage(), notification->getExpirationTime());
#endif
}

void Notificator::onModernNotificationFailed()
{
    MegaNotification *notification = (MegaNotification *)QObject::sender();
    if (currentNotification)
    {
        currentNotification->deleteLater();
    }
    currentNotification = notification;
    notifySystray((Notificator::Class)notification->getType(), notification->getTitle(),
                  notification->getText(), notification->getImage(),
                  notification->getExpirationTime(), true);
}

void Notificator::onMessageClicked()
{
    if (currentNotification)
    {
        currentNotification->emitLegacyNotificationActivated();
        currentNotification = NULL;
    }
}

void Notificator::notify(Class cls, const QString &title, const QString &text, const QIcon &icon, int millisTimeout)
{
    switch(mode)
    {
#ifdef USE_DBUS
    case Freedesktop:
        notifyDBus(cls, title, text, icon, millisTimeout);
        break;
#endif
    case QSystemTray:
        notifySystray(cls, title, text, icon, millisTimeout);
        break;
#ifdef Q_OS_MAC
    case UserNotificationCenter:
    {
        MegaNotification *n = new MegaNotification();
        if (title == tr("MEGAsync"))
        {
            n->setTitle(QString::fromUtf8("MEGA"));
        }
        else
        {
            n->setTitle(title);
        }
        n->setText(text);
        n->setExpirationTime(millisTimeout);
        n->setImagePath(defaultIconPath);
        n->setType(cls);
        MacNotificationHandler::instance()->showNotification(n);
        break;
    }
#endif
    default:
        switch(cls) // Set icon based on class
        {
            case Information: QMegaMessageBox::information(nullptr, title, text, QMessageBox::Ok); break;
            case Warning: QMegaMessageBox::warning(nullptr, title, text, QMessageBox::Ok); break;
            case Critical: QMegaMessageBox::critical(nullptr, title, text, QMessageBox::Ok); break;
        }

        break;
    }
}

void Notificator::notify(MegaNotification *notification)
{
    if (mode == QSystemTray)
    {
        notifySystray(notification);
        return;
    }

#ifdef Q_OS_MAC
    else if (mode == UserNotificationCenter)
    {
        MacNotificationHandler::instance()->showNotification(notification);
        return;
    }
#endif

#ifdef USE_DBUS
    if (mode == Freedesktop && dbussSupportsActions)
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
        notify((Class)notification->getType(), notification->getTitle(), notification->getText(),
               notification->getImage(), notification->getExpirationTime());
        delete notification;
    }
}

int MegaNotification::getStyle() const
{
    return style;
}

void MegaNotification::setStyle(int value)
{
    style = value;
}

QStringList MegaNotification::getActions() const
{
    return actions;
}

void MegaNotification::setActions(const QStringList &value)
{
    actions = value;
}

QIcon MegaNotification::getImage() const
{
    return image;
}

void MegaNotification::setImage(const QIcon &value)
{
    image = value;
}

int MegaNotification::getType() const
{
    return type;
}

void MegaNotification::setType(int value)
{
    type = value;
}

int64_t MegaNotification::getId() const
{
    return id;
}

void MegaNotification::setId(const int64_t &value)
{
    id = value;
}

QString MegaNotification::getData() const
{
    return data;
}

void MegaNotification::setData(const QString &value)
{
    data = value;
}

void MegaNotification::emitLegacyNotificationActivated()
{
    emit activated(Action::legacy);
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

MegaNotification::MegaNotification()
{
    qRegisterMetaType<CloseReason>("CloseReason");
    qRegisterMetaType<Action>("Action");
    title = QString::fromUtf8("MEGA");
    text = QString::fromUtf8("MEGAsync");
    expirationTime = 10000;
    style = -1;
    type = Notificator::Information;
    id = -1;
#if USE_DBUS
    dbusId = -1;
#endif

    connect(this, &MegaNotification::activated, this, &MegaNotification::deleteLater, Qt::QueuedConnection);
    connect(this, &MegaNotification::closed, this, &MegaNotification::deleteLater, Qt::QueuedConnection);
}

MegaNotification::~MegaNotification()
{
#ifdef _WIN32
    if (id != -1)
    {
        WinToast::instance()->hideToast(id);
    }
#endif
}

QString MegaNotification::getTitle() const
{
    return title;
}

void MegaNotification::setTitle(const QString &value)
{
    title = value;
}

QString MegaNotification::getText() const
{
    return text;
}

void MegaNotification::setText(const QString &value)
{
    text = value;
}

QString MegaNotification::getSource() const
{
    return source;
}

void MegaNotification::setSource(const QString &value)
{
    source = value;
}

int MegaNotification::getExpirationTime() const
{
    return expirationTime;
}

void MegaNotification::setExpirationTime(int value)
{
    expirationTime = value;
}

QString MegaNotification::getImagePath() const
{
    return imagePath;
}

void MegaNotification::setImagePath(const QString &value)
{
    imagePath = value;
}

#ifdef _WIN32

QMutex WinToastNotification::mutex;

WinToastNotification::WinToastNotification(QPointer<MegaNotification> megaNotification)
{
    this->notification = megaNotification;
}

WinToastNotification::~WinToastNotification()
{

}

void WinToastNotification::toastActivated()
{
    mutex.lock();
    if (notification)
    {
        emit notification->activated(MegaNotification::Action::content);
        notification = NULL;
    }
    mutex.unlock();
}

void WinToastNotification::toastActivated(int actionIndex)
{
    mutex.lock();
    if (notification)
    {
        emit notification->activated(static_cast<MegaNotification::Action>(actionIndex));
        notification = NULL;
    }
    mutex.unlock();
}

void WinToastNotification::toastDismissed(WinToastDismissalReason state)
{
    mutex.lock();
    if (notification)
    {
        auto reason = MegaNotification::CloseReason::Unknown;
        switch (state)
        {
        case WinToastDismissalReason::UserCanceled:
            reason = MegaNotification::CloseReason::UserAction;
            break;
        case WinToastDismissalReason::ApplicationHidden:
            reason = MegaNotification::CloseReason::AppHidden;
            break;
        case WinToastDismissalReason::TimedOut:
            reason = MegaNotification::CloseReason::TimedOut;
            break;
        }

        emit notification->closed(reason);
        notification = NULL;
    }
    mutex.unlock();
}

void WinToastNotification::toastFailed()
{
    mutex.lock();
    if (notification)
    {
        emit notification->failed();
        notification = NULL;
    }
    mutex.unlock();
}

#endif

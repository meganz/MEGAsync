#include <platform/PowerOptions.h>

#include <Preferences.h>
#include "megaapi.h"

#include <QString>
#include <QTimer>

#ifdef USE_DBUS
    #include <QDBusConnection>
    #include <QDBusConnectionInterface>
    #include <QDBusInterface>
    #include <QDBusReply>
    #include <QDBusUnixFileDescriptor>
#endif

#include <unistd.h>

class PowerOptionsImpl : public QObject
{
    const int MAX_SERVICES = 2;
    const uint INHIBIT_SUSPEND_GNOME = 4;

    const QString GNOME_SERVICE = QLatin1String("org.gnome.SessionManager");
    const QString GNOME_PATH = QLatin1String("/org/gnome/SessionManager");

    const QString FREEDESKTOP_POWERMANAGEMENT_SERVICE = QLatin1String("org.freedesktop.PowerManagement.Inhibit");
    const QString FREEDESKTOP_POWERMANAGEMENT_PATH = QLatin1String("/org/freedesktop/PowerManagement/Inhibit");

    const QString FREEDESKTOP_SCREENSAVER_SERVICE = QLatin1String("org.freedesktop.ScreenSaver");
    const QString FREEDESKTOP_SCREENSAVER_PATH = QLatin1String("/org/freedesktop/ScreenSaver");

    const QString FREEDESKTOP_SYSTEMD_SERVICE = QLatin1String("org.freedesktop.login1");
    const QString FREEDESKTOP_SYSTEMD_IFACE = FREEDESKTOP_SYSTEMD_SERVICE + QLatin1String(".Manager");
    const QString FREEDESKTOP_SYSTEMD_PATH = QLatin1String("/org/freedesktop/login1");

public:
    PowerOptionsImpl()
        : mKeepPCAwakeState(false)
    {}

    ~PowerOptionsImpl()
    {
        mKeepPCAwakeState = false;
        onKeepPCAwake();
    }

    bool changeKeepPCAwakeState(bool state)
    {
        auto result(true);
        auto newState(state && Preferences::instance()->awakeIfActiveEnabled());

        if(mKeepPCAwakeState != newState)
        {
            mKeepPCAwakeState = newState;
            result = onKeepPCAwake();
        }

        return result;
    }

public slots:
    bool onKeepPCAwake()
    {
        bool result(false);

#ifdef USE_DBUS
        //This check is not neccesary, but just in case...this slot is only called when the boolean is true
        QDBusConnection bus = QDBusConnection::sessionBus();

        if(bus.isConnected())
        {
            // First try session-level power management
            QDBusReply<QStringList> reply = bus.interface()->registeredServiceNames();
            auto services = reply.value();

            if(services.contains(GNOME_SERVICE))
            {
                result = runForGnome(bus);
            }
            else if(services.contains(FREEDESKTOP_POWERMANAGEMENT_SERVICE))
            {
                result = runForFreedesktopPowerManagement(bus);
            }
            else if(services.contains(FREEDESKTOP_SCREENSAVER_SERVICE))
            {
                result = runForFreedesktopScreenSaver(bus);
            }
            else
            {   // Then try system-level power management, on the "system" bus
                bus = QDBusConnection::systemBus();
                if(bus.isConnected())
                {
                    reply = bus.interface()->registeredServiceNames();
                    services = reply.value();

                    if(services.contains(FREEDESKTOP_SYSTEMD_SERVICE))
                    {
                        result = runForSystemD(bus);
                    }
                }
            }
        }
#endif

        return result;
    }

private:
#ifdef USE_DBUS
    bool runForGnome(const QDBusConnection& bus)
    {
        auto result(false);

        QDBusInterface sessionManagerInterface(
                    GNOME_SERVICE, GNOME_PATH,
                    GNOME_SERVICE, bus, this);
        if (sessionManagerInterface.isValid())
        {
            //By default 0
            uint windowId(0);

            if(mKeepPCAwakeState)
            {
                QDBusReply<uint> replyToInhibit = sessionManagerInterface.call(
                            QLatin1String("Inhibit"),
                            QLatin1String("megasync"), windowId, QLatin1String("Active transfers"), INHIBIT_SUSPEND_GNOME);
                mCookie = replyToInhibit.value();

                if(replyToInhibit.isValid())
                {
                    result = true;
                }

                log(sessionManagerInterface.interface(), QString(QLatin1String("Inhibiting")), replyToInhibit);
            }
            else
            {
                QDBusReply<void> replyToUnInhibit = sessionManagerInterface.call(
                            QLatin1String("Uninhibit"), mCookie);

                if(replyToUnInhibit.isValid())
                {
                    result = true;
                }

                log(sessionManagerInterface.interface(),QString(QLatin1String("Uninhibiting")), replyToUnInhibit);
            }
        }

        return result;
    }
#endif

#ifdef USE_DBUS
    bool runForFreedesktopScreenSaver(const QDBusConnection& bus)
    {
        auto result(false);

        QDBusInterface screenSaverInterface(
                    FREEDESKTOP_SCREENSAVER_SERVICE, FREEDESKTOP_SCREENSAVER_PATH,
                    FREEDESKTOP_SCREENSAVER_SERVICE, bus, this);
        if (screenSaverInterface.isValid())
        {
            result = runForFreeDesktop(screenSaverInterface);
        }

        return result;
    }
#endif

#ifdef USE_DBUS
    bool runForFreedesktopPowerManagement(const QDBusConnection& bus)
    {
        auto result(false);

        QDBusInterface powerManagementInterface(
                    FREEDESKTOP_POWERMANAGEMENT_SERVICE, FREEDESKTOP_POWERMANAGEMENT_PATH,
                    FREEDESKTOP_POWERMANAGEMENT_SERVICE, bus, this);
        if (powerManagementInterface.isValid())
        {
            result = runForFreeDesktop(powerManagementInterface);
        }

        return result;
    }
#endif

#ifdef USE_DBUS
    bool runForFreeDesktop(QDBusInterface& interface)
    {
        bool result(false);

        if(mKeepPCAwakeState)
        {
            QDBusReply<uint> replyToInhibit = interface.call(
                        QLatin1String("Inhibit"),
                        QLatin1String("megasync"), QLatin1String("Active transfers"));
            mCookie = replyToInhibit.value();

            if(replyToInhibit.isValid())
            {
                result = true;
            }

            log(interface.interface(), QString(QLatin1String("Inhibiting")), replyToInhibit);
        }
        else
        {
            QDBusReply<void> replyToUnInhibit = interface.call(
                        QLatin1String("UnInhibit"), mCookie);

            if(replyToUnInhibit.isValid())
            {
                result = true;
            }

            log(interface.interface(),QString(QLatin1String("Uninhibiting")), replyToUnInhibit);
        }

        return result;
    }
#endif

#ifdef USE_DBUS
    bool runForSystemD(const QDBusConnection& bus)
    {
        bool result(false);

        QDBusInterface systemDInterface(
                    FREEDESKTOP_SYSTEMD_SERVICE, FREEDESKTOP_SYSTEMD_PATH,
                    FREEDESKTOP_SYSTEMD_IFACE, bus, this);
        if (systemDInterface.isValid())
        {
            if(mKeepPCAwakeState)
            {
                QDBusReply<QDBusUnixFileDescriptor> replyToInhibit = systemDInterface.call(
                            QLatin1String("Inhibit"),
                            QLatin1String("sleep"), QLatin1String("MEGASync"), QLatin1String("Active transfers"), QLatin1String("block"));

                if(replyToInhibit.isValid())
                {
                    result = true;
                    mFileDescriptor = replyToInhibit.value().fileDescriptor();
                }

                log(systemDInterface.interface(), QString(QLatin1String("Inhibiting")), replyToInhibit);
            }
            else
            {
                if(close(mFileDescriptor) == 0)
                {
                    mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_DEBUG,QString(QLatin1String("Service %1. Sleep settings: Uninhibit sleep mode OK.")).arg(systemDInterface.interface()).toUtf8().constData());
                }
                else
                {
                    mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_DEBUG, QString(QLatin1String("Service %1. Sleep settings: Uninhibit sleep mode failed.")).arg(systemDInterface.interface()).toUtf8().constData());
                }
            }
        }

        return result;
    }
#endif

#ifdef USE_DBUS
    template <class ReturnType>
    void log(const QString& service, const QString operation
             ,const QDBusReply<ReturnType>& reply)
    {
        if(reply.isValid())
        {
            mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_DEBUG, QString(QLatin1String("Service %1. Sleep settings: %1 sleep mode OK.")).arg(service, operation).toUtf8().constData());
        }
        else
        {
            mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_DEBUG, QString(QLatin1String("Service %1. Sleep settings: %1 sleep mode failed: %2")).arg(service, operation, reply.error().message()).toUtf8().constData());
        }
    }
#endif

private:
    bool mKeepPCAwakeState;
    uint mCookie;
    int  mFileDescriptor;
};

// CLASS POWEROPTIONS

std::unique_ptr<PowerOptionsImpl> PowerOptions::mPowerOptionsImpl = mega::make_unique<PowerOptionsImpl>();

PowerOptions::PowerOptions()
{}

PowerOptions::~PowerOptions()
{}

bool PowerOptions::keepAwake(bool state)
{
    return mPowerOptionsImpl ? mPowerOptionsImpl->changeKeepPCAwakeState(state) : false;
}

void PowerOptions::appShutdown()
{
    // singletons are trouble.
    // global objects deletion order in different compilation units cannot be predicted.
    // delete this unpredictable singleton thing before it causes a shutdown crash
    // as it will try to log some messages on destruction.
    // And logging may (will) already have been destroyed (as it uses global objects too).
    mPowerOptionsImpl.reset();
}

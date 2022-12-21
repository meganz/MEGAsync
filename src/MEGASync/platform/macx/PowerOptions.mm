#include <platform/PowerOptions.h>

#include "mega/types.h"
#include "Preferences.h"
#include "megaapi.h"

#include <IOKit/pwr_mgt/IOPMLib.h>
#include <IOKit/IOMessage.h>

using namespace mega;

class PowerOptionsImpl : public QObject
{
public:
    PowerOptionsImpl()
        : mKeepPCAwayState(false)
    {}

    ~PowerOptionsImpl() = default;

    bool changeKeepPCAwakeState(bool state)
    {
        auto result(true);

        auto newState = state && Preferences::instance()->awakeIfActiveEnabled();


        if(mKeepPCAwayState != newState)
        {
            mKeepPCAwayState = newState;
            result = mKeepPCAwayState ? registerForSleepWakeNotifications() : unRegisterForSleepWakeNotifications();

            mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_DEBUG, QString(QLatin1String("Sleep setting has changed. New state: %1")).arg(mKeepPCAwayState).toStdString().c_str());
        }

        return result;
    }

private:
    bool registerForSleepWakeNotifications();
    bool unRegisterForSleepWakeNotifications();
    static void systemPowerEventCallback(void *ref_con, io_service_t service, natural_t message_type, void *message_argument);
    void onHandlePowerChange(natural_t messageType, void* messageArgument);

public slots:
    bool onKeepPCAwake();

private:
    bool mKeepPCAwayState;

    io_connect_t root_port;
    IONotificationPortRef notifyPortRef;
    io_object_t notifierObject;
    CFRunLoopRef runLoop;
};

bool PowerOptionsImpl::registerForSleepWakeNotifications()
{
    auto result(true);

    root_port = IORegisterForSystemPower(this, &notifyPortRef, systemPowerEventCallback, &notifierObject);
    if (root_port == 0)
    {
        result = false;
        mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_ERROR, "IORegisterForSystemPower failed\n");
    }

    CFRunLoopAddSource(CFRunLoopGetCurrent(),
                    IONotificationPortGetRunLoopSource(notifyPortRef), kCFRunLoopCommonModes);

    runLoop = CFRunLoopGetCurrent();

    return result;
}

bool PowerOptionsImpl::unRegisterForSleepWakeNotifications()
{
    CFRunLoopRemoveSource(runLoop,
                        IONotificationPortGetRunLoopSource(notifyPortRef),
                        kCFRunLoopCommonModes);

    IODeregisterForSystemPower(&notifierObject);
    IOServiceClose(root_port);
    IONotificationPortDestroy(notifyPortRef);

    return true;
}

void PowerOptionsImpl::systemPowerEventCallback(void *ref_con, io_service_t service, natural_t message_type, void *message_argument)
{
    static_cast<PowerOptionsImpl*>(ref_con)->onHandlePowerChange(message_type, message_argument);
}

void PowerOptionsImpl::onHandlePowerChange(natural_t messageType, void *messageArgument)
{
    switch (messageType)
    {
        case kIOMessageSystemWillSleep:
        /*delivered at the point the system is initiating a
        non-abortable sleep.*/
        IOAllowPowerChange(root_port, (long)messageArgument);
        mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_DEBUG, "kIOMessageSystemWillSleep - System is initiatin a non-abortable sleep.\n");
        break;
        case kIOMessageSystemWillNotSleep:
        /*delivered when some app client has vetoed an idle sleep
        request.*/
        mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_DEBUG, "kIOMessageSystemWillNotSleep - Idle sleep rejected. System will not sleep\n");
        break;
        case kIOMessageCanSystemSleep:
        /* Idle sleep is about to kick in. This message will not be sent for forced sleep.
        Applications have a chance to prevent sleep by calling IOCancelPowerChange.
        Most applications should not prevent idle sleep.

        Power Management waits up to 30 seconds for you to either allow or deny idle
        sleep. If you don't acknowledge this power change by calling either
        IOAllowPowerChange or IOCancelPowerChange, the system will wait 30
        seconds then go to sleep.
        */
        {
            if (!mKeepPCAwayState)
            {
                // allow idle sleep
                IOAllowPowerChange(root_port, (long)messageArgument);
                mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_DEBUG, "kIOMessageCanSystemSleep - System can sleep request - Accepted\n");
            }
            else
            {
                // disallow idle sleep
                IOCancelPowerChange(root_port, (long)messageArgument);
                mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_DEBUG, "kIOMessageCanSystemSleep - System can sleep request - Rejected\n");
            }
        }
        break;
        default:
            break;
    }
}


std::unique_ptr<PowerOptionsImpl> PowerOptions::mPowerOptionsImpl = mega::make_unique<PowerOptionsImpl>();

PowerOptions::PowerOptions(){}

PowerOptions::~PowerOptions(){}

bool PowerOptions::keepAwake(bool state)
{
    return mPowerOptionsImpl->changeKeepPCAwakeState(state);
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

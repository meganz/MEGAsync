#include <platform/PowerOptions.h>

#include "Preferences.h"
#include "megaapi.h"

#include <Windows.h>
#include <WinBase.h>
#include <powersetting.h>

class PowerOptionsImpl : public QObject
{
public:
    PowerOptionsImpl()
        : mKeepPCAwayState(false)
        , mPowerRequest(nullptr)
        , mGuiActivePowerScheme(nullptr)
        , mDefaultRequestDCTimeout(0)
        , mDefaultRequestACTimeout(0)
    {}

    ~PowerOptionsImpl()
    {
        if(mGuiActivePowerScheme)
        {
            LocalFree(mGuiActivePowerScheme);
            mGuiActivePowerScheme = nullptr;
        }
        mKeepPCAwayState = false;
        onKeepPCAwake();
    }

    bool changeKeepPCAwakeState(bool state)
    {
        auto result(true);
        auto newState = state && Preferences::instance()->awakeIfActiveEnabled();

        if(mKeepPCAwayState != newState)
        {
            mKeepPCAwayState = newState;
            result = onKeepPCAwake();
        }

        return result;
    }

public slots:
    bool onKeepPCAwake()
    {
        bool result(false);
        QString message;

        if (mKeepPCAwayState)
        {
            if(mPowerRequest == nullptr)
            {
                bool keepAwake(true);

                POWER_REQUEST_CONTEXT _PowerRequestContext;

                // Set up the diagnostic string
                _PowerRequestContext.Version = POWER_REQUEST_CONTEXT_VERSION;
                _PowerRequestContext.Flags = POWER_REQUEST_CONTEXT_SIMPLE_STRING;
                char text[] = "Active transfers";
                wchar_t wtext[20];
                mbstowcs(wtext, text, strlen(text)+1);//Plus null
                LPWSTR ptr = wtext;
                _PowerRequestContext.Reason.SimpleReasonString = ptr; // your reason for changing the power settings;

                // Create the request, get a handle
                mPowerRequest = PowerCreateRequest(&_PowerRequestContext);

                // Set the request
                keepAwake &= PowerSetRequest(mPowerRequest, _POWER_REQUEST_TYPE::PowerRequestSystemRequired) != 0;
                keepAwake &= PowerSetRequest(mPowerRequest, _POWER_REQUEST_TYPE::PowerRequestExecutionRequired) != 0;

                PowerGetActiveScheme(NULL, &mGuiActivePowerScheme);

                DWORD ACsize(0);
                DWORD DCsize(0);
                keepAwake &= PowerReadDCValue(NULL, mGuiActivePowerScheme, &GUID_IDLE_RESILIENCY_SUBGROUP, &GUID_EXECUTION_REQUIRED_REQUEST_TIMEOUT, NULL,
                                 NULL, &DCsize) == ERROR_SUCCESS;
                keepAwake &= PowerReadACValue(NULL, mGuiActivePowerScheme, &GUID_IDLE_RESILIENCY_SUBGROUP, &GUID_EXECUTION_REQUIRED_REQUEST_TIMEOUT, NULL,
                                 NULL, &ACsize) == ERROR_SUCCESS;
                keepAwake &= PowerReadDCValue(NULL, mGuiActivePowerScheme, &GUID_IDLE_RESILIENCY_SUBGROUP, &GUID_EXECUTION_REQUIRED_REQUEST_TIMEOUT, NULL,
                                 (PUCHAR)&mDefaultRequestDCTimeout, &DCsize) == ERROR_SUCCESS;
                keepAwake &= PowerReadACValue(NULL, mGuiActivePowerScheme, &GUID_IDLE_RESILIENCY_SUBGROUP, &GUID_EXECUTION_REQUIRED_REQUEST_TIMEOUT, NULL,
                                 (PUCHAR)&mDefaultRequestACTimeout, &ACsize) == ERROR_SUCCESS;

                PowerSetActiveScheme(NULL, mGuiActivePowerScheme);
                DWORD maxValue(UINT_MAX);
                keepAwake &=PowerWriteDCValueIndex(NULL, mGuiActivePowerScheme, &GUID_IDLE_RESILIENCY_SUBGROUP, &GUID_EXECUTION_REQUIRED_REQUEST_TIMEOUT, maxValue) == ERROR_SUCCESS;
                keepAwake &=PowerWriteACValueIndex(NULL, mGuiActivePowerScheme, &GUID_IDLE_RESILIENCY_SUBGROUP, &GUID_EXECUTION_REQUIRED_REQUEST_TIMEOUT, maxValue) == ERROR_SUCCESS;

                message = QString(QLatin1String("Sleep settings: System required request set. Result: %1"));

                result = keepAwake;
            }
        }
        else
        {
            if(mPowerRequest != nullptr)
            {
                bool keepAwake(true);

                PowerSetActiveScheme(NULL, mGuiActivePowerScheme);
                keepAwake &= PowerWriteDCValueIndex(NULL, mGuiActivePowerScheme, &GUID_IDLE_RESILIENCY_SUBGROUP, &GUID_EXECUTION_REQUIRED_REQUEST_TIMEOUT, mDefaultRequestDCTimeout) == ERROR_SUCCESS;
                keepAwake &= PowerWriteACValueIndex(NULL, mGuiActivePowerScheme, &GUID_IDLE_RESILIENCY_SUBGROUP, &GUID_EXECUTION_REQUIRED_REQUEST_TIMEOUT, mDefaultRequestACTimeout) == ERROR_SUCCESS;

                // Clear the request
                keepAwake &= PowerClearRequest(mPowerRequest, _POWER_REQUEST_TYPE::PowerRequestSystemRequired) != 0;
                keepAwake &= PowerClearRequest(mPowerRequest, _POWER_REQUEST_TYPE::PowerRequestExecutionRequired) != 0;

                CloseHandle(mPowerRequest);
                mPowerRequest = nullptr;

                message = QString(QLatin1String("Sleep settings: System required request cleared. Result: %1"));

                result = keepAwake;
            }
        }

        mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_DEBUG, message.arg(result).toStdString().c_str());

        return result;
    }

private:
    bool mKeepPCAwayState;
    HANDLE mPowerRequest;
    GUID* mGuiActivePowerScheme;
    DWORD mDefaultRequestDCTimeout;
    DWORD mDefaultRequestACTimeout;
};

std::unique_ptr<PowerOptionsImpl> PowerOptions::mPowerOptionsImpl = mega::make_unique<PowerOptionsImpl>();

PowerOptions::PowerOptions()
{

}

PowerOptions::~PowerOptions()
{

}

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


#include "WinThemeWatcher.h"

#include <QDebug>
#include <QSettings>

WinThemeWatcher::WinThemeWatcher(QObject* parent):
    QThread(parent),
    m_running(false),
    m_stopEvent(NULL)
{
    // Create manual-reset event to signal shutdown
    m_stopEvent = CreateEventW(NULL, TRUE, FALSE, NULL);
    startWatching();
}

WinThemeWatcher::~WinThemeWatcher()
{
    stopWatching(); // ensure thread exits
    if (m_stopEvent)
    {
        CloseHandle(m_stopEvent);
        m_stopEvent = NULL;
    }
}

void WinThemeWatcher::startWatching()
{
    if (!m_running)
    {
        ResetEvent(m_stopEvent); // clear stop signal
        m_running = true;
        start();
    }
}

void WinThemeWatcher::stopWatching()
{
    if (m_running)
    {
        m_running = false;
        SetEvent(m_stopEvent); // wake the thread immediately
        wait(); // block until thread exits
    }
}

void WinThemeWatcher::run()
{
    HKEY hKey;
    LONG result =
        RegOpenKeyExW(HKEY_CURRENT_USER,
                      L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
                      0,
                      KEY_NOTIFY,
                      &hKey);

    if (result != ERROR_SUCCESS)
    {
        qWarning() << "WinThemeWatcher: Failed to open registry key, error" << result;
        return;
    }

    HANDLE regEvent = CreateEventW(NULL, FALSE, FALSE, NULL);
    if (!regEvent)
    {
        qWarning() << "WinThemeWatcher: Failed to create registry event handle";
        RegCloseKey(hKey);
        return;
    }

    while (m_running)
    {
        // Request async registry notification
        result = RegNotifyChangeKeyValue(hKey,
                                         FALSE,
                                         REG_NOTIFY_CHANGE_LAST_SET,
                                         regEvent,
                                         TRUE // asynchronous mode
        );

        if (result != ERROR_SUCCESS)
        {
            qWarning() << "WinThemeWatcher: RegNotifyChangeKeyValue failed, error" << result;
            break;
        }

        // Wait for either registry change OR stop event
        HANDLE handles[2] = {regEvent, m_stopEvent};
        DWORD waitResult = WaitForMultipleObjects(2, handles, FALSE, INFINITE);

        if (waitResult == WAIT_OBJECT_0)
        {
            // registry changed
            if (m_running)
            {
                emit systemThemeChanged(getCurrentTheme());
            }
        }
        else if (waitResult == WAIT_OBJECT_0 + 1)
        {
            // stop event triggered
            break;
        }
        else
        {
            // unexpected error
            qWarning() << "WinThemeWatcher: WaitForMultipleObjects failed, error" << GetLastError();
            break;
        }
    }

    CloseHandle(regEvent);
    RegCloseKey(hKey);
}

Preferences::SystemColorScheme WinThemeWatcher::getCurrentTheme() const
{
    const QSettings settings(
        QLatin1String("HKEY_CURRENT_"
                      "USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize"),
        QSettings::NativeFormat);

    Preferences::SystemColorScheme scheme;

    // Get Applications appearence setting
    const auto appsUseLightTheme = settings.value(QLatin1String("AppsUseLightTheme"), QVariant());
    if (appsUseLightTheme.isValid())
    {
        const bool isDark = !appsUseLightTheme.toBool();
        scheme.appsScheme = Preferences::toTheme(isDark);
    }

    // Get System appearence setting
    const auto systemUsesLightTheme =
        settings.value(QLatin1String("SystemUsesLightTheme"), QVariant());
    if (systemUsesLightTheme.isValid())
    {
        const bool isDark = !systemUsesLightTheme.toBool();
        scheme.systemScheme = Preferences::toTheme(isDark);
    }
    return scheme;
}

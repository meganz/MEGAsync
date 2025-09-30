#include "MacThemeWatcher.h"

#include "MacThemeNotifier.h"

#include <QCoreApplication>
#include <QMetaObject>

static MacThemeWatcher* g_instance = nullptr;

MacThemeWatcher::MacThemeWatcher(QObject* parent):
    QObject(parent)
{
    g_instance = this;
    handle = MacTheme_Create(&MacThemeWatcher::StaticCallback);
}

MacThemeWatcher::~MacThemeWatcher()
{
    MacTheme_Destroy(handle);
    g_instance = nullptr;
}

Preferences::ThemeAppeareance MacThemeWatcher::getCurrentTheme() const
{
    bool d = false;
    MacTheme_GetCurrentAppearance(&d);

    return Preferences::toTheme(d);
}

void MacThemeWatcher::StaticCallback(bool isDark)
{
    const auto selection = Preferences::toTheme(isDark);

    // Switch to Qt's main thread to avoid issues between cocoa and Qt ui thread
    if (g_instance)
    {
        QMetaObject::invokeMethod(
            g_instance,
            [=]()
            {
                Q_EMIT g_instance->systemThemeChanged(selection);
            },
            Qt::QueuedConnection);
    }
}

#include "ThemeManager.h"

#include "Preferences/Preferences.h"
#include "Utilities.h"

const QMap<Preferences::ThemeType, QString> ThemeManager::mThemesMap = {
    {Preferences::ThemeType::LIGHT_THEME,  QLatin1String("Light")},
    {Preferences::ThemeType::DARK_THEME,  QLatin1String("Dark")}
};

ThemeManager::ThemeManager()
    : QObject(nullptr)
{
    setTheme(Preferences::instance()->getThemeType());
}

Preferences::ThemeType ThemeManager::getSelectedTheme() const
{
    return mCurrentTheme;
}

QString ThemeManager::getSelectedThemeString() const
{
    return mThemesMap.value(mCurrentTheme, QLatin1String("Light"));
}

ThemeManager* ThemeManager::instance()
{
    static ThemeManager manager;

    return &manager;
}

QStringList ThemeManager::themesAvailable() const
{
    return mThemesMap.values();
}

void ThemeManager::setTheme(Preferences::ThemeType theme)
{
    if (mCurrentTheme != theme)
    {
        mCurrentTheme = theme;

        Preferences::instance()->setThemeType(mCurrentTheme);

        emit themeChanged(theme);
        Utilities::propagateCustomEvent(ThemeChanged);
    }
}


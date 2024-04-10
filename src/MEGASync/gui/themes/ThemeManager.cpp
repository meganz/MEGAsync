#include "ThemeManager.h"

#include "Preferences/Preferences.h"

QMap<Preferences::Theme, QString> ThemeManager::mThemesMap = {
    {Preferences::Theme::LIGHT_THEME, tr("Light")},
    {Preferences::Theme::DARK_THEME, tr("Dark")}
};

ThemeManager::ThemeManager()
    : QObject(nullptr)
{
    setTheme(static_cast<Preferences::Theme>(Preferences::instance()->getTheme()));
}

Preferences::Theme ThemeManager::getSelectedTheme() const
{
    return mCurrentTheme;
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

void ThemeManager::setTheme(Preferences::Theme theme)
{
    if (mCurrentTheme != theme)
    {
        mCurrentTheme = theme;

        Preferences::instance()->setTheme(static_cast<std::underlying_type<Preferences::Theme>::type>(mCurrentTheme));

        emit themeChanged(theme);
    }
}




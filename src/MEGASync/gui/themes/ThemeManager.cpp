#include "ThemeManager.h"
#include "Preferences/Preferences.h"

const QMap<Preferences::ThemeType, QString> ThemeManager::mThemesMap = {
    {Preferences::ThemeType::LIGHT_THEME,  QObject::tr("Light")},
    {Preferences::ThemeType::DARK_THEME,  QObject::tr("Dark")}
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
    }
}


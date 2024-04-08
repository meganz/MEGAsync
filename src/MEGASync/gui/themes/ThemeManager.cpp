#include "ThemeManager.h"

#include <QMetaEnum>

QMap<Preferences::Theme, QString> ThemeManager::mThemesMap = {
    {Preferences::Theme::LIGHT_THEME, QLatin1String("Light")},
    {Preferences::Theme::DARK_THEME, QLatin1String("Dark")}
};

ThemeManager::ThemeManager() :
    mCurrentStyle(Preferences::Theme::LAST),
    mThemePaths{{Preferences::Theme::LIGHT_THEME, QLatin1String("light_tokens/")},
               {Preferences::Theme::DARK_THEME, QLatin1String("dark_tokens/")}}
{
    setTheme(static_cast<Preferences::Theme>(Preferences::instance()->getTheme()));
}

Preferences::Theme ThemeManager::getSelectedTheme()
{
    return static_cast<Preferences::Theme>(Preferences::instance()->getTheme());
}

ThemeManager* ThemeManager::instance()
{
    static ThemeManager manager;

    return &manager;
}

QStringList ThemeManager::themesAvailable() const
{
    static QStringList themes;
    if (themes.isEmpty())
    {
        QMetaEnum metaEnum = QMetaEnum::fromType<Preferences::Theme>();
        for(int index = 0; index < static_cast<std::underlying_type<Preferences::Theme>::type>(Preferences::Theme::LAST); ++index)
        {
            Preferences::Theme theme = static_cast<Preferences::Theme>(metaEnum.value(index));
            if (theme != Preferences::Theme::LAST)
            {
                themes.append(themeToString(theme));
            }
        }
    }

    return themes;
}

QString ThemeManager::themeToString(Preferences::Theme theme) const
{
    return mThemesMap.value(theme, QLatin1String("Light"));
}

void ThemeManager::setTheme(Preferences::Theme newTheme)
{
    if (mCurrentStyle != newTheme)
    {
        mCurrentStyle = newTheme;

        Preferences::instance()->setTheme(static_cast<int>(mCurrentStyle));

        emit themeChanged(mCurrentStyle);
    }
}




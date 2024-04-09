#include "ThemeManager.h"

#include "Preferences/Preferences.h"

ThemeManager::ThemeManager()
    : QObject(nullptr)
{
    setTheme(Preferences::instance()->getTheme());
}

QString ThemeManager::getSelectedTheme() const
{
    return Preferences::instance()->getTheme();
}

ThemeManager* ThemeManager::instance()
{
    static ThemeManager manager;

    return &manager;
}

QStringList ThemeManager::themesAvailable() const
{
    static QStringList themes{ QLatin1String("Light"), QLatin1String("Dark")};

    return themes;
}

void ThemeManager::setTheme(QString theme)
{
    if (mCurrentTheme != theme)
    {
        mCurrentTheme = theme;

        Preferences::instance()->setTheme(mCurrentTheme);

        emit themeChanged(mCurrentTheme);
    }
}




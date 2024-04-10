#include "QmlTheme.h"

#include "themes/ThemeManager.h"

#include <QTimer>

// note : this map is a conversion from theme enum to folder names that hold the color theme files.
QMap<Preferences::ThemeType, QString> QmlTheme::mThemesQmlMap = {
    {Preferences::ThemeType::LIGHT_THEME, QLatin1String("light")},
    {Preferences::ThemeType::DARK_THEME, QLatin1String("dark")}
};

QmlTheme::QmlTheme(QObject *parent)
    : QObject{parent}
{
    connect(ThemeManager::instance(), &ThemeManager::themeChanged, this, &QmlTheme::onThemeChanged);
}

void QmlTheme::onThemeChanged(Preferences::ThemeType theme)
{
    Q_UNUSED(theme);

    emit themeChanged(getTheme());
}

QString QmlTheme::getTheme() const
{
    QString returnValue;

    if (mThemesQmlMap.contains(ThemeManager::instance()->getSelectedTheme()))
    {
        returnValue = mThemesQmlMap[ThemeManager::instance()->getSelectedTheme()];
    }
    else
    {
        returnValue = mThemesQmlMap[Preferences::ThemeType::LIGHT_THEME];
    }

    return returnValue;
}

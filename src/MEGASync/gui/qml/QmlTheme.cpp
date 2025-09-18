#include "QmlTheme.h"

#include "ThemeManager.h"

#include <QTimer>

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

    returnValue = ThemeManager::instance()->getSelectedColorSchemaString();

    return returnValue.toLower();
}

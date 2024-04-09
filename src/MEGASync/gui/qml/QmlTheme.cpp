#include "QmlTheme.h"

#include "themes/ThemeManager.h"

#include <QTimer>

QmlTheme::QmlTheme(QObject *parent)
    : QObject{parent}
{
    connect(ThemeManager::instance(), &ThemeManager::themeChanged, this, &QmlTheme::themeChanged);
}

QString QmlTheme::getTheme() const
{
    return ThemeManager::instance()->getSelectedTheme().toLower();
}

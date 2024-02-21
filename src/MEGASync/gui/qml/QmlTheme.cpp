#include "QmlTheme.h"

#include <QTimer>

QmlTheme::QmlTheme(QObject *parent)
    : QObject{parent},
    mTheme(QString().fromUtf8("light"))
{
    startDemoChange();
}

QString QmlTheme::getTheme() const
{
    return mTheme;
}

void QmlTheme::setTheme(const QString& theme)
{
    if (!theme.isEmpty() && theme != mTheme)
    {
        mTheme = theme;
        emit themeChanged(mTheme);
    }
}

QStringList QmlTheme::getThemes() const
{
    static QStringList themes = {QString::fromUtf8("light"), QString::fromUtf8("dark")};

    return themes;
}

void QmlTheme::startDemoChange()
{
    // @jsubi.
    // TODO : get theme list.
    // TODO : get current theme.
    // TODO : set connection to capture theme changed event.

    // snipet to check theme change from light to dark.
    static QTimer timer;
    timer.start(10000);
    connect(&timer, &QTimer::timeout, this, [this](){
        emit themeChanged(QString::fromUtf8("dark"));
    });
}

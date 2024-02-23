#include "QmlTheme.h"

#include <QTimer>

#include <iostream>

QmlTheme::QmlTheme(QObject *parent)
    : QObject{parent},
    mTheme(QString::fromUtf8("light"))
{
    startDemoChange();
}

QString QmlTheme::getTheme() const
{
    return mTheme;
}

void QmlTheme::setTheme(const QString& theme)
{
    std::cout << "setTheme : " << theme.toStdString() << std::endl;

    if (!theme.isEmpty() && theme != mTheme)
    {
        std::cout << "Theme changed: " << theme.toStdString() << std::endl;
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
    timer.start(5000);
    connect(&timer, &QTimer::timeout, this, [this](){
        time_t t;
        srand((unsigned) time(&t));

        if (rand() % 2) {
            setTheme(QString::fromUtf8("dark"));
        }
        else {
            setTheme(QString::fromUtf8("light"));
        }
    });
}

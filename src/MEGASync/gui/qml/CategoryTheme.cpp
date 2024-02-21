#include "CategoryTheme.h"

#include <iostream>

CategoryTheme::CategoryTheme(const QmlTheme* const theme, QObject *parent)
    : QObject{parent},
    mTheme(theme)
{

}

QString CategoryTheme::getValue(QString category, QString tokenId)
{
    static int counter = 0;

    auto result = ++counter % 5;
    QString returnValue  = QString::fromUtf8("orange");

    switch(result)
    {
    case 0:
        returnValue = QString::fromUtf8("green");
        break;

    case 1:
        returnValue = QString::fromUtf8("blue"); // ok
        break;

    case 2:
        returnValue = QString::fromUtf8("red"); // ok
        break;
    }

    std::cout << " **************** return value : " << returnValue.toStdString() << std::endl;

    return returnValue;
}

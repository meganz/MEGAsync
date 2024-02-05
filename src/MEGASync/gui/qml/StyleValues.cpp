#include "StyleValues.h"
#include "qqmlcomponent.h"

#include <iostream>

StyleValues::StyleValues(QQmlEngine* engine, QObject *parent)
    : QObject{parent},
      mEngine{engine}
{
    QQmlComponent qmlComponent(mEngine);

    qmlComponent.loadUrl(QUrl(QString::fromUtf8("qrc:/common/dark/Styles.qml")));
    if (qmlComponent.isError()) {
        std::cout << "Error: " << qmlComponent.errorString().toStdString() << std::endl;
    }
    mDarkColors = qmlComponent.create();

    qmlComponent.loadUrl(QUrl(QString::fromUtf8("qrc:/common/light/Styles.qml")));
    if (qmlComponent.isError()) {
        std::cout << "Error: " << qmlComponent.errorString().toStdString() << std::endl;
    }
    mLightColors = qmlComponent.create();
}

StyleValues::~StyleValues()
{

}

QString StyleValues::getColor(QString tokenId)
{
    return mDarkColors->property(tokenId.toStdString().c_str()).toString();
}

void StyleValues::onThemeChanged()
{

}


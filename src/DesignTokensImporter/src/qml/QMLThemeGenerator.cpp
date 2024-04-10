#include "QMLThemeGenerator.h"

#include "IQMLThemeTarget.h"
#include "QMLThemeTargetFactory.h"

#include <QFileInfo>
#include <QRegularExpression>

using namespace DTI;

QmlThemeGenerator::QmlThemeGenerator(QObject *parent)
    : QObject{parent}
{}

void QmlThemeGenerator::start(const ThemedColourMap& themeData)
{
    /*
     * Deploy qml color targets.
    */
    std::unique_ptr<IQMLThemeTarget> qmlThemeTarget {QMLThemeTargetFactory::getQMLThemeTarget("qmlColorTheme")};
    if(qmlThemeTarget)
    {
        qmlThemeTarget->deploy(themeData);
    }

    /*
     * Deploy qml color manager target.
    */
    std::unique_ptr<IQMLThemeTarget> qmlThemeManagerTarget {QMLThemeTargetFactory::getQMLThemeTarget("qmlColorThemeManager")};
    if(qmlThemeManagerTarget)
    {
        qmlThemeManagerTarget->deploy(themeData);
    }

    emit finished();
}

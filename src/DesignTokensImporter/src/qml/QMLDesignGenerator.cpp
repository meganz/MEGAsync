#include "QMLDesignGenerator.h"

#include "QMLDesignTarget.h"
#include "QMLDesignTargetFactory.h"

#include <QFileInfo>
#include <QRegularExpression>

using namespace DTI;

QMLDesignGenerator::QMLDesignGenerator(QObject* parent)
    : QObject{parent}
{}

void QMLDesignGenerator::deploy(const DesignAssets& designAssets)
{
    /*
     * Deploy qml color targets.
    */
    std::unique_ptr<IQMLDesignTarget> qmlColorTarget {QMLDesignTargetFactory::getQMLDesignTarget("qmlColorTheme")};
    if(qmlColorTarget)
    {
        qmlColorTarget->deploy(designAssets);
    }

    /*
     * Deploy qml color manager target.
    */
    std::unique_ptr<IQMLDesignTarget> qmlThemeManagerTarget {QMLDesignTargetFactory::getQMLDesignTarget("qmlColorThemeManager")};
    if(qmlThemeManagerTarget)
    {
        qmlThemeManagerTarget->deploy(designAssets);
    }

    emit finished();
}

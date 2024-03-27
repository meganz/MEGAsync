#include "QMLStyleGenerator.h"

#include "IQMLStyleTarget.h"
#include "QMLStyleTargetFactory.h"

#include <QFileInfo>
#include <QRegularExpression>

using namespace DTI;

QmlStyleGenerator::QmlStyleGenerator(QObject *parent)
    : QObject{parent}
{}

void QmlStyleGenerator::start(const ThemedColourMap& styleData)
{
    for (auto themeIt = styleData.constBegin(); themeIt != styleData.constEnd(); ++themeIt)
    {
        const auto& styleData = themeIt.value();

        auto theme = themeIt.key().toLower();
        if (!theme.isEmpty() && !styleData.isEmpty())
        {
            foreach (const auto& styleTargetId, QMLStyleTargetFactory::getRegisteredStyleTargets())
            {
                // entry point in target execution
                std::unique_ptr<IQMLStyleTarget> qmlStyleTarget {QMLStyleTargetFactory::getQMLStyleTarget(styleTargetId)};
                if(qmlStyleTarget)
                {
                    qmlStyleTarget->deploy(theme, styleData);
                }
            }
        }
    }

    emit finished();
}

#include "QMLStyleGenerator.h"

#include "utilities.h"
#include "IQMLStyleTarget.h"
#include "QMLStyleTargetFactory.h"

#include <QFileInfo>
#include <QRegularExpression>

using namespace DTI;

QmlStyleGenerator::QmlStyleGenerator(QObject *parent)
    : QObject{parent}
{}

void QmlStyleGenerator::start(const FilePathColourMap& styleData)
{
    for (auto themeIt = styleData.constBegin(); themeIt != styleData.constEnd(); ++themeIt)
    {
        const auto& styleData = themeIt.value();

        /*
         * In future we will get the theme straight from the argument.
        */
        auto theme = Utilities::themeToString(Utilities::getTheme(themeIt.key()));
        theme = theme.toLower();

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

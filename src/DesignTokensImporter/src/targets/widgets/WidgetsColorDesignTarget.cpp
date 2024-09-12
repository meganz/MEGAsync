#include "WidgetsColorDesignTarget.h"

#include "DesignTargetFactory.h"
#include "PathProvider.h"
#include "Utilities.h"

#include <QDir>
#include <QFileInfo>
#include <QStringBuilder>
#include <QJsonObject>
#include <QJsonDocument>

using namespace DTI;

bool WidgetsColorDesignTarget::registered = ConcreteDesignTargetFactory<WidgetsColorDesignTarget>::Register("widgetsColor");

void WidgetsColorDesignTarget::deploy(const DesignAssets& designAssets) const
{
    QJsonObject jsonThemes;

    for (auto themeColorDataIt = designAssets.colorTokens.constKeyValueBegin(); themeColorDataIt != designAssets.colorTokens.constKeyValueEnd(); ++themeColorDataIt)
    {
        auto themeColorData = *themeColorDataIt;
        const auto& themeName = themeColorData.first;
        const auto& themeData = themeColorData.second;

        QJsonObject jsonTheme;

        for(auto colorDataIt = themeData.constKeyValueBegin(); colorDataIt != themeData.constKeyValueEnd(); ++colorDataIt)
        {
            auto colorData = *colorDataIt;
            const auto& colorName = colorData.first;
            const auto& colorValue = colorData.second;

            jsonTheme[colorName] = colorValue;
        }

        jsonThemes[themeName] = jsonTheme;
    }

    const QString directoryThemePath = QDir::currentPath() % PathProvider::RELATIVE_COLOR_DIR_PATH;

    if (Utilities::createDirectory(directoryThemePath))
    {
        const QString widgetsColorFilePath = directoryThemePath % "/" % PathProvider::COLOR_THEMED_TOKENS_FILE_NAME;

        if (Utilities::writeJSONToFile(QJsonDocument(jsonThemes), widgetsColorFilePath))
        {
             Utilities::logInfoMessage(QString::fromUtf8("The target widgetsColor has successfully generated the file : %0").arg(widgetsColorFilePath));
        }
    }
}




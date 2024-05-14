#include "WidgetsColorDesignTarget.h"

#include "WidgetsDesignTargetFactory.h"
#include "PathProvider.h"
#include "Utilities.h"

#include <QDir>
#include <QDebug>
#include <QFileInfo>
#include <QTextStream>
#include <QStringBuilder>
#include <QJsonObject>
#include <QJsonDocument>

using namespace DTI;

bool WidgetsColorDesignTarget::registered = WidgetsDesignFactory<WidgetsColorDesignTarget>::Register("widgetsColorTarget");

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
        Utilities::writeJSONToFile(QJsonDocument(jsonThemes), directoryThemePath % "/" + PathProvider::COLOR_THEMED_TOKENS_FILE_NAME);
    }
}




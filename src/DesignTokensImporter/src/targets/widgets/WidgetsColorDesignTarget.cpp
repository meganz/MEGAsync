#include "WidgetsColorDesignTarget.h"

#include "DesignTargetFactory.h"
#include "PathProvider.h"
#include "Utilities.h"

#include <QDir>
#include <QFileInfo>
#include <QJsonDocument>
#include <QStringBuilder>

using namespace DTI;

static auto NO_ALPHA_COLOR_SIZE{
    7u}; /* 7 is the size of #aa34bb, so solid alpha channel is missing */
static auto RGB_COLOR_VALUE_SIZE{6u};

bool WidgetsColorDesignTarget::registered =
    ConcreteDesignTargetFactory<WidgetsColorDesignTarget>::Register("widgetsColor");

void WidgetsColorDesignTarget::deploy(const DesignAssets& designAssets) const
{
    QJsonObject jsonThemes = createThemeJson(designAssets);

    if (!jsonThemes.isEmpty())
    {
        writeThemesToFile(jsonThemes);
    }
}

QJsonObject WidgetsColorDesignTarget::createThemeJson(const DesignAssets& designAssets) const
{
    QJsonObject jsonThemes;

    for (auto themeColorDataIt = designAssets.colorTokens.constKeyValueBegin();
         themeColorDataIt != designAssets.colorTokens.constKeyValueEnd();
         ++themeColorDataIt)
    {
        auto themeColorData = *themeColorDataIt;
        const auto& themeName = themeColorData.first;
        const auto& themeData = themeColorData.second;

        jsonThemes[themeName] = createColorJson(themeData);
    }

    return jsonThemes;
}

QJsonObject WidgetsColorDesignTarget::createColorJson(const ColorData& themeData) const
{
    QJsonObject jsonTheme;

    for (auto colorDataIt = themeData.constKeyValueBegin();
         colorDataIt != themeData.constKeyValueEnd();
         ++colorDataIt)
    {
        auto colorData = *colorDataIt;
        const auto& colorName = colorData.first;
        const auto& colorValue = colorData.second;

        jsonTheme[colorName] = adjustColorValue(colorValue);
    }

    return jsonTheme;
}

QString WidgetsColorDesignTarget::adjustColorValue(const QString& colorValue) const
{
    if (colorValue.size() == NO_ALPHA_COLOR_SIZE) // Solid alpha channel is missing
    {
        return QString("#ff%0").arg(colorValue.right(RGB_COLOR_VALUE_SIZE));
    }
    return colorValue;
}

void WidgetsColorDesignTarget::writeThemesToFile(const QJsonObject& jsonThemes) const
{
    const QString directoryThemePath = PathProvider::getColorDirPath();

    if (Utilities::createDirectory(directoryThemePath))
    {
        const QString widgetsColorFilePath =
            directoryThemePath % "/" % PathProvider::getColorThemedTokensFileName();

        if (Utilities::writeJSONToFile(QJsonDocument(jsonThemes), widgetsColorFilePath))
        {
            Utilities::logInfoMessage(
                QString::fromUtf8(
                    "The target widgetsColor has successfully generated the file : %0")
                    .arg(widgetsColorFilePath));
        }
    }
}

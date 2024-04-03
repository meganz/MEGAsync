#include "QMLColorThemeManagerTarget.h"

#include "QMLTargetUtils.h"
#include "QMLThemeTargetFactory.h"

#include <QDir>
#include <QDebug>
#include <QFileInfo>
#include <QTextStream>

#include <iostream>

using namespace DTI;

static const QString qmlColorThemeManagerTargetPath = "%1/gui/qml/common/ColorTheme.qml";
static const QString colorThemeManagerHeader = QString::fromUtf8("pragma Singleton\n\nimport QtQuick 2.15\n\nItem {\n    id: root\n\n    Loader{\n        id: loader\n        source: \"qrc:/common/themes/\"+themeManager.theme+\"/Colors.qml\"\n    }\n\n");
static const QString colorThemeManagerLine = QString::fromUtf8("\treadonly property color %1: loader.item.%2\n");
static const QString colorThemeManagerFooter = QString::fromUtf8("}\n");

bool QMLColorThemeManagerTarget::registered = ConcreteQMLThemeFactory<QMLColorThemeManagerTarget>::Register("qmlColorThemeManager");

void QMLColorThemeManagerTarget::deploy(const ThemedColourMap& themeData) const
{
    if (themeData.isEmpty())
    {
        qWarning() << __func__ << " Error : empty theme data.";
        return;
    }
    else if (!checkThemeData(themeData))
    {
        qWarning() << __func__ << " Error : themes have different tokens.";
        return;
    }


    QFile data(qmlColorThemeManagerTargetPath.arg(QDir::currentPath()));
    if (data.open(QFile::WriteOnly | QFile::Truncate))
    {
        QTextStream stream(&data);
        stream << colorThemeManagerHeader;

        // we just need the first theme, all themes should have the same tokens, verified in checkThemeData.
        auto themeIt = themeData.constBegin();
        const auto& themeData = themeIt.value();

        auto theme = themeIt.key().toLower();
        if (!theme.isEmpty() && !themeData.isEmpty())
        {
            const auto& colourMap = themeIt.value();

            for (auto it = colourMap.constBegin(); it != colourMap.constEnd(); ++it)
            {
                const auto& tokenId = it.key();
                auto normalizedTokenId = normalizeTokenId(tokenId);

                stream << colorThemeManagerLine.arg(normalizedTokenId, normalizedTokenId);
            }
        }

        stream << colorThemeManagerFooter;
    }
}

/*
 * All themes should have the same size and the same elements.
 */
bool QMLColorThemeManagerTarget::checkThemeData(const ThemedColourMap& themeData) const
{
    QStringList tokens;

    auto itFound = std::find_if(themeData.constBegin(), themeData.constEnd(), [&tokens](const QMap<QString, QString>& colorMap)
    {
        if (tokens.isEmpty())
        {
            tokens = colorMap.keys();
        }
        else if (tokens != colorMap.keys())
        {
            return true;
        }

        return false;
    });

    return itFound == themeData.constEnd();
}

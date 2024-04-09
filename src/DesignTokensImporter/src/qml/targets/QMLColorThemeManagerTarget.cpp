#include "QMLColorThemeManagerTarget.h"

#include "QMLTargetUtils.h"
#include "QMLThemeTargetFactory.h"

#include <QDir>
#include <QDebug>
#include <QFileInfo>
#include <QTextStream>

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
    bool returnValue = true;

    QStringList tokens;
    QString themeName;

    for (auto itTheme = themeData.constBegin(); itTheme != themeData.constEnd(); ++itTheme)
    {
        auto currentThemeName = itTheme.key();
        auto currentThemeData = itTheme.value();

        if (tokens.empty() && themeName.isEmpty())
        {
            themeName = currentThemeName;
            tokens = currentThemeData.keys();
        }
        else
        {
            QStringList currentTokens = currentThemeData.keys();

            // if we detect differences between token_id list, we log them.
            if (tokens != currentTokens)
            {
                returnValue = false;
                logColorTokensDifferences(themeName, tokens, currentThemeName, currentTokens);
            }
        }
    }

    return returnValue;
}

void QMLColorThemeManagerTarget::logColorTokensDifferences(const QString& themeName1, QStringList theme1ColorTokenList, const QString& themeName2, QStringList theme2ColorTokenList) const
{
    if (theme1ColorTokenList.size() != theme2ColorTokenList.size())
    {
        qWarning() << __func__ << " Error on themes : " << themeName1 << " & " << themeName2 << " have different sizes.";
    }

    auto itEndFoundDifferences = std::partition(theme1ColorTokenList.begin(), theme1ColorTokenList.end(), [&theme2ColorTokenList](const QString& tokenId)
    {
        return std::find(theme2ColorTokenList.cbegin(), theme2ColorTokenList.cend(), tokenId) == theme2ColorTokenList.cend();
    });

    if (itEndFoundDifferences != theme1ColorTokenList.end())
    {
        std::for_each(theme1ColorTokenList.begin(), itEndFoundDifferences, [&themeName1, &themeName2](const QString& tokenId)
        {
            qWarning() << "Error: couldn't find token " << tokenId << " from theme " << themeName1 << " in theme " << themeName2 << ".";
        });
    }

    itEndFoundDifferences = std::partition(theme2ColorTokenList.begin(), theme2ColorTokenList.end(), [&theme1ColorTokenList](const QString& tokenId)
    {
        return std::find(theme1ColorTokenList.cbegin(), theme1ColorTokenList.cend(), tokenId) == theme1ColorTokenList.cend();
    });

    if (itEndFoundDifferences != theme2ColorTokenList.end())
    {
        std::for_each(theme2ColorTokenList.begin(), itEndFoundDifferences, [&themeName1, &themeName2](const QString& tokenId)
        {
            qWarning() << "Error: couldn't find token " << tokenId << " from theme " << themeName2 << " in theme " << themeName1 << ".";
        });
    }
}

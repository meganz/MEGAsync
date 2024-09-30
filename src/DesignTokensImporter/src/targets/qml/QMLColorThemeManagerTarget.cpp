#include "QMLColorThemeManagerTarget.h"

#include "Utilities.h"
#include "QMLTargetUtils.h"
#include "DesignTargetFactory.h"

#include <QDir>
#include <QDebug>
#include <QFileInfo>
#include <QTextStream>

using namespace DTI;

static const QString qmlColorThemeManagerTargetPath = "%1/gui/qml/common/ColorTheme.qml";
static const QString colorThemeManagerHeader = QString::fromUtf8("pragma Singleton\n\nimport QtQuick 2.15\n\nItem {\n    id: root\n\n    Loader{\n        id: loader\n        source: \"qrc:/common/themes/\"+themeManager.theme+\"/Colors.qml\"\n    }\n\n");
static const QString colorThemeManagerLine = QString::fromUtf8("\treadonly property color %1: loader.item.%2\n");
static const QString colorThemeManagerFooter = QString::fromUtf8("}\n");

bool QMLColorThemeManagerTarget::registered = ConcreteDesignTargetFactory<QMLColorThemeManagerTarget>::Register("qmlColorThemeManager");

void QMLColorThemeManagerTarget::deploy(const DesignAssets& designAssets) const
{
    if (designAssets.colorTokens.isEmpty())
    {
        qWarning() << __func__ << " Error : empty theme data.";
        return;
    }
    else if (!checkThemeData(designAssets.colorTokens))
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
        auto themeIt = designAssets.colorTokens.constBegin();
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
        Utilities::logInfoMessage(QString::fromUtf8("The target qmlColorThemeManager has successfully generated the file : %0").arg(data.fileName()));
    }
    else
    {
        qWarning() << __func__ << " Error : couldn't open file " << data.fileName();
    }

    data.close();
}

//!
//! \brief QMLColorThemeManagerTarget::checkThemeData
//! \param themeData: Colour information of all themes
//! Returns true if all themes have the same size and identical elements, false otherwise
//!
bool QMLColorThemeManagerTarget::checkThemeData(const ThemedColorData& themeData) const
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

void QMLColorThemeManagerTarget::logColorTokensDifferences(const QString& themeName1, QStringList theme1ColorTokens, const QString& themeName2, QStringList theme2ColorTokens) const
{
    if (theme1ColorTokens.size() != theme2ColorTokens.size())
    {
        qWarning() << __func__ << " Error: Themes " << themeName1 << " and " << themeName2 << " have different numbers of color tokens.";
        // Note: Do not return early here, as the purpose of this function is to log the reason why the tokens are different
    }

    auto notFoundInTheme2It = std::partition(theme1ColorTokens.begin(), theme1ColorTokens.end(), [&theme2ColorTokens](const QString& tokenId)
    {
       return std::find(theme2ColorTokens.cbegin(), theme2ColorTokens.cend(), tokenId) == theme2ColorTokens.cend();
    });

    auto notFoundInTheme1It = std::partition(theme2ColorTokens.begin(), theme2ColorTokens.end(), [&theme1ColorTokens](const QString& tokenId)
    {
       return std::find(theme1ColorTokens.cbegin(), theme1ColorTokens.cend(), tokenId) == theme1ColorTokens.cend();
    });

    logMissingTokens(themeName1, themeName2, theme1ColorTokens.cbegin(), notFoundInTheme2It);
    logMissingTokens(themeName2, themeName1, theme2ColorTokens.cbegin(), notFoundInTheme1It);
}

void QMLColorThemeManagerTarget::logMissingTokens(const QString& sourceTheme, const QString& targetTheme, QStringList::const_iterator begin, QStringList::const_iterator end) const
{
    if (begin != end)
    {
        std::for_each(begin, end, [&sourceTheme, &targetTheme](const QString& tokenId)
        {
            qWarning() << "Error: Token " << tokenId << " from theme " << sourceTheme << " not found in theme " << targetTheme << ".";
        });
    }
}

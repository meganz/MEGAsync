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

            // if we detect differences between les token_id list, we will go into find the exact differences.
            if (tokens != currentTokens)
            {
                auto list1VsList2 = areDifferent(themeName, tokens, currentThemeName, currentTokens);
                auto list2VsList1 = areDifferent(currentThemeName, currentTokens, themeName, tokens);

                if (list1VsList2 || list2VsList1)
                {
                    return false;
                }
            }
        }
    }


    return true;
}


bool QMLColorThemeManagerTarget::areDifferent(QString& themeName1, const QStringList& list1, QString& themeName2, const QStringList& list2) const
{
    bool error = false;

    if (list1.size() != list2.size())
    {
        qWarning() << __func__ << " Error on themes : " << themeName1 << " & " << themeName2 << " have different sizes.";
        error = true;
    }

    for (auto list1Index = 0U; list1Index < list1.size(); ++list1Index)
    {
        auto value1 = list1[list1Index];
        bool found = false;

        for (auto list2Index = 0U; list2Index < list2.size() && !found; ++list2Index)
        {
            if (value1 == list2[list2Index])
            {
                found = true;
            }
        }

        if (!found)
        {
            // log error, i don't want to stop the main loop, i want to check all tokens.
            error = true;
            qWarning() << "Couldn't find tokendId " << value1 << " from theme " << themeName1  << " in theme " << themeName2;
        }
    }

    return error;
}

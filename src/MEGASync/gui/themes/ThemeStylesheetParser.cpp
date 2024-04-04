#include "ThemeStylesheetParser.h"

#include <QLatin1String>
#include <QRegularExpression>
#include <QSet>
#include <QStringBuilder>


const  QString ThemeStylesheetParser::TOKEN_HOVER = QLatin1String("hover");
const  QString ThemeStylesheetParser::TOKEN_PRESSED = QLatin1String("pressed");
const  QString ThemeStylesheetParser::TOKEN_NORMAL = QLatin1String("normal");


ThemeStylesheetParser::ThemeStylesheetParser()
{

}

void ThemeStylesheetParser::parseStyleSheet(const QString &stylesheet, const QString &uiFileName)
{
    static const QRegularExpression blockStartRegex(QLatin1String(R"(([^{]+){?)"));
    static const QRegularExpression objectNameRegex(QLatin1String(R"(/\*\s*(.*?)\s*\*/)"));
    static const QRegularExpression iconPathRegex(QLatin1String(R"(qproperty-icon:\s*url\(([^)]+)\))"));
    static const QRegularExpression alphanumericOnlyRegex(QLatin1String(R"([^a-zA-Z0-9])"));

    QRegularExpressionMatch match;

    QStringList lines = stylesheet.split(QLatin1String("\n"));
    QString currentBlock, currentSelector;
    QSet<QString> objectNames;
    bool isBlockOpen = false;

    for (const QString& line : lines)
    {
        QString trimmedLine = line.trimmed();

        if (!isBlockOpen)
        {
            match = blockStartRegex.match(trimmedLine);
            if (match.hasMatch())
            {
                currentSelector = match.captured(1).trimmed();
                currentBlock.clear();
                isBlockOpen = true;
            }
        }

        if (isBlockOpen)
        {
            currentBlock += trimmedLine % QLatin1String("\n");

            if (trimmedLine.contains(QLatin1String("{")))
            {
                if (!currentBlock.startsWith(currentSelector))
                {
                    currentBlock.prepend(currentSelector % QLatin1String(" {\n"));
                }
            }

            if (trimmedLine.contains(QLatin1String("}")))
            {
                match = objectNameRegex.match(currentBlock);
                if (match.hasMatch())
                {
                    QString objectName = match.captured(1);
                    QString key = uiFileName % QLatin1String("_") % objectName;

                    // Store the parsed stylesheet block in memory associated with its corresponding object name
                    mObjectNameToUiCSSFileStyleSheetsMap.insert(key, currentBlock); // Cache the parsed stylesheet in memory

                    objectNames.insert(objectName);

                    // Store Icon State Path in memory associated with its corresponding object name
                    QRegularExpressionMatch iconMatch = iconPathRegex.match(currentBlock);
                    if (iconMatch.hasMatch())
                    {
                        QString iconPath = iconMatch.captured(1);
                        QString selectorObjectName = currentSelector.split(QLatin1String(":")).first();
                        selectorObjectName.remove(alphanumericOnlyRegex); //Removes Non-alphanumeric characters from the objectName

                        ObjectSelector os(objectName, selectorObjectName, key);
                        IconStates& states = mObjectSelectorMap[os];

                        QString state = determineState(currentSelector);
                        if (state == TOKEN_HOVER)
                        {
                            states.hover = QIcon(iconPath);
                        } else if (state == TOKEN_PRESSED)
                        {
                            states.pressed = QIcon(iconPath);
                        } else
                        {
                            states.normal = QIcon(iconPath);
                        }
                    }
                }

                isBlockOpen = false;
            }
        }
    }

    mObjectNameToUiCSSFileNameMap.insert(uiFileName, objectNames);  // Associate objectNames with their respective CSS files
}

QString ThemeStylesheetParser::getThemeStylesheet(const QString& key) const
{
    QStringList valuesList = mObjectNameToUiCSSFileStyleSheetsMap.values(key);

    if (valuesList.isEmpty())
    {
        return QString();
    }

    return valuesList.join(QLatin1String("\n"));
}

QSet<QString> ThemeStylesheetParser::getObjectNamesInUICSSFile(const QString& themeKey) const
{
    return mObjectNameToUiCSSFileNameMap.value(themeKey);
}

QMap<QString, ThemeStylesheetParser::IconStates> ThemeStylesheetParser::getIconsForObject(const QString &objectName, const QString &themeKey) const
{
    QMap<QString, IconStates> selectorsIconStateInfo;

    for (auto it = mObjectSelectorMap.cbegin(); it != mObjectSelectorMap.cend(); ++it)
    {
        if (it.key().mObjectName == objectName && it.key().mThemeKey == themeKey)
        {
            selectorsIconStateInfo.insert(it.key().mSelector, it.value());
        }
    }
    return selectorsIconStateInfo;
}

QString ThemeStylesheetParser::determineState(const QString& selector)
{
    if (selector.contains(QLatin1String(":") % TOKEN_HOVER))
    {
        return TOKEN_HOVER;
    } else if (selector.contains(QLatin1String(":") % TOKEN_PRESSED))
    {
        return TOKEN_PRESSED;
    }
    return TOKEN_NORMAL;
}

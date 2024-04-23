#include "ThemeStylesheetParser.h"

#include <QLatin1String>
#include <QRegularExpression>
#include <QSet>
#include <QStringBuilder>

void ThemeStylesheetParser::parseStyleSheet(const QString &stylesheet, const QString &uiFileName)
{
    static const QRegularExpression blockStartRegex(QLatin1String(R"(([^{]+){?)"));
    static const QRegularExpression objectNameRegex(QLatin1String(R"(/\*\s*(.*?)\s*\*/)"));

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
                    mObjectNameToCSSFileStyleSheetsMap.insert(key, currentBlock); // Cache the parsed stylesheet in memory

                    objectNames.insert(objectName);
                }

                isBlockOpen = false;
            }
        }
    }

    mObjectNameToCSSFileNameMap.insert(uiFileName, objectNames);  // Associate objectNames with their respective CSS files
}

QString ThemeStylesheetParser::getThemeStylesheet(const QString& key) const
{
    QStringList valuesList = mObjectNameToCSSFileStyleSheetsMap.values(key);

    if (valuesList.isEmpty())
    {
        return QString();
    }

    return valuesList.join(QLatin1String("\n"));
}

QSet<QString> ThemeStylesheetParser::getObjectNamesInCSSFile(const QString& widgetThemeKey) const
{
    return mObjectNameToCSSFileNameMap.value(widgetThemeKey);
}




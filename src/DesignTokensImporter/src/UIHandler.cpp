#include "UIHandler.h"
#include "StylesheetParser.h"
#include "Utilities.h"
#include "Types.h"
#include "PathProvider.h"

#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QStringBuilder>
#include <QXmlStreamAttribute>


namespace DTI
{
    UIHandler::UIHandler(const QString& filePath) :
        mFilePath(filePath)
    {
        mStyleSheetInfo = parseUiFile(filePath);
    }

    bool UIHandler::containsTokens() const
    {
        return (!mStyleSheetInfo.isEmpty());
    }

    QString UIHandler::getFilePath() const
    {
        return mFilePath;
    }

    QVector<UIHandler::WidgetStyleInfo> UIHandler::parseUiFile(const QString& filePath)
    {
        QVector<WidgetStyleInfo> widgetInfoList;

        // Check if the file has a ".ui" extension
        if (!filePath.endsWith(PathProvider::UI_FILE_EXTENSION, Qt::CaseInsensitive))
        {
            qDebug() << "UIHandler::parseUiFile - ERROR! File does not have a '" << PathProvider::UI_FILE_EXTENSION <<"' extension:" << filePath;
            return widgetInfoList;
        }

        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            qDebug() << "UIHandler::parseUiFile - ERROR! Failed to open file for reading:" << filePath;
            return widgetInfoList;
        }

        QXmlStreamReader xmlReader(&file);
        WidgetStyleInfo currentWidgetInfo;

        bool parseStylesheet = false;

        while (!xmlReader.atEnd() && !xmlReader.hasError())
        {
            xmlReader.readNext();

            if (xmlReader.isStartElement())
            {
                if (xmlReader.name() == "widget")
                {
                    currentWidgetInfo.objectName = xmlReader.attributes().value("name").toString();
                    currentWidgetInfo.tokenStyles.clear();
                    parseStylesheet = false;
                }
                else if (xmlReader.name() == "property")
                {
                    if ((xmlReader.attributes().hasAttribute("name")) &&
                        (xmlReader.attributes().value("name") == "styleSheet"))
                    {
                        parseStylesheet = true;
                    }
                }
                else if (parseStylesheet && xmlReader.name() == "string")
                {
                    QString styleSheetText = xmlReader.readElementText();
                    StylesheetParser parser(styleSheetText, filePath);

                    updateCurrentWidgetInfo(currentWidgetInfo, parser);
                }
            }
            else if (xmlReader.isEndElement())
            {
                if (xmlReader.name() == "property")
                {
                    // Only append if there are tokens or state styles in the styleSheetInfo
                    if (parseStylesheet && !currentWidgetInfo.tokenStyles.isEmpty())
                    {
                        widgetInfoList.append(currentWidgetInfo);
                        parseStylesheet = false;
                    }
                }
            }
        }

        file.close();

        return widgetInfoList;
    }

    void UIHandler::updateCurrentWidgetInfo(WidgetStyleInfo& currentWidgetInfo, const StylesheetParser& parser)
    {
        auto tokenStyles = parser.tokenStyles();
        if (tokenStyles)
        {
            for (auto it = tokenStyles->constBegin(); it != tokenStyles->constEnd(); ++it)
            {
                Style tokenStyle;
                tokenStyle.cssSelectors = it.key();
                tokenStyle.properties = it.value();
                currentWidgetInfo.tokenStyles.append(tokenStyle);
            }
        }
    }

    QString UIHandler::createStyleBlock(const QString& cssSelectors, const QString& objectName, const QMap<QString, QString>& properties)
    {
        QString styleBlock;
        QTextStream stream(&styleBlock);
        stream << cssSelectors << " {\n";
        stream << "  /* " << objectName << " */\n";

        for (auto it = properties.constBegin(); it != properties.constEnd(); ++it)
        {
            stream << "  " << it.key() << ": " << it.value() << ";\n";
        }

        stream << "}\n\n";
        return styleBlock;
    }

    QString UIHandler::getStyleSheet(const QMap<QString, QString>& colourMap,  const QString& themeDirectoryName)
    {
        auto deepCopyStyleSheetInfo = [](const QVector<WidgetStyleInfo>& source) {
            QVector<WidgetStyleInfo> result;
            std::transform(source.begin(), source.end(), std::back_inserter(result),
                           [](const WidgetStyleInfo& info) {
                               return WidgetStyleInfo{info.objectName, info.tokenStyles}; // Deep copy using QMap's copy constructor
                           });
            return result;
        };

        QVector<WidgetStyleInfo> styleSheetInfoTemplate = deepCopyStyleSheetInfo(mStyleSheetInfo);

        QString cssStylesheet;
        cssStylesheet.reserve(2048); // Pre allocating memory to improve concatenation efficiency

        for (const auto& widgetInfo : styleSheetInfoTemplate)
        {
            for (const Style& tokenStyle : widgetInfo.tokenStyles)
            {
                QMap<QString, QString> resolvedProperties;
                for (auto it = tokenStyle.properties.constBegin(); it != tokenStyle.properties.constEnd(); ++it)
                {
                    const QString& propertyKey = it.key();
                    const QString& propertyValue = it.value();
                    resolvedProperties[propertyKey] = Utilities::findValueByKey(colourMap, propertyValue);
                }
                cssStylesheet += createStyleBlock(tokenStyle.cssSelectors, widgetInfo.objectName, resolvedProperties);
            }
        }

        return cssStylesheet;
    }

} // namespace DTI

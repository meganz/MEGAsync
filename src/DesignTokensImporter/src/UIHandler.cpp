#include "UIHandler.h"
#include "StylesheetParser.h"
#include "utilities.h"
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
                    currentWidgetInfo.imageStyles.clear();
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
                    if (parseStylesheet && (!currentWidgetInfo.tokenStyles.isEmpty() || !currentWidgetInfo.imageStyles.isEmpty()))
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

        auto imageStyles = parser.getImageStyles();
        if (imageStyles)
        {
            for (auto it = imageStyles->constBegin(); it != imageStyles->constEnd(); ++it)
            {
                ButtonStyle buttonStyle;
                buttonStyle.cssSelectors = it->cssSelector;
                buttonStyle.properties = it->styleMap;

                currentWidgetInfo.imageStyles.append(qMakePair(it->key, buttonStyle));
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

    QString UIHandler::determinePseudoClass(const QString& state)
    {
        if (state.contains("Pressed", Qt::CaseInsensitive))
        {
            return ":pressed";
        }
        else if (state.contains("Hovered", Qt::CaseInsensitive))
        {
            return ":hover";
        }
        return "";
    }

    QString UIHandler::getStyleSheet(const QMap<QString, QString>& colourMap,  const QString& themeDirectoryName)
    {
        auto deepCopyStyleSheetInfo = [](const QVector<WidgetStyleInfo>& source) {
            QVector<WidgetStyleInfo> result;
            std::transform(source.begin(), source.end(), std::back_inserter(result),
                           [](const WidgetStyleInfo& info) {
                               return WidgetStyleInfo{info.objectName, info.tokenStyles, info.imageStyles}; // Deep copy using QMap's copy constructor
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

            QMap<QString, QString> groupedStyles;
            for (const auto &pair : widgetInfo.imageStyles)
            {
                const QString& imageThemeKey = pair.first;
                const ButtonStyle& buttonStyle = pair.second;

                //To check whether it is Light or Dark Theme
                int lastDelimiterIndex = imageThemeKey.lastIndexOf(":");
                if (lastDelimiterIndex == -1 || !themeDirectoryName.contains(imageThemeKey.mid(lastDelimiterIndex + 1), Qt::CaseInsensitive))
                {
                    continue;
                }

                for (auto it = buttonStyle.properties.constBegin(); it != buttonStyle.properties.constEnd(); ++it)
                {
                    const QString& state = it.key();
                    const QMap<QString, QString> &properties = it.value();

                    QString pseudoClass = determinePseudoClass(state);
                    QString styleBlock = createStyleBlock(buttonStyle.cssSelectors + pseudoClass, widgetInfo.objectName, properties);
                    groupedStyles[buttonStyle.cssSelectors] += styleBlock;
                }
            }

            for (auto it = groupedStyles.cbegin(); it != groupedStyles.cend(); ++it)
            {
                const QString& styleBlock = it.value();
                cssStylesheet += styleBlock;
            }
        }

        return cssStylesheet;
    }

} // namespace DTI

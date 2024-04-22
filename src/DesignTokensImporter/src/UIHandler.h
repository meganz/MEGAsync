#ifndef UICLASS_H
#define UICLASS_H

#include <QStringList>
#include <QString>
#include <QMap>

namespace DTI
{
class StylesheetParser;
class UIHandler
{
public:
    UIHandler(const QString& filePath);

    bool containsTokens() const;
    QString getFilePath() const;
    QString getStyleSheet(const QMap<QString, QString>& colourMap, const QString& themeDirectoryName);


private:
    struct Style
    {
        QString cssSelectors;
        QMap<QString, QString> properties;
    };

    struct WidgetStyleInfo
    {
        QString objectName;
        QList<Style> tokenStyles;
    };

    QVector<WidgetStyleInfo> parseUiFile(const QString& filePath);
    void updateCurrentWidgetInfo(WidgetStyleInfo& currentWidgetInfo, const StylesheetParser& parser);
    QString createStyleBlock(const QString& cssSelectors, const QString& objectName, const QMap<QString, QString>& properties);

    QString mFilePath;
    QVector<WidgetStyleInfo> mStyleSheetInfo;
};
} // namespace DTI

#endif // UICLASS_H

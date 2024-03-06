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
    typedef QMap<QString, QMap<QString, QString>> styleMap;

    struct Style
    {
        QString cssSelectors;
        QMap<QString, QString> properties;
    };

    struct ButtonStyle
    {
        QString cssSelectors;
        styleMap properties;
    };

    struct WidgetStyleInfo
    {
        QString objectName;
        QList<Style> tokenStyles;
        QVector<QPair<QString, ButtonStyle>> imageStyles;
    };

    QVector<WidgetStyleInfo> parseUiFile(const QString& filePath);
    void updateCurrentWidgetInfo(WidgetStyleInfo& currentWidgetInfo, const StylesheetParser& parser);
    QString createStyleBlock(const QString& cssSelectors, const QString& objectName, const QMap<QString, QString>& properties);
    QString determinePseudoClass(const QString& state);

    QString mFilePath;
    QVector<WidgetStyleInfo> mStyleSheetInfo;
};
} // namespace DTI

#endif // UICLASS_H

#ifndef STYLESHEETPARSER_H
#define STYLESHEETPARSER_H

#include "StyleDefinitions.h"

#include <QMap>
#include <QString>

namespace DTI
{
class StylesheetParser
{
public:

    struct ImageThemeStyleInfo
    {
        QString key;
        QString cssSelector;
        ButtonStateStyleMap styleMap;
    };

    StylesheetParser(const QString& styleSheet, const QString& uiFilePath);

    const QMultiMap<QString, PropertiesMap>& tokenStyles() const;

    const QVector<ImageThemeStyleInfo>& getImageStyles() const;

private:

    struct CurrentStyleBlock
    {
        QString styleSheetLine;
        QString content;
        QString selector;
        QString objectName;
        bool hasSvgIcon = false;
        bool hasTokens = false;
        bool isBlockOpen = false;
        int braceLevel = 0;

        void clear()
        {
            styleSheetLine.clear();
            content.clear();
            selector.clear();
            objectName.clear();
            hasSvgIcon = false;
            hasTokens = false;
            isBlockOpen = false;
            braceLevel = 0;
        }
    };

    void processStyleSheet();
    bool parseCurrentStyleBlock(CurrentStyleBlock& currentStyleSheetLine);
    void processCurrentStyleBlockForTokens(const CurrentStyleBlock& currentBlock);
    void processCurrentStyleBlockForSVGIcons(const CurrentStyleBlock& currentBlock);

    const QString& mStyleSheet;
    const QString& mUiFilePath;
    QMultiMap<QString, PropertiesMap> mTokenStyles;
    QVector<ImageThemeStyleInfo>  mImageStyles;
};
}
#endif // STYLESHEETPARSER_H

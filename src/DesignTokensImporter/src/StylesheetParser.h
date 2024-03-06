#ifndef STYLESHEETPARSER_H
#define STYLESHEETPARSER_H

#include "Types.h"
#include "IQTWIDGETStyleTarget.h"

#include <QMap>
#include <QString>

#include <memory>

namespace DTI
{
class StylesheetParser
{
public:

    StylesheetParser(const QString& styleSheet, const QString& uiFilePath);
    const std::shared_ptr<QMultiMap<QString, PropertiesMap>> tokenStyles() const;
    std::shared_ptr<QVector<ImageThemeStyleInfo>> getImageStyles() const;

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
    void processStyleTokens(const CurrentStyleBlock& currentBlock, Targets style);
    std::shared_ptr<IQTWIDGETStyleTarget> getStyleTarget(const QString& styleTargetId) const;

    const QString& mStyleSheet;
    const QString& mUiFilePath;
    QMultiMap<QString, PropertiesMap> mTokenStyles;
    QVector<ImageThemeStyleInfo>  mImageStyles;
    QMap<QString,  std::shared_ptr<IQTWIDGETStyleTarget>> styleTargetMap;
};
}
#endif // STYLESHEETPARSER_H

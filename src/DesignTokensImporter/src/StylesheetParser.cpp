#include "StylesheetParser.h"
#include "utilities.h"
#include "SVGIcon.h"

#include <QStringList>

namespace DTI
{

static const QString UI_TOKEN_IDENTIFIER = QString::fromLatin1("/*token_");

StylesheetParser::StylesheetParser(const QString& styleSheet, const QString& uiFilePath) : mStyleSheet(styleSheet),
    mUiFilePath(uiFilePath)
{
    processStyleSheet();
}

void StylesheetParser::processStyleSheet()
{
    QStringList styleSheetLines = mStyleSheet.split(QChar('\n'), Qt::SkipEmptyParts);
    CurrentStyleBlock currentBlock;

    for (const QString& styleSheetLine : styleSheetLines)
    {
        currentBlock.styleSheetLine = styleSheetLine;
        if(parseCurrentStyleBlock(currentBlock))
        {
            if(currentBlock.hasTokens)
            {
                processCurrentStyleBlockForTokens(currentBlock);
            }

            if (currentBlock.hasSvgIcon)
            {
                processCurrentStyleBlockForSVGIcons(currentBlock);
            }

            currentBlock.clear();
        }
    }
}

bool StylesheetParser::parseCurrentStyleBlock(CurrentStyleBlock& currentBlock)
{
    QString trimmedLine = currentBlock.styleSheetLine.trimmed();

    if (!currentBlock.isBlockOpen)
    {
        int braceIndex = trimmedLine.indexOf('{');
        if (braceIndex != -1)
        {
            currentBlock.selector = braceIndex > 0 ? trimmedLine.left(braceIndex).trimmed() : currentBlock.content.trimmed();// The current line, or the part before the '{', is the identifier
            currentBlock.content.clear();
            currentBlock.hasSvgIcon = false;
            currentBlock.isBlockOpen = true;
            currentBlock.braceLevel = 1;
            if (braceIndex < trimmedLine.length() - 1)
            {
                // Content follows the opening brace on the same line
                currentBlock.content = trimmedLine.mid(braceIndex + 1).trimmed() + "\n";
            }
        }
        else
        {
            // If no brace is found, store the line as it is identifier/selector
            currentBlock.content = trimmedLine;
        }
    }
    else
    {
        currentBlock.braceLevel += trimmedLine.count('{');
        currentBlock.braceLevel -= trimmedLine.count('}');

        currentBlock.content += trimmedLine + "\n";

        if (trimmedLine.contains(".svg"))
        {
            currentBlock.hasSvgIcon = true;
        }

        if (trimmedLine.contains(UI_TOKEN_IDENTIFIER))
        {
            currentBlock.hasTokens = true;
        }

        if (currentBlock.braceLevel == 0)
        {
            currentBlock.isBlockOpen = false;
            return true; //Current Block is completed
        }
    }

    return false;
}

void StylesheetParser::processCurrentStyleBlockForTokens(const CurrentStyleBlock& currentBlock)
{
    QStringList styleSheetLines = currentBlock.content.split(QChar('\n'), Qt::SkipEmptyParts);

    PropertiesMap tokenProperties;
    for (const QString& styleSheetLine : styleSheetLines)
    {
        if (styleSheetLine.contains(UI_TOKEN_IDENTIFIER))
        {
            // Found a line in format: "/*token_background-color: {{surface-1}};*/"
            QString key = Utilities::getSubStringBetweenMarkers(styleSheetLine, UI_TOKEN_IDENTIFIER, ":");
            QString value = Utilities::getSubStringBetweenMarkers(styleSheetLine, "{{", "}}");

            // Skip lines with empty keys or values
            if (!key.isEmpty() && !value.isEmpty())
            {
                tokenProperties.insert(key,value);
            }
        }
    }

    mTokenStyles.insert(currentBlock.selector, tokenProperties);
}

void StylesheetParser::processCurrentStyleBlockForSVGIcons(const CurrentStyleBlock& currentBlock)
{
    QString uiFileName = Utilities::extractFileNameNoExtension(mUiFilePath);

    SVGIcon svgIcon(uiFileName);

    bool isProcessCurrentStyleSheetBlockSuccess = svgIcon.processStylesheet(currentBlock.content);
    if(!isProcessCurrentStyleSheetBlockSuccess)
    {
        return;
    }


    bool isGenerateSvgImagSuccess = svgIcon.generateSVGImageBasedOnState();
    if(!isGenerateSvgImagSuccess)
    {
        return;
    }

    const auto& imageStyleInfo = svgIcon.getImageStyle();
    for (auto it = imageStyleInfo.constBegin(); it != imageStyleInfo.constEnd(); ++it)
    {
        const QString& key = it.key();
        const ButtonStateStyleMap& styleMap = it.value();

        if (!key.isEmpty() && !styleMap.isEmpty())
        {
            ImageThemeStyleInfo imageStyle;
            imageStyle.key = key;
            imageStyle.styleMap = styleMap;
            imageStyle.cssSelector = currentBlock.selector;

            mImageStyles.append(imageStyle);
        }
    }
}

const QVector<StylesheetParser::ImageThemeStyleInfo>& StylesheetParser::getImageStyles() const
{
    return mImageStyles;
}

const QMultiMap<QString, PropertiesMap>& StylesheetParser::tokenStyles() const
{
    return mTokenStyles;
}

}


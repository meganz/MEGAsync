#include "StylesheetParser.h"
#include "QTWIDGETColorStyleTarget.h"
#include "QTWIDGETStyleTargetFactory.h"
#include "Utilities.h"

#include <QStringList>

namespace DTI
{

static const QRegularExpression UI_TOKEN_IDENTIFIER(R"(/\*token_\s*([\w-]+)\s*:\s*([^;]+);\s*\*/)");

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
        if (parseCurrentStyleBlock(currentBlock))
        {
            if (currentBlock.hasTokens)
            {
                processStyleTokens(currentBlock, Targets::ColorStyle);
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

        QRegularExpressionMatch match = UI_TOKEN_IDENTIFIER.match(trimmedLine);
        if (match.hasMatch())
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

void StylesheetParser::processStyleTokens(const CurrentStyleBlock& currentBlock, Targets style)
{
    IQTWIDGETStyleTarget::CurrentStyleBlockInfo currentBlockInfo;
    currentBlockInfo.content = currentBlock.content;
    currentBlockInfo.selector = currentBlock.selector;
    currentBlockInfo.filePath = mUiFilePath;

    auto targetString = Utilities::targetToString(style);
    std::shared_ptr<IQTWIDGETStyleTarget> qtwidgetTargetStyle;

    auto it = styleTargetMap.find(targetString);
    if (it != styleTargetMap.end())
    {
        qtwidgetTargetStyle = it.value();
    }
    else
    {
        qtwidgetTargetStyle = getStyleTarget(targetString);
        if (qtwidgetTargetStyle)
        {
            styleTargetMap.insert(targetString, qtwidgetTargetStyle);
        }
    }

    if(qtwidgetTargetStyle)
    {
        qtwidgetTargetStyle->process(currentBlockInfo);
    }
}

std::shared_ptr<IQTWIDGETStyleTarget> StylesheetParser::getStyleTarget(const QString &styleTargetId) const
{
    return std::shared_ptr<IQTWIDGETStyleTarget>(QTWIDGETStyleTargetFactory::getQTWIDGETStyleTarget(styleTargetId));
}

const std::shared_ptr<QMultiMap<QString, PropertiesMap>> StylesheetParser::tokenStyles() const
{
    auto retMap = std::make_shared< QMultiMap<QString, PropertiesMap>>();
    auto it = styleTargetMap.find(Utilities::targetToString(Targets::ColorStyle));
    if (it != styleTargetMap.end())
    {
        auto& qtwidgetTargetStyle = it.value();
        auto colorStyleTarget = dynamic_cast<QTWIDGETColorStyleTarget*>(qtwidgetTargetStyle.get());
        if (colorStyleTarget)
        {
            *retMap = colorStyleTarget->getTokenStyles();
        }
    }

    return retMap;
}
}

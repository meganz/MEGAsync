#include "QTWIDGETColorStyleTarget.h"
#include "QTWIDGETStyleTargetFactory.h"

#include "Utilities.h"

#include <QDir>
#include <QDebug>
#include <QFileInfo>
#include <QTextStream>
#include <QStringBuilder>

#include <iostream>

using namespace DTI;

static const QRegularExpression UI_TOKEN_IDENTIFIER(R"(/\*token_\s*([\w-]+)\s*:\s*([^;]+);\s*\*/)");

bool QTWIDGETColorStyleTarget::registered = QTWIDGETStyleFactory<QTWIDGETColorStyleTarget>::Register(Utilities::targetToString(Targets::ColorStyle).toStdString());

void QTWIDGETColorStyleTarget::process(const CurrentStyleBlockInfo& currentBlockInfo)
{
    QStringList styleSheetLines = currentBlockInfo.content.split(QChar('\n'), Qt::SkipEmptyParts);
    static const QRegularExpression braceRegex(R"(\{+\s*|\s*\}+)");

    PropertiesMap tokenProperties;
    for (const QString& styleSheetLine : styleSheetLines)
    {
        QRegularExpressionMatch match = UI_TOKEN_IDENTIFIER.match(styleSheetLine);
        if (match.hasMatch() && match.lastCapturedIndex() == 2)
        {
            QString key = match.captured(1).toLower();
            QString value = match.captured(2).trimmed().remove(braceRegex);

            // Skip lines with empty keys or values
            if (!key.isEmpty() && !value.isEmpty())
            {
                tokenProperties.insert(key,value);
            }
        }
    }

    mTokenStyles.insert(currentBlockInfo.selector, tokenProperties);
}

const QMultiMap<QString, PropertiesMap> &QTWIDGETColorStyleTarget::getTokenStyles() const
{
    return mTokenStyles;
}





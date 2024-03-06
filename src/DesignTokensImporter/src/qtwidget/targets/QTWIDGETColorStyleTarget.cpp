#include "QTWIDGETColorStyleTarget.h"
#include "QTWIDGETStyleTargetFactory.h"

#include "utilities.h"

#include <QDir>
#include <QDebug>
#include <QFileInfo>
#include <QTextStream>
#include <QStringBuilder>

#include <iostream>

using namespace DTI;

static const QString UI_TOKEN_IDENTIFIER = QString::fromLatin1("/*token_");

bool QTWIDGETColorStyleTarget::registered = QTWIDGETStyleFactory<QTWIDGETColorStyleTarget>::Register(Utilities::targetToString(Targets::ColorStyle).toStdString());

void QTWIDGETColorStyleTarget::process(const CurrentStyleBlockInfo& currentBlockInfo)
{
    QStringList styleSheetLines = currentBlockInfo.content.split(QChar('\n'), Qt::SkipEmptyParts);

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

    mTokenStyles.insert(currentBlockInfo.selector, tokenProperties);
}

const QMultiMap<QString, PropertiesMap> &QTWIDGETColorStyleTarget::getTokenStyles() const
{
    return mTokenStyles;
}





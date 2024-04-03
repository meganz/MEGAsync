#include "QTWIDGETImageStyleTarget.h"
#include "QTWIDGETStyleTargetFactory.h"

#include "Utilities.h"
#include "SVGIcon.h"

#include <QDir>
#include <QDebug>
#include <QFileInfo>
#include <QTextStream>
#include <QStringBuilder>


#include <iostream>

using namespace DTI;

bool QTWIDGETImageStyleTarget::registered = QTWIDGETStyleFactory<QTWIDGETImageStyleTarget>::Register(Utilities::targetToString(Targets::ImageStyle).toStdString());

void QTWIDGETImageStyleTarget::process(const CurrentStyleBlockInfo& currentBlockInfo)
{
    QString uiFileName = Utilities::extractFileNameNoExtension(currentBlockInfo.filePath);

    SVGIcon svgIcon(uiFileName);

    bool isProcessCurrentStyleSheetBlockSuccess = svgIcon.processStylesheet(currentBlockInfo.content);
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
            imageStyle.cssSelector = currentBlockInfo.selector;

            mImageStyles.append(imageStyle);
        }
    }
}

const QVector<ImageThemeStyleInfo>& QTWIDGETImageStyleTarget::getImageStyles() const
{
    return mImageStyles;
}



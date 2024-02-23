#include "QMLColorStyleTarget.h"

#include "QMLStyleTargetFactory.h"

#include <QDir>
#include <QDebug>
#include <QFileInfo>
#include <QTextStream>

#include <iostream>

using namespace DTI;

static const QString qmlColorStyleTargetPath = "%1/gui/qml/common/themes/%2";
static const QString qmlColorStyleFileName = "%1/Colors.qml";
static const QString colorStyleHeader = QString::fromUtf8("import QtQuick 2.15\n\nQtObject {\n\n");
static const QString colorStyleLine = QString::fromUtf8("\treadonly property color %1: \"%2\" \n");
static const QString colorStyleFooter = QString::fromUtf8("}\n");

bool QMLColorStyleTarget::registered = ConcreteQMLStyleFactory<QMLColorStyleTarget>::Register("qmlColorStyle");

void QMLColorStyleTarget::deploy(QString theme, const ColourMap& colourMap) const
{
    auto qmlColorStyleFilePath = qmlColorStyleTargetPath.arg(QDir::currentPath(), theme);

    QDir dir(qmlColorStyleFilePath);
    if (!dir.exists() && !dir.mkpath(qmlColorStyleFilePath))
    {
        qWarning() << "couldn't create directory " << qmlColorStyleFilePath;

        return;
    }

    QFile data(qmlColorStyleFileName.arg(qmlColorStyleFilePath));
    if (data.open(QFile::WriteOnly | QFile::Truncate))
    {
        QTextStream stream(&data);

        stream << colorStyleHeader;
        for (auto it = colourMap.constBegin(); it != colourMap.constEnd(); ++it)
        {
            const auto& tokenId = it.key();
            const auto& color = it.value();
            stream << colorStyleLine.arg(normalizeTokenId(tokenId), color);
        }
        stream << colorStyleFooter;
    }
}

/*
 * helper function : qml style files can't have - as a word separators, so we have to convert
 * from surface-al to surfaceAl
*/
QString QMLColorStyleTarget::normalizeTokenId(QString tokenId) const
{
    while(tokenId.indexOf('-') != -1)
    {
        auto index = tokenId.indexOf('-');
        tokenId.remove(index, 1);

        if (index < tokenId.size())
        {
            auto afterSeparatorChar = tokenId.at(index);
            tokenId[index] = afterSeparatorChar.toUpper();
        }
    }

    return tokenId;
}

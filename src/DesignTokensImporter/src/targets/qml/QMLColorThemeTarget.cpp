#include "QMLColorThemeTarget.h"

#include "DesignTargetFactory.h"
#include "PathProvider.h"
#include "QMLTargetUtils.h"
#include "Utilities.h"

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QTextStream>

using namespace DTI;

static const QString THEME_BASE_PATH = "/qml/common/themes/";
static const QString COLORS_QML_FILENAME = "Colors.qml";
static const QString colorThemeHeader = QString::fromUtf8("import QtQuick 2.15\n\nQtObject {\n\n");
static const QString colorThemeLine = QString::fromUtf8("\treadonly property color %1: \"%2\"\n");
static const QString colorThemeFooter = QString::fromUtf8("}\n");

bool QMLColorThemeTarget::registered = ConcreteDesignTargetFactory<QMLColorThemeTarget>::Register("qmlColorTheme");

void QMLColorThemeTarget::deploy(const DesignAssets& designAssets) const
{
    if (designAssets.colorTokens.isEmpty())
    {
        qWarning() << "Error : No themed colour data to deploy/process.";

        return;
    }

    for (auto themeIt = designAssets.colorTokens.constBegin(); themeIt != designAssets.colorTokens.constEnd(); ++themeIt)
    {
        const auto& colourMap = themeIt.value();

        auto themeName = themeIt.key().toLower();
        if (!themeName.isEmpty() && !colourMap.isEmpty())
        {
            QString themeDirPath = PathProvider::getUIPath() + THEME_BASE_PATH + themeName;

            QDir dir(themeDirPath);
            if (!dir.exists() && !dir.mkpath("."))
            {
                qWarning() << "couldn't create directory " << themeDirPath;
                return;
            }

            QString colorFilePath = themeDirPath + "/" + COLORS_QML_FILENAME;
            QFile data(colorFilePath);
            if (data.open(QFile::WriteOnly | QFile::Truncate))
            {
                QTextStream stream(&data);
                stream << colorThemeHeader;

                for (auto it = colourMap.constBegin(); it != colourMap.constEnd(); ++it)
                {
                    const auto& tokenId = it.key();
                    const auto& color = it.value().toUpper();
                    stream << colorThemeLine.arg(normalizeTokenId(tokenId), color);
                }

                stream << colorThemeFooter;

                Utilities::logInfoMessage(QString::fromUtf8("The target qmlColorTheme has successfully generated the file : %0").arg(data.fileName()));
            }
            else
            {
                qWarning() << __func__ << " Error : couldn't open file " << data.fileName();
            }

            data.close();
        }
    }
}

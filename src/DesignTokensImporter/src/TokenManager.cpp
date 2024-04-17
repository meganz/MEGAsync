#include "TokenManager.h"
#include "Utilities.h"
#include "PathProvider.h"
#include "IThemeGenerator.h"
#include "QMLThemeGenerator.h"
#include "QTWIDGETStyleGenerator.h"

#include <QDebug>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QStringBuilder>

using namespace DTI;

TokenManager::TokenManager()
{
    mCurrentDir = QDir::currentPath();
    qDebug() << __func__ << " Current working directory : " << mCurrentDir;
}

TokenManager* TokenManager::instance()
{
    static TokenManager manager;
    return &manager;
}

void TokenManager::run()
{
    // find core.json file
    QString pathToCoreFile = Utilities::resolvePath(mCurrentDir, PathProvider::RELATIVE_CORE_FILE_PATH);
    QFile coreFile(pathToCoreFile);
    if (!coreFile.exists())
    {
        qCritical() << __func__ << " Error : No core.json file found in  " << pathToCoreFile;
        return;
    }

    // load core.json file
    CoreMap coreMap = parseCore(pathToCoreFile);

    // find json colors themed files
    QString pathToColorThemedFiles = Utilities::resolvePath(mCurrentDir, PathProvider::RELATIVE_COLOR_TOKENS_PATH);
    QStringList colorThemedPathFiles = Utilities::findFilesInDir(pathToColorThemedFiles, PathProvider::JSON_NAME_FILTER);

    // stop if there are no Design Token files
    if (colorThemedPathFiles.isEmpty())
    {
        qCritical() << __func__ << " ERROR! No color themed files found in folder " << pathToColorThemedFiles;
        return;
    }

    // parse json color themed files.
    ThemedColourMap fileToColourMap = parseColorTokenJSON(colorThemedPathFiles, coreMap);

    // qml style generator entry point.
    std::unique_ptr<IThemeGenerator> styleGenerator{new QmlThemeGenerator()};
    styleGenerator->start(fileToColourMap);

    // qtwidget style generator entry point.
    std::unique_ptr<IThemeGenerator> qtWidgetStyleGenerator{new QTWIDGETStyleGenerator()};
    qtWidgetStyleGenerator->start(fileToColourMap);
}

void TokenManager::recurseCore(QString category, const QJsonObject& categoryObject, CoreMap& coreMap)
{
    const QStringList tokenKeys = categoryObject.keys();

    if (tokenKeys.contains(QLatin1String("$value")) && tokenKeys.contains(QLatin1String("$type")))
    {
        QJsonValue jType = categoryObject["$type"];
        QJsonValue jValue = categoryObject["$value"];

        if (!jType.isNull() && jValue.isString())
        {
            QString type = jType.toString();

            if (type == "color")
            {
                coreMap.insert(category, jValue.toString().remove(QChar('#')));
            }
        }
    }
    else
    {
        for (int index = 0; index < tokenKeys.size(); ++index)
        {
            QString subCategory = category + "." + tokenKeys[index];
            QJsonObject categoryObj = categoryObject.value(tokenKeys[index]).toObject();

            recurseCore(subCategory, categoryObj, coreMap);
        }
    }
}

CoreMap TokenManager::parseCore(const QString& coreFilePath)
{
    static QString ColorKey = "Colors";

    CoreMap returnValue;

    QFile inputFile(coreFilePath);
    if (!inputFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << __func__ << " Error : opening core file " << coreFilePath;
        return returnValue;
    }

    // Parse the input JSON document
    QJsonDocument jsonDocument = QJsonDocument::fromJson(inputFile.readAll());
    if (jsonDocument.isNull())
    {
        qDebug() << __func__ << " Error : parsing core file " << coreFilePath;
        return returnValue;
    }

    QJsonObject jsonObject = jsonDocument.object();

    const QStringList categoryKeys = jsonObject.keys();
    auto foundColorKeyIt = std::find_if(categoryKeys.constBegin(), categoryKeys.constEnd(), [](const QString& key){
        return key == ColorKey;
    });

    if (foundColorKeyIt != categoryKeys.constEnd())
    {
        QJsonObject categoryObject = jsonObject.value(ColorKey).toObject();
        recurseCore(ColorKey, categoryObject, returnValue);
    }

    return returnValue;
}

ThemedColourMap TokenManager::parseColorTokenJSON(const QStringList& colorTokenFilePathsList, const CoreMap& coreMap)
{
    ThemedColourMap retMap;

    if (colorTokenFilePathsList.isEmpty())
    {
        qDebug() << __func__ << " Error : colorTokenFilePathsList is empty.";
        return retMap;
    }

    foreach (const QString& colorTokenFilePath, colorTokenFilePathsList)
    {
        ColourMap colourMap = Utilities::parseColorThemeJSON(colorTokenFilePath, coreMap);
        if (colourMap.isEmpty())
        {
            qDebug() << __func__ << " Error : ColourMap is empty on file " << colorTokenFilePath;
            continue;
        }

        QString theme = Utilities::themeToString(Utilities::getTheme(colorTokenFilePath));
        if (theme.isEmpty())
        {
            qDebug() << __func__ << " Error : No valid theme found on " << colorTokenFilePath;

            continue;
        }

        retMap.insert(theme, colourMap);
    }

    return retMap;
}



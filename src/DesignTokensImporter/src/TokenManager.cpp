#include "TokenManager.h"
#include "Utilities.h"
#include "PathProvider.h"
#include "IStyleGenerator.h"
#include "QMLStyleGenerator.h"
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
    qDebug() << __PRETTY_FUNCTION__ << " Current working directory : " << mCurrentDir;
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
        qCritical() << __PRETTY_FUNCTION__ << " Error : No core.json file found in  " << pathToCoreFile;
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
        qCritical() << __PRETTY_FUNCTION__ << " ERROR! No color themed files found in folder " << pathToColorThemedFiles;
        return;
    }

    // parse json color themed files.
    ThemedColourMap fileToColourMap = parseColorTokenJSON(colorThemedPathFiles, coreMap);

    if(!generateTokenFiles(fileToColourMap))
    {
        qDebug() << __PRETTY_FUNCTION__ << " ERROR! Unable to generate token files";
        return;
    }

    // qml style generator entry point.
    std::unique_ptr<IStyleGenerator> styleGenerator{new QmlStyleGenerator()};
    styleGenerator->start(fileToColourMap);

    // qtwidget style generator entry point.
    std::unique_ptr<IStyleGenerator> qtWidgetStyleGenerator{new QTWIDGETStyleGenerator()};
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
                coreMap.insert(category, jValue.toString());
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
        qDebug() << __PRETTY_FUNCTION__ << " Error : opening core file " << coreFilePath;
        return returnValue;
    }

    // Parse the input JSON document
    QJsonDocument jsonDocument = QJsonDocument::fromJson(inputFile.readAll());
    if (jsonDocument.isNull())
    {
        qDebug() << __PRETTY_FUNCTION__ << " Error : parsing core file " << coreFilePath;
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
        qDebug() << __PRETTY_FUNCTION__ << " Error : colorTokenFilePathsList is empty.";
        return retMap;
    }

    foreach (const QString& colorTokenFilePath, colorTokenFilePathsList)
    {
        ColourMap colourMap = Utilities::parseColorThemeJSON(colorTokenFilePath, coreMap);
        if (colourMap.isEmpty())
        {
            qDebug() << __PRETTY_FUNCTION__ << " Error : ColourMap is empty on file " << colorTokenFilePath;
            continue;
        }

        QString theme = Utilities::themeToString(Utilities::getTheme(colorTokenFilePath));
        if (theme.isEmpty())
        {
            qDebug() << __PRETTY_FUNCTION__ << " Error : No valid theme found on " << colorTokenFilePath;

            continue;
        }

        retMap.insert(theme, colourMap);
    }

    return retMap;
}

bool TokenManager::generateTokenFiles(const ThemedColourMap& fileToColourMap)
{
    // Create Generated Directory
    Utilities::createDirectory(Utilities::resolvePath(mCurrentDir, PathProvider::RELATIVE_GENERATED_PATH));

    bool ret = true;

    // Save colourMaps .JSON files
    // Generate token values in hex format
    for (auto it = fileToColourMap.constBegin(); it != fileToColourMap.constEnd(); ++it)
    {
        const QString& filePath = it.key();
        const ColourMap& colourMap = it.value();
        QString fileName = Utilities::extractFileName(filePath);
        if(!Utilities::writeColourMapToJSON(colourMap, Utilities::resolvePath(mCurrentDir, PathProvider::RELATIVE_GENERATED_PATH) + "/" + fileName))
        {
            ret = false;
        }
    }

    return ret;
}


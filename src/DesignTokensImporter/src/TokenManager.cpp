#include "TokenManager.h"
#include "utilities.h"
#include "PathProvider.h"
#include "IStyleGenerator.h"
#include "QMLStyleGenerator.h"
#include "QTWIDGETStyleGenerator.h"

#include <QDebug>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QStringBuilder>

namespace DTI
{
    TokenManager::TokenManager()
    {
        mCurrentDir = QDir::currentPath();
        qDebug() << "TokenManager::TokenManager - Current working directory = " << mCurrentDir;
    }

    TokenManager* TokenManager::instance()
    {
        static TokenManager manager;
        return &manager;
    }

    void TokenManager::run()
    {
        // Load core.json file
        QString pathToCoreFile = Utilities::resolvePath(mCurrentDir, PathProvider::RELATIVE_CORE_FILE_PATH);
        QFile coreFile(pathToCoreFile);
        if (!coreFile.exists())
        {
            qCritical() << "TokenManager::run - ERROR! No core.json file found in  " << pathToCoreFile;
            return;
        }

        CoreMap coreMap = parseCore(pathToCoreFile);

        // Load .json colors themed files token files
        QString pathToColorThemedFiles = Utilities::resolvePath(mCurrentDir, PathProvider::RELATIVE_COLOR_TOKENS_PATH);
        QStringList colorThemedPathFiles = Utilities::findFilesInDir(pathToColorThemedFiles, PathProvider::JSON_NAME_FILTER);

        // Stop if there are no Design Token files
        if (colorThemedPathFiles.isEmpty())
        {
            qCritical() << "TokenManager::run - ERROR! No Design Token .JSON files found in folder " << pathToColorThemedFiles;
            return;
        }

        // Parse .json token files and create colour map
        FilePathColourMap fileToColourMap = parseTokenJSON(colorThemedPathFiles);
        if(!generateTokenFiles(fileToColourMap))
        {
            qDebug() << "TokenManager::run - ERROR! Unable to generate token files";
            return;
        }

        // qml style generator entry point.
        std::unique_ptr<IStyleGenerator> styleGenerator{new QmlStyleGenerator()};
        styleGenerator->start(fileToColourMap);

        // qtwidget style generator entry point.
        std::unique_ptr<IStyleGenerator> qtWidgetStyleGenerator{new QTWIDGETStyleGenerator()};
        qtWidgetStyleGenerator->start(fileToColourMap);
    }

    void TokenManager::recurseCore(QString category, const QJsonObject& categoryObject, CoreMap& returnValue)
    {
        // Use an index-based loop to avoid detachment warning
        const QStringList tokenKeys = categoryObject.keys();

        if (tokenKeys.contains(QLatin1String("$value")) && tokenKeys.contains(QLatin1String("$type")))
        {
            QJsonValue typeValue = categoryObject["$type"];
            QJsonValue valueValue = categoryObject["$value"];

            if (typeValue.isString() && valueValue.isString())
            {
                QString type = typeValue.toString();
                QString value = valueValue.toString();

                if (type == "color")
                {
                    /*
                    QColor color = stringToColor(value);
                    // Strip "--color-" from beginning of token
                    colourMap.insert(token.mid(COLOUR_TOKEN_START.size()),
                                     color.name(QColor::HexArgb));
                    */

                    returnValue.insert(category, categoryObject["$value"].toString());
                }
            }
        }
        else
        {
            for (int index = 0; index < tokenKeys.size(); ++index)
            {
                QString subCategory = category + "." + tokenKeys[index];
                QJsonObject categoryObj = categoryObject.value(tokenKeys[index]).toObject();

                recurseCore(subCategory, categoryObj, returnValue);
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
            qDebug() << __PRETTY_FUNCTION__ << " Error opening core file " << coreFilePath;
            return returnValue;
        }

        // Parse the input JSON document
        QJsonDocument jsonDocument = QJsonDocument::fromJson(inputFile.readAll());
        if (jsonDocument.isNull())
        {
            qDebug() << __PRETTY_FUNCTION__ << " Error parsing core file " << coreFilePath;
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

    FilePathColourMap TokenManager::parseTokenJSON(const QStringList& tokenFilePathsList)
    {
        FilePathColourMap retMap;

        if (tokenFilePathsList.isEmpty())
        {
            return retMap;
        }

        foreach (const QString& filePath, tokenFilePathsList)
        {
            ColourMap colourMap = Utilities::parseTokenJSON(filePath);
            retMap.insert(filePath, colourMap);
        }

        return retMap;
    }

    bool TokenManager::generateTokenFiles(const FilePathColourMap& fileToColourMap)
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

} // namespace DTI

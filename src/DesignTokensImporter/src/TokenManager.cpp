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
        // Load .json token files
        QStringList tokenFilePathsList = Utilities::findFilesInDir(Utilities::resolvePath(mCurrentDir, PathProvider::RELATIVE_TOKENS_PATH), PathProvider::JSON_NAME_FILTER);

        // Stop if there are no Design Token files
        if (tokenFilePathsList.isEmpty())
        {
            qDebug() << "TokenManager::run - ERROR! No Design Token .JSON files found in folder " << Utilities::resolvePath(mCurrentDir, PathProvider::RELATIVE_TOKENS_PATH);
            return;
        }

        // Parse .json token files and create colour map
        FilePathColourMap fileToColourMap = parseTokenJSON(tokenFilePathsList);
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

    FilePathColourMap TokenManager::parseTokenJSON(const QStringList& tokenFilePathsList)
    {
        FilePathColourMap retMap;

        if (tokenFilePathsList.isEmpty())
        {
            return retMap;
        }

        foreach (const QString& filePath, tokenFilePathsList)
        {
            ColourMap colourMap =  Utilities::parseTokenJSON(filePath);
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

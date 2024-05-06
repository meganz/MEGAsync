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

static const QString CoreMain = "Core/Main";
static const QString CoreColors = "Colors";
static const QString SemanticTokens = "Semantic tokens";

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
    // look for tokens.json file
    QString pathToDesignTokensFile = Utilities::resolvePath(mCurrentDir, PathProvider::RELATIVE_DESIGN_TOKENS_FILE_PATH);
    QFile designTokensFile(pathToDesignTokensFile);
    if (!designTokensFile.exists())
    {
        qCritical() << __func__ << " Error : No tokens.json file found in " << pathToDesignTokensFile;
        return;
    }

    if (!designTokensFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << __func__ << " Error : opening tokens.json file " << pathToDesignTokensFile;
        return;
    }

    // load core data
    const CoreData& coreData = parseCore(designTokensFile);

    // parse json color themed files.
    const ThemedColorData& themesData = parseTheme(designTokensFile, coreData);

    // qml style generator entry point.
    std::unique_ptr<IThemeGenerator> styleGenerator{new QmlThemeGenerator()};
    styleGenerator->start(themesData);

    // qtwidget style generator entry point.
    std::unique_ptr<IThemeGenerator> qtWidgetStyleGenerator{new QTWIDGETStyleGenerator()};
    qtWidgetStyleGenerator->start(themesData);
}

void TokenManager::recurseCore(QString category, const QJsonObject& coreColors, CoreData& coreData)
{
    const QStringList tokenKeys = coreColors.keys();

    if (tokenKeys.contains(QLatin1String("value")) && tokenKeys.contains(QLatin1String("type")))
    {
        QJsonValue jType = coreColors["type"];
        QJsonValue jValue = coreColors["value"];

        if (!jType.isNull() && jValue.isString())
        {
            QString type = jType.toString();

            if (type == "color")
            {
                /*
                 * Core color hex value format is #RRGGBBAA,
                 * but we need to convert it to AARRGGBB.
                */
                QString coreColorHex = jValue.toString().remove(QChar('#'));
                if (coreColorHex.length() == 8)
                {
                    coreColorHex = coreColorHex.right(2) + coreColorHex.left(6);
                }

                coreData.insert(category, coreColorHex);
            }
        }
    }
    else
    {
        for (int index = 0; index < tokenKeys.size(); ++index)
        {
            QString subCategory = category + "." + tokenKeys[index];
            QJsonObject categoryObj = coreColors.value(tokenKeys[index]).toObject();

            recurseCore(subCategory, categoryObj, coreData);
        }
    }
}

CoreData TokenManager::parseCore(QFile& designTokensFile)
{
    const QString errorPrefix = __func__ + QString(" Error : parsing design tokens file, ");

    CoreData coreData;

    designTokensFile.seek(0);
    QJsonDocument jsonDocument = QJsonDocument::fromJson(designTokensFile.readAll());
    if (jsonDocument.isNull())
    {
        qDebug() << errorPrefix << "couldn't read data.";
        return coreData;
    }

    QJsonObject jsonObject = jsonDocument.object();
    QStringList childKeys = jsonObject.keys();
    auto foundMatchingChildKeyIt = std::find_if(childKeys.constBegin(), childKeys.constEnd(), [](const QString& key){
        return key == CoreMain;
    });

    if (foundMatchingChildKeyIt == childKeys.constEnd())
    {
        qDebug() << errorPrefix << "key not found " << CoreMain;
        return coreData;
    }

    jsonObject = jsonObject.value(CoreMain).toObject();
    childKeys = jsonObject.keys();
    foundMatchingChildKeyIt = std::find_if(childKeys.constBegin(), childKeys.constEnd(), [](const QString& key){
        return key == CoreColors;
    });

    if (foundMatchingChildKeyIt == childKeys.constEnd())
    {
        qDebug() << errorPrefix << "key not found " << CoreColors;
        return coreData;
    }

    QJsonObject coreColors = jsonObject.value(CoreColors).toObject();
    recurseCore(CoreColors, coreColors, coreData);

    return coreData;
}

ThemedColorData TokenManager::parseTheme(QFile& designTokensFile, const CoreData& coreData)
{
    const QString errorPrefix = __func__ + QString(" Error : parsing design tokens file, ");

    ThemedColorData themedColorData;

    designTokensFile.seek(0);
    QJsonDocument jsonDocument = QJsonDocument::fromJson(designTokensFile.readAll());
    if (jsonDocument.isNull())
    {
        qDebug() << errorPrefix << "couldn't read data.";
        return themedColorData;
    }

    QJsonObject jsonObject = jsonDocument.object();
    QStringList childKeys = jsonObject.keys();
    std::for_each(childKeys.constBegin(), childKeys.constEnd(), [&](const QString& key)
    {
        if (key.contains(SemanticTokens))
        {
            QJsonObject themeObject = jsonObject.value(key).toObject();

            ColourMap colourMap = Utilities::parseColorTheme(themeObject, coreData);
            if (colourMap.isEmpty())
            {
                qDebug() << errorPrefix << "no color theme data for " << key;
                return;
            }

            QString theme = key.right(key.size() - key.lastIndexOf('/') - 1);
            if (theme.isEmpty())
            {
                qDebug() << errorPrefix << " Error : No valid theme found on key " << key;
                return;
            }

            themedColorData.insert(theme, colourMap);
        }
    });

    return themedColorData;
}



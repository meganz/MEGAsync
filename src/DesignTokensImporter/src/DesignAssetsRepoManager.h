#ifndef DESIGN_ASSETS_REPO_MANAGER_H
#define DESIGN_ASSETS_REPO_MANAGER_H

#include "Types.h"

#include <QMap>
#include <QFile>
#include <QStringList>
#include <QJsonArray>
#include <QJsonDocument>

namespace DTI
{
    class DesignAssetsRepoManager
    {
    public:
        DesignAssetsRepoManager();
        DesignAssets getDesignAssets();

    private:
        ThemedColorData getColorData();
        ThemedColorData parseTheme(QFile& designTokensFile, const ColorData& coreData);
        CoreData parseCore(QFile& designTokensFile);
        void recurseCore(QString category, const QJsonObject& coreColors, CoreData& coreData);
        ColorData parseColorTheme(const QJsonObject& jsonThemeObject, const CoreData& colorData);
    };
}

#endif




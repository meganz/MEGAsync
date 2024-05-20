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

        //!
        //! \brief Creates the color data structure organised by themes so targets can consume it.
        //!
        //! This function has the responsability to parse the themed colors from the design tokens file and
        //! look for the core colours referenced by. It will return a structure with the themed colors organized by themes.
        //!
        //! \param designAssets The design assets structure with all the data gathered from the design tokens.
        //!
        ColorData parseColorTheme(const QJsonObject& jsonThemeObject, const CoreData& colorData);
    };
}

#endif




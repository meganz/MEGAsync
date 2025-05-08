#include "DesignTokensImporter.h"

#include "DesignAssetsRepoManager.h"
#include "DesignGenerator.h"
#include "PathProvider.h"
#include "Utilities.h"

#include <QDir>
#include <QFileInfo>

using namespace DTI;

void DesignTokensImporter::run()
{
    // parse mega design assets repository.
    DesignAssetsRepoManager tokenManager;
    auto designAssets = tokenManager.getDesignAssets();

    // design generator entry point.
    if (designAssets.has_value())
    {
        DesignGenerator designGenerator;
        designGenerator.deploy(designAssets.value());
    }
}

bool DesignTokensImporter::initialize(const QString& megaSyncPath,
                                      const QString& designTokensFilePath)
{
    if (megaSyncPath.isEmpty())
    {
        qCritical() << "MegaSync path is required but was not provided.";
        return false;
    }

    if (designTokensFilePath.isEmpty())
    {
        qCritical() << "Design tokens file path is required but was not provided.";
        return false;
    }

    PathProvider::setMegaSyncPath(QDir(megaSyncPath).absolutePath());
    PathProvider::setDesignTokensFilePath(QDir(designTokensFilePath).absolutePath());

    PathProvider::setUIPath(PathProvider::getMegaSyncPath() + "/gui");

    if (!QDir(PathProvider::getMegaSyncPath()).exists())
    {
        qCritical() << "MegaSync path does not exist:" << PathProvider::getMegaSyncPath();
        return false;
    }

    QFileInfo tokensFileInfo(PathProvider::getDesignTokensFilePath());
    if (!tokensFileInfo.exists())
    {
        qCritical() << "Design tokens file does not exist:"
                    << PathProvider::getDesignTokensFilePath();
        qCritical() << "Did you forget to clone megadesignassets repository?";
        return false;
    }

    Utilities::logInfoMessage("DesignTokensImporter initialized successfully with paths:");
    Utilities::logInfoMessage("  MegaSync path: " + PathProvider::getMegaSyncPath());
    Utilities::logInfoMessage("  UI path: " + PathProvider::getUIPath());
    Utilities::logInfoMessage("  Design tokens file: " + tokensFileInfo.absoluteFilePath());

    return true;
}

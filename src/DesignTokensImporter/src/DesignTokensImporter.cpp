#include "DesignTokensImporter.h"

#include "DesignAssetsRepoManager.h"
#include "DesignGenerator.h"

using namespace DTI;

void DesignTokensImporter::run()
{
    // parse mega design assets repository.
    DesignAssetsRepoManager tokenManager;
    DesignAssets designAssets = tokenManager.getDesignAssets();

    // design generator entry point.
    DesignGenerator designGenerator;
    designGenerator.deploy(designAssets);
}

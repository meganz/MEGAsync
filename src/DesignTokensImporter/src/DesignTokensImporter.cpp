#include "DesignTokensImporter.h"

#include "DesignAssetsRepoManager.h"
#include "DesignGenerator.h"

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

#include "DesignTokensImporter.h"

#include "DesignAssetsRepoManager.h"
#include "IDesignGenerator.h"
#include "DesignGenerator.h"

using namespace DTI;

void DesignTokensImporter::run()
{
    // parse mega design assets repository.
    DesignAssetsRepoManager tokenManager;
    DesignAssets designAssets = tokenManager.getDesignAssets();

    // design generator entry point.
    std::unique_ptr<IDesignGenerator> designGenerator{new DesignGenerator()};
    designGenerator->deploy(designAssets);
}

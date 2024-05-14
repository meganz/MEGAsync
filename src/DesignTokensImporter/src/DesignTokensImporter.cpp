#include "DesignTokensImporter.h"

#include "DesignAssetsRepoManager.h"
#include "IDesignGenerator.h"
#include "QMLDesignGenerator.h"
#include "WidgetsDesignGenerator.h"

using namespace DTI;

void DesignTokensImporter::run()
{
    // parse mega design assets repository.
    DesignAssetsRepoManager tokenManager;
    DesignAssets designAssets = tokenManager.getDesignAssets();

    // qml design generator entry point.
    std::unique_ptr<IDesignGenerator> qmlDesignGenerator{new QMLDesignGenerator()};
    qmlDesignGenerator->deploy(designAssets);

    // widgets design generator entry point.
    std::unique_ptr<IDesignGenerator> widgetsDesignGenerator{new WidgetsDesignGenerator()};
    widgetsDesignGenerator->deploy(designAssets);
}

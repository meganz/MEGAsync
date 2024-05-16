#include "DesignGenerator.h"

#include "targets/DesignTarget.h"
#include "targets/DesignTargetFactory.h"

using namespace DTI;

DesignGenerator::DesignGenerator(QObject* parent)
    : QObject{parent}
{

}

void DesignGenerator::deploy(const DesignAssets& designAssets)
{
    auto registeredDesignTargets = DesignTargetFactory::getRegisteredDesignTargets();

    for (const auto& designTargetId : registeredDesignTargets)
    {
        auto designTargetInstance = DesignTargetFactory::getDesignTarget(designTargetId);

        if (designTargetInstance != nullptr)
        {
            std::unique_ptr<IDesignTarget> designTargetInstanceSafePtr{designTargetInstance};
            designTargetInstanceSafePtr->deploy(designAssets);
        }
    }
}




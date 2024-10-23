#include "DesignGenerator.h"

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
            designTargetInstance->deploy(designAssets);
        }
    }
}

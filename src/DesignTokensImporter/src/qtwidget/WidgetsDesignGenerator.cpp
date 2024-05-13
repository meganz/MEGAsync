#include "WidgetsDesignGenerator.h"

#include "WidgetsDesignTargetFactory.h"

using namespace DTI;

WidgetsDesignGenerator::WidgetsDesignGenerator(QObject* parent)
    : QObject{parent}
{

}

void WidgetsDesignGenerator::start(const ThemedColorData& themedColorData)
{
    auto registeredDesignTargets = WidgetsDesignTargetFactory::getRegisteredDesignTargets();

    for (const auto& designTarget : registeredDesignTargets)
    {
        auto designTargetInstance = WidgetsDesignTargetFactory::getWidgetsDesignTarget(designTarget);

        if (designTargetInstance != nullptr)
        {
            std::unique_ptr<IWidgetsDesignTarget> designTargetInstanceSafePtr{designTargetInstance};
            designTargetInstanceSafePtr->process(themedColorData);
        }
    }
}



